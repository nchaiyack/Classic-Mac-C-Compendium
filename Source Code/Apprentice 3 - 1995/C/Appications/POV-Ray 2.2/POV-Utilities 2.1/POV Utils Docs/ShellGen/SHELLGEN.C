/*
------------------------------------------------------------
	ShellGen - Snail shell generator
	By: ? - Algorithm Stolen from the following E-mail message,
		and turned into real code by Eduard [esp] Schwan
	#: 44383 S16/Raytrace Sources
	    13-Aug-92  11:48:24
	Sb: #44269-#Help!
	Fm: Dan Farmer 70703,1632
	To: Jerry Thomaston/DCA Inc. 76004,1655 (X)
	
	Jerry,
	
	You're doing the snail shell the hard way.  Have you tried
	shellgen? I've got a QBasic version for DOS (has a simple
	graphics display), but there's a portable c version in the
	dkb utilities package.  It uses a logarithmic spiral ala
	Clifford Pickover to rotate spheres into a shell. 
		-Dan
------------------------------------------------------------
	-oOutputFileName
	-nNumSpheres
	-aAlpha
	-bBeta
	-gGamma
	-sScaleFactor
	-T // (Do individual textures)
	-c // (Do CTDS output)
------------------------------------------------------------
	12/16/93 [esp] Added individual texturing and POV-Ray 2.0 syntax
------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#if defined(applec) || defined(THINK_C)
#include <SANE.h>	// pi()
#include "ShellGen.mac.h"
#else
// This is a no-op for non-Mac Systems
#define COOPERATE
#endif

#ifndef M_PI
#define M_PI	3.14159265358979
#endif

static	double	alpha = 2.1 ;   /* > 1 */
static	double	beta = -2.0 ;   /* ~ -2 */
static	double	gamma = 1.0 ;   /* 0.01 to 3 */
static  int steps = 200 ;   /* ~number of spheres generated */
static	double	a = 0.15 ;  /* exponent constant */
static	double	k = 1.0 ;   /* relative size */
static	int		DoIndivTextures = 0;/* flag, FALSE=One tex on whole obj, TRUE=separate tex on each sphere */
static	int		DoCTDS = 0;	/* flag, FALSE=POV-Ray, TRUE=CTDS */
static	char	*out_fname;
static	FILE	*out_file;

//--------------------------------------------------
static void get_cmdline_args(int argc, char **argv) // [esp] added routine
{
	int		c;

	c = 1;
	while (c < argc)
	{
		if ((argv[c][0] == '-') || (argv[c][0] == '/'))
		{
			switch (argv[c][1])
			{
				case 'o':	// -oOutputFileName
				case 'O':
					// get filename
					strcpy(out_fname,&argv[c][2]);
					break;

				case 'n':	// -nNumSpheres
				case 'N':
					steps = atoi(&argv[c][2]);
					if ((steps <= 1) || (steps > 9999))
						steps = 200;
					break;

				case 'a':	// -aAlpha
				case 'A':
					alpha = atof(&argv[c][2]);
					if ((alpha <= 1.0) || (alpha > 99.0))
						alpha = 1.1;
					break;

				case 'b':	// -bBeta
				case 'B':
					beta = atof(&argv[c][2]);
					if ((beta <= -3.0) || (beta > -1.0))
						beta = -2.0;
					break;

				case 'g':	// -gGamma
				case 'G':
					gamma = atof(&argv[c][2]);
					if ((gamma < 0.01) || (gamma > 4.0))
						gamma = 1.0;
					break;

				case 's':	// -sScaleFactor
				case 'S':
					k = atof(&argv[c][2]);
					if ((k <= 0.0) || (k > 99.0))
						k = 1.0;
					break;

				case 't':	// -T (Do individual textures)
				case 'T':
					DoIndivTextures = 1;
					break;

				case 'c':	// -c (Do CTDS output)
				case 'C':
					DoCTDS = 1;
					break;

				default:
					printf("Error! Unrecognized parameter '%s', ignored.\n",argv[c]);
					break;
			} // switch
		}
		else
			printf("Error! Unrecognized parameter '%s', ignored.\n",argv[c]);

		/* go to next parameter */
		c++;
	}
} // get_cmdline_args


//--------------------------------------------------
main(int argc, char **argv)
{
	double  r,x,y,z,rad,angle;
	double  xmid, xmin, xmax;
	double  ymid, ymin, ymax;
	double  zmid, zmin, zmax;
	int i ;

	out_fname = malloc(80);
	strcpy(out_fname,"ShellGen");

    if (argc > 1)
	    get_cmdline_args(argc, argv);

	// if no suffix on output file, add one
	if (!strchr(out_fname,'.'))
	{
		if (DoCTDS)
			strcat(out_fname,".ctd");
		else
			strcat(out_fname,".inc");
	}

    if ((out_file = fopen(out_fname,"w")) == NULL)
	{
		fprintf(stderr,"Error creating file '%s'.", out_fname);
		exit(1);
	}

	if (DoCTDS)
	{
		fprintf(out_file, "; CTDS format file, created by ShellGen\n");
		fprintf(out_file, "; alpha=%g beta=%g gamma=%g scale=%g\n",
			alpha,beta,gamma,k);
	}
	else
	{
		fprintf(out_file, "// Persistence of Vision 2.0 file, created by ShellGen\n");
		fprintf(out_file, "// nSpheres= %d alpha=%g beta=%g gamma=%g scale=%g\n\n",
			steps, alpha,beta,gamma,k);
		fprintf(out_file, "#declare ShellTexture = texture {\n");
		fprintf(out_file, "  pigment { marble scale 0.2 rotate -15*z }\n");
		fprintf(out_file, "  finish { ambient 0.2 diffuse 0.6 specular 0.7 }\n");
		fprintf(out_file, "} // ShellTexture\n\n");
		fprintf(out_file, "#declare ShellShape = union {\n");
	}

	xmin = 1e6; xmax = -xmin;
	ymin = 1e6; ymax = -ymin;
	zmin = 1e6; zmax = -zmin;

	for ( i = -steps*2.0/3.0; i <= steps/3.0 ; ++i )
	{
        COOPERATE
        if ((i % 10) == 0)
        	printf(".");
		angle = 3.0 * 6.0 * M_PI * (double)i / (double)steps ;
		r = k * exp( a * angle ) ;
		x = r * sin( angle ) ;
		z = r * cos( angle ) ;
		/* alternate formula: y = - beta * r */
		y = - alpha * r;
		rad = r / gamma ;

		if (x-rad < xmin) xmin = x-rad;
		if (y-rad < ymin) ymin = y-rad;
		if (z-rad < zmin) zmin = z-rad;

		if (x+rad > xmax) xmax = x+rad;
		if (y+rad > ymax) ymax = y+rad;
		if (z+rad > zmax) zmax = z+rad;

		if (DoCTDS)
		{
			fprintf(out_file, "%g %g %g  %g\n", x, y, z, rad ) ;
		}
		else
		{
			fprintf(out_file, "  sphere { 0*x, 1\n");
			if (DoIndivTextures)
				fprintf(out_file, "    texture { ShellTexture }\n");
			fprintf(out_file, "    scale %8.5g rotate %g*y translate <%g, %g, %g>\n",
				rad,
				angle*180.0/M_PI,
				x, y, z );
			fprintf(out_file, "  }\n");
		}
	} // for

	printf("\n");


	xmid = (xmin+xmax)/2.0;
	ymid = (ymin+ymax)/2.0;
	zmid = (zmin+zmax)/2.0;

	if (!DoCTDS)
	{
		fprintf(out_file, "  // bounded_by { box { <%g, %g, %g> <%g, %g, %g> } }\n",
			xmin, ymin, zmin, xmax, ymax, zmax);
		fprintf(out_file, "  // FYI, Center of object = <%g, %g, %g>\n",
			xmid, ymid, zmid);
			if (!DoIndivTextures)
				fprintf(out_file, "  texture { ShellTexture }\n");
		fprintf(out_file, "} // ShellShape\n");
	}

    fclose(out_file);
}
