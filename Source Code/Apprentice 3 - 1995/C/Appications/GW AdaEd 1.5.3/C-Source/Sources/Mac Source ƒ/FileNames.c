/* FileNames.c
 *
 * Unit to parse file names.  It receives a file name (which may
 * include path, name and extension) and based on options passed
 * in argument 'options' copies over to 'results' some parts of
 * the name
 *
 * manuel a. perez
 *
 */
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */


#include <ctype.h>

#include "FileNames.h"

/* Constants */
#define chExt '.'
#define chPath ':'

/* Prototype */
static	void	copyStr(const Str255 fstr, int fidx,  int fend,
						Str255 tstr, int *tidx);

#ifdef TESTING
#define TESTING

#include <stdio.h>
#include <string.h>

void test(Str255 string, short options, Str255 correct);
static int testNo = 0;
static Boolean printAll = false;
main()
{

	testNo = 0;
	printAll = true;

	/* First round of tests uses a test string with all parts */

	test("\pDisk:Folder:Name.ext", flPath, "\pDisk:Folder:");
	test("\pDisk:Folder:Name.ext", flName, "\pName");
	test("\pDisk:Folder:Name.ext", flExtension, "\p.ext");

	test("\pDisk:Folder:Name.ext", flPath+flName, "\pDisk:Folder:Name");
	test("\pDisk:Folder:Name.ext", flName+flExtension, "\pName.ext");
	test("\pDisk:Folder:Name.ext", flPath+flExtension, "\pDisk:Folder:.ext");

	test("\pDisk:Folder:Name.ext", flPath+flName+flExtension,
		 "\pDisk:Folder:Name.ext");

	/* Second round of tests : no path */

	test("\pName.ext", flPath, "\p");
	test("\pName.ext", flName, "\pName");
	test("\pName.ext", flExtension, "\p.ext");

	test("\pName.ext", flPath+flName, "\pName");
	test("\pName.ext", flName+flExtension, "\pName.ext");
	test("\pName.ext", flPath+flExtension, "\p.ext");

	test("\pName.ext", flPath+flName+flExtension,
		 "\pName.ext");

	/* Third round of tests : no name */

	test("\pDisk:Folder:.ext", flPath, "\pDisk:Folder:");
	test("\pDisk:Folder:.ext", flName, "\p");
	test("\pDisk:Folder:.ext", flExtension, "\p.ext");

	test("\pDisk:Folder:.ext", flPath+flName, "\pDisk:Folder:");
	test("\pDisk:Folder:.ext", flName+flExtension, "\p.ext");
	test("\pDisk:Folder:.ext", flPath+flExtension, "\pDisk:Folder:.ext");

	test("\pDisk:Folder:.ext", flPath+flName+flExtension,
		 "\pDisk:Folder:.ext");

	/* Fourth round of tests : no extension */

	test("\pDisk:Folder:Name", flPath, "\pDisk:Folder:");
	test("\pDisk:Folder:Name", flName, "\pName");
	test("\pDisk:Folder:Name", flExtension, "\p");

	test("\pDisk:Folder:Name", flPath+flName, "\pDisk:Folder:Name");
	test("\pDisk:Folder:Name", flName+flExtension, "\pName");
	test("\pDisk:Folder:Name", flPath+flExtension, "\pDisk:Folder:");

	test("\pDisk:Folder:Name", flPath+flName+flExtension,
		 "\pDisk:Folder:Name");

	/* Fifth round of tests : path alone */

	test("\pDisk:Folder:", flPath, "\pDisk:Folder:");
	test("\pDisk:Folder:", flName, "\p");
	test("\pDisk:Folder:", flExtension, "\p");

	test("\pDisk:Folder:", flPath+flName, "\pDisk:Folder:");
	test("\pDisk:Folder:", flName+flExtension, "\p");
	test("\pDisk:Folder:", flPath+flExtension, "\pDisk:Folder:");

	test("\pDisk:Folder:", flPath+flName+flExtension, "\pDisk:Folder:");

	/* Sixth round of tests : name alone */

	test("\pName", flPath, "\p");
	test("\pName", flName, "\pName");
	test("\pName", flExtension, "\p");

	test("\pName", flPath+flName, "\pName");
	test("\pName", flName+flExtension, "\pName");
	test("\pName", flPath+flExtension, "\p");

	test("\pName", flPath+flName+flExtension, "\pName");

	/* Seventh round of tests : extension alone */

	test("\p.ext", flPath, "\p:");
	test("\p.ext", flName, "\p");
	test("\p.ext", flExtension, "\p.ext");

	test("\p.ext", flPath+flName, "\p");
	test("\p.ext", flName+flExtension, "\p.ext");
	test("\p.ext", flPath+flExtension, "\p.ext");

	test("\p.ext", flPath+flName+flExtension, "\p.ext");

	printf("\nCompleted %d tests.\n", testNo);
}

void test(Str255 string, short options, Str255 correct)
{
#define boolChar(x)  ((x) ? 'T' : 'F')

	Str255 results;
	short len;
	Boolean keepName = options & flName;
	Boolean keepPath = options & flPath;
	Boolean keepExt  = options & flExtension;
	Boolean failed;

	testNo++;
	ParseFile(string, options, results);
	len = results[0] < correct[0] ? results[0] : correct[0];
	failed = (strncmp((char *)&results[1], (char *)&correct[1], len) != 0);

	if ((printAll) || (failed)) {

		printf("\n\n******* Test No. # %d *******\n", testNo);
		printf("Test String: %#s\n", string);
		printf("    Options: (Path = %c, Name = %c, Ext = %c)\n",
			 boolChar(keepPath), boolChar(keepName), boolChar(keepExt));
		printf("    Results: %#s   (%s)\n", results,
			(failed ? "Failure" : "Success"));
		printf("   Expected: %#s\n", correct);
	}
}


#endif


void	Uppercase(Str255 name)
{
register int i;

	for (i = 1; i <= name[0]; i++)
		if (islower(name[i]))
			name[i] = toupper(name[i]);
}

void	Lowercase(Str255 name)
{
register int i;

	for (i = 1; i <= name[0]; i++)
		if (isupper(name[i]))
			name[i] = tolower(name[i]);
}

void	ParseFile(const Str255 file, short options, Str255 results)
{
short len;
register int iFrom, idx;
int iTo;
Boolean keepName = options & flName;
Boolean keepPath = options & flPath;
Boolean keepExt  = options & flExtension;

Boolean	haveName;
short	startName;

Boolean	havePath;
short	startPath;

Boolean	haveExtension;
short	startExtension;


	len = file[0];
	iFrom = 1;
	iTo = 1;

	// Does 'file' have an extension in it?  Only if
	// while moving from the end of the string backwards
	// we find a chExt before we find a chPath
	haveExtension = false;
	haveName = false;
	havePath = false;
	startExtension = len+1;	// end of string, move it back if any
	startName = 1;			// beginning of string, move it forward
	startPath = 1;			// should always be beginning of string

	for (idx = len; idx > 0; idx--) {
		if (file[idx] == chExt && !haveExtension && !havePath) {
			haveExtension = true;
			startExtension = idx;
		}
		else if (file[idx] == chPath && !havePath) {
			havePath = true;
			haveName = (idx != startExtension - 1);		// empty name
			startName = idx + 1;
			break;
		}
	}


	// Now we are ready to do some work... check options
	// passed in by user and c
	if (keepPath) {
		copyStr(file, startPath, startName-1, results, &iTo);
	}

	if (keepName) {
		copyStr(file, startName, startExtension-1, results, &iTo);
	}
	
	if (keepExt) {
		copyStr(file, startExtension, len, results, &iTo);
	}
	
	results[0] = iTo-1;
}


static	void	copyStr(const Str255 fstr, int fstart, int fend,
						Str255 tstr, int *tidx)
{
register int i;

	for (i = fstart; i <= fend;)
		tstr[(*tidx)++] = fstr[i++];
}
