/*
 * gtgtle
 *
 * Parses two-line element sets of data used to compute satellite position and
 * velocity according to the SGP4 model. http://celestrak.com/columns/v04n03/
 */

#ifndef _GTGTLE_H_
#define _GTGTLE_H_

#include <iostream>

#include "Tle.h"

Tle ReadTleFromStream(std::istream& stream);
Tle ReadTleFromPath(const char* path);
Tle ReadTleFromBuffer(const char *buffer);

#endif
