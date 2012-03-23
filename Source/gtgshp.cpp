#include <stdio.h>
#include <math.h>
#include <string.h>

#include "CoordTopographic.h"

#include "gtgutil.h"
#include "gtgattr.h"
#include "gtg.h"

#include "gtgshp.h"

void ShapefileWriter::outputAttributes(int index, Eci *loc, CoordGeodetic *geo)
{
	DBFWriteIntegerAttribute(dbf_, index, 0, index);
	
	if (attribute_flags[ATTR_ALTITUDE]) {
		DBFWriteDoubleAttribute(dbf_, index, attribute_field[ATTR_ALTITUDE],
				geo->altitude);
	}
	
	if (attribute_flags[ATTR_VELOCITY]) {
		DBFWriteDoubleAttribute(dbf_, index, attribute_field[ATTR_VELOCITY],
				loc->GetVelocity().GetMagnitude());
	}
	
	if (attribute_flags[ATTR_TIMEUTC]) {
		DBFWriteStringAttribute(dbf_, index, attribute_field[ATTR_TIMEUTC],
				loc->GetDate().ToString().c_str());
	}
	
	if (attribute_flags[ATTR_TIMEUNIX]) {
		DBFWriteDoubleAttribute(dbf_, index, attribute_field[ATTR_TIMEUNIX],
				(double)(loc->GetDate().ToTime()));
	}
		
	if (attribute_flags[ATTR_LATITUDE]) {
		DBFWriteDoubleAttribute(dbf_, index, attribute_field[ATTR_LATITUDE],
				Util::RadiansToDegrees(geo->latitude));
	}

	if (attribute_flags[ATTR_LONGITUDE]) {
		DBFWriteDoubleAttribute(dbf_, index, attribute_field[ATTR_LONGITUDE],
				Util::RadiansToDegrees(geo->longitude));
	}
	
	if (attribute_flags[ATTR_OBS_RANGE]) {
		DBFWriteDoubleAttribute(dbf_, index, attribute_field[ATTR_OBS_RANGE],
				obs_->GetLookAngle(*loc).range);
	}
	
	if (attribute_flags[ATTR_OBS_RATE]) {
		DBFWriteDoubleAttribute(dbf_, index, attribute_field[ATTR_OBS_RATE],
				obs_->GetLookAngle(*loc).range_rate);
	}
	
	if (attribute_flags[ATTR_OBS_ELEVATION]) {
		DBFWriteDoubleAttribute(dbf_, index, attribute_field[ATTR_OBS_ELEVATION],
				Util::RadiansToDegrees(obs_->GetLookAngle(*loc).elevation));
	}
	
	if (attribute_flags[ATTR_OBS_AZIMUTH]) {
		DBFWriteDoubleAttribute(dbf_, index, attribute_field[ATTR_OBS_AZIMUTH],
				Util::RadiansToDegrees(obs_->GetLookAngle(*loc).azimuth));
	}
}

ShapefileWriter::ShapefileWriter(const char *basepath, enum output_feature_type features,
		double latitude, double longitude, double altitude)
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
	
	/* create the shapefile attribute table */
	dbf_ = DBFCreate(basepath);
	if (NULL == dbf_) {
		Fail("cannot create shapefile attribute table: %s\n", basepath);
	}
	
	/* step id field. (helps ensure valid dbf if no other attrs specified) */
	if (0 != DBFAddField(dbf_, "FID", FTInteger, 20, 0)) {
		Fail("cannot create step index attribute field\n");
	}
	
	initAttributes(dbf_);
	
	obs_ = new Observer(latitude, longitude, altitude);
}

int ShapefileWriter::output(Eci *loc, Eci *nextloc, bool split)
{
	CoordGeodetic locg(loc->ToGeodetic());
	double latitude[2];
	double longitude[2];
	SHPObject *obj = NULL;
	int index;
	int pointc = 1;
	
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
		if (split && ((longitude[0] > 0 && longitude[1] < 0) || (longitude[0] < 0 && longitude[1] > 0))) {
			
			// derived from http://geospatialmethods.org/spheres/
			// assumes spherical earth
			// kXKMPER is WGS-72 earth radius as defined in libsgp4/Globals.h
			
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
			double w = sqrt((kXKMPER * kXKMPER) / (pow(h, 2) + 1));
			
			// spherical coordinates of intersection points
			double lat1 = Util::RadiansToDegrees(asin(w / kXKMPER));
			double lon1 = (h * w) < 0 ? 180.0 : 0.0;
			double lat2 = Util::RadiansToDegrees(asin(-w / kXKMPER));
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
				int parts[2];
				double xv[4];
				double yv[4];
				
				parts[0] = 0;
				xv[0] = longitude[0];
				yv[0] = latitude[0];
				xv[1] = longitude[0] < 0 ? -180 : 180;
				yv[1] = intercept;
				
				parts[1] = 2;
				xv[2] = longitude[0] < 0 ? 180 : -180;
				yv[2] = intercept;
				xv[3] = longitude[1];
				yv[3] = latitude[1];
				
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
	
	outputAttributes(index, loc, &locg);

	Note("Lat: %lf, Lon: %lf\n", latitude[0], longitude[0]);
	
	return index;
}

void ShapefileWriter::close(void)
{
	SHPClose(shp_);
	DBFClose(dbf_);
	delete obs_;
}
