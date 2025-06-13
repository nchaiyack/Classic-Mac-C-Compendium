/*********************************************************************
 * HexEdit.h
 *
 * Simple Hex Editor
 * Copyright 1993, Jim Bumgardner
 *
 * Revision History is in History.Note
 *********************************************************************/

#include "ObjectWindow.h"
#include <PrintTraps.h>
#include <stdarg.h>

#if PROFILE			// 6/15 Optional profiling support
#include <Console.h>
#include <Profile.h>
#endif

#define CREATOR			'hDmp'
#define DEFFILETYPE		'TEXT'
#define SBarSize		16
#define	GrowIconSize	14
#define	MainWIND		128
#define MaxFileRAM		32000L
#define SlushRAM		1000L
#define AllocIncr		64L
#define HeaderHeight	20
#define FooterHeight	20
#define TopMargin		3
#define BotMargin		1
#define AddrPos			12
#define HexStart		9
#define AsciiStart		59
#define AsciiSpacing	6
#define LineHeight		11
#define DescendHeight	3
#define CharWidth		6
#define HexWidth		(CharWidth*3)
#define LineNbr(a)		((a) >> 4)
#define ColNbr(a)		((a) & 0x0F)
#define CharPos(a)		(((a) << 2) + ((a) << 1))	// Multiply by 6
#define HexPos(a)		(((a) << 4) + ((a) << 1))	// Multiply by 18
#define SaveChangesALRT	10000
#define StdErrorALRT	10001
#define NoForkALRT		10002
#define StdMessageALRT	10003
#define SearchDLOG		128
#define GotoDLOG		129
#define GetFileDLOG		1401
#define MenuBaseID		128
#define UndoSTRs		128
#define MaxWindowWidth	484	// Must be multiple of 2
#define MaxWindowHeight	512	// Note - these are NOT reversed!
#define PrefResType		'prf1'
#define	PrefResID		128

enum ChunkTypes		{CT_Original, CT_Work, CT_Unwritten };
enum ForkType 		{FT_Data, FT_Resource};
enum ForkModes		{FM_Data, FM_Rsrc, FM_Smart};
enum AsciiModes		{AM_Lo, AM_Hi};
enum EditMode 		{EM_Hex, EM_Ascii};
enum EditOperation	{EO_Typing=1, EO_Paste, EO_Insert, EO_Cut, EO_Clear, EO_Delete};
enum ErrorSeverity	{ES_Message, ES_Note, ES_Caution, ES_Stop};
enum CursorNumbers	{C_Arrow, C_Watch, C_IBeam};

// Menu Resource IDs
enum	{AppleMENU = 128, FileMENU, EditMENU, FindMENU, OptionsMENU};

// Menu Item Numbers
enum	{AM_About=1};

enum	{FM_New=1,FM_Open,FM_Close,	
		 FM_Save=5, FM_SaveAs, FM_Revert,
		 FM_PageSetup=9, FM_Print,
		 FM_Quit = 12};

enum 	{EM_Undo = 1, EM_Cut = 3, EM_Copy,
		 EM_Paste, EM_Clear, EM_SelectAll=8};
		 
enum	{SM_Find = 1, SM_FindForward, SM_FindBackward, SM_GotoAddress=5};

enum	{OM_HiAscii = 1, OM_DecimalAddr, OM_Overwrite, OM_Backups};


typedef struct EditChunk {
	struct EditChunk	**prev,**next;
	Boolean				loaded;			// Flag if chunk is currently loaded
	short				type;			// 0=Orig File, 1=Work File, 2=Unwritten
	Handle				data;			// Handle to Chunk Data
	long				size;			// Size of Chunk
	long				allocSize;		// Size of allocated pointer
	long				addr;			// Start Addr in updated File
	long				filePos;		// Start Addr in actual File
	long				lastCtr;		// Use Counter
} EditChunk;

typedef struct {
	ObjectWindowRecord	oWin;			// Window Record
	ControlHandle		vScrollBar;		// Vertical Scroll Bar
	EditChunk			**firstChunk;	// File's First Chunk
	EditChunk			**curChunk;		// File's Current Chunk
	FSSpec				fsSpec,workSpec;// File Specs for Original, Work File
	FSSpec				destSpec;		// File Spec for Save, Save As
	long				fileSize;		// Total File Size
	long				fileType;		// File Type
	long				creator;		// File Creator
	unsigned long		creationDate;	// Creation Date
	long				useCtr;			// Chunk access Counter
										// Chunks are unloaded from memory based on usage
	long				totLoaded;		// Amount of bytes in Memory
	long				editOffset;		// Display Offset
	long				startSel;		// First Character of Selection
	long				endSel;			// First Character AFTER Selection
	long				lastTypePos;	// Last Typing Insertion Point
	short				refNum;			// File's Reference Number
	short				workRefNum;		// Work File's Reference Number
	short				workBytesWritten;	// Size of Work File
	short				linesPerPage;	// Lines that fit in the window
	short				editMode;		// 0=Hex, 1=Ascii
	short				fork;			// 0=data 1=resource
	short				lastNybble;		// Last Hex Edit Nibble
	Boolean				loByteFlag;		// Editing Low Byte for Hex Editor
	Boolean				dirtyFlag;		// File has been modified
} EditWindowRecord, *EditWindowPtr;

typedef struct {
	short			type;				// Type of operation
	short			reserved;
	long			startSel;			// Start of Selection
	long			endSel;				// End of Selection
	long			fileSize;			// File Size for Undo Op
	EditChunk		**undoScrap;
	EditWindowPtr	window;
} UndoRecord;

typedef struct {
	short	asciiMode;
	short	decimalAddr;
	short	backupFlag;
	short	reserved[29];
} Preferences;

extern Preferences	gPrefs;
extern UndoRecord	gUndoRec,gRedoRec;
extern EditChunk	**gScrapChunk,**gSearchChunk;
extern Boolean		gQuitFlag,gSys7Flag,gColorQDFlag;
extern MenuHandle	gAppleMenu, gFileMenu, gEditMenu;
extern short		gScrapCount;
extern THPrint		gHPrint;
extern short		gHighChar, gOverwrite;
extern DialogPtr	gSearchWin;
extern short		gSearchDir;
extern Cursor		gWatchCursor,gIBeamCursor;
extern short		gMaxHeight;

// High Level Procedures
void AskEditWindow(void);
void OpenEditWindow(FSSpec *fsSpec);
void DisposeEditWindow(WindowPtr theWindow);
Boolean CloseEditWindow(WindowPtr theWindow);
Boolean CloseAllEditWindow(WindowPtr theWindow);
void MyIdle(WindowPtr theWindow, EventRecord *er);
void MyDraw(WindowPtr theWin);
void MyProcessKey(WindowPtr theWin, EventRecord *er);
void UpdateOnscreen(WindowPtr theWindow);
void MyHandleClick(WindowPtr theWin, Point where, EventRecord *er);
void DrawPage(EditWindowPtr dWin);
void InvertSelection(EditWindowPtr dWin);
void CursorOn(WindowPtr theWin);
void CursorOff(WindowPtr theWin);

// Chunk Operatations
void LoadFile(EditWindowPtr dWin);
void UnloadFile(EditWindowPtr dWin);
void DisposeChunk(EditWindowPtr dWin, EditChunk **cc);
EditChunk **NewChunk(long size, long addr, long filePos, short type);
EditChunk **AppendChunk(EditChunk **list, EditChunk **chunk);
EditChunk **GetChunkByAddr(EditWindowPtr dWin, long addr);
short GetByte(EditWindowPtr dWin, long addr);
void LoadChunk(EditWindowPtr dWin, EditChunk **cc);
void UnloadLeastUsedChunk(EditWindowPtr dWin);
void UnloadChunk(EditWindowPtr dWin, EditChunk	**cc, Boolean writeFlag);
void RewriteAddressChain(EditChunk **fc);

// Display Operations
void ScrollToSelection(EditWindowPtr dWin, long pos, Boolean forceUpdate, Boolean centerFlag);

// Edit Operations (high level)
void UndoSelection(EditWindowPtr dWin);
void CopySelection(EditWindowPtr dWin);
void CutSelection(EditWindowPtr dWin);
void PasteSelection(EditWindowPtr dWin);
void ClearSelection(EditWindowPtr dWin);
void CopyOperation(EditWindowPtr dWin, EditChunk ***scrapChunk);

// Edit Operations (low level)
void PasteOperation(EditWindowPtr dWin, EditChunk **scrapChunk);
void RememberOperation(EditWindowPtr dWin, short opType, UndoRecord *ur);
void DeleteSelection(EditWindowPtr dWin);

// File Operations
void SaveContents(WindowPtr theWin);
void SaveAsContents(WindowPtr theWin);
void RevertContents(WindowPtr theWin);
void InsureNameIsUnique(FSSpec *tSpec);
short MyRandom(short limit);

short ErrorAlert(short severity, char *str,...);
short OSErrorAlert(short severity, char *str, short oe);

