/*
** client.c
** 
** Sample application using the EnvironServer for environment
** variables.
**
** To be 100% correct, the #include <MacHeaders> and calls to DisposPtr()
** are necessary.  Otherwise, if you aren't worried about a few stray
** pointer blocks and a possibly fragmented heap, don't worry...
**
** This code if freely distributable and modifiable as long as I am
** mentioned in any derived works.  This code is Copyright (C) 1993 by
** Brent Burton.
**
** Brent Burton
** brentb@tamsun.tamu.edu
**
** 5/12/93 - created
** 5/29/93 - my birthday ;-)
** 5/31/93 - cleaned up the code, comments
*/

#define PEDANTIC 1      /* define to be 1 if you want 100% correctness */

#if PEDANTIC
#  include <MacHeaders>
#endif

#include <stdio.h>
#include "GetPutEnv.h"

void print_vars(void);

main()
{
	char s[20];
	int d1, d2, d3, d4, d5;

	printf("Standard UNIX program processing here....\n\n");
	printf("Press return\n\n");
	fgets(s, 10, stdin);
	printf("Initial values for environment vars are:\n");
	
	print_vars();

	printf("\nBeginning insertions...(expect all 0's now)\n");
	d1 = putenv("TERM=new_TERM_value");
	d2 = putenv("LANG=new_LANG_value");
	d3 = putenv("HOME=new_HOME_value");
	d4 = putenv("SHELL=new_SHELL_value");
	d5 = putenv("ORGANIZATION=Texas A&M University");
	printf("Return codes are:  %d  %d  %d  %d  %d\n", d1, d2, d3, d4, d5);
	printf("Done inserting...\n");

	printf("\nNew environment variable values are:\n");
	
	print_vars();
	printf("Done.\n");
}


/*
** print_vars
**
** print out the 5 environment variables I am testing.
*/
void
print_vars(void)
{
#if PEDANTIC
	Ptr s1, s2, s3, s4, s5;
#else
	char *s1, *s2, *s3, *s4, *s5;
#endif

	if ( (s1 = getenv("TERM")) == NULL)
		printf("s1(TERM) null.\n");
	else
	{
		printf("s1(TERM) = >%s<\n", s1);
#if PEDANTIC
		DisposPtr( s1);
#endif
	}

	
	if ( (s2 = getenv("LANG")) == NULL)
		printf("s2(LANG) null.\n");
	else
	{
		printf("s2(LANG) = >%s<\n", s2);
#if PEDANTIC
		DisposPtr( s2);
#endif
	}


	if ( (s3 = getenv("HOME")) == NULL)
		printf("s3(HOME) null.\n");
	else
	{
		printf("s3(HOME) = >%s<\n", s3);
#if PEDANTIC
		DisposPtr( s3);
#endif
	}


	if ( (s4 = getenv("SHELL")) == NULL)
		printf("s4(SHELL) null.\n");
	else
	{
		printf("s4(SHELL) = >%s<\n", s4);
#if PEDANTIC
		DisposPtr( s4);
#endif
	}


	if ( (s5 = getenv("ORGANIZATION")) == NULL)
		printf("s5(ORGANIZATION) null.\n");
	else
	{
		printf("s5(ORGANIZATION) = >%s<\n", s5);
#if PEDANTIC
		DisposPtr( s5);
#endif
	}
}