/* MAC STDWIN -- GET ARGC/ARGV. */

/* Copy the arguments passed from the finder into argc/argv.
   Neither MPW nor THINK C does this, making argc/argv pretty
   useless.  By using winitargs(&argc, &argv) you get the arguments
   that you expect.  When called to print, a "-p" flag is passed
   before the first argument.
*/

#include "macwin.h"
#ifdef MPW
#include <SegLoad.h>
#endif
#ifdef THINK_C_PRE_5_0
#include <SegmentLdr.h>
#endif

void
wargs(pargc, pargv)
	int *pargc;
	char ***pargv;
{
	L_DECLARE(argc, argv, char *);
	char apname[256];
	char buf[256];
	short aprefnum;
	Handle apparam;
	short message;
	short count;
	short i;
	
	GetAppParms(apname, &aprefnum, &apparam);
#ifndef CLEVERGLUE
	PtoCstr(apname);
#endif
	L_APPEND(argc, argv, char *, strdup(apname));
	
	CountAppFiles(&message, &count);
	if (message == appPrint) { /* Must have braces around L_*! */
		L_APPEND(argc, argv, char *, "-p");
	}
	
	for (i = 1; i <= count; ++i) {
		AppFile thefile;
		GetAppFiles(i, &thefile);
		fullpath(buf, thefile.vRefNum,
			p2cstr((char*)&thefile.fName));
		L_APPEND(argc, argv, char *, strdup(buf));
	}
	
	L_APPEND(argc, argv, char *, NULL);
	*pargc = argc - 1;
	*pargv = argv;
}
