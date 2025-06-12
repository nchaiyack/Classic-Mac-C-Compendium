/*______________________________________________________________________

	mssg.h - Virus Message Module Interface.
	
	Copyright © 1988, 1989, 1990, Northwestern University.
_____________________________________________________________________*/


#ifndef __mssg__
#define __mssg__

#ifndef __scn__
#include "scn.h"
#endif

extern short mssg_LookupTag (short repLine);
extern void mssg_ClearTags (void);
extern void mssg_M0 (short strInd);
extern void mssg_M1 (short strInd, Str255 p0);
extern void mssg_M2 (short strInd, Str255 p0, Str255 p1);
extern void mssg_M2_Tag (short strInd, Str255 p0, Str255 p1, short tag);
extern void mssg_Continue (short strInd, short msgNum);
extern void mssg_Blank (void);
extern void mssg_PrintFileName (scn_FListElHandle folderList, 
	CInfoPBRec *pBlock, short tag);
extern void mssg_CheckAccess (char accessRights, 
	scn_FListElHandle folderList, CInfoPBRec *pBlock);
extern void mssg_Begin (Boolean disinfect, long dirID, Str255 fName, 
	short fVRefNum, short volRefNum, short counterTop, short counterRight);
extern void mssg_End (Boolean disinfect, 
	Boolean canceled, Boolean *infected, Boolean *sysInfected);
extern void mssg_BumpCounter (short counter);
extern void mssg_ClearCounters (short counterTop, short counterRight);
extern void mssg_BadDisk (void);


#endif