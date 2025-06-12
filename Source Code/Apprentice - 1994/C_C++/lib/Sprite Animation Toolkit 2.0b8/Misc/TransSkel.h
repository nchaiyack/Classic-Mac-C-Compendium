//#ifndef _TransSkel_
//#define	_TransSkel_

//#define	supportDialogs

//¥ applec
//#define AppleC
//#else
//¥ AppleC
//#define applec
//¥
//¥

//¥ applec
#include <Menus.h>
#include <Dialogs.h>
#include <OSUtils.h>
#include <Types.h>
#include <Windows.h>
//¥

//#ifndef nil
//#define	nil	0L
//¥

void SkelOnePass (RgnHandle);
void SkelMain (void);
void SkelWhoa (void);
void SkelClobber (void);
void SkelRmveMenu (MenuHandle);
void SkelRmveWind (WindowPtr);
void SkelSetCursor (WindowPtr, CursHandle, RgnHandle);
Boolean SkelGetCursor (WindowPtr, CursHandle *, RgnHandle *);
void SkelGrowBounds (WindowPtr, short, short, short, short);
void SkelEventMask (short);
void SkelGetEventMask (short *);
SysEnvRec *SkelGetEnvirons (void);
void SkelSetReference (WindowPtr, long);
long SkelGetReference (WindowPtr);
void SkelEnableMenu (short, Boolean);
void SkelDisableMenu (short, Boolean);
void SkelEnableMenus ();
void SkelDisableMenus ();

//¥ supportDialogs
void SkelRmveDlog (DialogPtr);
void SkelDlogMask (short);
void SkelGetDlogMask (short *);
//¥

//¥ applec
Boolean SkelCheckTrap (short, TrapType);
void SkelInit (void (*pBye)(void), int);
extern void SkelMenu(
	MenuHandle theMenu, 
	void (*pSelect)(short), 				
	void (*pClobber)(MenuHandle), 
	Boolean isPull, 
	Boolean drawBar);
//void SkelMenu (
//	MenuHandle, 
//	void (*pSelect)(short), 
//	void (*pClobber)(void), 
//	Boolean, 
//	Boolean);
void SkelApple (char *, void (*about)(void));
void SkelHelp (char *, void (*help)(void));
WindowPtr SkelHelpWindow (char *, short,
							 void (*hActivte)(Boolean), void (*hClose)(short));
void SkelHelpEditOp (short);
void SkelRmveHelp (WindowPtr);
void SkelRmveHelpID (short rsrcID);
extern Boolean	SkelWindow(WindowPtr theWind, void (*pMouse)(Point,long,short), 
					void (*pKey)(char,unsigned char,short), 
					void (*pUpdate)(Boolean,short), void (*pActivate)(Boolean), 
					void (*pClose)(void), void (*pClobber)(void), 
					void (*pIdle)(void), Boolean frontOnly);
//Boolean SkelWindow (WindowPtr,
//				void (*pMouse)(Point, long, short),
//				void (*pKey)(char, unsigned char, short),
//				void (*pUpdate)(Boolean resized, short mods),
//				void (*pActivate)(Boolean),
//				void (*pClose)(void), void (*pClobber)(void), void (*pIdle)(void),
//				Boolean frontOnly);
void SkelBackground (void (*bkgnd)(void), long fgTime, long bgTime);
void SkelGetBackground (void (**bkgnd)(void), long *fgTime, long *bgTime);
void SkelEventHook (Boolean	(*p)(EventRecord *));
void SkelGetEventHook (Boolean (**p)(EventRecord *));
void SkelNetworkProc (void (*p)(EventRecord *));
void SkelGetNetworkProc (void (**p)(EventRecord *));
void SkelScrapConvert (void (*p)(Boolean));
void SkelGetScrapConvert (void (**p)(Boolean));
void SkelSuspendResumeProc (void (*p)(Boolean));
void SkelGetSuspendResume (void (**p)(Boolean));
void SkelChildDiedProc (void (*p)(EventRecord *));
void SkelGetChildDiedProc (void (**p)(EventRecord *));
void SkelHandleApp4Proc (void (*p)(EventRecord *));
void SkelGetHandleApp4Proc (void (**p)(EventRecord *));
void SkelZoomProc (void (*p)(WindowPtr, short, short, Boolean));
void SkelGetZoomProc (void (**p)(WindowPtr, short, short, Boolean));

//¥ supportDialogs
/* stuff for dialog support */
extern void		SkelDialog(DialogPtr theDialog, 
					Boolean (*pEvent)(short *,EventRecord *), 
					Boolean (*pCheck)(short), 
					Boolean (*pKeys)(short *,EventRecord *), 
					void (*pUpdate)(Boolean,short), void (*pClose)(void), 
					void (*pClobber)(void));
//void SkelDialog (DialogPtr,
//		Boolean (*pEvent)(short *, EventRecord *),
//		Boolean (*pCheck)(short),
//		Boolean (*pKeys)(short *, EventRecord *),
//		void (*pUpdate)(Boolean, short),
//		void (*pClose)(void), void (*pClobber)(void));
extern short	SkelModalDialog(DialogPtr dLog, 
					Boolean (*pEvent)(short *,EventRecord *), 
					Boolean (*pCheck)(short), 
					Boolean (*pKeys)(short *,EventRecord *), 
					void (*pUpdate)(Boolean,short), long refCon);
//short SkelModalDialog (DialogPtr,
//		Boolean (*pEvent)(short *, EventRecord *),
//		Boolean (*pCheck)(short),
//		Boolean (*pKeys)(short *, EventRecord *),
//		void (*pUpdate)(Boolean, short),
//		long refCon);
//¥

//¥#else
//¥ Boolean SkelCheckTrap (short, short);
void SkelInit (void (*pBye)(), int);
//void SkelMenu (MenuHandle, void (*pSelect)(), void (*)(), Boolean, Boolean);
void SkelApple (char *, void (*about)());
void SkelHelp (char *, void (*help)());
//¥ WindowPtr SkelHelpWindow (char *, short, void (*hActivte)(), void (*hClose)());
void SkelHelpEditOp (short);
void SkelRmveHelp (WindowPtr helpWin);
void SkelRmveHelpID (short rsrcID);

//Boolean SkelWindow (WindowPtr,
//				void (*pMouse)(), void (*pKey)(),
//				void (*pUpdate)(), void (*pActivate)(),
//				void (*pClose)(), void (*pClobber)(), void (*pIdle)(),
//				Boolean frontOnly);
void SkelBackground (void (*bkgnd)(), long fgTime, long bgTime);
void SkelGetBackground (void (**bkgnd)(), long *fgTime, long *bgTime);
void SkelSetMenuHook (long (*p)());
void SkelGetMenuHook (long (**p)());
void SkelEventHook (Boolean	(*p)());
void SkelGetEventHook (Boolean (**p)());
void SkelNetworkProc (void (*p)());
void SkelGetNetworkProc (void (**p)());
//¥ void SkelScrapConvert (void (*p)());
//¥ void SkelGetScrapConvert (void (**p)());
//¥ void SkelSuspendResumeProc (void (*p)());
//¥ void SkelGetSuspendResume (void (**p)());
void SkelChildDiedProc (void (*p)());
void SkelGetChildDiedProc (void (**p)());
void SkelHandleApp4Proc (void (*p)());
void SkelGetHandleApp4Proc (void (**p)());
//¥ void SkelZoomProc (void (*p)());
//¥ void SkelGetZoomProc (void (**p)());

//¥ supportDialogs
//void SkelDialog (DialogPtr, Boolean (*pEvent)(), Boolean (*pCheck)(),
//		Boolean (*pKeys)(), void (*pUpdate)(), void (*pClose)(), void (*pClobber)());
//short SkelModalDialog (DialogPtr, Boolean (*pEvent)(), Boolean (*pCheck)(),
//		Boolean (*pKeys)(), void (*pUpdate)(),
//		long refCon);
//¥

//¥

//¥
