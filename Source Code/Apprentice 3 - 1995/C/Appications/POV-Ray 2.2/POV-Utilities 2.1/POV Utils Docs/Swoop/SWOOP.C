//---------------------------------------------------------------------------
// Swoop.c copyright 1992, Douglas Otwell, all rights reserved
//
// You may make use of this program and/or code for personal purposes only.
// Do not distribute this program or code in any modified form.  Please send
// changes, comments, suggestions, etc. to the author. I can be reached at:
// Compuserve ID # 76236,1121 or the You Can Call Me Ray graphics BBS
//
//---------------------------------------------------------------------------
// Macintosh modifications & tweaking by Eduard [esp] Schwan
// To find the changes, just search for "[esp]" in this file.
//
// Here's my wishlist for a future Swoop.c :-)
// 1. Syntax of input language changed to be more verbose and
//    human-readable.  Specifically:
//       output_type 1 | 2 | 3 should be output_type RAW | POLYRAY | POVRAY
// 2. Better error checking. No check for both -p & -r command line args,
//    no check on reading numbers in from file...
// 3. Allow '#' or '//' style comments embedded in source file.
// 4. Change hard-coded 24's and 25's to command-line settable sizes of
//    max # of vertices & max # of variables.  Then dynamically allocate
//    the arrays.  Would let this generate surfaces only a Cray could render!
// just some ideas, do with them as you wish!  Awesome program already though!
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if defined(applec) || defined(THINK_C)
#include <string.h>		// ANSI says strcmp lives here..
						// not needed for IBM PC?
// if using Macintosh, include user interface stuff
#include "Swoop.mac.h"
#else
#include <conio.h>		// IBM-PC-specific???
// This is a no-op for non-Mac Systems
#define COOPERATE
#endif


#define DBL float
#define DEG2RAD 0.01745329251994329576

#define POLYRAY	1
#define RAW			2
#define TRANSLATE	1
#define SCALE		2
#define ROTATE		3
#define LINE	1
#define SIN		2


#define		MAX_VERTICES	25 // [esp] added MAX_VERTICES declaration

typedef struct vector_struct VECTOR;
struct vector_struct { DBL x, y, z; };

typedef struct polygon_struct POLYGON;
struct polygon_struct { VECTOR vert[MAX_VERTICES]; }; // [esp] chgd to MAX_VERTICES

typedef struct variable_struct VARIABLE;
struct variable_struct {
	int	var_type;
	DBL	parm1, parm2, parm3, parm4, parm5;
};

typedef struct transform_struct TRANSFORM;
struct transform_struct {
	int	tran_type;
	DBL	x, y, z;
	int	var_x, var_y, var_z;
};

static void read_infile(void); // [esp] added forward decl. for ANSI
// vvv [esp] added static
static void process_args (int argc, char *argv[]);
static void v_copy(VECTOR *target, VECTOR *source);
static void poly_copy(POLYGON *target, POLYGON *source);
static void v_transform(VECTOR *target, VECTOR *source);
static void v_rotate(VECTOR *v, VECTOR *theta);
static void v_translate(VECTOR *v, VECTOR *t);
static void v_scale(VECTOR *v, VECTOR *s);
static float evaluate_var(int v);
// ^^^ [esp] added static

// vvv [esp] changed to pointers for later dynamic allocation
//
// [esp] this also means that all occurrances of:
//   polyN.vert[k]
// are changed to:
//   polyN->vert[k]
//
#define		MAX_VARIABLE	24
#define		MAX_TRANSFORM	24
static POLYGON		*poly0, *poly1, *poly2;
static VARIABLE		*var;
static TRANSFORM	*tran;
// ^^^ [esp] changed to pointers for later dynamic allocation

static DBL theta, theta2, x_1, y_1, z_1;

static FILE *infile, *outfile;
// vvv [esp] changed to dynamically allocate, cut down on globals
#define FILENAME_SIZE	256
static char *outfile_name, *override_name;
static char *infile_name;
static char *swoop_name;
// ^^^ [esp] changed to dynamically allocate, cut down on globals

static int   output_type, override_type;
static int	f, fnext;
static int   t, no_of_segs, total_faces, total_trans;

int main (int argc, char *argv[])
{

//	int	ch;		-- [esp] not used
//	int face;	-- [esp] not used

	printf ("Swoop version 0.1 BETA by Douglas Otwell\n\n");

// vvv [esp] added
// allocate variables 
	poly0 = malloc(sizeof(POLYGON));
	poly1 = malloc(sizeof(POLYGON));
	poly2 = malloc(sizeof(POLYGON));
	var = malloc(MAX_VARIABLE * sizeof(VARIABLE*));
	tran = malloc(MAX_TRANSFORM * sizeof(TRANSFORM*));
	outfile_name = malloc(FILENAME_SIZE);
	override_name = malloc(FILENAME_SIZE);
	infile_name = malloc(FILENAME_SIZE);
	swoop_name = malloc(80);
	if (!(tran && outfile_name && override_name && infile_name && swoop_name))
	{
		puts("Error. Can't allocate memory during initialization!");
		exit(1);
	}
// ^^^ [esp] added

// Initialize defaults
	no_of_segs = 0;
	output_type = POLYRAY;
	strcpy(outfile_name, "swoop.raw");
	strcpy(swoop_name, "swoop");

// Get command line arguments
	process_args(argc, argv);

// Get input parameters
	read_infile();

	if (override_type != 0) {
		output_type = override_type;
	}
	if (override_name[0] != '\0') {
		strcpy(outfile_name, override_name);
	}

	poly_copy(poly1, poly0);


	if ((outfile = fopen(outfile_name, "wt")) == NULL) {
		fprintf(stderr, "Cannot open output file.\n");
		exit(1);
	}
	if (output_type == POLYRAY) {
		fprintf(outfile, "define %s object {\n", swoop_name);
	}

	printf("Writing %s\n", outfile_name);

//  Major loop performed once for each face of the swoop
	for (f = 1; f <= total_faces; f++) {
		fnext = f+1;
		if (fnext > total_faces) fnext = 1;

		//      Initialize starting point for t=0
		t=0;
		v_transform(&poly1->vert[f], &poly0->vert[f]);
		v_transform(&poly1->vert[fnext], &poly0->vert[fnext]);

		if (output_type == RAW) fprintf(outfile, "%s_side%1d\n", swoop_name, f);

		for (t=1; t <= no_of_segs; t++) {

			//	Perform transforms on poly0 to produce poly2
			v_transform(&poly2->vert[f], &poly0->vert[f]);
			v_transform(&poly2->vert[fnext], &poly0->vert[fnext]);

			if (output_type == POLYRAY) {
				fprintf(outfile, "object { polygon 3,\n");
				fprintf(outfile, "   <%f, %f, %f>,\n", poly1->vert[f].x, poly1->vert[f].y, poly1->vert[f].z);
				fprintf(outfile, "   <%f, %f, %f>,\n", poly2->vert[f].x, poly2->vert[f].y, poly2->vert[f].z);
				fprintf(outfile, "   <%f, %f, %f> } +\n", poly1->vert[fnext].x, poly1->vert[fnext].y, poly1->vert[fnext].z);
				fprintf(outfile, "object { polygon 3,\n");
				fprintf(outfile, "   <%f, %f, %f>,\n", poly1->vert[fnext].x, poly1->vert[fnext].y, poly1->vert[fnext].z);
				fprintf(outfile, "   <%f, %f, %f>,\n", poly2->vert[f].x, poly2->vert[f].y, poly2->vert[f].z);
				fprintf(outfile, "   <%f, %f, %f> }", poly2->vert[fnext].x, poly2->vert[fnext].y, poly2->vert[fnext].z);

				if (t != no_of_segs || f != total_faces) fprintf(outfile, " +\n");
				else 	fprintf(outfile, "\n}\n");
			}
			else {
				if (output_type == RAW) {
					fprintf(outfile, "%f %f %f ", poly1->vert[f].x, poly1->vert[f].y, poly1->vert[f].z);
					fprintf(outfile, "%f %f %f ", poly2->vert[f].x, poly2->vert[f].y, poly2->vert[f].z);
					fprintf(outfile, "%f %f %f\n", poly1->vert[fnext].x, poly1->vert[fnext].y, poly1->vert[fnext].z);
					fprintf(outfile, "%f %f %f ", poly1->vert[fnext].x, poly1->vert[fnext].y, poly1->vert[fnext].z);
					fprintf(outfile, "%f %f %f ", poly2->vert[f].x, poly2->vert[f].y, poly2->vert[f].z);
					fprintf(outfile, "%f %f %f\n", poly2->vert[fnext].x, poly2->vert[fnext].y, poly2->vert[fnext].z);
				}
			}

			// Set end polygon to start polygon
			v_copy(&poly1->vert[f], &poly2->vert[f]);
			v_copy(&poly1->vert[fnext], &poly2->vert[fnext]);
		}

	}

	fclose(outfile);
	return 0; // [esp] added for ANSI (int fn() must return something)
}

static void process_args (int argc, char *argv[]) // [esp] added static
{
	int i;

	infile_name[0] == '\0';
	override_type = 0;
	override_name[0] = '\0';

	if (argc < 2) {
		printf ("Usage: swoop inputfile [-options]\n\n");
		printf ("Options: -p     - Polyray output\n");
		printf ("         -r     - raw triangle output\n");
		printf ("         -ofile - output file name\n");
		exit(1);
	}

	sscanf(&argv[1][0], "%s", infile_name);

	for (i = 2; i < argc; i++) {
		if (argv[i][0] == '-' || argv[i][0] == '/') {
			switch (argv[i][1]) {
				case 'p':
				case 'P':
					override_type= POLYRAY;
					break;

				case 'r':
				case 'R':
					override_type= RAW;
					break;

				case 'O':
				case 'o':
					sscanf (&argv[i][2], "%s", override_name);
					break;

				default : printf ("\nInvalid option -%c\n", argv[i][1]);
					exit (1);
			}
		}
		else {
			if (infile_name[0] == '\0') {
				strcpy (infile_name, argv[i]);
			}
			else {
				printf("Unrecognized command line parameters\n");
				exit(1);
			}
		}
	}
}


static void v_transform(VECTOR *target, VECTOR *source) // [esp] added static
{
	VECTOR work;
	int	i;

	v_copy(target, source);

	for (i=1; i<=total_trans; i++) {
		if (tran[i].var_x != 0) {
			work.x = evaluate_var(tran[i].var_x);
		}
		else {
			work.x = tran[i].x;
		}
		if (tran[i].var_y != 0) {
			work.y = evaluate_var(tran[i].var_y);
		}
		else {
			work.y = tran[i].y;
		}
		if (tran[i].var_z != 0) {
			work.z = evaluate_var(tran[i].var_z);
		}
		else {
			work.z = tran[i].z;
		}

		if (tran[i].tran_type == TRANSLATE) {
			v_translate(target, &work);
		}
		if (tran[i].tran_type == SCALE) {
			v_scale(target, &work);
		}
		if (tran[i].tran_type == ROTATE) {
			v_rotate(target, &work);
		}
	}
}

static void v_rotate(VECTOR *v, VECTOR *theta) // [esp] added static
{
	DBL x1, y1, z1;

	// rotate around x
	y1 = (v->y * cos(theta->x * DEG2RAD)) - (v->z * sin(theta->x * DEG2RAD));
	z1 = (v->y * sin(theta->x * DEG2RAD)) + (v->z * cos(theta->x * DEG2RAD));
	v->y = y1; v->z = z1;

	// rotate around y
	x1 = (v->x * cos(theta->y * DEG2RAD)) + (v->z * sin(theta->y * DEG2RAD));
	z1 = (v->z * cos(theta->y * DEG2RAD)) - (v->x * sin(theta->y * DEG2RAD));
	v->x = x1; v->z = z1;

	// rotate around z
	x1 = (v->x * cos(theta->z * DEG2RAD)) - (v->y * sin(theta->z * DEG2RAD));
	y1 = (v->x * sin(theta->z * DEG2RAD)) + (v->y * cos(theta->z * DEG2RAD));
	v->x = x1; v->y = y1;
}

static void v_translate(VECTOR *v, VECTOR *t) // [esp] added static
{
	DBL x1, y1, z1;

	x1 = v->x + t->x;
	y1 = v->y + t->y;
	z1 = v->z + t->z;

	v->x = x1; v->y = y1; v->z = z1;
}


static void v_scale(VECTOR *v, VECTOR *s) // [esp] added static
{
	DBL x1, y1, z1;

	x1 = s->x * v->x;
	y1 = s->y * v->y;
	z1 = s->z * v->z;

	v->x = x1; v->y = y1; v->z = z1;
}

static void v_copy(VECTOR *target, VECTOR *source) // [esp] added static
{
	target->x = source->x;
	target->y = source->y;
	target->z = source->z;
}

static void poly_copy(POLYGON *target, POLYGON *source) // [esp] added static
{
	int   i;

	for (i=1; i<=total_faces; i++) {
		v_copy(&target->vert[i], &source->vert[i]);
	}
}

static float evaluate_var(int v) // [esp] added static
{
	DBL increment;

	increment = (var[v].parm2 - var[v].parm1) / no_of_segs;
	return var[v].parm1 + (t * increment);
}

//
// Read input file for parameters
//
static void read_infile(void) // [esp] changed declaration to static void
{
	int vert_ctr, var_ctr, tran_ctr;

	char buffer[256], keyword[16];
//	int		var_no_in;	-- not used [esp]
	int		var_type_in;
	char	x_in[80], y_in[80], z_in[80];
	float	start, end;

	printf("Reading %s\n", infile_name);
	if ((infile = fopen(infile_name, "rt")) == NULL) {
		fprintf(stderr, "Cannot open input file.\n");
		exit(1);
	}

	vert_ctr = 0; var_ctr = 0; tran_ctr = 0;


	while (fgets(buffer, 256, infile) != NULL) {
		if (sscanf(buffer, "%s", keyword) == EOF) {
			continue;
		}
		if (strcmp(keyword, "outfile") == 0) {
			sscanf(buffer, "%s %s", keyword, outfile_name);
			continue;
		}
		if (strcmp(keyword, "name") == 0) {
			sscanf(buffer, "%s %s", keyword, swoop_name);
			continue;
		}
		if (strcmp(keyword, "output_type") == 0) {
			sscanf(buffer, "%s %d", keyword, &output_type);
			continue;
		}
		if (strcmp(keyword, "segments") == 0) {
			sscanf(buffer, "%s %d", keyword, &no_of_segs);
			continue;
		}
		if (strcmp(keyword, "vertex") == 0) {
			vert_ctr++;
			if (vert_ctr > 24) {
				fprintf(stderr, "Too many vertices specified\n");
				fclose(infile);
				exit(1);
			}
			sscanf(buffer, "%s %f %f %f",
				keyword, &poly0->vert[vert_ctr].x, &poly0->vert[vert_ctr].y, &poly0->vert[vert_ctr].z);
			continue;
		}
		if (strcmp(keyword, "var") == 0) {
			var_ctr++;
			sscanf(buffer, "%s %d %f %f", keyword, &var_type_in, &start, &end);
			var[var_ctr].var_type = LINE;
			var[var_ctr].parm1 = start;
			var[var_ctr].parm2 = end;
			continue;
		}
		if (strcmp(keyword, "translate") == 0) {
			tran_ctr++;
			if (tran_ctr > 24) {
				fprintf(stderr, "Too many transformations specified\n");
				fclose(infile);
				exit(1);
			}
			tran[tran_ctr].tran_type = TRANSLATE;

			sscanf(buffer, "%s %s %s %s", keyword, x_in, y_in, z_in);
			if (x_in[0] == '&') {
				tran[tran_ctr].var_x = atoi(&x_in[1]);
			}
			else {
				tran[tran_ctr].var_x = 0;
				tran[tran_ctr].x = atof(x_in);
			}
			if (y_in[0] == '&') {
				tran[tran_ctr].var_y = atoi(&y_in[1]);
			}
			else {
				tran[tran_ctr].var_y = 0;
				tran[tran_ctr].y = atof(y_in);
			}
			if (z_in[0] == '&') {
				tran[tran_ctr].var_z = atoi(&z_in[1]);
			}
			else {
				tran[tran_ctr].var_z = 0;
				tran[tran_ctr].z = atof(z_in);
			}
			continue;
		}
		if (strcmp(keyword, "rotate") == 0) {
			tran_ctr++;
			if (tran_ctr > 24) {
				fprintf(stderr, "Too many transformations specified\n");
				fclose(infile);
				exit(1);
			}
			tran[tran_ctr].tran_type = ROTATE;

			sscanf(buffer, "%s %s %s %s", keyword, x_in, y_in, z_in);
			if (x_in[0] == '&') {
				tran[tran_ctr].var_x = atoi(&x_in[1]);
			}
			else {
				tran[tran_ctr].var_x = 0;
				tran[tran_ctr].x = atof(x_in);
			}
			if (y_in[0] == '&') {
				tran[tran_ctr].var_y = atoi(&y_in[1]);
			}
			else {
				tran[tran_ctr].var_y = 0;
				tran[tran_ctr].y = atof(y_in);
			}
			if (z_in[0] == '&') {
				tran[tran_ctr].var_z = atoi(&z_in[1]);
			}
			else {
				tran[tran_ctr].var_z = 0;
				tran[tran_ctr].z = atof(z_in);
			}
			continue;
		}
		if (strcmp(keyword, "scale") == 0) {
			tran_ctr++;
			if (tran_ctr > 24) {
				fprintf(stderr, "Too many transformations specified\n");
				fclose(infile);
				exit(1);
			}

			tran[tran_ctr].tran_type = SCALE;

			sscanf(buffer, "%s %s %s %s", keyword, x_in, y_in, z_in);
			if (x_in[0] == '&') {
				tran[tran_ctr].var_x = atoi(&x_in[1]);
			}
			else {
				tran[tran_ctr].var_x = 0;
				tran[tran_ctr].x = atof(x_in);
			}
			if (y_in[0] == '&') {
				tran[tran_ctr].var_y = atoi(&y_in[1]);
			}
			else {
				tran[tran_ctr].var_y = 0;
				tran[tran_ctr].y = atof(y_in);
			}
			if (z_in[0] == '&') {
				tran[tran_ctr].var_z = atoi(&z_in[1]);
			}
			else {
				tran[tran_ctr].var_z = 0;
				tran[tran_ctr].z = atof(z_in);
			}
			continue;
		}

		fprintf(stderr, "Unrecognized keyword: %s\n", keyword);
		fclose(infile);
		exit(1);
	}

	fclose(infile);
	total_faces = vert_ctr;
	total_trans = tran_ctr;

	if (no_of_segs == 0) {
		fprintf(stderr, "Segments must be greater than zero");
		exit(1);
	}

}
