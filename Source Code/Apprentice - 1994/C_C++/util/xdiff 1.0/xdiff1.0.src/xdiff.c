
# define MASTERDEF

# include <stdio.h>
# include <fcntl.h>
# include "xdiff.h"

static char space[1024];
/*
*  xdiff
*  Compares two text files and outputs a differences file which can be used to
*  execute duplicate edits on another system.
*
*
*  National Center for Supercomputing Applications
*  March 7, 1988
*  Completed June, 1989
*
*  This program is in the public domain
*
*  Tim Krauskopf
*/

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
		free((Ptr) f->lstore);
		
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
	
	sp->ld = endp;					/* anchor the endpoint */
	f->tsum = sumt;
#ifdef UNIX
	printf(" %ld bytes, %ld lines\n",f->len,f->lines);
#endif	
	return(0);
	
}

/*************************************************************/
/*  compfiles
*   Compare two loaded file structures and output the commands
*   required to build the second structure starting with only
*   the first file.
*
*   returns 0 if ok, negative for errors
*/
compfiles(f1,f2,dp)
	diffile *f1,*f2;
	FILE *dp;
	{
	register int i,j;
	register struct lndiff *lp1,*lp2;
	struct lndiff *end2;
	char *p;

/*
*  write some file info
*/
	fprintf(dp,"XDIFF update file for: %s\n",f2->fn);
/*
*  write the two file checksums as the first data line
*/
	fprintf(dp,"x%ld,%ld\n",f1->tsum,f2->tsum);
/*
*  For each line in the second file, find it in the first file.
*  For each line found, determine largest block and output a command.
*/
	lp2 = f2->lstore;			/* start of second file */
	f2->curstart = 0;
	end2 = f2->lstore + f2->lines;
	while (lp2 < end2) {		/* traverse second file */
	
		i = firstsrch(f1,f2);	/* try to find a match */
		if (i < 0) {			/* no match */
			p = lp2->ld;		/* start of line to write out */
			lp2++;				/* next line struct */
			f2->curstart++;
			fputc('a',dp);		/* write signal character */
			fwrite(p,1,(int)(lp2->ld - p),dp);	/* write out line */
		}
		else {					/* find length of block */

			j = i;				/* save starting line # of block */
			lp1 = f1->lstore + i;	/* where block must start */
			do {  					/* expand block */
				lp2++; lp1++;
				f2->curstart++;	/* position in file2 */
				f1->curstart++;
				i++;			/* how many lines */
			} while (lp2 < end2 && lp2->id == lp1->id); 
			
			fprintf(dp,"c%d,%d\n",j,i-1);
		}
		
		
	}  /* end traverse of second list */
		
	return(0);
}

/****************************************************************/
/*  firstsrch()
*  find the first matching id number from the first file
*  Definitely a O(n) search - linear.
*/

firstsrch(f1,f2)
	diffile *f1,*f2;
	{
	register uint32 i,lno;
	struct lndiff *ln1,*ln2,*endln,*stopln;
	
	ln2 = f2->lstore + f2->curstart; 
	i = ln2->id;
	ln1 = f1->lstore + f1->curstart;	/* start of search */
	lno = f1->curstart;
	endln = f1->lstore + f1->lines;		/* mid point of search */
	stopln = ln1;						/* end of search is where it started */
	
	while (ln1 < endln) {
		if (ln1->id == i)
			return(lno);
		lno++;
		ln1++;				/* next line to check */
	}
	
	ln1 = f1->lstore;		/* back to beginning */
	lno = 0L;
	
	while (ln1 < stopln) {
		if (ln1->id == i)
			return(lno);
		lno++;
		ln1++;
	}
	
	return(-1);				/* not found */
	
	
}


#ifdef UNIX

diffile f1,f2;

FILE *diffp;

main(argc,argv)
	int argc;
	char *argv[];
	{
	int32 dlen;
	
	if (argc < 2) {
		puts(" Usage: xdiff filename [diff_filename]");
		puts(" The control file is always prefixed with 'c.'");
		exit(1);
	}

	strcpy(f1.fn,"c.");			/* control file prefix */
	strcat(f1.fn,argv[1]);
	strcpy(f2.fn,argv[1]);		/* file to "diff" */
	
	printf("Loading control file: %s\n",f1.fn);
	
	if (loadfile(&f1)) {
		fprintf(stderr," Cannot load file: %s\n",f1.fn);
		exit(1);
	}
	
	printf("Loading edited file: %s\n",f2.fn);
	
	if (loadfile(&f2)) {
		fprintf(stderr," Cannot load file: %s\n",f2.fn);
		exit(1);
	}
	
	if (argc >= 3) {				/* where to send results */
		diffp = fopen(argv[2],"w");
		printf("Writing update information to: %s\n",argv[2]);
	}
	else {
		diffp = stdout;
		puts("Writing update information to: stdout");
	}
	
	if (NULL == diffp) {
		puts(" Cannot open diff output ");
		exit(1);
	}
	
	compfiles(&f1,&f2,diffp);
	
	dlen = ftell(diffp);
	printf("Update file is %ld bytes - done.\n",dlen);
	
	fclose(diffp);
	exit(0);
}

putit(x,y,s)
	int x,y,s;
	{
	puts(s);
	
}

#endif

