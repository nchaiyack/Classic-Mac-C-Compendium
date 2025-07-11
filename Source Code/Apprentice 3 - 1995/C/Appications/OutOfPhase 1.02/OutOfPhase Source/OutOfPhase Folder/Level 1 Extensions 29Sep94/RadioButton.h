/* RadioButton.h */

#ifndef Included_RadioButton_h
#define Included_RadioButton_h

/* RadioButton module depends on: */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Screen */
/* DataMunging */
/* EventLoop */

#include "Screen.h"

struct RadioButtonRec;
typedef struct RadioButtonRec RadioButtonRec;

/* allocate a new radio button.  Name is null terminated */
RadioButtonRec*			NewRadioButton(WinType* Window, char* Name,
											OrdType X, OrdType Y, OrdType Width, OrdType Height);

/* dispose of radio button and any internal data structures. */
void								DisposeRadioButton(RadioButtonRec* TheButton);

/* find out where the radio button is located */
OrdType							GetRadioButtonXLoc(RadioButtonRec* TheButton);
OrdType							GetRadioButtonYLoc(RadioButtonRec* TheButton);
OrdType							GetRadioButtonWidth(RadioButtonRec* TheButton);
OrdType							GetRadioButtonHeight(RadioButtonRec* TheButton);

/* change the location of the radio button */
void								SetRadioButtonLocation(RadioButtonRec* TheButton,
											OrdType X, OrdType Y, OrdType Width, OrdType Height);

/* do a full redraw of the button */
void								RedrawRadioButton(RadioButtonRec* TheButton);

/* handle a mouse down in the button.  returns True if the state changed. */
MyBoolean						RadioButtonMouseDown(RadioButtonRec* TheButton, OrdType X, OrdType Y);

/* force the state of the button to be a certain value */
void								SetRadioButtonState(RadioButtonRec* TheButton, MyBoolean TheState);

/* get the state of the button */
MyBoolean						GetRadioButtonState(RadioButtonRec* TheButton);

/* see if the location is in the radio button's box */
MyBoolean						RadioButtonHitTest(RadioButtonRec* TheButton, OrdType X, OrdType Y);

#endif
