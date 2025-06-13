/* resource IDs of menus */
#define appleID			1
#define fileID			2
#define editID			3
#define monitorID		4

#define appleM			0
#define fileM			1
#define editM			2
#define monitorM		3

#define fmNew			1
#define fmOpen			1
#define fmClose		3
#define fmSave			4
#define fmSaveAs		4
#define fmPageSetUp	6
#define fmPrint		7
#define fmQuit			9

#define emUndo 		1
#define emCut			3
#define emCopy		4
#define emPaste		5
#define emClear		6


#define WarningDlg		128
#define AboutDlg		129

pascal OSErr  MyHandleODoc (AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefCon);
pascal OSErr  MyHandlePDoc (AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefCon);
pascal OSErr  MyHandleOApp (AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefCon);
pascal OSErr  MyHandleQuit (AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);	
OSErr	MyGotRequiredParams (AppleEvent *theAppleEvent);
Boolean	AppleEventsInstalled (void);

void SetUpMenus(void);
void MainEvent(void);

void DoCommand(long mResult);
void DoFile(short item);
void DoMouseDown(short windowPart, WindowPtr whichWindow, EventRecord *myEvent);
void UpdateWindow(WindowPtr whichWindow);

void	DoClose(void);
void	DoOpen( FSSpec *myFSSpec );
void	OpenFile( FSSpec *myFSSpec );
void	RecurseDir(long dirIDToSearch);

