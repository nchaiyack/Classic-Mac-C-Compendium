/* MacUtilities.c */
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "MacUtilities.h"
#include "MacMemory.h"

#include "AdaFileTypes.h"

#define EXTENSION ".options"
#define NARGS		25

static char *argv[NARGS+1];
static char argbuf[256];


#ifdef TEST
#define TEST

#include <InitMacintosh.h>
#ifndef __FSPCOMPAT__
#include "FSpCompat.h"
#endif
#include "SFGetFolder.h"

main()
{
FSSpec folder, file;
long count, idx;

	InitMacintosh(1);
	FSMakeFSSpecCompat(0, 0, "\pMain:6-Extensiones del Sistema:", &folder);
	count = CountItemsInFolder(&folder);
	for (idx = 1; idx <= count; idx++) {
		GetItemInFolder(idx, &folder, &file);
		if (ItemIsFolder(file))
			printf("%ld: %#s\n", idx, file.name);
		else {
			Str255 fullPath;

			if (GetFullPath(file, &fullPath))
				printf("%ld: %#s\n", idx, fullPath);
			else
				printf("%ld: Folder %#s\n", idx, file.name);
		}
	}
	
	while (!Button());
}
#endif

int	getoptions(char *prog, char ***av)
{
char buff[BUFSIZ];
FILE *fp;
int i;
int argc = 0;
short len;

	fp = fopen(prog, "r");
	if (fp) {

		for (i = 0; fgets(buff, BUFSIZ, fp); i++) {
			len = strlen(buff);
			if (buff[len-1] == '\n')
				buff[len-1] = '\0';			// get rid of \n
			argv[i] = mmalloc(strlen(buff)+1);
			strcpy(argv[i], buff);
		}			
		fclose(fp);

		// finished reading options, now remove them
		remove(prog);

		*av = argv;
		argc = i;
		return argc;
	}
	else {
		argv[0] = mmalloc(strlen("noname")+1);
		strcpy(argv[0], "noname");

		return 1;
	}
}

