/*
 * gtgattr
 *
 * Handle initial option configuration of attributes and actual attribute table
 * setup and output.
 */

#ifndef _GTGATTR_H_
#define _GTGATTR_H_

#include <stdio.h>

#include "shapefil.h"

#include "Eci.h"
#include "CoordGeodetic.h"
#include "Observer.h"

/* used to parse attributes specified on command line and as dbf field titles */
enum attribute_ids {
	ATTR_LATITUDE = 0,
	ATTR_LONGITUDE,
	
	ATTR_MAIN_FIRST,
	ATTR_TIMEUTC = ATTR_MAIN_FIRST,
	ATTR_TIMEUNIX,
	ATTR_TIMEMFE,
	ATTR_ALTITUDE,
	ATTR_VELOCITY,
	ATTR_POSITION_X,
	ATTR_POSITION_Y,
	ATTR_POSITION_Z,
	ATTR_VELOCITY_X,
	ATTR_VELOCITY_Y,
	ATTR_VELOCITY_Z,
	ATTR_MAIN_LAST = ATTR_VELOCITY_Z,
	
	ATTR_OBS_FIRST,
	ATTR_OBS_RANGE = ATTR_OBS_FIRST,
	ATTR_OBS_RATE,
	ATTR_OBS_ELEVATION,
	ATTR_OBS_AZIMUTH,
	ATTR_OBS_LAST = ATTR_OBS_AZIMUTH,
	
	ATTR_COUNT
};

class AttributeWriter {
public:
	AttributeWriter(const char *basepath, bool has_observer, double lat, double lon, double alt, bool csvMode, bool csvHeader);
	
	~AttributeWriter() {}
	
	void output(int index, double minutes, const Eci& loc, const CoordGeodetic& geo);
		
	void close(void);
	
private:
	DBFHandle dbf_;
	FILE *csv_;
	Observer *observer_;
};

void FlagAllAttributes(bool flag_value, bool except_observer_attributes = false);
bool EnableAttribute(const char *desc);
bool EnableAttributeByID(int id);

#endif
