/**********************************/
/* test.c                         */
/*********************************************************************/
/* Testing function for the free sscanf()-compatible function.       */
/*                                                                   */
/* Send all bug reports to Brent Burton, bpb9204@tamsun.tamu.edu     */
/*********************************************************************/

#include <MacHeaders>
#include <stdio.h>          /* for comparisons */
#include "sscanf.h"      /* protos */

/** Uncomment only ONE of the following lines to do various testing **/

#define DO_TIMINGS    /* Test the conversion rates to the ANSI sscanf() */
/* #define FLOAT         /* Test the floating point scanning */
/* #define TEST_OCTAL    /* Test octal number scanning */

void main(void)
{
#ifdef FLOAT             /* test the floating point scanning */
	char *s = "-.34e-2";
	char *f = "%lf";
	double x;
	int c;
	
	x = 10.0;
	printf( "%lf\n", x);
	c = mysscanf( s, f, &x);
	printf( "args: %d   %s= %lf\n", c, s, x);
#endif

#ifdef TEST_OCTAL
	char *s = "127";		/* base-n string */
	char *f = "%o";
	int number;
	int c;
	
	number = 0;		/* change the variable so we know what we scan in */
	c = mysscanf( s, f, &number);
	printf("args=%d, %s = %o\n", c, s, number);
#endif

#ifdef DO_TIMINGS
   /* This code tests scanning in 10000 items of whichever type, */
   /* defined below.                                             */

	int count;
	char *s = "1.234e-123";		/* string to scan data from */
	char *f = "%lf";			/* format specifier for above data */
	char s1[100];				/* variable to store scanned data in */
	unsigned long int l1;
	int i1;
	double f1;
	long i;						/* loop counter */
	long start, end;			/* starting and ending times */
	
	printf("starting the timing...\n\n");
	start = TickCount();
	for (i=0; i<10000L; i++)
		count = mysscanf(s, f, &f1);
	end = TickCount();
	printf("mysscanf read 10000 in %5.2f seconds\n\n", (float)(end-start)/60.0);

	printf("starting the timing...\n");
	start = TickCount();
	for (i=0; i<10000L; i++)
		count = sscanf(s, f, &f1);
	end = TickCount();
	printf("sscanf read 10000 in %5.2f seconds\n", (float)(end-start)/60.0);
#endif

} /* main() */



