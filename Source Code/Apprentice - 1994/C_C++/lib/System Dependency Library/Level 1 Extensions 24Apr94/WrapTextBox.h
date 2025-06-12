/* WrapTextBox.h */

#ifndef Included_WrapTextBox_h
#define Included_WrapTextBox_h

/* WrapTextBox module depends on: */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Screen */
/* DataMunging */

#include "Screen.h"

/* draw a string but wordwrap it in the specified box.  The string is null terminated */
void			DrawWrappedTextBox(WinType* Window, char* Text, FontType FontID,
						FontSizeType FontSize, OrdType X, OrdType Y, OrdType Width, OrdType Height);

#endif
