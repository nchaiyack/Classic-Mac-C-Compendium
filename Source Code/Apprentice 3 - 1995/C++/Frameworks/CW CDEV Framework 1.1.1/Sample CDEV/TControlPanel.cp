#ifndef _H_TControlPanel
#include "TControlPanel.h"
#endif

   
// CW CDEV Framework, �1994-95, Matthew E. Axsom, All Rights Reserved
// Original Framework:  Matthew E. Axsom (chewey@top.monad.net)
// Contributions From:	dEVoN Hubbard (TDevon@aol.com)
//						Greg Landweber (greg@math.harvard.edu)
// -----------------------------------------------------------
// Version History
// -----------------------------------------------------------
// 1.1.1	Released 05/16/95 - mea
//			Default project is now configured to use the stub CDEV code resource.
//				Included stub CDEV in project.
//				Changed ResType of project preferences to 'cDEV'.
//
//			Modified 01/17/95 - mea
//			Added a check for the macDev message before processing results of function/method
//				calls in CDEVMain.c.  This check prevents the return code from runable() from 
//				being interpreted as an error message.  Serious nasty-ness would result if that
//				happened.  Only those CDEV's with 'mach' resources set to something other
//				that 0xFFFF0000 would have encountered this problem.  Thanks to Greg Landweber
//				for finding this bug!
				
// 1.1		Released 12/08/94 - mea
//			Added more robust error handling so as to conform to Apple's
//				standard cdev error return codes.
//			Added and corrected comments and udated instructions.
//			Also tested compilation under CW5  No problems.

// 1.1d2	Internal version - mea
//			Added a Init() message to base class and call it from CDEVMain
//				just after the object is created.  Should ease porting from Think.
//			Added a Close() message to base class and call it from CDEVMain just
//				before we delete the object.  Should ease porting from Think.
//			Eliminated calls to CommandKey() if keyEvtDev is generated via an
//				autoKey event.
//			Changed name of class source files from CDEVClass.c/h to
//				TControlPanel.cp/h.
//			Changed name of starter cdev source from CDEV.c to CDEV.cp.

// 1.1d1	Internal version - dth
//			(Modifications for easier porting from Think cdev class
//				projects without losing strengths of Chewey's framework)
//			Changed name of class from cdevObj to TControlPanel.
//			Changed method names to begin w/capital letter.
//			Changed hit() to ItemHit()
//			Changed KeyDown() & CommandKey() to accept short byte value.
//			Changed action() to give short byte value to key methods.
//			Instance variables are now prefixed with an 'f' to indicate
//				they are a member/field variable.

// 1.0.1	Released 12/13/94
//			DlgCut() added to cut() method.
//			DlgCopy() added to copy() method.
//			DlgPaste() added to paste() method.
//			DlgDelete() added to clear() method.
//			Note:  The above additions were made to the sample's base class
//   		in the original 1.0 distribution, but didn't make it (I'll plead
//			version control problems ;) into the framework's base class.
//			Thanks to dEVoN Hubbard for pointing this out!
//			
// 1.0 		Released 12/08/94
//			Initial verison

TControlPanel::TControlPanel(short numItems,DialogPtr cp)
{
	fLastItem = numItems;
	fDialog = cp;

	// Init() gets called from CDEVMain just after this call
}

// sorry, don't have anything to destroy
TControlPanel::~TControlPanel(void)
{
	// close gets called from CDEVMain just before this call
}

// provided for compatibility with Think class.
long TControlPanel::Init(void)
{
	return noErr;
}

// provided for compatibility with Think class.
long TControlPanel::Close(void)
{
	return noErr;
}

// handle actions not related to initing, opening or closing
long TControlPanel::actions(short message,short itemHit)
{
	long	result=0;
	
	switch (message)
		{
		// handle a click
		case hitDev:
			result=ItemHit(itemHit - fLastItem);	// normalize to our values
			break;
		
		// handle a null event by performing any idle time processing
		case nulDev:
			result=Idle();
			break;
			
		// handle user item updates
		case updateDev:
			result=Update();
			break;
		
		// activate things
		case activDev:
			result=Activate();
			break;
		
		// deactivate things
		case deactivDev:
			result=Deactivate();
			break;
		
		// keydown or autokey
		case keyEvtDev:
			// filter out command keys and handle them via commandKey()

			if ((fEvent->modifiers & cmdKey) == 0)
				result = KeyDown((unsigned char) fEvent->message);
			else
				// avoid processing command keys tied to autoKey events
				if (fEvent->message != autoKey)
					result = CommandKey((unsigned char) fEvent->message);
			break;
			
		// undo command from edit menu
		case undoDev:
			result=Undo();
			break;
		
		// cut command from edit menu
		case cutDev:
			result=Cut();
			break;
		
		// copy command from edit menu
		case copyDev:
			result=Copy();
			break;
		
		// paste command from edit menu
		case pasteDev:
			result=Paste();
			break;
		
		// clear command from edit menu
		case clearDev:
			result=Clear();
			break;
		}
	
	return result;
}

// handle command keys for normal editing items
long TControlPanel::CommandKey(short theChar)
{
	long	result=0;
	
	switch (theChar) {
		case 'z':	// cmd - z
		case 'Z':
			result=Undo();
			break;
		
		case 'x':	// cmd - x
		case 'X':
			result=Cut();
			break;
		
		case 'c':	// cmd - c
		case 'C':
			result=Copy();
			break;
		
		case 'v':	// cmd - v
		case 'V':
			result=Paste();
			break;
	}
	
	return result;
}

// handle a hit in the control panel.  Note lastItem has already been added to itemHit
long TControlPanel::ItemHit(short /*itemHit*/)
{
	return noErr;
}

// got a nulDev (nullEvent) message
long TControlPanel::Idle(void)
{
	return noErr;
}

// update any user items
long TControlPanel::Update(void)
{
	return noErr;
}

// activate items
long TControlPanel::Activate(void)
{
	return noErr;
}

// deactivate items
long TControlPanel::Deactivate(void)
{
	return noErr;
}

// handle a key down
long TControlPanel::KeyDown(short /*theChar*/)
{
	return noErr;
}

// handle an undo
long TControlPanel::Undo(void)
{
	return noErr;
}

// handle a cut
long TControlPanel::Cut(void)
{
	DlgCut(fDialog);
	
	return noErr;
}

// handle a copy
long TControlPanel::Copy(void)
{
	DlgCopy(fDialog);
	
	return noErr;
}

// handle a paste
long TControlPanel::Paste(void)
{
	DlgPaste(fDialog);

	return noErr;
}

// handle a clear
long TControlPanel::Clear(void)
{
	DlgDelete(fDialog);
	
	return noErr;
}
	
