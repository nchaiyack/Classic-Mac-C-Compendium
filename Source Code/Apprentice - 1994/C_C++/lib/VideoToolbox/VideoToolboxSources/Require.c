/*
Require.c

Require(long quickDrawVersion);
checks that the computer is providing the environment that your program needs
(cpu, fpu, and quickDraw), and fails gracefully if not. Also checks for
consistency of sizeof int and double among this program, the (possibly precompiled)
VideoToolbox.h header, and the Standard C library (i.e. "ANSI"). 

Call this at the beginning of your main program, so that your program will
fail gracefully--instead of crashing--when someone runs it on a computer
that lacks what your program needs, or when you've accidentally set up
inconsistent compiler options among the various parts of your project. All
tests automatically track your current compiler settings.

Here are the versions of quickdraw, as of System 7.1:

 gestaltOriginalQD = 0x000,						// original 1-bit QD
 gestalt8BitQD = 0x100,							// 8-bit color QD
 gestalt32BitQD = 0x200,						// 32-bit color QD
 gestalt32BitQD11 = 0x210,						// 32-bit color QDv1.1
 gestalt32BitQD12 = 0x220,						// 32-bit color QDv1.2
 gestalt32BitQD13 = 0x230,						// 32-bit color QDv1.3

QDv1.3, which is in System 7.0, is the first version of QuickDraw in which the
Apple Toolbox call GetPixBaseAddr() works correctly; earlier versions return
garbage.

     ==================================================================
     ROM Class             System Version       Gestalt Value
     ------------------------------------------------------------------
     Black-and-white class <  7.0               gestaltOriginalQD
     (ROM < 256K)          >= 7.0               gestaltOriginalQD
                                      and gestaltSystemVersion >= $0700

     Color QD class        <  7.0, no INITs     gestalt8BitQD
     (ROM = 256K)          6.0.3/6.0.4 and      gestalt32BitQD
                           32-Bit QD INIT 1.0
                           6.0.5-6.0.8 and      gestalt32BitQD12
                           32-Bit QD INIT 1.2
                           >= 7.0               gestalt32BitQD13

     ci class              6.0.4                gestalt32BitQD + 1
     (ROM > 256K)          6.0.5-6.0.8          gestalt32BitQD12
                           >= 7.0               gestalt32BitQD13
     ==================================================================
     (Source: Develop Issue 14, June 1993)

The types SCANF_INT, PRINTF_INT, SCANF_DOUBLE, and PRINTF_DOUBLE are for 
compatibility with MATLAB. When MATLAB is false they're just int and double.
We check both sprintf and sscanf since those are the most likely to break
if the MATLAB interface isn't set up correctly. When MATLAB is false this
is overkill, but does confirm that the ANSI library is compatible with the
current compiler options.

HISTORY:
2/20/93	dgp	Wrote it, motivated by a conversation with David Brainard.
9/8/93	dgp	Enhanced to check for inconsistent sizes of int and double.
9/13/93	dgp Moved Require to its own file.
*/
#include "VideoToolbox.h"
#include <math.h>
#include "mc68881.h"

void Require(long quickDrawVersion)
{
	long value;
	short inconsistent;
	short error;
	char s[16];
	SCANF_INT n[3];
	SCANF_DOUBLE a[5];
	static char programVsLibrary[]=
		"Oops. %s error. This program and the Standard C ÒANSIÓ library have been\n"
		"compiled with incompatible %s.\n";

	#if MAC_C
		error=Gestalt(gestaltFPUType,&value);
		if(error)PrintfExit("Sorry, I require Gestalt(). Your System is too old!\n");
		if(mc68881 && value==0)
			PrintfExit("Sorry. I've been compiled to use a floating point chip,"
				" and you don't have one.\n");
		error=Gestalt(gestaltProcessorType,&value);
		if(mc68020 && value<gestalt68020)
			PrintfExit("Sorry. I've been compiled to use a 68020 processor (or better),"
				" and you don't have one.\n");
		Gestalt(gestaltQuickdrawVersion,&value);
		if(value<quickDrawVersion)switch(quickDrawVersion){
			case gestalt8BitQD:
				PrintfExit("Sorry. This program requires Color QuickDraw.\n");
			default:
				PrintfExit("Sorry. This program requires 32-bit QuickDraw version 1.%ld\n",
				(quickDrawVersion-gestalt32BitQD)/0x10);
		}
	#endif
	inconsistent=0;
	if(sizeof(int)!=sizeof(struct PrecompileSizeofInt)){
		inconsistent=1;
		printf("Oops. "
		"The VideoToolbox.h header has been precompiled with a different size\n"
		"of int.\n");
	}
	sprintf(s,"%d,%d",(PRINTF_INT)1,(PRINTF_INT)-1);
	if(strcmp(s,"1,-1")!=0){
		inconsistent=1;
		printf(programVsLibrary,"sprintf","sizes of int");
	}
	n[2]=1;
	sscanf("1,-1","%d,%d",&n[0],&n[1]);
	if(n[0]!=1 || n[1]!=-1 || n[2]!=1){
		inconsistent=1;
		printf(programVsLibrary,"sscanf","sizes of int");
	}
	sprintf(s,"%g,%g",(PRINTF_DOUBLE)1.0,(PRINTF_DOUBLE)0.1);
	if(strcmp(s,"1,0.1")!=0){
		inconsistent=1;
		printf(programVsLibrary,"sprintf","formats for double");
	}
	a[2]=1.0;
	sscanf("1,0.1","%lf,%lf",&a[0],&a[1]);
	if(a[0]!=1.0 || a[1]!=(SCANF_DOUBLE)0.1 || a[2]!=1.0){
		inconsistent=1;
		printf(programVsLibrary,"sscanf","formats for double");
	}
	if(sqrt(0.1*0.1)!=0.1){
		inconsistent=1;
		printf("Oops: sqrt(0.1*0.1)!=0.1\n"
		"Perhaps this program and the Standard C ÒANSIÓ library have been\n"
		"compiled with incompatible formats for double.\n");
	}
	if(inconsistent)PrintfExit("");	/* Use empty string since stdio may be flakey. */
}
