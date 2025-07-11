/*
*
* fastsort - sort a file in place - fast!
*
* Written 03/01/89 by Edwin R. Carp
*
* Copyright 1989 by Edwin R. Carp
*
*
* John F. Haugh II, modified 3/3/89
*
* Completely rehacked to remove the two pass garbage
* and quit pushing strings about in memory.  Reads
* entire file in with one call, splits into lines and
* saves pointers to each.  Then sorts pointers and
* outputs lines.
*
* No big deal ...
*
*
* Terence M. Donahue, modified 3/4/89
*
* Uses fputs() instead of fprintf() to output the sorted lines
* Inlined the string compare into the compare() function.
*
* It is now about as fast as sort on my machine...
*
* There is a slow homemade quicksort routine #ifdef'ed out.
* Once it is fast enough, compile -DHOMEMADE to have it replace qsort
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

nomem (s)
char	*s;
{
	fprintf (stderr, "Can't get memory for %s\n", s);
	exit (1);
}

#ifdef HOMEMADE
/*
** This homemade quicksort is currently much slower than qsort,
** especially for large arrays.
**
** Future improvements:
**
**    inline the strcmps
**    do the recursive sort call on the smaller partition
**    switch to an insertion sort on partitions smaller than 8 elements
*/

#define exch(x,y) (temp = x, x = y, y = temp)

void sort(v,left,right)
     char *v[];
     int left,right;
{
  int i, last;
  char *temp;

  while (left < right) {
    /* Determine pivot by taking the median of left, middle, and right */
    i = (left+right)>>1;
    if (strcmp(v[left],v[right]) > 0) {
      if (strcmp(v[left],v[i]) < 0)       i = left;
      else if (strcmp(v[right],v[i]) > 0) i = right;
    }
    else {
      if (strcmp(v[left],v[i]) > 0)       i = left;
      else if (strcmp(v[right],v[i]) < 0) i = right;
    }

    exch(v[left],v[i]);

    last = left;
    for (i=left+1; i <= right; i++)
      if (strcmp(v[i],v[left]) < 0)
	if (i != ++last) { exch(v[last],v[i]); }

    exch(v[left],v[last]);

    if (left < last-1) sort(v, left, last-1);
    left = last+1;
  }
}

#else

compare(sp1,sp2)
     char **sp1,**sp2;
{
  char *s1,*s2;

  s1 = *sp1; s2 = *sp2;
  while(*s1 == *s2++)
    if(*s1++ == '\0') return 0;
  return(*s1 - *--s2);
}
#endif

main(argc, argv)
int argc;
char **argv;
{
	int	fd;
	char	*malloc ();
	char	*realloc ();
	char	*cp;
	char	*buf;
	char	**lines;
	int	cnt, cur, max;
	struct	stat	statbuf;
	FILE	*fp;

	if (argc < 2) {
		fprintf (stderr, "usage: fastsort files ...\n");
		exit (1);
	}
	while (*++argv) {
		if (stat (*argv, &statbuf)) {
			perror(*argv);
			continue;
		}
		if (! (buf = malloc ((unsigned) statbuf.st_size + 1)))
			nomem (*argv);

		if ((fd = open (*argv, O_RDONLY)) < 0) {
			perror (*argv);
			continue;
		}
		if (read (fd, buf, statbuf.st_size) != statbuf.st_size) {
			perror (*argv);
			free (buf);
			continue;
		}
		close (fd);

		*(cp = &buf[statbuf.st_size]) = '\0';

		cur = 0;
		max = 10;

		if (! (lines = (char **) malloc (sizeof (char *) * max)))
			nomem (*argv);

		while (--cp != buf) {
			if (*cp == '\n') {
				*cp = '\0';
				if (cur == max)
					if (! (lines = (char **) realloc (lines, sizeof (char *) * (max += 10))))
						nomem (*argv);
				lines[cur++] = cp + 1;
			}
		}
		lines[0] = buf;		/* fix our earlier mistake :-) */

#ifdef HOMEMADE
		sort (lines, 0, cur-1);
#else
		qsort ((char *) lines, cur, sizeof (char *), compare);
#endif

		if (! (fp = fopen (*argv, "w"))) {
			perror (*argv);
			continue;
		}
		for (max = cur, cur = 0;cur < max;cur++) {
			fputs (lines[cur], fp);
			putc ('\n', fp);
		}

		fflush (fp);
		fclose (fp);
		free (lines);
		free (buf);
	}
	exit (0);
}
