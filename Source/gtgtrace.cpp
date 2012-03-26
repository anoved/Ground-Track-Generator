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

#include "gtgtrace.h"

/*
	InitTime
	
	Parameters:
		desc, string to read time specification from. Accepts four formats:
				now - current time
				epoch - reference time of orbit info
				YYYY-MM-DD HH:MM:SS.SSSSSS UTC
				S - UNIX time (seconds since 1970-01-01 00:00:00)
		now, reference date to use if "now" time is specified
		tle, orbital elements to use if orbit "epoch" is specified
	
	Returns:
		A Julian date object initialized to time requested by desc
		Aborts if desc cannot be parsed.
*/
Julian InitTime(const char *desc, Julian now, Tle tle)
{
	Julian time;
	double offset;
	char unit;
	
	if (0 == strcmp("now", desc)) {
		time = now;
	} else if (2 == sscanf(desc, "now%lf%c", &offset, &unit)) {
		Timespan offset_timespan(0);
		switch (unit) {
			case 's': offset_timespan.AddSeconds(offset); break;
			case 'm': offset_timespan.AddSeconds(offset * 60.0); break;
			case 'h': offset_timespan.AddSeconds(offset * 60.0 * 60.0); break;
			case 'd': offset_timespan.AddSeconds(offset * 60.0 * 60.0 * 24.0); break;
			default:
				Fail("invalid current time offset unit: %c\n", unit);
				break;
		}
		time = now + offset_timespan;
	} else if (0 == strcmp("epoch", desc)) {
		time = tle.Epoch();
	} else if (2 == sscanf(desc, "epoch%lf%c", &offset, &unit)) {
		Timespan offset_timespan(0);
		switch (unit) {
			case 's': offset_timespan.AddSeconds(offset); break;
			case 'm': offset_timespan.AddSeconds(offset * 60.0); break;
			case 'h': offset_timespan.AddSeconds(offset * 60.0 * 60.0); break;
			case 'd': offset_timespan.AddSeconds(offset * 60.0 * 60.0 * 24.0); break;
			default:
				Fail("invalid epoch time offset unit: %c\n", unit);
				break;
		}
		time = tle.Epoch() + offset_timespan;
	} else {
		int year, month, day, hour, minute;
		double second;
		if (6 == sscanf(desc, "%4d-%2d-%2d %2d:%2d:%9lf UTC", &year, &month, &day, &hour, &minute, &second)) {
			time = Julian(year, month, day, hour, minute, second);
		} else {
			double unixtime;
			if (1 == sscanf(desc, "%lf", &unixtime)) {
				time = Julian((time_t)unixtime);
			} else {
				Fail("cannot parse time: %s\n", desc);
			}
		}
	}
		
	return time;
}

/*
	InitInterval
	
	Parameters:
		units, type of unit used to specify interval length
		interval_length, length of each interval in given units
	
	Return:
		A Timespan object initialized to the specified interval
*/
Timespan InitInterval(enum interval_unit_type units, double interval_length)
{
	Timespan interval;
	
	switch (units) {
		case seconds:
			interval.AddSeconds(interval_length);
			break;
		case minutes:
			interval.AddSeconds(interval_length * 60.0);
			break;
		case hours:
			interval.AddSeconds(interval_length * 60.0 * 60.0);
			break;
		case days:
			interval.AddSeconds(interval_length * 60 * 60 * 24);
			break;
	}
	
	return interval;
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
void GenerateGroundTrack(Tle& tle, SGP4& model, Julian& now, const GTGConfiguration& cfg)
{
	int step = 0;
	Julian time, endtime;
	Timespan interval;
	Eci eci(now, 0, 0, 0);
	
	/* for line output mode */
	Eci prevEci(eci);
	int prevSet = 0;
	
	/* Initialize the step interval */
	interval = InitInterval(cfg.unit, cfg.interval);
	Note("Step interval: %lf seconds\n", interval.GetTotalSeconds());
	
	/* Initialize the starting timestamp; default to epoch */
	time = InitTime(cfg.start == NULL ? "epoch" : cfg.start, now, tle);
	Note("Start time: %s\n", time.ToString().c_str());

	/* Initialize the ending timestamp, if needed */
	if (NULL != cfg.end) {
		
		endtime = InitTime(cfg.end, now, tle);
		
		/* Sanity check 1 */
		if (time >= endtime) {
			Fail("end time (%s) not after start time (%s)\n", endtime.ToString().c_str(), time.ToString().c_str());
		}
		
		/* Sanity check 2 */
		if (interval > endtime - time) {
			Fail("interval (%lf minutes) exceeds period between start time and end time (%lf minutes).\n", interval.GetTotalMinutes(), (endtime - time).GetTotalMinutes());
		}
			
		Note("End time: %s\n", endtime.ToString().c_str());
	}
	
	std::ostringstream ns;
	ns << tle.NoradNumber();
	std::string basepath(BuildBasepath(ns.str(), cfg));
	Note("Output basepath: %s\n", basepath.c_str());
	ShapefileWriter shout(basepath.c_str(), cfg.features, cfg.prj);
		
	while (1) {
		
		/* where is the satellite now? */
		try {
			eci = model.FindPosition(time);
		} catch (SatelliteException &e) {
			Note("satellite exception (stopping at step %d): %s\n", step, e.what());
			break;
		} catch (DecayedException &e) {
			Note("satellite decayed (stopping at step %d).\n", step);
			break;
		}
		
		if (line == cfg.features) {
			if (prevSet) {
				shout.output(&prevEci, &eci, cfg.split);
				step++;
			} else {
				/* prevSet is only false on the first pass, which yields an
				   extra interval not counted against step count - needed
				   since line segments imply n+1 intervals for n steps */
				prevSet = 1;
			}
			
			prevEci = eci;
			
		} else {
		
			shout.output(&eci);
			
			step++;
		}
		
		/* increment time interval */
		time += interval;
		
		/* stop ground track once we've exceeded step count or end time */
		if ((0 != cfg.steps) && (step >= cfg.steps)) {
			break;
		} else if ((NULL != cfg.end) && (time > endtime)) {
			break;
		}
		
	}
	
	shout.close();
}

/*
 * Create an SGP4 model for the specified satellite two-line element set
 * and start generating its ground track.
 */
void InitGroundTrace(Tle& tle, Julian& now, const GTGConfiguration &cfg)
{
	try {
		SGP4 model(tle);
		GenerateGroundTrack(tle, model, now, cfg);
	} catch (SatelliteException &e) {
		Fail("cannot initialize satellite model: %s\n", e.what());
	}
}
