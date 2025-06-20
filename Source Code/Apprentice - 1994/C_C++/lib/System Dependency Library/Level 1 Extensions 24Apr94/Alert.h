/* Alert.h */

#ifndef Included_Alert_h
#define Included_Alert_h

/* Alert module depends on: */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Screen */
/* DataMunging */
/* EventLoop */
/* SimpleButton */
/* WrapTextBox */
/* Menus */

/* initialize the alert system, allocate the bitmaps and temporary memory stash */
MyBoolean					InitializeAlertSubsystem(void);

/* clean up internal data structures */
void							ShutdownAlertSubsystem(void);

/* display a halt (Stopsign) alert.  ExtraInfo is inserted into Message where */
/* the _ character is */
void							AlertHalt(char* Message, char* ExtraInfo);

/* display a warning (!) alert. ExtraInfo is inserted into Message where */
/* the _ character is */
void							AlertWarning(char* Message, char* ExtraInfo);

/* display an informational (I) alert. ExtraInfo is inserted into Message where */
/* the _ character is */
void							AlertInfo(char* Message, char* ExtraInfo);

/* values that can be returned from AskYesNoCancel */
typedef enum {eYes EXECUTE(= -12343), eNo, eCancel} YesNoCancelType;

/* present a dialog with 3 buttons (yes, no, and cancel).  if Cancel is NIL, */
/* then there will only be 2 buttons.  ExtraInfo is inserted into Message where */
/* the _ character is */
YesNoCancelType		AskYesNoCancel(char* Message, char* ExtraInfo,
										char* Yes, char* No, char* Cancel);

#endif
