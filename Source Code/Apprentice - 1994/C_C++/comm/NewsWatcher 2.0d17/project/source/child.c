/*----------------------------------------------------------------------------

	child.c

	This module manages the parent/child window relationship
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include "glob.h"
#include "child.h"
#include "util.h"


/*	AddChild adds a child window to the windowlist of a parent
	Windows with associated child windows will close
	their children when closed
*/

void AddChild (WindowPtr parent, WindowPtr child)
{
	TWindow **parentInfo;
	TChild **newChild;
	
	parentInfo = (TWindow**) GetWRefCon(parent);
	
	newChild = (TChild**) MyNewHandle(sizeof(TChild));
	if (MyMemErr() != noErr)
		return;
	(**newChild).childWindow = child;
	(**newChild).next = (**parentInfo).childList;
	(**parentInfo).childList = newChild;
}


/* Removes a child window from the windowlist of a parent
*/

void RemoveChild (WindowPtr parent, WindowPtr child)
{
	TWindow **parentInfo;
	TChild **current,**prev;
	
	if (parent == nil) return;
	parentInfo = (TWindow**) GetWRefCon(parent);
	for (current = prev = (**parentInfo).childList;
		current != nil && (**current).childWindow != child;
		prev = current,current = (**current).next)
		;
	if (current) {
		if (prev == current) {
			(**parentInfo).childList = (**current).next;
		} else {
			(**prev).next = (**current).next;
		}
		MyDisposHandle((Handle)current);
	}
}


/*	FindChildByCellData locates an open child window corresponding to
	the cell data (index into group or subject array) of a cell in a
	list window. */
	
WindowPtr FindChildByCellData (WindowPtr wind, short cellData)
{
	TWindow **info,**childInfo;
	TChild **childListRec;
	WindowPtr child;
	EWindowKind kind;
	
	info = (TWindow**)GetWRefCon(wind);
	kind = (**info).kind;
	for (childListRec = (**info).childList; childListRec != nil; 
		childListRec = (**childListRec).next) 
	{
		child = (**childListRec).childWindow;
		childInfo = (TWindow**)GetWRefCon(child);
		if (kind == kSubject) {
			if ((**childInfo).parentSubject == cellData) return child;
		} else {
			if ((**childInfo).parentGroup == cellData) return child;
		}
	}
	return nil;
}


/*	FindChild locates an open child window corresponding to a cell in a 
	list window. */
	
WindowPtr FindChild (WindowPtr wind, Cell theCell)
{
	TWindow **info;
	ListHandle theList;
	short cellData, cellDataLen;
	
	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;
	cellDataLen = 2;
	LGetCell(&cellData, &cellDataLen, theCell, theList);
	return FindChildByCellData(wind, cellData);
}
