/*
 * gtgattr
 *
 * Handle initial option configuration of attributes and actual attribute table
 * setup and output.
 */

#ifndef _GTGATTR_H_
#define _GTGATTR_H_

#include "shapefil.h"

#include "Eci.h"
#include "CoordGeodetic.h"
#include "Observer.h"

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

void FlagAllAttributes(bool flag_value, bool except_observer_attributes = false);
bool EnableAttribute(const char *desc);
void InitAttributeObserver(bool observer_specified, double lat = 0, double lon = 0, double alt = 0);
void initAttributes(DBFHandle dbf);
void outputAttributes(DBFHandle dbf, int index, Eci& loc, CoordGeodetic& geo);
void CleanupAttribute(void);

#endif
