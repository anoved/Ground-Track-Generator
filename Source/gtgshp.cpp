#include <stdio.h>

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
		Fail("Could not create shapefile.\n");
	}
	
	/* create the shapefile attribute table */
	dbf_ = DBFCreate(basepath);
	if (NULL == dbf_) {
		Fail("Could not create attribute table.\n");
	}
	
	/* eventually the attribute table will be user configurable */
	fieldID = DBFAddField(dbf_, "ALTITUDE", FTDouble, 12, 6);
	if (-1 == fieldID) {
		Fail("Could not add Altitude field to attribute table.\n");
	}
	Note("Altitude field ID: %d\n", fieldID);
}

void ShapefileWriter::outputPoint(int feature, Eci eci)
{
	CoordGeodetic cg(eci.ToGeodetic());
	double latitude = Util::RadiansToDegrees(cg.latitude);
	double longitude = Util::RadiansToDegrees(cg.longitude);
	double altitude = cg.altitude;
	SHPObject *obj;
	int entityID;
	
	/* output the geometry */
	obj = SHPCreateSimpleObject(SHPT_POINT, 1, &longitude, &latitude, NULL);
	if (NULL == obj) {
		Fail("Point creation failed at feature %d\n", feature);
	}
	entityID = SHPWriteObject(shp_, -1, obj);
	SHPDestroyObject(obj);
	
	/* output the attribute */
	DBFWriteDoubleAttribute(dbf_, feature, 0, altitude);
	
	/* more explicit ID tracking/assignment would be a good */
}

void ShapefileWriter::outputLine(int feature, Eci start, Eci end)
{
	CoordGeodetic startCg(start.ToGeodetic()), endCg(end.ToGeodetic());
	double latitude[2];
	double longitude[2];
	double altitude = startCg.altitude;
	SHPObject *obj;
	int entityID;
	
	latitude[0] = Util::RadiansToDegrees(startCg.latitude);
	latitude[1] = Util::RadiansToDegrees(endCg.latitude);
	
	longitude[0] = Util::RadiansToDegrees(startCg.longitude);
	longitude[1] = Util::RadiansToDegrees(endCg.longitude);
	
	obj = SHPCreateSimpleObject(SHPT_ARC, 2, longitude, latitude, NULL);
	if (NULL == obj) {
		Fail("Line creation failed at feature %d\n", feature);
	}
	entityID = SHPWriteObject(shp_, -1, obj);
	SHPDestroyObject(obj);
	
	DBFWriteDoubleAttribute(dbf_, feature, 0, altitude);
}

void ShapefileWriter::close(void)
{
	SHPClose(shp_);
	DBFClose(dbf_);
}
