#ifndef __UTILS_H
#define __UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

void DEBUG(char *format, ... );

void warnMessage(char *format, ...);

void errMessage(char *format, ...);

#endif