/*
	PassODocs.h
	
	Header file for PassODocs.c
	
*/

#pragma once

#ifndef __H_PassODocs__
#define __H_PassODocs__

#define kMBar			128
#define kMApple		128
#define	kMAbout			1
#define kMFile			129
#define	kMOpen			1
#define	kMPrefs			2
#define	kMQuit			4

// Other Constants...
#define kWindNoStorage	((Ptr)0L)
#define kWindInFront		((WindowPtr)-1)

#ifndef normal
#define normal			0	/* text face normal */
#endif

#ifdef __cplusplus
extern "C" {
#endif

// General routines
void main(void);
OSErr InitToolbox(void);
OSErr Initialize(void);
OSErr CleanUp(void);
OSErr MainLoop(void);

// AppleEvent routines
OSErr InitializeAppleEvents(void);
pascal OSErr HandleOApp(AppleEvent* ae,AppleEvent* reply,long refCon);
pascal OSErr HandleODoc(AppleEvent* ae,AppleEvent* reply,long refCon);
pascal OSErr HandlePDoc(AppleEvent* ae,AppleEvent* reply,long refCon);
pascal OSErr HandleQuit(AppleEvent* ae,AppleEvent* reply,long refCon);
OSErr NotHandledEvent(AppleEvent* ae);
OSErr HandleMissedParams(AppleEvent* ae);
OSErr HandleRequiredParams(AppleEvent* ae);

// Event Handling Procedures
OSErr HandleEvent(EventRecord* er);
OSErr HandleMouse(EventRecord* er);
OSErr HandleLimitedMouse(EventRecord* er,short* part,Boolean* handled);
OSErr HandleKey(EventRecord* er);
OSErr HandleActivateEvt(EventRecord* er);
OSErr HandleUpdate(EventRecord* er);
OSErr HandleDisk(EventRecord* er);
OSErr HandleHighLevelEvent(EventRecord* er);
OSErr HandleActivate(EventRecord* er);
OSErr HandleDeactivate(EventRecord* er);
OSErr HandleMouseMoved(EventRecord* er);

// Menu Routines
OSErr HandleMenuEnabling(void);
OSErr HandleMenu(long ms);
OSErr HandleApple(short item);
OSErr HandleFile(short item);

// Support Routines
OSErr HandleAbout(void);
OSErr HandlePrefs(void);
OSErr HandleError(StringPtr msg,OSErr err);
OSErr HandleOpen(void);

void SetHelp(StringPtr s1,StringPtr s2);
void OSTypeToStr255(OSType type,StringPtr str);
void Str255ToOSType(StringPtr str,OSType* type);

OSErr ReplacePrefs(void);
OSErr HandleAppSelection(void);

short DlgFindItem(DialogPtr dlg,Point pt);

// About Routines
pascal Boolean CustomFilter(DialogPtr dlg,EventRecord* erp,short* item);
pascal void ItemIconFamily(WindowPtr wind,short item);
pascal void Item3DBox(WindowPtr wind,short item);
pascal void ItemAboutText(WindowPtr wind,short item);
pascal void ItemOvalFrame(WindowPtr wind,short item);
pascal void ItemBox(WindowPtr wind,short item);
pascal void ItemHelpText(WindowPtr wind,short item);
pascal void ItemHelpStrings(WindowPtr wind,short item);
pascal void ItemHelpTitle(WindowPtr wind,short item);

#ifdef __cplusplus
}
#endif

#endif /* __H_PassODocs__ */



