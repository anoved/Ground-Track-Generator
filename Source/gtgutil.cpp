#include "gtgutil.h"

#include <stdio.h>
#include <stdlib.h>

/* Print an error message to stderr and exit with failure status. */
void FailDetail(const char *file, int line, const char *errorString, ...) {
	va_list arglist;
	va_start(arglist, errorString);
	fprintf(stderr, "%s:%s:%d: ", _GTG_PROGRAM_, file, line);
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
	printf("usage: %s [--start] [--end] [--unit] [--interval] [--tle]\n", _GTG_PROGRAM_);
	printf("       [--steps] [--input] [--output] [--format] [output]\n");
	printf("Web page: <https://github.com/anoved/Ground-Track-Generator>\n");
	exit(EXIT_SUCCESS);
}

