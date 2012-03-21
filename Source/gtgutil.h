#ifndef _GTGUTIL_H_
#define _GTGUTIL_H_

#include "gtg.h"
#include <stdarg.h>

void Fail(const char *errorString, ...);
void Note(const char *noteString, ...);
void ShowVersion(void);
void ShowHelp(void);

#endif
