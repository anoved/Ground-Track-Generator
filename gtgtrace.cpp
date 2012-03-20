#include <stdio.h>
#include <string.h>
#include <math.h>

#include "gtg.h"
#include "gtgutil.h"
#include "gtgtle.h"
#include "gtgtrace.h"
#include "gtgshp.h"

#include "SGP4.h"
#include "Julian.h"
#include "Timespan.h"
#include "Tle.h"

Julian InitTime(const char *desc, Julian now, Tle tle)
{
	if (0 == strcmp("now", desc)) {
		return now;
	} else if (0 == strcmp("epoch", desc)) {
		return tle.Epoch();
	} else {
		/* here is an example of the time format output by Julian::ToString():
		 * 2012-03-19 21:30:13.819814 UTC
		 * That's fp precision 6; with 0 as fill character.
		 * %4d-%2d-%2d %2d:%2d:%9.6 UTC
		 * of course, perhaps UTC could be set to some other time zone
		 * Also, alternatively, just read a big number as seconds-since-epoch */
		 Fail("Arbitrary timestamp parsing is not yet implemented.\n");
	}
	
	// kludge
	return now;
}

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
	
	/* Initialize the feature interval */
	interval = InitInterval(cfg.interval_units, cfg.interval_length);
	
	/* Initialize the starting timestamp */
	time = InitTime(cfg.start, now, tle);
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
		
		/* output this location */
		OutputPoint(feature, eci);
		
		/* increment feature */
		feature++;
		
		/* increment time interval */
		time += interval;
		
		/* stop ground track once we've exceeded feature count or end time */
		if ((0 != cfg.feature_count) && (feature > cfg.feature_count)) {
			break;
		} else if ((NULL != cfg.end) && (time >= endtime)) {
			break;
		}
		
	}
		
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
	if ((NULL == cfg.tleText) and (NULL == cfg.inputTlePath)) {
		InitSatModel(ReadTleFromStream(&std::cin));
	} else if (NULL != cfg.tleText) {
		InitSatModel(ReadTleFromBuffer(cfg.tleText));
	} else if (NULL != cfg.inputTlePath) {
		InitSatModel(ReadTleFromPath(cfg.inputTlePath));
	}
}
