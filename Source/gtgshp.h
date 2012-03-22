#ifndef _GTGSHP_H_
#define _GTGSHP_H_

#include "Eci.h"
#include "CoordGeodetic.h"
#include "gtg.h"
#include "shapefil.h"

class ShapefileWriter
{
public:
	ShapefileWriter(const char *basepath, enum output_feature_type features);
	
	~ShapefileWriter()
	{
	}
	
	int output(Eci *loc, Eci *nextloc = NULL);
		
	void close(void);

private:
	
	void outputAttributes(int index, Eci *loc, CoordGeodetic *geo);
	
	int shpFormat_;
	SHPHandle shp_;
	DBFHandle dbf_;
	
};

#endif
