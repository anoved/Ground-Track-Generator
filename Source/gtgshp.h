/*
 * gtgshp
 *
 * Manage output to shapefile.
 */

#ifndef _GTGSHP_H_
#define _GTGSHP_H_

#include "Eci.h"
#include "CoordGeodetic.h"
#include "Observer.h"

#include "shapefil.h"

class ShapefileWriter
{
public:

	ShapefileWriter(const char *basepath, enum output_feature_type features, bool create_prj);
	
	~ShapefileWriter() {}
	
	int output(const Eci& loc, const CoordGeodetic& geo, Eci *nextloc = NULL, bool split = false, bool rawOutput = false);
		
	void close(void);

private:
	
	void CreateWGS72prj(const char *basepath);
	SHPObject* splitSegment(double lata, double lona, double latb, double lonb, const Eci& loc);
	
	int shpFormat_;
	SHPHandle shp_;
};

#endif
