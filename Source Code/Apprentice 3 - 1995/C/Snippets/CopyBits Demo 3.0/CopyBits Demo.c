//
// CopyBits Demo.c
//
// Written in May 1995 using Metrowerks CodeWarrior v5.
//
// The CopyBits Demo project v2.0 by Kenneth Worley.
// This code is Copyright 1995. All Rights Reserved.
// You may use this code in any project of your own. You may also
// redistribute this project to anyone else as long as 1) all the
// project files (including documentation files) are kept together,
// and 2) nothing is charged for the project.
//
// Please ask my permission before including this code on any
// disk or CD for sale.
//
// This is a tutorial project that demonstrates the use of
// CopyBits and offscreen Graphics Worlds (GWorlds). It shows
// several instances of copying images to and from offscreen
// graphics worlds using CopyBits, provides several macros that
// make the process a little easier, shows an example of
// drawing into an offscreen graphics world and how that can
// improve onscreen animation, and it demonstrates a "fade"
// using CopyBits to fade a portion of the screen to black.
//
// Version 1.5 adds a fade from image to image using CopyBits.
// Cool!
//
// In a nutshell, the app puts up a dialog that shows two
// "source" pictures (1 & 2) and a destination area. Clicking
// on the "Copy" buttons above each of the source pictures
// causes the picture to be copied to the destination area.
// Picture 1 is labelled as "flickery animation" and picture 2
// is labelled as "smooth animation." When you put the cursor
// over each picture, you should see a difference in how a
// colored circle is animated over the picture. You should
// occasionally see the background "flicker" through on
// picture 1, but not on picture 2 because it uses an
// intermediary GWorld to draw in.
//
// The Fade to black button over the destination area will
// cause that area to fade to black from whatever happens to
// be in there. It does this by repeatedly copying a gray
// rectangle from a GWorld into the destination area using
// the subPin transfer mode of CopyBits. The erase button
// erases the destination area.
//
// The Fade picture buttons cause the picture under the
// button to be faded into whatever is currently in the
// destination area. This is accomplished using CopyBits'
// blend transfer mode.
//
// Questions? Comments? Praise? Criticism? Jobs? Send them
// all to me at KNEworley@aol.com. I'm a 'freelance'
// programmer. I work on applications as well as low-level
// software (control panels, INITs, etc.)
//

#include <QDOffscreen.h>
#include <PictUtil.h>
#include "System Utilities.h"
#include "Dialog Utilities.h"
#include "Alert Utilities.h"

// ===================== DEFINES =====================

// The main dialog's resource ID.

#define kMainDlgID				128

// The two pictures in the main dialog.

#define kPict1ID				128
#define kPict2ID				129

// Item numbers in the main dialog.

#define kMainDlgQuit			1
#define kMainDlgCopyOne			2
#define kMainDlgFadeOne			3
#define kMainDlgCopyTwo			4
#define kMainDlgFadeTwo			5
#define kMainDlgErase			6
#define kMainDlgPICTOne			8
#define kMainDlgPICTTwo			9
#define kMainDlgDestPICT		10
#define kMainDlgFadeSpeed		15
#define kMainDlgFullScreen		20
#define kMainDlgPixelize		21
#define kMainDlgDepixelizeOne	22
#define kMainDlgDepixelizeTwo	23
#define kMainDlgFadeToBlack1	7
#define kMainDlgFadeToBlack2	24
#define kMainDlgFadeToBlack3	25
#define kMainDlgBlur			27
#define kMainDlgFlipHoriz		29
#define kMainDlgFlipVert		30
#define kMainDlgSlideL			34
#define kMainDlgSlideR			35
#define kMainDlgSlideT			36
#define kMainDlgSlideB			37
#define kMainDlgApertureIn		32
#define kMainDlgApertureOut		38
#define kMainDlgTechDemo		39

// The CopyBits Technical Demo dialog ID.

#define kTechDemoDlgID			130

// Item numbers in the Technical Demo dialog.

#define kTechDemoSourceFirst	10
#define kTechDemoSourceLast		14
#define kTechDemoMaskFirst		15
#define kTechDemoMaskLast		18
#define kTechDemoModeFirst		21
#define kTechDemoModeLast		36
#define kTechDemoDither			37
#define kTechDemoHilite			38
#define kTechDemoOpColorFirst	42
#define kTechDemoOpColorLast	46
#define kTechDemoDestPict		9
#define kTechDemoCopyBits		49
#define kTechDemoCopyMask		50
#define kTechDemoDone			51
#define kTechDemoSourcePict1	1
#define kTechDemoSourcePict2	2
#define kTechDemoSourcePict3	3
#define kTechDemoSourcePict4	4
#define kTechDemoSourcePict5	5
#define kTechDemoMaskPict1		6
#define kTechDemoMaskPict2		7
#define kTechDemoMaskPict3		8
#define kTechDemoMaskNone		53
#define kTechDemoOpColorPict	40
#define kTechDemoErase			54
#define kTechDemoCopyDeepMask	55
#define kTechDemoMaskRPict1		56
#define kTechDemoMaskRPict2		57
#define kTechDemoMaskRPict3		58
#define kTechDemoMaskRNone		60
#define kTechDemoMaskRFirst		61
#define kTechDemoMaskRLast		64

// Error messages. ('STR ' resource IDs).

#define kSys7Required			128
#define kMemoryOut				129
#define kGWorldErr				130
#define kNotYetImplemented		131
#define kMissingResource		132

// The generic error alert ID.

#define kAlertDialogID			129

// Item numbers in the generic alert dialog.

#define kAlertOKButton			1

// Codes sent to the Slide routine to indicate which
// way to slide the new picture in (from).
enum
{
	kLeft =		1,
	kRight =	2,
	kTop =		3,
	kBottom =	4
};

// Codes sent to the Aperture routine to indicate which
// way to do the effect.
enum
{
	kIn =		1,
	kOut =		2
};

// Easy way to access colors in the global colors array.
enum
{
	kWhite =	0,
	kLtGray =	1,
	kGray =		2,
	kDkGray =	3,
	kBlack =	4
};

// ===================== GLOBALS =====================

RGBColor	colors[5];	// white, black, and 3 grays

// ===================== MACROS =====================

// w can be a WindowPtr, a DialogPtr, or a GWorldPtr. In any of these
// cases, it returns the correct "bitmap" to send to CopyBits in the
// source or destination bitmap parameter.
//#define WINBITMAP(w)		((((WindowPeek)(w))->port).portBits)
#define WINBITMAP(w)		(((GrafPtr)(w))->portBits)

// w can also be a WindowPtr, DialogPtr, or GWorldPtr for any of these.
#define WINPORTRECT(w)		((((WindowPeek)(w))->port).portRect)
#define WINDOWWIDTH(w)		(WINPORTRECT(w).right - WINPORTRECT(w).left)
#define WINDOWHEIGHT(w)		(WINPORTRECT(w).bottom - WINPORTRECT(w).top)
#define WINCONTENTRECT(w)	((**((WindowPeek)(w))->contRgn).rgnBBox)
#define WINCONTENTRGN(w)	(((WindowPeek)(w))->contRgn)
#define WINVISIBLERGN(w)	(((WindowPeek)(w))->port.visRgn)
#define WINSTRUCTRECT(w)	((**((WindowPeek)(w))->strucRgn).rgnBBox)
#define WINSTRUCTRGN(w)		(((WindowPeek)(w))->strucRgn)
#define WINUPDATERECT(w)	((**((WindowPeek)(w))->updateRgn).rgnBBox)
#define WINUPDATERGN(w)		(((WindowPeek)(w))->updateRgn)

// This takes a GDHandle (device handle)
#define PIXELDEPTH(x)		((**((**(x)).gdPMap)).pixelSize)

// r is a Rect
#define TOPLEFT(r)			(* (Point*) &(r.top) )
#define BOTRIGHT(r)			(* (Point*) &(r.bottom) )

// ===================== PROTOTYPES =====================

void main( void );

void FadeToBlack1( WindowPtr destWin, Rect *destRect, short fadeSpeed );
void FadeToBlack2( WindowPtr destWin, Rect *destRect, short fadeSpeed );
void FadeToBlack3( WindowPtr destWin, Rect *destRect, short fadeSpeed );

void FadeToImage( GWorldPtr srcImage, Rect *srcRect,
	WindowPtr destWin, Rect *destRect, short fadeSpeed );

void Pixelize( WindowPtr srcWin, Rect *srcRect,
	WindowPtr destWin, Rect *destRect, short speed, Boolean pixelizing );

void FlickerAnimate( GWorldPtr srcPict, Rect *destRect );
void SmoothAnimate( GWorldPtr srcPict, Rect *destRect );

void FullScreenDemo( GWorldPtr pict1, GWorldPtr pict2, short fadeSpeed );

void Blur( WindowPtr destWin, Rect *destRect );

void FlipHorizontal( WindowPtr destWin, Rect *destRect );
void FlipVertical( WindowPtr destWin, Rect *destRect );

void Slide( WindowPtr srcWin, Rect *srcRect,
	WindowPtr destWin, Rect *destRect, short speed, short whichWay );
void Aperture( WindowPtr srcWin, Rect *srcRect,
	WindowPtr destWin, Rect *destRect, short speed, short inOrOut );

void DoTechDemo( void );

void DrawMaskRegion( WindowPtr win, short drawWhere, short whichMask );

// ===================== FUNCTIONS =====================

void main( void )
{
	DialogPtr		mainDlg;
	DialogPtr		aDlg;
	Rect			pict1Rect;
	Rect			pict2Rect;
	Rect			destRect;
	PenState		savedPen;
	short			itemHit;
	RGBColor		savedFore;
	RGBColor		savedBack;
	short			fadeSpeed;
	OSErr			myErr;
	Rect			pict1GRect;
	Rect			pict2GRect;
	GWorldPtr		pict1GWorld;
	GWorldPtr		pict2GWorld;
	EventRecord		myEvent;
	PicHandle		aPict;
	PictInfo		pictInfo;
	
	// Create some colors (white, 3 grays, and black).
	
	colors[0].red = colors[0].green = colors[0].blue = 0xFFFF;			// white
	colors[1].red = colors[1].green = colors[1].blue = 0xFFFF * 0.75;	// lt gray
	colors[2].red = colors[2].green = colors[2].blue = 0xFFFF * 0.5;	// gray
	colors[3].red = colors[3].green = colors[3].blue = 0xFFFF * 0.25;	// dk gray
	colors[4].red = colors[4].green = colors[4].blue = 0x0000;			// black
	
	// Make sure the main monitor is using a 5 bits per color inverse
	// table rather than the default 4. This will make CopyBits
	// operations using arithmetic transfer modes a bit more smooth
	// (but uses a little more memory).
	
	{
		GDHandle		aDevice;
		
		aDevice = GetGDevice();
		SetGDevice( GetMainDevice() );
		MakeITable( NULL, NULL, 5 );
		SetGDevice( aDevice );
	}
	
	// Initialize the Mac Toolbox
	
	InitToolbox();
	
	// Check the system version. This project requires System 7
	// or later for the GWorlds.
	
	if ( GetSysVersion() < 7 )
	{
		DoAlertStrID( kAlertDialogID, true, kSys7Required );
		ExitToShell();
	}
	
	// Load the main dialog and display it on screen. This
	// automatically displays our two source PICTS. Since there is
	// a 'dctb' (Dialog Color Table) resource with the same ID
	// as the dialog, the Dialog Manager uses NewColorDialog to
	// make the dialog, thus giving us a color drawing port.
	// In ResEdit, creating a 'dctb' resource for a dialog is as
	// easy as clicking Custom for the color scheme rather than
	// default when you have the 'DLOG' resource open.
	
	mainDlg = GetNewDialog( kMainDlgID, NULL, (WindowPtr)(-1L) );
	if ( !mainDlg )
	{
		DoAlertStrID( kAlertDialogID, true, kMemoryOut );
		ExitToShell();
	}
	
	// Select the text in the fade speed text edit box.
	
	SelectTextField( mainDlg, kMainDlgFadeSpeed );
	
	// Make sure the dialog is visible and that it's the current port.
	
	ShowWindow( mainDlg );
	SetGWorld( (CGrafPtr)mainDlg, NULL );
	
	// Get the picture rectangles and the destination picture's
	// rectangle. Save them for later use.
	
	GetItemRect( mainDlg, kMainDlgPICTOne, &pict1Rect );
	GetItemRect( mainDlg, kMainDlgPICTTwo, &pict2Rect );
	GetItemRect( mainDlg, kMainDlgDestPICT, &destRect );
	
	// Draw a box around each of the picture areas to better
	// define them on screen.
	{
		Rect		tempRect;
		
		tempRect = destRect;
		InsetRect( &tempRect, -1, -1 );
		FrameRect( &tempRect );
		tempRect = pict1Rect;
		InsetRect( &tempRect, -1, -1 );
		FrameRect( &tempRect );
		tempRect = pict2Rect;
		InsetRect( &tempRect, -1, -1 );
		FrameRect( &tempRect );
	}
	
	// Draw each of the two "source" pictures into an offscreen
	// Graphics World for use later. For example, when the user
	// passes the cursor over the left picture, regular animation
	// techniques are used to make a circle "float" above the
	// picture (causes flicker). This involves repeatedly
	// copying the original picture to the window, then drawing
	// the circle over it. When the user passes the cursor over
	// the right picture, the original picture is drawn to an
	// offscreen gworld and the circle is drawn over it, then
	// that image is drawn to the window resulting in flicker-free
	// animation of the floating circle.
	//
	// We need the two gworlds here so we have the original
	// pictures to copy from.
	
		// Load picture 1 from resource.
		aPict = GetPicture( kPict1ID );
		if ( !aPict )
		{
			DoAlertStrID( kAlertDialogID, true, kMissingResource );
			ExitToShell();
		}
		
		// Get it's optimum rectangle.
		myErr = GetPictInfo( aPict, &pictInfo, 0, 0, 0, 0 );
		pict1GRect = pictInfo.sourceRect;
		
		// Release any extra items GetPictInfo may have allocated
		// if we're not going to use them.
		if ( pictInfo.commentHandle )
			DisposeHandle( (Handle)pictInfo.commentHandle );
		if ( pictInfo.fontHandle )
			DisposeHandle( (Handle)pictInfo.fontHandle );
		
		// Make a GWorld for the picture.
		myErr = NewGWorld( &pict1GWorld, 0, &pict1GRect, NULL, NULL, 0L );
		if ( myErr || !pict1GWorld )
		{
			DoAlertStrID( kAlertDialogID, true, kGWorldErr );
			ExitToShell();
		}

		// Lock the GWorld's pixels and draw the picture into it.
		LockPixels( GetGWorldPixMap( pict1GWorld ) );
		SetGWorld( (CGrafPtr)pict1GWorld, NULL );
		PaintRect( &pict1GRect );	// first paint it all black
		DrawPicture( aPict, &pict1GRect );
		UnlockPixels( GetGWorldPixMap( pict1GWorld ) );
		
		// Load picture 2 from resource.
		aPict = GetPicture( kPict2ID );
		if ( !aPict )
		{
			DoAlertStrID( kAlertDialogID, true, kMissingResource );
			ExitToShell();
		}
		
		// Get it's optimum rectangle.
		myErr = GetPictInfo( aPict, &pictInfo, 0, 0, 0, 0 );
		pict2GRect = pictInfo.sourceRect;

		// Release any extra items GetPictInfo may have allocated
		// if we're not going to use them.
		if ( pictInfo.commentHandle )
			DisposeHandle( (Handle)pictInfo.commentHandle );
		if ( pictInfo.fontHandle )
			DisposeHandle( (Handle)pictInfo.fontHandle );
		
		// Make a GWorld for the picture.
		myErr = NewGWorld( &pict2GWorld, 0, &pict2GRect, NULL, NULL, 0L );
		if ( myErr || !pict1GWorld )
		{
			DoAlertStrID( kAlertDialogID, true, kGWorldErr );
			ExitToShell();
		}

		// Lock the GWorld's pixels and draw the picture into it.
		LockPixels( GetGWorldPixMap( pict2GWorld ) );
		SetGWorld( (CGrafPtr)pict2GWorld, NULL );
		PaintRect( &pict2GRect );	// first paint it all black
		DrawPicture( aPict, &pict2GRect );
		UnlockPixels( GetGWorldPixMap( pict2GWorld ) );
		
	// Make sure and restore the dialog as the current port.
	
	SetGWorld( (CGrafPtr)mainDlg, NULL );
		
	// Now wait for the user to press a button in the dialog.
	
	itemHit = -1;
	while ( itemHit != kMainDlgQuit )
	{
		WaitNextEvent( everyEvent, &myEvent, GetCaretTime(), NULL );		
		
		// Check for disk events (bad disk mount).
		
		if ( (myEvent.what == diskEvt) &&
			(HiWord(myEvent.message) != noErr) )
		{
			Point		dlgUpLeftCorner = { 100, 80 };	// ignored in Sys 7
			
			DIBadMount( dlgUpLeftCorner, myEvent.message );	// ignore result
		}
		// If we get an update event, redraw the squares around the
		// picture areas.
		else if ( myEvent.what == updateEvt )
		{
			Rect		tempRect;
			
			tempRect = destRect;
			InsetRect( &tempRect, -1, -1 );
			FrameRect( &tempRect );
			tempRect = pict1Rect;
			InsetRect( &tempRect, -1, -1 );
			FrameRect( &tempRect );
			tempRect = pict2Rect;
			InsetRect( &tempRect, -1, -1 );
			FrameRect( &tempRect );
		}
		
		// If the mouse is over one of the pictures, call a routine
		// to animate a "floating circle" over the picture. A
		// different method is used for each picture to demonstrate
		// the differences.
		{
			Point		mouseLoc;
			
			GetMouse( &mouseLoc );
			
			if ( PtInRect( mouseLoc, &pict1Rect ) )
				FlickerAnimate( pict1GWorld, &pict1Rect );
			else if ( PtInRect( mouseLoc, &pict2Rect ) )
				SmoothAnimate( pict2GWorld, &pict2Rect );
		}

		// Pass the event to DialogSelect which takes care of tracking
		// controls and updating everything (except the destination
		// area) for us.
		
		if ( DialogSelect( &myEvent, &aDlg, &itemHit ) )
		{
			if ( myEvent.what == mouseDown )
			{
				// Select the fade speed text field.
				
				SelectTextField( mainDlg, kMainDlgFadeSpeed );
				
				// Get the fade speed from the dialog (as a number).
				
				fadeSpeed = GetDialogNumberField( mainDlg,
					kMainDlgFadeSpeed );
			}
			
			// What we do here depends on what the user clicked
			
			switch( itemHit )
			{
				case kMainDlgCopyOne:
				
					// This is an example of a very courteous CopyBits
					// operation saving and restoring the pen state as
					// well as the fore and background colors and restoring
					// them afterwards. I don't really need to do this here
					// because I know they're all set right in the first
					// place, but they're here as an example.
					
					// Save the pen state and foreground/background colors
					
						GetPenState( &savedPen );
						GetForeColor( &savedFore );
						GetBackColor( &savedBack );
					
					// Set foreground color to black, background color to white
					
						RGBForeColor( &colors[kBlack] );
						RGBBackColor( &colors[kWhite] );
						
					// Just use CopyBits to copy the PICT. We're actually
					// copying from one place on the dialog to another here.

						CopyBits( &WINBITMAP( mainDlg ), &WINBITMAP( mainDlg ),
							&pict1Rect, &destRect, srcCopy, NULL );
					
					// Restore the saved pen state and colors
					
						SetPenState( &savedPen );
						RGBForeColor( &savedFore );
						RGBBackColor( &savedBack );
						
					break;
	
				case kMainDlgFadeOne:
				
					// Call FadeToImage to fade picture 1 into the
					// destination area.
					
					FadeToImage( pict1GWorld, &WINPORTRECT( pict1GWorld ),
						mainDlg, &destRect, fadeSpeed );
					
					break;
	
				case kMainDlgCopyTwo:
				
					// Just use CopyBits to copy the PICT from one place in
					// the window to another.
					
						CopyBits( &WINBITMAP( mainDlg ), &WINBITMAP( mainDlg ),
							&pict2Rect, &destRect, srcCopy, NULL );
						
					break;
	
				case kMainDlgFadeTwo:
				
					// Call FadeToImage to fade picture 2 into the
					// destination area.
					
					FadeToImage( pict2GWorld, &WINPORTRECT( pict2GWorld ),
						mainDlg, &destRect, fadeSpeed );
										
					break;
	
				case kMainDlgErase:
				
					// Erase the destination area.
					
					EraseRect( &destRect );
					
					break;
	
				case kMainDlgFadeToBlack1:
					
					// Call FadeToBlack to fade the destination rectangle.
					
					FadeToBlack1( mainDlg, &destRect, fadeSpeed );

					break;
	
				case kMainDlgFadeToBlack2:
					
					// Call FadeToBlack to fade the destination rectangle.
					
					FadeToBlack2( mainDlg, &destRect, fadeSpeed );

					break;
	
				case kMainDlgFadeToBlack3:
					
					// Call FadeToBlack to fade the destination rectangle.
					
					FadeToBlack3( mainDlg, &destRect, fadeSpeed );

					break;
	
				case kMainDlgPixelize:
				
					// Call Pixelize.
					
					Pixelize( mainDlg, &destRect,
						mainDlg, &destRect, fadeSpeed, true );
					
					break;
				
				case kMainDlgDepixelizeOne:
				
					// Call Pixelize.
					
					LockPixels( GetGWorldPixMap( pict1GWorld ) );
					Pixelize( (WindowPtr)pict1GWorld, &WINPORTRECT( pict1GWorld ),
						mainDlg, &destRect, fadeSpeed, false );
					UnlockPixels( GetGWorldPixMap( pict1GWorld ) );
					
					break;
					
				case kMainDlgDepixelizeTwo:
				
					// Call Pixelize.
					
					LockPixels( GetGWorldPixMap( pict2GWorld ) );
					Pixelize( (WindowPtr)pict2GWorld, &WINPORTRECT( pict2GWorld ),
						mainDlg, &destRect, fadeSpeed, false );
					UnlockPixels( GetGWorldPixMap( pict2GWorld ) );
					
					break;
					
				case kMainDlgFullScreen:
				
					// Fade full screen to black, then image 1, then image 2,
					// then back to original.

					FullScreenDemo( pict1GWorld, pict2GWorld, fadeSpeed );

					break;
					
				case kMainDlgBlur:
				
					// Blur the destination area.
					
					Blur( (WindowPtr)mainDlg, &destRect );
				
					break;
					
				case kMainDlgFlipHoriz:
				
					// Flip the destination area horizontally.
					
					FlipHorizontal( (WindowPtr)mainDlg, &destRect );
					
					break;
					
				case kMainDlgFlipVert:
				
					// Flip the destination area horizontally.
					
					FlipVertical( (WindowPtr)mainDlg, &destRect );
					
					break;
				
				case kMainDlgSlideL:
					
					// Slide picture one into destination from left.
					
					LockPixels( GetGWorldPixMap( pict1GWorld ) );
					Slide( (WindowPtr)pict1GWorld, &WINPORTRECT( pict1GWorld ),
						mainDlg, &destRect, fadeSpeed, kLeft );
					UnlockPixels( GetGWorldPixMap( pict1GWorld ) );
					
					break;
				
				case kMainDlgSlideR:
				
					// Slide picture one into destination from right.
					
					LockPixels( GetGWorldPixMap( pict1GWorld ) );
					Slide( (WindowPtr)pict1GWorld, &WINPORTRECT( pict1GWorld ),
						mainDlg, &destRect, fadeSpeed, kRight );
					UnlockPixels( GetGWorldPixMap( pict1GWorld ) );
					
					break;
				
				case kMainDlgSlideT:
				
					// Slide picture one into destination from top.
					
					LockPixels( GetGWorldPixMap( pict1GWorld ) );
					Slide( (WindowPtr)pict1GWorld, &WINPORTRECT( pict1GWorld ),
						mainDlg, &destRect, fadeSpeed, kTop );
					UnlockPixels( GetGWorldPixMap( pict1GWorld ) );
					
					break;
				
				case kMainDlgSlideB:
				
					// Slide picture one into destination from bottom.
					
					LockPixels( GetGWorldPixMap( pict1GWorld ) );
					Slide( (WindowPtr)pict1GWorld, &WINPORTRECT( pict1GWorld ),
						mainDlg, &destRect, fadeSpeed, kBottom );
					UnlockPixels( GetGWorldPixMap( pict1GWorld ) );
					
					break;
				
				case kMainDlgApertureIn:
				
					// Close a circle in on the destination revealing
					// picture two outside the circle.
					
					LockPixels( GetGWorldPixMap( pict2GWorld ) );
					Aperture( (WindowPtr)pict2GWorld, &WINPORTRECT( pict2GWorld ),
						mainDlg, &destRect, fadeSpeed, kIn );
					UnlockPixels( GetGWorldPixMap( pict2GWorld ) );
					
					break;
				
				case kMainDlgApertureOut:
				
					// Open a circle out on the destination revealing
					// picture two inside the circle.
					
					LockPixels( GetGWorldPixMap( pict2GWorld ) );
					Aperture( (WindowPtr)pict2GWorld, &WINPORTRECT( pict2GWorld ),
						mainDlg, &destRect, fadeSpeed, kOut );
					UnlockPixels( GetGWorldPixMap( pict2GWorld ) );
					
					break;
				
				case kMainDlgTechDemo:
				
					// Open the technical demo window and let the user
					// mess around.
					
					DoTechDemo();

					// Make sure the current port is set back to the
					// main dialog.
					
					SetGWorld( (CGrafPtr)mainDlg, NULL );

					break;
					
				case kMainDlgQuit:
				
					break;
			}
		}
	}
	
	// Get rid of the main dialog.
	
	DisposeDialog( mainDlg );
	
	// Restore the main monitor's inverse table to 4 bits per
	// color (which is the default).

	{
		GDHandle		aDevice;
		
		aDevice = GetGDevice();
		SetGDevice( GetMainDevice() );
		MakeITable( NULL, NULL, 4 );
		SetGDevice( aDevice );
	}
}

// FadeToBlack1 creates a black rectangle in an offscreen GWorld that
// is the same size as the destination area. It then calls FadetoImage
// to fade this into the destination thus fading it to black.
//
void FadeToBlack1( WindowPtr destWin, Rect *destRect, short fadeSpeed )
{
	CGrafPtr		savedPort;
	GDHandle		savedDevice;
	GWorldPtr		offscreenWorld;
	OSErr			myErr;
	
	// Save the current port/device. This should be used instead
	// of GetPort. Though savedPort is defined as a CGrafPtr,
	// the routine may return a GrafPtr or pointer to a GWorld
	// instead (GWorldPtr).
	
		GetGWorld( &savedPort, &savedDevice );

	// Create an offscreen GWorld with the same depth and size as the
	// destination window/rectangle. Even though the rectangle is all the
	// same shade, we make it the same size as the destination because
	// CopyBits works much faster when it doesn't have to resize the
	// image. The parameters are as follows:
	//		&offscreenWorld		ptr to new graphics world
	//		0					bit depth same as graphics device of dest rect
	//		destRect			bounds rectangle of my GWorld
	//		NULL				handle to a color table record - NULL means
	//							use the default record for that depth
	//		NULL				handle to a graphics device record - we aren't
	//							creating a new graphics device
	//		0L					no flags
	
		myErr = NewGWorld( &offscreenWorld, 0, destRect, NULL, NULL, 0L );
	
	// Make sure the GWorld was created
	
		if ( myErr )
		{
			DoAlertStrID( kAlertDialogID, true, kGWorldErr );
			ExitToShell();
		}
	
	// Get the GWorld's pixel map handle and lock the pixels while we're
	// drawing to the GWorld. We have to do this because a GWorld actually
	// holds its pixel map in a handle rather than a pointer. LockPixels
	// makes sure the pixel map doesn't move. You should lock a GWorld's
	// pixels before drawing to or copying from the GWorld (and unlock
	// them afterward).
	
		LockPixels( GetGWorldPixMap( offscreenWorld ) );
	
	// Make the GWorld the current port.
	
		SetGWorld( offscreenWorld, NULL );
		
	// Paint the offscreen GWorld all black.
	
		PaintRect( &WINPORTRECT( offscreenWorld ) );
			
	// Restore the saved port/device.
	
		SetGWorld( savedPort, savedDevice );
		
	// Unlock the pixels of the GWorld.
	
		UnlockPixels( GetGWorldPixMap( offscreenWorld ) );
	
	// Call FadeToImage to fade the black rectangle to the destination.
	
		FadeToImage( offscreenWorld, &WINPORTRECT( offscreenWorld ),
			destWin, destRect, fadeSpeed );
	
	// Now get rid of the black GWorld.
	
		DisposeGWorld( offscreenWorld );
}

// FadeToBlack2 uses CopyBits and an offscreen graphics world to
// fade an area (specified by destRect) in a window (specified
// by destWin) to black. It does this by creating an offscreen
// graphics world, painting the GWorld gray, then repeatedly
// copying the gray rectangle to the destination area using the
// subPin transfer method of CopyBits. This causes the destination
// to grow darker and darker.
//
void FadeToBlack2( WindowPtr destWin, Rect *destRect, short fadeSpeed )
{
	CGrafPtr		savedPort;
	GDHandle		savedDevice;
	GWorldPtr		offscreenWorld;
	Rect			offscreenRect;
	OSErr			myErr;
	PenState		savedPen;
	RGBColor		savedFore;
	RGBColor		savedBack;
	long			repetitions;
	RGBColor		grayColor;
	
	// Save the current port/device. This should be used instead
	// of GetPort. Though savedPort is defined as a CGrafPtr,
	// the routine may return a GrafPtr or pointer to a GWorld
	// instead (GWorldPtr).
	
		GetGWorld( &savedPort, &savedDevice );

	// Set the current port to the destination window
	
		SetGWorld( (CGrafPtr)destWin, NULL );
	
	// Save the pen state and color info of the destination window
	
		GetPenState( &savedPen );
		GetForeColor( &savedFore );
		GetBackColor( &savedBack );
	
	// Make sure the foreground color of the destination window
	// is black and the background color is white.
	
		RGBForeColor( &colors[kBlack] );
		RGBBackColor( &colors[kWhite] );
		
	// Convert the destination rectangle into global coordinates
	
		offscreenRect = (*destRect);
		LocalToGlobal( &TOPLEFT(offscreenRect) );
		LocalToGlobal( &BOTRIGHT(offscreenRect) );
	
	// Make the offscreen rectangle smaller. It doesn't matter since it's
	// all the same color and it'll use up less memory.
	
		offscreenRect.right = offscreenRect.left + 2;
		offscreenRect.bottom = offscreenRect.top + 2;
		
	// Create an offscreen GWorld with the same depth as the
	// destination window/rectangle. The parameters are as follows:
	//		&offscreenWorld		ptr to new graphics world
	//		0					bit depth same as graphics device of dest rect
	//		&offscreenRect		bounds rectangle of my GWorld
	//		NULL				handle to a color table record - NULL means
	//							use the default record for that depth
	//		NULL				handle to a graphics device record - we aren't
	//							creating a new graphics device
	//		0L					no flags
	
		myErr = NewGWorld( &offscreenWorld, 0, &offscreenRect, NULL, NULL, 0L );
	
	// Make sure the GWorld was created
	
		if ( myErr )
		{
			DoAlertStrID( kAlertDialogID, true, kGWorldErr );
			ExitToShell();
		}
	
	// Make the GWorld the current port
	
		SetGWorld( offscreenWorld, NULL );
		
	// Get the GWorld's pixel map handle and lock the pixels while we're
	// drawing in the GWorld. We have to do this because a GWorld actually
	// holds its pixel map in a handle rather than a pointer. LockPixels
	// makes sure the pixel map doesn't move. You should lock a GWorld's
	// pixels before drawing to or copying from the GWorld (and unlock
	// them afterward).
	
		LockPixels( GetGWorldPixMap( offscreenWorld ) );
	
	// The speed of the fade determines what shade of gray we'll paint
	// the offscreen GWorld. fadeSpeed can range from 0 to 20 with 20
	// being the fastest.
	{
		unsigned short	maxValue=0xFFFF;
		
		if ( fadeSpeed <= 0 ) fadeSpeed = 1;	// make sure speed is not zero
		if ( fadeSpeed > 20 ) fadeSpeed = 20;	// make sure not over 20
		
		grayColor.blue = (fadeSpeed * 625) + 8000;
		grayColor.red = grayColor.green = grayColor.blue;
		RGBForeColor( &grayColor );
		repetitions = maxValue / grayColor.blue;
	}

	// Paint it all gray
	
		PaintRect( &(offscreenWorld->portRect) );
	
	// Make the destination window the current port
	
		SetGWorld( (CGrafPtr)destWin, NULL );
		
	// Loop and use CopyBits to copy the gray rectangle into the
	// destination window/rect using the subPin transfer method
	// so that the picture gets darker and darker.
	
		while ( repetitions )
		{
			CopyBits( &WINBITMAP( offscreenWorld ), &WINBITMAP( destWin ),
				&(offscreenWorld->portRect), destRect, subPin, NULL );

			repetitions--;
		}

	// Make the GWorld the current port
	
		SetGWorld( offscreenWorld, NULL );
		
	// Now we'll do a fast fade just to make sure everything went completely
	// to black.
	{
		unsigned short	maxValue=0xFFFF;
		
		grayColor.blue = 20500;
		grayColor.red = grayColor.green = grayColor.blue;
		RGBForeColor( &grayColor );
		repetitions = maxValue / grayColor.blue + 1;
	}

	// Paint it all gray
	
		PaintRect( &(offscreenWorld->portRect) );
	
	// Make the destination window the current port
	
		SetGWorld( (CGrafPtr)destWin, NULL );
		
	// Loop and use CopyBits to copy the gray rectangle into the
	// destination window/rect using the subPin transfer method
	// so that the picture gets darker and darker.
	
		while ( repetitions )
		{
			CopyBits( &WINBITMAP( offscreenWorld ), &WINBITMAP( destWin ),
				&(offscreenWorld->portRect), destRect, subPin, NULL );

			repetitions--;
		}

	// Unlock the pixels again.
	
		UnlockPixels( GetGWorldPixMap( offscreenWorld ) );
	
	// Paint the destination rectangle all black.
	
		PaintRect( destRect );
		
	// Restore the saved port/device
	
		SetGWorld( savedPort, savedDevice );
		
	// Restore the pen state and color info of the destination window
	
		SetPenState( &savedPen );
		RGBForeColor( &savedFore );
		RGBBackColor( &savedBack );
		
	// Dispose of the GWorld
	
		DisposeGWorld( offscreenWorld );
}


// FadeToBlack3 uses CopyBits and an offscreen graphics world to
// fade an area (specified by destRect) in a window (specified
// by destWin) to black. It does this by creating an offscreen
// graphics world, and repeatedly copying darker and darker
// shades of gray into the destination using the adMin transfer
// method of CopyBits. This method compares the pixels in the
// source (gray) and destination images and replaces the pixel
// in the destination if the source is darker (closer to zero).
// This has the effect of making the destination darker and
// darker starting with the lighter colors and ending with the
// darker colors and eventually blacking the destination out.
//
void FadeToBlack3( WindowPtr destWin, Rect *destRect, short fadeSpeed )
{
	CGrafPtr		savedPort;
	GDHandle		savedDevice;
	GWorldPtr		offscreenWorld;
	Rect			offscreenRect;
	OSErr			myErr;
	RGBColor		savedFore;
	RGBColor		savedBack;
	RGBColor		grayColor;
	long			colorStep;
	short			speed;
	
	// Save the current port/device. This should be used instead
	// of GetPort. Though savedPort is defined as a CGrafPtr,
	// the routine may return a GrafPtr or pointer to a GWorld
	// instead (GWorldPtr).
	
		GetGWorld( &savedPort, &savedDevice );

	// Set the current port to the destination window.
	
		SetGWorld( (CGrafPtr)destWin, NULL );
	
	// Save the color info of the destination window.
	
		GetForeColor( &savedFore );
		GetBackColor( &savedBack );
	
	// Make sure the foreground color of the destination window
	// is black and the background color is white.
	
		RGBForeColor( &colors[kBlack] );
		RGBBackColor( &colors[kWhite] );
		
	// Convert the destination rectangle into global coordinates.
	
		offscreenRect = (*destRect);
		LocalToGlobal( &TOPLEFT(offscreenRect) );
		LocalToGlobal( &BOTRIGHT(offscreenRect) );
	
	// Create an offscreen GWorld with the same depth and size as the
	// destination window/rectangle. Even though the rectangle is all the
	// same shade, we make it the same size as the destination because
	// CopyBits works much faster when it doesn't have to resize the
	// image. The parameters are as follows:
	//		&offscreenWorld		ptr to new graphics world
	//		0					bit depth same as graphics device of dest rect
	//		&offscreenRect		bounds rectangle of my GWorld
	//		NULL				handle to a color table record - NULL means
	//							use the default record for that depth
	//		NULL				handle to a graphics device record - we aren't
	//							creating a new graphics device
	//		0L					no flags
	
		myErr = NewGWorld( &offscreenWorld, 0, &offscreenRect, NULL, NULL, 0L );
	
	// Make sure the GWorld was created
	
		if ( myErr )
		{
			DoAlertStrID( kAlertDialogID, true, kGWorldErr );
			ExitToShell();
		}
	
	// Get the GWorld's pixel map handle and lock the pixels while we're
	// drawing to the GWorld. We have to do this because a GWorld actually
	// holds its pixel map in a handle rather than a pointer. LockPixels
	// makes sure the pixel map doesn't move. You should lock a GWorld's
	// pixels before drawing to or copying from the GWorld (and unlock
	// them afterward).
	
		LockPixels( GetGWorldPixMap( offscreenWorld ) );
	
	// Determine the number to subtract from the gray shade every time we
	// loop. This is determined by the fadeSpeed parameter.
	
		if ( fadeSpeed < 0 )
			speed = 0;
		else if ( fadeSpeed > 20 )
			speed = 20;
		else
			speed = fadeSpeed;
			
		colorStep = ( 0xFFFF / (21-fadeSpeed) ) / 2;
		
	// Our gray color just starts out as white.
	
		grayColor = colors[kWhite];
	
	// Loop until the gray color is black.
	
		while ( grayColor.blue > 0 )
		{
			// Make the GWorld the current port.
			
			SetGWorld( offscreenWorld, NULL );
				
			// Subtract the color step value from the gray value and
			// paint the offscreen GWorld.
			
			if ( grayColor.blue > colorStep )
				grayColor.blue -= colorStep;
			else
				grayColor.blue = 0;
			grayColor.green = grayColor.red = grayColor.blue;
			RGBForeColor( &grayColor );
			PaintRect( &WINPORTRECT( offscreenWorld ) );
			
			// Make the destination window the current port.
			
			SetGWorld( (CGrafPtr)destWin, NULL );
			
			// Use CopyBits with the adMin transfer mode to copy the
			// gray rectangle to the destination window/area.
			
			CopyBits( &WINBITMAP( offscreenWorld ),
				&WINBITMAP( destWin ),
				&WINPORTRECT( offscreenWorld ),
				destRect, adMin, NULL );
		}
		
	// Set the current port to the destination window.
	
		SetGWorld( (CGrafPtr)destWin, NULL );
	
	// Restore the color info of the destination window.
	
		RGBForeColor( &savedFore );
		RGBBackColor( &savedBack );
		
	// Restore the saved port/device.
	
		SetGWorld( savedPort, savedDevice );
		
	// Unlock the pixels of the GWorld and dispose of it.
	
		UnlockPixels( GetGWorldPixMap( offscreenWorld ) );
		DisposeGWorld( offscreenWorld );
}

// FadeToImage takes the source image and does a smooth fade in
// the destination window and rect from the image that is currently
// there to the source image. This is done using CopyBits with
// the "blend" transfer mode.
//
void FadeToImage( GWorldPtr srcImage, Rect *srcRect,
	WindowPtr destWin, Rect *destRect, short fadeSpeed )
{
	CGrafPtr			savedPort;
	GDHandle			savedDevice;
	RGBColor			savedFore, savedBack;
	PenState			savedPen;
	RGBColor			grayColor;
	float				speed;
	unsigned short		lastColor;
	
	// Make sure speed is within our limits and set it appropriately.
	// We want to end up with a range of 1.2 to 3.2 from the original
	// 0 to 20 range.
	
	if ( fadeSpeed < 1 ) speed = 1.2;
	else if ( fadeSpeed > 20 ) speed = 3.2;
	else
	{
		speed = fadeSpeed;
		speed = 1.2 + (speed/10);
	}
	
	// Save the current port and device
	
		GetGWorld( &savedPort, &savedDevice );
		
	// Set the port to the destination window and save its
	// pen state and fore/background color values.
	
		SetGWorld( (CGrafPtr)destWin, NULL );
		GetPenState( &savedPen );
		GetForeColor( &savedFore );
		GetBackColor( &savedBack );
	
	// Make sure the port's fore and background colors are set
	// correctly for CopyBits to work correctly.
		
		RGBForeColor( &colors[kBlack] );
		RGBBackColor( &colors[kWhite] );
			
	// Lock the pixels in the source image. This is required
	// when copying from a GWorld.
		
		LockPixels( GetGWorldPixMap( srcImage ) );
	
	// Set the beginning blend weight. This will be multiplied by
	// speed before being used the first time.
	
		grayColor.blue = grayColor.green = grayColor.red = 0x0F00;
		lastColor = 0;
		
	// Loop and copy the source image to the destination
	// blending more and more of the source image in by using
	// a lighter and lighter "blend weight" (set by the OpColor
	// function).

		while ( grayColor.blue < 0xFFFF )
		{
			// Lighten the blend weight color.
			
			grayColor.blue *= speed;
			
			// Make sure the number didn't roll over.
			
			if ( grayColor.blue < lastColor )
				grayColor.blue = 0xFFFF;
			
			grayColor.red = grayColor.green = grayColor.blue;

			// Remember the number for next time.
			
			lastColor = grayColor.blue;
			
			// Set the new blend weight.
			
			OpColor( &grayColor );
			
			// copy the image
			
			CopyBits( &WINBITMAP( srcImage ),
				&WINBITMAP( destWin ),
				srcRect, destRect, blend, NULL );
		}

	// Finally, just copy the source image to the destination
	// window unchanged to make sure the transformation is
	// complete.
	
		CopyBits( &WINBITMAP( srcImage ),
			&WINBITMAP( destWin ),
			srcRect, destRect, srcCopy, NULL );
	
	// Reset the blend weight to black.
	// This is the "normal" weight.
	
		OpColor( &colors[kBlack] );
	
	// Unlock the pixels in the source image.
	
		UnlockPixels( GetGWorldPixMap( srcImage ) );
		
	// Restore the destination window's properties.
	
		SetPenState( &savedPen );
		RGBForeColor( &savedFore );
		RGBBackColor( &savedBack );
		
	// Restore the current port and device.
	
		SetGWorld( savedPort, savedDevice );
}

// Pixelize makes a picture "blockier" in several steps.
// It first copies the original picture from the destination
// area, then repeatedly copies the original picture
// into areas smaller than the size of the previous one,
// using CopyBits to scale the picture back up into the
// destination window at the original size.
//
// The srcWin/srcRect and destWin/destRect may be the same
// window/area to pixelize, but not to depixelize.
// When depixelizing, srcWin will usually be a GWorldPtr
// containing an image the same size as the destination area.
//
void Pixelize( WindowPtr srcWin, Rect *srcRect,
	WindowPtr destWin, Rect *destRect, short speed, Boolean pixelizing )
{
	GWorldPtr		srcImage;
	GWorldPtr		intermedImage;
	Rect			intermedRect;
	CGrafPtr		savedPort;
	GDHandle		savedDevice;
	OSErr			myErr;
	float			theSpeed;
	Boolean			done;
	
	// Make sure speed is within our limits and set it appropriately.
	// We want to end up with a range of 1.1 to 2.1 from the original
	// 0 to 20 range.
	
	if ( speed < 1 ) theSpeed = 1.1;
	else if ( speed > 20 ) theSpeed = 2.1;
	else
	{
		theSpeed = speed;
		theSpeed = 1.1 + (theSpeed/20);
	}
	
	// Save the original port and device.
	
	GetGWorld( &savedPort, &savedDevice );
	
	// Create an offscreen GWorld with the same depth as the
	// destination window/rectangle. The parameters are as follows:
	//		&srcImage			ptr to new graphics world
	//		0					bit depth same as graphics device of dest rect
	//		destRect			bounds rectangle of my GWorld
	//		NULL				handle to a color table record - NULL means
	//							use the default record for that depth
	//		NULL				handle to a graphics device record - we aren't
	//							creating a new graphics device
	//		0L					no flags
	
		myErr = NewGWorld( &srcImage, 0, destRect, NULL, NULL, 0L );
	
	// Make sure the GWorld was created
	
		if ( myErr )
		{
			DoAlertStrID( kAlertDialogID, true, kGWorldErr );
			ExitToShell();
		}
	
	// Also create an intermediate GWorld used in the scaling/copying.
	
		myErr = NewGWorld( &intermedImage, 0, destRect, NULL, NULL, 0L );
		
	// Make sure the GWorld was created
	
		if ( myErr )
		{
			DoAlertStrID( kAlertDialogID, true, kGWorldErr );
			ExitToShell();
		}
	
	// Copy the source image into the GWorld.
	
	SetGWorld( srcImage, NULL );
	CopyBits( &WINBITMAP( srcWin ), &WINBITMAP( srcImage ),
		srcRect, &WINPORTRECT( srcImage ), srcCopy, NULL );
	
	// Lock the pixels of the source and intermediate GWorlds
	// since we'll be copying from them.
	
	LockPixels( GetGWorldPixMap( srcImage ) );
	LockPixels( GetGWorldPixMap( intermedImage ) );
	
	// We need a beginning intermediate image size.
	
	if ( pixelizing )
		intermedRect = WINPORTRECT( intermedImage );
	else
	{
		intermedRect = WINPORTRECT( intermedImage );
		intermedRect.bottom /= 15;
		intermedRect.right /= 15;
	}
	
	// Loop and copy the image into smaller GWorlds.
	
	done = false;
	while ( !done )
	{
		// Adjust the intermediate rectangle size and determine if
		// this will be the last time through the loop.
		
		if ( pixelizing )
		{
			intermedRect.bottom /= theSpeed;
			intermedRect.right /= theSpeed;

			if ( intermedRect.bottom < ( WINPORTRECT( intermedImage ).bottom / 10 ) )
				done = true;
		}
		else
		{
			intermedRect.bottom *= theSpeed;
			intermedRect.right *= theSpeed;
			
			if (( intermedRect.bottom > WINPORTRECT( intermedImage ).bottom ) ||
				( intermedRect.right > WINPORTRECT( intermedImage ).right ))
			{
				intermedRect = WINPORTRECT( intermedImage );
				done = true;
			}
		}
		
		// Copy the original image into the intermediate GWorld.
		
		SetGWorld( (CGrafPtr)intermedImage, NULL );
		CopyBits( &WINBITMAP( srcImage ), &WINBITMAP( intermedImage ),
			&WINPORTRECT( srcImage ), &intermedRect, srcCopy, NULL );
		
		// Copy the intermediate GWorld to the destination.
		
		SetGWorld( (CGrafPtr)destWin, NULL );
		CopyBits( &WINBITMAP( intermedImage ), &WINBITMAP( destWin ),
			&intermedRect, destRect, srcCopy, NULL );
	}
	
	// Unlock everyone's pixels again.

	UnlockPixels( GetGWorldPixMap( srcImage ) );
	UnlockPixels( GetGWorldPixMap( intermedImage ) );

	// Restore the original port and device.
	
	SetGWorld( savedPort, savedDevice );
	
	// Destroy the GWorld holding the original image and the
	// intermediate GWorld.
	
	DisposeGWorld( srcImage );
	DisposeGWorld( intermedImage );
}

// FlickerAnimate animates a colored circle floating over
// a picture at the mouse location. It draws directly to
// the screen resulting in a circle that the background
// occasionally "flickers" through. Drawing will occur in
// the current port.
//
void FlickerAnimate( GWorldPtr srcPict, Rect *destRect )
{
	Point		mouseLoc;
	Rect		mouseRect;
	RGBColor	savedFore, savedBack;
	RgnHandle		savedClip;
	RGBColor	redColor;
	long		ignore;
		
	// Make the color to draw the circle with.
	
	redColor.blue = redColor.green = 0;
	redColor.red = 0xFFFF;
	
	// Save fore and background colors and make them
	// fore-black and back-white.
	
	GetForeColor( &savedFore ); GetBackColor( &savedBack );
	RGBForeColor( &colors[kBlack] ); RGBBackColor( &colors[kWhite] );
	
	// Save the clipping region.
	
	savedClip = NewRgn();
	GetClip( savedClip );

	// Set the clipping region to just cover the picture.
	
	ClipRect( destRect );

	// Lock the pixel map of the src GWorld while copying from it.
	
	LockPixels( GetGWorldPixMap( srcPict ) );

	// Loop and draw the picture and circle repeatedly.
	
	do
	{
		// Get a mouse location.
		
			GetMouse( &mouseLoc );
		
		// Make a square around the mouse location to draw
		// the circle in.
		
			mouseRect.left = mouseRect.right = mouseLoc.h;
			mouseRect.top = mouseRect.bottom = mouseLoc.v;
			InsetRect( &mouseRect, -20, -20 );
	
		// Copy the picture to the window.

			CopyBits( &WINBITMAP( srcPict ),
				&WINBITMAP( FrontWindow() ),
				&WINPORTRECT( srcPict ),
				destRect, srcCopy, NULL );
			
		// Draw the circle at the mouse location.
		
			RGBForeColor( &redColor );
			PaintOval( &mouseRect );
			RGBForeColor( &colors[kBlack] );
		
		// Delay for a fraction of a second here. If we
		// don't, the flickering is really, really bad.
		
			Delay( 2, &ignore );
			
	} while ( PtInRect( mouseLoc, destRect ) );

	// Draw the picture one more time to make sure the
	// circle is erased.
	
		CopyBits( &WINBITMAP( srcPict ),
			&WINBITMAP( FrontWindow() ),
			&WINPORTRECT( srcPict ),
			destRect, srcCopy, NULL );

	// Unlock the GWorld's pixel map.
	
		UnlockPixels( GetGWorldPixMap( srcPict ) );

	// Restore the clipping region.
	
		SetClip( savedClip );
	
	// Restore the saved fore and background colors.
	
		RGBForeColor( &savedFore );
		RGBBackColor( &savedBack );
}

// SmoothAnimate animates a colored circle floating over
// a picture at the mouse location. It draws to a GWorld
// then copies the GWorld to the screen. This results in
// flicker-free animation where the background never
// shows through the floating circle.
//
void SmoothAnimate( GWorldPtr srcPict, Rect *destRect )
{
	Point			mouseLoc;
	Rect			mouseRect;
	RGBColor		savedFore, savedBack;
	PixMapHandle	srcPixMap, midPixMap;
	GWorldPtr		middleMan;
	CGrafPtr		thisWindow;
	GDHandle		thisDevice;
	RGBColor		purpleColor;
	OSErr			myErr;
	
	// Save the current window and device (this is the window
	// we're drawing into.)
	
	GetGWorld( &thisWindow, &thisDevice );
	
	// Make the color to draw the circle with.
	
	purpleColor.blue = purpleColor.red = 0xFFFF;
	purpleColor.green = 0;
	
	// Save fore and background colors and make them
	// fore-black and back-white.
	
	GetForeColor( &savedFore ); GetBackColor( &savedBack );
	RGBForeColor( &colors[kBlack] ); RGBBackColor( &colors[kWhite] );
	
	// Make a GWorld to draw the picture and circle into. The
	// onscreen image will be copied from this "middle man."
	// It is the same size and depth as the destination
	// on screen.
	//		&middleMan			ptr to new graphics world
	//		0					bit depth same as graphics device of dest rect
	//		destRect			bounds rectangle of new GWorld
	//		NULL				handle to a color table record - NULL means
	//							use the default record for that depth
	//		NULL				handle to a graphics device record - we aren't
	//							creating a new graphics device
	//		0L					no flags
	
		myErr = NewGWorld( &middleMan, 0, destRect, NULL, NULL, 0L );

	// Make sure the GWorld was created
	
		if ( myErr || !middleMan )
		{
			DoAlertStrID( kAlertDialogID, true, kGWorldErr );
			ExitToShell();
		}
	
	// Lock the pixel map of the src GWorld and the middleMan
	// GWorld while copying from them.
	
		srcPixMap = GetGWorldPixMap( srcPict );
		LockPixels( srcPixMap );
		midPixMap = GetGWorldPixMap( middleMan );
		LockPixels( midPixMap );

	// Loop and draw the pict and the circle repeatedly.
	
	do
	{
		// Get a mouse location and translate into coordinates
		// local to the middleMan GWorld.
		
			GetMouse( &mouseLoc );
			mouseLoc.h -= destRect->left;
			mouseLoc.v -= destRect->top;

		// Make a square around the mouse location to draw
		// the circle in.
		
			mouseRect.left = mouseRect.right = mouseLoc.h;
			mouseRect.top = mouseRect.bottom = mouseLoc.v;
			InsetRect( &mouseRect, -20, -20 );
	
		// Set the current port to middleMan.
		
			SetGWorld( (CGrafPtr)middleMan, NULL );
		
		// Copy the picture to the middleMan GWorld.

			CopyBits( &WINBITMAP( srcPict ),
				&WINBITMAP( middleMan ),
				&WINPORTRECT( srcPict ),
				&WINPORTRECT( middleMan ), srcCopy, NULL );
		
		// Draw the circle at the mouse location.
		
			RGBForeColor( &purpleColor );
			PaintOval( &mouseRect );
			RGBForeColor( &colors[kBlack] );
		
		// Reset the current port to the destination window.
		
			SetGWorld( thisWindow, thisDevice );
		
		// Now copy the middleMan image to the destination
		// window on screen.
		
			CopyBits( &WINBITMAP( middleMan ),
				&WINBITMAP( thisWindow ),
				&WINPORTRECT( middleMan ),
				destRect, srcCopy, NULL );
				
	} while ( PtInRect( mouseLoc, &WINPORTRECT( middleMan ) ) );

	// Draw the picture one more time to make sure the
	// circle is erased.
	
		CopyBits( &WINBITMAP( srcPict ),
			&WINBITMAP( FrontWindow() ),
			&WINPORTRECT( srcPict ),
			destRect, srcCopy, NULL );

	// Unlock the GWorld's and middleMan's pixel maps.
	
		UnlockPixels( srcPixMap );
		UnlockPixels( midPixMap );
	
	// Dispose of the middleMan GWorld.
	
		DisposeGWorld( middleMan );

	// Restore the saved fore and background colors.
	
		RGBForeColor( &savedFore );
		RGBBackColor( &savedBack );
}

// FullScreenDemo fades the whole screen to black, then to picture one
// then to picture two and finally to the original screen (which is
// saved in a GWorld).
//
void FullScreenDemo( GWorldPtr pict1, GWorldPtr pict2, short fadeSpeed )
{
	// Fade full screen to black, then image 1, then image 2,
	// then back to original.
	
	OSErr			myErr;
	GWorldPtr		theScreen;
	CGrafPort		myPort;
	CGrafPtr		savedPort;
	GDHandle		savedDevice;
	
	// If Color QuickDraw is not available, don't do this.
	
		if ( !HasColorQuickDraw() )
			return;
	
	// Save the current port and device.
	
		GetGWorld( &savedPort, &savedDevice );
	
	// Open a port that covers the main screen. OpenCPort, by default,
	// makes the port the same size as the main screen.
	
		OpenCPort( &myPort );
		SetGWorld( &myPort, NULL );
	
	// Make a GWorld to store the current screen picture in.
	
		myErr = NewGWorld( &theScreen, 0, &WINPORTRECT( &myPort ), NULL, NULL, 0L );
		if ( myErr )
		{
			CloseCPort( &myPort );
			DoAlertStrID( kAlertDialogID, true, kGWorldErr );
			ExitToShell();
			return;
		}

	// Copy the screen picture into the GWorld.
	
		CopyBits( &WINBITMAP( &myPort ),
					&WINBITMAP( theScreen ),
					&WINPORTRECT( &myPort ),
					&WINPORTRECT( theScreen ),
					srcCopy, NULL );

	// Now fade the screen to black.
	
		FadeToBlack2( (WindowPtr)&myPort, &WINPORTRECT( &myPort ), fadeSpeed );
		
	// Slide the first image in from the left.
	
		LockPixels( GetGWorldPixMap( pict1 ) );
		Slide( (WindowPtr)pict1, &WINPORTRECT( pict1 ),
			(WindowPtr)&myPort, &WINPORTRECT( &myPort ), fadeSpeed, kLeft );
		UnlockPixels( GetGWorldPixMap( pict1 ) );
		
	// Fade the second image in.
	
		FadeToImage( pict2, &WINPORTRECT( pict2 ),
			(WindowPtr)&myPort, &WINPORTRECT( &myPort ), fadeSpeed );
			
	// Slide the original screen back in from the right.
	
		LockPixels( GetGWorldPixMap( theScreen ) );
		Slide( (WindowPtr)theScreen, &WINPORTRECT( theScreen ),
			(WindowPtr)&myPort, &WINPORTRECT( &myPort ), fadeSpeed, kRight );
		UnlockPixels( GetGWorldPixMap( theScreen ) );
	
	// Now get rid of the GWorld we used to hold the screen image.
	
		DisposeGWorld( theScreen );

	// Close the port.
	
		CloseCPort( &myPort );
	
	// Restore the saved port and device.
	
		SetGWorld( savedPort, savedDevice );
}

// Blur blurs the destination picture by varying the destination image
// in an offscreen GWorld slightly and blending the varied images back
// into the original destination image.
//
void Blur( WindowPtr destWin, Rect *destRect )
{
	CGrafPtr		savedPort;
	GDHandle		savedDevice;
	GWorldPtr		offscreenWorld;
	Rect			copyRect;
	RGBColor		savedFore, savedBack;
	RGBColor		grayColor;
	OSErr			myErr;
	short			x;
	RgnHandle		savedClip;
	
	// Save the current device and port.
	
	GetGWorld( &savedPort, &savedDevice );
	
	// Create a GWorld the same size as the destination image
	// with the same depth as the destination window/rectangle.
	// The parameters are as follows:
	//		&offscreenWorld		ptr to new graphics world
	//		0					bit depth same as graphics device of dest rect
	//		destRect			bounds rectangle of my GWorld
	//		NULL				handle to a color table record - NULL means
	//							use the default record for that depth
	//		NULL				handle to a graphics device record - we aren't
	//							creating a new graphics device
	//		0L					no flags
	
		myErr = NewGWorld( &offscreenWorld, 0, destRect, NULL, NULL, 0L );
	
	// Make sure the GWorld was created
	
		if ( myErr )
		{
			DoAlertStrID( kAlertDialogID, true, kGWorldErr );
			ExitToShell();
		}
		
	// Make the GWorld the current port.
	
	SetGWorld( (CGrafPtr)offscreenWorld, NULL );
	
	// Copy the destination image into the GWorld.
	
	CopyBits( &WINBITMAP( destWin ), &WINBITMAP( offscreenWorld ),
		destRect, &WINPORTRECT( offscreenWorld ), srcCopy, NULL );
	
	// Make the destination window the current port.
	
	SetGWorld( (CGrafPtr)destWin, NULL );
	
	// Save the destination window's fore/background colors.
	
	GetBackColor( &savedBack );
	GetForeColor( &savedFore );
	
	// Make sure the destination window's fore/background colors
	// are black/white.
	
	RGBForeColor( &colors[kBlack] );
	RGBBackColor( &colors[kWhite] );
	
	// Save the destination window's clip region and set it to
	// the destination rectangle.
	
	savedClip = NewRgn();
	GetClip( savedClip );
	ClipRect( destRect );
	
	// Set the OpColor for blending.

	grayColor.red = grayColor.green = grayColor.blue = 0x4000;
	OpColor( &grayColor );

	// Lock the GWorld's pixels while we copy from it.
	
	LockPixels( GetGWorldPixMap( offscreenWorld ) );
	
	// Blend the GWorld image back into the destination image in
	// a circular pattern.
	
	for ( x=1; x<9; x++ )
	{
		copyRect = *destRect;
		
		switch( x )
		{
			case 1:
				OffsetRect( &copyRect, -2, 0 );
				break;
			case 2:
				OffsetRect( &copyRect, -1, -1 );
				break;
			case 3:
				OffsetRect( &copyRect, 0, -2 );
				break;
			case 4:
				OffsetRect( &copyRect, 1, -1 );
				break;
			case 5:
				OffsetRect( &copyRect, 2, 0 );
				break;
			case 6:
				OffsetRect( &copyRect, 1, 1 );
				break;
			case 7:
				OffsetRect( &copyRect, 0, 2 );
				break;
			case 8:
				OffsetRect( &copyRect, -1, 1 );
				break;
		}
		
		CopyBits( &WINBITMAP( offscreenWorld ), &WINBITMAP( destWin ),
			&WINPORTRECT( offscreenWorld ), &copyRect, blend, NULL );
	}

	// Unlock the GWorld's pixels.
	
	UnlockPixels( GetGWorldPixMap( offscreenWorld ) );
	
	// Reset OpColor to black.
	
	OpColor( &colors[kBlack] );
	
	// Get rid of the GWorld.
	
	DisposeGWorld( offscreenWorld );
	
	// Restore the destination window's fore/background colors.
	
	RGBForeColor( &savedFore );
	RGBBackColor( &savedBack );
	
	// Restore the destination window's clipping region.
	
	SetClip( savedClip );
	DisposeRgn( savedClip );
	
	// Restore the saved port and device.
	
	SetGWorld( savedPort, savedDevice );
}

// FlipHorizontal copies the image in the window and rectangle
// specified and draws it back out reversed horizontally.
//
void FlipHorizontal( WindowPtr destWin, Rect *destRect )
{
	CGrafPtr		savedPort;
	GDHandle		savedDevice;
	GWorldPtr		offscreenWorld;
	RGBColor		savedFore, savedBack;
	OSErr			myErr;
	short			x;
	Rect			copyFrom, copyTo;
	
	// Save the current device and port.
	
	GetGWorld( &savedPort, &savedDevice );
	
	// Create a GWorld the same size as the destination image
	// with the same depth as the destination window/rectangle.
	// The parameters are as follows:
	//		&offscreenWorld		ptr to new graphics world
	//		0					bit depth same as graphics device of dest rect
	//		destRect			bounds rectangle of my GWorld
	//		NULL				handle to a color table record - NULL means
	//							use the default record for that depth
	//		NULL				handle to a graphics device record - we aren't
	//							creating a new graphics device
	//		0L					no flags
	
		myErr = NewGWorld( &offscreenWorld, 0, destRect, NULL, NULL, 0L );
	
	// Make sure the GWorld was created
	
		if ( myErr )
		{
			DoAlertStrID( kAlertDialogID, true, kGWorldErr );
			ExitToShell();
		}
		
	// Make the GWorld the current port.
	
	SetGWorld( (CGrafPtr)offscreenWorld, NULL );
	
	// Set up the copy from and to rectangles.
	
	copyFrom = *destRect;
	copyFrom.right = copyFrom.left + 1;
	copyTo = WINPORTRECT( offscreenWorld );
	copyTo.left = copyTo.right - 1;
	
	// Copy the destination image into the GWorld column by column
	// reversing it as we go.
	
	for ( x=0; x<WINDOWWIDTH(destWin); x++ )
	{
		CopyBits( &WINBITMAP( destWin ), &WINBITMAP( offscreenWorld ),
			&copyFrom, &copyTo, srcCopy, NULL );
		
		OffsetRect( &copyFrom, 1, 0 );
		OffsetRect( &copyTo, -1, 0 );
	}
	
	// Make the destination window the current port.
	
	SetGWorld( (CGrafPtr)destWin, NULL );
	
	// Save the destination window's fore/background colors.
	
	GetBackColor( &savedBack );
	GetForeColor( &savedFore );
	
	// Make sure the destination window's fore/background colors
	// are black/white.
	
	RGBForeColor( &colors[kBlack] );
	RGBBackColor( &colors[kWhite] );
	
	// Lock the GWorld's pixels while we copy from it.
	
	LockPixels( GetGWorldPixMap( offscreenWorld ) );
	
	// Now copy the reversed image back to the destination window.
	
	CopyBits( &WINBITMAP( offscreenWorld ), &WINBITMAP( destWin ),
		&WINPORTRECT( offscreenWorld ), destRect, srcCopy, NULL );
		
	// Unlock the GWorld's pixels now.
	
	UnlockPixels( GetGWorldPixMap( offscreenWorld ) );
	
	// Get rid of the GWorld.
	
	DisposeGWorld( offscreenWorld );
	
	// Restore the destination window's fore/background colors.
	
	RGBForeColor( &savedFore );
	RGBBackColor( &savedBack );
	
	// Restore the saved port and device.
	
	SetGWorld( savedPort, savedDevice );
}

// FlipVertical copies the image in the window and rectangle
// specified and draws it back out reversed vertically.
//
void FlipVertical( WindowPtr destWin, Rect *destRect )
{
	CGrafPtr		savedPort;
	GDHandle		savedDevice;
	GWorldPtr		offscreenWorld;
	RGBColor		savedFore, savedBack;
	OSErr			myErr;
	short			x;
	Rect			copyFrom, copyTo;
	
	// Save the current device and port.
	
	GetGWorld( &savedPort, &savedDevice );
	
	// Create a GWorld the same size as the destination image
	// with the same depth as the destination window/rectangle.
	// The parameters are as follows:
	//		&offscreenWorld		ptr to new graphics world
	//		0					bit depth same as graphics device of dest rect
	//		destRect			bounds rectangle of my GWorld
	//		NULL				handle to a color table record - NULL means
	//							use the default record for that depth
	//		NULL				handle to a graphics device record - we aren't
	//							creating a new graphics device
	//		0L					no flags
	
		myErr = NewGWorld( &offscreenWorld, 0, destRect, NULL, NULL, 0L );
	
	// Make sure the GWorld was created
	
		if ( myErr )
		{
			DoAlertStrID( kAlertDialogID, true, kGWorldErr );
			ExitToShell();
		}
		
	// Make the GWorld the current port.
	
	SetGWorld( (CGrafPtr)offscreenWorld, NULL );
	
	// Set up the copy from and to rectangles.
	
	copyFrom = *destRect;
	copyFrom.bottom = copyFrom.top + 1;
	copyTo = WINPORTRECT( offscreenWorld );
	copyTo.top = copyTo.bottom - 1;
	
	// Copy the destination image into the GWorld row by row
	// reversing it as we go.
	
	for ( x=0; x<WINDOWHEIGHT(destWin); x++ )
	{
		CopyBits( &WINBITMAP( destWin ), &WINBITMAP( offscreenWorld ),
			&copyFrom, &copyTo, srcCopy, NULL );
		
		OffsetRect( &copyFrom, 0, 1 );
		OffsetRect( &copyTo, 0, -1 );
	}
	
	// Make the destination window the current port.
	
	SetGWorld( (CGrafPtr)destWin, NULL );
	
	// Save the destination window's fore/background colors.
	
	GetBackColor( &savedBack );
	GetForeColor( &savedFore );
	
	// Make sure the destination window's fore/background colors
	// are black/white.
	
	RGBForeColor( &colors[kBlack] );
	RGBBackColor( &colors[kWhite] );
	
	// Lock the GWorld's pixels while we copy from it.
	
	LockPixels( GetGWorldPixMap( offscreenWorld ) );
	
	// Now copy the reversed image back to the destination window.
	
	CopyBits( &WINBITMAP( offscreenWorld ), &WINBITMAP( destWin ),
		&WINPORTRECT( offscreenWorld ), destRect, srcCopy, NULL );
		
	// Unlock the GWorld's pixels now.
	
	UnlockPixels( GetGWorldPixMap( offscreenWorld ) );
	
	// Get rid of the GWorld.
	
	DisposeGWorld( offscreenWorld );
	
	// Restore the destination window's fore/background colors.
	
	RGBForeColor( &savedFore );
	RGBBackColor( &savedBack );
	
	// Restore the saved port and device.
	
	SetGWorld( savedPort, savedDevice );
}

// Slide slides the source image onto the destination from the
// direction specified in whichWay.
//
void Slide( WindowPtr srcWin, Rect *srcRect,
	WindowPtr destWin, Rect *destRect, short speed, short whichWay )
{
	RGBColor			savedFore, savedBack;
	CGrafPtr			savedPort;
	GDHandle			savedDevice;
	short				x;
	short				speedToUse, pixelsPerLoop;
	short				pixelsToMove;
	RgnHandle			savedClip;
	Rect				drawDest;
	short				hDelta, vDelta;
	short				iterations;
	long				delayPerLoop, ignore;
	
	// Make sure the speed is within acceptable range. Normally
	// zero is the slowest speed, but we're changing that to 1
	// for this routine.
	
	if ( speed <= 0 ) speedToUse = 1;
	else if ( speed > 20 ) speedToUse = 20;
	else speedToUse = speed;
	
	// Determine how much to delay each loop based on the size
	// of the destination.
	
	x = (destRect->right - destRect->left);
	if ( x < 200 ) delayPerLoop = 5;
	else if ( x < 400 ) delayPerLoop = 4;
	else if ( x < 600 ) delayPerLoop = 3;
	else if ( x < 800 ) delayPerLoop = 2;
	else if ( x < 1000 ) delayPerLoop = 1;
	else delayPerLoop = 0;
	
	// If the destination is not the same size as the source,
	// cut the delay in half to compensate for sizing the picture.
	// I could, of course just copy the source into a GWorld that
	// is the same size as the destination and the sizing would only
	// take place once, but that would also require much more
	// memory than the way I'm doing it now (just copying/sizing
	// each time).
	
	if ( ((destRect->right - destRect->left) != (srcRect->right - srcRect->left)) ||
		((destRect->bottom - destRect->top) != (srcRect->bottom - srcRect->top)) )
		delayPerLoop /= 2;
	
	// Determine how many pixels the source will have to move to
	// completely cover the destination. This depends on the
	// direction that we're moving.
	
	if ( whichWay == kLeft || whichWay == kRight )
		pixelsToMove = destRect->right - destRect->left;
	else
		pixelsToMove = destRect->bottom - destRect->top;
		
	// Determine how many pixels to move the source over onto the
	// destination each time through the loop based on the speed
	// parameter.
	
	pixelsPerLoop = speedToUse * 2;
	
	// Save the current port and set it to the destination window.
	
	GetGWorld( &savedPort, &savedDevice );
	SetGWorld( (CGrafPtr)destWin, NULL );
	
	// Save the dest window's colors and set them to black/white.
	
	GetForeColor( &savedFore );  RGBForeColor( &colors[kBlack] );
	GetBackColor( &savedBack );  RGBBackColor( &colors[kWhite] );
	
	// Save the destination window's clipping region and clip
	// to the destination rectangle.
	
	savedClip = NewRgn();
	GetClip( savedClip );
	ClipRect( destRect );
	
	// Determine where the drawing destination rectangle should
	// initially be. At the same time, set the horizontal and
	// vertical deltas for moving the rectangle.
	
	drawDest = *destRect;
	hDelta = 0;
	vDelta = 0;
	
	switch( whichWay )
	{
		case kLeft:
			OffsetRect( &drawDest, -pixelsToMove, 0 );
			hDelta = pixelsPerLoop;
			break;
		case kRight:
			OffsetRect( &drawDest, pixelsToMove, 0 );
			hDelta = -pixelsPerLoop;
			break;
		case kTop:
			OffsetRect( &drawDest, 0, -pixelsToMove );
			vDelta = pixelsPerLoop;
			break;
		case kBottom:
			OffsetRect( &drawDest, 0, pixelsToMove );
			vDelta = -pixelsPerLoop;
	}

	// Loop how many times?
	
	iterations = pixelsToMove / pixelsPerLoop;
	
	// Loop and move the source onto the image.
	
	for ( x=1; x<=iterations; x++ )
	{
		// Move the drawing destination rectangle.
		
		OffsetRect( &drawDest, hDelta, vDelta );
		
		// Now copy the source image into the drawing
		// destination rectangle we just moved.
		
		CopyBits( &WINBITMAP( srcWin ), &WINBITMAP( destWin ),
			srcRect, &drawDest, srcCopy, NULL );
		
		// Delay.
		
		if ( delayPerLoop )
			Delay( delayPerLoop, &ignore );
	}
	
	// Copy the image directly into the destination to make
	// sure the move is complete.
	
	CopyBits( &WINBITMAP( srcWin ), &WINBITMAP( destWin ),
			srcRect, destRect, srcCopy, NULL );
			
	// Restore the clipping region.
	
	SetClip( savedClip );
	DisposeRgn( savedClip );
	
	// Restore the dest window's colors.
	
	RGBForeColor( &savedFore );
	RGBBackColor( &savedBack );
	
	// Restore the saved port.
	
	SetGWorld( savedPort, savedDevice );
}

// Aperture introduces the source image onto the destination either
// from the inside of a growing circle, or the outside of a shrinking
// circle. We use CopyMask along with a circle drawn in a GWorld to
// accomplish this.
//
void Aperture( WindowPtr srcWin, Rect *srcRect,
	WindowPtr destWin, Rect *destRect, short speed, short inOrOut )
{
	short		speedToUse;
	CGrafPtr	savedPort;
	GDHandle	savedDevice;
	RGBColor	savedFore, savedBack;
	RgnHandle	savedClip;
	GWorldPtr	maskWorld;
	Rect		maskRect;
	short		sizeDelta;
	short		destSize;
	short		x;
	short		steps;
	
	// Save the current port/device.
	
	GetGWorld( &savedPort, &savedDevice );
	
	// Create a GWorld for the mask image the same size as the
	// destination rectangle.
	
	SetGWorld( (CGrafPtr)srcWin, NULL );
	
	if ( NewGWorld( &maskWorld, 0, srcRect, NULL, NULL, 0 ) != noErr )
	{
		DoAlertStrID( kAlertDialogID, true, kGWorldErr );
		ExitToShell();
	}
	else
		LockPixels( GetGWorldPixMap( maskWorld ) );

	// Set the current port to the destination window.
	
	SetGWorld( (CGrafPtr)destWin, NULL );
	
	// Save the port's colors and set them to black/white.
	
	GetForeColor( &savedFore );  RGBForeColor( &colors[kBlack] );
	GetBackColor( &savedBack );  RGBBackColor( &colors[kWhite] );
	
	// Save the port's clip region and clip to the destination rect.
	
	savedClip = NewRgn();
	GetClip( savedClip );
	ClipRect( destRect );
	
	// Make sure the speed is within the correct range.
	
	if ( speed <= 0 ) speedToUse = 1;
	else if ( speed > 20 ) speedToUse = 20;
	else speedToUse = speed;
	
	// Determine if the destination rectangle is wider than it
	// is tall or the other way around and save the bigger number.
	
	if ( (destRect->right - destRect->left) >
				(destRect->bottom - destRect->top) )
		destSize = (destRect->right - destRect->left);
	else
		destSize = (destRect->bottom - destRect->top);
	
	// Make the rectangle that the circle will start in and create
	// a delta value to determine if the circle gets bigger or
	// smaller and by how much each time through the loop.
	// Make sure it's square.
	
	maskRect = WINPORTRECT( maskWorld );
	if ( maskRect.bottom - maskRect.top > maskRect.right - maskRect.left )
	{
		x = maskRect.bottom - maskRect.top;
		InsetRect( &maskRect, -(x - (maskRect.right - maskRect.left))/2, 0 );
	}
	else
	{
		x = maskRect.right - maskRect.left;
		InsetRect( &maskRect, 0, -(x - (maskRect.bottom - maskRect.top))/2 );
	}
	
	if ( inOrOut == kIn )
	{
		sizeDelta = (destSize / ((21-speedToUse) * 5) + 1);
		InsetRect( &maskRect, -(destSize*0.1), -(destSize*0.1) );
		steps = ((destSize*1.2)/(sizeDelta*2));
	}
	else
	{
		sizeDelta = -((destSize / ((21-speedToUse) * 5) + 1));
		InsetRect( &maskRect, ((destSize/2)*0.9), ((destSize/2)*0.9) );
		steps = -((destSize*1.2)/(sizeDelta*2));
	}
	
	// Loop and copy the source image through the mask to the
	// destination making the mask circle bigger or smaller each
	// time.
	
	for( x=1; x<=steps; x++ )
	{
		// Draw the circle in the mask GWorld.
		
		SetGWorld( (CGrafPtr)maskWorld, NULL );
		EraseRect( &WINPORTRECT( maskWorld ) );
		PaintArc( &maskRect, 0, 360 );
		
		// If we're going inward, invert the mask GWorld.
		
		if ( inOrOut == kIn )
			InvertRect( &WINPORTRECT( maskWorld ) );
		
		// Copy the source through the mask to the dest.
		
		SetGWorld( (CGrafPtr)destWin, NULL );
		CopyMask( &WINBITMAP( srcWin ), &WINBITMAP( maskWorld ),
			&WINBITMAP( destWin ), srcRect, &WINPORTRECT( maskWorld ),
			destRect );
			
		// Change the size of the circle.
		
		InsetRect( &maskRect, sizeDelta, sizeDelta );
	}
	
	// Copy the source image directly to the destination to make
	// sure it ends correctly.
	
	CopyBits( &WINBITMAP( srcWin ), &WINBITMAP( destWin ),
		srcRect, destRect, srcCopy, NULL );
		
	// Restore the port's colors and clip region.
	
	SetClip( savedClip );
	RGBForeColor( &savedFore );
	RGBBackColor( &savedBack );
	
	// Restore the saved port.
	
	SetGWorld( savedPort, savedDevice );
	
	// Dispose of the GWorld we used for the mask.
	
	UnlockPixels( GetGWorldPixMap( maskWorld ) );
	DisposeGWorld( maskWorld );
}

// DoTechDemo brings up a new dialog that allows you to experiment
// with all the different ways of using CopyBits, CopyMask, and
// CopyDeepMask with different source images and masks.
//
void DoTechDemo( void )
{
	DialogPtr		demoDlg;
	short			itemHit;
	EventRecord		myEvent;
	DialogPtr		aDlg;
	Rect			sourceRect, destRect, opColorRect, maskRect;
	RgnHandle		maskRegion;
	short			copyMode;	// transfer mode
	short			ditherAdd;
	short			hiliteAdd;
	RGBColor		saveColor;
	Rect			tempRect;
	Boolean			maskSet;
	GWorldPtr		maskGWorld;	// to hold a pixel map mask
	
	// Load the main (only) dialog and display it on screen. This
	// automatically displays our two source PICTS. Since there is
	// a 'dctb' (Dialog Color Table) resource with the same ID
	// as the dialog, the Dialog Manager uses NewColorDialog to
	// make the dialog, thus giving us a color drawing port.
	
	demoDlg = GetNewDialog( kTechDemoDlgID, NULL, (WindowPtr)(-1L) );
	if ( !demoDlg )
	{
		DoAlertStrID( kAlertDialogID, true, kMemoryOut );
		ExitToShell();
	}
	ShowWindow( demoDlg );

	// Make sure the demo dialog is the current port.
	
	SetGWorld( (CGrafPtr)demoDlg, NULL );
	
	// Set controls to defaults.
	
	SetRadioButton( demoDlg, kTechDemoSourceFirst,
		kTechDemoSourceLast, kTechDemoSourceFirst ); // first source image
	SetRadioButton( demoDlg, kTechDemoMaskFirst,
		kTechDemoMaskLast, kTechDemoMaskLast );		// no mask image
	SetRadioButton( demoDlg, kTechDemoModeFirst,
		kTechDemoModeLast, kTechDemoModeFirst );	// srcCopy xfer mode
	SetOnOff( demoDlg, kTechDemoDither, false );	// no dither
	SetOnOff( demoDlg, kTechDemoHilite, false );	// no hilite
	SetRadioButton( demoDlg, kTechDemoOpColorFirst,
		kTechDemoOpColorLast, kTechDemoOpColorLast );	// black OpColor
	SetRadioButton( demoDlg, kTechDemoMaskRFirst,
		kTechDemoMaskRLast, kTechDemoMaskRLast );	// no mask region
	
	// Set default CopyBits parameters.
	
	GetItemRect( demoDlg, kTechDemoSourcePict1, &sourceRect );
	GetItemRect( demoDlg, kTechDemoDestPict, &destRect );
	GetItemRect( demoDlg, kTechDemoOpColorPict, &opColorRect );
	maskSet = false;
	copyMode = srcCopy;
	hiliteAdd = ditherAdd = 0;
	maskRegion = NULL;
	
	// Create a GWorld to hold the pixel map mask for CopyMask and CopyDeepMask.
	// Note that I do this because when I just used the image already in the
	// dialog (without copying it to its own bitmap), things did not work
	// correctly. Apparently, the mask has to be in a separate bit/pixmap from
	// the source and destination images.
	
	SetRect( &maskRect, 0, 0, 50, 50 );
	
	if ( NewGWorld( &maskGWorld, 0, &maskRect, NULL, NULL, 0 ) != noErr )
	{
		DoAlertStrID( kAlertDialogID, true, kGWorldErr );
		ExitToShell();
	}
	
	// Paint the OpColor pict black since that's the default setting.
	
	PaintRect( &opColorRect );
	
	// Erase the destination rectangle and draw rectangles around the
	// OpColor, destination, and mask region areas.
	
	EraseRect( &destRect );
	{
		tempRect = destRect;
		InsetRect( &tempRect, -1, -1 );
		FrameRect( &tempRect );
		GetItemRect( demoDlg, kTechDemoOpColorPict, &tempRect );
		InsetRect( &tempRect, -1, -1 );
		FrameRect( &tempRect );
		GetItemRect( demoDlg, kTechDemoMaskRPict1, &tempRect );
		InsetRect( &tempRect, -1, -1 );
		FrameRect( &tempRect );
		GetItemRect( demoDlg, kTechDemoMaskRPict2, &tempRect );
		InsetRect( &tempRect, -1, -1 );
		FrameRect( &tempRect );
		GetItemRect( demoDlg, kTechDemoMaskRPict3, &tempRect );
		InsetRect( &tempRect, -1, -1 );
		FrameRect( &tempRect );
	}
	
	// Draw in the mask regions in the dialog.
	
	DrawMaskRegion( demoDlg, kTechDemoMaskRPict1, 1 );
	DrawMaskRegion( demoDlg, kTechDemoMaskRPict2, 2 );
	DrawMaskRegion( demoDlg, kTechDemoMaskRPict3, 3 );
	
	// Now wait for the user to press a control in the dialog.
	
	itemHit = -1;
	while ( itemHit != kTechDemoDone )
	{
		WaitNextEvent( everyEvent, &myEvent, GetCaretTime(), NULL );
		
		// Check for disk events (bad disk mount).
		
		if ( (myEvent.what == diskEvt) &&
			(HiWord(myEvent.message) != noErr) )
		{
			Point		dlgUpLeftCorner = { 100, 80 };	// ignored in Sys 7
			
			DIBadMount( dlgUpLeftCorner, myEvent.message );	// ignore result
		}

		// Pass the event to DialogSelect which takes care of tracking
		// controls and updating everything (except the destination
		// area) for us.
		
		if ( DialogSelect( &myEvent, &aDlg, &itemHit ) )
		{
			// What we do here depends on what the user clicked
			
			if ( aDlg == demoDlg )
				switch( itemHit )
				{
					// These radio buttons change the source rectangle.
					
					case kTechDemoSourceFirst:
					case kTechDemoSourcePict1:
						SetRadioButton( demoDlg, kTechDemoSourceFirst,
							kTechDemoSourceLast, kTechDemoSourceFirst );
						GetItemRect( demoDlg, kTechDemoSourcePict1, &sourceRect );
						break;
					case kTechDemoSourceFirst+1:
					case kTechDemoSourcePict2:
						SetRadioButton( demoDlg, kTechDemoSourceFirst,
							kTechDemoSourceLast, kTechDemoSourceFirst+1 );
						GetItemRect( demoDlg, kTechDemoSourcePict2, &sourceRect );
						break;
					case kTechDemoSourceFirst+2:
					case kTechDemoSourcePict3:
						SetRadioButton( demoDlg, kTechDemoSourceFirst,
							kTechDemoSourceLast, kTechDemoSourceFirst+2 );
						GetItemRect( demoDlg, kTechDemoSourcePict3, &sourceRect );
						break;
					case kTechDemoSourceFirst+3:
					case kTechDemoSourcePict4:
						SetRadioButton( demoDlg, kTechDemoSourceFirst,
							kTechDemoSourceLast, kTechDemoSourceFirst+3 );
						GetItemRect( demoDlg, kTechDemoSourcePict4, &sourceRect );
						break;
					case kTechDemoSourceLast:
					case kTechDemoSourcePict5:
						SetRadioButton( demoDlg, kTechDemoSourceFirst,
							kTechDemoSourceLast, kTechDemoSourceLast );
						GetItemRect( demoDlg, kTechDemoSourcePict5, &sourceRect );
						break;
					
					// These radio buttons change the mask for CopyMask. Each of
					// the first three copy the mask image shown in the dialog
					// into an offscreen GWorld (maskGWorld) to be used as the mask.
					// The fourth is the setting for no mask.
					
					case kTechDemoMaskPict1:
					case kTechDemoMaskPict2:
					case kTechDemoMaskPict3:
					
						// Translate clicks on mask images themselves
						// to the radio buttons.
						if ( itemHit == kTechDemoMaskPict1 )
							itemHit = kTechDemoMaskFirst;
						else if ( itemHit == kTechDemoMaskPict2 )
							itemHit = kTechDemoMaskFirst+1;
						else
							itemHit = kTechDemoMaskFirst+2;
							
					case kTechDemoMaskFirst:
					case kTechDemoMaskFirst+1:
					case kTechDemoMaskFirst+2:
					
						// Set the proper radio button.
						SetRadioButton( demoDlg, kTechDemoMaskFirst,
							kTechDemoMaskLast, itemHit );
							
						// Get the rectangle of the selected mask image.
						GetItemRect( demoDlg,
							kTechDemoMaskPict1 + itemHit - kTechDemoMaskFirst,
							&tempRect );
						
						// Lock the mask GWorld's pixels and make it the current port.
						LockPixels( GetGWorldPixMap( maskGWorld ) );
						SetGWorld( (CGrafPtr)maskGWorld, NULL );
						
						// Copy the mask image from the dialog to the GWorld.
						CopyBits( &WINBITMAP( demoDlg ), &WINBITMAP( maskGWorld ),
							&tempRect, &maskRect, srcCopy, NULL );
						
						// Make the dialog the current port again.
						SetGWorld( (CGrafPtr)demoDlg, NULL );
						
						// Unlock the GWorld's pixels.
						UnlockPixels( GetGWorldPixMap( maskGWorld ) );
						
						// There is a mask to use.
						maskSet = true;
						
						break;

					case kTechDemoMaskLast:	// no mask
					case kTechDemoMaskNone:
					
						SetRadioButton( demoDlg, kTechDemoMaskFirst,
							kTechDemoMaskLast, kTechDemoMaskLast );
						maskSet = false;
						
						if ( maskRegion )
						{
							DisposeRgn( maskRegion );
							maskRegion = NULL;
						}
						break;
						
					// These radio buttons change the OpColor.
					
					case kTechDemoOpColorFirst:
						SetRadioButton( demoDlg, kTechDemoOpColorFirst,
							kTechDemoOpColorLast, kTechDemoOpColorFirst );
						OpColor( &colors[kWhite] );
						GetForeColor( &saveColor );
						RGBForeColor( &colors[kWhite] );
						PaintRect( &opColorRect );
						RGBForeColor( &saveColor );
						break;
					case kTechDemoOpColorFirst+1:
						SetRadioButton( demoDlg, kTechDemoOpColorFirst,
							kTechDemoOpColorLast, kTechDemoOpColorFirst+1 );
						OpColor( &colors[kLtGray] );
						GetForeColor( &saveColor );
						RGBForeColor( &colors[kLtGray] );
						PaintRect( &opColorRect );
						RGBForeColor( &saveColor );
						break;
					case kTechDemoOpColorFirst+2:
						SetRadioButton( demoDlg, kTechDemoOpColorFirst,
							kTechDemoOpColorLast, kTechDemoOpColorFirst+2 );
						OpColor( &colors[kGray] );
						GetForeColor( &saveColor );
						RGBForeColor( &colors[kGray] );
						PaintRect( &opColorRect );
						RGBForeColor( &saveColor );
						break;
					case kTechDemoOpColorFirst+3:
						SetRadioButton( demoDlg, kTechDemoOpColorFirst,
							kTechDemoOpColorLast, kTechDemoOpColorFirst+3 );
						OpColor( &colors[kDkGray] );
						GetForeColor( &saveColor );
						RGBForeColor( &colors[kDkGray] );
						PaintRect( &opColorRect );
						RGBForeColor( &saveColor );
						break;
					case kTechDemoOpColorLast:
						SetRadioButton( demoDlg, kTechDemoOpColorFirst,
							kTechDemoOpColorLast, kTechDemoOpColorLast );
						OpColor( &colors[kBlack] );
						GetForeColor( &saveColor );
						RGBForeColor( &colors[kBlack] );
						PaintRect( &opColorRect );
						RGBForeColor( &saveColor );
						break;
						
					// These radio buttons change the transfer mode.
					
					case kTechDemoModeFirst:
						SetRadioButton( demoDlg, kTechDemoModeFirst,
							kTechDemoModeLast, kTechDemoModeFirst );
						copyMode = srcCopy;
						break;
					case kTechDemoModeFirst+1:
						SetRadioButton( demoDlg, kTechDemoModeFirst,
							kTechDemoModeLast, kTechDemoModeFirst+1 );
						copyMode = srcOr;
						break;
					case kTechDemoModeFirst+2:
						SetRadioButton( demoDlg, kTechDemoModeFirst,
							kTechDemoModeLast, kTechDemoModeFirst+2 );
						copyMode = srcXor;
						break;
					case kTechDemoModeFirst+3:
						SetRadioButton( demoDlg, kTechDemoModeFirst,
							kTechDemoModeLast, kTechDemoModeFirst+3 );
						copyMode = srcBic;
						break;
					case kTechDemoModeFirst+4:
						SetRadioButton( demoDlg, kTechDemoModeFirst,
							kTechDemoModeLast, kTechDemoModeFirst+4 );
						copyMode = notSrcCopy;
						break;
					case kTechDemoModeFirst+5:
						SetRadioButton( demoDlg, kTechDemoModeFirst,
							kTechDemoModeLast, kTechDemoModeFirst+5 );
						copyMode = notSrcOr;
						break;
					case kTechDemoModeFirst+6:
						SetRadioButton( demoDlg, kTechDemoModeFirst,
							kTechDemoModeLast, kTechDemoModeFirst+6 );
						copyMode = notSrcXor;
						break;
					case kTechDemoModeFirst+7:
						SetRadioButton( demoDlg, kTechDemoModeFirst,
							kTechDemoModeLast, kTechDemoModeFirst+7 );
						copyMode = notSrcBic;
						break;
					case kTechDemoModeFirst+8:
						SetRadioButton( demoDlg, kTechDemoModeFirst,
							kTechDemoModeLast, kTechDemoModeFirst+8 );
						copyMode = blend;
						break;
					case kTechDemoModeFirst+9:
						SetRadioButton( demoDlg, kTechDemoModeFirst,
							kTechDemoModeLast, kTechDemoModeFirst+9 );
						copyMode = addPin;
						break;
					case kTechDemoModeFirst+10:
						SetRadioButton( demoDlg, kTechDemoModeFirst,
							kTechDemoModeLast, kTechDemoModeFirst+10 );
						copyMode = addOver;
						break;
					case kTechDemoModeFirst+11:
						SetRadioButton( demoDlg, kTechDemoModeFirst,
							kTechDemoModeLast, kTechDemoModeFirst+11 );
						copyMode = subPin;
						break;
					case kTechDemoModeFirst+12:
						SetRadioButton( demoDlg, kTechDemoModeFirst,
							kTechDemoModeLast, kTechDemoModeFirst+12 );
						copyMode = transparent;
						break;
					case kTechDemoModeFirst+13:
						SetRadioButton( demoDlg, kTechDemoModeFirst,
							kTechDemoModeLast, kTechDemoModeFirst+13 );
						copyMode = addMax;
						break;
					case kTechDemoModeFirst+14:
						SetRadioButton( demoDlg, kTechDemoModeFirst,
							kTechDemoModeLast, kTechDemoModeFirst+14 );
						copyMode = subOver;
						break;
					case kTechDemoModeLast:
						SetRadioButton( demoDlg, kTechDemoModeFirst,
							kTechDemoModeLast, kTechDemoModeLast );
						copyMode = adMin;
						break;
						
					// These check boxes change the transfer mode.
					
					case kTechDemoDither:
						Toggle( demoDlg, kTechDemoDither );
						if ( GetOnOff( demoDlg, kTechDemoDither ) )
							ditherAdd = ditherCopy;
						else
							ditherAdd = 0;
						break;
					case kTechDemoHilite:
						Toggle( demoDlg, kTechDemoHilite );
						if ( GetOnOff( demoDlg, kTechDemoHilite ) )
							hiliteAdd = hilite;
						else
							hiliteAdd = 0;
						break;
					
					// These buttons set the mask region.
					
					case kTechDemoMaskRPict1:
					case kTechDemoMaskRPict2:
					case kTechDemoMaskRPict3:
					
					// Translate clicks on the mask regions to the
					// actual radio buttons.
					if ( itemHit == kTechDemoMaskRPict1 )
						itemHit = kTechDemoMaskRFirst;
					else if ( itemHit == kTechDemoMaskRPict2 )
						itemHit = kTechDemoMaskRFirst+1;
					else
						itemHit = kTechDemoMaskRFirst+2;
						
					case kTechDemoMaskRFirst:
					case kTechDemoMaskRFirst+1:
					case kTechDemoMaskRFirst+2:
						SetRadioButton( demoDlg, kTechDemoMaskRFirst,
							kTechDemoMaskRLast, itemHit );
						if ( maskRegion )
							DisposeRgn( maskRegion );
						maskRegion = NewRgn();
						OpenRgn();
						if ( itemHit == kTechDemoMaskRFirst )
							DrawMaskRegion( demoDlg, kTechDemoDestPict, 1 );
						else if ( itemHit == kTechDemoMaskRFirst+1 )
							DrawMaskRegion( demoDlg, kTechDemoDestPict, 2 );
						else
							DrawMaskRegion( demoDlg, kTechDemoDestPict, 3 );
						CloseRgn( maskRegion );

						break;
						
					case kTechDemoMaskRLast:		// no mask region
					case kTechDemoMaskRNone:
						SetRadioButton( demoDlg, kTechDemoMaskRFirst,
							kTechDemoMaskRLast, kTechDemoMaskRLast );
						DisposeRgn( maskRegion );
						maskRegion = NULL;
						break;
						
					// These buttons take some action.
					
					case kTechDemoCopyBits:
						
						CopyBits( &WINBITMAP( demoDlg ),
							&WINBITMAP( demoDlg ),
							&sourceRect, &destRect,
							copyMode + ditherAdd + hiliteAdd, maskRegion );
						break;
						
					case kTechDemoCopyMask:
						
						if ( maskSet )
						{
							// Lock the mask GWorld's pixelMap.
							LockPixels( GetGWorldPixMap( maskGWorld ) );
							
							// CopyMask to the destination.
							CopyMask( &WINBITMAP( demoDlg ),
								&WINBITMAP( maskGWorld ),
								&WINBITMAP( demoDlg ),
								&sourceRect, &maskRect,
								&destRect );

							// Unlock the mask GWorld's pixel map.
							UnlockPixels( GetGWorldPixMap( maskGWorld ) );
						}
						else
						{
							CopyMask( &WINBITMAP( demoDlg ),
								NULL,
								&WINBITMAP( demoDlg ),
								&sourceRect, NULL, &destRect );
						}
						break;
					
					case kTechDemoCopyDeepMask:
					
						if ( maskSet )
						{
							// Lock the mask GWorld's pixelMap.
							LockPixels( GetGWorldPixMap( maskGWorld ) );
							
							// CopyDeepMask to the destination.
							CopyDeepMask( &WINBITMAP( demoDlg ),
								&WINBITMAP( maskGWorld ),
								&WINBITMAP( demoDlg ),
								&sourceRect, &maskRect,
								&destRect,
								copyMode + ditherAdd + hiliteAdd,
								maskRegion );

							// Unlock the mask GWorld's pixel map.
							UnlockPixels( GetGWorldPixMap( maskGWorld ) );
						}
						else
						{
							// CopyDeepMask to the destination.
							CopyDeepMask( &WINBITMAP( demoDlg ),
								NULL,
								&WINBITMAP( demoDlg ),
								&sourceRect, NULL, &destRect,
								copyMode + ditherAdd + hiliteAdd,
								maskRegion );
						}
						break;
					
					case kTechDemoErase:
					
						EraseRect( &destRect );
						break;
						
					case kTechDemoDone:
					
						// Close the demo dialog.
						DisposeDialog( demoDlg );
						
						break;
				}
		}
	}
	
	// Reset the OpColor to black
	
	OpColor( &colors[kBlack] );
	
	// Dispose of the maskGWorld.
	
	DisposeGWorld( maskGWorld );
	
	// Dispose of the mask region if one was set.
	
	if ( maskRegion )
		DisposeRgn( maskRegion );
}

// DrawMaskRegion draws the contents of each of the three mask regions.
// The drawing takes place in the rectangle of the item sent as the
// drawWhere parameter. The whichMask parameter determines what is
// drawn. When creating the actual mask region to use in the CopyBits
// operation, we have to make sure and draw the region in the coordinates
// of the destination image.
//
void DrawMaskRegion( WindowPtr win, short drawWhere, short whichMask )
{
	Rect		tempRect, tempRect2;
	short		x;
	
	GetItemRect( win, drawWhere, &tempRect );

	switch( whichMask )
	{
		case 1:		// Star
			{
				short	vMid, hMid;
				
				vMid = tempRect.top + (tempRect.bottom - tempRect.top)/2;
				hMid = tempRect.left + (tempRect.right - tempRect.left)/2;
				
				MoveTo( hMid, tempRect.top );
				LineTo( hMid+4, vMid-4 );
				LineTo( tempRect.right, vMid );
				LineTo( hMid+4, vMid+4 );
				LineTo( hMid, tempRect.bottom );
				LineTo( hMid-4, vMid+4 );
				LineTo( tempRect.left, vMid );
				LineTo( hMid-4, vMid-4 );
				LineTo( hMid, tempRect.top );
				
				break;
			}
		case 2:		// Square in middle
			InsetRect( &tempRect, 10, 10 );
			FrameRect( &tempRect );
			break;
			
		case 3:		// Concentric Squares
			tempRect2 = tempRect;
			while ( (tempRect2.right - tempRect2.left) > 2 )
			{
				FrameRect( &tempRect2 );
				InsetRect( &tempRect2, 2, 2 );
			}
			break;
	}
}

