#ifndef _GTG_H_
#define _GTG_H_

#define _GTG_NAME_ "Ground Track Generator"
#define _GTG_PROGRAM_ "gtg"
#define _GTG_VERSION_ "0.1"

enum interval_unit_type {
	seconds,
	minutes,
	hours,
	days
};

enum output_feature_type {
	point,
	line
};

struct configuration {
	char *start;
	char *end;
	enum interval_unit_type unit;
	double interval;
	int steps;
	char *basepath;
	enum output_feature_type features;
	int verbose;
	int split;
	double obslat;
	double obslon;
	double obsalt;
	char *prefix;
	char *suffix;
	int prj;
};

extern struct configuration cfg;

#endif
