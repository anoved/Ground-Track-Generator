/*
 * gtgattr
 *
 * Handle initial option configuration of attributes and actual attribute table
 * setup and output.
 */

#include <string.h>

#include "gtgutil.h"

#include "gtgattr.h"

/* DBF width and decimal precision values are presently somewhat arbitrary */
typedef struct attribute_options {
	const char *name;
	DBFFieldType type;
	int width;
	int decimals;
} GTGAttributes;

GTGAttributes attribute_options[] = {
		{"altitude", FTDouble, 20, 6},  // geodetic alt of sat (km)
		{"velocity", FTDouble, 20, 6},  // magnitude of sat velocity (km/s)
		{"time", FTString, 31, 0},      // YYYY-MM-DD HH:MM:SS.SSSSSS UTC
		{"unixtime", FTInteger, 20, 0}, // unix time (integer seconds)
		{"latitude", FTDouble, 20, 6},  // geodetic lat of sat
		{"longitude", FTDouble, 20, 6}, // geodetic lon of sat
		
		{"range", FTDouble, 30, 6},     // range (km) to observer
		{"rate", FTDouble, 20, 6},      // range rate (km/s) to observer
		{"elevation", FTDouble, 20, 6}, // elevation of sat from obs station
		{"azimuth", FTDouble, 20, 6}    // azimuth of sat from obs station
};

/* each element is set to true if the corresponding attribute should be output */
bool attribute_flags[ATTR_COUNT];

/* the index of the corresponding field in the output attribute table */
int attribute_field[ATTR_COUNT];

/*
 * Check whether any attributes that require an observer are enabled,
 * and if so, abort the program if observer_specified is false.
 */
void CheckAttributeObserver(bool observer_specified)
{
	if (not observer_specified) {
		for (int attr = ATTR_OBS_FIRST; attr <= ATTR_OBS_LAST; attr++) {
			if (attribute_flags[attr]) {
				Fail("%s attribute requires an --observer\n", attribute_options[attr].name);
			}
		}
	}
}

/*
 * Enable or disable all attributes, according to flag_value.
 * If except_observer_attributes is true, attributes that require an observer
 * are disabled regardless of flag_value.
 */
void FlagAllAttributes(bool flag_value, bool except_observer_attributes)
{
	for (int attr = 0; attr < ATTR_COUNT; attr++) {
		if (except_observer_attributes &&
				(attr >= ATTR_OBS_FIRST && attr <= ATTR_OBS_LAST)) {
			attribute_flags[attr] = false;
		} else {
			attribute_flags[attr] = flag_value;
		}
	}
}

/*
 * Check if the specified attribute name s is recognized.
 * If so, return the index of the corresponding attribute setup record.
 * If not, return -1.
 */
int IsValidAttribute(const char *s)
{
	for (int i = 0; i < ATTR_COUNT; i++) {
		if (0 == strcmp(s, attribute_options[i].name)) {
			return i;
		}
	}
	return -1;
}

/*
 * Enable output of attribute named by desc.
 * Returns false if the attribute name was not recognized; otherwise true.
 */
bool EnableAttribute(const char *desc)
{
	int attrid = -1;
	if (-1 != (attrid = IsValidAttribute(desc))) {
		attribute_flags[attrid] = true;
		return true;
	}
	return false;
}

/*
 * Create output attribute table fields for each attribute that is enabled.
 * Remember the actual index of each attribute table field.
 */
void initAttributes(DBFHandle dbf)
{
	int field;
	for (int attr = 0; attr < ATTR_COUNT; attr++) {
		if (attribute_flags[attr]) {
			field = DBFAddField(dbf, attribute_options[attr].name, 
					attribute_options[attr].type, attribute_options[attr].width,
					attribute_options[attr].decimals);
			if (-1 == field) {
				Fail("cannot create attribute field: %s\n", attribute_options[attr].name);
			}
			attribute_field[attr] = field;
		}
	}
}

/*
 * Output an attribute record (at position index) for the specified loc.
 * Only output enabled attributes.
 */
void outputAttributes(DBFHandle dbf, int index, Eci& loc, CoordGeodetic& geo, Observer &obs)
{
	DBFWriteIntegerAttribute(dbf, index, 0, index);
	
	if (attribute_flags[ATTR_ALTITUDE]) {
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_ALTITUDE],
				geo.altitude);
	}
	
	if (attribute_flags[ATTR_VELOCITY]) {
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_VELOCITY],
				loc.GetVelocity().GetMagnitude());
	}
	
	if (attribute_flags[ATTR_TIMEUTC]) {
		DBFWriteStringAttribute(dbf, index, attribute_field[ATTR_TIMEUTC],
				loc.GetDate().ToString().c_str());
	}
	
	if (attribute_flags[ATTR_TIMEUNIX]) {
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_TIMEUNIX],
				(double)(loc.GetDate().ToTime()));
	}
		
	if (attribute_flags[ATTR_LATITUDE]) {
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_LATITUDE],
				Util::RadiansToDegrees(geo.latitude));
	}

	if (attribute_flags[ATTR_LONGITUDE]) {
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_LONGITUDE],
				Util::RadiansToDegrees(geo.longitude));
	}
	
	if (attribute_flags[ATTR_OBS_RANGE]) {
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_OBS_RANGE],
				obs.GetLookAngle(loc).range);
	}
	
	if (attribute_flags[ATTR_OBS_RATE]) {
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_OBS_RATE],
				obs.GetLookAngle(loc).range_rate);
	}
	
	if (attribute_flags[ATTR_OBS_ELEVATION]) {
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_OBS_ELEVATION],
				Util::RadiansToDegrees(obs.GetLookAngle(loc).elevation));
	}
	
	if (attribute_flags[ATTR_OBS_AZIMUTH]) {
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_OBS_AZIMUTH],
				Util::RadiansToDegrees(obs.GetLookAngle(loc).azimuth));
	}

}
