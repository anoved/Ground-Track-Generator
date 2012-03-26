/*
 * gtgtle
 *
 * Parses two-line element sets of data used to compute satellite position and
 * velocity according to the SGP4 model. http://celestrak.com/columns/v04n03/
 */

#ifndef _GTGTLE_H_
#define _GTGTLE_H_

#include <iostream>
#include <queue>

#include "Tle.h"

bool ReadTlesFromStream(std::istream& stream, std::queue<Tle>& tles);
bool ReadTlesFromPath(const char *path, std::queue<Tle>& tles);
bool ReadTlesFromBuffer(const char *buffer, std::queue<Tle>& tles);

#endif
