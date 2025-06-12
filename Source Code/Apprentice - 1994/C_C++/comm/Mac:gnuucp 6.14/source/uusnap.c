#ifndef lint
static char sccsid[] = "@(#)uusnap.c	1.5 87/09/25";
/* Derived from uusnap.c	5.7 (Berkeley) 10/9/85 */
#endif

/*
 *	Uusnap - displays a snapshot of the uucp system.
 *	originally by	RJKing WECo-MG6565 May 83
 *
 *	Obtained from Rick Adams by John Gilmore, Aug '87.
 *	Mods for uuslave by John Gilmore, Aug '87.
 */


#include <ctype.h>

#include "includes.h"
#include "uucp.h"
#include "sysdep.h"

#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#ifdef	NDIR
#include "ndir.h"
#else
#include <sys/dir.h>
#endif
#endif UNIX

#ifdef VMS
#include <stat.h>
#define time_t unsigned
#define MAXNAMLEN 255
#define index strchr
#define rindex strrchr
#endif VMS

#ifdef MAC
#define MAXNAMLEN 255
#endif MAC


char *index(), *rindex();

#define	NSYSTEM	100				/* max # of systems queued */

#define	CMDSLEN	5				/* Length of trailer */
#define	DATALEN	5				/* Length of trailer */
#define	XEQTLEN	5				/* Length of trailer */
#define	NUMCTRS	3				/* # file types to count */
#define	CMDTYPE	0				/* Index into scnt.cntr */
#define	DATTYPE	1				/* Index into scnt.cntr */
#define	XEQTYPE	2				/* Index into scnt.cntr */

struct	scnt {					/* System count structure */
		char	name[MAXBASENAME+1];	/* Name of system */
		short	cntr[NUMCTRS];		/* Count */
		char	stst[32];		/* STST Message */
		time_t	locked;			/* If LCK.sys present */
		int	lockpid;		/* LCK  Process ID */
		int	lockchain;		/* system # of next w/lockpid */
		char	justlck;		/* Only here due to LCK file */
		char	printed;		/* We have printed this sys */
		int	st_type;		/* STST Type */
		int	st_count;		/* STST Count */
		time_t	st_lastime;		/* STST Last time tried */
		time_t	st_retry;		/* STST Secs to retry */
	     };

int	sndx;					/* Number of systems */
struct	scnt	sys[NSYSTEM];			/* Systems queued */
int	printorder[NSYSTEM];			/* Sordid order */
int xqtisrunning = 0;

/* Sort function for qsort */
int
cmpname(a, b)
	char *a, *b;
{
	int i, j;

	i = *(int *)a;
	j = *(int *)b;

	return strcmp(sys[i].name, sys[j].name);
}


main(argc, argv)
	int argc;
	char **argv;
{
	register int i, j, nlen = 0;
	int ich;
	time_t	curtime, t;

	argv++;
	while (argv[0] && argv[0][0] == '-') {
		switch (argv[0][1]) {
		case 'x':
			debug = atoi(&argv[0][2]);
			break;
		case 'C':
			uuControl = &argv[0][2];
			break;
		default:
			fprintf(stderr, "uusnap: bad arg %s\n", argv[0]);
			exit(1);
		}
		argv++;
	}

	if (argv[0]) {
		fprintf(stderr, "usage: uusnap [-xN] [-Cconfigfile]\n");
		exit(1);
	}

	read_params(uuControl);
	chdir(Spool);
	scandir("C", CMDSLEN, NULL, CMDTYPE);
	scandir("D", DATALEN, NULL, DATTYPE);
	scandir("X", XEQTLEN, 'X', XEQTYPE);
	getstst();
	time(&curtime);
	for(i=0; i<sndx; ++i) {
		printorder[i] = i;	/* Set up initial print order */
		if((j = strlen(sys[i].name)) > nlen)
			nlen = j;	/* Get max name length */
	}

	qsort((char *)printorder, sndx, sizeof (*printorder), cmpname);
	
	for(j=0; j<sndx; ++j) {
		i = printorder[j];	/* Get real entry number */
		if (sys[i].printed) continue;
		t = (sys[i].st_lastime +sys[i].st_retry) - curtime;

		/* decide if STST text is worth printing */
		if (-t < ONEDAY*2 && sys[i].st_type == SS_WRONGTIME) {
			sys[i].stst[0] = '\0';
			if (sys[i].cntr[0]+sys[i].cntr[1]+sys[i].cntr[2] == 0)
				continue;	/* ignore entire line */
		}

		printf("%-*.*s ", nlen, nlen, sys[i].name);
		if(sys[i].cntr[CMDTYPE])
			printf("%3.d Cmd%s ", sys[i].cntr[CMDTYPE],
				sys[i].cntr[CMDTYPE]>1?"s":" ");
		else
			printf("   ---   ");
		if(sys[i].cntr[DATTYPE])
			printf("%3.d Data ", sys[i].cntr[DATTYPE]);
		else
			printf("   ---   ");
		if(sys[i].cntr[XEQTYPE])
			printf("%3.d Xqt%s ", sys[i].cntr[XEQTYPE],
				sys[i].cntr[XEQTYPE]>1?"s":" ");
		else
			printf("   ---   ");
		if(*sys[i].stst == NULL || sys[i].locked > sys[i].st_lastime) {
			if(sys[i].locked) {
				printf("*LOCKED");
				for (ich = sys[i].lockchain;
				     ich >= 0 && ich != i;
				     ich = sys[ich].lockchain) {
					printf("  %s", sys[ich].name);
					sys[ich].printed++;
				}
				if (sys[i].lockpid)
					printf("  process %d", sys[i].lockpid);
			}
			printf("\n");
			continue;
		}
		printf(" %s  ", sys[i].stst);
		/* decide if STST info is worth pursuing */
		if (-t < ONEDAY*2 && (sys[i].st_count == 0
		  || sys[i].st_type == SS_WRONGTIME
		  || (sys[i].st_type == SS_INPROGRESS && sys[i].locked))) {
			printf("\n");
			continue;
		}
		t = (sys[i].st_lastime +sys[i].st_retry) - curtime;
		if (-t < ONEDAY*2 && sys[i].st_type != SS_FAIL)
			t = 0;

		if (sys[i].st_count > MAXRECALLS)
			printf("at MAX RECALLS");
		else if (-t >= ONEDAY*2)
			printf("%ld days ago", (long)-t/ONEDAY);
		else if (t <= 0)
			printf("Retry time reached");
		else if (t < 60)
			printf("Retry time %ld sec%s", (long)(t%60),
					(t%60)!=1? "s": "");
		else
			printf("Retry time %ld min%s", (long)(t/60),
				(t/60)!=1? "s": "");
		if(sys[i].st_count > 1)
			printf(" Count: %d\n", sys[i].st_count);
		else
			printf("\n");
	}
	if (xqtisrunning)
		printf("\nUuxqt is running\n");
	exit(0);
}

scandir(prfx, flen, fchr, type)
	char *prfx, fchr;
	int type;
{
	register char *qname;
	register int i, fnamlen, plen;
	char	fnam[MAXNAMLEN+1];

	plen = strlen(prfx) + 1;
	if (!work_scan((char *)NULL, prfx))
		return;				/* No work of this type */

	while((qname = work_next()) != NULL) {
		strcpy(fnam, &qname[plen]);
		fnamlen = strlen(fnam);
		if(flen > 0) {
			char c;
			fnamlen -= flen;
			c = fnam[fnamlen];
			if (islower(c))
				c = toupper(c);
			if (type == DATTYPE && (c != 'S' && c != 'B')) {
				fnamlen -= 2;	/* For Honey DanBer */
				fnam[fnamlen] = NULL;
			} else {
				fnam[fnamlen] = NULL;
				fnamlen = MAXBASENAME; /* yes, after = NULL*/
			}
		} else {
			for(; fnamlen>0; --fnamlen) {
				if(fnam[fnamlen] == fchr) {
					fnam[fnamlen] = NULL;
					break;
				}
			}
			fnamlen = MAXBASENAME;
		}
		for(i=0; i<sndx; ++i) {
			if(strncmp(fnam, sys[i].name, fnamlen) == SAME) {
				++sys[i].cntr[type];
				break;
			}
		}
		if(i == sndx) {
			strcpy(sys[i].name, fnam);
			++sys[i].cntr[type];
			++sndx;
		}
	}
}

getstst()
{
	register int i, csys;
	register char *tp;
	char	buff[128];
	register char *qname;
	char	*name;
	char	*mungename;
	register FILE *st;
	int	pid;
	struct stat stbuf;
	long atol();

	if (work_scan((char *)NULL, "LCK"))
	while ((qname = work_next()) != NULL) {
		name = &qname[4];
		for(csys=0; csys<sndx; ++csys) {
			if(strncmp(name, sys[csys].name, SYSNSIZE)
				== SAME)
				break;
		}
		if(csys == sndx) {
			/* remember our name */
			strcpy(sys[csys].name, name);
			/* We're only in it for the lock file */
			sys[csys].justlck++;	
			++sndx;
		}
		mungename = munge_filename(qname);
		if (stat(mungename, &stbuf) < 0)
			sys[csys].locked = 1;
		else {
			sys[csys].locked = stbuf.st_mtime;
			if((st = fopen(mungename, "rb")) != NULL) {
				if (1 != fread(&sys[csys].lockpid,
				    sizeof (sys[csys].lockpid), 1, st))
					sys[csys].lockpid = 0;
				fclose(st);
			}
		}
		continue;
	}

	/*
	 * Match up locked ttys, systems, queues, etc.
	 *
	 * We go through all the sys entries, and for each one that
	 * has a LCK. file, chain it to all subsequent entries that are
	 * locked by the same pid (process).
	 *
	 * Any matching subsequent entry that is only in the table because of
	 * a LCK. file, gets its "printed" flag set, so it will not be
	 * printed on a line by itself, but instead will be printed on
	 * the end of a line for a system name.  If we found any system
	 * names while building the chain, and the first entry in the chain
	 * is a lock-only entry, we set *its* printed flag too.  If we
	 * didn't find any system names, we keep the printed flag off
	 * so that the whole lock-only chain will be printed on one line.
	 */
	for(csys=0; csys<sndx; ++csys)
		sys[csys].lockchain = -1;
	for(csys=0; csys<sndx; ++csys) {
		if (sys[csys].lockpid && sys[csys].lockchain == -1) {
			int gotsystem = 0;

			sys[csys].lockchain = csys;	/* Make 1-entry chain */

			pid = sys[csys].lockpid;
			for (i = csys+1; i < sndx; i++) {
				if (pid == sys[i].lockpid) {
					/* Add to chain */
					sys[i].lockchain = sys[csys].lockchain;
					sys[csys].lockchain = i;
					if (sys[i].justlck) sys[i].printed++;
					else		    gotsystem++;
				}
			}
			if (sys[csys].justlck && gotsystem)
				sys[csys].printed++;
		}
	}

	/* Now check system status files */
	if (work_scan((char *)NULL, "STST"))
	while ((qname = work_next()) != NULL) {
		for(csys=0; csys<sndx; ++csys) {
			if(strncmp(&qname[5], sys[csys].name, SYSNSIZE) == SAME)
				break;
		}
		strcpy(sys[csys].name, &qname[5]);
		if(csys == sndx) {
			++sndx;
		}
		if((st = fopen(munge_filename(qname), "r")) == NULL) {
			sys[csys].stst[0] = '\0';
			continue;
		}
		buff[0] = '\0';
		fgets(buff, sizeof(buff), st);
		fclose(st);
		if(tp = rindex(buff, ' '))
			*tp = NULL;		/* drop system name */
		else
			continue;
		for(i=0, tp=buff;  i<4;  ++i, ++tp)
			if((tp = index(tp, ' ')) == NULL)
				break;
		if(i != 4)
			continue;
		strncpy(sys[csys].stst, tp, sizeof(sys[csys].stst));
		tp = buff;
		sys[csys].st_type = atoi(tp);
		tp = index(tp+1, ' ');
		sys[csys].st_count = atoi(tp+1);
		tp = index(tp+1, ' ');
		sys[csys].st_lastime = atol(tp+1);
		tp = index(tp+1, ' ');
		sys[csys].st_retry = atol(tp+1);
	}
}
