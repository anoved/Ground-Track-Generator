#include <stdio.h>
#include <math.h>

#include "Observer.h"
#include "CoordTopographic.h"

#include "gtgshp.h"
#include "gtgutil.h"

ShapefileWriter::ShapefileWriter(const char *basepath, enum output_format_type format)
{
	int fieldID;
	
	switch (format) {
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
	
	/* create the shapefile attribute table */
	dbf_ = DBFCreate(basepath);
	if (NULL == dbf_) {
		Fail("cannot create shapefile attribute table: %s\n", basepath);
	}
	
	/* eventually the attribute table will be user configurable */
	fieldID = DBFAddField(dbf_, "ALTITUDE", FTDouble, 12, 6);
	if (-1 == fieldID) {
		Fail("cannot add ALTITUDE field to attribute table\n");
	}
	fieldID = DBFAddField(dbf_, "VELOCITY", FTDouble, 12, 6);
	if (-1 == fieldID) {
		Fail("cannot add VELOCITY field to attribute table\n");
	}
}

double DistanceToMeridian(double lon)
{
	double result;
	
	// for prime meridian:
	// result = -1.0 * lon;
	
	if (lon < 0) {
		result = -180 - lon;
	} else {
		result = 180 - lon;
	}
	
	return result;
}

int ShapefileWriter::output(Eci *loc, Eci *nextloc)
{
	CoordGeodetic locg(loc->ToGeodetic());
	double latitude[2];
	double longitude[2];
	SHPObject *obj = NULL;
	int index;
	int pointc = 1;
	
	/* attributes */
	double altitude = locg.altitude;
	double velocity = loc->GetVelocity().GetMagnitude();
	
	/* loc is used for points and line segment start */
	latitude[0] = Util::RadiansToDegrees(locg.latitude);
	longitude[0] = Util::RadiansToDegrees(locg.longitude);
	
	/* nextloc is used for line segment end, if needed */
	if (NULL != nextloc && shpFormat_ == SHPT_ARC) {
		/* not necessary to keep nextlocg around; we use loc for all attributes */
		CoordGeodetic nextlocg(nextloc->ToGeodetic());
		pointc = 2;
		latitude[1] = Util::RadiansToDegrees(nextlocg.latitude);
		longitude[1] = Util::RadiansToDegrees(nextlocg.longitude);

		/* This line segment's endpoints are in different E/W hemispheres */		
		if (cfg.split && ((longitude[0] > 0 && longitude[1] < 0) || (longitude[0] < 0 && longitude[1] > 0))) {
			
			// assumes spherical earth
			// derived from http://geospatialmethods.org/spheres/
			// (algorithm described at http://geospatialmethods.org/spheres/GCIntersect.html#GCIGC)
			// convert this math to use constants and functions from SGP4++
			
			// for the purpose of determining whether this segment crosses the
			// 180th or prime meridian, the radius (and shape) of the earth
			// don't really matter. However, for 
			
			double EARTH_RADIUS = 6367.435; // km
			
			// cartesian coordinates of satellite points
			double radlon0 = Util::DegreesToRadians(longitude[0]);
			double radlat0 = Util::DegreesToRadians(latitude[0]);
			double radlon1 = Util::DegreesToRadians(longitude[1]);
			double radlat1 = Util::DegreesToRadians(latitude[1]);
			double x0 = cos(radlon0) * cos(radlat0);
			double y0 = sin(radlon0) * cos(radlat0);
			double z0 = sin(radlat0);
			double x1 = cos(radlon1) * cos(radlat1);
			double y1 = sin(radlon1) * cos(radlat1);
			double z1 = sin(radlat1);
			
			// coefficients of great circle plane defined by satellite points
			double a1 = (y0 * z1) - (y1 * z0);
			double b1 = (x1 * z0) - (x0 * z1);
			double c1 = (x0 * y1) - (x1 * y0);
						
			// coefficients of great circle plane for prime/180th meridian
			//double a2 = 0.0;
			//double b2 = 1.0;
			//double c2 = 0.0;
						
			// g, h, w intersection point...
			//double numerator = 0; // ((a2 * c1) - (c2 * a1));
			//double denominator = a1; // ((b2 * a1) - (a2 * b1));
			double g = 0; // numerator / denominator;
			
			// numerator = ((-g * b1) - c1);
			// numerator = -c1; // since g is 0
			double h = -c1 / a1; // numerator / a1;
			
			double numerator = pow(EARTH_RADIUS, 2);
			double denominator = pow(h, 2) + pow(g, 2) + 1;
			double w = sqrt(numerator / denominator);
			
			// cartesian coordinates of intersection points
			double h1 = h * w;
			double g1 = g * w;
			double w1 = w;
			double h2 = -h * w;
			double g2 = -g * w;
			double w2 = -w;
			
			// spherical coordinates of intersection points
			double lat1 = Util::RadiansToDegrees(asin(w1 / EARTH_RADIUS));
			double lon1 = Util::RadiansToDegrees(atan2(g1, h1));
			double lat2 = Util::RadiansToDegrees(asin(w2 / EARTH_RADIUS));
			double lon2 = Util::RadiansToDegrees(atan2(g2, h2));		
					
			// where the approachrate < 0 is the meridian that is being approached.
			Note("\tSegment great circle intersects 0/180 meridian great circle at:\n");
			Note("\tlat1: %lf, lon1: %lf\n", lat1, lon1);
			Note("\tlat2: %lf, lon2: %lf\n", lat2, lon2);
			
			double yintercept = 999;
			
			if (Observer(lat1, lon1, 0).GetLookAngle(*loc).range_rate < 0) {
				// crossing pt1; is it the 180th?
				if (180 == fabs(lon1)) {
					// yes; crossing pt1, 180th - output split
					yintercept = lat1;
					Note("\tCrosses 180th meridian at lat1: %lf\n", yintercept);
				} else {
					// crossing pt1, 0 - ignore
					Note("\tCrosses prime meridian at lat1; ignoring\n");
				}
			} else {
				// crossing pt2; is it the 180th?
				if (180 == fabs(lon2)) {
					// yes, crossing pt2, 180th - output split
					yintercept = lat2;
					Note("\tCrosses 180th meridian at lat2: %lf\n", yintercept);
				} else {
					// crossing pt2, 0 - ignore
					Note("\tCrosses prime meridian at lat2; ignoring\n");
				}
			}
			
			if (yintercept != 999) {
				double xv[4];
				double yv[4];
				int parts[2];
				
				parts[0] = 0;
				xv[0] = longitude[0];
				yv[0] = latitude[0];
				xv[1] = longitude[0] < 0 ? -180 : 180;
				yv[1] = yintercept;
				
				parts[1] = 2;
				xv[2] = longitude[0] < 0 ? 180 : -180;
				yv[2] = yintercept;
				xv[3] = longitude[1];
				yv[3] = latitude[1];
							
				// output split line segment shape
				if (NULL == (obj = SHPCreateObject(shpFormat_, -1, 2, parts,
						NULL, 4, xv, yv, NULL, NULL))) {
					Fail("cannot create split line segment\n");
				}
			}
		}
			
	} else if (shpFormat_ == SHPT_ARC) {
		Fail("line output requires two points; only one received\n");
	}
		
	/* output the geometry */
	if (NULL == obj) {
		// in most cases we'll need to do this, but if we crossed a key meridian
		// we'll already have a split shape object to output
		obj = SHPCreateSimpleObject(shpFormat_, pointc, longitude, latitude, NULL);
	}
	if (NULL == obj) {
		Fail("cannot create shape\n"); // not very informative
	}
	index = SHPWriteObject(shp_, -1, obj);
	SHPDestroyObject(obj);
	
	/* placeholder attribute output */
	DBFWriteDoubleAttribute(dbf_, index, 0, altitude);
	DBFWriteDoubleAttribute(dbf_, index, 1, velocity);

	Note("Lat: %lf, Lon: %lf, Alt: %lf, VelMag: %lf, Vel: %s\n", latitude[0], longitude[0], altitude, velocity, loc->GetVelocity().ToString().c_str());
	
	return index;
}

void ShapefileWriter::close(void)
{
	SHPClose(shp_);
	DBFClose(dbf_);
}
