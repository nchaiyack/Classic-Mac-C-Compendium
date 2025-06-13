/*
 Program : Coil
 Purpose : Create coil objects for PoV Ray v1.0 raytracer
 Created : 7/25/92
 By      : Bill Kirby CIS [70711,2407]
 File    : Coil.c
 Compiler: Borland C++ v3.1
 Model   : Small
 Comments: Used to create twisted coil object made with spheres
*/

/*
	Updates for Macintosh port and command line additions
	by Eduard [esp] Schwan 11/17/92 - 11/20/93
	These changes are:
	- Program now compiles under Apple's and Symantec's Mac C compilers as well
	- Program now updated to POV-Ray 2.0 format.
	- Program now can output CTDS format as well as POV-Ray 2.0 format.
	- If no command line parms given, program acts as before, prompting
	  the user for inputs.  But now, each of the variables can be set via
	  command line options too.  They can be prefaced with either '-' or '/':
	  Option			Description
	  -----------		-----------------------
	  -oFname			output file name to create
	  -nNumSpheres		Total # of spheres to create
	  -xNumSpheresInX	# of sphere "strands"
	  -tNumTwistsPerRev	# of twists in 1 revolution
	  -aMajorRadius		Major "torus" radius
	  -bMinorRadius		Minor radius
	  -rSphereRadius	Size of the spheres themselves
	  -sScaleFactor		overall object scale factor
	  -fFudgeFactor		bounding shape's fudge factor
	  -c 				Do CTDS output, not POV-Ray
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// vvv [esp] added
#if defined(applec) || defined(THINK_C)
// if using Macintosh, include user interface stuff
#include "COIL.mac.h"
#else
// This is a no-op for non-Mac Systems
#define COOPERATE
#endif
// ^^^ [esp] added

#ifndef PI
#define PI 3.1415926535897932384626
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

//--------------------------------------------------
static void show_title(void);
static void get_cmdline_args(int argc, char **argv);
static void get_inputs(void);
static void write_header(char *filename, char *union_name);
static void write_piece(double xpos, double ypos, double zpos, double radius);
static void write_end(void);
static void err_exit(char *message);

//--------------------------------------------------
static FILE *outfile;
static char *filename, *union_name, *buff; // [esp] changed to dynamic allocation
static double x1,y1,x2,y2,z2,xpos,ypos,zpos,Rad1,rad2,radius, fudge; // [esp] added fudge
static double angle1,angle2,k;
static long i,j,Ntwist,Ntube,steps;
static int		DoCTDS;	// added [esp]
static double xmax, xmin, ymax, ymin, zmax, zmin; // [esp] moved init down into main

//--------------------------------------------------
main(argc,argv)
int argc;  char *argv[];
{

// vvv [esp] added
// [esp] NOTE:
// this was done primarily to reduce global static variable space
// in Macintosh implementation, since we're linking ALL the utilities
// together into one giant application.
	filename = malloc(80);
	union_name = malloc(80);
	buff = malloc(256);
// [esp] NOTE:
// this was done for Mac implementation compatibility.  Because main() is
// really just a subroutine which can be called multiple times, we can't
// expect these globals to be re-initialized each time in, unless we
// explicitly set them here.
	xmax = -1e300;
	xmin = 1e300;
	ymax = -1e300;
	ymin = 1e300;
	zmax = -1e300;
	zmin = 1e300;
// ^^^ [esp] added

    show_title();

/* Get coil values from user */
    if (argc > 1)
	    get_cmdline_args(argc, argv); // [esp] added
	else
        get_inputs();

/* Open file for output and write header */
    printf("\n\nCreating data file %s\n",filename);
    if ((outfile = fopen(filename,"w")) == NULL)
     err_exit("Opening file.");

	if (!DoCTDS)
 	   write_header(filename,union_name);

/* Compute twisted coil object */

    for(i=0;i<steps;++i){
      COOPERATE // [esp] added for Mac multitasking
      angle1 = 2 * 3.14159265;
      angle1 *= Ntube * (double)i/ (double)steps;
      x1 = cos( angle1 );
      y1 = sin( angle1 );
      angle2 = (double)( Ntwist + 1.0/Ntube) * angle1;
      x2 = cos( angle2 );
      z2 = sin( angle2);
      xpos = k * ((Rad1 * x1) + (rad2 * x2 * x1));
      ypos = k * ((Rad1 * y1) + (rad2 * x2 * y1));
      zpos = k * rad2 * z2;
      xmax = max(xmax,xpos);
      xmin = min(xmin,xpos);
      ymax = max(ymax,ypos);
      ymin = min(ymin,ypos);
      zmax = max(zmax,zpos);
      zmin = min(zmin,zpos);
      write_piece(xpos,ypos,zpos,radius);
    }

/* Write object end and close file */
	if (!DoCTDS)
	    write_end();
    fclose(outfile);
    printf("\nFile %s created.\n",filename);
    return(0);
}

//--------------------------------------------------
static void show_title(void)
{
    printf("Coil v2.00b\n");
    printf("Creates a PoV-Ray v2.0 raytracer data file of a twisted coil object.\n");
    printf("- W. D. Kirby 7/25/92\n\n");
// [esp] This next line should be removed if author syncs this source
    printf("- Non-Sanctioned changes by Eduard [esp] Schwan 11/20/93\n\n");
}

//--------------------------------------------------
static void init_defaults(int	forceIt) // [esp] added routine
{
    if((filename[0]=='\0')||forceIt) strcpy(filename,"coil.inc");
    if((union_name[0]=='\0')||forceIt) strcpy(union_name,"coil");
    if((steps==0)||forceIt) steps = 100;
    if((Ntube==0)||forceIt) Ntube = 2;
    if((Ntwist==0)||forceIt) Ntwist = 2;
    if((Rad1==0.0)||forceIt) Rad1 = 1.0;
    if((rad2==0.0)||forceIt) rad2 = 0.25;
    if((radius==0.0)||forceIt) radius = 0.25;
    if((k==0.0)||forceIt) k = 1.0;
    if((fudge==0.0)||forceIt) fudge = 1.05;
    if((DoCTDS==0)||forceIt) DoCTDS = 0;
} // init_defaults

//--------------------------------------------------
static void get_cmdline_args(int argc, char **argv) // [esp] added routine
{
	int		c;

	init_defaults(TRUE);
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
					strcpy(filename,&argv[c][2]);
					// if no suffix, add one
					if (!strchr(filename,'.'))
						strcat(filename,".inc");
					// also create name of object
					strcpy(union_name,filename);
					// make sure to remove the filename suffix!
					*(char*)strchr(union_name,'.') = '\0';
					break;

				case 'n':	// -nNumSpheres
				case 'N':
					steps = atoi(&argv[c][2]);
					if ((steps <= 0) || (steps > 9999))
						steps = 100;
					break;

				case 'x':	// -xNumSpheresInCrossSection
				case 'X':
					Ntube = atoi(&argv[c][2]);
					if ((Ntube <= 0) || (Ntube > 99))
						Ntube = 2;
					break;

				case 't':	// -tNumTwistsPerRevolution
				case 'T':
					Ntwist = atoi(&argv[c][2]);
					if ((Ntwist <= 0) || (Ntwist > 999))
						Ntwist = 2;
					break;

				case 'a':	// -aMajorRadius
				case 'A':
					Rad1 = atof(&argv[c][2]);
					if ((Rad1 <= 0.0) || (Rad1 > 999.0))
						Rad1 = 1.0;
					break;

				case 'b':	// -bMinorRadius
				case 'B':
					rad2 = atof(&argv[c][2]);
					if ((rad2 <= 0.0) || (rad2 > 999.0))
						rad2 = 0.25;
					break;

				case 'r':	// -rSphereRadius
				case 'R':
					radius = atof(&argv[c][2]);
					if ((radius <= 0.0) || (radius > 999.0))
						radius = 0.25;
					break;

				case 's':	// -sScaleFactor
				case 'S':
					k = atof(&argv[c][2]);
					if ((k <= 0.0) || (k > 999.0))
						k = 1.0;
					break;

				case 'f':	// -fFudgeFactor
				case 'F':
					fudge = atof(&argv[c][2]);
					if ((fudge < 1.0) || (fudge > 2.0))
						fudge = 1.1;
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
static void get_inputs(void)
{
/* Get coil values from user */

    printf("Output filename? [coil.inc]: ");
    gets(buff);
    strcpy(filename,buff);

    printf("Union name? [coil]: ");
    gets(buff);
    strcpy(union_name,buff);

    printf("Number of spheres? [100]: ");
    gets(buff);
    steps = atoi(buff);

    printf("Number of spheres in cross-section? [2]: ");
    gets(buff);
    Ntube = atoi(buff);

    printf("Number of twists per revolution? [2]: ");
    gets(buff);
    Ntwist = atoi(buff);

    printf("Major radius? [1.0]: ");
    gets(buff);
    sscanf (buff, "%lf", &Rad1);

    printf("Minor radius? [0.25]: ");
    gets(buff);
    sscanf (buff, "%lf", &rad2);

    printf("Sphere radius? [0.25]: ");
    gets(buff);
    sscanf (buff, "%lf", &radius);

    printf("Overall scale value (k)? [1.0]: ");
    gets(buff);
    sscanf (buff, "%lf", &k);

/* Set up default values */
	init_defaults(FALSE); // [esp] changed to call common routine instead
}

//--------------------------------------------------
static void write_header(char *filename,  char *union_name)
{
    fprintf(outfile,"// File : %s  Union Name : %s \n",filename,union_name);
    fprintf(outfile,"// This data file created by COIL.EXE v2.0 for the PoV Ray v2.0 raytracer.\n\n");
    fprintf(outfile,"//Twists=%ld Cross Section=%ld Spheres=%ld Scale=%lf\n\n",Ntwist,Ntube,steps,k);
    fprintf(outfile,"#declare %s_Texture = texture {\n",union_name);
    fprintf(outfile,"  pigment { color rgb <0.0, 1.0, 0.0> }\n");
    fprintf(outfile,"  finish {\n");
    fprintf(outfile,"    ambient 0.2\n");
    fprintf(outfile,"    diffuse 0.7\n");
    fprintf(outfile,"    phong 0.8 phong_size 60.0\n");
    fprintf(outfile,"  }\n");
    fprintf(outfile,"}\n\n");
    fprintf(outfile,"#declare %s_Shape = object {\n union {\n",union_name);
}

//--------------------------------------------------
static void write_piece(double xpos, double ypos, double zpos,double radius)
{
	if (DoCTDS)
   	    fprintf(outfile,"%6.3g %6.3g %6.3g   %6.3g\n",xpos,ypos,zpos,radius);
	else
   	    fprintf(outfile,"      sphere { <%g, %g, %g> %g }\n",xpos,ypos,zpos,radius);
}

//--------------------------------------------------
static void write_end(void)
{
    fprintf(outfile," }\n\n");
    xmax = fudge * (xmax + radius); // [esp] changed to use fudge
    xmin = fudge * (xmin - radius); // [esp] " " "
    ymax = fudge * (ymax + radius); // [esp] " " "
    ymin = fudge * (ymin - radius); // [esp] " " "
    zmax = fudge * (zmax + radius); // [esp] " " "
    zmin = fudge * (zmin - radius); // [esp] " " "
    fprintf(outfile,"  bounded_by {\n");
    fprintf(outfile,"//  box { <%g,%g,%g> <%g,%g,%g> }\n",xmin,ymin,zmin,xmax,ymax,zmax);
    fprintf(outfile,"    sphere { <0,0,0> 1 scale <%g,%g,%g> }\n",xmax,ymax,zmax*3);
    fprintf(outfile,"  }\n");
    fprintf(outfile,"  texture { %s_Texture }\n", union_name);
    fprintf(outfile,"}\n\n");
}

//--------------------------------------------------
static void err_exit(char *message)
{
    puts("\n\nERROR! \n");
    puts(message);
    puts("- Exiting \n");
    exit(1);
}
