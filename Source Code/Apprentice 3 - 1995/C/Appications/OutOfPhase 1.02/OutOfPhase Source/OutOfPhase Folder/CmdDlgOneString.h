/* CmdDlgOneString.h */

#ifndef Included_CmdDlgOneString_h
#define Included_CmdDlgOneString_h

/* CmdDlgOneString module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Screen */
/* EventLoop */
/* Menus */
/* TextEdit */
/* SimpleButton */
/* WrapTextBox */
/* DataMunging */
/* Main */
/* Alert */

/* present a dialog box that allows the user to edit a single string parameter */
/* returns True if the user changes the value and clicks OK. */
MyBoolean				CommandDialogOneString(char* Prompt, char* BoxName, char** DataInOut);

#endif
