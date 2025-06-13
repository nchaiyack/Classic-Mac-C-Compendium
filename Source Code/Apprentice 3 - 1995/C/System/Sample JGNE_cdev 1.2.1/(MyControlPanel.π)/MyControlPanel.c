/*
	MyControlPanel.c

	Written by Ken Worley, 06/03/94, using Symantec Think C 7.0.
	Copyright 1994.
	AOL KNEworley
	internet KNEworley@aol.com
	
	Feel free to use this code in a control panel of your own, but give me proper
	credit in your documentation or about box.  Feel free to distribute this
	code in its entirety to anyone, but never do so without the copyright notice
	above nor without its accompanying files.  This code is NOT in the public
	domain.  Use of this code in a commercial product requires my permission.
	
	This control panel is designed (at this time) to work in the Sample JGNE/cdev
	project/package.
	
	This control panel uses no global variables, so it's not necessary
	to set up and restore A4.  All of our 'global' data is in the shared data
	structure myData (in a pointer to a block in the system heap).
	
	The Control Panel Manager sends messages to your control panel through
	its main routine when it is opened, closed, or manipulated in any way that you
	would need to respond to.
	
	The routines already set up here will handle ALL of the messages that
	would be received.  Most of them, at the present time, don't do anything
	since this is only an example.  By simply inserting code into the routines,
	however, you can add functionality.
	
	The three most common messages received are:
	
		initDev		We receive this when the user opens our control panel (or
					selects it in the scrolling list of control panels in System 6).
		
		closeDev	We receive this when the user closes the control panel (or
					selects a different control panel in System 6).
					
		hitDev		The user has manipulated a control in the control panel (i.e.
					pressed a button or clicked in a text field, etc.)
					
		Other messages are received when the user selects items in the Edit menu,
		when the control panel is sent to the background or brought to the foreground,
		when the user types, and when the control panel needs to be updated.  Controls
		in the panel's DITL (item list) are updated automatically.
		
	The cdevValue parameter of main is a mechanism that allows you to store data
	across calls to the control panel.  Normally, when an initDev message is received,
	we would allocate a new handle and return the handle as the function's
	return value (unless an error occurred).  The next time main is called by the
	Control Panel Manager, the same value is passed back to us in cdevValue so that
	we can access the memory.
	
	Normally, we don't need to actually allocate anything here, but
	we still use the mechanism to retain the address of our shared data structure
	between calls.  When a closeDev message is received, we dispose of
	the memory block and return cdevUnset to indicate no handle is allocated.
	
	If, however, the INIT wasn't loaded/run at startup time, our shared data
	structure was never allocated.  If this happens, we allocate storage for
	it here so that the control panel can still run normally and preferences
	can be stored.
	
	Other possible return values are:
	
		cdevUnset	Control Panel Manager need not keep track of any memory
		cdevGenErr	generic error - no error dialog is shown but panel does not load
		cdevMemErr	not enough memory to continue - error dialog shown
		cdevResErr	a resource is unavailable - error dialog shown
		A handle to some memory for the Control Panel Manager to keep track of
			between calls.
	
	Instead of allocating new memory, we look up the address of our shared memory
	structure in a resource created by the extension code.  We use the cdevValue
	mechanism to save that as long as the control panel is open.  This memory
	was initially allocated by the extension code (which stored the address
	in the resource).  This resource is of type 'memA'.
	
	I use a resource of type 'pref' to hold the control panel's "preferences."  At
	this time, the resource is a struct containing only two fields:  On and
	ShowIcon.  'On' indicates whether the patch should take action when trapped to and
	'ShowIcon' indicates whether the extension's icon should be shown at system
	startup time.
	
	If the user opens the control panel when the INIT hasn't run yet, the on/off
	setting is automatically set to off when the control panel is opened.  That
	way, when the user turns it on, a dialog is shown warning that changes take
	effect on restart.  This will happen EVERY time the control panel is opened
	until it is actually loaded at startup time.  So, if the user turns the control
	panel on (and it wasn't loaded at startup), then closes it and opens it again,
	it will once again be off.  It didn't turn off again, though, until the control
	panel was opened again.  Seemed like a good thing to do at the time...
	
	Because of the fact that we release our resource every time the control panel is
	closed, if you need to access values manipulated by the control panel in the
	patch code for example, you may want to duplicate the final values in the
	shared data structure.  It would be most convenient to do this in the CloseCP
	routine just before the resource is released.  I store the on/off status of
	the control panel in the field 'CPon' in the shared data.  The patch code uses
	this field to determine if it should take any action.  The control panel also
	refers to the field 'patched' to determine if the patch was installed at
	system startup.
	
	Any resources used by a control panel must be in the range -4064 thru -4033.
	The only exception has to do with balloon help.  Of that range, -4064 thru -4049
	is reserved for standard resources (and some optional resources).  You can use
	-4048 thru -4033 for any resources you include for use with your control panel.
	This is from the New Inside Macintosh: More Macintosh Toolbox (chapter 8).
	
	Coordinates in your control panel dialog:
	
		System 7 lets you use almost any size control panel you wish.  System 6 and
		earlier (which uses the Control Panel desk accessory) requires the control
		panel to be in the rectangle (-1, 87, 255, 322).  In both cases, the origin
		of your control panel should be at (-1, 87).  This leaves space for the
		scrolling list of icons in the Control Panel desk accessory and is left
		off in System 7.
	
	The 'nrct' resource:
	
		This resource contains coordinates for rectangles within your control
		panel.  The Control Panel Manager will draw a 2-pixel box around each
		rectangle defined in this resource.  You may include only one rectangle
		if you like (to surround the entire control panel), or you can define 
		several to separate different sets of controls.  This example includes
		three rectangles:  one surrounds the on and off radio buttons, another
		surrounds the PICT at the top of the control panel, and another
		surrounds the rest of the controls/items.  This resource should have an
		ID number of -4064.  If the rectangles you define do not form a rectangle
		when put together, the extra space is filled in with a gray pattern.
	
	The 'DITL' resource:
	
		This is a standard dialog item list resource that defines all the controls
		and other items in your control panel.  The ID numbers you give each item
		are used to identify those items in your code that responds to the hitDev
		message.  This resource should have an ID number of -4064.
		
		NOTE:  According to the New Inside Macintosh:  More Macintosh Toolbox,
		chapter 8 (Control Panels), if you use a font other than the standard 
		application font (if you define an 'finf' resource), you must define
		text as user items rather than static text items in order for the
		control panel to run in the Control Panel Desk Accessory in System 6
		and earlier.
		
	The 'ICN#' (and related) resources:
		
		This is simply the icon that represents the control panel in the finder.
		The ICN# resource actually refers to other resources that define icons
		for different screen depths and statuses.  It is sometimes referred to
		as an 'icon family' resource.  This resource must have an ID number of
		-4064 and must be purgeable.
		
		The Mac does NOT automatically draw your control panel's icon on the
		screen during startup.  We use a routine called ShowIconFamily in our
		extension code to do this.
	
	The 'mach' resource:
	
		This resource indicates to the Control Panel Manager what type of machine
		this control panel runs on or if it needs to check with the control panel
		for that.  It has these possible values (in hex, 4 bytes):
		
			0000 FFFF	Call control panel with macDev message.  The control panel
						will return true if it can run on this machine and false if
						it cannot.  This example code will simply return true if
						called with the macDev message.
			
			3FFF 0000	Runs on Mac II systems only.
			
			7FFF 0400	Runs on all Macs with an ADB (Apple Desktop Bus).
			
			FFFF 0000	Runs on all Macs.
	
	The 'finf' resource:
	
		This resource specifies a font other than the standard application font
		to be used for drawing the text in your control panel.  This only works
		in System 7.  In earlier systems, define text as user items instead of
		static text items and draw the text in whatever font you wish.  This
		resource should have an ID number of -4049.
		
		For purposes of this example, I did use static text items and an 'finf'
		resource.  In system 6 and earlier, text will be drawn in the standard
		application font.
		
	Of course, your control panel will need to include a file reference (FREF)
	resource, bundle (BNDL) resource, and signature resource.  Refer to your
	development system documentation or Inside Macintosh for information on
	these resources.
*/


#define	LOCKP	HLock( (Handle)myData->CPprefsRsrc )	/* lock handle to prefs rsrc */
#define	UNLOCKP	HUnlock( (Handle)myData->CPprefsRsrc )	/* unlock same */

#include "SharedData.h"			/* Includes the definition of the data */
								/* structure shared with the extension */
								/* and task code */

/* about box item defines */

#define	kAboutDialogClrID		-4048
#define	kAboutDialogBWID		-4045
#define kAboutOKButton		1
#define kAboutTextFld		2

/* changes after restart dialog defines */

#define kRestartDialogID	-4047
#define kRestartOKButton	1
#define kRestartText		2

/* control panel unavailable dialog defines */

#define kUnavailDialogID	-4046
#define kUnavailOKButton	1
#define kUnavailText		2

/* control panel item defines */

#define	kCPAboutButton		1
#define kCPShowIconBox		2
#define kCPOnRadioButton	3
#define kCPOffRadioButton	4
#define kCPShiftBox			5
#define kCPCommandBox		6
#define kCPOptionBox		7
#define kCPControlBox		8


/* prototypes */

pascal long	main( short message, short item, short numItems, short cpID,
	EventRecord *evt, long cdevValue, DialogPtr theCP );
myDataPtr	InitCP( DialogPtr theCP );
void	CloseCP( myDataPtr myData );
void	Hit( myDataPtr myData, short whichItem );
void	Twiddle( myDataPtr myData );
void	Update( myDataPtr myData );
void	Activate( myDataPtr myData );
void	Deactivate( myDataPtr myData );
void	KeyPress( myDataPtr myData, EventRecord *event );
void	Undo( myDataPtr myData );
void	Cut( myDataPtr myData );
void	Copy( myDataPtr myData );
void	Paste( myDataPtr myData );
void	Clear( myDataPtr myData );
void	About( myDataPtr myData );
void	DoRestartDialog( void );
void	DoUnavailDialog( void );
void	GetSysInformation( myDataPtr myData );

/* Functions */

pascal long	main( short message, short item, short numItems, short cpID,
	EventRecord *evt, long cdevValue, DialogPtr theCP )
{
	myDataPtr		myData;		/* address of my shared data structure */
	long			result;		/* result to return */
	char			theChar;	/* used to handle some key presses */

	/*	This function gets called every time any action is taken in this control
	 *	panel (including opening & closing).  If this isn't the first call to
	 *	the control panel (just opening), the value we need for myData should
	 *	be passed to us in cdevValue.
	 */
	 	if ( message == initDev )
	 		myData = NULL;
	 	else
	 		myData = (myDataPtr)cdevValue;
	
	/*	The macDev message is sent if the Mac wants to see if we should be
	 *	showing in the Control Panel.  Since we want to be run on any machine,
	 *	we do no checking; we just return true.
	 */
	 
		if ( message == macDev )
			return 1L;
	
	/*	Here, we decide what to do based on the 'message' received from the
	 *	Control Panel.
	 */
	 
	 	result = (long)myData;	/* Unless there's an error, always return myData */
	 	
		switch ( message )
		{
			case initDev:	/* initialize - user opened this control panel */
				myData = InitCP( theCP );
				if ( myData )
				{
					result = (long)myData;
				}
				else
				{
					DoUnavailDialog();
					result = cdevGenErr;
				}
				break;

			case closeDev:	/* close - last call to this device before closing */
				CloseCP( myData );
				break;
				
			case hitDev:	/* user has pressed mouse button on an item */
				Hit( myData, item - numItems );
				break;
				
			case nulDev:	/* nothing else to report - so twiddle */
				Twiddle( myData );
				break;
				
			case updateDev:	/* update panel window (redraw controls, etc.) */
				Update( myData );
				break;
				
			case activDev:	/* becoming active after being in background */
				Activate( myData );
				break;
				
			case deactivDev:	/* becoming inactive after being in foreground */
				Deactivate( myData );
				break;
				
			case keyEvtDev:	/* user has pressed a key */
				/* Check to see if the command key was down for menu equivalents */
					if ( ( evt->what != autoKey ) && ( evt->modifiers & cmdKey ) )
					{
						theChar = ( evt->message & charCodeMask );
						switch ( theChar )
						{
							case 'z':			/* menu command equivalents */
							case 'Z':
								Undo( myData );		/* undo */
								break;
							case 'x':
							case 'X':
								Cut( myData );		/* cut */
								break;
							case 'c':
							case 'C':
								Copy( myData );		/* copy */
								break;
							case 'v':
							case 'V':
								Paste( myData );	/* paste */
								break;
						}
					}
					else
					{
						KeyPress( myData, evt );	/* process other keystroke */
					}
				break;
				
			case undoDev:	/* user picked UNDO from the Edit menu */
				Undo( myData );
				break;
				
			case cutDev:	/* user picked CUT from the Edit menu */
				Cut( myData );
				break;
				
			case copyDev:	/* user picked COPY from the Edit menu */
				Copy( myData );
				break;
				
			case pasteDev:	/* user picked PASTE from the Edit menu */
				Paste( myData );
				break;
				
			case clearDev:	/* user picked CLEAR from the Edit menu */
				Clear( myData );
				break;
		}
	return result;
}


myDataPtr	InitCP( DialogPtr theCP )
{
	short		itemType;		/* these 3 local vars that can be used to */
	Handle		itemHandle;		/* manipulate items in the control     */
	Rect		itemRect;		/* panel dialog */
	myDataPtr	myData;			/* a pointer to our shared data structure */
	
	memAddrHdl	memAddr;		/* Handle to the rsrc containing the addr of our */
								/* shared memory data structure */
	CPprefsPtr	myPrefs;		/* pointer to our preferences resource struct */
	
	/*	At this point, the control panel has just been opened. */
	
	/*	Attempt to load the resource containing the addr of our shared memory struct */
	
		memAddr = (memAddrHdl)Get1Resource( kMemAddrType, kMemAddrID );
		
		if ( !memAddr )		/* could not load resource - INIT didn't run */
		{
			/* allocate memory for myData */
			
			myData = (myDataPtr)NewPtrSys( sizeof( myDataStruct ) );
			
			if ( myData )
			{
				myData->INITrun = false;
				GetSysInformation( myData );
			}
			else
			{
				return NULL;	//error cannot allocate storage for myData
			}
		}
		else	/* addr rsrc was loaded - check to see if it's valid */
		{
			myData = (myDataPtr)((*memAddr)->theAddr); /* get addr of shared memory */
			ReleaseResource( (Handle)memAddr );	/* release the rsrc - we have the info */
			
			/* check the checkValue field of the shared data to make sure */
			/* we REALLY have the real shared data */
			
			if ( myData->checkValue != 0x12341234 )	// INIT didn't run
			{
				/* allocate memory for myData */
				
				myData = (myDataPtr)NewPtrSys( sizeof( myDataStruct ) );
				
				if ( myData )
					myData->INITrun = false;
				else
					return NULL;	//error cannot alloc storage for myData
			}
		}
			
	/*	Load the control panel preferences resource.  Leave it unlocked except when
	 *	when we need to lock it.  When the control panel is closed, the resource
	 *	will be released.
	 */
		
		myData->CPprefsRsrc =
			(CPprefsHandle)Get1Resource( kCPprefsRsrcType, kCPprefsRsrcID );
		
		if ( myData->CPprefsRsrc )
		{
			LOCKP;								/* it's there - lock it down and */
			myPrefs = (*(myData->CPprefsRsrc));	/* dereference for easy access */
												/* since it's locked */
		}
		else	/* it's not there - create a new resource */
		{
			myData->CPprefsRsrc =
				(CPprefsHandle)NewHandleClear( sizeof( CPprefsStruct ) );
			
			if ( myData->CPprefsRsrc )
			{
				LOCKP;									/* lock the new handle */
				myPrefs = (*(myData->CPprefsRsrc));	/* dereference for easy access */
														/* since it's locked */
				
				myPrefs->On = true;					/* default is to set both of */
				myPrefs->ShowIcon = true;			/* these to true */
				myPrefs->modifiers = controlKey + optionKey + shiftKey;	/* defaults */
				
				AddResource( (Handle)myData->CPprefsRsrc, kCPprefsRsrcType,
					kCPprefsRsrcID, "\pPreferences" );
			}
			else	/* unable to create new Handle for resource */
			{
				return NULL;			/* exit with an error */
			}
		}
	
	// If the INIT was not run, set the control panel to "off".  When the user
	// turns the control panel back "on," the changes on restart dialog will be
	// shown.
	
		if ( !myData->INITrun )
		{
			myPrefs->On = false;
		}
		
	/*	Save the dialog ptr value for access globally through myData */
	
		myData->CPdlg = theCP;
		
	/*	Set all controls to their correct values. */
	
		/* Get the ON button item */
		
			GetDItem( myData->CPdlg, kCPOnRadioButton,
				&itemType, &itemHandle, &itemRect );
				
			if ( myPrefs->On )
				SetCtlValue( (ControlHandle)itemHandle, 1 );
			else
				SetCtlValue( (ControlHandle)itemHandle, 0 );
				
		/* Get the OFF button item */
		
			GetDItem( myData->CPdlg, kCPOffRadioButton,
				&itemType, &itemHandle, &itemRect );
				
			if ( myPrefs->On )
				SetCtlValue( (ControlHandle)itemHandle, 0 );
			else
				SetCtlValue( (ControlHandle)itemHandle, 1 );
				
		/* Get the Show Icon check box item */
		
			GetDItem( myData->CPdlg, kCPShowIconBox,
				&itemType, &itemHandle, &itemRect );
			
			if ( myPrefs->ShowIcon )
				SetCtlValue( (ControlHandle)itemHandle, 1 );
			else
				SetCtlValue( (ControlHandle)itemHandle, 0 );

		/* Get the Shift key check box item */
		
			GetDItem( myData->CPdlg, kCPShiftBox,
				&itemType, &itemHandle, &itemRect );
			
			if ( myPrefs->modifiers & shiftKey )
				SetCtlValue( (ControlHandle)itemHandle, 1 );
			else
				SetCtlValue( (ControlHandle)itemHandle, 0 );

		/* Get the Command key check box item */
		
			GetDItem( myData->CPdlg, kCPCommandBox,
				&itemType, &itemHandle, &itemRect );
			
			if ( myPrefs->modifiers & cmdKey )
				SetCtlValue( (ControlHandle)itemHandle, 1 );
			else
				SetCtlValue( (ControlHandle)itemHandle, 0 );

		/* Get the Option key check box item */
		
			GetDItem( myData->CPdlg, kCPOptionBox,
				&itemType, &itemHandle, &itemRect );
			
			if ( myPrefs->modifiers & optionKey )
				SetCtlValue( (ControlHandle)itemHandle, 1 );
			else
				SetCtlValue( (ControlHandle)itemHandle, 0 );

		/* Get the Control key check box item */
		
			GetDItem( myData->CPdlg, kCPControlBox,
				&itemType, &itemHandle, &itemRect );
			
			if ( myPrefs->modifiers & controlKey )
				SetCtlValue( (ControlHandle)itemHandle, 1 );
			else
				SetCtlValue( (ControlHandle)itemHandle, 0 );

	/*	Unlock the prefs rsrc, but don't allow it to be purged */
	
		HNoPurge( (Handle)myData->CPprefsRsrc );
		UNLOCKP;

	return myData;
}


void	CloseCP( myDataPtr myData )
{
	/*	Here, we would want to destroy anything we created while the control panel
	 *	was "up."  The only thing left should be the shared data structure (which
	 *	we only destroy if the INIT didn't run and we had to create storage
	 *	for the data here).  Leaving things lying around in memory can only lead
	 *	to system bombs.  If we wanted to save any settings to disk, this would
	 *	be the place to do it.
	 */
	 
	 CPprefsHandle		prefsHandle;

	 /* Write our preferences resource back to the resource file */
	 
	 	prefsHandle = myData->CPprefsRsrc;
	 	ChangedResource( (Handle)prefsHandle );
	 	WriteResource( (Handle)prefsHandle );
	 
	 /* Save the on/off status of the control panel in the CPon field of the */
	 /* shared data structure. */
	 
	 	myData->CPon = (*prefsHandle)->On;
	 
	 /* Save the modifier keys marked in the control panel also */
	 
	 	myData->CPmodifiers = (*prefsHandle)->modifiers;
	 	
	 /* Unlock the resource and release it.  We'll reload it next time we're opened. */
	 
	 	UNLOCKP;
	 	ReleaseResource( (Handle)prefsHandle );
	 	myData->CPprefsRsrc = NULL;
	 	prefsHandle = NULL;
	 
	 /* If the INIT didn't run and myData was created here, destroy it now */
	 
	 	if ( !myData->INITrun )
	 		DisposPtr( (Ptr)myData );
}


void	Hit( myDataPtr myData, short whichItem )
{
	/*	The user has pressed a button or clicked in a text editing field in the
	 *	control panel.  Handle the 'hit' here.
	 */
	 
	short		itemType;		/* these 3 local vars that can be used to */
	Handle		itemHandle;		/* manipulate items in the control     */
	Rect		itemRect;		/* panel dialog */

	CPprefsHandle	prefsHandle;	/* used to more conveniently access the prefs rsrc */
	
	LOCKP;
	prefsHandle = myData->CPprefsRsrc;
	
	switch ( whichItem )
	{
		case kCPAboutButton:
			About( myData );
			break;
			
		case kCPOnRadioButton:
			if ( !(*prefsHandle)->On )	/* if off */
			{
				GetDItem( myData->CPdlg, kCPOnRadioButton,
					&itemType, &itemHandle, &itemRect );
				SetCtlValue( (ControlHandle)itemHandle, 1 );
				
				GetDItem( myData->CPdlg, kCPOffRadioButton,
					&itemType, &itemHandle, &itemRect );
				SetCtlValue( (ControlHandle)itemHandle, 0 );
				
				(*prefsHandle)->On = true;
				
				// if the task was not installed, or the INIT didn't run,
				// warn user that changes won't take effect until restart
				
					if ( !myData->taskInstalled || !myData->INITrun )
						DoRestartDialog();
			}
			break;
			
		case kCPOffRadioButton:
			if ( (*prefsHandle)->On )	/* if on */
			{
				GetDItem( myData->CPdlg, kCPOnRadioButton,
					&itemType, &itemHandle, &itemRect );
				SetCtlValue( (ControlHandle)itemHandle, 0 );
				
				GetDItem( myData->CPdlg, kCPOffRadioButton,
					&itemType, &itemHandle, &itemRect );
				SetCtlValue( (ControlHandle)itemHandle, 1 );
				
				(*prefsHandle)->On = false;
			}
			break;

		case kCPShowIconBox:
			GetDItem( myData->CPdlg, kCPShowIconBox,
				&itemType, &itemHandle, &itemRect );

			if ( (*prefsHandle)->ShowIcon )	/* if on */
			{
				SetCtlValue( (ControlHandle)itemHandle, 0 );		/* turn it off */
				(*prefsHandle)->ShowIcon = false;
			}
			else
			{
				SetCtlValue( (ControlHandle)itemHandle, 1 );		/* turn it on */
				(*prefsHandle)->ShowIcon = true;
			}
			break;

		case kCPShiftBox:
			GetDItem( myData->CPdlg, kCPShiftBox,
				&itemType, &itemHandle, &itemRect );

			if ( (*prefsHandle)->modifiers & shiftKey )	/* if on */
				SetCtlValue( (ControlHandle)itemHandle, 0 );		/* turn it off */
			else										/* else */
				SetCtlValue( (ControlHandle)itemHandle, 1 );		/* turn it on */
				
			(*prefsHandle)->modifiers ^= shiftKey;		/* reverse shift key bit */
			
			break;

		case kCPCommandBox:
			GetDItem( myData->CPdlg, kCPCommandBox,
				&itemType, &itemHandle, &itemRect );

			if ( (*prefsHandle)->modifiers & cmdKey )	/* if on */
				SetCtlValue( (ControlHandle)itemHandle, 0 );		/* turn it off */
			else										/* else */
				SetCtlValue( (ControlHandle)itemHandle, 1 );		/* turn it on */
			
			(*prefsHandle)->modifiers ^= cmdKey;		/* reverse command key bit */
			break;

		case kCPOptionBox:
			GetDItem( myData->CPdlg, kCPOptionBox,
				&itemType, &itemHandle, &itemRect );

			if ( (*prefsHandle)->modifiers & optionKey )	/* if on */
				SetCtlValue( (ControlHandle)itemHandle, 0 );		/* turn it off */
			else											/* else */
				SetCtlValue( (ControlHandle)itemHandle, 1 );		/* turn it on */
				
			(*prefsHandle)->modifiers ^= optionKey;		/* reverse option key bit */
			
			break;

		case kCPControlBox:
			GetDItem( myData->CPdlg, kCPControlBox,
				&itemType, &itemHandle, &itemRect );

			if ( (*prefsHandle)->modifiers & controlKey )	/* if on */
				SetCtlValue( (ControlHandle)itemHandle, 0 );		/* turn it off */
			else											/* else */
				SetCtlValue( (ControlHandle)itemHandle, 1 );		/* turn it on */
				
			(*prefsHandle)->modifiers ^= controlKey;	/* reverse control key bit */
			
			break;
	}
	UNLOCKP;
}


void	Twiddle( myDataPtr myData )
{
	/*	The user isn't doing anything at the moment, so take this time to do
	 *	odds and ends (if needed) in this idle time.
	 */


}


void	Update( myDataPtr myData )
{
	/*	We need to redraw the panel because something has screwed it up (like
	 *	another window moved from in front of it).  This is the same as in any
	 *	application except we don't call BeginUpdate() and EndUpdate().  The
	 *	Dialog Manager should take care of redrawing any controls.
	 */
	 
}


void	Activate( myDataPtr myData )
{
	/*	We're becoming active either because we were just opened, or we were in
	 *	the background and we're being brought to the foreground.  Take care of
	 *	hiliting text fields or list items or whatever.
	 */
	 
}


void	Deactivate( myDataPtr myData )
{
	/*	We're becoming inactive because we're being sent to the background.  Take
	 *	care of unhiliting text fields, list items, or whatever.
	 */

}


void	KeyPress( myDataPtr myData, EventRecord *event )
{
	/*	The user has pressed a key on the keyboard (or a combination of keys).  In
	 *	any case, a keyboard event has been generated.  The main routine has already
	 *	checked for command key menu equivalents, so any real typing is handled
	 *	here (possibly by passing it on to textEdit if you have text fields).
	 *	I saw a note from Symantec in their cdev example that said text fields don't
	 *	seem to work correctly in control panels, so they convert theirs to user
	 *	items (at least while the panel's open).  I haven't tried it.
	 */

}


void	Undo( myDataPtr myData )
{
	/*	User has selected UNDO from the Edit menu.  Undo the last action here if
	 *	it's appropriate and you've implemented an undo.  Otherwise, ignore.
	 */

	SysBeep( 3 );
}


void	Cut( myDataPtr myData )
{
	/*	User has selected CUT from the Edit menu.  If you have a text field that
	 *	is active, pass this along to textEdit.  Otherwise, ignore.
	 */
	
	SysBeep( 3 );
}


void	Copy( myDataPtr myData )
{
	/*	User has selected COPY from the Edit menu.  If you have a text field that
	 *	is active, pass this along to textEdit.  Otherwise, ignore.
	 */
	
	SysBeep( 3 );
}


void	Paste( myDataPtr myData )
{
	/*	User has selected PASTE from the Edit menu.  If you have a text field that
	 *	is active, pass this along to textEdit.  Otherwise, ignore.
	 */
	
	SysBeep( 3 );
}


void	Clear( myDataPtr myData )
{
	/*	User has selected CLEAR from the Edit menu.  If you have a text field that
	 *	is active, pass this along to textEdit.  Otherwise, ignore.
	 */
	
	SysBeep( 3 );
}

void	About( myDataPtr myData )
{
	/*	This routine displays an about box for this control panel. */
	
	WindowPtr		winMgrPort;		/* store current window mgr port here */
	WindowPtr		currentPort;	/* store the current port here */
	DialogPtr		aboutDlg;		/* to store our dialog in */
	
	short		itemType;		/* these 3 local variables are used to */
	Handle		itemHandle;		/* manipulate items in the dialog.     */
	Rect		itemRect;

	short		itemHit;		/* use with ModalDialog */
	Point		refPt;			/* a reference point */
	
	/*	Get the window manager port and the current port (our control panel)
	 *	Then convert the upper left corner of the control panel to a global
	 *	point for referencing our about dialog.
	 */
	 
	 	GetWMgrPort( &winMgrPort );
	 	GetPort( &currentPort );
	 	
	 	SetPt( &refPt, 0, 0 );
	 	LocalToGlobal( &refPt );
	 	refPt.h += 93;		/* move to right over actual panel */
	 	
	/*	Now, load our dialog resource, move it on top of the control panel,
	 *	make it the current port, then make it visible (the resource should
	 *	mark the dialog as NOT initially visible.
	 */
		
		if ( myData->hasColor )
			aboutDlg = GetNewDialog( kAboutDialogClrID, NULL, (WindowPtr)-1L );
		else
			aboutDlg = GetNewDialog( kAboutDialogBWID, NULL, (WindowPtr)-1L );
		
		/* This is an example - move your about dialog wherever you want */
		
		MoveWindow( aboutDlg, refPt.h, refPt.v, true );
		
		SetPort( aboutDlg );
		
		ShowWindow( aboutDlg );
	
	 /*	Get the OK button item and draw a bold border around it to show that
	  *	it's the default button.
	  */
	 
	 	GetDItem( aboutDlg, kAboutOKButton, &itemType, &itemHandle, &itemRect );
	 	InsetRect( &itemRect, -4, -4 );
	 	PenSize( 3, 3 );
	 	FrameRoundRect( &itemRect, 16, 16 );
	 	PenSize( 1, 1 );
	 
	 /*	Loop and call ModalDialog until the user presses the OK button */
	 
	 	ModalDialog( NULL, &itemHit );
	 	while ( itemHit != kAboutOKButton )
	 		ModalDialog( NULL, &itemHit );
	 	
	 /*	Now get rid of the dialog and set the port back to the control panel */
	 
	 	DisposDialog( aboutDlg );
	 	SetPort( currentPort );
}

void	DoRestartDialog( void )
{
	/*	This routine displays a dialog that tells the user that changes made
	 *	won't take effect until after the computer is restarted.
	 */
	
	WindowPtr		winMgrPort;		/* store current window mgr port here */
	WindowPtr		currentPort;	/* store the current port here */
	DialogPtr		theDlg;			/* to store our dialog in */
	
	short		itemType;		/* these 3 local variables are used to */
	Handle		itemHandle;		/* manipulate items in the dialog.     */
	Rect		itemRect;

	short		itemHit;		/* use with ModalDialog */
	Point		refPt;			/* a reference point */
	
	/*	Get the window manager port and the current port (our control panel)
	 *	Then convert the upper left corner of the control panel to a global
	 *	point for referencing our dialog.
	 */
	 
	 	GetWMgrPort( &winMgrPort );
	 	GetPort( &currentPort );
	 	
	 	SetPt( &refPt, 0, 0 );
	 	LocalToGlobal( &refPt );
	 	refPt.h += 107;		/* move to right over actual panel */
	 	refPt.v += 80;		/* also move it down some */
	 	
	/*	Now, load our dialog resource, move it on top of the control panel,
	 *	make it the current port, then make it visible (the resource should
	 *	mark the dialog as NOT initially visible.
	 */
	
		theDlg = GetNewDialog( kRestartDialogID, NULL, (WindowPtr)-1L );
		
		/* This is an example - move your about dialog wherever you want */
		
		MoveWindow( theDlg, refPt.h, refPt.v, true );
		
		SetPort( theDlg );
		
		SysBeep( 3 );
		ShowWindow( theDlg );
	
	 /*	Get the OK button item and draw a bold border around it to show that
	  *	it's the default button.
	  */
	 
	 	GetDItem( theDlg, kRestartOKButton, &itemType, &itemHandle, &itemRect );
	 	InsetRect( &itemRect, -4, -4 );
	 	PenSize( 3, 3 );
	 	FrameRoundRect( &itemRect, 16, 16 );
	 	PenSize( 1, 1 );
	 
	 /*	Loop and call ModalDialog until the user presses the OK button */
	 
	 	ModalDialog( NULL, &itemHit );
	 	while ( itemHit != kRestartOKButton )
	 		ModalDialog( NULL, &itemHit );
	 	
	 /*	Now get rid of the dialog and set the port back to the control panel */
	 
	 	DisposDialog( theDlg );
	 	SetPort( currentPort );
}


void	DoUnavailDialog( void )
{
	/*	This routine displays a dialog that tells the user that the control
	 *	panel cannot be accessed because it needs to be loaded at startup.
	 */
	
	WindowPtr		winMgrPort;		/* store current window mgr port here */
	WindowPtr		currentPort;	/* store the current port here */
	DialogPtr		theDlg;			/* to store our dialog in */
	
	short		itemType;		/* these 3 local variables are used to */
	Handle		itemHandle;		/* manipulate items in the dialog.     */
	Rect		itemRect;

	short		itemHit;		/* use with ModalDialog */
	Point		refPt;			/* a reference point */
	
	/*	Get the window manager port and the current port (our control panel)
	 *	Then convert the upper left corner of the control panel to a global
	 *	point for referencing our dialog.
	 */
	 
	 	GetWMgrPort( &winMgrPort );
	 	GetPort( &currentPort );
	 	
	 	SetPt( &refPt, 0, 0 );
	 	LocalToGlobal( &refPt );
	 	refPt.h += 107;		/* move to right over actual panel */
	 	refPt.v += 80;		/* also move it down some */
	 	
	/*	Now, load our dialog resource, move it on top of the control panel,
	 *	make it the current port, then make it visible (the resource should
	 *	mark the dialog as NOT initially visible.
	 */
	
		theDlg = GetNewDialog( kUnavailDialogID, NULL, (WindowPtr)-1L );
		
		/* This is an example - move your about dialog wherever you want */
		
		MoveWindow( theDlg, refPt.h, refPt.v, true );
		
		SetPort( theDlg );
		
		SysBeep( 3 );
		ShowWindow( theDlg );
	
	 /*	Get the OK button item and draw a bold border around it to show that
	  *	it's the default button.
	  */
	 
	 	GetDItem( theDlg, kUnavailOKButton, &itemType, &itemHandle, &itemRect );
	 	InsetRect( &itemRect, -4, -4 );
	 	PenSize( 3, 3 );
	 	FrameRoundRect( &itemRect, 16, 16 );
	 	PenSize( 1, 1 );
	 
	 /*	Loop and call ModalDialog until the user presses the OK button */
	 
	 	ModalDialog( NULL, &itemHit );
	 	while ( itemHit != kUnavailOKButton )
	 		ModalDialog( NULL, &itemHit );
	 	
	 /*	Now get rid of the dialog and set the port back to the control panel */
	 
	 	DisposDialog( theDlg );
	 	SetPort( currentPort );
}

void	GetSysInformation( myDataPtr myData )
{
	SysEnvRec		env;
			
	/* Find out if this machine has color QuickDraw and find system vers. */
	
		SysEnvirons( 2, &env );
		
		myData->hasColor = env.hasColorQD;
		
		if ( env.systemVersion >= 0x0700 )
			myData->sysVersion = 7;
		else if ( env.systemVersion >= 0x0600 )
			myData->sysVersion = 6;
		else
			myData->sysVersion = 0;	/* we don't care about anything < 6 */
}
