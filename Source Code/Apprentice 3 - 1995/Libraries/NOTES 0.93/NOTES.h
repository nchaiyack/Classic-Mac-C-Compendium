/*****************************************************************
 *
 * NOTES.h
 *
 * See NOTES.c file for detailed descriptions.
 *
 *****************************************************************/

#pragma once

// defines

#define NOTE_NUM_ARROWS 4


// types


typedef struct {
	Rect			tools;
	Rect			title;
	Rect			new;
	Rect			delete;
	Rect			font;
	Rect			size;
	Rect			left;
	Rect			center;
	Rect			right;
	Rect			plain;
	Rect			bold;
	Rect			italic;
	Rect			bringToFront;
	Rect			sendToBack;
	Rect			showMargin;
	Rect			hideMargin;
} NoteTools, *NoteToolsPtr;


typedef struct {
	QElemPtr		qLink;			// pointer to next environ
	short			qType;			// ignored
	WindowPtr		theWindow;		// associated window
	QHdr			notes;			// list of notes
	float			hSize;			// horizontal grid size
	float			vSize;			// vertical grid size
	short			hZero;			// relative zero from environ's window's actual zero
	short			vZero;			// relative zero from environ's window's actual zero
	short			columns;		// columns in grid
	short			rows;			// rows in grid
	short			font;			// default text font number
	Str255			fontName;		// default font name
	short			size;			// default text size
	short			style;			// default text style
	short			justify;		// default text justification
	Ptr				editNote;		// pointer to note begin edited
	TEHandle		hTE;			// edit text for edited note
	Boolean			showMargin;		// flag to show margins when editing
	Boolean			editMode;		// flag is TRUE when editing notes
	Boolean			visible;		// flag is TRUE when notes are visible
	Boolean			dirty;			// flag is TRUE when a note has changed
	NoteTools		tools;			// rects for Note Tool box
	long			maxStorage;		// maximum compressed list storage
	long			usedStorage;	// current amount of required storage
} NoteEnviron, *NoteEnvironPtr;


typedef struct {
	QElemPtr		qLink;			// pointer to next note
	short			qType;			// ignored
	NoteEnviron		*environ;		// associated note environment
	unsigned char	*text;			// pointer to note text
	short			length;			// length of note text
	short			h;				// horizontal grid location of note
	short			v;				// vertical grid location of note
	short			width;			// horizontal width in grid cells
	short			height;			// vertical width in grid cells
	Rect			marginRect;		// Rect enclosing margins, border, text
	Rect			noteRect;		// Rect enclosing border, text
	Rect			textRect;		// Rect enclosing text
	Point			arrowPt;		// Point of first arrow
	Rect			growRect;		// Rect for resizing note
	Rect			dragRect;		// Rect for dragging note
	short			font;			// text font number
	Str255			fontName;		// text font name
	short			size;			// text size
	short			style;			// text style
	short			justify;		// text justification
	short			leftArrows[NOTE_NUM_ARROWS];	// left arrows
	short			topArrows[NOTE_NUM_ARROWS];		// top arrows
	short			rightArrows[NOTE_NUM_ARROWS];	// right arrows
	short			bottomArrows[NOTE_NUM_ARROWS];	// bottom arrows
	float			orig_hSize;		// hSize of environ note was created in
	float			orig_vSize;		// vSize of environ note was created in
} Note, *NotePtr;


typedef struct {
	short			h;				// horizontal grid location of note
	short			v;				// vertical grid location of note
	short			width;			// horizontal width in grid cells
	short			height;			// vertical width in grid cells
	short			size;			// text size
	short			style;			// text style
	short			justify;		// text justification
	short			leftArrows[NOTE_NUM_ARROWS];	// left arrows
	short			topArrows[NOTE_NUM_ARROWS];		// top arrows
	short			rightArrows[NOTE_NUM_ARROWS];	// right arrows
	short			bottomArrows[NOTE_NUM_ARROWS];	// bottom arrows
	float			orig_hSize;		// hSize of environ note was created in
	float			orig_vSize;		// vSize of environ note was created in
	short			fontNameLength;	// length of variable size font name to follow
	short			textLength;		// length of variable size note text to follow
} CompressedNote, CompressedNotePtr;


typedef struct {
	short			version;		// version of CompressedList & CompressedNote format
	short			numOfNotes;		// number of notes to follow
	long			unused;			// for future use
	long			storage;		// total size in bytes of CompressedList
	// variable size data follows
} CompressedList, *CompressedListPtr;



// prototypes

short NOTES_Init(void);

void NOTES_Dispose(void);

short NOTES_NewEnviron(WindowPtr theWindow, float hSize, float vSize,
						long maxStorage, NoteEnvironPtr *environ);

void NOTES_DisposeEnviron(NoteEnviron *environ);

void NOTES_SetGridSize(NoteEnviron *environ, float hSize, float vSize);

void NOTES_DrawEnviron(NoteEnviron *environ);

void NOTES_Visible(NoteEnviron *environ, Boolean visibleFlag);

void NOTES_Activate(NoteEnviron *environ, Boolean activeFlag);

void NOTES_EditMode(NoteEnviron *environ, Boolean editFlag);

void NOTES_Idle(void);

Boolean NOTES_MouseInText(Point where);

Boolean NOTES_DoClick(WindowPtr theWindow, EventRecord *theEvent);

Boolean NOTES_DoCut(WindowPtr theWindow);

Boolean NOTES_DoCopy(WindowPtr theWindow);

Boolean NOTES_DoPaste(WindowPtr theWindow);

Boolean NOTES_DoClear(WindowPtr theWindow);

Boolean NOTES_DoKey(WindowPtr theWindow, EventRecord *theEvent);

Boolean NOTES_GetChanged(NoteEnviron *environ);

void NOTES_SetChanged(NoteEnviron *environ, Boolean changedFlag);

short NOTES_Count(NoteEnviron *environ);

short NOTES_GetList(NoteEnviron *environ, CompressedListPtr *compressedList);

void NOTES_ClearList(NoteEnviron *environ);

short NOTES_SetList(NoteEnviron *environ, CompressedList *compressedList);
