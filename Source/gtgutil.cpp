#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "gtg.h"

#include "gtgutil.h"

namespace gtgutil {
	bool verbose = false;
}

/* Print an error message to stderr and exit with failure status. */
void FailDetail(const char *file, int line, const char *errorString, ...) {
	va_list arglist;
	va_start(arglist, errorString);
	fprintf(stderr, "%s:%s:%d: ", _GTG_PROGRAM_, file, line);
	vfprintf(stderr, errorString, arglist);
	va_end(arglist);
	exit(EXIT_FAILURE);
}

void SetVerbosity(bool verbose)
{
	gtgutil::verbose = verbose;
}

/* Print a diagnostic message. We could rewrite these to a log file. */
void Note(const char *noteString, ...) {
	va_list arglist;
	if (gtgutil::verbose) {
		va_start(arglist, noteString);
		vprintf(noteString, arglist);
		va_end(arglist);
	}
}

void ShowVersion(void)
{
	printf("%s %s\n", _GTG_NAME_, _GTG_VERSION_);
	exit(EXIT_SUCCESS);
}

void ShowHelp(void)
{
	printf("%s %s\n", _GTG_NAME_, _GTG_VERSION_);
	printf("usage: %s [OPTIONS] [TLE [TLE ...]]\n", _GTG_PROGRAM_);
	printf("Options:\n");
	printf("\t--unit/-u days | hours | minutes | seconds\n");
	printf("\t\tDefault: minutes\n");
	printf("\t--interval/-l N\n");
	printf("\t\tDefault: 1.0\n");
	printf("\t--steps/-n N\n");
	printf("\t\tDefault: 100\n");
	printf("\t--start/-s now | epoch | YYYY-MM-DD HH:MM:SS.SSSSSS UTC | S\n");
	printf("\t\tDefault: epoch (TLE reference date)\n");
	printf("\t--end/-e now | epoch | YYYY-MM-DD HH:MM:SS.SSSSSS UTC | S\n");
	printf("\t\tIf specified, overrides --steps.\n");
	printf("\t--features/-f point | line\n");
	printf("\t\tDefault: point\n");
	printf("\t--split/-d\n");
	printf("\t\tIf generating line features, split segments that cross 180th meridian.\n");
	printf("\t--observer/-g LATITUDE LONGITUDE [ALTITUDE]\n");
	printf("\t\tLocation of observer used to compute some attributes. Altitude in km.\n");
	printf("\t--attributes/-a all | standard | [ATTRIBUTE [ATTRIBUTE...]]\n");
	printf("\t\tWhich attributes to output. standard outputs all except those requiring observer.\n");
	printf("\t--tle/-t TEXT | --input/-i PATH\n");
	printf("\t\tTLE read from TEXT value or PATH file respectively.\n");
	printf("\t--output/-o DIRECTORY\n");
	printf("\t\tIf not specified, output is written to current working directory.\n");
	printf("\t--verbose\n");
	printf("\t\tPrint diagnostic status messages.\n");
	printf("\t--help/-?\n");
	printf("\t\tShow this help text.\n");
	printf("Web page: <https://github.com/anoved/Ground-Track-Generator>\n");
	exit(EXIT_SUCCESS);
}

