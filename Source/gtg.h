#ifndef _GTG_H_
#define _GTG_H_

#define _GTG_NAME_ "Ground Track Generator"
#define _GTG_PROGRAM_ "gtg"
#define _GTG_VERSION_ "1.0"

enum output_feature_type {
	point,
	line
};

typedef struct configuration {
	char *start;
	char *end;
	int forceend;
	char unit;
	double interval;
	int steps;
	char *basepath;
	enum output_feature_type features;
	int split;
	bool has_observer;
	double obslat;
	double obslon;
	double obsalt;
	char *prefix;
	char *suffix;
	int prj;
	bool single;
	bool csvMode;
	bool csvHeader;
} GTGConfiguration;

#endif
