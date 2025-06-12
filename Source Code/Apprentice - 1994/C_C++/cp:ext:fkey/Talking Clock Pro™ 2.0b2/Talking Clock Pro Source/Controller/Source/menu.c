/*
 * menu.h
 */

#include "menu.h"
#include "window.h"
#include "x.h"
#include "speech.h"
#include "util.h"


Handle gMenuBar ;


void
DoMenu ( EventRecord * event ) {

	UpdateMenus ( ) ;
	SelectMenu ( MenuSelect ( event -> where ) ) ;
}


void
SelectMenu ( long selection ) {

	DispatchMenu ( selection >> 16 , selection & 0xffff ) ;
}


void
DispatchMenu ( short menu , short item ) {

Str255 menuItem ;
MenuHandle mh ;
DefWindowRec * rec ;
WindowPtr wp ;

	if ( ! menu || ! item ) {
		return ;
	}
	HiliteMenu ( menu ) ;
#if 0
	if ( menu == EDIT_MENU ) { /* edit menu */
		if ( item <= CLEAR_ITEM ) {
			if ( SystemEdit ( item - 1 ) ) {
				HiliteMenu ( 0L ) ;
				return ;
			}
		}
	}
#endif

	if ( item ) {
		mh = GetMHandle ( menu ) ;
		if ( mh ) {
			GetItem ( mh , item , menuItem ) ;
			rec = GetFrontWindow ( & wp ) ;
			SetPort ( wp ) ;
			if ( rec && rec -> DoCommand ) {
				FailErr ( ( * ( rec -> DoCommand ) ) ( wp , rec -> data , menu ,
					item , menuItem ) ) ;
			} else {
				FailErr ( ( * app . DoCommand ) ( wp , app . data , menu , item ,
					menuItem ) ) ;
			}
		}
	}
	HiliteMenu ( 0L ) ;
}


void
UpdateMenus ( void ) {

WindowPtr wp ;
DefWindowRec * rec = GetFrontWindow ( & wp ) ;

	UpdateWinMenus ( wp , rec ) ;
}


void
UpdateWinMenus ( WindowPtr wp , DefWindowRec * rec ) {

	DisableAll ( ) ;
	if ( rec && rec -> PrepareMenus ) {
		FailErr ( ( * ( rec -> PrepareMenus ) ) ( wp , rec -> data ) ) ;
	} else {
		FailErr ( ( * app . PrepareMenus ) ( wp , app . data ) ) ;
	}
}


static void
MakeVoiceMenu ( MenuHandle mh ) {

short numVoices ;
short voiceNum ;
VoiceSpec spec ;
VoiceDescription info ;

	FailErr ( CountVoices ( & numVoices ) ) ;
	for ( voiceNum = 1 ; voiceNum <= numVoices ; voiceNum ++ ) {
		FailErr ( GetIndVoice ( voiceNum , & spec ) ) ;
		FailErr ( GetVoiceDescription ( & spec , & info , sizeof ( info ) ) ) ;
		AppendMenu ( mh , "\p\001" ) ;
		SetItem ( mh , voiceNum , info . name ) ;
	}
}


void
MakeMenus ( void ) {

Handle h ;
MenuHandle mh ;

	gMenuBar = GetResource ( 'MBAR' , 128 ) ;
	FailNil ( gMenuBar ) ;

	h = GetNewMBar ( 128 ) ;
	FailNil ( h ) ;
	SetMenuBar ( h ) ;
	DrawMenuBar ( ) ;

	mh = GetMHandle ( APPLE_MENU ) ; /* Add apple menu items */
	FailNil ( mh ) ;
	AddResMenu ( mh , 'DRVR' ) ;

	mh = GetMHandle ( FONT_MENU ) ; /* Add fonts */
	FailNil ( mh ) ;
	AddResMenu ( mh , 'FONT' ) ;

	DisableCmd ( FONT_MENU , 0 ) ;
	DisableCmd ( SIZE_MENU , 0 ) ;
	DisableCmd ( CLOCK_MENU , 0 ) ;
}


void
DisableMenu ( short menu ) {

MenuHandle mh ;
short items ;

	mh = GetMHandle ( menu ) ;
	FailNil ( mh ) ;
	items = CountMItems ( mh ) ;
	while ( items -- ) {
		DisableItem ( mh , items + 1 ) ;
		CheckItem ( mh , items + 1 , 0 ) ;
	}
}


void
EnableMenu ( short menu ) {

MenuHandle mh ;
short items ;

	mh = GetMHandle ( menu ) ;
	FailNil ( mh ) ;
	items = CountMItems ( mh ) ;
	while ( items -- ) {
		EnableItem ( mh , items + 1 ) ;
	}
}


void
DisableAll ( void ) {

short * ptr = ( short * ) * gMenuBar ;
short num = * ( ptr ++ ) ;

	HLock ( gMenuBar ) ;
	while ( num -- ) {
		if ( * ptr == APPLE_MENU ) {
			ptr ++ ;
			continue ;
		}
		DisableMenu ( * ( ptr ++ ) ) ;
	}
	HUnlock ( gMenuBar ) ;
}



void
EnableCmd ( short menu , short item ) {

MenuHandle mh ;

	mh = GetMHandle ( menu ) ;
	FailNil ( mh ) ;
	EnableItem ( mh , item ) ;
}


void
DisableCmd ( short menu , short item ) {

MenuHandle mh ;

	mh = GetMHandle ( menu ) ;
	FailNil ( mh ) ;
	DisableItem ( mh , item ) ;
}


void
CheckCmd ( short menu , short item , Boolean checked ) {

MenuHandle mh ;

	mh = GetMHandle ( menu ) ;
	FailNil ( mh ) ;
	CheckItem ( mh , item , checked ) ;
}


void
CheckStr ( short menu , unsigned char * str , Boolean checked ) {

MenuHandle mh ;
short items ;
Str255 mstr ;

	mh = GetMHandle ( menu ) ;
	FailNil ( mh ) ;
	items = CountMItems ( mh ) ;
	while ( items -- ) {
		GetItem ( mh , items + 1 , mstr ) ;
		if ( EqualString ( mstr , str , 0 , 1 ) ) {
			CheckItem ( mh , items + 1 , checked ) ;
			break ;
		}
	}
}


void
SizeMenuOutlines ( short fontNum ) {

MenuHandle mh = GetMHandle ( SIZE_MENU ) ;
short items ;
long l ;
Str15 size ;
Boolean bb ;

	FailNil ( mh ) ;
	items = CountMItems ( mh ) ;
	while ( items -- ) {
		if ( items == 0 || ( items + 1 ) >= FIRST_SIZE_ITEM ) {
			GetItem ( mh , items + 1 , size ) ;
			StringToNum ( size , & l ) ;
			if ( RealFont ( fontNum , l ) ) {
				SetItemStyle ( mh , items + 1 , outline ) ;
			} else {
				SetItemStyle ( mh , items + 1 , 0 ) ;
			}
		}
	}
	bb = RealFont ( fontNum , 219 ) ;
	SetItemStyle ( mh , SMALLER_ITEM , bb ) ;
	SetItemStyle ( mh , LARGER_ITEM , bb ) ;
}
