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

Observer *attribute_observer = NULL;

/*
 * If observer_specified, create ground attribute_observer at lat/lon/alt.
 * Otherwise, check whether any attributes that require an observer are enabled,
 * and if so, abort the program.
 */
void InitAttributeObserver(bool observer_specified, double lat, double lon, double alt)
{
	if (observer_specified) {
		attribute_observer = new Observer(lat, lon, alt);
		Note("Observer: %s\n", attribute_observer->GetLocation().ToString().c_str());
	} else {
		for (int attr = ATTR_OBS_FIRST; attr <= ATTR_OBS_LAST; attr++) {
			if (attribute_flags[attr]) {
				Fail("%s attribute requires an --observer\n", attribute_options[attr].name);
			}
		}
	}
}

/*
 * Deallocate attribute-related memory.
 */
void CleanupAttribute(void)
{
	delete attribute_observer;
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
void outputAttributes(DBFHandle dbf, int index, Eci& loc, CoordGeodetic& geo)
{
	Note("Attributes:\n\tFID: %d\n", index);
	DBFWriteIntegerAttribute(dbf, index, 0, index);
	
	if (attribute_flags[ATTR_ALTITUDE]) {
		double alt = geo.altitude;
		Note("\t%s: %lf km\n", attribute_options[ATTR_ALTITUDE].name, alt);
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_ALTITUDE], alt);
	}
	
	if (attribute_flags[ATTR_VELOCITY]) {
		double velocity = loc.GetVelocity().GetMagnitude();
		Note("\t%s: %lf km/s\n", attribute_options[ATTR_VELOCITY].name, velocity);
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_VELOCITY], velocity);
	}
	
	if (attribute_flags[ATTR_TIMEUTC]) {
		const char *timeutc = loc.GetDate().ToString().c_str();
		Note("\t%s: %s\n", attribute_options[ATTR_TIMEUTC].name, timeutc);
		DBFWriteStringAttribute(dbf, index, attribute_field[ATTR_TIMEUTC], timeutc);
	}
	
	if (attribute_flags[ATTR_TIMEUNIX]) {
		double unixtime = (double)(loc.GetDate().ToTime());
		Note("\t%s: %lf seconds\n", attribute_options[ATTR_TIMEUNIX].name, unixtime);
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_TIMEUNIX], unixtime);
	}
		
	if (attribute_flags[ATTR_LATITUDE]) {
		double latitude = Util::RadiansToDegrees(geo.latitude);
		Note("\t%s: %lf\n", attribute_options[ATTR_LATITUDE].name, latitude);
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_LATITUDE], latitude);
	}

	if (attribute_flags[ATTR_LONGITUDE]) {
		double longitude = Util::RadiansToDegrees(geo.longitude);
		Note("\t%s: %lf\n", attribute_options[ATTR_LONGITUDE].name, longitude);
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_LONGITUDE], longitude);
	}
	
	if (attribute_flags[ATTR_OBS_RANGE]) {
		double range = attribute_observer->GetLookAngle(loc).range;
		Note("\t%s: %lf km\n", attribute_options[ATTR_OBS_RANGE].name, range);
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_OBS_RANGE], range);
	}
	
	if (attribute_flags[ATTR_OBS_RATE]) {
		double range_rate = attribute_observer->GetLookAngle(loc).range_rate;
		Note("\t%s: %lf km/s\n", attribute_options[ATTR_OBS_RATE].name, range_rate);
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_OBS_RATE], range_rate);
	}
	
	if (attribute_flags[ATTR_OBS_ELEVATION]) {
		double elevation = Util::RadiansToDegrees(attribute_observer->GetLookAngle(loc).elevation);
		Note("\t%s: %lf\n", attribute_options[ATTR_OBS_ELEVATION].name, elevation);
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_OBS_ELEVATION], elevation);
	}
	
	if (attribute_flags[ATTR_OBS_AZIMUTH]) {
		double azimuth = Util::RadiansToDegrees(attribute_observer->GetLookAngle(loc).azimuth);
		Note("\t%s: %lf\n", attribute_options[ATTR_OBS_AZIMUTH].name, azimuth);
		DBFWriteDoubleAttribute(dbf, index, attribute_field[ATTR_OBS_AZIMUTH], azimuth);
	}
}
