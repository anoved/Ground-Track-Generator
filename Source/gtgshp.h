#ifndef _GTGSHP_H_
#define _GTGSHP_H_

#include "Eci.h"
#include "gtg.h"
#include "shapefil.h"

class ShapefileWriter
{
public:
	ShapefileWriter(const char *basepath, enum output_format_type format);
	
	~ShapefileWriter()
	{
	}
	
	int output(Eci *loc, Eci *nextloc = NULL);
		
	void close(void);

private:
	
	int shpFormat_;
	SHPHandle shp_;
	DBFHandle dbf_;
	
};

#endif
