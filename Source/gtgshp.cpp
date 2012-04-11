/*
 * gtgshp
 *
 * Manage output to shapefile.
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <fstream>

#include "CoordTopographic.h"

#include "gtgutil.h"
#include "gtgattr.h"
#include "gtg.h"

#include "gtgshp.h"

/*
 * Prepare to generate output. Creates shapefile (point or line type determined
 * by features argument) at basepath. Creates projection file if create_prj true.
 */
ShapefileWriter::ShapefileWriter(const char *basepath, enum output_feature_type features, bool create_prj)
{
	switch (features) {
		case point:
			shpFormat_ = SHPT_POINT;
			break;
		case line:
			shpFormat_ = SHPT_ARC;
			break;
	}
	
	/* create the shapefile geometry */
	shp_ = SHPCreate(basepath, shpFormat_);
	if (NULL == shp_) {
		Fail("cannot create shapefile: %s\n", basepath);
	}
		
	if (create_prj) {
		CreateWGS72prj(basepath);
	}
}

/*
 * Output a "Well Known Text" .prj file explicitly stating geodetic system.
 * SGP4++ uses WGS-72 (see kXKMPER & kF in Globals.h).
 * "Revising Spacetrack Report 3" defaults to WGS-72 (see "Constants", p. 15).
 * WGS-72 in various formats: http://spatialreference.org/ref/epsg/4322/
 */
void ShapefileWriter::CreateWGS72prj(const char *basepath)
{
	std::string prjpath(basepath);
	std::ofstream prjf;
	
	prjpath += ".prj";
	prjf.open(prjpath.c_str());	
	if (!prjf.is_open()) {
		Fail("cannot create prj file: %s\n", prjpath.c_str());
	}
	
	prjf << "GEOGCS[\"WGS 72\",DATUM[\"WGS_1972\",SPHEROID[\"WGS 72\",6378135,298.26]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4322\"]]" << std::endl;
	
	prjf.close();
}

/*
 * Given two points (a and b) that are in different hemispheres, return a
 * feature consisting of two line segments split at the 180th meridian IFF
 * the satellite path from a to b crosses it, as determined by approach rate.
 * (Otherwise, if the path crosses the 0 prime meridian, return NULL.)
 * The split point is not computed with the same precision as trace points!
 * Splitting line segments is intended as a cosmetic display convenience.
 */
SHPObject* ShapefileWriter::splitSegment(
		double lata, double lona, double latb, double lonb, const Eci& loc)
{
	SHPObject *obj = NULL;
	
	/*
		I have the geodetic coordinates for two points (0 and 1).
		I want to find the geodetic latitude of the point where a
		certain intervening meridian intersects the shortest line
		between the two points.
		
		With a spherical earth, I can treat the arc between the two
		points as part of a great circle, and find its intersection
		with the meridian's great circle.
		
		How can I find the intersection point in the case of an
		oblate spheroid earth?
		
		The generic term for the shortest
		surface arc between the two points is a "geodesic"; on a
		spherical earth, the geodesic between the two points is
		simply an arc segment of a great circle. Here is some info
		on geodesics of oblate spheroids, which may be helpful:
		
		http://mathworld.wolfram.com/OblateSpheroidGeodesic.html
	*/
	
	// derived from http://geospatialmethods.org/spheres/
	// assumes spherical earth
	// kXKMPER is WGS-72 earth radius as defined in libsgp4/Globals.h
	
	// cartesian coordinates of satellite points
	double radlon0 = Util::DegreesToRadians(lona);
	double radlat0 = Util::DegreesToRadians(lata);
	double radlon1 = Util::DegreesToRadians(lonb);
	double radlat1 = Util::DegreesToRadians(latb);
	double x0 = cos(radlon0) * cos(radlat0);
	double y0 = sin(radlon0) * cos(radlat0);
	double z0 = sin(radlat0);
	double x1 = cos(radlon1) * cos(radlat1);
	double y1 = sin(radlon1) * cos(radlat1);
	double z1 = sin(radlat1);
	
	// coefficients of great circle plane defined by satellite points
	double a1 = (y0 * z1) - (y1 * z0);
	double c1 = (x0 * y1) - (x1 * y0);
				
	// cartesian coordinates g, h, w for one point where that great
	// circle plane intersects the plane of the prime/180th meridian
	double h = -c1 / a1;
	double w = sqrt((kXKMPER * kXKMPER) / (pow(h, 2) + 1));
	
	// spherical coordinates of intersection points
	double lat1 = Util::RadiansToDegrees(asin(w / kXKMPER));
	double lon1 = (h * w) < 0 ? 180.0 : 0.0;
	double lat2 = Util::RadiansToDegrees(asin(-w / kXKMPER));
	double lon2 = (-h * w) < 0 ? 180.0 : 0.0;
			
	// negative range_rate indicates satellite is approaching observer;
	// the point that it is approaching is the point it will cross.
	double intercept;
	bool intersects_180th = false;
	if (Observer(lat1, lon1, 0).GetLookAngle(loc).range_rate < 0) {
		if (180.0 == lon1) {
			intersects_180th = true;
			intercept = lat1;
		}
	} else {
		if (180.0 == lon2) {
			intersects_180th = true;
			intercept = lat2;
		}
	}
	
	// so now after all that we know whether this segment crosses the
	// 180th meridian. If so, split the segment into two pieces.
	// intercept is [APPROXIMATELY] the latitude at which the great
	// circle arc between loc and nextloc crosses the 180th meridian.
	if (intersects_180th) {
		int parts[2];
		double xv[4];
		double yv[4];
		
		parts[0] = 0;
		xv[0] = lona;
		yv[0] = lata;
		xv[1] = lona < 0 ? -180 : 180;
		yv[1] = intercept;
		
		parts[1] = 2;
		xv[2] = lona < 0 ? 180 : -180;
		yv[2] = intercept;
		xv[3] = lonb;
		yv[3] = latb;
		
		if (NULL == (obj = SHPCreateObject(SHPT_ARC, -1, 2, parts, NULL, 4, xv, yv, NULL, NULL))) {
			Fail("cannot create split line segment\n");
		}
		
		Note("Split on 180th meridian at latitude: %lf\n", intercept);
	}
	
	return obj;
}

/*
 * Write the ground trace coordinates of the given satellite loc to output.
 * If specified, nextloc specifies location of satellite at next trace step -
 * used here only if outputting line features for the segment endpoint. If split
 * is true, then line features that cross the dateline will be split into east
 * and west hemisphere segments. Attributes for loc are also output.
 */
int ShapefileWriter::output(const Eci& loc, const CoordGeodetic& geo, Eci *nextloc, bool split, bool rawOutput)
{
	double latitude[2];
	double longitude[2];
	SHPObject *obj = NULL;
	int index;
	int pointc = 1;

	/* geo loc is used for points and line segment start */
	latitude[0] = Util::RadiansToDegrees(geo.latitude);
	longitude[0] = Util::RadiansToDegrees(geo.longitude);

	Note("Latitude: %lf\n", latitude[0]);
	Note("Longitude: %lf\n", longitude[0]);
	
	if (rawOutput) {
		printf("%.9lf,%.9lf", latitude[0], longitude[0]);
	}
	
	/* nextloc is used for line segment end, if needed */
	if (NULL != nextloc && shpFormat_ == SHPT_ARC) {
		/* not necessary to keep nextgeo around; we use loc for all attributes */
		CoordGeodetic nextgeo(nextloc->ToGeodetic());
		pointc = 2;
		latitude[1] = Util::RadiansToDegrees(nextgeo.latitude);
		longitude[1] = Util::RadiansToDegrees(nextgeo.longitude);

		/* This line segment's endpoints are in different E/W hemispheres */		
		if (split && ((longitude[0] > 0 && longitude[1] < 0) || (longitude[0] < 0 && longitude[1] > 0))) {
			/* If the segment crosses the 180th meridian, splitSegment will
			   split this line at the 180th meridian and return a SHPObject.
			   Otherwise (if the segment crosses the prime meridian), NULL. */
			obj = splitSegment(latitude[0], longitude[0], latitude[1], longitude[1], loc);		
		}
	} else if (shpFormat_ == SHPT_ARC) {
		Fail("line output requires two points; only one received\n");
	}
		
	if (NULL == obj) {
		obj = SHPCreateSimpleObject(shpFormat_, pointc, longitude, latitude, NULL);
	}
	if (NULL == obj) {
		Fail("cannot create shape\n"); // not very informative
	}
	index = SHPWriteObject(shp_, -1, obj);
	SHPDestroyObject(obj);
	
	return index;
}

/*
 * Conclude output by closing shapefile and cleaning up.
 */
void ShapefileWriter::close(void)
{
	SHPClose(shp_);
}
