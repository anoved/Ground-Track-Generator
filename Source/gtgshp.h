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

	ShapefileWriter(const char *basepath, enum output_feature_type features,
			double latitude, double longitude, double altitude, bool create_prj);
	
	~ShapefileWriter()
	{
	}
	
	int output(Eci *loc, Eci *nextloc = NULL, bool split = false);
		
	void close(void);

private:
	
	void CreateWGS72prj(const char *basepath);
	SHPObject* splitSegment(double lata, double lona, double latb, double lonb, Eci& loc);
	
	int shpFormat_;
	SHPHandle shp_;
	DBFHandle dbf_;
	Observer *obs_;
};

#endif
