
/*
 *  stringx.h
 *
 *	miscellaneous string functions
 *
 *  Copyright (c) 1994 Ithran Einhorn.  All rights reserved.
 *
 */

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

int stricmp(const char *, const char *);
char *TrimRight(char *t_str);

#ifdef __cplusplus
}
#endif
