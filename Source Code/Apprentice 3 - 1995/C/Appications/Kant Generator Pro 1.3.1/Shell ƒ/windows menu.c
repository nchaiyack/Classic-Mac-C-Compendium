/* this code is based on Matt Mora's window menu management code; thanks Matt */

#include "windows menu.h"
#include "window layer.h"

#define kWindowsSeparator		3			/* 1 + # of items that are always in windows menu */
#define theMenu					gWindowsMenu

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif
struct MyWindowsQueue
{
	struct			MyWindowsQueue *next;
	short			type;
	WindowRef		theWindow;
};
#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

typedef struct MyWindowsQueue	MyWindowsQueue, *MyWindowsQueuePtr;

extern	MenuHandle		gWindowsMenu;		/* see lyr_menu.c */
static	QHdr			gWindowsMenuQueue;

void AddToWindowsMenu(WindowRef theWindow)
{
	Str255				theStr;
	MyWindowsQueuePtr	queuePtr;
	
	if (CountMItems(theMenu)<kWindowsSeparator)
		InsMenuItem(theMenu, "\p-", kWindowsSeparator);
	
	if ((queuePtr=(MyWindowsQueuePtr)NewPtrClear(sizeof(*queuePtr)))!=0L)
	{
		queuePtr->theWindow=theWindow;
		Enqueue((QElemPtr)queuePtr, &gWindowsMenuQueue);

		AppendMenu(theMenu, "\p ");
		GetWTitle(theWindow, theStr);
		SetItem(theMenu, CountMItems(theMenu), theStr);
		
		DrawMenuBar();
	}
}

void ModifyFromWindowsMenu(WindowRef theWindow)
{
	short				i;
	Str255				theStr;
	MyWindowsQueuePtr	queuePtr;
	
	for (queuePtr=(MyWindowsQueuePtr)gWindowsMenuQueue.qHead, i=0; queuePtr!=0L;
		queuePtr=queuePtr->next, i++)
	{
		if (queuePtr->theWindow==theWindow)
		{
			GetWTitle(theWindow, theStr);
			SetItem(theMenu, kWindowsSeparator+i+1, theStr);
		}
	}
}

void RemoveFromWindowsMenu(WindowRef theWindow)
{
	short				i;
	MyWindowsQueuePtr	queuePtr;
	
	for (queuePtr=(MyWindowsQueuePtr)gWindowsMenuQueue.qHead, i=0; queuePtr!=0L;
		queuePtr=queuePtr->next, i++)
	{
		if ((queuePtr->theWindow==theWindow) && (!Dequeue((QElemPtr)queuePtr, &gWindowsMenuQueue)))
		{
			DelMenuItem(theMenu, kWindowsSeparator+i+1);
			if (CountMItems(theMenu) == kWindowsSeparator)
				DelMenuItem(theMenu, kWindowsSeparator);
		}
	}
}

void SelectWindowsMenuItem(short item)
{
	short			i;
	MyWindowsQueuePtr queuePtr;
	
	for (queuePtr=(MyWindowsQueuePtr)gWindowsMenuQueue.qHead, i=kWindowsSeparator+1;
			queuePtr && (i<item); queuePtr=queuePtr->next, i++)
	{
		/* nothing here intentionally */
	}
	if (queuePtr!=0L)
		MySelectWindow(queuePtr->theWindow);
}

void RebuildWindowsMenu(WindowRef checkedWindow)
{
	short				i, menuKeyIndex;
	MyWindowsQueuePtr	queuePtr;
	WindowRef			theWindow;
	Str255				theStr;
	
	if (checkedWindow==0L)
		return;
	
	AppendMenu(theMenu, "\p-");
	for (queuePtr=(MyWindowsQueuePtr)gWindowsMenuQueue.qHead, i=kWindowsSeparator+1, menuKeyIndex=1;
		queuePtr!=0L; queuePtr=queuePtr->next, i++, menuKeyIndex++)
	{
		theWindow=queuePtr->theWindow;
		AppendMenu(theMenu, "\p ");
		GetWTitle(theWindow, theStr);
		SetItem(theMenu, CountMItems(theMenu), theStr);
		CheckItem(theMenu, i, (queuePtr->theWindow==checkedWindow));
	}
}

void AdjustWindowsMenu(WindowRef checkedWindow)
{
	short				i, menuKeyIndex;
	MyWindowsQueuePtr	queuePtr;
	
	if (CountMItems(theMenu)<kWindowsSeparator)
		return;
	
	for (queuePtr=(MyWindowsQueuePtr)gWindowsMenuQueue.qHead, i=kWindowsSeparator+1, menuKeyIndex=1;
		queuePtr!=0L; queuePtr=queuePtr->next, i++, menuKeyIndex++)
	{
		CheckItem(theMenu, i, (queuePtr->theWindow==checkedWindow));
	}
}

short GetNumberOfOpenWindows(void)
{
	short			menuCount;
	
	menuCount=CountMItems(theMenu);
	if (menuCount<kWindowsSeparator)
		return 0;
	return menuCount-kWindowsSeparator;
}
