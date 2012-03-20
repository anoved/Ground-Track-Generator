#include "gtgutil.h"

#include <stdio.h>
#include <stdlib.h>

/* Print an error message to stderr and exit with failure status. */
void Fail(const char *errorString, ...) {
	va_list arglist;
	va_start(arglist, errorString);
	fprintf(stderr, "ERROR:\n");
	vfprintf(stderr, errorString, arglist);
	va_end(arglist);
	exit(EXIT_FAILURE);
}

/* Print a diagnostic message. We could rewrite these to a log file. */
void Note(const char *noteString, ...) {
	va_list arglist;
	va_start(arglist, noteString);
	vprintf(noteString, arglist);
	va_end(arglist);
}

