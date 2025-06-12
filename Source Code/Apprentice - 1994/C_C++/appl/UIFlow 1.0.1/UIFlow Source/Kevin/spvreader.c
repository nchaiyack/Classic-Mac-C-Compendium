
/*

This program will read an input file for multiflow off the network and
write the file out to a file named spv.in. Then Dr. S. P. Vankas subroutine
is called. This subroutine reads in the the spv.in file and generates an
output file called spv.out, and returns. Then the routine to convert the
vset hdf file to a raster image is called, and the results are shiped back
to CFDFront.

cc -I/usr/local/apps/dtm/include main.c /usr/local/apps/dtm/lib/libdtm.a -lnet
	Thomas Redman
*/


#define MAC
#include <stdio.h>
#pragma segment conv
#include <fcntl.h>

#ifdef MAC
#define malloc NewPtr
#define free DisposePtr
#endif

/* dtm stuff. */
int   inport, outport;
FILE * 	fptest;
int 	xsize;
extern void UIFLOW();

/* flags. */
int flags;
#define dtm 1
#define dots 2
#define DFR8 4 
#define TESTFILE "test.out"

extern void ShowL__FPc(char*);
extern short StopKey__Fv(void);
extern short SetStop__Fv(void);

void SHOWLINE(char * msg)
	{
	ShowL__FPc(msg);
	}

short STOPKEY(void)
	{
	short i;
	i = StopKey__Fv();
	return i;
	}

void ABORTC(void)
	{
	SetStop__Fv();
	return;
	}

int RunSpv (char * /*dfFileName*/)
{
/*	extern int ConvertVSet2Raster(); */
	
	/* call the uiflow stuff. */
	UIFLOW ();
/*	ConvertVSet2Raster (dfFileName); */
	return 0;
}
