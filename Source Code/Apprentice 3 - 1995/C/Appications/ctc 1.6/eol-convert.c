/* eol-convert.c
 *
 * converts the EOL in TEXT files to the chosen system
 * History...
 * RMF	Deallocated memory on errors, and closed file if error allocating memory.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <Gestalt.h>
#include <Memory.h>
#include "ctc.h"


#define STRING_TERMINATOR	('\0')
#define CR	((unsigned char) 0x0d)
#define LF	((unsigned char) 0x0a)

int process (FILE *path, char *EOL, char *buffer)
{	char *start, *ptr;

	start = buffer;
	for (ptr = buffer; *ptr; )
		switch (*ptr) {
			default:	ptr++;	break;
			
			case CR:	/* MAC or DOS end of line has been found */
				*ptr = STRING_TERMINATOR;
				fprintf (path, "%s%s", start, EOL);
				ptr++;
				if (*ptr == LF) ptr++;	/* skip the LF - eg. MSDOS */
				start = ptr;
				break;
				
			case LF:	/* UNIX end of line has been found */
				*ptr = STRING_TERMINATOR;
				fprintf (path, "%s%s", start, EOL);
				ptr++;
				start = ptr;
				break;
			}
			
	if (start < ptr)	/* write the last line, maybe an EOL */
		fprintf (path, "%s", start);
	return 0;
}	/* End of () */


void ConvertFile (char *thefile, short EOLmode)
{	FILE *path;
	long int length, qty;
	Handle h = nil;			/* RMF - Added support for System 7 tempory memory */
	char *buffer, EOL[6];
	Boolean HasTempMem = false;
	long resp;
	OSErr err;

	switch (EOLmode) {
		case DOS:	strcpy (EOL, "\r\n\0"); break;
		case MAC:	strcpy (EOL, "\r\0"); break;
		case UNIX:	strcpy (EOL, "\n\0"); break;
		default:	return;
		}
	
	if ( Gestalt(gestaltOSAttr, &resp) == noErr) {
	 	HasTempMem = (resp & ( 1 << gestaltTempMemTracked) ) && 
	 		(resp & ( 1 << gestaltTempMemSupport) ) && 
	 		(resp & ( 1 << gestaltRealTempMemory) );
			}
				
	path = fopen (thefile, "rb");
	if (!path) {
		SysBeep(0);
		return;		/* could not open the file for read */
		}
	/*
	 * find the length of the file 
	 * and allocate a buffer for its contents
	 */
	fseek (path, (long int) 0, SEEK_END);
	length = ftell (path);
	fseek (path, (long int) 0, SEEK_SET);
	
	h = nil;
	if (HasTempMem) h = TempNewHandle((Size) length+1, &err);
	if (!h) h = NewHandle ((Size) length+1);
	
	if (!h) goto bailopen;			/* RMF - not enough memory for the buffer */
	buffer = (char *) *h;
	
	buffer[length] = STRING_TERMINATOR;	/* just in case */
	
	/*
	 * read the file into the buffer
	 */
	qty = fread ( (void *) buffer, (size_t) length, (size_t) 1, path);
	
	/*
	 * re-open the file for writing
	 * RMF - NOTE: File type and creator get set to: '????' by this routine...
	 */
	freopen (thefile, "wb", path);
	if (!path) goto bailopen;		/* could not open the file for write */
	
	fseek (path, (long int) 0, SEEK_SET);
	/*
	 * do the conversion of the EOL characters
	 */
	process (path, EOL, buffer);
	
bailopen:		/* RMF */
	fclose (path);
	if (h) DisposeHandle(h);
}	/* End of () */
