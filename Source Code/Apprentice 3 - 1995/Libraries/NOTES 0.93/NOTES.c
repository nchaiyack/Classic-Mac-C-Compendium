/*****************************************************************
 *
 * NOTES.c
 *
 * Version 0.93, Created December 1994, Modified April 1995
 *
 * Macintosh Version by Tad Woods
 * P.O. Box 1210, Salem, VA 24153
 * CompuServe: 70312,3552   America Online: "Tad Woods"
 * Internet         mail: tandtsw@roanoke.infi.net
 *    anonymous ftp host: ftp.infi.net
 *             directory: /pub/customer/roanoke/tandtsw
 *
 * NOTES is a library of functions that implement editing
 * and/or displaying of helpful notes in an application.
 * The notes "overlay" and "point to" other information in
 * a window.
 *
 * The files required to use NOTES are Notes.c, Notes.h, and
 * NOTES.Resources.Rsrc. You may compile these files into your
 * own applications at no charge, as long as those applications
 * are used in-house or distributed for free. There is a one-time
 * fee of $50.00 to use NOTES in shareware or commercial
 * applications. NOTES has been tested and used but comes with
 * no guarantees or technical support. Do not sell or distribute
 * NOTES source code without the permission of T&T Software.
 *
 * The basic use of NOTES works as follows. Call the functions
 * listed in NOTES.h from your program. Descriptions of each
 * function are found in this file with their implementation.
 *
 * Call NOTES_Init and NOTES_Dispose at the start and exit
 * of your program.
 * Call NOTES_NewEnviron to initialize an environment of notes.
 * One environment can include an unlimited list of notes, or
 * as many notes that can be stored in a specified number of
 * bytes. Currently, one note environment is displayed within
 * one window. You can create multiple note environments for
 * multiple windows.
 *
 * Your program must call NOTES_Activate in response to an
 * activate event for the note environment's window.
 * Your program must call NOTES_DrawEnviron in response to
 * an update event for the note environment's window. It
 * must also be called last, after any regular drawing your
 * program does. This allows the notes to be on top of the
 * screen you are trying to document.
 *
 * When the note environment is in edit mode your program 
 * should:
 *	 Call NOTES_DoClick in response to a mouse click
 *   inside the note environment's window.
 *   Call NOTES_DoKey in response to a key event for the
 *   note environment's window.
 *   Call NOTES_Idle once through its main event loop.
 *   Call NOTES_MouseInText to see if it should change
 *   the mouse cursor to an I-beam.
 *   Call NOTES_DoCut, NOTES_DoCopy, NOTES_DoPaste,
 *   NOTES_DoClear in response to Edit menu selections.
 *      Note: NOTES_DoKey handles calling these functions
 *      in response to command-X,C,V.
 *   Note: You may call these functions when the note
 *   environment is not in edit mode; they will do
 *   nothing and return, or do nothing and return FALSE.
 *
 * Call NOTES_Visible to show and hide the notes in the
 * current environment. You may also want to disable
 * some of your program's functions (e.g. menu items)
 * while notes are visible and re-enable them when notes
 * are hidden.
 * To add notes to an environment, call NOTES_EditMode
 * with editFlag set TRUE and NOTES_Visible with visibleFlag
 * set TRUE. If your program calls NOTES_DoClick and
 * NOTES_DoKey as described above, you will be able to add
 * and edit notes. Each of a note's left, top, right, and
 * bottom edge may contain up to four arrows that point at
 * information described by the note. Click near the note's
 * edge to add and remove arrows.
 * Save the current note environment by calling NOTES_GetList
 * and then write the returned information to disk in your
 * program. Restore that note environment later by calling
 * NOTES_SetList.
 *
 * Version 0.93 - hSize and vSize are now floats.
 *                Added code to adjust the textRect when notes
 *                are created in an environment with hSize and
 *                vSize and later displayed in an environment
 *                with a different hSize and vSize.
 *                Replaced the NOTES_Changed function with
 *                NOTES_GetChanged and NOTES_SetChanged.
 *				  Tab key tabs to next note when editing.
 * Version 0.92 - First released version.
 * 
 *****************************************************************/


#include "NOTES.h"


// defines global only to this file

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define COPY_PSTR(src, dest) BlockMove((Ptr)(src), (Ptr)(dest), ((unsigned char *)(src))[0] + 1);

#define gNOTES_TOOLS_PICT_ID	30000
#define gOUT_OF_STORAGE_ID		30000

#define gMARGIN_HANDLES			12
#define gMARGIN_TEXT			8

#define ARROW_WIDTH				15

#define ARROW_LEFT				0
#define ARROW_TOP				1
#define ARROW_RIGHT				2
#define ARROW_BOTTOM			3

#define POP_UP_MENU_ID			99

#define LONG_MAX		2147483647L


// variables global only to this file

static PicHandle			gNotesToolsPict;
static QHdr					gNoteEnvirons;
static NoteEnvironPtr		gActiveEnviron;


// prototypes for this file

Boolean NOTES_EnoughStorage(NoteEnviron *environ, long subtract, long add);
Boolean NOTES_SetTools(NoteEnviron *environ, short font, Str255 fontName,
						short size, short style, short justify);
void NOTES_EnvironCalcRects(NoteEnviron *environ);
short NOTES_NewNote(NoteEnviron *environ, NotePtr *note);
void NOTES_DisposeNote(Note *note);
void NOTES_NoteCalcRects(Note *note);
Note *NOTES_GetLastNote(NoteEnviron *environ);
short NOTES_StartEditNote(Note *note);
void NOTES_StopEditNote(NoteEnviron *environ);
void NOTES_DrawTools(NoteEnviron *environ);
void NOTES_DrawArrow(Note *note, short index, short arrow);
void NOTES_DrawNote(Note *note);
void NOTES_BringToFront(Note *note);
void NOTES_SendToBack(Note *note);
void NOTES_InvalRects(NoteEnviron *environ, Rect *oldRect, Rect *newRect);
void NOTES_InvertRect(NoteEnviron *environ, Rect *theRect);
Boolean NOTES_DragRect(NoteEnviron *environ, Rect *dragRect, Point startPt,
						float snapH, float snapV, short *h, short *v);
Boolean NOTES_GrowRect(NoteEnviron *environ, Rect *growRect, Point startPt,
						float snapH, float snapV, short *width, short *height);
Boolean NOTES_TrackRect(NoteEnviron *environ, Rect *trackRect);
short NOTES_SearchMenu(MenuHandle theMenu, Str255 findItemString);
void NOTES_UpdateEditedNote(Note *note);
Boolean NOTES_SelectFont(NoteEnviron *environ, Point menuPt);
Boolean NOTES_SelectFontSize(NoteEnviron *environ, Point menuPt);
Boolean NOTES_NewArrow(Note *note, Point where, short arrow);



/*****************************************************************
 *
 * NOTES_Init
 *
 * Returns an error code or 0.
 * Call this before calling any other NOTES routine.
 *
 *****************************************************************/

short NOTES_Init(void)
{
	gNotesToolsPict = GetPicture(gNOTES_TOOLS_PICT_ID);
	if (gNotesToolsPict == NULL)
		return ResError();
		
	gNoteEnvirons.qFlags = 0;
	gNoteEnvirons.qHead = NULL;
	gNoteEnvirons.qTail = NULL;

	gActiveEnviron = NULL;
	
	return noErr;
} // NOTES_Init



/*****************************************************************
 *
 * NOTES_Dispose
 *
 * Call this before your program exits.
 *
 *****************************************************************/

void NOTES_Dispose(void)
{
	QElemPtr		qPtr;
	QElemPtr		nextqPtr;
	
	nextqPtr = gNoteEnvirons.qHead;		// dispose of note environments
	while ((qPtr = nextqPtr) != NULL)
	{
		nextqPtr = qPtr->qLink;
		NOTES_DisposeEnviron((NoteEnviron *)qPtr);
	}

	ReleaseResource((Handle)gNotesToolsPict);
} // NOTES_Dispose



/*****************************************************************
 *
 * NOTES_EnoughStorage
 *
 *  -> environ			pointer to note environment
 *  -> subtract			number of bytes to subtract
 *  -> add				number of bytes to add
 *
 * Returns TRUE and updates the usedStorage variable if
 * enough storage is available to "subtract" and "add" the
 * respective variables from and to usedStorage, otherwise
 * does nothing and returns FALSE.
 *
 *****************************************************************/

Boolean NOTES_EnoughStorage(NoteEnviron *environ, long subtract, long add)
{
	long		freeStorage = environ->maxStorage - (environ->usedStorage + add - subtract);
	Str255		text;
	
	if (freeStorage < 0L)
	{
		InitCursor();
		NumToString(-(freeStorage), text);
		if (freeStorage == -1)
			ParamText("\p(", text, "\p byte needed.)", "\p");
		else
			ParamText("\p(", text, "\p bytes needed.)", "\p");
		Alert(gOUT_OF_STORAGE_ID, NULL);
		return FALSE;
	}
	else
	{
		environ->usedStorage += add - subtract;
		return TRUE;
	}
} // NOTES_EnoughStorage



/*****************************************************************
 *
 * NOTES_NewEnviron
 *
 *  -> theWindow		pointer to window to associate notes with
 *  -> hSize			horizontal size of one grid cell in pixels
 *  -> vSize			vertical size of one grid cell in pixels
 *  -> maxStorage		maximum bytes environ can grow in terms
 *						of the ability to store it compressed
 *						(0 = unlimited; Each note requires 50 to
 *						80 bytes plus the text in the note.)
 *						(see NOTES_GetList and NOTES_SetList)
 * <-  environ			pointer to a new note environment
 *
 * Returns an error code or 0.
 * Allocates a new note environment associated with theWindow.
 * The hSize and vSize values are used to calculate a grid
 * coordinate system for the notes on top of the window. Notes
 * are always aligned and sized with cells in this grid.
 * Notes will not be visible until NOTES_Visible is called.
 *
 *****************************************************************/

short NOTES_NewEnviron(WindowPtr theWindow, float hSize, float vSize,
						long maxStorage, NoteEnvironPtr *environ)
{
	*environ = (NoteEnviron *)NewPtr(sizeof(NoteEnviron));
	if (*environ == NULL)
		return MemError();
	
	(*environ)->theWindow = theWindow;
	
	(*environ)->notes.qFlags = 0;
	(*environ)->notes.qHead = NULL;
	(*environ)->notes.qTail = NULL;
	(*environ)->hZero = 0;
	(*environ)->vZero = 0;
	NOTES_SetGridSize(*environ, hSize, vSize);
	COPY_PSTR("\pGeneva", (*environ)->fontName);
	GetFNum((*environ)->fontName, &((*environ)->font));
	(*environ)->size = 9;
	(*environ)->style = 0;
	(*environ)->justify = 0;
	(*environ)->showMargin = TRUE;
	(*environ)->editNote = NULL;
	(*environ)->hTE = NULL;
	(*environ)->editMode = FALSE;
	(*environ)->visible = FALSE;
	(*environ)->dirty = FALSE;
	(*environ)->tools.tools.left = (*environ)->hZero + (short)((*environ)->hSize);
	(*environ)->tools.tools.top = (*environ)->vZero + (short)((*environ)->vSize);
	NOTES_EnvironCalcRects((*environ));
	if (maxStorage)
		(*environ)->maxStorage = MAX(maxStorage, sizeof(CompressedList));
	else
		(*environ)->maxStorage = LONG_MAX;
	(*environ)->usedStorage = sizeof(CompressedList);
	
	Enqueue((QElemPtr)(*environ), &gNoteEnvirons);
	
	return noErr;
} // NOTES_NewEnviron



/*****************************************************************
 *
 * NOTES_DisposeEnviron
 *
 *  -> environ		pointer to note environment
 *
 * Disposes of a note environment.
 *
 *****************************************************************/

void NOTES_DisposeEnviron(NoteEnviron *environ)
{
	QElemPtr		qPtr;
	QElemPtr		nextqPtr;
	
	Dequeue((QElemPtr)environ, &gNoteEnvirons);

	if (environ == gActiveEnviron)
		gActiveEnviron = NULL;
		
	nextqPtr = environ->notes.qHead;		// dispose of notes in this noteEnviron
	while ((qPtr = nextqPtr) != NULL)
	{
		nextqPtr = qPtr->qLink;
		NOTES_DisposeNote((Note *)qPtr);
	}
	
	DisposPtr((Ptr)environ);
} // NOTES_DisposeEnviron



/*****************************************************************
 *
 * NOTES_EnvironCalcRects
 *
 *  -> environ		pointer to note environment
 *
 * Calculates and sets the rest of the Rect fields of a 
 * NoteTools record based on the tools.tools.left and 
 * tools.tools.top fields of the record.
 *
 *****************************************************************/

void NOTES_EnvironCalcRects(NoteEnviron *environ)
{
	NoteTools	*tools = &(environ->tools);
	
	// tools->tools.left and tools->tools.top must already be set to valid values
	tools->tools.right = tools->tools.left + 223;
	tools->tools.bottom = tools->tools.top + 56;

	tools->title.left = tools->tools.left + 1;
	tools->title.top = tools->tools.top + 1;
	tools->title.right = tools->tools.left + 223;
	tools->title.bottom = tools->tools.top + 12;

	tools->new.left = tools->tools.left + 1;
	tools->new.top = tools->tools.top + 13;
	tools->new.right = tools->tools.left + 34;
	tools->new.bottom = tools->tools.top + 33;

	tools->delete.left = tools->tools.left + 1;
	tools->delete.top = tools->tools.top + 34;
	tools->delete.right = tools->tools.left + 34;
	tools->delete.bottom = tools->tools.top + 54;

	tools->font.left = tools->tools.left + 36;
	tools->font.top = tools->tools.top + 13;
	tools->font.right = tools->tools.left + 74;
	tools->font.bottom = tools->tools.top + 33;

	tools->size.left = tools->tools.left + 36;
	tools->size.top = tools->tools.top + 34;
	tools->size.right = tools->tools.left + 74;
	tools->size.bottom = tools->tools.top + 54;

	tools->left.left = tools->tools.left + 76;
	tools->left.top = tools->tools.top + 13;
	tools->left.right = tools->tools.left + 111;
	tools->left.bottom = tools->tools.top + 26;

	tools->center.left = tools->tools.left + 76;
	tools->center.top = tools->tools.top + 27;
	tools->center.right = tools->tools.left + 111;
	tools->center.bottom = tools->tools.top + 40;

	tools->right.left = tools->tools.left + 76;
	tools->right.top = tools->tools.top + 41;
	tools->right.right = tools->tools.left + 111;
	tools->right.bottom = tools->tools.top + 54;

	tools->plain.left = tools->tools.left + 113;
	tools->plain.top = tools->tools.top + 13;
	tools->plain.right = tools->tools.left + 140;
	tools->plain.bottom = tools->tools.top + 26;

	tools->bold.left = tools->tools.left + 113;
	tools->bold.top = tools->tools.top + 27;
	tools->bold.right = tools->tools.left + 140;
	tools->bold.bottom = tools->tools.top + 40;

	tools->italic.left = tools->tools.left + 113;
	tools->italic.top = tools->tools.top + 41;
	tools->italic.right = tools->tools.left + 140;
	tools->italic.bottom = tools->tools.top + 54;
	
	tools->bringToFront.left = tools->tools.left + 142;
	tools->bringToFront.top = tools->tools.top + 13;
	tools->bringToFront.right = tools->tools.left + 183;
	tools->bringToFront.bottom = tools->tools.top + 33;

	tools->sendToBack.left = tools->tools.left + 142;
	tools->sendToBack.top = tools->tools.top + 34;
	tools->sendToBack.right = tools->tools.left + 183;
	tools->sendToBack.bottom = tools->tools.top + 54;

	tools->showMargin.left = tools->tools.left + 185;
	tools->showMargin.top = tools->tools.top + 13;
	tools->showMargin.right = tools->tools.left + 221;
	tools->showMargin.bottom = tools->tools.top + 33;

	tools->hideMargin.left = tools->tools.left + 185;
	tools->hideMargin.top = tools->tools.top + 34;
	tools->hideMargin.right = tools->tools.left + 221;
	tools->hideMargin.bottom = tools->tools.top + 54;
} // NOTES_EnvironCalcRects



/*****************************************************************
 *
 * NOTES_SetGridSize
 *
 *  -> environ			pointer to note environment
 *  -> hSize			horizontal size of one grid cell in pixels
 *  -> vSize			vertical size of one grid cell in pixels
 *
 * Sets the size of grid cells in the environ's coordinate
 * system. This function is called by NOTES_NewEnviron and
 * should otherwise probably not need to be called.
 *
 *****************************************************************/

void NOTES_SetGridSize(NoteEnviron *environ, float hSize, float vSize)
{
	QElemPtr		qPtr;
	Rect			oldRect;
	
	environ->hSize = hSize;
	environ->vSize = vSize;
	environ->columns = ((float)(environ->theWindow->portRect.right -
						environ->theWindow->portRect.left) / hSize);
	environ->rows = ((float)(environ->theWindow->portRect.bottom -
						environ->theWindow->portRect.top) / vSize);

	qPtr = environ->notes.qHead;		// recalculate note Rects for new grid sizes
	while (qPtr != NULL)
	{
		oldRect.left = ((Note *)qPtr)->marginRect.left;
		oldRect.top = ((Note *)qPtr)->marginRect.top;
		oldRect.right = ((Note *)qPtr)->marginRect.right;
		oldRect.bottom = ((Note *)qPtr)->marginRect.bottom;
		NOTES_NoteCalcRects((Note *)qPtr);
		if (environ->visible)
			NOTES_InvalRects(environ, &oldRect, &(((Note *)qPtr)->marginRect));
		qPtr = qPtr->qLink;
	}
} // NOTES_SetGridSize



/*****************************************************************
 *
 * NOTES_SetTools
 *
 *  -> environ			pointer to note environment
 *  -> font
 *  -> fontName
 *  -> size
 *  -> style
 *  -> justify
 *
 * Returns TRUE if any of the passed-in values are different
 * from the environ's current value. In which case the environ
 * is also updated with the new values.
 * Also updates the buttons on the Note Tool palette.
 *
 *****************************************************************/

Boolean NOTES_SetTools(NoteEnviron *environ, short font, Str255 fontName,
						short size, short style, short justify)
{
	Boolean			updated = FALSE;
	
	if (font != environ->font)
	{
		environ->font = font;
		updated = TRUE;
	}
		
	if (fontName)
	{
		COPY_PSTR(fontName, environ->fontName);
		updated = TRUE;
	}
	
	if (size != environ->size)
	{
		environ->size = size;
		updated = TRUE;
	}
		
	if (justify != environ->justify)
	{
		switch (environ->justify)
		{
			case 0:
				NOTES_InvertRect(environ, &(environ->tools.left));
				break;
			case 1:
				NOTES_InvertRect(environ, &(environ->tools.center));
				break;
			case -1:
				NOTES_InvertRect(environ, &(environ->tools.right));
				break;
		}
		switch (justify)
		{
			case 0:
				NOTES_InvertRect(environ, &(environ->tools.left));
				break;
			case 1:
				NOTES_InvertRect(environ, &(environ->tools.center));
				break;
			case -1:
				NOTES_InvertRect(environ, &(environ->tools.right));
				break;
		}
		environ->justify = justify;
		updated = TRUE;
	}
	
	if (style & 0x8000)
	{
		style &= 0x7FFF;
		if (style != 0)
			style ^= environ->style;
	}

	if (style != environ->style)
	{
		if (style == 0x0000)
			NOTES_InvertRect(environ, &(environ->tools.plain));
		if (environ->style == 0x0000)
			NOTES_InvertRect(environ, &(environ->tools.plain));
		if ((style & 0x0001) != (environ->style & 0x0001))
			NOTES_InvertRect(environ, &(environ->tools.bold));
		if ((style & 0x0002) != (environ->style & 0x0002))
			NOTES_InvertRect(environ, &(environ->tools.italic));
			
		environ->style = style;
		updated = TRUE;
	}
	
	return updated;
} // NOTES_SetTools



/*****************************************************************
 *
 * NOTES_NewNote
 *
 *  -> environ			pointer to note environment
 * <-  note				pointer to a new note
 *
 * Returns an error code or 0.
 * Allocates a new note in the environment.
 *
 *****************************************************************/

short NOTES_NewNote(NoteEnviron *environ, NotePtr *note)
{
	short		loop;
	
	if (!NOTES_EnoughStorage(environ, 0L, sizeof(CompressedNote) + environ->fontName[0]))
		return noErr;
	
	(*note) = (Note *)NewPtr(sizeof(Note));
	if (*note == NULL)
	{
		return MemError();
	}
	
	(*note)->environ = 	environ;
	(*note)->text = NULL;
	(*note)->length = 0;
	(*note)->h = (environ->columns / 2) - (short)(80.0 / environ->hSize);
	(*note)->width = (short)(160.0 / environ->hSize);
	(*note)->v = (environ->rows / 2) - (short)(20.0 / environ->vSize);
	(*note)->height = (short)(40.0 / environ->vSize);
	(*note)->orig_hSize = environ->hSize;
	(*note)->orig_vSize = environ->vSize;
	NOTES_NoteCalcRects((*note));
	(*note)->font = environ->font;
	COPY_PSTR(environ->fontName, (*note)->fontName);
	(*note)->size = environ->size;
	(*note)->style = environ->style;
	(*note)->justify = environ->justify;
	for(loop = 0; loop < NOTE_NUM_ARROWS; loop++)
	{
		(*note)->leftArrows[loop] = -1;
		(*note)->topArrows[loop] = -1;
		(*note)->rightArrows[loop] = -1;
		(*note)->bottomArrows[loop] = -1;
	}
	
	NOTES_StopEditNote(environ);
	NOTES_StartEditNote(*note);
	
	Enqueue((QElemPtr)(*note), &(environ->notes));
	
	return noErr;
} // NOTES_NewNote



/*****************************************************************
 *
 * NOTES_DisposeNote
 *
 *  -> note				pointer to a note
 *
 * Disposes of a note from it's environ.
 *
 *****************************************************************/

void NOTES_DisposeNote(Note *note)
{
	Dequeue((QElemPtr)note, &(note->environ->notes));

	if ((Note *)(note->environ->editNote) == note)
	{
		NOTES_StopEditNote(note->environ);
		note->environ->editNote = (Ptr)NOTES_GetLastNote(note->environ);
	}

	NOTES_EnoughStorage(note->environ, sizeof(CompressedNote) +
			note->fontName[0] + note->length, 0L);

	if (note->environ->visible)
		NOTES_InvalRects(note->environ, &(note->marginRect), NULL);
	
	if (note->text)
		DisposPtr((Ptr)note->text);
	DisposPtr((Ptr)note);
} // NOTES_DisposeNote



/*****************************************************************
 *
 * NOTES_NoteCalcRects
 *
 *  -> note				pointer to a note
 *
 * Calculates and sets the Rect fields of the Note record
 * to the environ's current grid size and the note's location.
 *
 *****************************************************************/

void NOTES_NoteCalcRects(Note *note)
{
	NoteEnviron		*environ = note->environ;
	short			offstToCntr;
	float			hAdjust, vAdjust;
	
										// set Rect enclosing border, text
	note->noteRect.left = environ->hZero + (short)((float)(note->h) * environ->hSize);
	note->noteRect.top = environ->vZero + (short)((float)note->v * environ->vSize);
	note->noteRect.right = note->noteRect.left + (short)((float)note->width * environ->hSize);
	note->noteRect.bottom = note->noteRect.top + (short)((float)note->height * environ->vSize);
	
										// compensate for difference in size of environ when note
										// was created and the size of the current environ
	hAdjust = ((((float)note->width * note->orig_hSize) - ((float)note->width * environ->hSize)) + 0.5) / 2.0;
	vAdjust = ((((float)note->height * note->orig_vSize) - ((float)note->height * environ->vSize)) + 0.5) / 2.0;
	note->noteRect.left -= (short)hAdjust;
	note->noteRect.top -= (short)vAdjust;
	note->noteRect.right += (short)(hAdjust + 0.5);
	note->noteRect.bottom += (short)(vAdjust + 0.5);

										// set Rect enclosing margins, border, text
	note->marginRect.left = note->noteRect.left - gMARGIN_HANDLES;
	note->marginRect.top = note->noteRect.top - gMARGIN_HANDLES;
	note->marginRect.right = note->noteRect.right + gMARGIN_HANDLES;
	note->marginRect.bottom = note->noteRect.bottom + gMARGIN_HANDLES;
	
										// set Rect enclosing text
	note->textRect.left = note->noteRect.left + gMARGIN_TEXT;
	note->textRect.top = note->noteRect.top + gMARGIN_TEXT;
	note->textRect.right = note->noteRect.right - gMARGIN_TEXT;
	note->textRect.bottom = note->noteRect.bottom - gMARGIN_TEXT;

										// set point of first arrow
	note->arrowPt.h = note->noteRect.left + (short)hAdjust;
	offstToCntr = (((note->noteRect.right - note->noteRect.left) -	// offset to left of center cell
				(short)(environ->hSize)) / 2) % (short)(environ->hSize);
	if (offstToCntr)												// offset 1st arrow from note such
		note->arrowPt.h -= ((short)(environ->hSize) - offstToCntr);	// that the middle arrow is centered			

	note->arrowPt.v = note->noteRect.top + (short)vAdjust;
	offstToCntr = (((note->noteRect.bottom - note->noteRect.top) -	// offset to top of center cell
				(short)(environ->vSize)) / 2) % (short)(environ->vSize);
	if (offstToCntr)												// offset 1st arrow from note such
		note->arrowPt.v -= ((short)(environ->vSize) - offstToCntr);	// that the middle arrow is centered			

										// set Rect enclosing drag region
	note->dragRect.left = note->marginRect.left;
	note->dragRect.top = note->marginRect.top;
	note->dragRect.right = note->marginRect.right;
	note->dragRect.bottom = note->marginRect.top + gMARGIN_HANDLES;
	
										// set Rect enclosing size box
	note->growRect.left = note->marginRect.right - gMARGIN_HANDLES;
	note->growRect.top = note->marginRect.bottom - gMARGIN_HANDLES;
	note->growRect.right = note->marginRect.right;
	note->growRect.bottom = note->marginRect.bottom;
} // NOTES_NoteCalcRects



/*****************************************************************
 *
 * NOTES_GetLastNote
 *
 *  -> environ			pointer to note environment
 *
 * Returns pointer to last note in queue.
 *
 *****************************************************************/

Note *NOTES_GetLastNote(NoteEnviron *environ)
{
	QElemPtr		qPtr;
	Note			*note;
	
	qPtr = environ->notes.qHead;
	note = (Note *)qPtr;
	while (qPtr != NULL)
	{
		note = (Note *)qPtr;
		qPtr = qPtr->qLink;
	}
	
	return note;
} // NOTES_GetLastNote



/*****************************************************************
 *
 * NOTES_StartEditNote
 *
 *  -> note				pointer to a note
 *
 * Returns 0 or an error code.
 * Allocates a text edit for note and makes it the active
 * note for editing.
 *
 *****************************************************************/

short NOTES_StartEditNote(Note *note)
{
	QElemPtr		qPtr;
	GrafPtr			remPort;
	short			err;
	
	if (note == NULL)
		return noErr;

	note->environ->editNote = (Ptr)note;

	if ((!(note->environ->visible)) || (!(note->environ->editMode)))
		return noErr;

	GetPort(&remPort);
	SetPort(note->environ->theWindow);

	TextFont(note->font);
	TextSize(note->size);
	TextFace(note->style);
	
	note->environ->hTE = TENew(&(note->textRect), &(note->textRect));
	if (note->environ->hTE == NULL)
	{
		err = MemError();
		SetPort(remPort);
		return err;
	}
	TESetJust(note->justify, note->environ->hTE);
	
	if (note->text)
	{
		TESetText((Ptr)(note->text), note->length, note->environ->hTE);
		TESetSelect(note->length, note->length, note->environ->hTE);
	}
	
	TEActivate(note->environ->hTE);
	
	NOTES_InvalRects(note->environ, NULL, &(note->marginRect));
	
	SetPort(remPort);
	
	NOTES_SetTools(note->environ, note->font, note->fontName, note->size,
					note->style, note->justify);

	return noErr;
} // NOTES_StartEditNote



/*****************************************************************
 *
 * NOTES_StopEditNote
 *
 *  -> environ		pointer to note environment
 *
 * Returns 0 or an error code.
 * Disposes of a text edit for the currently edited note.
 *
 *****************************************************************/

void NOTES_StopEditNote(NoteEnviron *environ)
{
	Note 		*note = (Note *)(environ->editNote);

	if ((note) && (environ->hTE))
	{
		TEDeactivate(environ->hTE);
		
		if (note->text)						// dispose of old text
			DisposPtr((Ptr)note->text);
		
											// copy new text
		note->text = (unsigned char *)NewPtr((**(environ->hTE)).teLength);
		if (note->text)
		{
			note->length = (**(environ->hTE)).teLength;
			HLock((**(environ->hTE)).hText);
			BlockMove(*((**(environ->hTE)).hText), note->text, note->length);
			HUnlock((**(environ->hTE)).hText);
		}
		else
		{
			NOTES_EnoughStorage(gActiveEnviron, (**(environ->hTE)).teLength, 0L);
			note->length = 0;
		}
		
		TEDispose(environ->hTE);
		environ->hTE = NULL;
		NOTES_InvalRects(environ, NULL, &(note->marginRect));
	}
} // NOTES_StopEditNote



/*****************************************************************
 *
 * NOTES_DrawTools
 *
 *  -> environ		pointer to note environment
 *
 * Draws the tools palette.
 *
 *****************************************************************/

void NOTES_DrawTools(NoteEnviron *environ)
{
	NoteTools	*tools = &(environ->tools);
	
	DrawPicture(gNotesToolsPict, &(tools->tools));
	
								// highlight style button(s)
	if (environ->style == 0x0000)
		InvertRect(&(tools->plain));
	else
	{
		if (environ->style & 0x0001)
			InvertRect(&(tools->bold));
		if (environ->style & 0x0002)
			InvertRect(&(tools->italic));
	}
	
								// highlight justify button
	switch (environ->justify)
	{
		case 0:
			InvertRect(&(tools->left));
			break;
		case 1:
			InvertRect(&(tools->center));
			break;
		case -1:
			InvertRect(&(tools->right));
			break;
		default:
			break;	// error
	}
	
								// highlight show or hide margins button
	if (environ->showMargin)
		InvertRect(&(tools->showMargin));
	else
		InvertRect(&(tools->hideMargin));

} // NOTES_DrawTools



/*****************************************************************
 *
 * NOTES_DrawArrow
 *
 *  -> note				pointer to a note
 *  -> index			index in ..Arrows array the position 
 *						for this arrow is stored
 *  -> arrow			ARROW_LEFT, ARROW_TOP, ARROW_RIGHT,
 *						ARROW_BOTTOM
 *
 * Draws an arrow. If an arrow is no longer visible because
 * the size of the note changed, it is removed.
 *
 *****************************************************************/
 
void NOTES_DrawArrow(Note *note, short index, short arrow)
{
	NoteEnviron		*environ = note->environ;
	Point			arrowPt;
	short			loop;
	
	switch (arrow)
	{
		case ARROW_LEFT:
			arrowPt.h = note->noteRect.left + 3;
			arrowPt.v = note->arrowPt.v + (short)((float)(note->leftArrows[index]) * environ->vSize) +
							(((short)(environ->vSize) - ARROW_WIDTH) / 2);
			if (arrowPt.v >= note->noteRect.bottom)
			{
				note->leftArrows[index] = -1;	// remove arrow we can't see
				break;
			}
			for(loop = ARROW_WIDTH; loop >= -1; loop -= 2)
			{
				if (loop <= (ARROW_WIDTH / 2))	// draw white outline around part
				{								// of arrow outside of border
					ForeColor(whiteColor);
					MoveTo(arrowPt.h, arrowPt.v - 1);
					Line(0, loop + 2);
					ForeColor(blackColor);
				}
				if (loop > 0)
				{								// draw arrow
					MoveTo(arrowPt.h, arrowPt.v);
					Line(0, loop);
					(arrowPt.h)--;
					(arrowPt.v)++;
				}
			}
			break;
			
		case ARROW_TOP:
			arrowPt.h = note->arrowPt.h + (short)((float)(note->topArrows[index]) * environ->hSize) +
							(((short)(environ->hSize) - ARROW_WIDTH) / 2);
			arrowPt.v = note->noteRect.top + 3;
			if (arrowPt.h >= note->noteRect.right)
			{
				note->topArrows[index] = -1;	// remove arrow we can't see
				break;
			}
			for(loop = ARROW_WIDTH; loop >= -1; loop -= 2)
			{
				if (loop <= (ARROW_WIDTH / 2))	// draw white outline around part
				{								// of arrow outside of border
					ForeColor(whiteColor);
					MoveTo(arrowPt.h - 1, arrowPt.v);
					Line(loop + 2, 0);
					ForeColor(blackColor);
				}
				if (loop > 0)
				{								// draw arrow
					MoveTo(arrowPt.h, arrowPt.v);
					Line(loop, 0);
					(arrowPt.h)++;
					(arrowPt.v)--;
				}
			}
			break;
			
		case ARROW_RIGHT:
			arrowPt.h = note->noteRect.right - 4;
			arrowPt.v = note->arrowPt.v + (short)((float)(note->rightArrows[index]) * environ->vSize) +
							(((short)(environ->vSize) - ARROW_WIDTH) / 2);
			if (arrowPt.v >= note->noteRect.bottom)
			{
				note->rightArrows[index] = -1;	// remove arrow we can't see
				break;
			}
			for(loop = ARROW_WIDTH; loop >= -1; loop -= 2)
			{
				if (loop <= (ARROW_WIDTH / 2))	// draw white outline around part
				{								// of arrow outside of border
					ForeColor(whiteColor);
					MoveTo(arrowPt.h, arrowPt.v - 1);
					Line(0, loop + 2);
					ForeColor(blackColor);
				}
				if (loop > 0)
				{								// draw arrow
					MoveTo(arrowPt.h, arrowPt.v);
					Line(0, loop);
					(arrowPt.h)++;
					(arrowPt.v)++;
				}
			}
			break;
			
		case ARROW_BOTTOM:
			arrowPt.h = note->arrowPt.h + (short)((float)(note->bottomArrows[index]) * environ->hSize) +
							(((short)(environ->hSize) - ARROW_WIDTH) / 2);
			arrowPt.v = note->noteRect.bottom - 4;
			if (arrowPt.h >= note->noteRect.right)
			{
				note->bottomArrows[index] = -1;	// remove arrow we can't see
				break;
			}
			for(loop = ARROW_WIDTH; loop >= -1; loop -= 2)
			{
				if (loop <= (ARROW_WIDTH / 2))	// draw white outline around part
				{								// of arrow outside of border
					ForeColor(whiteColor);
					MoveTo(arrowPt.h - 1, arrowPt.v);
					Line(loop + 2, 0);
					ForeColor(blackColor);
				}
				if (loop > 0)
				{								// draw arrow
					MoveTo(arrowPt.h, arrowPt.v);
					Line(loop, 0);
					(arrowPt.h)++;
					(arrowPt.v)++;
				}
			}
			break;
	}
} // NOTES_DrawArrow



/*****************************************************************
 *
 * NOTES_DrawNote
 *
 *  -> note				pointer to a note
 *
 * Draws a note.
 *
 *****************************************************************/

void NOTES_DrawNote(Note *note)
{
	Rect		tempRect;
	short		loop;
	
	if ((note->environ->editMode) && (note->environ->showMargin))
	{
		EraseRect(&(note->marginRect));
		FrameRect(&(note->marginRect));
		
								// frame size box
		FrameRect(&(note->growRect));
		
		if (note == (Note *)(note->environ->editNote))
		{
								// draw size box
			tempRect.left = note->growRect.left + 4;
			tempRect.top = note->growRect.top + 4;
			tempRect.right = note->growRect.right - 2;
			tempRect.bottom = note->growRect.bottom - 2;
			FrameRect(&tempRect);
			tempRect.left = note->growRect.left + 2;
			tempRect.top = note->growRect.top + 2;
			tempRect.right = note->growRect.right - 5;
			tempRect.bottom = note->growRect.bottom - 5;
			EraseRect(&tempRect);
			FrameRect(&tempRect);

								// draw drag region
			for (loop = 2; loop < gMARGIN_HANDLES; loop += 2)
			{
				MoveTo(note->dragRect.left, note->dragRect.top + loop);
				LineTo(note->dragRect.right - 1, note->dragRect.top + loop);
			}
		}
	}
	else
	{
		EraseRect(&(note->noteRect));
	}

								// frame note
	tempRect.left = note->noteRect.left + 1;
	tempRect.top = note->noteRect.top + 1;
	tempRect.right = note->noteRect.right - 1;
	tempRect.bottom = note->noteRect.bottom - 1;
	FrameRect(&tempRect);

								// draw note text
	if ((note->environ->editMode) && (note == (Note *)(note->environ->editNote)))
	{
								// draw edit text
		tempRect.left = note->textRect.left - 2;
		tempRect.top = note->textRect.top - 2;
		tempRect.right = note->textRect.right + 2;
		tempRect.bottom = note->textRect.bottom + 2;
		FrameRect(&tempRect);
		TEUpdate(&(note->environ->theWindow->portRect), note->environ->hTE);
	}
	else if (note->length)
	{
								// draw text
		TextFont(note->font);
		TextSize(note->size);
		TextFace(note->style);
		TextBox((Ptr)(note->text), note->length, &(note->textRect), note->justify);
	}

								// draw arrows
	for(loop = 0; loop < NOTE_NUM_ARROWS; loop++)
	{
		if (note->leftArrows[loop] != -1)
			NOTES_DrawArrow(note, loop, ARROW_LEFT);
		if (note->topArrows[loop] != -1)
			NOTES_DrawArrow(note, loop, ARROW_TOP);
		if (note->rightArrows[loop] != -1)
			NOTES_DrawArrow(note, loop, ARROW_RIGHT);
		if (note->bottomArrows[loop] != -1)
			NOTES_DrawArrow(note, loop, ARROW_BOTTOM);
	}
} // NOTES_DrawNote



/*****************************************************************
 *
 * NOTES_DrawEnviron
 *
 *  -> environ		pointer to note environment
 *
 * Draws all environment's notes, and the tool palette if 
 * the environ is in edit mode.
 * Call in response to an update event in the environ's window.
 * Does nothing if the notes for the window's environ are not
 * currently visible (see NOTES_Visible).
 *
 *****************************************************************/

void NOTES_DrawEnviron(NoteEnviron *environ)
{
	QElemPtr		qPtr;
	GrafPtr			remPort;
	
	if (!(environ->visible))
		return;
		
	GetPort(&remPort);
	SetPort(environ->theWindow);

	PenSize(1, 1);
	ForeColor(blackColor);
	BackColor(whiteColor);

	qPtr = environ->notes.qHead;		// draw all notes in this environ
	while (qPtr != NULL)
	{
		if ((!(environ->editMode)) || (qPtr != (QElemPtr)(environ->editNote)))
			NOTES_DrawNote((Note *)qPtr);
		qPtr = qPtr->qLink;
	}
	
	if ((environ->editMode) && (environ->editNote))	// draw edited note last
		NOTES_DrawNote((Note *)(environ->editNote));
	
	if (environ->editMode)
		NOTES_DrawTools(environ);

	SetPort(remPort);
} // NOTES_DrawEnviron



/*****************************************************************
 *
 * NOTES_BringToFront
 *
 *  -> note				pointer to a note
 *
 * Puts a note in the back of the queue for drawing last,
 * thus making it appear in front of any overlapping notes.
 *
 *****************************************************************/

void NOTES_BringToFront(Note *note)
{
	Dequeue((QElemPtr)note, &(note->environ->notes));
	Enqueue((QElemPtr)note, &(note->environ->notes));
	NOTES_InvalRects(note->environ, NULL, &(note->marginRect));
} // NOTES_BringToFront



/*****************************************************************
 *
 * NOTES_SendToBack
 *
 *  -> note				pointer to a note
 *
 * Puts a note in the front of the queue for drawing first,
 * thus making it appear behind of any overlapping notes.
 *
 *****************************************************************/

void NOTES_SendToBack(Note *note)
{
	NOTES_BringToFront(note);
	
	while (note->environ->notes.qHead != (QElemPtr)note) // move all other notes behind
	{
		NOTES_BringToFront((Note *)(note->environ->notes.qHead));
	}
} // NOTES_SendToBack



/*****************************************************************
 *
 * NOTES_InvalRects
 *
 *  -> environ		pointer to note environment
 *  -> oldRect		pointer to Rect to invalidate or NULL
 *  -> newRect		pointer to Rect to invalidate or NULL
 *
 * Invalidates oldRect and newRect in the environ's window
 * for updating. And erases the area in oldRect that is not
 * also part of newRect.
 *
 *****************************************************************/

void NOTES_InvalRects(NoteEnviron *environ, Rect *oldRect, Rect *newRect)
{
	GrafPtr		remPort;
	RgnHandle	oldRgn;
	RgnHandle	newRgn;
	
	GetPort(&remPort);
	SetPort(environ->theWindow);

	if (oldRect)
	{
		if (newRect)
		{
			oldRgn = NewRgn();
			newRgn = NewRgn();
			RectRgn(oldRgn, oldRect);
			RectRgn(newRgn, newRect);
			DiffRgn(oldRgn, newRgn, oldRgn);
			EraseRgn(oldRgn);
			InvalRgn(oldRgn);
			DisposeRgn(oldRgn);
			DisposeRgn(newRgn);
			InvalRect(newRect);
		}
		else
			EraseRect(oldRect);
			
		InvalRect(oldRect);
	}
	else if (newRect)
		InvalRect(newRect);
	
	SetPort(remPort);
} // NOTES_InvalRects



/*****************************************************************
 *
 * NOTES_InvertRect
 *
 *  -> environ		pointer to note environment
 *  -> theRect		pointer to Rect to invalidate for updating
 *
 * Inverts a Rect in the environ's window.
 *
 *****************************************************************/

void NOTES_InvertRect(NoteEnviron *environ, Rect *theRect)
{
	GrafPtr		remPort;
	
	GetPort(&remPort);
	SetPort(environ->theWindow);

	InvertRect(theRect);
	
	SetPort(remPort);
} // NOTES_InvertRect



/*****************************************************************
 *
 * NOTES_Visible
 *
 *  -> environ			pointer to note environment
 *  -> visibleFlag		TRUE to show notes, FALSE to hide
 *
 * Shows or hides the environ's notes.
 *
 *****************************************************************/

void NOTES_Visible(NoteEnviron *environ, Boolean visibleFlag)
{
	QElemPtr		qPtr;

	if (environ->visible == visibleFlag)
		return;
		
	environ->visible = visibleFlag;
	
	NOTES_StopEditNote(environ);

	qPtr = environ->notes.qHead;	// force redraw of all notes
	while (qPtr != NULL)
	{
		if (visibleFlag)
			NOTES_InvalRects(environ, NULL, &(((Note *)qPtr)->marginRect));
		else
			NOTES_InvalRects(environ, &(((Note *)qPtr)->marginRect), NULL);
		qPtr = qPtr->qLink;
	}
	
	if (environ->editMode)
	{
		if (visibleFlag)
			NOTES_InvalRects(environ, NULL, &(environ->tools.tools));
		else
			NOTES_InvalRects(environ, &(environ->tools.tools), NULL);
	}
	
	NOTES_StartEditNote((Note *)(environ->editNote));

} // NOTES_Visible



/*****************************************************************
 *
 * NOTES_Activate
 *
 *  -> environ			pointer to note environment
 *  -> activeFlag		TRUE to activate environment, FALSE
 *						to deactivate
 *
 * Activates or deactivates a note environ. Call in response
 * to an activate event for the environ's window.
 *
 *****************************************************************/

void NOTES_Activate(NoteEnviron *environ, Boolean activeFlag)
{
	if (activeFlag)
	{
		gActiveEnviron = environ;
		if (environ->hTE)
			TEActivate(environ->hTE);
	}
	else
	{
		gActiveEnviron = NULL;
		if (environ->hTE)
			TEDeactivate(environ->hTE);
	}
} // NOTES_Activate



/*****************************************************************
 *
 * NOTES_EditMode
 *
 *  -> environ			pointer to note environment
 *  -> editFlag			TRUE to edit notes, FALSE to exit
 *						edit mode
 *
 * Enable or disable editing notes for the environ. If the
 * notes are visible or when they become visible (see
 * NOTES_Visible) the tool palette will be available for
 * adding, deleting, and editing notes.
 *
 *****************************************************************/

void NOTES_EditMode(NoteEnviron *environ, Boolean editFlag)
{
	QElemPtr		qPtr;

	if (environ->editMode == editFlag)
		return;
		
	environ->editMode = editFlag;
	
	if (environ->visible)
	{
		NOTES_StopEditNote(environ);

		if (environ->showMargin)
		{
			qPtr = environ->notes.qHead;	// force redraw of all notes
			while (qPtr != NULL)
			{
				NOTES_InvalRects(environ, &(((Note *)qPtr)->marginRect), &(((Note *)qPtr)->noteRect));
				qPtr = qPtr->qLink;
			}
		}
		
		if (editFlag)
			NOTES_InvalRects(environ, NULL, &(environ->tools.tools));
		else
			NOTES_InvalRects(environ, &(environ->tools.tools), NULL);
		
		NOTES_StartEditNote((Note *)(environ->editNote));
	}
} // NOTES_EditMode



/*****************************************************************
 *
 * NOTES_Idle
 *
 * Flashes the cursor when a note is being edited in the active
 * environ.
 * Returns FALSE if the active window does not have a note
 * environment (see NOTES_Activate), or the notes for the
 * active window's environ are not currently visible (see
 * NOTES_Visible), or the environ is not in edit mode with a
 * note being edited (see NOTES_EditMode).
 *
 *****************************************************************/

void NOTES_Idle(void)
{
	if ((gActiveEnviron) && (gActiveEnviron->visible) && (gActiveEnviron->hTE))
		TEIdle(gActiveEnviron->hTE);
} // NOTES_Idle



/*****************************************************************
 *
 * NOTES_MouseInText
 *
 *  -> where		current mouse position in local coordinates
 *
 * Returns TRUE if where is inside a note's edit text box
 * when it is being edited.
 * Returns FALSE if the active window does not have a note
 * environment (see NOTES_Activate), or the notes for the
 * active window's environ are not currently visible (see
 * NOTES_Visible), or the environ is not in edit mode with a
 * note being edited (see NOTES_EditMode).
 *
 *****************************************************************/

Boolean NOTES_MouseInText(Point where)
{
	
	return ((gActiveEnviron) && (gActiveEnviron->visible) && (gActiveEnviron->hTE) &&
		(PtInRect(where, &((**(gActiveEnviron->hTE)).viewRect))));
} // NOTES_MouseInText



/*****************************************************************
 *
 * NOTES_DragRect
 *
 *  -> environ		pointer to note environment
 *  -> dragRect		pointer to rectangle to drag
 *  -> startPt		mouse coordinate to start drag
 *  -> snapH		horizontal pixels to snap to ( >= 1)
 *  -> snapV		vertical pixels to snap to ( >= 1)
 * <-> h			pointer to old and new h cell, or NULL
 * <-> v			pointer to old and new v cell, or NULL
 *
 * Returns TRUE if dragRect's coordinates changed.
 * Drags a note's Rect around the window.
 *
 *****************************************************************/

Boolean NOTES_DragRect(NoteEnviron *environ, Rect *dragRect, Point startPt,
						float snapH, float snapV, short *h, short *v)
{
	RgnHandle		dragRgn;
	long			distVH;
	short			distH;
	short			distV;
	
	startPt.h += environ->hZero;
	startPt.v += environ->vZero;

	dragRgn = NewRgn();
	
	RectRgn(dragRgn, dragRect);
	
	distVH = DragGrayRgn(dragRgn, startPt, &(environ->theWindow->portRect),
						&(environ->theWindow->portRect), 0, NULL);

	DisposeRgn(dragRgn);

	distH = (distVH & 0x0000FFFF);
	distV = (distVH >> 16);
	
	if ((distVH != 0x00000000) && (distVH != 0x80008000))
	{
		if (h)
			*h += (short)((float)distH / snapH);
		distH = (short)((float)distH / snapH);
		distH = (short)((float)distH * snapH);
		if (v)
			*v += (short)((float)distV / snapV);
		distV = (short)((float)distV / snapV);
		distV = (short)((float)distV * snapV);

		dragRect->left += distH;
		dragRect->top += distV;
		dragRect->right += distH;
		dragRect->bottom += distV;
		return TRUE;
	}
	
	return FALSE;
} // NOTES_DragRect



/*****************************************************************
 *
 * NOTES_GrowRect
 *
 *  -> environ		pointer to note environment
 *  -> growRect		pointer to rectangle to grow
 *  -> startPt		mouse coordinate to start grow
 *  -> snapH		horizontal pixels to snap to ( >= 1)
 *  -> snapV		vertical pixels to snap to ( >= 1)
 * <-> width 		pointer to old and new width in cells
 * <-> heigth 		pointer to old and new height in cells
 *
 * Returns TRUE if growRect's coordinates changed.
 * Grows or shrinks the size of a note.
 *
 *****************************************************************/

Boolean NOTES_GrowRect(NoteEnviron *environ, Rect *growRect, Point startPt,
						float snapH, float snapV, short *width, short *height)
{
	WindowPtr		growWindow;
	Rect			tempRect;
	long			sizeVH;
	short			sizeH;
	short			sizeV;
	Rect			limitRect;
	short			marginH = (growRect->right - growRect->left) - (short)((float)(*width) * snapH);
	short			marginV = (growRect->bottom - growRect->top) - (short)((float)(*height) * snapH);
	
	startPt.h += environ->hZero;
	startPt.v += environ->vZero;
	LocalToGlobal(&startPt);
	
	tempRect.left = growRect->left;
	tempRect.top = growRect->top + 18;		// 18 compensates for no title bar
	tempRect.right = growRect->right;
	tempRect.bottom = growRect->bottom;
	LocalToGlobal((Point *)&(tempRect.top));
	LocalToGlobal((Point *)&(tempRect.bottom));
	
	growWindow = NewWindow(NULL, &tempRect, "\p", FALSE, documentProc,
							(WindowPtr)(-1L), FALSE, 0L);
	if (growWindow == NULL)
		return FALSE;

	limitRect.left = 26 + marginH;
	limitRect.top = 26 + marginV - 18;
	limitRect.right = (short)(environ->hSize * (float)(environ->columns)) - growRect->left;
	limitRect.bottom = (short)(environ->vSize * (float)(environ->rows)) - growRect->top - 18;
	
	sizeVH = GrowWindow(growWindow, startPt, &limitRect);
	
	DisposeWindow(growWindow);

	if (sizeVH)
	{
		sizeH = (sizeVH & 0x0000FFFF) - marginH;
		*width = (short)(((float)sizeH + (snapH / 2.0)) / snapH);
		growRect->right = growRect->left + (short)((float)(*width) * snapH) + marginH;
		
		sizeV = (sizeVH >> 16) - marginV + 18;
		*height = (short)(((float)sizeV + (snapV / 2.0)) / snapV);
		growRect->bottom = growRect->top + (short)((float)(*height) * snapV) + marginV;
		
		return TRUE;
	}
	
	return FALSE;
} // NOTES_GrowRect



/*****************************************************************
 *
 * NOTES_TrackRect
 *
 *  -> environ		pointer to note environment
 *  -> trackRect	pointer to rectangle to track mouse within
 *
 * Returns TRUE if mouse is released within trackRect.
 * Used to track pressing buttons in the tool palette.
 *
 *****************************************************************/

Boolean NOTES_TrackRect(NoteEnviron *environ, Rect *trackRect)
{
	Point		where;
	Boolean		highlighted = FALSE;
	
	if (StillDown())
	{
		while (WaitMouseUp())
		{
			GetMouse(&where);
			where.h -= environ->hZero;
			where.v -= environ->vZero;
			
			if (PtInRect(where, trackRect))
			{
				if (!highlighted)
				{
					InvertRect(trackRect);
					highlighted = TRUE;
				}
			}
			else
			{
				if (highlighted)
				{
					InvertRect(trackRect);
					highlighted = FALSE;
				}
			}
		}
		if (highlighted)
			InvertRect(trackRect);
	}

	return highlighted;
} // NOTES_TrackRect



/*****************************************************************
 *
 * NOTES_SearchMenu
 *
 *	-> theMenu			Mac menu handle
 *  -> findItemString	menu item to find
 *
 * Returns the menu item number of the item named
 * findItemString.
 *
 ****************************************************************/

short NOTES_SearchMenu(MenuHandle theMenu, Str255 findItemString)
{
	short		item = CountMItems(theMenu);
	Str255		itemString;
	
	while (item > 0)
	{
		GetItem(theMenu, item, itemString);
		if (EqualString(findItemString, itemString, TRUE, FALSE))
			return item;
		item--;
	}
	return 0;
} // NOTES_SearchMenu



/*****************************************************************
 *
 * NOTES_UpdateEditedNote
 *
 *  -> note				pointer to a note
 *
 * Copies text display characteristics from the current tool
 * palette settings, and updates the note to reflect those
 * settings.
 *
 *****************************************************************/

void NOTES_UpdateEditedNote(Note *note)
{
	FontInfo		fInfo;
	
	if (note == NULL)
		return;

	note->font = note->environ->font;
	COPY_PSTR(note->environ->fontName, note->fontName);
	note->size = note->environ->size;
	note->style = note->environ->style;
	note->justify = note->environ->justify;
	
	if (note->environ->hTE)
	{
		(**(note->environ->hTE)).destRect.left = note->textRect.left;
		(**(note->environ->hTE)).destRect.top = note->textRect.top;
		(**(note->environ->hTE)).destRect.right = note->textRect.right;
		(**(note->environ->hTE)).destRect.bottom = note->textRect.bottom;
		
		(**(note->environ->hTE)).viewRect.left = note->textRect.left;
		(**(note->environ->hTE)).viewRect.top = note->textRect.top;
		(**(note->environ->hTE)).viewRect.right = note->textRect.right;
		(**(note->environ->hTE)).viewRect.bottom = note->textRect.bottom;

		(**(note->environ->hTE)).txFont = note->font;
		(**(note->environ->hTE)).txSize = note->size;
		(**(note->environ->hTE)).txFace = note->style;
		TESetJust(note->justify, note->environ->hTE);
		
		TextFont(note->font);
		TextSize(note->size);
		TextFace(note->style);
		GetFontInfo(&fInfo);
		(**(note->environ->hTE)).lineHeight = fInfo.ascent + fInfo.descent + fInfo.leading;
		(**(note->environ->hTE)).fontAscent = fInfo.ascent;
		
		TECalText(note->environ->hTE);
	}
	
	NOTES_InvalRects(note->environ, NULL, &(note->marginRect));
	
	note->environ->dirty = TRUE;
} // NOTES_UpdateEditedNote



/*****************************************************************
 *
 * NOTES_SelectFont
 *
 *  -> environ		pointer to note environment
 *  -> menuPt		mouse click in global coordinates
 *
 * Returns TRUE if a new font is selected.
 * Displays a pop-up menu of available fonts.
 *
 *****************************************************************/

Boolean NOTES_SelectFont(NoteEnviron *environ, Point menuPt)
{
	MenuHandle		fontMenu;
	long			menuItem;
	short			curSelection;
	Str255			text;
	Boolean			result = FALSE;
	
	menuPt.h += environ->hZero;
	menuPt.v += environ->vZero;
	LocalToGlobal(&menuPt);
	
	fontMenu = NewMenu(POP_UP_MENU_ID, "\p");
	if (fontMenu == NULL)
		return FALSE;
	AddResMenu(fontMenu, 'FONT');
	InsertMenu(fontMenu, -1);

	curSelection = NOTES_SearchMenu(fontMenu, environ->fontName);
	if (curSelection)					// mark the currently selected font
		CheckItem(fontMenu, curSelection, TRUE);
	
	
	NOTES_InvertRect(environ, &(environ->tools.font));
	menuItem = PopUpMenuSelect(fontMenu, menuPt.v, menuPt.h, curSelection);
	NOTES_InvertRect(environ, &(environ->tools.font));
	
	if (menuItem & 0xFFFF0000)
	{
		menuItem &= 0x0000FFFF;
		if (curSelection != menuItem)
		{
			GetItem(fontMenu, menuItem, text);
			
			if (!NOTES_EnoughStorage(environ, environ->fontName[0], text[0]))
				result = FALSE;
			else
			{
				COPY_PSTR(text, environ->fontName);
				GetFNum(environ->fontName, &(environ->font));
				result = TRUE;
			}
		}
	}

	DeleteMenu(POP_UP_MENU_ID);
	DisposeMenu(fontMenu);

	return result;
} // NOTES_SelectFont



/*****************************************************************
 *
 * NOTES_SelectFontSize
 *
 *  -> environ		pointer to note environment
 *  -> menuPt		mouse click in global coordinates
 *
 * Returns TRUE if a new font size is selected.
 * Displays a pop-up menu of available font sizes.
 *
 *****************************************************************/

Boolean NOTES_SelectFontSize(NoteEnviron *environ, Point menuPt)
{
	MenuHandle		sizeMenu;
	long			menuItem;
	short			curSelection;
	Str255			text;
	long			size;
	Boolean			result = FALSE;
	
	menuPt.h += environ->hZero;
	menuPt.v += environ->vZero;
	LocalToGlobal(&menuPt);
	
	sizeMenu = NewMenu(POP_UP_MENU_ID, "\p");
	if (sizeMenu == NULL)
		return FALSE;
	
	for (size = 9L; size < 25L; size++)
	{
		if (RealFont(environ->font, size))
		{
			NumToString(size, text);
			AppendMenu(sizeMenu, text);
		}
	}
	InsertMenu(sizeMenu, -1);

	NumToString(environ->size, text);
	curSelection = NOTES_SearchMenu(sizeMenu, text);
	if (curSelection)						// mark the currently selected font
		CheckItem(sizeMenu, curSelection, TRUE);
	
	NOTES_InvertRect(environ, &(environ->tools.size));
	menuItem = PopUpMenuSelect(sizeMenu, menuPt.v, menuPt.h, curSelection);
	NOTES_InvertRect(environ, &(environ->tools.size));

	if (menuItem & 0xFFFF0000)
	{
		menuItem &= 0x0000FFFF;
		if (curSelection != menuItem)
		{
			GetItem(sizeMenu, menuItem, text);
			StringToNum(text, &size);
			environ->size = size;
			result = TRUE;
		}
	}

	DeleteMenu(POP_UP_MENU_ID);
	DisposeMenu(sizeMenu);

	return result;
} // NOTES_SelectFontSize



/*****************************************************************
 *
 * NOTES_NewArrow
 *
 *  -> note				pointer to a note
 *  -> where			mouse click in local coordinates
 *  -> arrow			ARROW_LEFT, ARROW_TOP, ARROW_RIGHT,
 *						ARROW_BOTTOM
 *
 * Returns TRUE if arrow was placed or removed from the note.
 * Calculates where to place an arrow based on a user click
 * near the note's border.
 *
 *****************************************************************/

Boolean NOTES_NewArrow(Note *note, Point where, short arrow)
{
	short		h;
	short		v;
	short		loop;
	short		useIndex = -1;
	
	switch (arrow)
	{
		case ARROW_LEFT:
			v = (short)((float)(where.v - note->arrowPt.v) / note->environ->vSize);
			
			for(loop = 0; loop < NOTE_NUM_ARROWS; loop++)
			{
				if (note->leftArrows[loop] == v)
				{
					note->leftArrows[loop] = -1;
					return TRUE;
				}
				else if (note->leftArrows[loop] == -1)
					useIndex = loop;
			}
			if (useIndex > -1)
			{
				note->leftArrows[useIndex] = v;
				return TRUE;
			}
			break;
			
		case ARROW_TOP:
			h = (short)((float)(where.h - note->arrowPt.h) / note->environ->hSize);
			
			for(loop = 0; loop < NOTE_NUM_ARROWS; loop++)
			{
				if (note->topArrows[loop] == h)
				{
					note->topArrows[loop] = -1;
					return TRUE;
				}
				else if (note->topArrows[loop] == -1)
					useIndex = loop;
			}
			if (useIndex > -1)
			{
				note->topArrows[useIndex] = h;
				return TRUE;
			}
			break;
			
		case ARROW_RIGHT:
			v = (short)((float)(where.v - note->arrowPt.v) / note->environ->vSize);
			
			for(loop = 0; loop < NOTE_NUM_ARROWS; loop++)
			{
				if (note->rightArrows[loop] == v)
				{
					note->rightArrows[loop] = -1;
					return TRUE;
				}
				else if (note->rightArrows[loop] == -1)
					useIndex = loop;
			}
			if (useIndex > -1)
			{
				note->rightArrows[useIndex] = v;
				return TRUE;
			}
			break;
			
		case ARROW_BOTTOM:
			h = (short)((float)(where.h - note->arrowPt.h) / note->environ->hSize);
			
			for(loop = 0; loop < NOTE_NUM_ARROWS; loop++)
			{
				if (note->bottomArrows[loop] == h)
				{
					note->bottomArrows[loop] = -1;
					return TRUE;
				}
				else if (note->bottomArrows[loop] == -1)
					useIndex = loop;
			}
			if (useIndex > -1)
			{
				note->bottomArrows[useIndex] = h;
				return TRUE;
			}
			break;
	}

	return FALSE;
} // NOTES_NewArrow



/*****************************************************************
 *
 * NOTES_DoClick
 *
 *  -> theWindow		pointer to active window
 *  -> theEvent			pointer to mouse event record
 *
 * Returns TRUE if the click was processed. Call this in
 * response to a mouse down event in a window with a note
 * environ.
 * Returns FALSE if theWindow is not the active window (see
 * NOTES_Activate), or if there is no note environ for the
 * window, or the notes for the window's environ are not
 * currently visible (see NOTES_Visible), or the environ is
 * not in edit mode (see NOTES_EditMode).
 *
 *****************************************************************/
 
Boolean NOTES_DoClick(WindowPtr theWindow, EventRecord *theEvent)
{
	NoteEnviron		*environ = gActiveEnviron;
	QElemPtr		qPtr;
	Note			*note;
	GrafPtr			remPort;
	Rect			oldRect;
	Point			where;
	Boolean			result = FALSE;
	
	if ((environ == NULL) || (environ->theWindow != theWindow) ||
			(!(environ->editMode)) || (!(environ->visible)))
	{
		return FALSE;
	}
	
	where.h = theEvent->where.h - environ->hZero;
	where.v = theEvent->where.v - environ->vZero;

	GetPort(&remPort);
	SetPort(environ->theWindow);
	GlobalToLocal(&where);					// convert global to local coordinates
	
	if (PtInRect(where, &(environ->tools.tools)))				// click in tools?
	{
		if (PtInRect(where, &(environ->tools.title)))			// drag tools?
		{
			oldRect.left = environ->tools.tools.left;
			oldRect.top = environ->tools.tools.top;
			oldRect.right = environ->tools.tools.right;
			oldRect.bottom = environ->tools.tools.bottom;
			if (NOTES_DragRect(environ, &(environ->tools.tools), where, 1.0, 1.0, NULL, NULL))
			{
				NOTES_EnvironCalcRects(environ);
				NOTES_InvalRects(environ, &oldRect, &(environ->tools.tools));
			}
		}
		
		else if (PtInRect(where, &(environ->tools.new)))		// new note
		{
			if (NOTES_TrackRect(environ, &(environ->tools.new)))
			{
				NOTES_NewNote(environ, &note);
				environ->dirty = TRUE;
			}
		}
		
		else if (PtInRect(where, &(environ->tools.delete)))		// delete note
		{
			if (environ->editNote)
			{
				if (NOTES_TrackRect(environ, &(environ->tools.delete)))
				{
					NOTES_DisposeNote((Note *)(environ->editNote));
					NOTES_StartEditNote((Note *)(environ->editNote));
					environ->dirty = TRUE;
				}
			}
		}

		else if (PtInRect(where, &(environ->tools.font)))		// text font
		{
			if (NOTES_SelectFont(environ, where))
				NOTES_UpdateEditedNote((Note *)(environ->editNote));
		}

		else if (PtInRect(where, &(environ->tools.size)))		// text font size
		{
			if (NOTES_SelectFontSize(environ, where))
				NOTES_UpdateEditedNote((Note *)(environ->editNote));
		}

		else if (PtInRect(where, &(environ->tools.left)))		// text justify left
		{
			if (NOTES_SetTools(environ, environ->font, NULL,
					environ->size, environ->style, 0))
			{
				NOTES_UpdateEditedNote((Note *)(environ->editNote));
			}
		}

		else if (PtInRect(where, &(environ->tools.center)))	// text justify center
		{
			if (NOTES_SetTools(environ, environ->font, NULL,
					environ->size, environ->style, 1))
			{
				NOTES_UpdateEditedNote((Note *)(environ->editNote));
			}
		}

		else if (PtInRect(where, &(environ->tools.right)))		// text justify right
		{
			if (NOTES_SetTools(environ, environ->font, NULL,
					environ->size, environ->style, -1))
			{
				NOTES_UpdateEditedNote((Note *)(environ->editNote));
			}
		}

		else if (PtInRect(where, &(environ->tools.plain)))		// text style plain
		{
			if (NOTES_SetTools(environ, environ->font, NULL,
					environ->size, 0x8000, environ->justify))
			{
				NOTES_UpdateEditedNote((Note *)(environ->editNote));
			}
		}

		else if (PtInRect(where, &(environ->tools.bold)))		// text style bold
		{
			if (NOTES_SetTools(environ, environ->font, NULL,
					environ->size, 0x8001, environ->justify))
			{
				NOTES_UpdateEditedNote((Note *)(environ->editNote));
			}
		}

		else if (PtInRect(where, &(environ->tools.italic)))	// text style italic
		{
			if (NOTES_SetTools(environ, environ->font, NULL,
					environ->size, 0x8002, environ->justify))
			{
				NOTES_UpdateEditedNote((Note *)(environ->editNote));
			}
		}

		else if (PtInRect(where, &(environ->tools.bringToFront)))	// bring to front
		{
			if (environ->editNote)
			{
				if (NOTES_TrackRect(environ, &(environ->tools.bringToFront)))
				{
					NOTES_BringToFront((Note *)(environ->editNote));
					environ->dirty = TRUE;
				}
			}
		}

		else if (PtInRect(where, &(environ->tools.sendToBack)))	// send to back
		{
			if (environ->editNote)
			{
				if (NOTES_TrackRect(environ, &(environ->tools.sendToBack)))
				{
					NOTES_SendToBack((Note *)(environ->editNote));
					environ->dirty = TRUE;
				}
			}
		}

		else if (PtInRect(where, &(environ->tools.showMargin)))	// show margins
		{
			if (!(environ->showMargin))
			{
				environ->showMargin = TRUE;
				NOTES_InvertRect(environ, &(environ->tools.showMargin));
				NOTES_InvertRect(environ, &(environ->tools.hideMargin));
			
				qPtr = environ->notes.qHead;	// force redraw of all notes
				while (qPtr != NULL)
				{
					NOTES_InvalRects(environ, NULL, &(((Note *)qPtr)->marginRect));
					qPtr = qPtr->qLink;
				}
			}
		}

		else if (PtInRect(where, &(environ->tools.hideMargin)))	// hide margins
		{
			if (environ->showMargin)
			{
				environ->showMargin = FALSE;
				NOTES_InvertRect(environ, &(environ->tools.showMargin));
				NOTES_InvertRect(environ, &(environ->tools.hideMargin));
			
				qPtr = environ->notes.qHead;	// force redraw of all notes
				while (qPtr != NULL)
				{
					NOTES_InvalRects(environ, &(((Note *)qPtr)->marginRect), &(((Note *)qPtr)->noteRect));
					qPtr = qPtr->qLink;
				}
			}
		}
		
		result = TRUE;
	}
	else
	{
										// find a note clicked in
		note = NULL;
		qPtr = environ->notes.qHead;	// check for click in all notes
		while (qPtr != NULL)
		{
			if ((environ->showMargin) && (environ->editMode))
			{
				if (PtInRect(where, &(((Note *)qPtr)->marginRect)))
					note = (Note *)qPtr;
			}
			else if (PtInRect(where, &(((Note *)qPtr)->noteRect)))
			{
				note = (Note *)qPtr;
			}
			qPtr = qPtr->qLink;
		}

		if (note)
		{								// note was clicked in
			if (note != (Note *)(environ->editNote))
			{
				NOTES_StopEditNote(note->environ);
				NOTES_StartEditNote(note);
			}
			
			else if (PtInRect(where, &(note->textRect)))
			{
				where.h += environ->hZero;
				where.v += environ->vZero;
				if (environ->hTE)
					TEClick(where, (theEvent->modifiers & shiftKey) == shiftKey, environ->hTE);
			}
			
			else if (PtInRect(where, &(note->dragRect)))
			{
				oldRect.left = note->marginRect.left;
				oldRect.top = note->marginRect.top;
				oldRect.right = note->marginRect.right;
				oldRect.bottom = note->marginRect.bottom;
				if (NOTES_DragRect(environ, &(note->marginRect), where, environ->hSize,
									environ->vSize, &(note->h), &(note->v)))
				{
					NOTES_NoteCalcRects(note);
					NOTES_UpdateEditedNote(note);
					NOTES_InvalRects(environ, &oldRect, &(note->marginRect));
				}
			}
			
			else if (PtInRect(where, &(note->growRect)))
			{
				oldRect.left = note->marginRect.left;
				oldRect.top = note->marginRect.top;
				oldRect.right = note->marginRect.right;
				oldRect.bottom = note->marginRect.bottom;
				if (NOTES_GrowRect(environ, &(note->marginRect), where, environ->hSize,
									environ->vSize, &(note->width), &(note->height)))
				{
					NOTES_NoteCalcRects(note);
					NOTES_UpdateEditedNote(note);
					NOTES_InvalRects(environ, &oldRect, &(note->marginRect));
				}
			}

			else if (PtInRect(where, &(note->noteRect)))
			{
				if (where.h < note->textRect.left)
					NOTES_NewArrow(note, where, ARROW_LEFT);
				else if (where.v < note->textRect.top)
					NOTES_NewArrow(note, where, ARROW_TOP);
				else if (where.h > note->textRect.right)
					NOTES_NewArrow(note, where, ARROW_RIGHT);
				else if (where.v > note->textRect.bottom)
					NOTES_NewArrow(note, where, ARROW_BOTTOM);
				NOTES_InvalRects(environ, &(note->marginRect), &(note->noteRect));
				environ->dirty = TRUE;
			}
			
			result = TRUE;
		}
	}

	SetPort(remPort);
	
	return result;
} // NOTES_DoClick



/*****************************************************************
 *
 * NOTES_DoCut
 *
 *  -> theWindow		pointer to active window
 *
 * Returns TRUE if Cut was processed.
 * Returns FALSE if theWindow is not the active window (see
 * NOTES_Activate), or if there is no note environ for the
 * window, or the notes for the window's environ are not
 * currently visible (see NOTES_Visible), or the environ is
 * not in edit mode (see NOTES_EditMode).
 * Cuts text currently selected from a note to the clipboard.
 *
 *****************************************************************/
 
Boolean NOTES_DoCut(WindowPtr theWindow)
{
	short		oldLength;
	short		newLength;
	
	if ((gActiveEnviron) && (gActiveEnviron->theWindow == theWindow) &&
		(gActiveEnviron->editMode) && (gActiveEnviron->hTE))
	{
		oldLength = (**(gActiveEnviron->hTE)).teLength;
		
		TECut(gActiveEnviron->hTE);
		if (ZeroScrap() == noErr)
			TEToScrap();
			
		newLength = (**(gActiveEnviron->hTE)).teLength;
		NOTES_EnoughStorage(gActiveEnviron, oldLength, newLength);

		gActiveEnviron->dirty = TRUE;
		
		return TRUE;
	}
	return FALSE;
} // NOTES_DoCut



/*****************************************************************
 *
 * NOTES_DoCopy
 *
 *  -> theWindow		pointer to active window
 *
 * Returns TRUE if Copy was processed.
 * Returns FALSE if theWindow is not the active window (see
 * NOTES_Activate), or if there is no note environ for the
 * window, or the notes for the window's environ are not
 * currently visible (see NOTES_Visible), or the environ is
 * not in edit mode (see NOTES_EditMode).
 * Copys text currently selected from a note to the clipboard.
 *
 *****************************************************************/
 
Boolean NOTES_DoCopy(WindowPtr theWindow)
{
	if ((gActiveEnviron) && (gActiveEnviron->theWindow == theWindow) &&
		(gActiveEnviron->editMode) && (gActiveEnviron->hTE))
	{
		TECopy(gActiveEnviron->hTE);
		if (ZeroScrap() == noErr)
			TEToScrap();
			
		return TRUE;
	}
	return FALSE;
} // NOTES_DoCopy



/*****************************************************************
 *
 * NOTES_DoPaste
 *
 *  -> theWindow		pointer to active window
 *
 * Returns TRUE if Paste was processed.
 * Returns FALSE if theWindow is not the active window (see
 * NOTES_Activate), or if there is no note environ for the
 * window, or the notes for the window's environ are not
 * currently visible (see NOTES_Visible), or the environ is
 * not in edit mode (see NOTES_EditMode).
 * Pastes text from the clipboard to a note currently
 * being edited.
 *
 *****************************************************************/
 
Boolean NOTES_DoPaste(WindowPtr theWindow)
{
	short		pasteOverLength;
	short		pasteInLength;
	
	if ((gActiveEnviron) && (gActiveEnviron->theWindow == theWindow) &&
		(gActiveEnviron->editMode) && (gActiveEnviron->hTE))
	{
		pasteOverLength = (**(gActiveEnviron->hTE)).selEnd - (**(gActiveEnviron->hTE)).selStart;
		pasteInLength = TEGetScrapLen();
		
		if (!NOTES_EnoughStorage(gActiveEnviron, pasteOverLength, pasteInLength))
			return TRUE;

		if (TEFromScrap() == noErr)
			TEPaste(gActiveEnviron->hTE);
			
		gActiveEnviron->dirty = TRUE;
		
		return TRUE;
	}
	return FALSE;
} // NOTES_DoPaste



/*****************************************************************
 *
 * NOTES_DoClear
 *
 *  -> theWindow		pointer to active window
 *
 * Returns TRUE if Clear was processed.
 * Returns FALSE if theWindow is not the active window (see
 * NOTES_Activate), or if there is no note environ for the
 * window, or the notes for the window's environ are not
 * currently visible (see NOTES_Visible), or the environ is
 * not in edit mode (see NOTES_EditMode).
 * Clears text currently selected from a note to the clipboard.
 *
 *****************************************************************/
 
Boolean NOTES_DoClear(WindowPtr theWindow)
{
	short		oldLength;
	short		newLength;

	if ((gActiveEnviron) && (gActiveEnviron->theWindow == theWindow) &&
		(gActiveEnviron->editMode) && (gActiveEnviron->hTE))
	{
		oldLength = (**(gActiveEnviron->hTE)).teLength;

		TEDelete(gActiveEnviron->hTE);

		newLength = (**(gActiveEnviron->hTE)).teLength;
		NOTES_EnoughStorage(gActiveEnviron, oldLength, newLength);

		gActiveEnviron->dirty = TRUE;
		
		return TRUE;
	}
	return FALSE;
} // NOTES_DoClear



/*****************************************************************
 *
 * NOTES_DoKey
 *
 *  -> theWindow		pointer to active window
 *  -> theEvent			pointer to key event record
 *
 * Returns TRUE if the key was processed. Call this in
 * response to a key event in a window with a note
 * environ.
 * Returns FALSE if theWindow is not the active window (see
 * NOTES_Activate), or if there is no note environ for the
 * window, or the notes for the window's environ are not
 * currently visible (see NOTES_Visible), or the environ is
 * not in edit mode (see NOTES_EditMode).
 *
 *****************************************************************/
 
Boolean NOTES_DoKey(WindowPtr theWindow, EventRecord *theEvent)
{
	char		key;
	short		oldLength;
	short		newLength;
	
	if ((gActiveEnviron) && (gActiveEnviron->theWindow == theWindow) &&
		(gActiveEnviron->editMode) && (gActiveEnviron->hTE))
	{
		key = theEvent->message & charCodeMask;
		
		if (theEvent->modifiers & cmdKey)
		{
			switch (key)
			{
				case 'X':
				case 'x':
					return NOTES_DoCut(theWindow);
				case 'C':
				case 'c':
					return NOTES_DoCopy(theWindow);
				case 'V':
				case 'v':
					return NOTES_DoPaste(theWindow);
				default:
					return FALSE;
			}
		}

		switch (key)
		{
			case 0x7F:			// delete forward
				key = 0x08;
				if (((**(gActiveEnviron->hTE)).selEnd - (**(gActiveEnviron->hTE)).selStart == 0) &&
					((**(gActiveEnviron->hTE)).selStart + 1 <= (**(gActiveEnviron->hTE)).teLength))
				{
					TESetSelect((**(gActiveEnviron->hTE)).selStart, (**(gActiveEnviron->hTE)).selStart + 1, gActiveEnviron->hTE);
				}
				else
					return TRUE;
				break;
			
			case 0x09:			// tab to next note in list
				NOTES_StopEditNote(gActiveEnviron);
				if (((Note *)(gActiveEnviron->editNote))->qLink)
					NOTES_StartEditNote((Note *)(((Note *)(gActiveEnviron->editNote))->qLink));
				else
					NOTES_StartEditNote((Note *)(gActiveEnviron->notes.qHead));
				return TRUE;
			
			case 0x03:			// ignore enter
				return TRUE;
				
			default:
				break;
		}
		
		oldLength = (**(gActiveEnviron->hTE)).teLength;

		TEKey(key, gActiveEnviron->hTE);
		
		newLength = (**(gActiveEnviron->hTE)).teLength;
		if (!NOTES_EnoughStorage(gActiveEnviron, oldLength, newLength))
		{
			TESetSelect((**(gActiveEnviron->hTE)).selEnd - (newLength - oldLength), (**(gActiveEnviron->hTE)).selEnd, gActiveEnviron->hTE);
			TEDelete(gActiveEnviron->hTE);
		}
		
		gActiveEnviron->dirty = TRUE;

		return TRUE;
	}
	return FALSE;
} // NOTES_DoKey



/*****************************************************************
 *
 * NOTES_GetChanged
 *
 *  -> environ		pointer to note environment
 *
 * Returns TRUE if at least one change has been made
 * to a note in the environ since the last call to 
 * NOTES_NewEnviron, NOTES_SetList, or NOTES_SetChanged.
 * This may be useful to know whether or not you need to
 * call NOTES_GetList and save the note environment.
 *
 *****************************************************************/

Boolean NOTES_GetChanged(NoteEnviron *environ)
{
	return environ->dirty;
} // NOTES_GetChanged



/*****************************************************************
 *
 * NOTES_SetChanged
 *
 *  -> environ		pointer to note environment
 *  -> changedFlag	TRUE or FALSE
 *
 * Sets the internal "dirty" flag to TRUE or FALSE.
 * NOTES_GetChanged will subsequently return this value.
 *
 *****************************************************************/

void NOTES_SetChanged(NoteEnviron *environ, Boolean changedFlag)
{
	environ->dirty = changedFlag;
} // NOTES_SetChanged



/*****************************************************************
 *
 * NOTES_Count
 *
 *  -> environ		pointer to note environment
 *
 * Returns number of notes in the environ.
 * This may be useful to know whether to enable or disable
 * an option to make the notes visible.
 *
 *****************************************************************/

short NOTES_Count(NoteEnviron *environ)
{
	QElemPtr		qPtr;
	short			count = 0;
	
	qPtr = environ->notes.qHead;		// count all notes
	while (qPtr != NULL)
	{
		count++;
		qPtr = qPtr->qLink;
	}
	return count;
} // NOTES_Count



/*****************************************************************
 *
 * NOTES_GetList
 *
 *  -> environ			pointer to note environment
 *  -> compressedList	pointer to compressed data describing
 *						the notes in environment, on entry if
 *						*compressedList is NULL it will be
 *						allocated		
 *
 * Returns 0 or an error code.
 * Obtains a compressed copy of the environ for saving and 
 * later restoring via NOTES_SetList.
 *
 *****************************************************************/

short NOTES_GetList(NoteEnviron *environ, CompressedListPtr *compressedList)
{
	QElemPtr			qPtr;
	unsigned char		*data;
	CompressedNote		cNote;
	Note				*note;
	short				loop;
	
	if (*compressedList == NULL)
	{
		*compressedList = (CompressedList *)NewPtr(environ->usedStorage);
		if (*compressedList == NULL)
			return MemError();
	}
	
	NOTES_StopEditNote(environ);

	data = (unsigned char *)(*compressedList);
	
	(*compressedList)->version = 2;
	(*compressedList)->numOfNotes = 0;
	(*compressedList)->unused = 0L;
	(*compressedList)->storage = environ->usedStorage;
	
	data += sizeof(CompressedList);
	
	qPtr = environ->notes.qHead;		// save all notes
	while ((note = (Note *)qPtr) != NULL)
	{
		cNote.h = note->h;				// copy note info to compressed record
		cNote.v = note->v;
		cNote.width = note->width;
		cNote.height = note->height;
		cNote.size = note->size;
		cNote.style = note->style;
		cNote.justify = note->justify;
		for(loop = 0; loop < NOTE_NUM_ARROWS; loop++)
		{
			cNote.leftArrows[loop] = note->leftArrows[loop];
			cNote.topArrows[loop] = note->topArrows[loop];
			cNote.rightArrows[loop] = note->rightArrows[loop];
			cNote.bottomArrows[loop] = note->bottomArrows[loop];
		}
		cNote.orig_hSize = note->orig_hSize;
		cNote.orig_vSize = note->orig_vSize;
		cNote.fontNameLength = note->fontName[0];
		cNote.textLength = note->length;
		
										// copy compressed record to compressedList
		BlockMove((Ptr)&cNote, (Ptr)data, sizeof(CompressedNote));
		data += sizeof(CompressedNote);
		
										// copy font name to compressedList
		BlockMove((Ptr)&(note->fontName[1]), (Ptr)data, note->fontName[0]);
		data += note->fontName[0];
		
										// copy text to compressedList
		BlockMove((Ptr)(note->text), (Ptr)data, note->length);
		data += note->length;

		((*compressedList)->numOfNotes)++;
		
		qPtr = qPtr->qLink;
	}
	
	NOTES_StartEditNote((Note *)(environ->editNote));

	return noErr;
} // NOTES_GetList



/*****************************************************************
 *
 * NOTES_ClearList
 *
 *  -> environ			pointer to note environment
 *
 * Removes all notes from environ.
 *
 *****************************************************************/

void NOTES_ClearList(NoteEnviron *environ)
{
	QElemPtr		qPtr;
	QElemPtr		nextqPtr;
	
	nextqPtr = environ->notes.qHead;		// dispose of notes in this noteEnviron
	while ((qPtr = nextqPtr) != NULL)
	{
		nextqPtr = qPtr->qLink;
		NOTES_DisposeNote((Note *)qPtr);
	}
	
	environ->dirty = TRUE;
} // NOTES_ClearList




/*****************************************************************
 *
 * NOTES_SetList
 *
 *  -> environ			pointer to note environment
 *  -> compressedList	pointer to compressed data describing
 *						the notes in environment
 *
 * Returns 0 or an error code.
 * Calls NOTES_ClearList and installs the notes defined
 * by compressedList. compressedList should be a value
 * previously obtained from NOTES_GetList.
 *
 *****************************************************************/

short NOTES_SetList(NoteEnviron *environ, CompressedList *compressedList)
{
	unsigned char		*data;
	CompressedNote		cNote;
	Note				*note = NULL;
	short				noteLoop;
	short				loop;
	short				err;
	
	NOTES_ClearList(environ);
	
	data = (unsigned char *)(compressedList);

	environ->usedStorage = compressedList->storage;

	data += sizeof(CompressedList);
	
	for(noteLoop = 0; noteLoop < compressedList->numOfNotes; noteLoop++)
	{
										// copy next compressed record out of compressedList
		BlockMove((Ptr)data, (Ptr)&cNote, sizeof(CompressedNote));
		data += sizeof(CompressedNote);

		note = (Note *)NewPtr(sizeof(Note));	// allocate a new note
		if (note == NULL)
			return MemError();
		
		note->environ = environ;				// copy compressed record to note
		note->text = (unsigned char *)NewPtr(cNote.textLength);
		if (note->text == NULL)
		{
			err = MemError();
			DisposPtr((Ptr)note);
			return err;
		}
		note->length = cNote.textLength;
		
		note->h = cNote.h;
		note->v = cNote.v;
		note->width = cNote.width;
		note->height = cNote.height;
		note->orig_hSize = (compressedList->version > 1 ? cNote.orig_hSize : environ->hSize);
		note->orig_vSize = (compressedList->version > 1 ? cNote.orig_vSize : environ->vSize);
		NOTES_NoteCalcRects(note);
		note->size = cNote.size;
		note->style = cNote.style;
		note->justify = cNote.justify;
		for(loop = 0; loop < NOTE_NUM_ARROWS; loop++)
		{
			note->leftArrows[loop] = cNote.leftArrows[loop];
			note->topArrows[loop] = cNote.topArrows[loop];
			note->rightArrows[loop] = cNote.rightArrows[loop];
			note->bottomArrows[loop] = cNote.bottomArrows[loop];
		}
	
										// copy font name out of compressedList
		note->fontName[0] = cNote.fontNameLength;
		BlockMove((Ptr)data, (Ptr)&(note->fontName[1]), note->fontName[0]);
		GetFNum(note->fontName, &(note->font));
		data += note->fontName[0];
	
										// copy text out of compressedList
		BlockMove((Ptr)data, (Ptr)(note->text), note->length);
		data += note->length;
		
		if (environ->visible)
			NOTES_InvalRects(environ, NULL, &(note->marginRect));

		Enqueue((QElemPtr)note, &(environ->notes));
	}
	
	NOTES_StartEditNote(note);
	
	environ->dirty = FALSE;

	return noErr;
} // NOTES_SetList
