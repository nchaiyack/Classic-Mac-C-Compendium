#include "main.h"


/***************************
** Error()
**
** Error displaying routine.
****************************/

void Error(Str255 s1, Str255 s2, Str255 s3, Str255 s4)
{
	DialogPtr errorDlog;
	GrafPtr savedPort;
	short int itemHit = -9;
	
	GetPort( &savedPort);
	ParamText(s1, s2, s3, s4);
	errorDlog = GetNewDialog(ErrorDlogID, NULL, (void*)(-1) );
	if (errorDlog == NULL) {SysBeep(5);return;}
	SelectWindow((WindowPtr)errorDlog);
	while (itemHit != ErrorOKButt)
		ModalDialog( NULL, &itemHit);
	DisposDialog(errorDlog);
	errorDlog = NULL;
	SetPort(savedPort);    /* restore the old port */
} /* DoAbout() */
