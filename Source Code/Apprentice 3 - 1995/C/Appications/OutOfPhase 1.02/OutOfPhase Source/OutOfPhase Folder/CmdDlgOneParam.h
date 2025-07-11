/* CmdDlgOneParam.h */

#ifndef Included_CmdDlgOneParam_h
#define Included_CmdDlgOneParam_h

/* CmdDlgOneParam module depends on */
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
/* Numbers */

/* present a dialog box that allows the user to edit a single parameter */
/* returns True if the user changes the value and clicks OK. */
MyBoolean				CommandDialogOneParam(char* Prompt, char* BoxName, double* DataInOut);

#endif
