/*
 * gtgutil
 *
 * Provides utility functions for displaying error, status, and help messages.
 */

#ifndef _GTGUTIL_H_
#define _GTGUTIL_H_

void SetVerbosity(bool verbose = true);
void FailDetail(const char *file, int line, const char *errorString, ...);
void Note(const char *noteString, ...);
void ShowVersion(void);
void ShowHelp(void);

#define Fail(format, ...) FailDetail(__FILE__, __LINE__, (format), ## __VA_ARGS__)

#endif
