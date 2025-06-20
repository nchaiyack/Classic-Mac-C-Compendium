/* SimpleButton.h */

#ifndef Included_SimpleButton_h
#define Included_SimpleButton_h

/* SimpleButton module depends on: */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Screen */
/* DataMunging */
/* EventLoop */

#include "Screen.h"

struct SimpleButtonRec;
typedef struct SimpleButtonRec SimpleButtonRec;

/* allocate the simple button.  Name is null terminated */
SimpleButtonRec*		NewSimpleButton(WinType* Window, char* Name,
											OrdType X, OrdType Y, OrdType Width, OrdType Height);

/* dispose of the button and internal data */
void								DisposeSimpleButton(SimpleButtonRec* TheButton);

/* indicate whether this button is a default button or not.  if it is, it will */
/* be hilighted specially (heavy outline).  It will NOT be drawn though */
void								SetDefaultButtonState(SimpleButtonRec* TheButton, MyBoolean Default);

/* find out where the button is located */
OrdType							GetSimpleButtonXLoc(SimpleButtonRec* TheButton);
OrdType							GetSimpleButtonYLoc(SimpleButtonRec* TheButton);
OrdType							GetSimpleButtonWidth(SimpleButtonRec* TheButton);
OrdType							GetSimpleButtonHeight(SimpleButtonRec* TheButton);

/* change the location of the button */
void								SetSimpleButtonLocation(SimpleButtonRec* TheButton,
											OrdType X, OrdType Y, OrdType Width, OrdType Height);

/* do a full redraw of the button */
void								RedrawSimpleButton(SimpleButtonRec* TheButton);

/* do a mouse down in the button.  if Tracking != NIL, then it will be repeatedly */
/* called with the Inside status until the mouse goes up */
MyBoolean						SimpleButtonMouseDown(SimpleButtonRec* TheButton, OrdType X, OrdType Y,
											void (*Tracking)(void* Refcon, MyBoolean Inside), void* Refcon);

/* momentarily flash the button.  this is used to provide visual feedback in case */
/* a key press performs the same function as clicking the button. */
void								FlashButton(SimpleButtonRec* TheButton);

/* find out if the specified location is within the button box */
MyBoolean						SimpleButtonHitTest(SimpleButtonRec* TheButton, OrdType X,
											OrdType Y);

#endif
