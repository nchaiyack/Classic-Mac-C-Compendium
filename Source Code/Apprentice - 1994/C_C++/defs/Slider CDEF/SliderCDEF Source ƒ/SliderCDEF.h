#include <Start.h>
#include <QDOffscreen.h>
#include <Retrace.h>

// These values appear in the contrlValue field of a button control's record. (?)
#define kInvisible	0
#define kVisible	255
#define kInactive	255

// Width of the slider's indicator in pixels.
#define kThumbSize	40

#define kVBLCount   1

pascal long GetVBLRec(void)
    = 0x2E88;

typedef struct VBLRec {
    VBLTask	xVBLTask;
    Boolean		xDoTask;
    long		vblA5;      // 4 bytes before the VBLTask data
} VBLRec, *VBLRecPtr;


// This structure will be allocated during initialization, and
// a handle to it will be stored in the contrlData field of the
// control record.
typedef struct SliderDataRec {
	GWorldPtr		sliderPort;
	Rect			sliderPortRect;
	Rect			sliderTrackRect;
	Rect			sliderThumbRect;
	Rect			sliderRect;
	GWorldPtr		currPort;
	GDHandle		currDev;
} SliderDataRec, *SliderDataPtr, **SliderDataHnd;


// These colors will be used a lot, so they're put together in a structure.
typedef struct SliderColors {
	RGBColor	black;
	RGBColor	white;
	RGBColor	darkGray;
	RGBColor	lightGray;
	RGBColor	darkColor;
	RGBColor	lightColor;
} SliderColors, *SliderColorsPtr, **SliderColorsHnd;	


// When the thumbCntl message is received, param contains a
// pointer to this structure.
typedef struct ThumbCntlRec {
	Rect		limitRect;
	Rect		slopRect;
	short	axis;
} ThumbCntlRec, *ThumbCntlRecPtr, **ThumbCntlRecHnd;


// SliderCDEF.c
pascal long SliderCDEF( short varCode, ControlHandle theControl, short message, long param );

// SliderDrawControl.c
void SliderDrawControl( short varCode, ControlHandle theControl, long param );
void SliderDrawControlTrack( ControlHandle theControl, SliderColors *colors );
void SliderDrawControlThumb( ControlHandle theControl, SliderColors *colors );
void SliderDrawControlValue( ControlHandle theControl, SliderColors *colors );
void SliderDrawControlInactive( short varCode, ControlHandle theControl, long param );
void SliderBlitControl( ControlHandle theControl, SliderColors *colors );

// SliderTestControl.c
long SliderTestControl( short varCode, ControlHandle theControl, long param );

// SliderCalcCRgns.c
void SliderCalcCRgns( short varCode, ControlHandle theControl, long param );

// SliderInitControl.c
void SliderInitControl( short varCode, ControlHandle theControl, long param );
void SliderCreateOffscreenWorld( ControlHandle theControl );
void SliderDrawControlParts( ControlHandle theControl );

// SliderDisposeControl.c
void SliderDisposeControl( short varCode, ControlHandle theControl, long param );

// SliderPositionControl.c
void SliderPositionControl( short varCode, ControlHandle theControl, long param );

// SliderThumbControl.c
void SliderThumbControl( short varCode, ControlHandle theControl, long param );

// SliderDragControl.c
long SliderDragControl( short varCode, ControlHandle theControl, long param );
OSErr VBLTaskSetup( void );
OSErr RemoveVBLTask( void );
void MyVBLTask( void );

// SliderCalcCntlRgn.c
void SliderCalcCntlRgn( short varCode, ControlHandle theControl, long param );

// SliderCalcThumbRgn.c
void SliderCalcThumbRgn( short varCode, ControlHandle theControl, long param );

// SliderUtils.c
Rect CalcThumbRect( ControlHandle theControl );
void SliderGetControlColors( ControlHandle theControl, SliderColors *colors );
Boolean IsHorizontal( ControlHandle theControl );