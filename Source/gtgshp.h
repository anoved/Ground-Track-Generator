#ifndef _GTGSHP_H_
#define _GTGSHP_H_

#include "Eci.h"
#include "CoordGeodetic.h"
#include "gtg.h"
#include "shapefil.h"

/* used to parse attributes specified on command line and as dbf field titles */
enum attribute_ids {
	ATTR_ALTITUDE = 0,
	ATTR_VELOCITY,
	ATTR_COUNT
};

extern const char *attribute_names[];
extern int attribute_flags[];
extern int attribute_field[];

int IsValidAttribute(const char *s);

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

	void initAttributes(void);
	void outputAttributes(int index, Eci *loc, CoordGeodetic *geo);
	
	int shpFormat_;
	SHPHandle shp_;
	DBFHandle dbf_;
	
};

#endif
