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
		{"altitude", FTDouble, 20, 9},  // geodetic alt of sat (km)
		{"velocity", FTDouble, 20, 9},  // magnitude of sat velocity (km/s)
		{"time", FTString, 31, 0},      // YYYY-MM-DD HH:MM:SS.SSSSSS UTC
		{"unixtime", FTInteger, 20, 0}, // unix time (integer seconds)
		{"mfe", FTDouble, 20, 9},       // minutes from epoch (time to TLE)
		{"latitude", FTDouble, 20, 9},  // geodetic lat of sat
		{"longitude", FTDouble, 20, 9}, // geodetic lon of sat
		{"xposition", FTDouble, 20, 9}, // ECI x (km)
		{"yposition", FTDouble, 20, 9}, // ECI y (km)
		{"zposition", FTDouble, 20, 9}, // ECI z (km)
		{"xvelocity", FTDouble, 20, 9}, // ECI x velocity (km/s)
		{"yvelocity", FTDouble, 20, 9}, // ECI y velocity (km/s)
		{"zvelocity", FTDouble, 20, 9}, // ECI z velocity (km/s)
		
		{"range", FTDouble, 20, 9},     // range (km) to observer
		{"rate", FTDouble, 20, 9},      // range rate (km/s) to observer
		{"elevation", FTDouble, 20, 9}, // elevation of sat from obs station
		{"azimuth", FTDouble, 20, 9}    // azimuth of sat from obs station
};

/* each element is set to true if the corresponding attribute should be output */
bool attribute_flags[ATTR_COUNT];

/* the index of the corresponding field in the output attribute table */
int attribute_field[ATTR_COUNT];

Observer *attribute_observer = NULL;

/* updated by initAttributes for each TLE processed */
Julian attribute_epoch;

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
void initAttributes(DBFHandle dbf, const Julian& epoch)
{
	int field;
	attribute_epoch = epoch;
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
	
	/* (Consider pre-calculating certain values used in multiple attributes,
	   such as loc.GetDate(), loc.GetPosition(), loc.GetVelocity(), and GetLookAngle(),
	   to avoid redundancy in the case where multiple of them are enabled. */
	
	for (int attr = 0; attr < ATTR_COUNT; attr++) {
		
		/* skip disabled attributes */
		if (!attribute_flags[attr]) {
			continue;
		}
		
		/* handle string and numeric attributes differently */
		if (FTString == attribute_options[attr].type) {
			const char *s;
			switch (attr) {
				case ATTR_TIMEUTC: s = loc.GetDate().ToString().c_str(); break;
				default:
					Fail("unhandled string attribute id: %d\n", attr);
					break;
			}
			Note("\t%s: %s\n", attribute_options[attr].name, s);
			DBFWriteStringAttribute(dbf, index, attribute_field[attr], s);
		} else if (FTInteger == attribute_options[attr].type) {
			long n;
			switch (attr) {
				case ATTR_TIMEUNIX: n = (long)(0.5 + loc.GetDate().ToTime()); break;
				default:
					Fail("unhandled integer attribute id: %d\n", attr);
					break;
			}
			Note("\t%s: %ld\n", attribute_options[attr].name, n);
			DBFWriteIntegerAttribute(dbf, index, attribute_field[attr], n);
		} else if (FTDouble == attribute_options[attr].type) {
			double n;
			switch (attr) {
				case ATTR_TIMEMFE:       n = (loc.GetDate() - attribute_epoch).GetTotalMinutes(); break;
				case ATTR_ALTITUDE:      n = geo.altitude; break;
				case ATTR_VELOCITY:      n = loc.GetVelocity().GetMagnitude(); break;
				case ATTR_LATITUDE:      n = Util::RadiansToDegrees(geo.latitude); break;
				case ATTR_LONGITUDE:     n = Util::RadiansToDegrees(geo.longitude); break;
				case ATTR_POSITION_X:    n = loc.GetPosition().x; break;
				case ATTR_POSITION_Y:    n = loc.GetPosition().y; break;
				case ATTR_POSITION_Z:    n = loc.GetPosition().z; break;
				case ATTR_VELOCITY_X:    n = loc.GetVelocity().x; break;
				case ATTR_VELOCITY_Y:    n = loc.GetVelocity().y; break;
				case ATTR_VELOCITY_Z:    n = loc.GetVelocity().z; break;
				case ATTR_OBS_RANGE:     n = attribute_observer->GetLookAngle(loc).range; break;
				case ATTR_OBS_RATE:      n = attribute_observer->GetLookAngle(loc).range_rate; break;
				case ATTR_OBS_ELEVATION: n = Util::RadiansToDegrees(attribute_observer->GetLookAngle(loc).elevation); break;
				case ATTR_OBS_AZIMUTH:   n = Util::RadiansToDegrees(attribute_observer->GetLookAngle(loc).azimuth); break;
				default:
					Fail("unhandled floating point attribute id: %d\n", attr);
					break;
			}
			Note("\t%s: %.9lf\n", attribute_options[attr].name, n);
			DBFWriteDoubleAttribute(dbf, index, attribute_field[attr], n);
		} else {
			Fail("unhandled attribute type: %d\n", attribute_options[attr].type);
		}
	}
}
