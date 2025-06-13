/* CmdDlgTwoParams.h */

#ifndef Included_CmdDlgTwoParams_h
#define Included_CmdDlgTwoParams_h

/* CmdDlgTwoParams module depends on */
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

/* present a dialog box that allows the user to edit two parameters */
/* returns True if the user changes the value and clicks OK. */
MyBoolean				CommandDialogTwoParams(char* Prompt, char* FirstBoxName,
									double* FirstDataInOut, char* SecondBoxName, double* SecondDataInOut);

#endif
