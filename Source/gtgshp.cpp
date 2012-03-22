#include <stdio.h>
#include <math.h>

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

		/* This line segment's endpoints are in different E/W hemispheres;
	    */		
		if (cfg.split && ((longitude[0] > 0 && longitude[1] < 0) || (longitude[0] < 0 && longitude[1] > 0))) {

			/* FUDGEWORK. We assume this line segment crosses the 180th
			   meridian instead of the prime meridian simply because both
			   endpoint longitude values are closer to 180 than 0. This may
			   be incorrect in a variety of cases. For example, large model
			   intervals may result in such widely spaced points that the
			   trace skips from hemisphere to hemisphere (or orbit to orbit).
			   
			   Practically speaking, --format line is intended to produce
			   smooth-looking track lines, so OK small intervals are typical.
			   (Close to the poles, the absolute size of even small intervals
			   may place endpoints such that this is confused. In these cases,
			   even if it is correctly determined that the track crosses the
			   180th meridian, the cartesian intercept interpolation may split
			   the intercept in the wrong place. Great circles, man!)
			   
			   We could forget about trying to ID which meridian, and just
			   output split segments for the 180th AND prime meridians.
			   However, limitations of this interpolation technique still
			   apply - and it requires the distance to the crossed meridian,
			   meaning the problem of identifying which that is is not resolved.
			   
			   (Looking at polar test cases are helpful.)
			   
			   Bottom line, this |longitudes| > 90 test is bogus.
			   
			   */
			if (fabs(longitude[0]) > 90 && fabs(longitude[1]) > 90) {
				
				/* naive interpolation of meridian intercept */
				double x1 = longitude[0];
				double y1 = latitude[0];
				double x2 = longitude[1];
				double y2 = latitude[1];						
				double d1 = fabs(DistanceToMeridian(x1));
				double d2 = fabs(DistanceToMeridian(x2));
				double dy = y2 - y1;
				double dx = d1 + d2;
				double m  = dy / dx;
				double yi = (m * DistanceToMeridian(x1)) + y1;
				
				/* all vertices of multipart segments go in the same xy arrays;
				   a separate "part start" array points to the start of each */
				double xv[4];
				double yv[4];
				int partindices[2];
				
				partindices[0] = 0;
				xv[0] = x1;
				yv[0] = y1;			
				xv[1] = x1 < 0 ? -180 : 180;
				yv[1] = yi;
				
				partindices[1] = 2;
				xv[2] = x1 < 0 ? 180 : -180;
				yv[2] = yi;
				xv[3] = x2;
				yv[3] = y2;
	
				Note("180th meridian latitude intercept: %lf\n", yi);
				if (NULL == (obj = SHPCreateObject(shpFormat_, -1, 2, partindices, NULL, 4, xv, yv, NULL, NULL))) {
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

	Note("Lat: %lf, Lon: %lf, Alt: %lf, Vel: %lf\n", latitude[0], longitude[0], altitude, velocity);
	
	return index;
}

void ShapefileWriter::close(void)
{
	SHPClose(shp_);
	DBFClose(dbf_);
}
