/*********************************************************************

	miniedit.h
	
	header file for Miniedit
	
	Copyright (c) 1989 Symantec Corporation.  All rights reserved.
	
*********************************************************************/

#define windowID 		128
#define ErrorAlert		256
#define	AdviseAlert		257

/* resource IDs of menus */
#define appleID			128
#define fileID			129
#define editID			130

/* Edit menu command indices */
#define undoCommand 	1
#define cutCommand		3
#define copyCommand		4
#define pasteCommand	5
#define clearCommand	6

/* Menu indices */
#define appleM			0
#define fileM			1
#define editM			2

#define fmNew		1
#define fmOpen		2
#define fmClose		4
#define fmSave		5
#define fmSaveAs	6
#define fmRevert	7
#define fmPageSetUp	9
#define fmPrint		10
#define fmQuit		12

#define aaSave		1
#define aaDiscard	2
#define aaCancel	3

#define SBarWidth	15


int CantOpen (void);
int SetUpCursors(void);
int SetUpMenus(void);
int MainEvent(void);
int DoMouseDown (int windowPart, WindowPtr whichWindow, EventRecord *myEvent);
int DoCommand(long mResult);
int MaintainCursor(void);
int MaintainMenus(void);
