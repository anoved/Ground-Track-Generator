#include <stdio.h>
#include <string.h>
#include <math.h>

#include "gtgtrace.h"

#include "gtg.h"
#include "gtgutil.h"
#include "gtgtle.h"
#include "gtgshp.h"

#include "SGP4.h"
#include "Julian.h"
#include "Timespan.h"
#include "Tle.h"

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

	if (0 == strcmp("now", desc)) {
		time = now;
	} else if (0 == strcmp("epoch", desc)) {
		time = tle.Epoch();
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
				Fail("Could not scan time as timestamp or unix time\n");
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

void GenerateGroundTrack(Tle tle, SGP4 model)
{
	int feature = 0;
	Julian now;
	Julian time, endtime;
	Timespan interval;
	Eci eci(now, 0, 0, 0);
	
	/* for line output mode */
	Eci prevEci(eci);
	int prevSet = 0;
	
	/* Initialize the feature interval */
	interval = InitInterval(cfg.unit, cfg.interval);
	
	/* Initialize the starting timestamp; default to epoch */
	time = InitTime(cfg.start == NULL ? "epoch" : cfg.start, now, tle);
	Note("Start time: %s\n", time.ToString().c_str());

	/* Initialize the ending timestamp, if needed */
	if (NULL != cfg.end) {
		
		endtime = InitTime(cfg.end, now, tle);
		
		/* Sanity check 1 */
		if (time >= endtime) {
			Fail("End time must be after start time.\n");
		}
		
		/* Sanity check 2 */
		if (interval > endtime - time) {
			Fail("Trace interval exceeds time between start and end.\n");
		}
			
		Note("End time: %s\n", endtime.ToString().c_str());
	}
	
	ShapefileWriter shout(cfg.shpPath, cfg.format);
	
	while (1) {
		
		/* where is the satellite now? */
		try {
			eci = model.FindPosition(time);
		} catch (SatelliteException &e) {
			Fail("Satellite exception: %s\n", e.what());
		} catch (DecayedException &e) {
			/* A decaying orbit is OK - we just stop the trace now. */
			Note("Satellite decayed (interval %d).\n", feature);
			break;
		}
		
		if (line == cfg.format) {
			if (prevSet) {
				shout.output(&prevEci, &eci);
				feature++;
			} else {
				/* prevSet is only false on the first pass, which yields an
				   extra interval not counted against feature count - needed
				   since line segments imply n+1 intervals for n features */
				prevSet = 1;
			}
			
			prevEci = eci;
			
		} else {
		
			/* output this location */
			shout.output(&eci);
			
			/* increment feature */
			feature++;
		}
		
		/* increment time interval */
		time += interval;
		
		/* stop ground track once we've exceeded step count or end time */
		if ((0 != cfg.steps) && (feature > cfg.steps)) {
			break;
		} else if ((NULL != cfg.end) && (time >= endtime)) {
			break;
		}
		
	}
	
	shout.close();
	
}

void InitSatModel(Tle tle) {
	try {
		SGP4 model(tle);
		GenerateGroundTrack(tle, model);
	} catch (SatelliteException &e) {
		Fail("Could not initialize satellite model: %s\n", e.what());
	}
}

void StartGroundTrack(void)
{
	if ((NULL == cfg.tleText) and (NULL == cfg.tlePath)) {
		InitSatModel(ReadTleFromStream(&std::cin));
	} else if (NULL != cfg.tleText) {
		InitSatModel(ReadTleFromBuffer(cfg.tleText));
	} else if (NULL != cfg.tlePath) {
		InitSatModel(ReadTleFromPath(cfg.tlePath));
	}
}
