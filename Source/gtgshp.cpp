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

// assumes we're dealing with the ±180 meridian, not the 0 meridian
// determining which is which is a tricky issue.
// best idea for a solution so far is to choose whichever meridian
// is closer to the longitude; this implies some arbitrary limit on interval
// size - steps which span a half hemisphere or more would trick that method


double DistanceToMeridian(double lon, bool prime)
{
	double result;
	
	if (prime) {
		result = -1.0 * lon;
	} else {
		if (lon < 0) {
			result = -180 - lon;
		} else {
			result = 180 - lon;
		}
	}	
	
	return result;
}


double AbsoluteDistanceToMeridian(double lon, bool prime)
{
	return fabs(DistanceToMeridian(lon, prime));
}

int ShapefileWriter::output(Eci *loc, Eci *nextloc)
{
	CoordGeodetic locg(loc->ToGeodetic());
	double latitude[2];
	double longitude[2];
	SHPObject *obj;
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
		
		if ((longitude[0] > 0 && longitude[1] < 0) || (longitude[0] < 0 && longitude[1] > 0)) {
			// crosses a meridian.
			// we really only care about the prime antimeridian (±180˚ longitude)

			double x1 = longitude[0];
			double y1 = latitude[0];
			double x2 = longitude[1];
			double y2 = latitude[1];

			printf("meridian crossing detected\n");
	
			bool prime;
			if (fabs(x1) < 90 && fabs(x2) < 90) {
				prime = true;
				printf("my guess is that this is the prime meridian\n");
			} else {
				prime = false;
				printf("my guess is that this is the 180th meridian\n");
			}
						
			double d1 = AbsoluteDistanceToMeridian(x1, prime);
			double d2 = AbsoluteDistanceToMeridian(x2, prime);
			double dy = y2 - y1;
			double dx = d1 + d2;
			double m  = dy / dx;
			double ym = m * DistanceToMeridian(x1, prime) + y1;
			
			printf("suggested latitude intercept ym = %lf\n", ym);
			
						
		}
			
	} else if (shpFormat_ == SHPT_ARC) {
		Fail("line output requires two points; only one received\n");
	}
		
	/* output the geometry */
	obj = SHPCreateSimpleObject(shpFormat_, pointc, longitude, latitude, NULL);
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
