/*
	Sponge2.c - by Stephen Coy, CIS: 70413,136

	create a recursive sponge from spheres
	
	Usage:
		Sponge2 N outfilename
			where:
				N = recursion depth (0 to 3)
				outfilename = output text file to create

	Modified from Sponge 2.0 by Eduard [esp] Schwan 12/16/93
		- Now can output POV-Ray 2.0 output
		- Adds textures to different levels
		- Writes to output file (name passed as 2nd parm)
*/

#include <stdio.h>
#include <stdlib.h>	/* for atoi */
#include <string.h> /* for strcpy */

#if defined(applec) || defined(THINK_C)
// if using Macintosh, include user interface stuff
#include "sponge.mac.h"
#else
// This is a no-op for non-Mac Systems
#define COOPERATE
#endif

#define MAX_LEVEL       (2)
#define RT_DEFAULT		0
#define RT_POV_RAY		1

static	int     max_level = MAX_LEVEL;
static	int     rt_current = RT_POV_RAY;
static	long	sponge_counter;
static	FILE	*outfile;

void foo(double x, double y, double z, double rad, int level);
void write_sponge_atom(double x, double y, double z, double rad, int level);
void write_header(int maxlevel);
void write_trailer(void);


/* --------------------------------------- */
main(int ac, char **av)
{
	char	outfname[64];

	if(ac >= 2)
	{
		max_level = atoi(av[1]);
		if (max_level > 3)
			max_level = MAX_LEVEL;
	}

	if (ac == 3)
		strcpy(outfname, av[2]);
	else
		sprintf(outfname, "Sponge%d.inc", max_level);

	outfile = fopen(outfname, "w");
	if (!outfile)
		outfile = stdout;

	write_header(max_level);
	sponge_counter = 0;

	foo(0.0, 0.0, 0.0, 1.0, 0);

	write_trailer();

	if ((outfile != NULL) && (outfile != stdout))
		fclose(outfile);
} /* main */

/* --------------------------------------- */
void foo(double x, double y, double z, double rad, int level)
{
	double  offset;

	/* spit out current level */
/*	fprintf(outfile, "\n  // Level %d\n",level); -- [esp] debug */
	write_sponge_atom(x,         y,        z,       rad, level);
	write_sponge_atom(x+rad*2,   y,        z,       rad, level);
	write_sponge_atom(x,         y+rad*2,  z,       rad, level);
	write_sponge_atom(x,         y,        z+rad*2, rad, level);
	write_sponge_atom(x-rad*2,   y,        z,       rad, level);
	write_sponge_atom(x,         y-rad*2,  z,       rad, level);
	write_sponge_atom(x,         y,        z-rad*2, rad, level);

	level++;
	if(level > max_level)
		return;

	offset = rad * 2.0;
	rad /= 3.0;
	foo(x+offset, y, z+offset, rad, level);
	foo(x+offset, y+offset, z+offset, rad, level);
	foo(x, y+offset, z+offset, rad, level);
	foo(x-offset, y+offset, z+offset, rad, level);
	foo(x+offset, y-offset, z+offset, rad, level);
	foo(x-offset, y-offset, z+offset, rad, level);
	foo(x-offset, y, z+offset, rad, level);
	foo(x, y-offset, z+offset, rad, level);

	foo(x+offset, y, z-offset, rad, level);
	foo(x+offset, y+offset, z-offset, rad, level);
	foo(x, y+offset, z-offset, rad, level);
	foo(x-offset, y+offset, z-offset, rad, level);
	foo(x+offset, y-offset, z-offset, rad, level);
	foo(x-offset, y-offset, z-offset, rad, level);
	foo(x-offset, y, z-offset, rad, level);
	foo(x, y-offset, z-offset, rad, level);

	foo(x+offset, y+offset, z, rad, level);
	foo(x-offset, y+offset, z, rad, level);
	foo(x+offset, y-offset, z, rad, level);
	foo(x-offset, y-offset, z, rad, level);
} /* foo */

/* --------------------------------------- */
void write_header(int maxlevel)
{
	switch (rt_current)
	{
		int	k;

		case RT_DEFAULT:
			break;
		case RT_POV_RAY:
			fprintf(outfile, "// Persistence of Vision 2.0 Source file\n");
			fprintf(outfile, "// Sponge%d.inc - Level %d Recursive Sponge\n",maxlevel,maxlevel);
			fprintf(outfile, "//\n\n");
			for (k=0; k<=maxlevel; k++)
			{
				fprintf(outfile, "#declare SpongeAtom_Tex%d = texture\n", k);
				fprintf(outfile, "{\n");
				fprintf(outfile, "  pigment { color red 1 }\n");
				fprintf(outfile, "  finish  { specular 0.7 roughness 0.01 }\n");
				fprintf(outfile, "}\n");
			}
			fprintf(outfile, "\n// You may define the atomic shape to be anything:\n");
			fprintf(outfile, "#declare SpongeAtomShape = sphere { <0, 0, 0> 1.0 }\n\n");
			fprintf(outfile, "#declare SpongeShape = union\n{\n");
			break;
	}
} /* write_header */

/* --------------------------------------- */
void write_trailer(void)
{
	switch (rt_current)
	{
		case RT_DEFAULT:
			break;
		case RT_POV_RAY:
			fprintf(outfile, "} // union\n");
			fprintf(outfile, "\n// %ld atoms generated\n", sponge_counter);
			break;
	}
} /* write_trailer */

/* --------------------------------------- */
void write_sponge_atom(double x, double y, double z, double rad, int level)
{
	sponge_counter++;
	if ((sponge_counter % 10) == 0)
	{
		printf("Now at atom # %5ld\n",sponge_counter);
		COOPERATE
	}

	switch (rt_current)
	{
		case RT_DEFAULT:
			fprintf(outfile,
				"  sphere { center %.4f %.4f %.4f radius %.4f }\n",
				x, y, z, rad);
			break;
		case RT_POV_RAY:
			fprintf(outfile,
				"  object { SpongeAtomShape texture { SpongeAtom_Tex%d } scale <%g, %g, %g> translate <%g, %g, %g> }\n",
				level, rad, rad, rad, x, y, z);
			break;
	}

} /* write_sponge_atom */
