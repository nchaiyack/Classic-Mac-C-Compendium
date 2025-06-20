/* CmdDlgOneBinaryChoice.h */

#ifndef Included_CmdDlgOneBinaryChoice_h
#define Included_CmdDlgOneBinaryChoice_h

/* CmdDlgOneBinaryChoice module depend on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Screen */
/* EventLoop */
/* Menus */
/* SimpleButton */
/* WrapTextBox */
/* DataMunging */
/* Alert */
/* RadioButton */

/* dialog box that gives the user a choice of two radio buttons.  it returns True */
/* if the user accepted the change and there was a change. */
MyBoolean				CommandDialogOneBinaryChoice(char* Prompt, char* TrueButtonName,
									char* FalseButtonName, MyBoolean* FlagInOut);

#endif
