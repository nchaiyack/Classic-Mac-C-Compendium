/* IconButton.h */

#ifndef Included_IconButton_h
#define Included_IconButton_h

/* IconButton module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */
/* Memory */
/* EventLoop */

/* IconButton provides a flexible picture-based button.  It can behave like a */
/* checkbox (toggled) or a radio button (family), or a simple button (no state). */

#include "Screen.h"

struct IconButtonRec;
typedef struct IconButtonRec IconButtonRec;

/* these determine how the button behaves when clicked on */
typedef enum
	{
		eIconRadioMode EXECUTE(= -9),
		eIconCheckMode,
		eIconSimpleMode
	} IconButtonModes;

/* allocate a new icon button.  This one is used if the button should convert */
/* the specified raw images into internal bitmaps. */
IconButtonRec*	NewIconButtonRawBitmaps(WinType* Window, OrdType X, OrdType Y,
									OrdType Width, OrdType Height, unsigned char* RawUnselected,
									unsigned char* RawUnselectedMouseDown, unsigned char* RawSelected,
									unsigned char* RawSelectedMouseDown, int BytesPerRow,
									IconButtonModes WhatMode);

/* allocate a new icon button.  this one is used if the bitmaps are already */
/* allocated and you just want to use them.  when disposed, the bitmaps will NOT */
/* be disposed. */
IconButtonRec*	NewIconButtonPreparedBitmaps(WinType* Window, OrdType X, OrdType Y,
									OrdType Width, OrdType Height, Bitmap* Unselected,
									Bitmap* UnselectedMouseDown, Bitmap* Selected,
									Bitmap* SelectedMouseDown, IconButtonModes WhatMode);

/* dispose the button, and the bitmaps if appropriate */
void						DisposeIconButton(IconButtonRec* TheButton);

/* find out where the icon button is */
OrdType					GetIconButtonXLoc(IconButtonRec* TheButton);
OrdType					GetIconButtonYLoc(IconButtonRec* TheButton);
OrdType					GetIconButtonWidth(IconButtonRec* TheButton);
OrdType					GetIconButtonHeight(IconButtonRec* TheButton);

/* change the location of the icon button */
void						SetIconButtonLocation(IconButtonRec* TheButton, OrdType X, OrdType Y);

/* do a full redraw of the button */
void						RedrawIconButton(IconButtonRec* TheButton);

/* handle a mouse down in the button.  returns True if the state of the button */
/* changed.  (or if the mouse went up inside the button, if it is in simple */
/* button mode.)  If Tracking != NIL, then it will be repeatedly called, with */
/* the Inside status until the mouse goes up. */
MyBoolean				IconButtonMouseDown(IconButtonRec* TheButton, OrdType X, OrdType Y,
									void (*Tracking)(void* Refcon, MyBoolean Inside), void* Refcon);

/* if the button is a stateful button, this forces the state to a value */
void						SetIconButtonState(IconButtonRec* TheButton, MyBoolean TheState);

/* get the value of the state variable */
MyBoolean				GetIconButtonState(IconButtonRec* TheButton);

/* see if the specified location is within the bounds of the button */
MyBoolean				IconButtonHitTest(IconButtonRec* TheButton, OrdType X, OrdType Y);

#endif
