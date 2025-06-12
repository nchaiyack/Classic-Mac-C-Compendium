/* CApplication.h */

#pragma once

#include <AppleEvents.h>
#include "CWindow.h"
#include "MiscInfo.h"

class CDocument;
class CAboutWindow;
class CMyApplication;

struct		CApplication	:	CWindow
	{
	 private:
		static CApplication*	GlobalApplication;
	 public:
		CSack*				ListOfDocuments;
		CAboutWindow*	AboutWindow;

		/* initialization routine */
		/* */		CApplication();
		/* */		~CApplication();

		/* high level events get sent here to be handled */
		void		ReceiveHighLevelEvent(EventRecord* Event);

		void		RegisterDocument(CDocument* TheDocument);
		void		DeregisterDocument(CDocument* TheDocument);
		void		InitiateQuit(void);

	 protected:
		/* high level event handlers send events here to be handled */
		void		DispatchOpenApplication(void);
		void		DispatchOpenDocument(FSSpec* TheFSSpec);
		void		DispatchPrintDocument(FSSpec* TheFSSpec);
		void		DispatchQuit(void);

		/* function to check to see that all required parameters have been gotten */
		static	OSErr					MyGotRequiredParams(AppleEvent* theAppleEvent);
		/* handler for open application--presents an untitled document */
		static	pascal	OSErr	MyHandleOApp(AppleEvent* theAppleEvent,
														AppleEvent* reply, long handlerRefcon);
		/* handler for print documents */
		static	pascal	OSErr	MyHandleODoc(AppleEvent* theAppleEvent,
														AppleEvent* reply, long handlerRefcon);
		/* handler for print documents */
		static	pascal	OSErr	MyHandlePDoc(AppleEvent* theAppleEvent,
														AppleEvent* reply, long handlerRefcon);
		/* handle a quit event */
		static	pascal	OSErr	MyHandleQuit(AppleEvent* theAppleEvent,
														AppleEvent* reply, long handlerRefcon);
	 public:
		/* If the active window can't handle a menu command, it is sent here */
		MyBoolean	DoMenuCommand(ushort MenuCommand);

		/* re-enable menu items that the application can handle */
		void		EnableMenuItems(void);

		/* post the menus to the menu bar */
		void		InitMenuBar(void);

		/* display the about window */
		void		DoAboutWindow(MyBoolean AutoFlag);

		/* create a new document */
		void		DoMenuNew(void);

		/* open a document using standard get file */
		void		DoMenuOpen(void);
		void		ConstructFileTypeTable(OSType Table[NUMFILETYPES], short* NumTypes,
							pascal Boolean (**FileFilter)(CInfoPBRec* pb));
	};

#ifndef COMPILING_CAPPLICATION_C
	extern CMyApplication* Application;
#endif
