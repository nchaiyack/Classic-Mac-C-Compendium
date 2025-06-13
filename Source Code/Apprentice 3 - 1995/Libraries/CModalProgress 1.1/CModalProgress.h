// ===========================================================================
//	ModalProgress.h
//	
//	This file contains the header information for the ModalProgress class.
//	This class provides an interface for displaying to the user, a visual 
//	indication to the progress of a time intensive task. 
//
//	The resource ID of a dialog resource is passed to the constructor which
//	will be used in the progress dialog. There are a number of things to
//	consider when creating the dialog as certain items must be present for
//	some actions to take place.
//
//	If an OK equivalent button is needed, then use item 1 for this purpose.
//	If you wish this button to have a border, create a user item the size of
//	the outline, place it over the OK button, this should be item 3.
//
//	A CANCEL button should use item number 2, if required.
//
//	Standard key equvalents are supported, Enter, Return, Command-., ESC for
//	each of these buttons.
//
//	If these particular items are not required by the dialog, they must be
//	generated as hidden disabled items. Do not just leave the items out of
//	the diaog.
//
//
//	If a progress bar is required in the dialog (which is expected for most
//	uses of this class) it should be generated as a user item that is the
//	size of the frame that is required for this bar. The item number of this
//	user item is passed to the SetProgressBar or SetInfiniteBar method.
//	
//	If a simple text value of percent complete is required, the item should
//	be generated as a StaticText item, and its item number passed to the
//	SetPercentText method along with the resolution of update.
//
//
//	History
//	-------
//	
//	Graham Heathcote, 1st of September 1994.
//		-	Creation of initial version 1.0 release
//
//	Graham Heathcote, 5th of February 1995.
//		-	Added two new fields to hold the UniversalProcPtrs needed for PowerPC compilers.
//
//	Graham Heathcote, 16th of February 1995.
//		-	Added the SetParamText() method and the extra SetupDialog() method.
//		-	Added the CanProcessEvent() method which lets the main application pass in an
//			event for the dialog to process if it can.
//		-	Added the ProcessIdle() method.
//
//	Version 1.1
//
//	Copyright 1994, 1995 Alysoft Solutions. All rights reserved.
//
// ===========================================================================

#ifndef __CMODALPROGRESS__
#define __CMODALPROGRESS__

#ifndef __DIALOGS__
#include <dialogs.h>
#endif

typedef enum {
	kDialogContinues							= 0,
	kDialogOKHit,
	kDialogCancelHit,
	kDialogExceeding100pc,

	kStateSpaceWithinLimit,
	kStateSpaceExceedingLimit
} tDialogProcessStates ;

typedef enum {
	kDefaultButtonIndex							= 1,
	kCancelButtonIndex,
	kDefaultButtonOutlineIndex
} tDialogItemIndicies ;

#define	kIndicatorOutline						0x0001
#define	kIndicatorContent						0x0002

#define	kDefaultInfiniteDrawDelay				1
#define	kInfinitePaternResID					6000

class CModalProgress
{
	private:
		float					fStartStatePercent ;
		float					fCurrentStateSpace ;
		float					fCurrentStateValue ;
		float					fCurrentStateSpacePercent ;
		long					fInfiniteDrawTime ;
		Boolean					fPercentText ;
		Boolean					fProgressBar ;
		Boolean					fInfiniteBar ;
		short					fPercentTextItem ;
		short					fProgressBarItem ;
		short					fInfiniteBarItem ;
		short					fPercentTextDeltaLimit ;
		UserItemUPP				fButtonOutlineDrawProc ;
		UserItemUPP				fProgressBarDrawProc ;

		Boolean			FlashButton(DialogPtr theDialog, short buttonIndex) ;
		void			UpdateProgressIndicator(float newPercent) ;

	protected:
		virtual Boolean	ProcessEvent(EventRecord *theEvent, short *result) ;
		virtual	void	DrawPercentIndicator(float percent, short deltaLimit, short itemIndex, short part) ;
		virtual	void	DrawStdBarIndicator(float percent, short itemIndex, short part) ;
		virtual	void	DrawInfiniteBarIndicator(float percent, short itemIndex, short part) ;
		virtual	short	CreateTheDialog(short dlgResId) ;

	public:
		DialogPtr				fDialog ;
		float					fCurrentPercent ;
		long					fInfiniteDrawDelay ;

		CModalProgress() ;							// Constructor
		~CModalProgress() ;							// Destructor

		virtual void	SetParamText(void) ;
		virtual	short	SetupDialog(short dlgResId) ;
		virtual	short	SetupDialog(short dlgResId, ConstStr255Param param0, ConstStr255Param param1, ConstStr255Param param2, ConstStr255Param param3) ;
		virtual	void	SetProgressBar(short dlgItem) ;
		virtual	void	SetInfiniteBar(short dlgItem) ;
		virtual	void	SetPercentText(short dlgItem, short deltaLimit) ;
		
		virtual	void	SetCurrentState(float statePercent) ;
		virtual	void	SetStateSpace(float space) ;
		virtual short	SetCurrentStateValue(float value) ;

		virtual	void	DrawProgressIndicator(float percent, short part) ;
		
		virtual	void	BeginModal(void) ;
		virtual short	ProcessIdle(void) ;
		virtual	short	ProcessModal(void) ;
		virtual Boolean	CanProcessEvent(EventRecord *theEvent, short *result) ;
		virtual	void	EndModal(void) ;

} ;

#endif
