#ifndef __FSPCOMPAT__
#include "FSpCompat.h"
#endif

#if defined(FabSystem7orlater)

#define	FabStandardGetFile	StandardGetFile
#define	FabStandardPutFile	StandardPutFile

#else

#include	"Utils6.h"

#endif

enum
{
kCacheMask   = 0x0010,
kNoCacheMask = 0x0020
};


//#if GENERATING68K
//#pragma parameter MyNumToString(__D0,__A0)
//extern pascal void MyNumToString(long theNum,Str255 theString)
// TWOWORDINLINE(0x4267, 0xA9EE);
//#else
#define	MyNumToString	NumToString
//#endif

/* WantThisHandleSafe locks high the handle and returns the previous state */

#if GENERATING68K
#pragma parameter __D0 WantThisHandleSafe(__A0)
extern pascal SignedByte WantThisHandleSafe(Handle myH) = {0xA069, 0x1F00, 0xA064, 0xA029, 0x101F};
#else
SignedByte WantThisHandleSafe(Handle myH);
#endif

/* ClearHiliteMode clears the HiliteMode bit with BCLR */

/*pascal void ClearHiliteMode(void) = {0x08B8, 0x0007, 0x0938};
*/

#define numOfLongs(s)	(((s)>>2) - 1L)

#if GENERATING68K
#pragma parameter MyZeroBuffer(__A0,__D1)
extern pascal void MyZeroBuffer(long *p, long s) = {0x7000, 0x20C0, 0x51C9, 0xFFFC };

#pragma parameter MyFillBuffer(__A0,__D1,__D0)
extern pascal void MyFillBuffer(long *p, long s, long filler) = {0x20C0, 0x51C9, 0xFFFC };

#pragma parameter __D0 mySwap(__D0)
extern pascal long mySwap(long s) = {0x4840 };
#else
void MyZeroBuffer(long *p, long s);
void MyFillBuffer(long *p, long s, long filler);
long mySwap(long s);
#endif


#define toMenu(m, i)	(((long)m << 16) + i)
#define	CHARFROMMESSAGE(mess)	(*(((unsigned char *)(&(mess))) + 3))
#define ISAPPWINDOW(w)	(((WindowPeek) w)->windowKind >= userKind)
#define ISDAWINDOW(w)	(((WindowPeek) w)->windowKind < 0)
#define isMovableModal(w)	(GetWVariant(w) == movableDBoxProc)

#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define MIN(a, b)	((a) < (b) ? (a) : (b))

/* strings */
enum simplestrings {
kSTR_NOOPENORPRINT = -16397,
kSTR_ApplicationName = -16396
};

/* Menu list resource IDs */
enum menures {
kRes_Menu_Apple = 128,
kRes_Menu_File,
kRes_Menu_Edit
};

enum edititems {
kMItem_Undo = 1,
kMItem_Cut = 3,
kMItem_Copy,
kMItem_Paste,
kMItem_Clear,
kMItem_Select_All
};

enum {
kUndoKey = 0x7A10,
kCutKey = 0x7810,
kCopyKey = 0x6310,
kPasteKey = 0x7610,
kHelpKey = 0x7205,
kFwdDel = 0x757F,
kHomeKey = 0x7301,
kEndKey = 0x7704,
kPgUpKey = 0x740B,
kPgDnKey = 0x790C,
kClearKey = 0x471B,
kEscapeKey = 0x351B,
kEnterKey = 0x4C03
};

/* for KeyState */
enum {
kCommandKeyCode = 55,
kShiftKeyCode,
kCapsLockKeyCode,
kOptionKeyCode,
kControlKeyCode
};

enum {
kWantRestart = false,
kWantShutdown = true
};

enum {
kCreat = 'FABR',
kAEAlert = 'ALRT'
};

enum {
kSTR_RegDECALLOWED = 302,
kSTR_EMAILADDR = 320,
kSTR_WWWURL = 321,
kSTR_MAILTOURL = 322,
kDLOG_Register = 300,
kDLOG_MovableAbout = 301,
kDLOG_MovableCredits = 302,
kDITLItem_AboutPICT = 3,
kALRT_GENERICERROR = 130,
kALRT_OLDSYSTEM = 131,
kALRT_AEPROCESSERR = 134,
kALRT_INITWINDSERROR = 136,
kALRT_AEINSTALLEVTERR = 137,
kALRT_REGCANNOTREPLACE = 201,
kALRT_REGSUCCESSSAVED = 202,
kALRT_REGDISKFULL = 203,
kALRT_REGUNKNOWNWRITEERROR = 204,
kALRT_REGCOULDNOTOPENFORWRITING = 205,
kALRT_REGCOULDNOTCREATE = 206,
kALRT_WOWTHATMANYCOPIES = 207,
kALRT_PLEASEDISMISSMOVABLEMODAL = 500,
kALRT_DISMISSORDRAGANDDROP = 501,
kALRT_INTERNETCONFIG = 600,
kALRT_ICMISSINGHELPER = 601,
kALRT_SEPARATEREGISTERAPP = 602,
kALRT_SHAREWAREREMINDER = 603
};

enum {
kItemUserName = 3,
kItemCompany,
kItemAddress,
kItemCity,
kItemState,
kItemZIP,
kItemCountry,
kItemE_mail,
kItemPhone,
kItemFAX,
kItemQuantity,
kItemCopyFrom,
kItemComments,
kItemTotals = 27
};

OSErr AddSTRRes2Doc(FSSpec *theDoc, OSType myFcrea, OSType myFtype, short STRid, ScriptCode lScript);
OSErr AddSTRHand2Doc(FSSpec *theDoc, OSType myFcrea, OSType myFtype, StringHandle msgString, ScriptCode lScript);
OSErr AddRes2Doc(FSSpec *theDoc, Handle thisHandle, ResType rType, short rID);
void SendmyAE(AEEventClass, AEEventID myAEvtID, AEIdleProcPtr, AESendMode theMode);
void SendmyAEPShort(AEEventClass myclass, AEEventID myAEvtID, AEIdleProcPtr IdleFunct, AESendMode theMode, short theAlert);
void ZoomRectToRect(RectPtr startingRect, RectPtr endingRect, short numofDivs);
void ZoomRectToRectAutoGrafPort(RectPtr startingRect, RectPtr endingRect, short numofDivs);
Handle NewHandleGeneral(Size blockSize);
Handle Get1ResGeneral(ResType theType, short theID);
OSType FindFinderAtEaseProcess(ProcessSerialNumber *);
OSType FindFinderProcess(void);
OSErr SendShutdownToFinder(AEIdleProcPtr, Boolean);
short StopAlert_UPP(short alertID, ModalFilterProcPtr filterProc);
short StopAlert_AE(short alertID, ModalFilterProcPtr filterProc, AEIdleProcPtr IdleFunct);
short CautionAlert_UPP(short alertID, ModalFilterProcPtr filterProc);
short CautionAlert_AE(short alertID, ModalFilterProcPtr filterProc, AEIdleProcPtr IdleFunct);
short NoteAlert_UPP(short alertID, ModalFilterProcPtr filterProc);
short NoteAlert_AE(short alertID, ModalFilterProcPtr filterProc, AEIdleProcPtr IdleFunct);
short Alert_UPP(short alertID, ModalFilterProcPtr filterProc);
short Alert_AE(short alertID, ModalFilterProcPtr filterProc, AEIdleProcPtr IdleFunct);

typedef struct {
	FSSpec	destFile;
	ScriptCode	theScript;
	} StdFileResult, *StdFileResultPtr;

/* dialog box manager */

struct dialogitems {
	short	itemNumber;
	short	group;
	long	refCon;
	};

typedef struct dialogitems dialogItems, *dialogItemsPtr;

void FlashButton(DialogPtr dlg, short item);
void OutlineButton(DialogPtr oftheDialog, short myItem);

short HandleDialog(ModalFilterProcPtr filterProc,
					dialogItemsPtr things,
					void (*initProc)(DialogPtr),
					void (*userProc)(DialogPtr, Handle, short),
					short resId);

/* about box manager */

void myAbout(void (*theIdleProc)(void),
			UserItemUPP DrawQTPict,
			void (*UpdateProc)(EventRecord *),
			void (*ActivateProc)(EventRecord *)
			);
Handle OpenSplash(DialogPtr *, UserItemUPP DrawQTPict);
void DisposeSplash(Handle, DialogPtr);
void myMovableModalAbout(
			Point *aboutTopLeft,
			Point *creditsTopLeft,
			void (*AdjustMenus)(TEHandle),
			void (*Handle_My_Menu)(long),
			void (*DomyKeyEvent)(EventRecord *),
			void (*DoUpdate)(EventRecord *),
			void (*DoActivate)(EventRecord *),
			void (*DoHiLevEvent)(EventRecord *),
			void (*DoOSEvent)(EventRecord *),
			void (*DoIdle)(void),
			unsigned long minSleep
			);

void CopyEMailAddrToClip(void);
void CopyWWWURLToClip(void);
Boolean GetFontNumber(ConstStr255Param fontName, short *fontNum);
Boolean CmdPeriod(EventRecord *theEvent);
Boolean CmdPeriodOrEsc(EventRecord *theEvent);
Boolean CmdPeriodOrEscConfirm(EventRecord *theEvent, short alertID, ModalFilterProcPtr filterProc);
void HandleRegistration(/*ModalFilterProcPtr filterProc,*/
						Point *regDTopLeft,
						long (*regCfgInfo)(Handle, long, StringPtr),
						void (*AdjustMenus)(TEHandle),
						void (*Handle_My_Menu)(long),
						void (*DomyKeyEvent)(EventRecord *),
						void (*DoUpdate)(EventRecord *),
						void (*DoActivate)(EventRecord *),
						void (*DoHiLevEvent)(EventRecord *),
						void (*DoOSEvent)(EventRecord *),
						void (*DoIdle)(void),
						unsigned long minSleep,
						short dollars);
void CopyBugReportForm(long (*regCfgInfo)(Handle, long, StringPtr));
OSErr AskForDestFile(StdFileResultPtr whichFile, ConstStr255Param prompt, StringPtr docName);
void BCDVersNumToString(long num, StringPtr str);
void BCDLongVersNumToString(long num, StringPtr str);
Boolean TrapAvailable(short theTrap);
OSErr InstallRequiredAEHandlers(AEEventHandlerProcPtr myOAPP,
								AEEventHandlerProcPtr myODOC,
								AEEventHandlerProcPtr myPDOC,
								AEEventHandlerProcPtr myQUIT);

