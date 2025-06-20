// ObjectWindow.h

#pragma once

#define MyWindowID	1000

typedef struct {
	CWindowRecord 	theWin;
	Boolean			ownStorage;
	Boolean			floating;
	void		(*Dispose)(WindowPtr theWin);
	void		(*Update)(WindowPtr theWin);
	void		(*Activate)(WindowPtr theWin);
	void		(*HandleClick)(WindowPtr theWin, Point where);
	void		(*Draw)(WindowPtr theWin);
	void		(*Idle)(WindowPtr theWin, EventRecord *theEvent);
	void		(*ProcessKey)(EventRecord *theEvent);
	void		(*Save)(WindowPtr theWin, SFReply *itsReply);
} ObjectWindowRecord, *ObjectWindowPtr;

WindowPtr InitObjectWindow(short resID, ObjectWindowPtr theStorage, Boolean isFloating);

void DisposeObjectWindow(WindowPtr theWin, Boolean disposeFlag);
void DefaultUpdate(WindowPtr theWin);
void DefaultActivate(WindowPtr theWin);
void DefaultHandleClick(WindowPtr theWin, Point where);
void DefaultDispose(WindowPtr theWin);
void DefaultDraw(WindowPtr theWin);


WindowPtr FrontDocument(void);
WindowPtr FrontFloating(void);
void SelectDocument(WindowPtr theWindow);
void SelectFloating(WindowPtr theWindow);
void LocateWindows(void);
