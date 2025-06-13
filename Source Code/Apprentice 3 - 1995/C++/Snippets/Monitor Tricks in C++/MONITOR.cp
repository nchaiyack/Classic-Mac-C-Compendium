//
// MONITOR.cp		verson 1.0
//
// Method definitions for the MONITOR class in C++.
//
// Requires System 7 or later.
//
// Public Domain.
// Written May 1995 by Kenneth Worley.
//
// The fading code in this class was derived from the public domain work
// of N. Jonas Englund, Mark Womack, and Macneil Shonle in the clut fade
// project. You can contact them at: AOL: MarkWomack,
// Internet: markwomack@aol.com and Macneil Shonle - AOL: MacneilS,
// Internet: macneils@aol.com
//
// You can contact me at AOL: KNEworley, Internet: KNEworley@aol.com
// I'm a freelance programmer. Send me work!
//

#include "MONITOR.h"
#include <Palettes.h>


#define MyDisposeHandle(h)		if ( h )										\
								{												\
									DisposeHandle( (Handle)h );					\
									h = NULL;									\
								}


// MONITOR Constructor.
//
MONITOR::MONITOR( GDHandle aDevice )
{
	IMonitor( aDevice );
}

// MONITOR Constructor.
//
MONITOR::MONITOR( void )
{
}

// MONITOR Destructor doesn't do anything.
//
MONITOR::~MONITOR( void )
{

}

// IMonitor just sets some defaults. It is called either by the
// MONITOR constructor if a device handle is specified then, or
// it must be called explicitly if the constructor with no
// arguments is used.
//
void MONITOR::IMonitor( GDHandle aDevice )
{
	steps = 0;
	fading = false;		// not fading right now
	myDevice = aDevice;		// save a reference to the device
	myClut = (**(**myDevice).gdPMap).pmTable; // get the monitor’s current clut
	mySavedClut = NULL;
	destClut = NULL;
}

// FadeToClut sets up a fade from the current color table to the one whose
// handle is sent in the dest parameter. The fade will be done in the
// number of steps specified in the stepsToTake parameter. The fade is not
// done in FadeToClut. Rather, FadeToClut sets a flag (fading) to true.
// Then, each time FadeStep is called, a step of the fade is done. Once
// FadeStep has been called enough times for the fade to complete, it sets
// fading back to false.
//
void MONITOR::FadeToClut( CTabHandle dest, short stepsToTake )
{
	long		difference;
	GDHandle	savedDevice;
	short		i;
	Boolean		okay = false;
	
	// If a fade is already in progress, finish it quickly then
	// proceed with this fade.
	
	while ( fading )
		FadeStep();
	
	// Save the current graphics device.
	
	savedDevice = GetGDevice();
	
	// Make this device the current device.
	
	SetGDevice( myDevice );
	
	do
	{
	
	// Save a copy of the destination color table in an instance
	// variable.
	
	destClut = dest;
	if ( HandToHand( (Handle*)&destClut ) != noErr )
	{
		destClut = NULL;
		break;
	}
	else
		HLock( (Handle)destClut );
	
	// Also lock this monitor's clut while fading.
	
	HLock( (Handle)myClut );
	
	// Make sure stepsToTake is a reasonable number, then put it in steps.
	
	if ( stepsToTake < 1 )
		steps = 1;
	else if ( stepsToTake > 300 )
		steps = 300;
	else
		steps = stepsToTake;
		
	// Set up the delta values for each color table element.
	
	for( i=0; i <= (**destClut).ctSize; i++ )
	{
		// Take the difference between the color values for each color and each
		// table element in the source and destination color tables and divide
		// that value by the number of steps we'll be taking. For each step,
		// this is the value we'll add to the clut elements.
  
		difference = ( (long)(**myClut).ctTable[i].rgb.red -
			(long)(**destClut).ctTable[i].rgb.red );
		redDeltas[i] = difference / (long)steps;
  
  		difference = ( (long)(**myClut).ctTable[i].rgb.green -
  			(long)(**destClut).ctTable[i].rgb.green );
  		grnDeltas[i] = difference / (long)steps;
  
		difference = ( (long)(**myClut).ctTable[i].rgb.blue -
			(long)(**destClut).ctTable[i].rgb.blue );
		blueDeltas[i] = difference / (long)steps;
	}

	// Set the fading flag to indicate a fade is in progress.
	
	fading = true;
	
	// Looks like things went okay.
	
	okay = true;
	
	}
	while ( false );	// This "loop" just allows me to jump
						// down to here with a break statement.
	
	// If things didn't go okay, get rid of allocated memory.
	
	if ( !okay )
	{
		MyDisposeHandle( destClut );
		fading = false;
	}
	
	// Restore the previously saved device.
	
	SetGDevice( savedDevice );
}

// FadeStep is the method that takes care of the actual fading. It does one
// step of the fade each time it's called. When the fade is complete,
// it cleans up by calling FinishFade. That causes the flag 'fading' to be
// set to false.
//
void MONITOR::FadeStep( void )
{
	GDHandle		savedDevice;
	short			colorIndex;
	
	if ( !steps )
		return;
		
	// Save the current device.
	
	savedDevice = GetGDevice();
 
	// Set it to this device.
	
	SetGDevice( myDevice );

	// Do a step in the fade. Each element in the color table gets one
	// step closer to the color in the destination color table.
	
		for( colorIndex = 0; colorIndex <= (**destClut).ctSize; colorIndex++ )
		{ // make the source more and more like the dest
			(**myClut).ctTable[colorIndex].rgb.red -= redDeltas[colorIndex];
			(**myClut).ctTable[colorIndex].rgb.green -= grnDeltas[colorIndex];
			(**myClut).ctTable[colorIndex].rgb.blue -= blueDeltas[colorIndex];
		}
  
		SetEntries( 0, (**myClut).ctSize, (ColorSpec *)&(**myClut).ctTable );
	
	// Decrement step.
	
	steps--;
	
	// See if we're done fading. If so, call FinishFade.
	if ( steps == 0 )
		FinishFade();
	
	// Restore the previously saved device.
	
	SetGDevice( savedDevice );
}

// FinishFade cleans up after a fade and makes sure the monitor's
// clut is exactly the same as the destination clut. It disposes
// of the destination clut and builds the inverse table for the
// current clut. It also sets the fading flag to false so everyone
// can tell we're finished fading. If this method were called in
// the middle of a fade, the fade would immediately finish in one
// step.
//
void MONITOR::FinishFade( void )
{
	// Make sure steps is zero now.
	
	steps = 0;
	
	// Make sure the clut exactly matches the destination clut and create
	// the inverse table.
	
	SetEntries( 0, (**destClut).ctSize, (ColorSpec *)&(**destClut).ctTable );
	MakeITable( nil, nil, 0 );

	// Get rid of the destination clut.
	
	MyDisposeHandle( destClut );
	
	// Unlock this monitor's clut again.
	
	HUnlock( (Handle)myClut );
	
	// Reset the fading flag to false.
	
	fading = false;
}

// AbortFade stops fading the screen where it's at. It cleans up
// just like FinishFade, but doesn't make sure the color table
// exactly matches the destination clut.
//
void MONITOR::AbortFade( void )
{
	// Make sure steps is zero now.
	
	steps = 0;
	
	// Create the inverse table.
	
	MakeITable( nil, nil, 0 );

	// Get rid of the destination clut.
	
	MyDisposeHandle( destClut );
	
	// Unlock this monitor's clut again.
	
	HUnlock( (Handle)myClut );
	
	// Reset the fading flag to false.
	
	fading = false;
}

// SaveCurrentClut makes a copy of the current color table and
// puts the copy's handle in the instance variable mySavedClut.
//
Boolean MONITOR::SaveCurrentClut( void )
{
	MyDisposeHandle( mySavedClut );
	
	mySavedClut = myClut;
	if ( HandToHand( (Handle*)&mySavedClut ) != noErr )
	{
		mySavedClut = NULL;
		return false;
	}
	else
	{
		HNoPurge( (Handle)mySavedClut );
		return true;
	}
}

// FadeToBlack makes a black RGB record and calls FadeToColor
// with it.
//
void MONITOR::FadeToBlack( short stepsToTake )
{
	RGBColor	blackColor;
	
	// Make the black color.
	blackColor.blue = blackColor.red = blackColor.green = 0;

	// Fade to that color.
	
	FadeToColor( &blackColor, stepsToTake );
}

// FadeToColor creates a new color table where every element
// is the color specified in the aColor parameter. Then it
// calls FadeToClut to fade to the new clut. If you want to
// fade back to an image after calling this method, you should
// call SaveCurrentClut before calling this method. Then you
// can call FadeToSaved afterward to restore the screen.
//
void MONITOR::FadeToColor( RGBColor *aColor, short stepsToTake )
{
	CTabHandle	dest;
	short		x;
	
	// Get copy of the current color table.
	dest = myClut;
	if ( HandToHand( (Handle*)&dest ) != noErr )
		return;
	else
		HNoPurge( (Handle)dest );

	// Make the color table all one color.
	for( x = 0; x <= (**dest).ctSize; x++ )
		(**dest).ctTable[x].rgb = (*aColor);

	// Fade to the custom black color table.
	FadeToClut( dest, stepsToTake );

	// Get rid of the destination color table.
	MyDisposeHandle( dest );
}

// FadeToSaved fades the screen to the color table saved in
// the instance variable mySavedClut. That clut is usually
// put there by the SaveCurrentClut method.
//
void MONITOR::FadeToSaved( short stepsToTake )
{
	FadeToClut( mySavedClut, stepsToTake );
}

// FadeToClutSync does the same thing as FadeToClut but it
// takes care of making sure the fade is complete before
// returning.
//
void MONITOR::FadeToClutSync( CTabHandle dest, short stepsToTake )
{
	FadeToClut( dest, stepsToTake );
	while ( fading )
		FadeStep();
}

// FadeToBlackSync does the same thing as FadeToBlack but it
// takes care of making sure the fade is complete before
// returning.
//
void MONITOR::FadeToBlackSync( short stepsToTake )
{
	FadeToBlack( stepsToTake );
	while ( fading )
		FadeStep();
}

// FadeToColorSync does the same thing as FadeToColor, but it
// takes care of making sure the fade is complete before
// returning.
//
void MONITOR::FadeToColorSync( RGBColor *aColor, short stepsToTake )
{
	FadeToColor( aColor, stepsToTake );
	while ( fading )
		FadeStep();
}

// FadeToSavedSync does the same thing as FadeToSaved but it
// takes care of making sure the fade is complete before
// returning.
//
void MONITOR::FadeToSavedSync( short stepsToTake )
{
	FadeToSaved( stepsToTake );
	while ( fading )
		FadeStep();
}

// MaxScreenDepth returns the maximum bit depth that the monitor
// is capable of (i.e. 8 for 256 colors).
//
short MONITOR::MaxScreenDepth( void )
{
	short		x;
	short		hasThisDepth=1;
	
	for ( x=2; x<=32; x*=2 )
		if ( HasDepth( myDevice, x, gdDevType, 1 ) )
			hasThisDepth = x;
		else
			break;
	
	return hasThisDepth;
}

// GetScreenDepth returns the current bit depth of this monitor.
//
short	MONITOR::GetScreenDepth( void )
{
	return (**(**myDevice).gdPMap).pixelSize;
}

// GetScreenMode returns one if the monitor is displaying colors, zero
// if it's displaying in grays.
//
short	MONITOR::GetScreenMode( void )
{
	if ( TestDeviceAttribute( myDevice, gdDevType ) )
		return 1;	//colors
	else
		return 0;	//grays
}

// SetScreenDepth changes the bit depth of the screen (the number of colors
// that it can display at once).
//
void	MONITOR::SetScreenDepth( short newDepth )
{
	GDHandle		savedDevice;
	
	savedDevice = GetGDevice();
	SetGDevice( myDevice );
	
	if ( newDepth != GetScreenDepth() )
		if ( HasDepth( myDevice, newDepth, 1 << gdDevType, GetScreenMode() ) )
			SetDepth( myDevice, newDepth, 1 << gdDevType, GetScreenMode() );
	
	SetGDevice( savedDevice );
}

// SetScreenMode sets this monitor to color if 1 is sent, or grays
// if zero is sent.
//
void	MONITOR::SetScreenMode( short newMode )
{
	GDHandle		savedDevice;
	
	savedDevice = GetGDevice();
	SetGDevice( myDevice );
	
	if ( newMode != GetScreenMode() )
		if ( HasDepth( myDevice, GetScreenDepth(), 1 << gdDevType, newMode ) )
			SetDepth( myDevice, GetScreenDepth(), 1 << gdDevType, newMode );
	
	SetGDevice( savedDevice );
}

// SetScreenModeDepth sets the monitor to the color mode and depth
// specified.
//
void	MONITOR::SetScreenModeDepth( short newDepth, short newMode )
{
	GDHandle		savedDevice;
	
	savedDevice = GetGDevice();
	SetGDevice( myDevice );
	
	if ( (newDepth != GetScreenDepth()) || (newMode != GetScreenMode()) )
		if ( HasDepth( myDevice, newDepth, 1 << gdDevType, newMode ) )
			SetDepth( myDevice, newDepth, 1 << gdDevType, newMode );
	
	SetGDevice( savedDevice );
}

