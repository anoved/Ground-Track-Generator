#ifndef _GTGTLE_H_
#define _GTGTLE_H_

#include <iostream>

#include "Tle.h"

Tle ReadTleFromStream(std::istream& stream);
Tle ReadTleFromPath(const char* infile);
Tle ReadTleFromBuffer(const char *buffer);

#endif
