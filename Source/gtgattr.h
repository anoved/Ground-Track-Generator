#ifndef _GTGATTR_H_
#define _GTGATTR_H_

#include "shapefil.h"

/* used to parse attributes specified on command line and as dbf field titles */
enum attribute_ids {
	ATTR_ALTITUDE = 0,
	ATTR_VELOCITY,
	ATTR_TIMEUTC,
	ATTR_TIMEUNIX,
	ATTR_LATITUDE,
	ATTR_LONGITUDE,
	
	ATTR_OBS_FIRST,
	ATTR_OBS_RANGE = ATTR_OBS_FIRST,
	ATTR_OBS_RATE,
	ATTR_OBS_ELEVATION,
	ATTR_OBS_AZIMUTH,
	ATTR_OBS_LAST = ATTR_OBS_AZIMUTH,
	
	ATTR_COUNT
};

extern bool attribute_flags[];
extern int attribute_field[];

void FlagAllAttributes(bool flag_value, bool except_observer_attributes = false);
bool EnableAttribute(const char *desc);
void CheckAttributeObserver(bool observer_specified);
void initAttributes(DBFHandle dbf);

#endif
