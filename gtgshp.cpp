#include <stdio.h>
#include "gtgshp.h"

void OutputPoint(int feature, Eci eci)
{
	CoordGeodetic gc(eci.ToGeodetic());
	printf("%3d - %s\n", feature, gc.ToString().c_str());
}
