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
		{"latitude", FTDouble, 20, 9},  // geodetic lat of sat
		{"longitude", FTDouble, 20, 9}, // geodetic lon of sat

		{"time", FTString, 31, 0},      // YYYY-MM-DD HH:MM:SS.SSSSSS UTC
		{"unixtime", FTInteger, 20, 0}, // unix time (integer seconds)
		{"mfe", FTDouble, 20, 8},       // minutes from epoch (time to TLE)
		{"altitude", FTDouble, 20, 9},  // geodetic alt of sat (km)
		{"velocity", FTDouble, 20, 9},  // magnitude of sat velocity (km/s)
		{"heading", FTDouble, 20, 9},   // degrees clockwise from north
		{"xposition", FTDouble, 20, 8}, // ECI x (km)
		{"yposition", FTDouble, 20, 8}, // ECI y (km)
		{"zposition", FTDouble, 20, 8}, // ECI z (km)
		{"xvelocity", FTDouble, 20, 9}, // ECI x velocity (km/s)
		{"yvelocity", FTDouble, 20, 9}, // ECI y velocity (km/s)
		{"zvelocity", FTDouble, 20, 9}, // ECI z velocity (km/s)
		{"shadow", FTInteger, 2, 0},    // 0 illuminated, 1 penumbral, 2 umbral
		
		{"range", FTDouble, 20, 9},     // range (km) to observer
		{"rate", FTDouble, 20, 9},      // range rate (km/s) to observer
		{"elevation", FTDouble, 20, 9}, // elevation of sat from obs station
		{"azimuth", FTDouble, 20, 9},   // azimuth of sat from obs station
		{"solarelev", FTDouble, 20, 9}, // elevation of sun from obs station
		{"solarazim", FTDouble, 20, 9}  // azimuth of sun from obs station
};

/* each element is set to true if the corresponding attribute should be output */
bool attribute_flags[ATTR_COUNT];

/* the index of the corresponding field in the output attribute table */
int attribute_field[ATTR_COUNT];

AttributeWriter::AttributeWriter(const char *basepath, bool has_observer, double lat, double lon, double alt, bool csvMode, bool csvHeader)
{
	/* First thing first - */
	if (has_observer) {
		observer_ = new Observer(lat, lon, alt);
	} else {
		observer_ = NULL;
		for (int attr = ATTR_OBS_FIRST; attr <= ATTR_OBS_LAST; attr++) {
			if (attribute_flags[attr]) {
				Fail("attribute %s requires an --observer\n", attribute_options[attr].name);
			}
		}
	}
	
	if (csvMode) {

		/* Setup for CSV (plain text) attribute output */
		
		/* Just to be clear - we're not using the dbf_ in this case. */
		dbf_ = NULL;
		
		/* Open the output stream specified by basepath. */
		/* (Or, if it's NULL, just use standard output.) */
		if (NULL != basepath) {
			if (NULL == (csv_ = fopen(basepath, "w"))) {
				Fail("cannot create CSV attribute table: %s\n", basepath);
			}
		} else {
			csv_ = stdout;
		}
		
		/* optionally print header row */
		if (csvHeader) {
			fprintf(csv_, "id");
			for (int attr = 0; attr < ATTR_COUNT; attr++) {
				if (attribute_flags[attr]) {
					fprintf(csv_, ",%s", attribute_options[attr].name);
				}
			}
			fprintf(csv_, "\n");
		}
		
	} else {
		
		/* Setup for DBF (shapefile) attribute output */
		
		/* Just to be clear - we're not using the csv_ in this case. */
		csv_ = NULL;
		
		/* open the attribute table */
		dbf_ = DBFCreate(basepath);
		if (NULL == dbf_) {
			Fail("cannot create attribute table: %s\n", basepath);
		}
		
		/* create the id field */
		if (0 != DBFAddField(dbf_, "FID", FTInteger, 10, 0)) {
			Fail("cannot create attribute index field\n");
		}
		
		/* initialize the attribute table */
		for (int attr = 0; attr < ATTR_COUNT; attr++) {
			if (attribute_flags[attr]) {
				int field = DBFAddField(dbf_,
						attribute_options[attr].name,
						attribute_options[attr].type,
						attribute_options[attr].width,
						attribute_options[attr].decimals);
				if (-1 == field) {
					Fail("cannot create attribute field: %s\n", attribute_options[attr].name);
				}
				attribute_field[attr] = field;
			}
		}
	}
	
	sun_ = new SolarPosition;
}

/*
 * Close the DBF file (which writes it) and release the observer_ memory.
 */
void AttributeWriter::close(void)
{
	if (NULL != dbf_) {
		DBFClose(dbf_);
	}
	if ((NULL != csv_) && (stdout != csv_)) {
		fclose(csv_);
	}	
	delete observer_;
	delete sun_;
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
 * Enable output of attribute identified by id.
 * Returns false if attribute ID is out of range; otherwise true.
 */
bool EnableAttributeByID(int id)
{
	if (id >= 0 && id < ATTR_COUNT) {
		attribute_flags[id] = true;
		return true;
	}
	return false;
}

/*
 * Output an attribute record (at position index) for the specified loc.
 * Only output enabled attributes.
 */
void AttributeWriter::output(int index, double mfe, const Eci& loc, const CoordGeodetic& geo)
{
	Note("Attributes:\n\tFID: %d\n", index);
	if (NULL != csv_) {
		fprintf(csv_, "%d", index);
	} else {
		DBFWriteIntegerAttribute(dbf_, index, 0, index);
	}
	
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
			
			if (NULL != csv_) {
				fprintf(csv_, ",%s", s);
			} else {
				DBFWriteStringAttribute(dbf_, index, attribute_field[attr], s);
			}
			
		} else if (FTInteger == attribute_options[attr].type) {
			long n;
			switch (attr) {
				case ATTR_TIMEUNIX: n = (long)(0.5 + loc.GetDate().ToTime()); break;
				case ATTR_ILLUMINATION:
					{
						// "Visually Observing Earth Satellites" by T.S. Kelso,
						// 1996, http://www.celestrak.com/columns/v03n01/
						
						Vector sunToEarthVector = sun_->FindPosition(loc.GetDate()).GetPosition();
						Vector satToEarthVector = loc.GetPosition();
						Vector satToSunVector = satToEarthVector.Subtract(sunToEarthVector);
						
						// apparent width of earth radius, in radians, from satellite's perspective
						// kXKMPER is WGS 72 equatorial earth radius, in km, from libsgp4/Globals.h
						// (so this is not geodetically correct - radius depends on our position)
						double earthSemidiameter = asin(kXKMPER/satToEarthVector.GetMagnitude());
						
						// apparent width of solar radius, in radians, from satellite's perspective
						// using 695500.0 as very approximate solar radius
						double sunSemidiameter = asin(695500.0/satToSunVector.GetMagnitude());
						
						double earthSunAngle = acos(satToEarthVector.Dot(satToSunVector) /
								(satToEarthVector.GetMagnitude() * satToSunVector.GetMagnitude()));
						
						// by default, consider the satellite illuminated
						n = ATTR_ILLUMINATION_ILLUMINATED;
						if ((earthSemidiameter > sunSemidiameter) &&
								(earthSunAngle < (earthSemidiameter - sunSemidiameter))) {
							// in this case, satellite is in umbral eclipse (full shadow)
							n = ATTR_ILLUMINATION_UMBRAL;
						} else if ((fabs(earthSemidiameter - sunSemidiameter) < earthSunAngle)
								&& (earthSunAngle < (earthSemidiameter + sunSemidiameter))) {
							// in this case, satellite is in penumbral eclipse (partial shadow)
							n = ATTR_ILLUMINATION_PENUMBRAL;
						}
						
					}
					break;
				default:
					Fail("unhandled integer attribute id: %d\n", attr);
					break;
			}
			Note("\t%s: %ld\n", attribute_options[attr].name, n);
			
			if (NULL != csv_) {
				fprintf(csv_, ",%ld", n);
			} else {
				DBFWriteIntegerAttribute(dbf_, index, attribute_field[attr], n);
			}
			
		} else if (FTDouble == attribute_options[attr].type) {
			double n;
			switch (attr) {
				case ATTR_LATITUDE:      n = Util::RadiansToDegrees(geo.latitude); break;
				case ATTR_LONGITUDE:     n = Util::RadiansToDegrees(geo.longitude); break;
				case ATTR_TIMEMFE:       n = mfe; break;
				case ATTR_ALTITUDE:      n = geo.altitude; break;
				case ATTR_VELOCITY:      n = loc.GetVelocity().GetMagnitude(); break;
				case ATTR_HEADING:
					{
						// offsetting position by one centimeter in the direction
						// of current travel. taking look angle azimuth from
						// sub-satellite ground position to this offset position
						// as the heading. it's an approximate hack in lieue of
						// converting ECI coordinates to ECEF->NED to get heading.
						Vector motion = loc.GetVelocity();
						double motionMagnitude = motion.GetMagnitude();
						// offset from loc by a centimeter (unit motion vector in km * 0.00001)
						Eci offsetLoc(loc.GetDate(), Vector(
								loc.GetPosition().x + motion.x / motionMagnitude * 0.00001,
								loc.GetPosition().y + motion.y / motionMagnitude * 0.00001,
								loc.GetPosition().z + motion.z / motionMagnitude * 0.00001));
						Observer nadir(Util::RadiansToDegrees(geo.latitude),
								Util::RadiansToDegrees(geo.longitude), 0);
						n = Util::RadiansToDegrees(nadir.GetLookAngle(offsetLoc).azimuth);
					}
					break;
				case ATTR_POSITION_X:    n = loc.GetPosition().x; break;
				case ATTR_POSITION_Y:    n = loc.GetPosition().y; break;
				case ATTR_POSITION_Z:    n = loc.GetPosition().z; break;
				case ATTR_VELOCITY_X:    n = loc.GetVelocity().x; break;
				case ATTR_VELOCITY_Y:    n = loc.GetVelocity().y; break;
				case ATTR_VELOCITY_Z:    n = loc.GetVelocity().z; break;
				case ATTR_OBS_RANGE:     n = observer_->GetLookAngle(loc).range; break;
				case ATTR_OBS_RATE:      n = observer_->GetLookAngle(loc).range_rate; break;
				case ATTR_OBS_ELEVATION: n = Util::RadiansToDegrees(observer_->GetLookAngle(loc).elevation); break;
				case ATTR_OBS_AZIMUTH:   n = Util::RadiansToDegrees(observer_->GetLookAngle(loc).azimuth); break;
				case ATTR_OBS_SOLARELEV: n = Util::RadiansToDegrees(observer_->GetLookAngle(sun_->FindPosition(loc.GetDate())).elevation); break;
				case ATTR_OBS_SOLARAZIM: n = Util::RadiansToDegrees(observer_->GetLookAngle(sun_->FindPosition(loc.GetDate())).azimuth); break;
				default:
					Fail("unhandled floating point attribute id: %d\n", attr);
					break;
			}
			Note("\t%s: %.*lf\n", attribute_options[attr].name, attribute_options[attr].decimals, n);
			
			if (NULL != csv_) {
				/* print csv value using same precision specified in dbf */
				fprintf(csv_, ",%.*lf", attribute_options[attr].decimals, n);
			} else {
				DBFWriteDoubleAttribute(dbf_, index, attribute_field[attr], n);
			}
			
		} else {
			Fail("unhandled attribute type: %d\n", attribute_options[attr].type);
		}
	}
	
	/* terminate the csv record */
	if (NULL != csv_) {
		fprintf(csv_, "\n");
	}
}
