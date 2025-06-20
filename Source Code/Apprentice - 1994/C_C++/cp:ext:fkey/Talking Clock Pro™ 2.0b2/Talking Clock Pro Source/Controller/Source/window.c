/*
 * window.c
 */

#include "window.h"
#include "x.h"
#include "menu.h"
#include "str.h"


#define MY_KIND 4711


static Boolean inFront = 1 ;
Boolean quit = 0 ;
long gSleep = 0x7fffffff ;
RgnHandle gMouse = NULL ;
EventRecord gEvent ;
Rect gDragLimit ;
Rect gSizeLimit ;
DefWindowRec app ;
static WindowPtr downWindow = NULL ;
short notificationLevel = 2 ;
Boolean notificationSound = 1 ;


WindowPtr
MakeWindow ( FSSpec * file , DefWindowRec * rec ) {

WindowPtr wp ;
Ptr h ;

	wp = GetNewWindow ( rec -> windID , NULL , ( WindowPtr ) -1 ) ;
	FailNil ( wp ) ;
	h = NewPtrClear ( sizeof ( DefWindowRec ) ) ;
	FailNil ( h ) ;
	* ( DefWindowRec * ) h = * rec ;
	SetWRefCon ( wp , ( long ) h ) ;
	( ( WindowPeek ) wp ) -> windowKind = MY_KIND ;
	TRY {
		if ( rec -> OnCreate ) {
			FailErr ( ( * ( rec -> OnCreate ) ) ( wp ,
				& ( ( ( DefWindowRec * ) h ) -> data ) , file ) ) ;
		} else {
			SelectWindow ( wp ) ;
		}
	} CATCH {
		DisposeWindow ( wp ) ;
		DisposePtr ( h ) ;
	} DONE ;

	return wp ;
}


OSErr
DestroyWindow ( WindowPtr wp ) {

short err = 0 ;

	if ( wp == downWindow ) {
		downWindow = NULL ;
	}
	if ( ( ( WindowPeek ) wp ) -> windowKind == MY_KIND ) {
	DefWindowRec * rec = ( DefWindowRec * ) GetWRefCon ( wp ) ;
		if ( rec -> OnDestroy ) {
			err = ( * ( rec -> OnDestroy ) ) ( wp , rec -> data ) ;
		} else {
			DisposeWindow ( wp ) ;
		}
		if ( ! err ) {
			DisposePtr ( ( Ptr ) rec ) ;
		}
	}
	return err ;
}


DefWindowRec *
GetFrontWindow ( WindowPtr * wp ) {

	* wp = FrontWindow ( ) ;
	return RecFromWindow ( * wp ) ;
}


DefWindowRec *
RecFromWindow ( WindowPtr wp ) {
	if ( ! wp ) {
		return NULL ;
	}
	if ( ( ( WindowPeek ) wp ) -> windowKind != MY_KIND ) {
		return NULL ;
	}
	return ( DefWindowRec * ) GetWRefCon ( wp ) ;
}


void
IdleFrontWindow ( void ) {
WindowPtr wp = NULL ;
DefWindowRec * rec = GetFrontWindow ( & wp ) ;
	if ( rec && rec -> DoIdle ) {
		FailNil ( wp ) ;
		SetPort ( wp ) ;
		FailErr ( ( * ( rec -> DoIdle ) ) ( wp , rec -> data , & gSleep ) ) ;
	} else {
		FailErr ( ( * app . DoIdle ) ( wp , app . data , & gSleep ) ) ;
	}
}


static void
MouseDown ( EventRecord * event ) {

WindowPtr wp = NULL ;
short code ;
DefWindowRec * rec = NULL ;

	code = FindWindow ( event -> where , & wp ) ;
	rec = RecFromWindow ( wp ) ;
	downWindow = wp ;

	switch ( code ) {
	case inContent :
		if ( rec -> DoMouseDown ) {
			SetPort ( wp ) ;
			GlobalToLocal ( & ( event -> where ) ) ;
			FailErr ( ( * ( rec -> DoMouseDown ) ) ( wp , rec -> data , event ) ) ;
		} else {
			FailErr ( ( * app . DoMouseDown ) ( wp , app . data , event ) ) ;
		}
		break ;
	case inSysWindow :
		SystemClick ( event , wp ) ;
		break ;
	case inGoAway :
		if ( TrackGoAway ( wp , event -> where ) ) {
			FailErr ( DestroyWindow ( wp ) ) ;
		}
		break ;
	case inDrag :
		DragWindow ( wp , event -> where , & gDragLimit ) ;
		break ;
	case inGrow :
		{	long res = GrowWindow ( wp , event -> where , & gSizeLimit ) ;
			if ( res ) {
				SizeWindow ( wp , res & 0xffff , ( res >> 16 ) & 0xffff , 1 ) ;
			}
		}
		break ;
	case inMenuBar :
		DoMenu ( event ) ;
		break ;
	case inZoomIn :
	case inZoomOut :
	case inDesk :
	default :
		break ;
	}
}


static void
MouseUp ( EventRecord * event ) {

WindowPtr wp = NULL ;
short code ;
DefWindowRec * rec = NULL ;

	code = FindWindow ( event -> where , & wp ) ;
	rec = RecFromWindow ( wp ) ;

	if ( downWindow && ( downWindow == wp ) ) {
	WindowPtr frontWin ;
	( void ) GetFrontWindow ( & frontWin ) ;
		if ( frontWin != wp ) {
			SelectWindow ( wp ) ;
		}
	}
	downWindow = NULL ;

	switch ( code ) {
	case inContent :
		if ( rec -> DoMouseUp ) {
			SetPort ( wp ) ;
			GlobalToLocal ( & ( event -> where ) ) ;
			FailErr ( ( * ( rec -> DoMouseUp ) ) ( wp , rec -> data , event ) ) ;
			break ;
		} else {
			FailErr ( ( * app . DoMouseUp ) ( wp , rec -> data , event ) ) ;
		}
		break ;
	case inSysWindow :
	case inGoAway :
	case inDrag :
	case inGrow :
	case inMenuBar :
	case inZoomIn :
	case inZoomOut :
	case inDesk :
	default :
		break ;
	}
}


static void
WinKey ( EventRecord * event , WindowPtr wp , DefWindowRec * rec ) {

	if ( wp ) {
		SetPort ( wp ) ;
	}
	if ( rec && rec -> DoKeyDown ) {
		FailErr ( ( * ( rec -> DoKeyDown ) ) ( wp , rec -> data , event ) ) ;
	} else {
		FailErr ( ( * app . DoKeyDown ) ( wp , app . data , event ) ) ;
	}
}


static void
KeyDown ( EventRecord * event ) {

WindowPtr wp = NULL ;
DefWindowRec * rec = GetFrontWindow ( & wp ) ;
long sel ;

	if ( event -> modifiers & cmdKey ) {
		UpdateWinMenus ( wp , rec ) ;
		sel = MenuKey ( event -> message & 0xff ) ;
		if ( sel ) {
			SelectMenu ( sel ) ;
			return ;
		}
	}
	WinKey ( event , wp , rec ) ;
}


static void
AutoKey ( EventRecord * event ) {

WindowPtr wp = NULL ;
DefWindowRec * rec = GetFrontWindow ( & wp ) ;

	WinKey ( event , wp , rec ) ;
}


static void
ActivateEvt ( EventRecord * event ) {

WindowPtr wp = ( WindowPtr ) event -> message ;
DefWindowRec * rec = RecFromWindow ( wp ) ;

	SetPort ( wp ) ;
	if ( rec && rec -> DoActivate ) {
		FailErr ( ( * ( rec -> DoActivate ) ) ( wp , rec -> data , event ) ) ;
	} else {
		FailErr ( ( * app . DoActivate ) ( wp , app . data , event ) ) ;
	}
}


static void
UpdateEvt ( EventRecord * event ) {

WindowPtr wp = ( WindowPtr ) event -> message ;
DefWindowRec * rec = RecFromWindow ( wp ) ;

	BeginUpdate ( wp ) ;
	SetPort ( wp ) ;
	if ( rec && rec -> DoUpdate ) {
		FailErr ( ( * ( rec -> DoUpdate ) ) ( wp , rec -> data , event ) ) ;
	} else {
		FailErr ( ( * app . DoUpdate ) ( wp , app . data , event ) ) ;
	}
	EndUpdate ( wp ) ;
}


static void
DiskEvt ( EventRecord * event ) {

	if ( event -> message & 0xffff0000 ) {
	Point p = { 120 , 100 } ;
		FailErr ( DIBadMount ( p , event -> message ) ) ;
	}
}


static void
OSEvt ( EventRecord * event ) {

WindowPtr wp = NULL ;
DefWindowRec * rec = GetFrontWindow ( & wp ) ;

	if ( rec && rec -> DoSwitch ) {
		FailErr ( ( * ( rec -> DoSwitch ) ) ( wp , rec -> data , event ) ) ;
	} else {
		FailErr ( ( * app . DoSwitch ) ( wp , rec -> data , event ) ) ;
	}
	if ( event -> message & 0xff000000 == 0x01000000 ) { /* Suspend/Resume */
		if ( event -> message & 1 ) {
			inFront = 1 ;
		} else {
			inFront = 0 ;
		}
	}
}


static void
HighLevelEvent ( EventRecord * event ) {

short err ;

	err = AEProcessAppleEvent ( event ) ;
}


void
HandleEvent ( EventRecord * event ) {

	switch ( event -> what ) {

	case mouseDown :
		MouseDown ( event ) ;
		break ;
	case mouseUp :
		MouseUp ( event ) ;
		break ;
	case keyDown :
		KeyDown ( event ) ;
		break ;
	case autoKey :
		AutoKey ( event ) ;
		break ;
	case activateEvt :
		ActivateEvt ( event ) ;
		break ;
	case updateEvt :
		UpdateEvt ( event ) ;
		break ;
	case diskEvt :
		DiskEvt ( event ) ;
		break ;
	case osEvt :
		OSEvt ( event ) ;
		break ;
	case kHighLevelEvent :
		HighLevelEvent ( event ) ;
		break ;
	default :
		IdleFrontWindow ( ) ;
		break ;
	}
}


void
DriveWindow ( void ) {

	if ( WaitNextEvent ( -1 , & gEvent , gSleep , gMouse ) ) {
		gSleep = 0 ;
		HandleEvent ( & gEvent ) ;
	} else {
		gSleep = 0x7fffffff ;
		IdleFrontWindow ( ) ;
	}
}


Boolean
InFront ( void ) {
	return inFront ;
}


static NMRec theRec ;

void
Notify ( void ) {
	ClearMem ( & theRec , sizeof ( theRec ) ) ;
}


void
DeNotify ( void ) {
}

