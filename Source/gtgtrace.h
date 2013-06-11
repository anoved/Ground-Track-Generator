/*
 * gtgtrace
 *
 * Performs the core loop of the program: propagating satellite position
 * forward for the specified number of steps from the specified start time.
 */

#ifndef _GTGTRACE_H_
#define _GTGTRACE_H_

void InitGroundTrace(Tle& tle, DateTime& now, const GTGConfiguration& cfg, const TimeSpan& interval);

#endif
