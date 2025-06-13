/* EditCommandParameters.h */

#ifndef Included_EditCommandParameters_h
#define Included_EditCommandParameters_h

/* EditCommandParameters module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* NoteObject */
/* Memory */
/* NoteAttributeDialog */
/* BinaryCodedDecimal */
/* TrackObject */
/* DataMunging */
/* CmdDlgOneParam */
/* CmdDlgTwoParams */
/* CmdDlgOneBinaryChoice */
/* CmdDlgOneString */

/* forwards */
struct NoteObjectRec;
struct TrackObjectRec;

/* present a dialog box appropriate to the object type which allows the */
/* user to edit the object's attributes */
void							EditNoteOrCommandAttributes(struct NoteObjectRec* NoteCommand,
										struct TrackObjectRec* Track);

#endif
