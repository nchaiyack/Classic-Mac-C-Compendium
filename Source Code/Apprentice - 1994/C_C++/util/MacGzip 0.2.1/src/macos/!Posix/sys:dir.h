/* $Id: $ */

/*
 * "Dir.h" for the Macintosh.
 * Public domain by Guido van Rossum, CWI, Amsterdam (July 1987).
 */

#pragma once

#ifdef THINK_C
#include <stdio.h>
#endif /* THINK_C */

typedef struct {
	long dirid;
	int nextfile;
} DIR;

struct direct {
	char d_name[FILENAME_MAX];
};

DIR *opendir(char *);
struct dirent *readdir(DIR *);
int closedir(DIR *);
void rewinddir(DIR*);
