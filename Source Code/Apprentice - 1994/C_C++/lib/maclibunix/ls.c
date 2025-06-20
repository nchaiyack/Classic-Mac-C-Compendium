/* An ls-like program for use as an MPW tool.
   Main improvement over the MPW built-in command Files is columnar
   output.  Main disadvantage is the absence of a "-l" flag.
   Public domain by Guido van Rossum, CWI, Amsterdam (July 1987).
*/

#include <stdio.h>
#include "dir.h"
#include "stat.h"

#include "macdefs.h"

char *strcpy();

int
main(argc, argv)
	int argc;
	char **argv;
{
	setvbuf(stdout, (char*) NULL, _IOLBF, BUFSIZ);
	if (argc > 1)
		lslist(argc-1, argv+1);
	else
		lsdir(":");
	return 0;
}

struct item {
	char *name;
	long flags;
};

#define ISDIR 1		/* flag set for directory */

int
compare(a, b)
	struct item *a, *b;
{
	return IUMagString(a->name, b->name,
		strlen(a->name), strlen(b->name));
}

outsortedlist(listc, listv)
	int listc;
	struct item *listv;
{
	qsort(listv, listc, sizeof(struct item), compare);
	outlist(listc, listv, 0, 0);
}

outlist(listc, listv, colwidth, screenwidth)
	int listc;
	struct item *listv;
	int colwidth;
	int screenwidth;
{
	int i, j, ncols, nrows;
	char *p;
	
	if (colwidth <= 1) colwidth= 16;
	if (screenwidth <= 1) screenwidth= 72;
	ncols= screenwidth/colwidth;
	nrows= (listc+ncols-1) / ncols;
	
	for (i= 0; i < nrows; ++i) {
		for (j= i; j < listc; j += nrows) {
			if (listv[j].flags & ISDIR) {
				char buf[40];
				sprintf(buf, ":%s:", listv[j].name);
				p= buf;
			}
			else
				p= listv[j].name;
			if (j+nrows < listc)
				printf("%-*.*s ", colwidth, colwidth, p);
			else
				printf("%s\n", p);
		}
	}
}

lslist(argc, argv)
	int argc;
	char **argv;
{
	struct item *files= NULL;
	struct item *dirs= NULL;
	int nfiles= 0;
	int ndirs= 0;
	struct stat buf;
	int i;
	
	for (i= 0; i < argc; ++i) {
		if (isdir(argv[i]))
			addtolist(argv[i], &dirs, &ndirs, ISDIR);
		else if (stat(argv[i], &buf) >= 0)
			addtolist(argv[i], &files, &nfiles, 0);
		else
			fprintf(stderr, "Can't stat %s\n", argv[i]);
	}
	outsortedlist(nfiles, files);
	qsort(dirs, ndirs, sizeof(struct item), compare);
	for (i= 0; i < ndirs; ++i) {
		printf("\n%s\n", dirs[i]);
		lsdir(dirs[i]);
	}
}

/* "Intelligent" filename concatenation.  Buf becomes path:name, but:
   insert a SEP only if path doesn't end in SEP and name doesn't
   begin with SEP; remove a SEP if path ends in SEP and name begins
   with SEP. Returns buf. */

char *
mkpath(buf, path, name)
	char *buf;
	char *path;
	char *name;
{
	char *p;
	
	strcpy(buf, path);
	p= buf + strlen(buf);
	if (p > buf) {
		if (p[-1] != SEP) {
			if (name[0] != SEP)
				*p++= SEP;
		}
		else if (name[0] == SEP)
			++name;
	}
	strcpy(p, name);
	return buf;
}

lsdir(dir)
	char *dir;
{
	DIR *dirp= opendir(dir);
	struct direct *dp;
	struct item *listv= NULL;
	int listc= 0;
	char buf[256];
	
	if (dirp == NULL) {
		fprintf("can't open directory %s\n", dir);
		return -1;
	}
	while ((dp= readdir(dirp)) != NULL)
		addtolist(dp->d_name, &listv, &listc,
			isdir(mkpath(buf, dir, dp->d_name)) ? ISDIR : 0);
	closedir(dirp);
	if (listc > 0) {
		outsortedlist(listc, listv);
		while (--listc >= 0)
			freemem(listv[listc]);
		freemem((char*) listv);
	}
}

char *malloc();
char **realloc();

char *
getmem(n)
	int n;
{
	char *p= malloc((unsigned) n);
	
	if (p == 0)
		memexh();
	return p;
}

regetmem(ps, n)
	char **ps;
	int n;
{
	if (*ps == NULL)
		*ps= getmem(n);
	else {
		*ps= realloc(*ps, (unsigned) n);
		if (*ps == NULL)
			memexh();
	}
}

freemem(p)
	char *p;
{
	free(p);
}

memexh()
{
	fprintf(stderr, "out of memory\n");
	exit(3);
}

char *
strdup(s)
	char *s;
{
	char *p= getmem(strlen(s)+1);
	
	return strcpy(p, s);
}

addtolist(s, plistv, plistc, flags)
	char *s;
	struct item **plistv;
	int *plistc;
	int flags;
{
	regetmem(& (char *) *plistv, (*plistc+1) * sizeof(**plistv));

	(*plistv)[*plistc].name= strdup(s);
	(*plistv)[*plistc].flags= flags;
	++*plistc;
}

isdir(path)
	char *path;
{
	struct stat buf;
	
	return stat(path, &buf) == 0 && (buf.st_mode&S_IFMT) == S_IFDIR;
}
