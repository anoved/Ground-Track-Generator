#include <string.h>

#include "gtgattr.h"
#include "gtgutil.h"

/* DBF width and decimal precision values are presently somewhat arbitrary */
struct attribute_options {
	const char *name;
	DBFFieldType type;
	int width;
	int decimals;
} attribute_options[] = {
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

/* returns index of attribute, if valid, or -1 if not */
int IsValidAttribute(const char *s)
{
	for (int i = 0; i < ATTR_COUNT; i++) {
		if (0 == strcmp(s, attribute_options[i].name)) {
			return i;
		}
	}
	return -1;
}

/* returns true if attribute was enabled; false if not (invalid name) */
bool EnableAttribute(const char *desc)
{
	int attrid = -1;
	if (-1 != (attrid = IsValidAttribute(desc))) {
		attribute_flags[attrid] = true;
		return true;
	}
	return false;
}

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
