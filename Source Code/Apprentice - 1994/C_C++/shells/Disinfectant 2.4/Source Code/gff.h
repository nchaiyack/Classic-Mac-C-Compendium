/*______________________________________________________________________

	gff.h - Get File or Folder Module Interface.
	
	Copyright © 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
_____________________________________________________________________*/

#ifndef __gff__
#define __gff__

extern void gff_Get (Point *where, Str255 prompt, FileFilterProcPtr fileFilter, 
	short numTypes, SFTypeList typeList, SFReply *reply, 
	short dlgID);
	
#endif
