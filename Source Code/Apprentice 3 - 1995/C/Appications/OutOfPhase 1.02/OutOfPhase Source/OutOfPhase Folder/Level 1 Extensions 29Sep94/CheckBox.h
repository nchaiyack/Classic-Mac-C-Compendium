/* CheckBox.h */

#ifndef Included_CheckBox_h
#define Included_CheckBox_h

/* CheckBox module depends on: */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Screen */
/* DataMunging */
/* EventLoop */

/* CheckBox module provides a name and box with an X in it.  The X in the */
/* box reflects the state (True = X, False = empty) of the object.  The state */
/* is toggled when the box is clicked on. */

#include "Screen.h"

struct CheckBoxRec;
typedef struct CheckBoxRec CheckBoxRec;

/* create a new check box.  Name is null-terminated.  returns NIL if it failed */
CheckBoxRec*		NewCheckBox(WinType* Window, char* Name,
									OrdType X, OrdType Y, OrdType Width, OrdType Height);

/* dispose of a check box and internal data */
void						DisposeCheckBox(CheckBoxRec* TheButton);

/* find out where the box is located */
OrdType					GetCheckBoxXLoc(CheckBoxRec* TheButton);
OrdType					GetCheckBoxYLoc(CheckBoxRec* TheButton);
OrdType					GetCheckBoxWidth(CheckBoxRec* TheButton);
OrdType					GetCheckBoxHeight(CheckBoxRec* TheButton);

/* change the location of the box */
void						SetCheckBoxLocation(CheckBoxRec* TheButton,
									OrdType X, OrdType Y, OrdType Width, OrdType Height);

/* redraw the box completely */
void						RedrawCheckBox(CheckBoxRec* TheButton);

/* handle mouse downs.  returns True if the state of the box changed. */
MyBoolean				CheckBoxMouseDown(CheckBoxRec* TheButton, OrdType X, OrdType Y);

/* force the state of the box to a certain value */
void						SetCheckBoxState(CheckBoxRec* TheButton, MyBoolean TheState);

/* get the state of the box */
MyBoolean				GetCheckBoxState(CheckBoxRec* TheButton);

/* check to see if the specified location is in the box */
MyBoolean				CheckBoxHitTest(CheckBoxRec* TheButton, OrdType X, OrdType Y);

#endif
