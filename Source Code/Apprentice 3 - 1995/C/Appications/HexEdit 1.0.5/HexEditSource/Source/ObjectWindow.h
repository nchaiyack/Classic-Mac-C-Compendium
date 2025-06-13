// ObjectWindow.h

#pragma once

#define MyWindowID	1000

typedef struct {
	WindowRecord	theWin;
	Boolean			ownStorage;
	Boolean			active;
	Boolean			floating;
	void	(*Dispose)(WindowPtr theWin);
	void	(*Update)(WindowPtr theWin);
	void	(*Activate)(WindowPtr theWin, Boolean active);
	void	(*HandleClick)(WindowPtr theWin, Point where, EventRecord *er);
	void	(*Draw)(WindowPtr theWin);
	void	(*Idle)(WindowPtr theWin, EventRecord *er);
	void	(*Save)(WindowPtr theWin);
	void	(*SaveAs)(WindowPtr theWin);
	void	(*Revert)(WindowPtr theWin);
	void	(*ProcessKey)(WindowPtr theWin, EventRecord *theEvent);
} ObjectWindowRecord, *ObjectWindowPtr;

WindowPtr InitObjectWindow(short resID, ObjectWindowPtr theStorage, Boolean isFloating);

void DisposeObjectWindow(WindowPtr theWin, Boolean disposeFlag);
void DefaultUpdate(WindowPtr theWin);
void DefaultActivate(WindowPtr theWin, Boolean active);
void DefaultHandleClick(WindowPtr theWin, Point where, EventRecord *er);
void DefaultDispose(WindowPtr theWin);
void DefaultDraw(WindowPtr theWin);
