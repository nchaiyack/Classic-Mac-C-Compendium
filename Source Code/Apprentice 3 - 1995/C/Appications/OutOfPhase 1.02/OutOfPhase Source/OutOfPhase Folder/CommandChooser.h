/* CommandChooser.h */

#ifndef Included_CommandChooser_h
#define Included_CommandChooser_h

/* CommandChooser module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* NoteObject */
/* Screen */
/* Memory */
/* SimpleButton */
/* StringList */
/* Array */
/* EventLoop */

#include "NoteObject.h"

/* display a scrolling list and let the user choose a command.  if the user cancels */
/* then is returns False, otherwise True. */
MyBoolean					ChooseCommandFromList(NoteCommands* Command);

#endif
