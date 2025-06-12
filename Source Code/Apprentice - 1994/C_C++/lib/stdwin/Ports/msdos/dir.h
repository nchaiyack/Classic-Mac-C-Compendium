/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1990. */

/*
 * Interface for MS-DOS version of UNIX directory access package.
 * (opendir, readdir, closedir).
 */

struct direct {
	char d_namlen;
	char d_name[13];
};

typedef char DIR;

extern DIR *opendir();
extern struct direct *readdir();
extern closedir();
