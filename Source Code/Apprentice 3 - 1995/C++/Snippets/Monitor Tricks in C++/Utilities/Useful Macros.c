//
// UsefulMacros.c v1.0
//
// by Kenneth Worley
// Public Domain
//
// Contact me at:   America Online: KNEworley
// 			internet: KNEworley@aol.com or kworley@fascination.com
//
// Many useful macros that do many different things! by Kenneth Worley.
// Saves you a ton of typing if you can bear using macros!
//
// Pascal string macros: src, dest, from, and onto are Str255 strings.
// MCopyString(src,dest)		Puts string in src into dest
// MAppendString(from,onto)		Appends string in from onto string in onto
//
// Memory macros: p is a pointer, h is a handle, l is a long value
// MSafeDisposePtr(p)			If p is not NULL, disposes of p and sets it to NULL
// MSafeDisposeHandle(h)		If h is not NULL, disposes of h and sets it to NULL
// MHiWord(l)					Returns low word of l as a short value
// MLoWord(l)					Returns high word of l as a short value
//
// Rectangle macros: r is a Rect (not a pointer to a Rect).
// MTopLeft(r)					Returns top left corner of the rect as a Point
// MBotRight(r)					Returns bottom right corner of the rect as a Point
// MGlobalToLocalRect(r)		Converts r from global to local coordinates
// MLocalToGlobalRect(r)		Converts r from local to global coordinates
// MRectWidth(r)				Returns width of rectangle r
// MRectHeight(r)				Returns height of rectangle r
//
// Event macros: e is a pointer to an EventRecord.
// MEventChar(e)			Returns character typed (for keyDown and autoKey events)
// MIsResumeEvent(e)		Returns true if e is a resume event
// MIsActivateEvent(e)		Returns true if e is an activate event
// MShiftDown(e)			Returns true if shift key was down when event e was made
// MCommandDown(e)			Returns true if command key was down when event e was made
// MOptionDown(e)			Returns true if option key was down when event e was made
// MControlDown(e)			Returns true if control key was down when event e was made
// MCapsLockDown(e)			Returns true if CAPS lock was down when event e was made
//
// Window macros: w is a WindowPtr (or DialogPtr).
// MWinBitMap(w)			Returns BitMap of window w (i.e. CopyBits( &MWinBitMap(...
// MWinPortRect(w)			Returns port rectangle of window w (local coord)
// MWinWidth(w)				Returns width of port rectangle of window w
// MWinHeight(w)			Returns height of port rectangle of window w
// MWinContentRect(w)		Returns content rectangle of window w (global coord)
// MWinContentRgn(w)		Returns content region handle of window w
// MWinVisibleRect(w)		Returns visible rectangle of window w (global coord)
// MWinVisibleRgn(w)		Returns visible region handle of window w
// MWinStructRect(w)		Returns structure rect of window w (global coord)
// MWinStructRgn(w)			Returns structure region handle of window w
// MWinUpdateRect(w)		Returns update rectangle of window w (global coord)
// MWinUpdateRgn(w)			Returns update region handle of window w
//
// Control macros: c is a ControlHandle.
// MControlRect(c)			Returns control's rectangle (local coord)
// MControlWidth(c)			Returns width of control's rectangle
// MControlHeight(c)		Returns height of control's rectangle
//
// Graphics Device macros: g is a GDHandle.
// MGDPixelDepth(g)			Returns pixel depth of graphics device g
// MGDColorTable(g)			Returns the color table handle (clut) of g
// MGDBoundsRect(g)			Returns the bounds rectangle of g (global coord)
// MGDBitMap(g)				Returns BitMap of g (using MWinBitMap)

// MDelay(d)				Delays for d ticks (60ths of a second)

#ifndef Useful_Macros_h
#define Useful_Macros_h

// Pascal string macros: x & y are Str255 types.
// Don't work with static strings.

#define MCopyString(src,dest)		BlockMove( src, dest, src[0]+1 )

#define MAppendString(from,onto)										\
			if ( from[0]+onto[0] <= 255 )								\
			{															\
				BlockMove( &from[1], &onto[onto[0]+1], from[0] );		\
				onto[0] += from[0];										\
			}															\
			else														\
			{															\
				BlockMove( &from[1], &onto[onto[0]+1], 255-onto[0] );	\
				onto[0] = 255;											\
			}

// Memory macros: p is a pointer, h is a handle.

#define MSafeDisposePtr(p)	if ( p )								\
							if ( GetPtrSize(p) )					\
							{										\
								DisposePtr( (Ptr)p );				\
								p = NULL;							\
							}

#define MSafeDisposeHandle(h)	if ( h )								\
								if ( GetHandleSize(h)					\
								{										\
									DisposeHandle( (Handle)h );			\
									h = NULL;							\
								}

// More memory macros: l is a long.
#define MHiWord(l)			((short)(l >> 16))
#define MLoWord(l)			((short)(l & 0xFFFF))

// Rectangle macros: r is a Rect (not a Rect*).

#define MTopLeft(r)				(* (Point*) &(r.top) )
#define MBotRight(r)			(* (Point*) &(r.bottom) )
#define MGlobalToLocalRect(r)		GlobalToLocal( &MTopLeft(r) );		\
									GlobalToLocal( &MBotRight(r) )
#define MLocalToGlobalRect(r)		LocalToGlobal( &MTopLeft(r) );		\
									LocalToGlobal( &MBotRight(r) )
#define MRectWidth(r)			(r.right - r.left)
#define MRectHeight(r)			(r.bottom - r.top)

// Event macros: e is an event record pointer.
#define MEventChar(e)			( (e)->message & charCodeMask )

#define MIsResumeEvent(e)	( ((e)->message & suspendResumeMessage) == resumeFlag )
#define MIsActivateEvent(e)	( (e)->modifiers & activeFlag )

#define MShiftDown(e)		( (e)->modifiers & shiftKey )
#define MCommandDown(e)		( (e)->modifiers & cmdKey )
#define MOptionDown(e)		( (e)->modifiers & optionKey )
#define MControlDown(e)		( (e)->modifiers & controlKey )
#define MCapsLockDown(e)	( (e)->modifiers & alphaLock )

// Window macros: w is a WindowPtr.
#define MWinBitMap(w)		((((WindowPeek)(w))->port).portBits)
#define MWinPortRect(w)		((((WindowPeek)(w))->port).portRect)
#define MWinWidth(w)		(MRectWidth(MWinPortRect(w)))
#define MWinHeight(w)		(MRectHeight(MWinPortRect(w)))
#define MWinContentRect(w)	((**((WindowPeek)(w))->contRgn).rgnBBox)
#define MWinContentRgn(w)	(((WindowPeek)(w))->contRgn)
#define MWinVisibleRect(w)	((**((WindowPeek)(w))->visRgn).rgnBBox)
#define MWinVisibleRgn(w)	(((WindowPeek)(w))->visRgn)
#define MWinStructRect(w)	((**((WindowPeek)(w))->strucRgn).rgnBBox)
#define MWinStructRgn(w)	(((WindowPeek)(w))->strucRgn)
#define MWinUpdateRect(w)	((**((WindowPeek)(w))->updateRgn).rgnBBox)
#define MWinUpdateRgn(w)	(((WindowPeek)(w))->updateRgn)

// Control macros: c is a ControlHandle
#define MControlRect(c)		((**(c)).contrlRect)
#define MControlWidth(c)	(MRectWidth(MControlRect(c)))
#define MControlHeight(c)	(MRectHeight(MControlRect(c)))

// Graphics Device macros: g is a GDHandle
#define MGDPixelDepth(g)		((**((**(g)).gdPMap)).pixelSize)
#define MGDColorTable(g)		((**((**(g)).gdPMap)).pmTable)
#define MGDBoundsRect(g)		((**((**(g)).gdPMap)).bounds)
#define MGDBitMap(g)			MWinBitMap(g)

// Delay macro: d is a long (# of ticks)
#define MDelay(d)			{ long ignore; Delay(d,&ignore); }


#endif //Useful_Macros_h
