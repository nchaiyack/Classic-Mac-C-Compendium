/* NoteAttributeDialog.h */

#ifndef Included_NoteAttributeDialog_h
#define Included_NoteAttributeDialog_h

/* NoteAttributeDialog module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* NoteObject */
/* Screen */
/* Main */
/* Menus */
/* EventLoop */
/* SimpleButton */
/* TextEdit */
/* RadioButton */
/* Alert */
/* Numbers */
/* WrapTextBox */
/* SymbolicDuration */
/* SymbolicPitch */
/* DataMunging */
/* SymbolicIsItInThere */
/* CheckBox */
/* TrackObject */

/* forwards */
struct NoteObjectRec;
struct TrackObjectRec;

/* show dialog box allowing user to edit note's parameters */
void						EditNoteParametersDialog(struct NoteObjectRec* Note,
									struct TrackObjectRec* Track);

#endif
