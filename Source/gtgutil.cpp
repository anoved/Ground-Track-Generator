#include "gtgutil.h"

#include <stdio.h>
#include <stdlib.h>

/* Print an error message to stderr and exit with failure status. */
void Fail(const char *errorString, ...) {
	va_list arglist;
	va_start(arglist, errorString);
	fprintf(stderr, "%s: ", _GTG_PROGRAM_);
	vfprintf(stderr, errorString, arglist);
	va_end(arglist);
	exit(EXIT_FAILURE);
}

/* Print a diagnostic message. We could rewrite these to a log file. */
void Note(const char *noteString, ...) {
	va_list arglist;
	if (cfg.verbose) {
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
	printf("usage: gtg [--start] [--end] [--interval_unit] [--interval_length] [--tle]\n");
	printf("           [--feature_count] [--input] [--output] [--format] [output]\n");
	printf("Web page: <https://github.com/anoved/Ground-Track-Generator>\n");
	exit(EXIT_SUCCESS);
}

