#ifndef _GTG_H_
#define _GTG_H_

enum interval_unit_type {
	seconds,
	minutes,
	hours,
	days
};

enum output_format_type {
	point,
	line
};

struct configuration {
	char *start;
	char *end;
	enum interval_unit_type interval_units;
	double interval_length;
	int feature_count;
	char *tleText;
	char *inputTlePath;
	char *outputShpBasepath;
	enum output_format_type format;
};

extern struct configuration cfg;

#endif
