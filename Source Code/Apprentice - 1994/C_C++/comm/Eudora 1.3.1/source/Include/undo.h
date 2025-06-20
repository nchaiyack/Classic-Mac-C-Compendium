/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * definitions for undo
 ************************************************************************/
typedef struct
{
	UHandle text; 				/* any text associated with the item */
	int startFrom;				/* where to start undoing from */
	int goTil;						/* where to stop undoing */
	Boolean wasWhat;			/* what we just did */
	MyWindowPtr win;			/* window this applies to */
	TEHandle teh; 				/* teRec this applies to */
	Boolean didUndo;			/* did we just finish undoing? */
	Boolean didClick; 		/* we just clicked somewhere */
	short oldSelStart;		/* selStart at the point of undoability */
	short oldSelEnd;			/* ditto, but end */
} UndoType;

void DoUndo(void);
void NukeUndo(MyWindowPtr win);
void TEPrepareUndo(MyWindowPtr win, TEEnum what);
void SetUndoMenu(void);
