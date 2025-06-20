/* Macintosh emulation of Unix 'access()' system call.
   Public domain by Guido van Rossum, CWI, Amsterdam (July 1987).

   This version ignores the mode flags; it assumes files can always
   be read or written when they exist.  This is more or less true,
   except on write-protected volumes and maybe in a shared file system
   situation.  Note that the Finder's 'locked' bit is ignored by
   the file system so you can still write such files from within
   an application.
   Execute permission might check the file type and return Yes
   if this is APPL, but I have no use for it right now anyway,
   so why bother. */

#include "macdefs.h"

int
access(path, mode)
	char *path;
	int mode;
{
	FileParam pb;
	char name[MAXPATH];
	
	strncpy(name, path, sizeof name);
	pb.ioNamePtr= (StringPtr) c2pstr(name);
	pb.ioVRefNum= 0;
	pb.ioFVersNum= 0;
	pb.ioFDirIndex= 0;
	if (PBGetFInfo(&pb, FALSE) != noErr) {
		errno= ENOENT;
		return -1;
	}
	return 0;
}
