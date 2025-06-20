// Simple Sample
// A sample cdev created using the CW CDEV Framework 1.1.1
// �1994-95, Matthew E. Axsom
// By: Matthew E. Axsom (chewey@top.monad.net)
//--------------------------
// Version History
// 1.1.0	05/16/95
//			Now uses stub CDEV code resource to eliminate problems with multi-segment cdev's.  The
//				following changes were made to accomidate the stub:
//					Added stub CDEV resource file to project.
//					Changed project ResType to 'cDEV'
//					Moved CDEV.cp and TControlPanel.cp to a new segment.
//
// 1.0.1	12/20/94
//			Updated vers resources to contain CDEV name and copyright info.  Also
//				corrected version information.
//			
//			Updated source code to be compatable w/v1.1 of the framework.
//			Changed name of class to from myCDevObj to myCDEV

// 1.0 		12/09/94 
//			Initial release

// While it's not the most complicated cdev in the world, it
// hopefully will give you the ideas behind the framework.
// -Chewey

#include "CDEV.h"

// control panel items
enum {
	kUserItem=1,
	kCheckBox,
	kTextBox=4
};

// base resource id
enum {
	kBaseID=-4064
};

// items in STR# resource
enum {
	kDeactivatedText=1,
	kAlertText,
	kInitialText
};

// prototypes for 2 required procedures
long runable(void);
TControlPanel *makeCDEV(short numItems,DialogPtr cp);

// return 1 so we can run.
long runable(void)
{
	return 1;
}

// code that allocates our cdev
TControlPanel *makeCDEV(short numItems,DialogPtr cp)
{
	// all we need to do here is allocate *our* object that controls the cdev
	return new myCDEV(numItems,cp);
}

// constructor for our cdev
myCDEV::myCDEV(short numItems,DialogPtr cp) : TControlPanel(numItems,cp)
{
	// set up our internal members
	activated=false;
	lastTime=0;
	showSeconds=true;
	timeString[0]=0;		// initially no string
	
	// set the check box up
	setCheckBox();
	
	// put the initial text in the edit text box
	
	// get initial string
	Str255	s;
	GetIndString(s,kBaseID,kInitialText);

	// get the handle to the text, install and select all the text
	Handle	h;
	Rect	r;
	short	type;
	
	GetDItem(fDialog,fLastItem+kTextBox,&type,&h,&r);
	SetIText(h,s);
	SelIText(fDialog,fLastItem+kTextBox,0,32000);
}

// destructor for our cdev
myCDEV::~myCDEV(void)
{
	// I have no dynamic storage that I need to get rid of before we close
}

// handles a mouse hit in the control panel
long myCDEV::ItemHit(short itemHit)
{	
	// the only hitable item I need to worry about is the checkbox...
	switch (itemHit) {
		case kCheckBox:
			// modify our internal member to the checkbox's new state and update the check box
			showSeconds=!showSeconds;
			setCheckBox();
			
			// forces a redraw on the next idle
			lastTime=0;
			break;
	}

	return noErr;	
}

// handling of nulDev message
long myCDEV::Idle(void)
{
	// only do this if we are in the forground
	if (activated)
		// if the time has changed, do an update
		if (getTime())
			Update();
			
	return noErr;
}

// update user items
long myCDEV::Update(void)
{
	Handle	h;
	Rect	r;
	short	type;
	GrafPtr	savePort;
	
	// since this can get called from a variety of places, I want to make sure that
	// we are drawing into the correct window.
	GetPort(&savePort);
	SetPort(fDialog);
	
	// get and draw the time sting
	GetDItem(fDialog,fLastItem+kUserItem,&type,&h,&r);
	TextBox(&timeString[1],timeString[0],&r,teFlushDefault);
	
	// restore the original drawing port
	SetPort(savePort);
	
	return noErr;
}

// activate user items
long myCDEV::Activate(void)
{
	activated=true;	// we are being brought into the forground.
	lastTime=0;		// force us to update on the next idle call

	return noErr;
}

// deactivate user items
long myCDEV::Deactivate(void)
{
	activated=false;	// we are being pushed into the background
	
	// get the deactivated text and draw it
	GetIndString(timeString,kBaseID,kDeactivatedText);
	Update();

	return noErr;
}

// undo from edit menu or cmd-z
long myCDEV::Undo(void)		
{
	Str255	s;
	
	// pop up an alert letting the user know that we don't support undo
	GetIndString(s,kBaseID,kAlertText);
	ParamText(s,nil,nil,nil);
	NoteAlert(kBaseID+1,nil);
	
	return noErr;
}

// gets the current time and puts it into a string if it's not the same as the time
// we currently have
Boolean myCDEV::getTime(void)
{
	unsigned long	nuTime;
	Boolean			result=false;	// assume no update
	
	// get the time it is now
	GetDateTime(&nuTime);
	
	// if it's different than the last posted time then...
	if (nuTime != lastTime) {
	
		// create a time string based on the users settings
		IUTimeString(nuTime,showSeconds,timeString);
		
		// lastTime is now the current time
		lastTime=nuTime;
		
		// let the caller know that we updated the time
		result=true;
	}
	
	return result;
}

// check and unchecks the checkbox
void myCDEV::setCheckBox(void)
{
	short	type;
	Handle	h;
	Rect	r;
	
	// get the control handle to the checkbox and set to to whatever
	// showSeconds is.
	GetDItem(fDialog,fLastItem+kCheckBox,&type,&h,&r);
	SetCtlValue((ControlHandle)h,showSeconds);
}
