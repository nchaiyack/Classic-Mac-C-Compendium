/* FrameObject.h */

#ifndef Included_FrameObject_h
#define Included_FrameObject_h

/* FrameObject module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* NoteObject */
/* Memory */
/* DataMunging */
/* Screen */
/* NoteImages */
/* Frequency */
/* StaffCalibration */
/* Fractions */

#include "Screen.h"

/* this is the width of the note part of an icon */
#define ICONWIDTH (20)

/* this is how much space to put between notes in the same frame */
#define INTERNALSEPARATION (12)

/* how much from the top of the note to the staff line intersection point */
#define TOPNOTESTAFFINTERSECT (23)

/* how much from the starting edge of the note icon does the note really start */
#define LEFTNOTEEDGEINSET (6)

struct FrameObjectRec;
typedef struct FrameObjectRec FrameObjectRec;

/* forward declarations */
struct FractionRec;

/* allocate a new frame. */
FrameObjectRec*				NewFrame(void);

/* dispose of a frame and all notes it contains.  be careful here since this won't */
/* worry about tie references to notes contained in the frame. */
void									DisposeFrameAndContents(FrameObjectRec* Frame);

/* get the number of notes contained in the frame */
long									NumNotesInFrame(FrameObjectRec* Frame);

/* get the note specified by the index from the frame object */
struct NoteObjectRec*	GetNoteFromFrame(FrameObjectRec* Frame, long Index);

/* delete a note in the frame.  returns True if successful. */
MyBoolean							DeleteNoteFromFrame(FrameObjectRec* Frame, long Index);

/* append a note to the frame.  returns True if successful. */
MyBoolean							AppendNoteToFrame(FrameObjectRec* Frame, struct NoteObjectRec* Note);

/* find out if this is a command frame.  command frames are allowed to have a */
/* single command and nothing else in them */
MyBoolean							IsThisACommandFrame(FrameObjectRec* Frame);

/* find out the width of this command/note frame and draw it if the flag is set. */
/* it assumes the clipping rectangle is set up properly.  the X and Y parameters */
/* specify the left edge of the note and the Middle C line. */
/* this routine does not handle drawing of ties. */
OrdType								WidthOfFrameAndDraw(WinType* Window, OrdType X, OrdType Y,
												FontType Font, FontSizeType FontSize, OrdType FontHeight,
												FrameObjectRec* Frame, MyBoolean ActuallyDraw,
												MyBoolean GreyedOut);

/* find out the duration of the specified frame.  returns the duration of the */
/* frame as a fraction */
void									DurationOfFrame(FrameObjectRec* Frame, struct FractionRec* Frac);

/* make a total (deep) copy of the frame and the notes it contains */
FrameObjectRec*				DeepDuplicateFrame(FrameObjectRec* Frame);

#endif
