/* error.c */

#include "error.h"
#include "windows.h"

void myError( char *errMsg, Boolean fatal )
{
	SetPortToWindow();
	ErrorDLOG( errMsg, fatal );
}