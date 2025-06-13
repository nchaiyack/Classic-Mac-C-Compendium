// main.c
//
// part of Monitor Tricks in C++ v1.0
//
// by Kenneth Worley  5/28/95
// Public Domain.
//
// You may use any of this code in your own projects without
// restriction.
//
// This project uses a simple "dashboard" application to demonstrate
// a MONITOR class that can change the depth and mode of a monitor
// and also execute a clut fade on the monitor.
//
// In the dialog, the bit depth and mode settings affect only the
// main monitor, but the fading buttons affect all monitors that
// have their boxes checked.

// ===================== Included Files =====================

#include "Alert Utilities.h"
#include "System Utilities.h"
#include "Dialog Utilities.h"
#include "Useful Macros.c"
#include "MONITOR.h"

// ===================== Dialog Item Numbers =====================

#define kFirstMonitor	10

#define kColors			8
#define kGrays			9

#define kMillions		2
#define kThousands		3
#define k256			4
#define k16				5
#define k4				6
#define kBW				7

#define kQuit			1

#define kFadeAllBlack	20
#define kFadeInSuccession	21
#define kFadeRGB		22

// ===================== Other Defines =====================

#define kMaxMonitors	10	// like anyone's got this many!

// ===================== Prototypes =====================

void main( void );
void Run( void );
void HandleDisk( EventRecord *event );
void FadeAllBlack( MONITOR *monitors[], Boolean monitorsChecked[] );
void FadeInSuccession( MONITOR *monitors[], Boolean monitorsChecked[] );
void DoFadeReps( MONITOR *monitors[], Boolean monitorsChecked[], short reps );
void FadeThroughRGB( MONITOR *monitors[], Boolean monitorsChecked[] );

void main( void )
{
	// Initialize the Mac Toolbox.
	
	InitToolbox();
	
	// Check for System 7 and Color QuickDraw.
	
		if ( (!HasColorQuickDraw()) || (GetSysVersion() < 7) )
		{
			DoAlertStr( 129, true,
				"\pSorry! System 7 and Color QuickDraw are required!" );
			ExitToShell();
		}
	
	Run();
}

void Run( void )
{
	EventRecord		event;
	Boolean			done = false;
	DialogPtr		aDlg;
	DialogPtr		myDialog;
	short			itemHit;
	short			x;

	MONITOR			*monitors[kMaxMonitors];
	GDHandle		aDevice;
	short			noOfMonitors;
	Boolean			monitorsChecked[10];
	MONITOR			*mainMonitor;
	
	// Load the main dialog.
	
		myDialog = GetNewDialog( 128, NULL, (WindowRef)(-1L) );
	
	// Find all the monitors attached and create a MONITOR object
	// for each one.
	
	aDevice = GetDeviceList();
	x=1;
	
	while ( aDevice )
	{
		if ( aDevice == GetMainDevice() )
		{
			monitors[0] = new MONITOR( aDevice );
			mainMonitor = monitors[0];
		}
		else
		{
			monitors[x] = new MONITOR( aDevice );
			x++;
		}
		
		aDevice = GetNextDevice( aDevice );
	}
	noOfMonitors = x;	// This is how many monitors are attached.
	
	// Disable the checkboxes for extra monitors that we don't have.
	
	for ( x=noOfMonitors+1; x<=10; x++ )
		ActivateControl( myDialog, x + kFirstMonitor - 1, false );
	
	// Check the box for the main monitor by default and make sure
	// that's the only true value in the monitorsChecked array.
	
	SetOnOff( myDialog, kFirstMonitor, true );
	
	monitorsChecked[0] = true;
	for ( x=1; x<10; x++ )
		monitorsChecked[x] = false;
	
	// Turn off all the radio buttons for bit depth except the depth
	// that the main monitor is at.
	
	switch ( mainMonitor->GetScreenDepth() )
	{
		case 1:
			SetRadioButton( myDialog, kMillions, kBW, kBW );
			break;
		case 2:
			SetRadioButton( myDialog, kMillions, kBW, k4 );
			break;
		case 4:
			SetRadioButton( myDialog, kMillions, kBW, k16 );
			break;
		case 8:
			SetRadioButton( myDialog, kMillions, kBW, k256 );
			break;
		case 16:
			SetRadioButton( myDialog, kMillions, kBW, kThousands );
			break;
		case 32:
			SetRadioButton( myDialog, kMillions, kBW, kMillions );
			break;
	}
	
	// Disable the depth buttons that don't apply to the main monitor.
	// (i.e. the ones you don't have enough VRAM for).
	
	switch( mainMonitor->MaxScreenDepth() )
	{
		case 1:
			ActivateControl( myDialog, k4, false );
		case 2:
			ActivateControl( myDialog, k16, false );
		case 4:
			ActivateControl( myDialog, k256, false );
		case 8:
			ActivateControl( myDialog, kThousands, false );
		case 16:
			ActivateControl( myDialog, kMillions, false );
	}
	
	// Turn on the appropriate radio button for color/gray status and
	// turn off the other one.
	
	if ( mainMonitor->GetScreenMode() )	// colors
		SetRadioButton( myDialog, kColors, kGrays, kColors );
	else
		SetRadioButton( myDialog, kColors, kGrays, kGrays );

	// Show the dialog.
	
	ShowWindow( myDialog );
	
	// Event loop.
	
	while ( !done )
	{
		WaitNextEvent( everyEvent, &event, GetCaretTime(), NULL );

		switch( event.what )
		{
			case diskEvt:
				
				HandleDisk( &event );
				break;
			
			default:
	
			// Pass the event to the Dialog Manager.
			
			if ( DialogSelect( &event, &aDlg, &itemHit ) )
			
				// See what control was manipulated.
				
				switch ( itemHit )
				{
					// Quit Button
					
					case kQuit:
					
						done = true;
						break;
					
					// Colors/Grays radio buttons
					
					case kColors:
					
						if ( mainMonitor->GetScreenMode() == 0 )
						{
							SetRadioButton( myDialog, kColors, kGrays, kColors );
							mainMonitor->SetScreenMode( 1 );
						}
						break;
					
					case kGrays:
	
						if ( mainMonitor->GetScreenMode() == 1 )
						{
							SetRadioButton( myDialog, kColors, kGrays, kGrays );
							mainMonitor->SetScreenMode( 0 );
						}
						break;
	
					// Screen Depth radio buttons
					
					case kMillions:
					
						SetRadioButton( myDialog, kMillions, kBW, kMillions );
						mainMonitor->SetScreenDepth( 32 );
						break;
						
					case kThousands:
					
						SetRadioButton( myDialog, kMillions, kBW, kThousands );
						mainMonitor->SetScreenDepth( 16 );
						break;
						
					case k256:
					
						SetRadioButton( myDialog, kMillions, kBW, k256 );
						mainMonitor->SetScreenDepth( 8 );
						break;
						
					case k16:
					
						SetRadioButton( myDialog, kMillions, kBW, k16 );
						mainMonitor->SetScreenDepth( 4 );
						break;
						
					case k4:
					
						SetRadioButton( myDialog, kMillions, kBW, k4 );
						mainMonitor->SetScreenDepth( 2 );
						break;
						
					case kBW:
					
						SetRadioButton( myDialog, kMillions, kBW, kBW );
						mainMonitor->SetScreenDepth( 1 );
						break;
					
					// Fade button to fade all to black
					
					case kFadeAllBlack:
					
						FadeAllBlack( monitors, monitorsChecked );
						break;
					
					// Fade button to fade monitors in succession
					
					case kFadeInSuccession:
					
						FadeInSuccession( monitors, monitorsChecked );
						break;
					
					// Fade button to fade through red, green & blue
					
					case kFadeRGB:
					
						FadeThroughRGB( monitors, monitorsChecked );
						break;
					
					// If it wasn't any of those, it must be one of the
					// monitor checkboxes.
					
					default:
					
						Toggle( myDialog, itemHit );
						monitorsChecked[itemHit - kFirstMonitor] =
							GetOnOff( myDialog, itemHit );
						break;
				}
		}
	}
}

// HandleDisk checks the event message when we get a disk event
// to see if the disk could not be mounted. If so, it calls
// DIBadMount to allow the user to eject or initialize it.
// The message field contains the drive number in the low
// word and the result from the PBMountVol call in the high
// word.
//
void HandleDisk( EventRecord *event )
{
	if ( HiWord( event->message ) != noErr )
	{
		// A bad disk was inserted.
		
		Point		ignoredPt = {100,80};	//ignored in Sys7
		
		// Allow user to initialize or eject bad disk.
		DIBadMount( ignoredPt, event->message );
	}
}

// Fade all selected monitors to black and then back again.
//
void FadeAllBlack( MONITOR *monitors[], Boolean monitorsChecked[] )
{
	short		x;
	
	// Do the fade to black (making sure to save the current clut first).
	// This gets them started.
	
	for ( x=0; x<kMaxMonitors; x++ )
		if ( monitorsChecked[x] )
		{
			monitors[x]->SaveCurrentClut();
			monitors[x]->FadeToBlack( 100 );	// in 100 steps
		}
	
	// Keep calling the monitors to keep fading until they're done.
	// Use a ridiculously large number so we know the fades will
	// complete. It returns when the fades are through anyway.
	
	DoFadeReps( monitors, monitorsChecked, 10000 );
	
	// Now that they're faded to black, we have to restore them
	// (after a short delay). We tell each monitor to fade back
	// to the saved clut.
	
	MDelay ( 60 );
	
	for ( x=0; x<kMaxMonitors; x++ )
		if ( monitorsChecked[x] )
			monitors[x]->FadeToSaved( 100 );	// in 100 steps
	
	// Keep calling the monitors to keep fading until they're done.
	
	DoFadeReps( monitors, monitorsChecked, 10000 );
}

// Fade each of the selected monitors to black, but start each when
// the one before it is halfway done fading. Fade back up in the same
// manner.
//
void FadeInSuccession( MONITOR *monitors[], Boolean monitorsChecked[] )
{
	short		x;
	
	// Start each monitor fading. After each one we start, we let
	// fading on any monitors go for 50 steps.
	
	for ( x=0; x<kMaxMonitors; x++ )
		if ( monitorsChecked[x] )
		{
			monitors[x]->SaveCurrentClut();
			monitors[x]->FadeToBlack( 100 );	// in 100 steps
			DoFadeReps( monitors, monitorsChecked, 50 );
		}
	
	// Keep calling the monitors to keep fading until they're done.
	
	DoFadeReps( monitors, monitorsChecked, 10000 );
	
	// Now fade the monitors back to their saved cluts.

	for ( x=0; x<kMaxMonitors; x++ )
		if ( monitorsChecked[x] )
		{
			monitors[x]->FadeToSaved( 100 );	// in 100 steps
			DoFadeReps( monitors, monitorsChecked, 50 );
		}

	// Keep calling the monitors to keep fading until they're done.
	
	DoFadeReps( monitors, monitorsChecked, 10000 );
}

// DoFadeReps calls the FadeStep method of all monitor objects whose elements
// are checked in the monitorsChecked array. It does this either the number
// of times sent in the reps parameter, or however many times it takes to
// finish fading all the monitors, whichever is LEAST. In other words, this
// function will always return when all monitors are finished fading
// regardless of the number sent in reps. If they aren't done fading after
// reps repetitions, though, the function returns with all monitors NOT
// fully faded.
//
void DoFadeReps( MONITOR *monitors[], Boolean monitorsChecked[], short reps )
{
	short		x,y;
	Boolean		stillFading;
	
	for ( x=1; x<=reps; x++ )
	{
		stillFading = false;
		
		for ( y=0; y<kMaxMonitors; y++ )
			if ( monitorsChecked[y] )
				if ( monitors[y]->fading )
				{
					stillFading = true;
					monitors[y]->FadeStep();
				}
				
		if ( !stillFading )	// if no monitors are fading anymore
			break;
	}
}

// Fade all the selected monitors through the colors red, green,
// and blue, then back to the original clut.
//
void FadeThroughRGB( MONITOR *monitors[], Boolean monitorsChecked[] )
{
	short			x;
	short			whichColor;
	RGBColor		aColor;
	
	// Save the current color table of each monitor affected.
	
	for ( x=0; x<kMaxMonitors; x++ )
		if ( monitorsChecked[x] )
			monitors[x]->SaveCurrentClut();

	// Loop through the three colors.
	
	for ( whichColor = 1; whichColor <= 3; whichColor++ )
	{
		// Make a color.
		
		switch( whichColor )
		{
			case 1:
				aColor.red = 64000;
				aColor.green = aColor.blue = 0;
				break;
			
			case 2:
				aColor.green = 64000;
				aColor.red = aColor.blue = 0;
				break;
			
			case 3:
				aColor.blue = 64000;
				aColor.red = aColor.green = 0;
				break;
		}
		
		// Start each affected monitor fading toward the color.
		
		for ( x=0; x<kMaxMonitors; x++ )
			if ( monitorsChecked[x] )
				monitors[x]->FadeToColor( &aColor, 75 ); // in 75 steps
		
		// Finish the fade.
		
		DoFadeReps( monitors, monitorsChecked, 10000 );
	}
	
	// Now fade all affected monitors back to the clut we
	// originally saved.
	
	for ( x=0; x<kMaxMonitors; x++ )
		if ( monitorsChecked[x] )
			monitors[x]->FadeToSaved( 75 );
	
	// And finish the fades.
	
	DoFadeReps( monitors, monitorsChecked, 10000 );
}
