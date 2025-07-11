/*---------------------------------------------------------------------
	Program: Dlog.h
	By: Geoffrey Slinker
	Date: 11:57:23 AM  5/2/92
	Purpose: Contains definitions of the superclass Dlog
---------------------------------------------------------------------*/
#pragma once

/**********************************************************************/
/*-------------------------
Class Definition for my Dlog
which is a Dialog Class.
-------------------------*/
class Dlog : direct {
	public:
		DialogPtr myDlog;
		int rsrcNumber;
		
		int itemHit, itemType, dialogDone;
		Rect itemRect;
		Handle itemHandle;
		
	public:
		Dlog(void);	// constructor
		~Dlog(void);
		
		virtual void loadResource( int number);	// loads resource and sets number
		virtual void showDialog(void);	// Draws dialog and waits for click
		virtual void HandleDialog(EventRecord *aEvent);	//  contains dialog loop
		virtual void RedrawDialog(void);
		virtual void DamageDialog(void);
		virtual void PutUpDialog(void);
		
	protected:
		virtual void SetItemValues(void);	// Should be overridden
		virtual int HandleDialogItem(int itemHit, Point thePoint, int thePart);	// Must be overloaded
}; // class Dlog