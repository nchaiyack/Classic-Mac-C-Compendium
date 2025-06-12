/*
ffprintf.c
ffprintf(o,format,...);
Uses fprintf to print to two output streams, o[0] and o[1], usually stdout and a file. 
Any NULL stream is ignored.
It also saves and restores the port and GDevice.
Copyright (c) 1990 Denis G. Pelli
HISTORY:
9/14/90	dgp	wrote it, based on my UserPrintf(), which it replaces
8/27/92	dgp	check for 8-bit quickdraw before using GDevices.
*/
#include "VideoToolbox.h"
#include <stdarg.h>      /* for variable-number-of-argument macros */

int ffprintf(FILE *stream[2],char *format,...)
{
	va_list args;
	GDHandle oldDevice;
	int i,j;
	long value=0;
  
	Gestalt(gestaltQuickdrawVersion,&value);
	if(value>=gestalt8BitQD){
		oldDevice = GetGDevice();
		SetGDevice(GetMainDevice());
	}
	
	/* print copies to all non-NULL streams in stream[] */
	for(i=0;i<2; i++){
		va_start(args, format);
		if(stream[i] != NULL) j=vfprintf(stream[i],format,args);
	}
	va_end(args);
	
	if(value>=gestalt8BitQD)SetGDevice(oldDevice);
	return j;
}
