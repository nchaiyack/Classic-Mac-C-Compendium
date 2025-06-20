// File "main.c"

#include <GestaltEqu.h>

#include "main.h"
#include "List.h"
#include "Windows.h"

// * **************************************************************************** * //
// Global Declarations

short hasColorQD;
WindowPtr gWindow;
EventRecord gEvent;
ListHandle gList;

// * **************************************************************************** * //
// * **************************************************************************** * //

void DoInit() {
	long response;
	Rect destRect;
	
	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0);
	
	hasColorQD = (Gestalt(gestaltQuickdrawFeatures, &response) ||
		((response & (1 << gestaltHasColor)) == 0)) ? 0 : -1;
	
	SetRect(&destRect, 40, 40, 200, 250);
	gWindow = (hasColorQD) ?
			NewCWindow(0, &destRect, "\pList Tester", 0, documentProc, (WindowPtr) -1, -1, 0) :
			NewWindow(0, &destRect, "\pList Tester", 0, documentProc, (WindowPtr) -1, -1, 0);
	CreateList();
	ShowWindow(gWindow);
	}

// * **************************************************************************** * //
// * **************************************************************************** * //

void DoLoop() {
	short running;
	long sizeVH;
	Rect sizeRect;
	GrafPtr savePort;
	WindowPtr whichWin;
	EventRecord theEvent;
	
	running = -1;
	while(running) {
		if (WaitNextEvent(everyEvent, &theEvent, 60, 0)) {
			switch(theEvent.what) {
				case nullEvent:
					break;
				case mouseDown:
					switch(FindWindow(theEvent.where, &whichWin)) {
						case inMenuBar: SysBeep(7); break;
						case inContent: 
							GetPort(&savePort);
							SetPort(gWindow);
							GlobalToLocal(&theEvent.where);
							LClick(theEvent.where, theEvent.modifiers, gList);
							SetPort(savePort);
							break;
						case inDrag:
							DragWindow(gWindow, theEvent.where, &screenBits.bounds);
							break;
						case inGrow:
							sizeRect = screenBits.bounds;
							sizeRect.left = 40;
							sizeRect.top = 60;
							sizeVH = GrowWindow(gWindow, theEvent.where, &sizeRect);
							if (sizeVH) {
								SizeWindow(gWindow, LoWord(sizeVH), HiWord(sizeVH), -1);
								LSize(LoWord(sizeVH)-15, HiWord(sizeVH)-15, gList);
								GetPort(&savePort);
								SetPort(gWindow);
								InvalRect(&gWindow->portRect);
								SetPort(savePort);
								}
							break;
						case inGoAway:
							if (TrackGoAway(gWindow, theEvent.where)) running = 0;
							break;
						default: break;
						}
					break;
				case keyDown:
				case autoKey:
					if (((theEvent.message & keyCodeMask) >> 8 == 0x0C) && 
							(theEvent.modifiers & cmdKey)) running = 0;
					break;
				case updateEvt:
					DoUpdate();
					break;
				case activateEvt:
					DoActivate(theEvent.modifiers & activeFlag);
					break;
				default:
					break;
				}
			SetCursor(&arrow);
			}
		}
	}

// * **************************************************************************** * //
// * **************************************************************************** * //

void DoClose() {
	DisposeList();
	DisposeWindow(gWindow);
	}

// * **************************************************************************** * //
// * **************************************************************************** * //

void main() {

	DoInit();
	DoLoop();
	DoClose();
	}
