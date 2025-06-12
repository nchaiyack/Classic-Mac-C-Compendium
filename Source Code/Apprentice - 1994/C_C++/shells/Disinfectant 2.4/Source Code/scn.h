/*______________________________________________________________________

	scn.h - Volume Scanning Module Interface.
	
	Copyright © 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
_____________________________________________________________________*/

#ifndef __scn__
#define __scn__


/* The following structure is used to maintain a list of folder names.  
	
	When the caller-supplied doFile routine is called, it is passed a
	handle to the head of the list.  The list contains all the 
	folder names in the current path, in reverse order (lowest-level 
	folder name to highest-level folder name).  The list is maintained as 
	a push-down stack:  When a new folder is encountered it is added to the 
	head of the list, and it is removed when the scan of that folder is 
	complete. */

typedef struct scn_FListEl {
	struct scn_FListEl	**next;			/* handle to next folder name */
	Str255					name;				/* folder name */
	char						accessRights;	/* access rights - see IM V-391 */
} scn_FListEl, *scn_FListElPtr, **scn_FListElHandle;

typedef Boolean (*scn_DoFilePtr)(CInfoPBRec *pBlock,
	scn_FListElHandle folderList, long RefCon, Boolean mfs);

typedef Boolean (*scn_CheckCancelPtr)(void);

extern Boolean scn_Vol (short volRefNum, long dirID, 
	Str255 fName, short fVRefNum, scn_DoFilePtr doFile, 
	long refCon, scn_CheckCancelPtr checkCancel, 
	Rect *fldNameRect, Rect *fleNameRect, Rect *thrmRect, 
	short nameFont, short nameSize);
extern void scn_Update (Rect *thrmRect);


#endif