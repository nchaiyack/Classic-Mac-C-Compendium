/*
 * clwinrec.c
 */

#include <AERegistry.h>
#include <AEObjects.h>

#include <PlotIconSuite.h>
#include "TalkConstants.h"

#include "apprec.h"
#include "clwinrec.h"
#include "x.h"
#include "menu.h"
#include "window.h"
#include "util.h"
#include "str.h"

#define MAX_SIZE 500
#define MIN_SIZE 5


DefWindowRec clwinRec = {

	129 , NULL ,

	ClwinCr ,
	ClwinDe ,
	ClwinUp ,
	ClwinMD ,
	ClwinMU ,
	ClwinKD ,
	ClwinAK ,
	ClwinAc ,
	NULL ,
	ClwinId ,
	ClwinPr ,
	ClwinCo ,
	ClwinAE

} ;


typedef struct ClWinData {
	Str32		fontName ;
	short		fontSize ;
	short		baseLine ;
	short		winHeight ;
	short		winWid ;
	short		winX ;
	short		winY ;
	short		lastMin ;
	short		lastSec ;
	Boolean		showSecs ;
	Boolean		smoothUpdates ;
	Boolean		antiAlias ;
} ClWinData , * ClWinPtr ;


static short
MaxDigWid ( void ) {

short wid = 0 ;
char dig ;
short w ;

	for ( dig = '0' ; dig <= '9' ; dig ++ ) {
		w = CharWidth ( dig ) ;
		if ( w > wid ) {
			wid = w ;
		}
	}
	return wid ;
}


static void
ClWindowCalc ( ClWinPtr ptr , WindowPtr wp ) {

Str32 name ;
FontInfo fi ;
short num ;
Point p = { 0 , 0 } ;

	SetPort ( wp ) ;
	GetFNum ( ptr -> fontName , & num ) ;
	TextFont ( num ) ;
	TextSize ( ptr -> fontSize ) ;
	GetFontInfo ( & fi ) ;
	ptr -> baseLine = fi . ascent + fi . leading ;
	ptr -> winHeight = ptr -> baseLine + fi . descent + fi . leading ;
	ptr -> winWid = MaxDigWid ( ) * ( ptr -> showSecs ? 9 : 7 ) ;
	LocalToGlobal ( & p ) ;
	ptr -> winX = p . h ;
	ptr -> winY = p . v ;
	if ( ptr -> smoothUpdates ) {
		if ( ! OffscreenAvailable ( ) ) {
			ptr -> smoothUpdates = 0 ;
		}
	}
	SizeWindow ( wp , ptr -> winWid , ptr -> winHeight , 1 ) ;
	InvalRect ( & ( wp -> portRect ) ) ;
}


/*	A window is being created or opened. Allocate data and		*/
/*	select the window									*/
OSErr
ClwinCr ( WindowPtr wp , Handle * data , FSSpec * file ) {

ClWinPtr ptr ;
FontInfo fi ;

	ClWinAdd ( wp , app . data ) ;
	* data = NewHandleClear ( sizeof ( ClWinData ) ) ;
	HLockHi ( * data ) ;
	ptr = ( ClWinPtr ) * * data ;
	GetFontName ( GetAppFont ( ) , ptr -> fontName ) ;
	ptr -> showSecs = 0 ;
	ptr -> fontSize = GetDefFontSize ( ) ;
	ClWindowCalc ( ptr , wp ) ;
	SelectWindow ( wp ) ;
	ShowWindow ( wp ) ;
	return noErr ;
}


/*	Window is being destroyed. Put up a warning dialog, and		*/
/*	return errCancel if the user cancels closing - else call	*/
/*	DisposeWindow here											*/
OSErr
ClwinDe ( WindowPtr wp , Handle data ) {

	ClWinRemove ( wp , app . data ) ;
	DisposeHandle ( data ) ;
	DisposeWindow ( wp ) ;
	return noErr ;
}


/*	Update the window - BeginUpdate is already called			*/
OSErr
ClwinUp ( WindowPtr wp , Handle data , EventRecord * event ) {

Str63 time ;
unsigned long now ;
ClWinPtr ptr = ( ClWinPtr ) * data ;
short pos ;
short num ;
Boolean doSmooth = ptr -> smoothUpdates ;
GWorldPtr lptr = NULL , sptr = NULL ;
PixMapHandle lpix = NULL , spix = NULL ;
Rect r , r2 ;
short err ;
Point p = { 0 , 0 } ;
short mul = 1 ;

	GetDateTime ( & now ) ;
	IUTimeString ( now , ptr -> showSecs , time ) ;
	GetFNum ( ptr -> fontName , & num ) ;
	TextFont ( num ) ;
	TextSize ( ptr -> fontSize ) ;
	pos = ( ptr -> winWid - StringWidth ( time ) ) / 2 ;

	if ( doSmooth ) {
		r = wp -> portRect ;
		LocalToGlobal ( & p ) ;
		OffsetRect ( & r , p . h , p . v ) ;
		err = NewGWorld ( & sptr , 0 , & r , NULL , NULL , useTempMem ) ;
		OffsetRect ( & r , - p . h , - p . v ) ;
	} else {
		err = 1 ;
	}
	if ( ptr -> antiAlias && ! err ) {
		r2 = wp -> portRect ;
		if ( ptr -> fontSize > 24 ) {
			r2 . right <<= 1 ;
			r2 . bottom <<= 1 ;
			mul = 2 ;
		} else {
			r2 . right <<= 2 ;
			r2 . bottom <<= 2 ;
			mul = 4 ;
		}
		err = NewGWorld ( & lptr , 0 , & r2 , NULL , NULL , useTempMem ) ;
	}
	if ( sptr && lptr && ! err ) {
	GWorldPtr oldWorld ;
	GDHandle oldGD ;
		LockPixels ( GetGWorldPixMap ( lptr ) ) ;
		GetGWorld ( & oldWorld , & oldGD ) ;
		SetGWorld ( lptr , NULL ) ;
		EraseRect ( & r2 ) ;
		TextFont ( num ) ;
		TextSize ( ptr -> fontSize * mul ) ;
		MoveTo ( pos * mul , ptr -> baseLine * mul ) ;
		DrawString ( time ) ;
		LockPixels ( GetGWorldPixMap ( sptr ) ) ;
		SetGWorld ( sptr , NULL ) ;
		CopyBits ( ( BitMapPtr ) & ( ( ( GrafPtr ) lptr ) -> portBits ) ,
			( BitMapPtr ) & ( ( ( GrafPtr ) sptr ) -> portBits ) , & r2 , & r ,
			ditherCopy , NULL ) ;
		UnlockPixels ( GetGWorldPixMap ( lptr ) ) ;
		SetGWorld ( oldWorld , oldGD ) ;
	} else if ( sptr && ! err ) {
	GWorldPtr oldWorld ;
	GDHandle oldGD ;
		LockPixels ( GetGWorldPixMap ( sptr ) ) ;
		GetGWorld ( & oldWorld , & oldGD ) ;
		SetGWorld ( sptr , NULL ) ;
		EraseRect ( & r ) ;
		TextFont ( num ) ;
		TextSize ( ptr -> fontSize ) ;
		MoveTo ( pos + r . left , ptr -> baseLine + r . top ) ;
		DrawString ( time ) ;
		SetGWorld ( oldWorld , oldGD ) ;
	} else {
		err = err ? err : 1 ;
	}
	doSmooth = ! err ;

/* Don't erase until we know what to do! */
	if ( ! doSmooth ) {
		EraseRect ( & ( wp -> portRect ) ) ;
		MoveTo ( pos , ptr -> baseLine ) ;
		DrawString ( time ) ;
	} else {
		CopyBits ( ( BitMapPtr ) & ( ( ( GrafPtr ) sptr ) -> portBits ) ,
			& ( wp -> portBits ) , & r , & ( wp -> portRect ) , srcCopy , NULL ) ;
	}
	if ( lptr ) {
		DisposeGWorld ( lptr ) ;
	}
	if ( sptr ) {
		DisposeGWorld ( sptr ) ;
	}

	return noErr ;
}


/*	The user clicked in your window. The window port is set and	*/
/*	event -> where is translated into local coordinates now		*/
OSErr
ClwinMD ( WindowPtr wp , Handle data , EventRecord * event ) {

/* The framework has converted the point to local coordinates; convert back */
	LocalToGlobal ( & ( event -> where ) ) ;
	DragWindow ( wp , event -> where , & gDragLimit ) ;
/* Save the new position */
	ClWindowCalc ( ( ClWinPtr ) * data , wp ) ;
	return noErr ;
}


/*	MouseUp in a window - you probably should do nothing		*/
OSErr
ClwinMU ( WindowPtr wp , Handle data , EventRecord * event ) {

	return noErr ;
}


/*	The user typed in the window - command keys are already		*/
/*	taken care of and don't come here							*/
OSErr
ClwinKD ( WindowPtr wp , Handle data , EventRecord * event ) {

	return noErr ;
}


/*	You probably just want to call the KeyDown handler here		*/
OSErr
ClwinAK ( WindowPtr wp , Handle data , EventRecord * event ) {

	return noErr ;
}


/*	Activate event - highlight/dehighlight controls & caret		*/
OSErr
ClwinAc ( WindowPtr wp , Handle data , EventRecord * event ) {

	if ( event -> modifiers & activeFlag ) {
		EnableCmd ( FONT_MENU , 0 ) ;
		EnableCmd ( SIZE_MENU , 0 ) ;
		EnableCmd ( CLOCK_MENU , 0 ) ;
	} else {
		DisableCmd ( FONT_MENU , 0 ) ;
		DisableCmd ( SIZE_MENU , 0 ) ;
		DisableCmd ( CLOCK_MENU , 0 ) ;
	}
	return noErr ;
}


/*	You should probably build a suitable activate event and		*/
/*	send to the activate handling for MultiFinder switches		*/
/*	In this application, we set this record field to NULL to		*/
/*	pass the event to the application instead.				*/
OSErr
ClwinSw ( WindowPtr wp , Handle data , EventRecord * event ) {

	return noErr ;
}


/*	Idle time - set the sleep parameter if it's too large for	*/
/*	your needs. GetCaretTime() is a good value for text editors	*/
OSErr
ClwinId ( WindowPtr wp , Handle data , long * sleep ) {

	return AppId ( wp , app . data , sleep ) ;
}


/*	This is called when menus need to be updated.				*/
OSErr
ClwinPr ( WindowPtr wp , Handle data ) {

Str15 num ;
ClWinPtr ptr = ( ClWinPtr ) * data ;
short fNum ;

	FailErr ( AppPr ( wp , app . data ) ) ;
	EnableCmd ( FILE_MENU , CLOSE_ITEM ) ;
	SetPort ( wp ) ;
	GetFNum ( ptr -> fontName , & fNum ) ;
	EnableMenu ( FONT_MENU ) ;
	EnableMenu ( SIZE_MENU ) ;
	SizeMenuOutlines ( fNum ) ;
	DisableCmd ( SIZE_MENU , SIZE_DELIMITER ) ;
	CheckStr ( FONT_MENU , ptr -> fontName , 1 ) ;
	NumToString ( ptr -> fontSize , num ) ;
	CheckStr ( SIZE_MENU , num , 1 ) ;
	EnableCmd ( CLOCK_MENU , SECONDS_ITEM ) ;
	CheckCmd ( CLOCK_MENU , SECONDS_ITEM , ptr -> showSecs ) ;
	if ( OffscreenAvailable ( ) ) {
		EnableCmd ( CLOCK_MENU , SMOOTH_ITEM ) ;
		CheckCmd ( CLOCK_MENU , SMOOTH_ITEM , ptr -> smoothUpdates ) ;
		if ( ptr -> smoothUpdates ) {
			EnableCmd ( CLOCK_MENU , ANTI_ALIAS_ITEM ) ;
			CheckCmd ( CLOCK_MENU , ANTI_ALIAS_ITEM , ptr -> antiAlias ) ;
		}
	}
	return noErr ;
}


/*	A menu selection was made - take appropriate action			*/
OSErr
ClwinCo ( WindowPtr wp , Handle data , short menu , short item ,
	unsigned char * itemStr ) {

ClWinPtr ptr = ( ClWinPtr ) * data ;
long size ;

	if ( menu == FILE_MENU && item == CLOSE_ITEM ) {
		DestroyWindow ( wp ) ;
	} else if ( menu == FONT_MENU ) {
		CopyPString ( itemStr , ptr -> fontName ) ;
		ClWindowCalc ( ptr , wp ) ;
	} else if ( menu == SIZE_MENU ) {
		switch ( item ) {
		case SMALLER_ITEM :
			if ( ptr -> fontSize > MIN_SIZE ) {
				if ( KeyIsDown ( 0x3a ) ) {
					ptr -> fontSize >>= 1 ;
					if ( ptr -> fontSize < MIN_SIZE ) {
						ptr -> fontSize = MIN_SIZE ;
					}
				} else {
					ptr -> fontSize -- ;
				}
			}
			break ;
		case LARGER_ITEM :
			if ( ptr -> fontSize < MAX_SIZE ) {
				if ( KeyIsDown ( 0x3a ) ) {
					ptr -> fontSize <<= 1 ;
					if ( ptr -> fontSize > MAX_SIZE ) {
						ptr -> fontSize = MAX_SIZE ;
					}
				} else {
					ptr -> fontSize ++ ;
				}
			}
			break ;
		default :
			StringToNum ( itemStr , & size ) ;
			ptr -> fontSize = size ;
			break ;
		}
		ClWindowCalc ( ptr , wp ) ;
	} else if ( menu == CLOCK_MENU ) {
		switch ( item ) {
		case SECONDS_ITEM :
			ptr -> showSecs = ! ptr -> showSecs ;
			break ;
		case SMOOTH_ITEM :
			ptr -> smoothUpdates = ! ptr -> smoothUpdates ;
			break ;
		case ANTI_ALIAS_ITEM :
			ptr -> antiAlias = ! ptr -> antiAlias ;
			break ;
		}
		ClWindowCalc ( ptr , wp ) ;
	} else {
		FailErr ( AppCo ( wp , app . data , menu , item , itemStr ) ) ;
	}

	return noErr ;
}


/*	An AppleEvent was received with this window in front		*/
OSErr
ClwinAE ( WindowPtr wp , Handle data , AppleEvent * event ,
	AppleEvent * reply ) {

	return noErr ;
}


void
ClWinWrite ( short refNum , WindowPtr wp ) {

DefWindowRec * dr ;
long size ;
ClWinData data ;

	if ( ! wp ) {
		return ;
	}
	dr = RecFromWindow ( wp ) ;
	if ( ! dr ) {
		return ;
	}
	data = * ( ClWinPtr ) * ( dr -> data ) ;
	size = sizeof ( ClWinData ) ;
	FailErr ( FSWrite ( refNum , & size , ( Ptr ) & data ) ) ;
}


void
ClWinRead ( short refNum , WindowPtr wp ) {

DefWindowRec * dr ;
long size ;
ClWinData data ;
Point p ;

	size = sizeof ( ClWinData ) ;
	FailErr ( FSRead ( refNum , & size , ( Ptr ) & data ) ) ;

	if ( ! wp ) {
		return ;
	}
	dr = RecFromWindow ( wp ) ;
	if ( ! dr ) {
		return ;
	}
	* ( ClWinPtr ) * ( dr -> data ) = data ;
	p . h = data . winX + 10 ;
	p . v = data . winY + 5 ;
	if ( PtInRgn ( p , GetGrayRgn ( ) ) ) {
		MoveWindow ( wp , data . winX , data . winY , 1 ) ;
	}
	ClWindowCalc ( ( ClWinPtr ) * ( dr -> data ) , wp ) ;
}


void
ClWinRunClock ( WindowPtr wp , long * sleep , short nowMin , short nowSec ) {

long canSleep ;
DefWindowRec * dr ;
ClWinPtr ptr ;

	if ( ! wp ) {
		return ;
	}
	dr = RecFromWindow ( wp ) ;
	if ( ! dr ) {
		return ;
	}
	ptr = ( ClWinPtr ) * ( dr -> data ) ;

	canSleep = ptr -> showSecs ? 10 : 180 ;
	if ( * sleep > canSleep ) {
		* sleep = canSleep ;
	}
	if ( ( nowMin != ptr -> lastMin ) ||
		( ptr -> showSecs && ( nowSec != ptr -> lastSec ) ) ) {
		SetPort ( wp ) ;
		ptr -> lastMin = nowMin ;
		ptr -> lastSec = nowSec ;
		ClwinUp ( wp , dr -> data , NULL ) ;
	}
}
