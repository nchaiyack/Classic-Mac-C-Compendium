/******************************************************************************
 DialogControls.c by Todd Clements (tclement@hmc.edu)
	This demo shows how to do different things with dialogs:
		1) Set the window font and size
		2) Patch NewControl() so that you have controls in the window font
			(thanks to Jens Alfke <jens_alfke@powertalk.apple.com> for the code)
		3) Use Popup Menus in modal dialogs
		4) Use AppendDITL() to get a NewsWatcher-like preferences box
		5) Use styled text edit records from resources to display text
		6) Write a filter for a ModalDialog
		7) Use MovableModal dialog boxes
		
	Some some of this code is stolen from my own program, others stolen from other
	people, but it's all here for you!
	
	If you have any comments about the code, any improvements/additions/bugs,
	please feel free to contact me at the above address.
	
	I make no guarantees regarding the decency of this code.  Run it and use
	it at your own risk.
	
	January 26, 1995	-	1.0 - First release
	May 9, 1995			-	1.1 - Second release
									Changed to MovableModal dialog box
									Added "Reset Panel to Defaults" button
									Disabled controls when window deactivated
******************************************************************************/


#include "DialogControls.h"
#include "Functions.h"
#include "PatchNewControl.h"
#include "MyMenus.h"
#include "MovableModal.h"

/******************************************************************************
	Comments about the resource file for DialogControls.c
	
	The heights of the control items in my dialog have been adjusted to reflect
	the smaller size of the geneva font.  There is probably a standard size I
	should use, but I didn't bother to look it up.  If you care, you should probably
	look it up.  (But, if you care, you'll know that Apple tells you not to use
	custom fonts for control items (although, see just about any System 7.5 control
	panel, and then scratch your head...))
	
	All the controls in the dialog box are the same width, per Apple HIGs (Human
	Interface Guidelines).  This is so that for scripts that read from right
	to left, the controls will appear correctly.
	
	See the note in CheckHamburgerItems() about the true grey in dimmed controls.
	
	Note that the procIDs for the popup menus are different.  1008 is for a standard
	popup.  1016 is for a popup in the window font.
******************************************************************************/

#define		pbOK			1
#define		pbCancel		2
#define		pbRevert		9

#define		rFirstOutline	4
#define		rLastOutline	8

/** Defines for the menus **/
#define		mApple			128
#define		mFile			129
#define		mEdit			130

#define		miAbout			1

#define		miOpen			1
#define		miClose			2
#define		miQuit			7

#define		popmiHotDog		1
#define		popmiHamburger	2

/** Defines for the two windows that have stuff in them **/

#define		ditlHamburger	301
#define		ditlHotDog		300

#define		wPreferences	128

#define		popWhichFood	3

/* This is for hamburger */
#define		rbBeef		1
#define		rbChicken	2
#define		rbOther		3
#define		cbLie		4
#define		cbHamRelish	5
#define		cbHamOnion	6
#define		cbHamKetchup	7
#define		cbHamMustard	8
#define		cbHamTomato		9
#define		cbHamLettuce	10
#define		cbHamPickle		11

/* This is for hot dog */
#define		rbChargeThroughNose	1
#define		rbBeReasonable		2
#define		rbArmAndLeg			3
#define		rbGiveAway			4
#define		cbHotRelish			5
#define		cbHotOnion			6
#define		cbHotKetchup		7
#define		cbHotMustard		8
#define		popHotType			9

#define		popmiRegular		1
#define		popmiPolish			2
#define		popmiKosher			3
#define		popmiVeggie			4


FoodPtr			food;				// Variable storing all checkbox/radio button information
Boolean			gDone = false;		// Should the program terminate?
EventRecord		event;				// The event
TEHandle		aboutText;			// Handle to the text edit record (we need it in various places)


/** This is where all the meat to the program is. =) **/
void	main( void )
{
	InitToolBox();
	SetUpGlobals();
	SetupMenus();
	EventLoop();
}



/** Just your standard routines here **/
void 	InitToolBox( void )
{
	InitGraf(&(qd.thePort));
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);	// I don't remember if this is supposed to be something else�
	InitCursor();

	MaxApplZone();
	MoreMasters();
	MoreMasters();
}


/** Allocate memory for, and initialize our global memory. **/
void	SetUpGlobals( void )
{
	food = (FoodPtr) NewPtr( sizeof( Food ) );
	if( !food ) /* We're all left hungry */
	{
		SysBeep( 10 );
		ExitToShell();
	}
	
	DefaultHotDogSettings( food );
	DefaultHamburgerSettings( food );

}


/** Set the defaults for the Hot Dog settings.  Used for
    initialization and "Reset" button.                    **/
void	DefaultHotDogSettings( FoodPtr theFood )
{
	theFood->hotDog.price = rbGiveAway;
	theFood->hotDog.relish = true;
	theFood->hotDog.onion = true;
	theFood->hotDog.ketchup = true;
	theFood->hotDog.mustard = false;
	theFood->hotDog.hotType = popmiRegular;
}


/** Set the defaults for the Hamburger settings.  Used for
    initialization and "Reset" button.                    **/
void	DefaultHamburgerSettings( FoodPtr theFood )
{
	theFood->ham.hamType = rbBeef;
	theFood->ham.lie = false;
	theFood->ham.relish = true;
	theFood->ham.onion = true;
	theFood->ham.ketchup = true;
	theFood->ham.mustard = false;
	theFood->ham.tomato = false;
	theFood->ham.lettuce = true;
	theFood->ham.pickle = true;
}


/** Get the menus from the resource and display them. **/
void	SetupMenus( void )
{
	Handle		menuBar;
	MenuHandle	menu;
	
	menuBar = GetNewMBar( 128 );
	if( menuBar == nil )
	{
		SysBeep( 10 ); /* Probably should display error */
		ExitToShell();
	}
	
	SetMenuBar( menuBar );
	
	menu = GetMenuHandle( 128 );
	AppendResMenu( menu, 'DRVR' );
	
	// MenuEnable is my own function (see Functions.c) that allows you to use the menu resource number
	// followed by the menu item to enable/disable menu items.  0 as the menu item means the menu itself.
	// You'll see a lot of my canned functions throughout this program.
	MenuEnable( mEdit, 0, false );
	MenuEnable( mFile, miClose, false ); /* We have no use for this menu item*/
	
	DrawMenuBar();
}

/** Where everything exciting gets passed on to other people **/
void	EventLoop( void )
{
	while( !gDone )
	{
		if( WaitNextEvent( everyEvent, &event, 60, nil ) )
		{
			switch( event.what )
			{
				case	mouseDown:
					HandleMouseDown( );
					break;
				case	keyDown:
					HandleKeyDown( );
					break;
				case	activateEvt:
				case	autoKey:
				case	updateEvt:
					break;
				default:
					break;
					
			}
		}
	}
}


/** Did someone press the mouse?
	This function doesn't do much because, well, this program doesn't do much.
	However, a lot of the switches were left in so you could just add your own code
	if you wanted to. **/
void	HandleMouseDown( void )
{
	short		code;
	WindowPtr	window;
	GrafPtr		thePort;
	
	code = FindWindow( event.where, &window );
	
	switch( code )
	{
		case	inMenuBar:
			InterpretMenu( );
			break;
		case	inSysWindow:
			GetPort( &thePort );
			SystemClick( &(event), window );
			SetPort( thePort );
			break;
		case 	inDrag:						//	These are here to show possible
		case 	inGoAway:					//	events you could handle.  I'm
		case	inContent:					//	completely modal, so I don't
		case 	inGrow:						//	have to handle them. 
		case 	inZoomIn: case 	inZoomOut:
			break;
		default:
			break;
	}
}


/** A key was pressed.
	All we care about here is if the key is a menu item equivalent.  If it is, pass
	it to our menu handling function. **/
void	HandleKeyDown( void )
{
	int		menu, menuItem;
	long	theMenu;

	if(( event.modifiers & cmdKey ) != 0 )
	{
		theMenu = MenuKey( event.message & charCodeMask );
		menu = HiWord( theMenu );
		menuItem = LoWord( theMenu );
		HiliteMenu( menu );
		HandleMenu( menu, menuItem );
	}
}

/** If there was a mouseDown in a menu, we have to find out which menu item was selected. **/
void	InterpretMenu( void )
{
	long	menuChoice;
	
	menuChoice = MenuSelect( event.where );
	DoThisMenu( menuChoice );
}

/** If the user chose a desk accessory, launch it. **/
void	HandleDeskAccessory( int	menuItem )
{
	Str255		accName;
	
	MenuHandle	appleMenu = GetMenuHandle( 128 );
	GetMenuItemText( appleMenu, menuItem, accName );
	OpenDeskAcc( accName );
}

/** Needed for the MovableModal thing�it breaks a menu code into a menu and a menu item. **/
void	DoThisMenu( long menuCode )
{
	int		theMenu;
	int		theMenuItem;
	
	theMenu = HiWord( menuCode );
	theMenuItem = LoWord( menuCode );
	
	HandleMenu( theMenu, theMenuItem );
}


/** Here is where we give away all the menu power. **/
void 	HandleMenu( int menu, int menuItem )
{
	switch( menu )
	{
		case mApple:
			switch( menuItem )
			{
				case 1:
					DoAboutMenu();
					break;
				default:
					HandleDeskAccessory( menuItem );
					break;
			}
			break;
		case mFile:
			switch(menuItem)
			{
				case miQuit:
					gDone = true;
					break;
				case miOpen:
					DoTheMagic();
					break;
				default:
					break;
			}
			break;
		case mEdit:
			break;
		default:
			break;
	}
	// The menu manager hilites the menu, but waits for you to unhilite it.
	HiliteMenu( 0 );
}


/** The about menu simply puts up a styled text block and waits for a click. **/
void	DoAboutMenu( void )
{
	Rect		firstRect;
	Handle		firstText;
	StScrpHandle styleHdl;
	DialogPtr	aboutPtr;
	short		hit;
	Boolean		done = false;
	ModalFilterUPP	filter;
	
	// You have to use this function if you are going to be compiling for the PowerPC.
	// In the 68K realm, you can still pass AboutFilter itself to ModalDialog(), but
	// this is the new way of doing it.  It works on both platforms since the compiler
	// knows what to do on either platform.
	filter = NewModalFilterProc( AboutFilter );
	if( !filter ) return;
	
	// Get a new dialog, and set the font of the dialog
	aboutPtr = MyGetNewDialogFont( 129, true, nil, geneva, 10 );
	
	if( aboutPtr  == nil )
	{	SysBeep( 10 ); return; }
	
	// Canned routine - get the rect of a dialog item
	GetDialogItemRect( aboutPtr, 1, &firstRect );
	
	// Create our TextEdit record
	aboutText = TEStyleNew( &firstRect, &firstRect );

	// Center the text
	TESetAlignment( teCenter, aboutText );

	// Look to see if we can find the text we want in the resource.  If we can't, beep
	// angrily, and return.
	firstText = GetResource( 'TEXT', 128 );
	if( !firstText )
	{
		SysBeep( 10 );
		DisposeDialog( aboutPtr );
		return;
	}

	HLock( firstText );

	// Get our style record that matches our text record
	styleHdl = (StScrpHandle)(GetResource( 'styl', 128 ) );
	if( styleHdl )
	{
		// If the style was there, insert it and the text
		TEStyleInsert( *firstText, GetResourceSizeOnDisk(firstText), styleHdl, aboutText );
	}
	else
	{
		// If the style was not there, insert the text all by itself.
		TEInsert( *firstText, GetResourceSizeOnDisk( firstText ), aboutText );
	}

	HUnlock( firstText );

	// Make sure there is an update event generated.
	// The Window Manager will automatically create an update event, but this is to be safe
	InvalRect( &firstRect );
	
	// Always use ReleaseResource() on resource handles, not DisposeHandle()!
	ReleaseResource( (Handle) firstText );
	ReleaseResource( (Handle) styleHdl );
	
	// Simply wait for a click
	while( !done )
	{
		ModalDialog( filter, &hit );
		{
			if( hit != 0 )
			{
				TEDispose( aboutText );
				DisposeDialog( aboutPtr );
				done = true;
			}
		}
	}
	
	// Always be kind to your memory!
	DisposeRoutineDescriptor( (UniversalProcPtr) AboutFilter );
}

/** This is our very simple filter for the about box.  It doesn't do much except check
	for updateEvents, but you can add anything you want to it, including stuff it would
	do during idle time (like scroll some text (See MovieScroll.c from Ken Long) **/
pascal Boolean AboutFilter(DialogPtr inputDialog, EventRecord *myDialogEvent, short *theDialogItem)
{
	Boolean	returnVal = false;

	switch( myDialogEvent->what )
	{
		case updateEvt:			// This is all we care about
		{
			Rect		rect;
			
			BeginUpdate( inputDialog );
			GetDialogItemRect( inputDialog, 1, &rect );
			TEUpdate( &rect, aboutText );
			EndUpdate( inputDialog );
			returnVal = true;
			*theDialogItem = 0;
		}
		break;
		default:
			break;
	}
	
	return returnVal;	// Tell the Dialog Manager whether we handled this event or not.
}


/** This is what we're really here for, though, folks! **/
void	DoTheMagic( void )
{
	DialogPtr		dialog;
	short			whichFood = popmiHotDog;
	short			standardDITL;	/* number of items in standar DITL for our window */
	Food			newFood;		/* Copy this so we can allow a cancel */
	Boolean			done = false;
	short			hit;
	short			itemType;
	Handle			foodPopHandle;
	Rect			foodPopRect;
	short			newValue;
	
	// A very useful routine.  It makes it so that all controls that are check boxes and
	// radio buttons (you can change it if you like) use the current window font.
	PatchNewControl();
	
	// Canned routine for getting a dialog box and setting the font as well
	dialog = MyGetNewDialogFont( 128, true, nil, geneva, 10 );
	
	if( !dialog )
	{
		SysBeep( 10 );
		return;
	}
	
	// Set the menus for a movable modal dialog box
	MovableModalMenus();
	
	SetDialogDefaultItem( dialog, pbOK );
	SetDialogCancelItem( dialog, pbCancel );
	
	// Copy our variables so that we are only changing temporary ones
	BlockMove( food, &newFood, sizeof( Food ) );

	// We could just use SnatchHandle here, but I decided not to for some odd reason
	GetDialogItem( dialog, popWhichFood, &itemType, &foodPopHandle, &foodPopRect );
	SetControlValue( (ControlHandle) foodPopHandle, whichFood );

	// Count the number of items we have in our dialog before we add new ones
	standardDITL = CountDITL( dialog );

	// Add the correct DITL (based on whichFood), and check the correct stuff
	SetUpFoodDitl( dialog, standardDITL, whichFood, &newFood );

	// We aren't doing drawing routines here because when a dialog is put up, it
	// automatically generates an updateEvt, so you don't have to deal with it specifically.
		
	while( !done )
	{
		MovableModalDialog( 0L, &hit, dialog );
		
		switch( hit )
		{
			case pbOK:
				BlockMove( &newFood, food, sizeof( Food ) );
				done = true;
				break;
			case pbCancel:
				done = true;
				break;
			case popWhichFood:
				GetDialogItem( dialog, popWhichFood, &itemType, &foodPopHandle, &foodPopRect );
				newValue = GetControlValue( (ControlHandle) foodPopHandle );
				if( whichFood == newValue )
					break;
				else
				{
					whichFood = newValue;
					TakeOutCurrentDITL( dialog, standardDITL );
					SetUpFoodDitl( dialog, standardDITL, whichFood, &newFood );
				}
			
				break;
			default:
				switch( whichFood )
				{
					case popmiHotDog:
						HandleHotDogItems( dialog, standardDITL, &newFood, hit );
						break;
					case popmiHamburger:
						HandleHamburgerItems( dialog, standardDITL, &newFood, hit );
						break;
				}
				break;
		}
	}

	UnPatchNewControl();
	TwiddleMenus();

	DisposeDialog( dialog );

}

/* I use this as a quick way to get a new dialog, put it behind any window I want,
	as well as set the window font all in one fell swoop. */
DialogPtr	MyGetNewDialogFont( int	rsrcId, Boolean moveToFront, WindowPtr behindWhich,
					short fontNo, short textSize )
{
	DialogPtr		window;
	
	if( moveToFront ) behindWhich = (WindowPtr) -1L;

	window = GetNewDialog( rsrcId, nil, behindWhich );
	
	if( !window )
	{
		SysBeep( 10 );
		return nil;
	}

	SetWRefCon( window, (long) rsrcId );
	SetPort( (GrafPtr) window );
	
	SetDialogFontAndSize( window, fontNo, textSize );
	
	ShowWindow( window );
	return window;
}

/* Also stolen from an unknown source */
void	SetDialogFontAndSize( DialogPtr theDialog, short fontNo, short fontSize )
{
	FontInfo	fInfo;

	TextFont( fontNo );
	TextSize( fontSize );
	GetFontInfo( &fInfo );
	
	(**(((DialogPeek)theDialog)->textH)).txFont = geneva;
	(**(((DialogPeek)theDialog)->textH)).txSize = 9;
	
	(**(((DialogPeek)theDialog)->textH)).lineHeight = fInfo.ascent + fInfo.descent + fInfo.leading;
	(**(((DialogPeek)theDialog)->textH)).fontAscent = fInfo.ascent;
}

/** Frame the options area **/
void	FrameOptions( DialogPtr dialog )
{
	int		i;
	Rect	theRect;
	
	for( i = rFirstOutline; i <= rLastOutline; i++ )
	{
		GetDialogItemRect( dialog, i, &theRect );
		FrameRect( &theRect );
	}
}

/** Add the correct DITLs, and then call the checking procedure **/
void	SetUpFoodDitl( DialogPtr window, short standardDITL, short whichFood, FoodPtr newFood )
{
	Handle		addDitl;
	
	addDitl = GetResource( 'DITL', whichFood + ditlHotDog - 1 );
	if( !addDitl )
	{
		SysBeep( 10 );
		ExitToShell();
	}
	
	HLock( addDitl );
	AppendDITL( window, addDitl, overlayDITL );
	HUnlock( addDitl );
	ReleaseResource( addDitl );
	
	SelectTheRightStuff( window, standardDITL, whichFood, newFood );
}

/** Remove the current EXTRA items in the dialog box, to set up for the next one **/
void	TakeOutCurrentDITL( DialogPtr window, short standardDITL )
{
	short		ditlCount;
	
	ditlCount = CountDITL( window );
	
	ShortenDITL( window, ditlCount - standardDITL );
}

/** Just sees which item we wish to go through and make sure everything is checked for **/
void	SelectTheRightStuff( DialogPtr window, short standardDITL, short whichFood, FoodPtr newFood )
{
	switch( whichFood )
	{
		case popmiHotDog:
			CheckHotDogItems( window, whichFood, standardDITL, newFood );
			break;
		case popmiHamburger:
			CheckHamburgerItems( window, whichFood, standardDITL, newFood );
			break;
		default:
			break;
	}
}


/** The following two routines simply select the right items in the dialog box depending
	on the values of the variables **/
void	CheckHotDogItems( DialogPtr window, short whichFood, short standardDITL, FoodPtr newFood )
{
	short		itemType;
	Handle		foodPopHandle;
	Rect		foodPopRect;
	short		i;
	
	/* Make the popup list point to the right thing */
	GetDialogItem( window, popHotType+standardDITL, &itemType, &foodPopHandle, &foodPopRect );
	SetControlValue( (ControlHandle) foodPopHandle, newFood->hotDog.hotType );
	
	// Check the radio buttons appropriately
	for( i = rbChargeThroughNose; i <= rbGiveAway; i++ )
		SelectButton( window, i+standardDITL, i == newFood->hotDog.price );

	// Do button stuff
	SelectButton( window, cbHotRelish+standardDITL, newFood->hotDog.relish );
	SelectButton( window, cbHotOnion+standardDITL, newFood->hotDog.onion );
	SelectButton( window, cbHotKetchup+standardDITL, newFood->hotDog.ketchup );
	SelectButton( window, cbHotMustard+standardDITL, newFood->hotDog.mustard );
}

void	CheckHamburgerItems( DialogPtr window, short whichFood, short standardDITL, FoodPtr newFood )
{
	short		i;
	
	for( i = rbBeef; i <= rbOther; i++ )
		SelectButton( window, i+standardDITL, i == newFood->ham.hamType );

	SelectButton( window, cbHamRelish+standardDITL, newFood->ham.relish );
	SelectButton( window, cbHamOnion+standardDITL, newFood->ham.onion );
	SelectButton( window, cbHamKetchup+standardDITL, newFood->ham.ketchup );
	SelectButton( window, cbHamMustard+standardDITL, newFood->ham.mustard );
	SelectButton( window, cbHamTomato+standardDITL, newFood->ham.tomato );
	SelectButton( window, cbHamLettuce+standardDITL, newFood->ham.lettuce );
	SelectButton( window, cbHamPickle+standardDITL, newFood->ham.pickle );
	
	SelectButton( window, cbLie+standardDITL, (newFood->ham.hamType == rbOther && newFood->ham.lie ) );
	
	// EnableButton is very interesting.  You will note (those of you with color Macs) that
	// the "Lie about contents" button greyed in TRUE grey, not in the fake grey pattern
	// that we are used to seeing.  This is because I created a custom color table for my
	// dialog box.  This made the Dialog Manger know that we had a full color dialog
	// box, and so it knew to display in real rather than fake grey.  All you have to do
	// is choose "Custom" in the "Window Colors" part of the dialog box editor in ResEdit,
	// change one item, and change it back.  Then, all of your controls will be properly greyed
	// without any extra code.
	EnableButton( window, cbLie+standardDITL, newFood->ham.hamType == rbOther );
}

/** Handle it if we clicked somewhere when the HotDog items DITL was up **/
void	HandleHotDogItems( DialogPtr window, short standardDITL, FoodPtr newFood, short hit )
{
	if ( hit == pbRevert )
	{
		DefaultHotDogSettings( newFood );
		CheckHotDogItems( window, 0, standardDITL, newFood );
		return;
	}
	
	switch( hit - standardDITL)
	{
		case rbChargeThroughNose: case rbBeReasonable: case rbArmAndLeg: case rbGiveAway:
			SelectButton( window, newFood->hotDog.price+standardDITL, false );
			newFood->hotDog.price = hit-standardDITL;
			SelectButton( window, newFood->hotDog.price+standardDITL, true );
			break;
		case cbHotRelish:
			CheckButton( window, cbHotRelish+standardDITL );
			newFood->hotDog.relish = !(newFood->hotDog.relish);
			break;
		case cbHotOnion:
			CheckButton( window, cbHotOnion+standardDITL );
			newFood->hotDog.onion = !(newFood->hotDog.onion);
			break;
		case cbHotKetchup:
			CheckButton( window, cbHotKetchup+standardDITL );
			newFood->hotDog.ketchup = !(newFood->hotDog.ketchup);
			break;
		case cbHotMustard:
			CheckButton( window, cbHotMustard+standardDITL );
			newFood->hotDog.mustard = !(newFood->hotDog.mustard);
			break;
		case popHotType:
			{short		itemType;
			Handle		foodPopHandle;
			Rect		foodPopRect;
			GetDialogItem( window, popHotType+standardDITL, &itemType, &foodPopHandle, &foodPopRect );
			newFood->hotDog.hotType = GetControlValue( (ControlHandle) foodPopHandle );
			}break;
	}
}


void	HandleHamburgerItems( DialogPtr window, short standardDITL, FoodPtr newFood, short hit )
{
	if ( hit == pbRevert )
	{
		DefaultHamburgerSettings( newFood );
		CheckHamburgerItems( window, 0, standardDITL, newFood );
		return;
	}

	switch( hit - standardDITL)
	{
		case rbBeef: case rbChicken: case rbOther:
			SelectButton( window, newFood->ham.hamType+standardDITL, false );
			SelectButton( window, hit, true );
			newFood->ham.hamType = hit-standardDITL;
			if( newFood->ham.hamType == rbOther )
				EnableButton( window, cbLie+standardDITL, true );
			else
			{
				EnableButton( window, cbLie+standardDITL, false );
				newFood->ham.lie = false;
				SelectButton( window, cbLie+standardDITL, false );
			}
			break;
		case cbLie:
			CheckButton( window, cbLie+standardDITL );
			newFood->ham.lie = !(newFood->ham.lie);
			break;
		case cbHamRelish:
			CheckButton( window, cbHamRelish+standardDITL );
			newFood->ham.relish = !(newFood->ham.relish);
			break;
		case cbHamOnion:
			CheckButton( window, cbHamOnion+standardDITL );
			newFood->ham.onion = !(newFood->ham.onion);
			break;
		case cbHamKetchup:
			CheckButton( window, cbHamKetchup+standardDITL );
			newFood->ham.ketchup = !(newFood->ham.ketchup);
			break;
		case cbHamMustard:
			CheckButton( window, cbHamMustard+standardDITL );
			newFood->ham.mustard = !(newFood->ham.mustard);
			break;
		case cbHamTomato:
			CheckButton( window, cbHamTomato+standardDITL );
			newFood->ham.tomato = !(newFood->ham.tomato);
			break;
		case cbHamLettuce:
			CheckButton( window, cbHamLettuce+standardDITL );
			newFood->ham.lettuce = !(newFood->ham.lettuce);
			break;
		case cbHamPickle:
			CheckButton( window, cbHamPickle+standardDITL );
			newFood->ham.pickle = !(newFood->ham.pickle);
			break;
	}
}

/** This function is a hack.  You should do a better job with it. =)  **/
void	FindWindowToUpdate( WindowPtr window )
{
	long		ID;
	GrafPtr		old;
	
	// I store the window number in the ref con of the window (see MyGetNewDialogFont())
	// Normally I have a function to do this, but in this case, I only call it once.
	ID = (long) GetWRefCon( window );
	
	GetPort( &old );

	SetPort( window );
	
	switch (ID)
	{
		case wPreferences:
			// This is a weird thing.  Normally you stick redrawing functions within the BeginUpdate()
			// and EndUpdate() calls, but here, if I try, they don't draw correctly.  It's possible
			// that the dialog manager draws over anything I draw, and that's why, but I haven't
			// run across that in any other program I've written.  Any ideas anyone?
			BeginUpdate( window );
			EndUpdate( window );
	
			FrameOptions( (DialogPtr) window );
			break;
		default:
			break;
	}

	SetPort( old );
}
/** THE END!!!!! **/
