/*
 * gtgtrace
 *
 * Performs the core loop of the program: propagating satellite position
 * forward for the specified number of steps from the specified start time.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SGP4.h"
#include "Julian.h"
#include "Timespan.h"
#include "Tle.h"

#include "gtg.h"
#include "gtgutil.h"
#include "gtgtle.h"
#include "gtgshp.h"
#include "gtgattr.h"
#include "gtgtrace.h"

/*
 * Given an offset value and a unit, calculate the offset in minutes.
 */
double OffsetInMinutes(double offset, char unit)
{
	double offsetMinutes;
	switch (unit) {
		case 's':
			offsetMinutes = offset / 60.0;
			break;
		case 'm':
			offsetMinutes = offset;
			break;
		case 'h':
			offsetMinutes = offset * 60.0;
			break;
		case 'd':
			offsetMinutes = offset * 1440.0;
			break;
		default:
			Fail("invalid time offset unit: %c\n", unit);
			break;
	}
	return offsetMinutes;
}

/*
	InitTime
	
	Parameters:
		desc, string to read time specification from. Accepts four formats:
				now[OFFSET] - current time
				epoch[OFFSET] - reference time of orbit info
					If specified, OFFSET is number (positive or negative)
					followed by a character indicating units - s seconds,
					m minutes, h hours, d days.
				YYYY-MM-DD HH:MM:SS.SSSSSS UTC
				S - UNIX time (seconds since 1970-01-01 00:00:00)
		now, reference date to use if "now" time is specified
		epoch, reference date to use if orbit "epoch" is specified
	
	Returns:
		MFE (minutes from TLE epoch) of the described time
*/
double InitTime(const char *desc, const Julian& now, const Julian& epoch)
{
	double mfe = 0;
	double offset;
	char unit;
	
	if (0 == strcmp("now", desc)) {
		mfe = (now - epoch).GetTotalMinutes();
	} else if (2 == sscanf(desc, "now%32lf%c", &offset, &unit)) {
		mfe = (now - epoch).GetTotalMinutes() + OffsetInMinutes(offset, unit);
	} else if (0 == strcmp("epoch", desc)) {
		mfe = 0.0;
	} else if (2 == sscanf(desc, "epoch%32lf%c", &offset, &unit)) {
		mfe = OffsetInMinutes(offset, unit);
	} else {
		int year, month, day, hour, minute;
		double second;
		if (6 == sscanf(desc, "%4d-%2d-%2d %2d:%2d:%9lf UTC", &year, &month, &day, &hour, &minute, &second)) {
			Julian time(year, month, day, hour, minute, second);
			mfe = (time - epoch).GetTotalMinutes();
		} else {
			double unixtime;
			if (3 == sscanf(desc, "%32lf%32lf%c", &unixtime, &offset, &unit)) {
				Julian time((time_t)unixtime);
				mfe = (time - epoch).GetTotalMinutes() + OffsetInMinutes(offset, unit);
			} else if (1 == sscanf(desc, "%32lf", &unixtime)) {
				Julian time((time_t)unixtime);
				mfe = (time - epoch).GetTotalMinutes();
			} else {
				Fail("cannot parse time: %s\n", desc);
			}
		}
	}
		
	return mfe;
}

/*
 * Construct the output filename, minus shapefile file extensions.
 * The general format is: <basepath>/<prefix>rootname<suffix>
 *
 * If this is the only output file and basepath is specified,
 * the format is simply basepath and other elements are ignored.
 */
std::string BuildBasepath(const std::string& rootname, const GTGConfiguration& cfg)
{
	std::string shpbase;
	
	/* if only one TLE was specified, we use --output as the unmodified
	   basepath, and ignore --prefix and --suffix, and do not insert any ID.
	   This only applies if --output is defined! Otherwise, use id/prefix/suffix. */
	if (cfg.single && (cfg.basepath != NULL)) {
		shpbase += cfg.basepath;
		return shpbase;
	}
	
	if (NULL != cfg.basepath) {
		shpbase += cfg.basepath;
		if ('/' != shpbase[shpbase.length() - 1]) {
			shpbase += '/';
		}
	}
	
	if (NULL != cfg.prefix) {
		shpbase += cfg.prefix;
	}
	
	shpbase += rootname;
	
	if (NULL != cfg.suffix) {
		shpbase += cfg.suffix;
	}
	
	return shpbase;
}

/*
 * Do some initialization that may be specific to this track (output start time,
 * name, etc.) and do the main orbit propagation loop, outputting at each step.
 */
void GenerateGroundTrack(Tle& tle, SGP4& model, Julian& now,
		const GTGConfiguration& cfg, const Timespan& interval)
{
	int step = 0;
	Eci eci(now, 0, 0, 0);
	bool stop = false;
	CoordGeodetic geo;
	
	double minutes;
	double startMFE;
	double endMFE;
	double intervalMinutes;
	
	ShapefileWriter *shpwriter = NULL;
	AttributeWriter *attrwriter = NULL;
	int shpindex = 0;
	
	intervalMinutes = interval.GetTotalMinutes();
	
	/* for line output mode */
	Eci prevEci(eci);
	int prevSet = 0;
	
	/* Initialize the starting timestamp; default to epoch */
	startMFE = InitTime(cfg.start == NULL ? "epoch" : cfg.start, now, tle.Epoch());
	minutes = startMFE;
		
	Note("TLE epoch: %s\n", tle.Epoch().ToString().c_str());	
	Note("Start MFE: %.9lf\n", startMFE);
	
	/* Initialize the ending timestamp, if needed */
	if (NULL != cfg.end) {
		
		endMFE = InitTime(cfg.end, now, tle.Epoch());
		
		/* Sanity check 1 */
		if (startMFE >= endMFE) {
			Fail("end time (%.9lf MFE) not after start time (%.9lf MFE)\n", endMFE, startMFE);
		}
		
		/* Sanity check 2 */
		if (intervalMinutes > endMFE - startMFE) {
			Fail("interval (%.9lf minutes) exceeds period between start time and end time (%.9lf minutes).\n", intervalMinutes, endMFE - startMFE);
		}
				
		Note("End MFE: %.9lf\n", endMFE);
	}
	

	std::ostringstream ns;
	ns << tle.NoradNumber();
	std::string basepath(BuildBasepath(ns.str(), cfg));
	if (!(cfg.csvMode && (cfg.basepath == NULL))) {
		Note("Output basepath: %s\n", basepath.c_str());
	}
	if (!cfg.csvMode) {
		shpwriter = new ShapefileWriter(basepath.c_str(), cfg.features, cfg.prj);
	}
	attrwriter = new AttributeWriter(
				cfg.csvMode && (cfg.basepath == NULL) ? NULL : basepath.c_str(),
				cfg.has_observer, cfg.obslat, cfg.obslon, cfg.obsalt,
				cfg.csvMode, cfg.csvHeader);
	
	while (1) {
		
		/* where is the satellite now? */
		try {
			eci = model.FindPosition(minutes);
		} catch (SatelliteException &e) {
			Warn("satellite exception (stopping at step %d): %s\n", step, e.what());
			break;
		} catch (DecayedException &e) {
			Warn("satellite decayed (stopping at step %d).\n", step);
			break;
		}
		
		if (line == cfg.features) {
			if (prevSet) {
				geo = prevEci.ToGeodetic();
				if (cfg.csvMode) {
					// increment shpindex ourselves in csvMode
					attrwriter->output(shpindex++, minutes, prevEci, geo);
				} else {
					shpindex = shpwriter->output(geo, eci.ToGeodetic(), cfg.split, prevEci);
					attrwriter->output(shpindex, minutes, prevEci, geo);
				}
				step++;
			} else {
				/* prevSet is only false on the first pass, which yields an
				   extra interval not counted against step count - needed
				   since line segments imply n+1 intervals for n steps */
				prevSet = 1;
			}
			
			prevEci = eci;
			
		} else {
			geo = eci.ToGeodetic();
			if (cfg.csvMode) {
				attrwriter->output(shpindex++, minutes, eci, geo);
			} else {
				shpindex = shpwriter->output(geo);
				attrwriter->output(shpindex, minutes, eci, geo);
			}
			step++;
		}
		
		/* increment time interval */
		minutes += intervalMinutes;
		
		/* stop ground track once we've exceeded step count or end time */
		if ((0 != cfg.steps) && (step >= cfg.steps)) {
			break;
		} else if ((NULL != cfg.end) && (minutes >= endMFE) ) {
			if (!cfg.forceend or stop) {
				break;
			} else {
				/* force output of the exact end time, then stop next time */
				minutes = endMFE;
				stop = true;
			}
		}
		
	}
	
	if (shpwriter != NULL) {
		shpwriter->close();
		delete shpwriter;
	}
	if (attrwriter != NULL) {
		attrwriter->close();
		delete attrwriter;
	}
}

/*
 * Create an SGP4 model for the specified satellite two-line element set
 * and start generating its ground track.
 */
void InitGroundTrace(Tle& tle, Julian& now, const GTGConfiguration &cfg,
		const Timespan& interval)
{
	try {
		SGP4 model(tle);
		GenerateGroundTrack(tle, model, now, cfg, interval);
	} catch (SatelliteException &e) {
		Fail("cannot initialize satellite model: %s\n", e.what());
	}
}
