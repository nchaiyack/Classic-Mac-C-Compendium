/*==============================================================================
Project:	POV-Utilities

File:	Lissa.c

Description:
Lissa is a special curve generator for the POV-Ray
Utilities.  Lissajous curves are oscillating sine curves
that generate interesting "basket-weave" patterns.   This
program accepts command line parameters to customize some
of its calculations, then generates a text file as output
that contains a sequence of points along a 3-D lissajous
trail.  The lissajous pattern is scaled to unit size, i.e.
within the box from -1,-1,-1 to 1,1,1.  The output file
format is for the Connect-the-Dots Smoother POV-Utility
(CTDS).  This lissajous output file is intended to be run
through the CTDS program, which will emit a POV-Ray scene
data file for a continuous "tube" that describes the
lissajous curve. This can then be rendered by POV-Ray.

The command line parameters are:
lissajous -xX_FACTOR -yY_FACTOR -zZ_FACTOR -pMAX_POINTS -dCTDS_DIAM -tMAX_THETA -fOUTNAME
Where:
Name        Type    DefaultVal  Description
---------   ------  ----------  ----------------------
X_FACTOR    int       3         X-Theta multiplier (-100 to 100)
Y_FACTOR    int       5         Y-Theta multiplier (-100 to 100)
Z_FACTOR    int       7         Z-Theta multiplier (-100 to 100)
MAX_POINTS  int     100         total # of points to generate (2 to 1000)
CTDS_DIAM   fp        0.1       diameter of tubes (0.001 to 1.0)*
MAX_THETA   fp        1.0       scale factor of 2*pi to iterate to (0.1 to 10.0)**
OUTNAME     str     stdout      text file name to create & write to (stdout if none)

*   Remember the Lissajous curve is within the bounds -1,-1,-1 to 1,1,1,
	so a tube diameter of 0.1 is 1/10th the size of the curve area.
**  The curve is usually traced through a single 360 degree sweep.  This
	scale factor lets you change how far to go, by multiplying by
	2*pi (or 360 degrees.)  So 0.5 would be 180 degrees, 1.0 is 360.

This program was originally written for the Macintosh under MPW C 3.2, but
was intended to be very portable to other compilers and hardware platforms.

The source code is available from the author, and may be already posted on
CompuServe and America Online.

For more information on this program, or if you have suggestions, bug-fixes,
or porting fixes, please contact the author:
Eduard [esp] Schwan
CompuServe: 71513,2161
Internet:   71513.2161@compuserve.com
USMail:     1112 Oceanic Drive, Encinitas, Calif. 92024-4007
------------------------------------------------------------------------------
Change History:
	920916	[esp]	Created.
	921020	[esp]	1.0 beta, Integrated into POV-Utilities suite
	921214	[esp]	Cleaned up a bit
	931120	[esp]	Readied for utilities 2.0 release
	931120	[esp]	CTDS 3 doesn't like degenerates when connecting beg/end, removed last point
==============================================================================*/

// Standard ANSI C headers
#include <stdio.h>	// printf()
#include <stdlib.h>	// atoi(), atof()
#include <string.h>	// strcpy()
#include <math.h>	// sin(), cos()
#include <ctype.h>	// toupper()

#if defined(applec) || defined(THINK_C)
// if using Macintosh, include user interface stuff
#include "Lissa.mac.h"
#else
// This is a no-op for non-Mac Systems
#define COOPERATE
#endif


//------------------------------------------------------------------------
#define		VERSION			"2.0"
#define		FLOAT			double
#define		SIN				sin
#define		COS				cos
#define		FILENAME_SIZE	64

#define		PI				3.1415926535793
#define		THETA_0			0.0

/* initial default values */
#define		X_FACTOR		3
#define		Y_FACTOR		5
#define		Z_FACTOR		7
#define		MAX_POINTS		100
#define		OUTER_RADIUS	1.0
#define		FULL_THETA		((FLOAT)2*PI)
#define		CTDS_DIAM		0.1


//------------------------------------------------------------------------
static	int		xFactor			= X_FACTOR;		// -xNNN
static	int		yFactor			= Y_FACTOR;		// -yNNN
static	int		zFactor			= Z_FACTOR;		// -zNNN
static	int		maxPoints		= MAX_POINTS;	// -pNNN
static	FLOAT	ctdsDiameter	= CTDS_DIAM;	// -dNNN
static	FLOAT	maxTheta		= 1.0;			// -tNNN
static	char	outFName[FILENAME_SIZE]	= "";	// -fOUTNAME


//------------------------------------------------------------------------
static void Usage(char * progname)
{
	printf("The intuitively obvious usage is:\n");
	printf(" %s -xX_FACTOR -yY_FACTOR -zZ_FACTOR -pMAX_POINTS -dCTDS_DIAM -tMAX_THETA -fOUTNAME\n", progname);
	printf(" Where:\n");
	printf("   X_FACTOR    int  is X-Theta multiplier (-100 to 100)\n");
	printf("   Y_FACTOR    int  is Y-Theta multiplier (-100 to 100)\n");
	printf("   Z_FACTOR    int  is Z-Theta multiplier (-100 to 100)\n");
	printf("   MAX_POINTS  int  is total # of points to generate (2 to 1000)\n");
	printf("   CTDS_DIAM   fp   is diameter of tubes (0.001 to 1.0)\n");
	printf("   MAX_THETA   fp   is scale factor of 2*pi to iterate to (0.1 to 10.0)\n");
	printf("   OUTNAME     str  is text file name to create & write to (stdout if none)\n");
	exit(1);
} // Usage


//------------------------------------------------------------------------
static void HandleArgs(int argc, char **argv)
{
	int		argn;
	int		showUsage = 0;

	if (argc <= 1)
		showUsage++;

	argn=1;
	while (argn < argc)
	{
		if (argv[argn][0] == '-')
		{
			if (!isalpha(argv[argn][1]))
			{
				printf("#1# Bad parameter #%d '%c'\n", argn, argv[argn][1]);
				showUsage++;
				break;
			}
			else
			switch (toupper(argv[argn][1]))
			{
				case 'X':
					xFactor = atoi(&argv[argn][2]);
// printf("dbg: -x = %d\n",xFactor);
					if (xFactor < -100)
						xFactor = -100;
					else if (xFactor > 100)
						xFactor = 100;
					break;

				case 'Y':
					yFactor = atoi(&argv[argn][2]);
// printf("dbg: -y = %d\n",yFactor);
					if (yFactor < -100)
						yFactor = -100;
					else if (yFactor > 100)
						yFactor = 100;
					break;

				case 'Z':
					zFactor = atoi(&argv[argn][2]);
// printf("dbg: -z = %d\n",zFactor);
					if (zFactor < -100)
						zFactor = -100;
					else if (zFactor > 100)
						zFactor = 100;
					break;

				case 'P':
					maxPoints = atoi(&argv[argn][2]);
// printf("dbg: -p = %d\n",maxPoints);
					if (maxPoints < 2)
						maxPoints = 2;
					else if (maxPoints > 1000)
						maxPoints = 1000;
					break;

				case 'D':
					ctdsDiameter = atof(&argv[argn][2]);
// printf("dbg: -d = %f\n",ctdsDiameter);
					if (ctdsDiameter < 0.001)
						ctdsDiameter = 0.001;
					else if (ctdsDiameter > 1.0)
						ctdsDiameter = 1.0;
					break;

				case 'T':
					maxTheta = atof(&argv[argn][2]);
// printf("dbg: -t = %f\n",maxTheta);
					if (maxTheta < 0.1)
						maxTheta = 0.1;
					else if (maxTheta > 10.0)
						maxTheta = 10.0;
					maxTheta *= FULL_THETA;
					break;

				case 'F':
					strcpy(outFName, &argv[argn][2]);
// printf("dbg: -f = %s\n",outFName);
					break;

				otherwise:
					printf("#1# Bad parameter #%d '%s'\n", argn, argv[argn]);
					showUsage++;
					break;
			}
		}
		else
		{
			printf("#2# Bad parameter #%d '%s'\n", argn, argv[argn]);
			showUsage++;
		}
	argn++;
	}

	// if requested, print showUsage error message
	if (showUsage)
		Usage(argv[0]);

} // HandleArgs


//------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int		n;
	FLOAT	x,y,z;
	FLOAT	dTheta		= FULL_THETA/MAX_POINTS;
	FLOAT	theta		= THETA_0;
	FILE	*outfile;

	// Check parameters

	HandleArgs(argc, argv);

	// create file for output, or use stdout if none specified

	if (strlen(outFName) > 0)
	{
		outfile = fopen(outFName, "w");
	}
	else
		outfile = stdout;

	// figure out how much to change theta between points, from 0 to n-1

	dTheta		= maxTheta/maxPoints;

	// dump header into output stream

	fprintf(outfile, "; Lissa %s, POV-Utility by Eduard [esp] Schwan\n",VERSION);
	fprintf(outfile, "; Note: This is a CTDS input file\n");
	fprintf(outfile, ";   Lissa Factors=(%dx,%dy,%dz)\n", xFactor, yFactor, zFactor);
	fprintf(outfile, ";   ctdsDiameter=%f, MaxTheta=%lf, MaxPoints=%d\n", ctdsDiameter, maxTheta, maxPoints);

	// Do the real hard calculations on each point here :-)

	for (n=0; n<maxPoints; n++)
	{
		// give occasional user feedback
		if (((n % 50) == 0) || (n == maxPoints-1))
			printf("At point # %d\n",n);
		// give breath to other processes
		COOPERATE
		// figure it out
		x = OUTER_RADIUS * SIN(xFactor*theta);
		y = OUTER_RADIUS * COS(yFactor*theta);
		z = OUTER_RADIUS * SIN(zFactor*theta);
		// print it out
		fprintf(outfile, "%12.8g  %12.8g  %12.8g  %14.6g\n", x, y, z, ctdsDiameter);
		// next..
		theta += dTheta;
	}

	// close the output file if not stdout & it was opened OK

	if (outfile && (outfile != stdout))
		fclose(outfile);

	return 0;

} // main
