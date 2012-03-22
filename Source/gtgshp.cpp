#include <stdio.h>
#include <math.h>

#include "Observer.h"
#include "CoordTopographic.h"

#include "gtgshp.h"
#include "gtgutil.h"

ShapefileWriter::ShapefileWriter(const char *basepath, enum output_feature_type features)
{
	int fieldID;
	
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
			
			// derived from http://geospatialmethods.org/spheres/
			// assumes spherical earth
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
			double c1 = (x0 * y1) - (x1 * y0);
						
			// cartesian coordinates g, h, w for one point where that great
			// circle plane intersects the plane of the prime/180th meridian
			double h = -c1 / a1;
			double w = sqrt(pow(EARTH_RADIUS, 2) / (pow(h, 2) + 1));
			
			// spherical coordinates of intersection points
			double lat1 = Util::RadiansToDegrees(asin(w / EARTH_RADIUS));
			double lon1 = (h * w) < 0 ? 180.0 : 0.0;
			double lat2 = Util::RadiansToDegrees(asin(-w / EARTH_RADIUS));
			double lon2 = (-h * w) < 0 ? 180.0 : 0.0;
					
			// negative range_rate indicates satellite is approaching observer;
			// the point that it is approaching is the point it will cross.
			double intercept = 999; // not a valid latitude we'll encounter
			if (Observer(lat1, lon1, 0).GetLookAngle(*loc).range_rate < 0) {
				if (180.0 == lon1) {
					intercept = lat1;
				}
			} else {
				if (180.0 == lon2) {
					intercept = lat2;
				}
			}
			
			// so now after all that we know whether this segment crosses the
			// 180th meridian. If so, split the segment into two pieces.
			// intercept is [APPROXIMATELY] the latitude at which the great
			// circle arc between loc and nextloc crosses the 180th meridian.
			if (999 != intercept) {
				int parts[2] = {0, 2};
				double xv[4] = {longitude[0], latitude[0], longitude[0] < 0 ? -180 : 180, intercept};
				double yv[4] = {longitude[0] < 0 ? 180 : -180, intercept, longitude[1], latitude[1]};
				if (NULL == (obj = SHPCreateObject(SHPT_ARC, -1, 2, parts, NULL, 4, xv, yv, NULL, NULL))) {
					Fail("cannot create split line segment\n");
				}
				Note("Split segment at dateline at latitude: %lf\n", intercept);
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
