#ifndef __UTILS_H
#define __UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

void DEBUG(char *format, ... );

void raiseWarning(int lineno, char *format,...);

void raiseError(int lineno, char *format, ...);




#endif