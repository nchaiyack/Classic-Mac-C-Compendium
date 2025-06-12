/*
*  xfixup.c
*
*  Code to take a control file and an update stream and produce a 
*  modified file.
*  For use with xdiff.c.
*
*  Tim Krauskopf  April 1, 1988
*  Copyright (c) 1988 Board of Trustees of the University of Illinois
*
*/
#include <stdio.h>
#include <fcntl.h>
#include "xdiff.h"

diffile f1,finf;

FILE *fp;
int32 check1,check2;
int docheck=1,sf=1;

#ifdef CTSS
long myseek();
#define lseek(A,B,C)  myseek((A),(B),(C))
#endif

main(argc,argv)
	int argc;
	char *argv[];
	{
	int32 ln,sl,el;
	struct lndiff *lnp,*tlnp;
	char *c1,*c2;
	
	if (argc < 3) {
#ifdef CTSS
		puts(" Usage: xfix filename updatefile [-nc]");
#else
		puts(" Usage: xfix [-nc] filename updatefile");
#endif
		puts(" -nc option doesn't require checksums to match");
		exit(1);
	}
	
	if (!strncmp(argv[1],"-nc",3)) {		/* check nc option */
		sf++;
		docheck=0;
	}
	else if (!strncmp(argv[argc-1],"-nc",3)) {
		docheck=0;
	}
	
	strcpy(f1.fn,"c.");
	strcat(f1.fn,argv[sf]);			/* name of control file */
	
	printf("Loading: %s and %s\n",f1.fn,argv[sf+1]);
	
	if (0 > loadfile(&f1)) {
		puts(" Cannot load control file");
		exit(1);
	}
	
	strcpy(finf.fn, argv[sf+1]);
	if (0 > loadfile(&finf)) {
		puts(" Cannot load update file");
		exit(1);
	}
/*
*  both control and update file loaded, now write out modified file
*/
	if (NULL == (fp = fopen(&f1.fn[2],"w",0666))) {
		puts(" Cannot create output file");
		exit(1);
	}
/*
*  Check first file whole checksum, after printing message.
*/
	lnp = finf.lstore;			/* start of line info */
	c1 = lnp->ld;
	lnp++;						/* line info after first line */
	c2 = lnp->ld - 1;			/* end of first line */
	*c2 = '\0';
	puts(c1);					/* prints first line of update file */
	

	if (2 != sscanf(lnp->ld,"x%ld,%ld",&check1,&check2)) {
		puts(" Cannot find checksums in update file");
		exit(1);
	}
	if (docheck && (check1 & 0x3fffffff) != (f1.tsum & 0x3fffffff) ) {
		puts(" Error: Checksum failed to match for control file.");
		puts(" You can use the -nc option to skip the checksum verification,");
		puts(" but the resulting file is likely to be corrupted.");
		exit(1);
	}
	lnp++;
	
	printf("Checksum ok, starting %ld lines of updates...\n",finf.lines);
	
	for (ln=3; ln < finf.lines; ln++,lnp++) {
		if (*lnp->ld == 'c') {			/* copy instruction */
			if (2 != sscanf(lnp->ld,"c%d,%d",&sl,&el)) {
				puts(" Bad copy command in updates");
				exit(1);
			}
			if (sl > f1.lines)
				sl = f1.lines;		/* safety, when user messes up */
			if (el > f1.lines)
				el = f1.lines;		/* for corrupted control files */
				
			tlnp = f1.lstore + sl;
			c1 = tlnp->ld;
			tlnp = f1.lstore + el + 1;
			c2 = tlnp->ld;
			fwrite(c1,1,c2-c1,fp);	
		}
		else if (*lnp->ld == 'a') {		/* append instruction */
			fwrite(lnp->ld + 1,1,(lnp+1)->ld - lnp->ld -1,fp);
		}
		else {
			puts(" Error in update file, not a 'c' or 'a' command ");
			exit(1);
		}
	}
	
	fclose(fp);
	
	puts("Done with updates");
	exit(0);
	
}
	
/**********************************************************************/

#define fix
/***********************************************************/
/*  loadfile
*  Load and compute id checksums for a new file.
*  Efficiency counts!
*
*  Loads the file structure with every applicable field.
*
*  Returns 0 if ok, negative on error.
*/
loadfile(f)
	diffile *f;
	{
	register char *p;
	register int i;
	register int32 sum,sumt;
 	struct lndiff *sp;
	int32 j;
	char *endp;
	
	if (0 > (f->fd = open(f->fn,O_RDONLY))) {
		return(-1);
	}
	
	f->len = lseek(f->fd,(long)0,2);		/* find length of file */
	lseek(f->fd,(long)0,0);					/* back to beginning */
	
	if (f->store)							/* if previous allocation, lose it */
		free(f->store);
		
	if (NULL == (f->store = malloc(f->len+10))) {
		return(-2);
	}
	
	if (0 >= (f->len = read(f->fd,f->store,f->len))) {
		return(-3);
	}
	
	close(f->fd);
	
/*
*  len now has the exact number of bytes read, and that is what we'll work with.
*
*  Allocate storage by counting the number of end-of-lines in the file and 
*  then allocate that number of line control blocks.
*/
	f->store[f->len] = '\n';		/* need anchoring line end */
#ifdef fix
	f->store[++f->len] = '\n';		/* double anchor for fixup */
#endif

	p = f->store;
	endp = f->store + f->len;		/* end of scanning */
	i = 0;							/* end of lines found */
	while (p < endp)
		if (*p++ == '\n')
			i++;
	
	f->lines = i;
	f->curstart = 0L;
	
	if (f->lstore)					/* free previous allocation */
		free(f->lstore);
		
	if (NULL == (f->lstore = (struct lndiff *)malloc(
		(i+1) * sizeof(struct lndiff)  ))) {
		return(-4);
	}
/*
*  fill line control structure with hashing data.  One in 4 billion chance
*  of two non-matching lines accidentally matching.
*/
	p = f->store;
	endp = f->store + f->len;		/* end of scanning */
	sp = f->lstore;
	j = 0;							/* counting segments */
	sumt = 0L;
	
	while (p < endp) {
		sum = 0L;
		sp->ld = p;					/* start of this line's data */
		while (*p != '\n') {
			if (sum & 0x8000000L)  	/* rotate operation */
				sum = (sum & 0x7fffffffL) + 1;
			sum = (sum << 1) + (*p++);
		}
		sp->id = sum;
		sumt += sum;
		sp->num = j++;
		sp++; p++;
	}
		
	f->tsum = sumt;
	return(0);
	
}

#ifdef CTSS

/******************************************************************/
/* open,read,lseek and close
*  for fun-time CTSS
*   One file at a time, because there is only one fp.
*/
FILE *cfp;

open(s,i)
	char *s;
	int i;
	{
	
	if (NULL == (cfp = fopen(s,"r")))
		return(-1);
		
	else
		return(1);
		
}

read(fd,buf,n)
	int fd,n;
	char *buf;
	{
	
	if (fd != 1)
		return(-1);
		
	return(fread(buf,1,n,cfp));

}

close(fd)
	int fd;
	{
	
	if (fd != 1)
		return(-1);
		
	return(fclose(cfp));
	
}

long myseek(fd,l,code)
	long l;
	int code,fd;
	{
	
	if (code == 0 && l == (long)0) {
		rewind(cfp);
		return((long)0);
	}
	
	if (code == 2 && l == (long)0) {
		fseek(cfp,(long)0,2);
		return(ftell(cfp));
	}
	
	return((long)-1);
}
#endif
