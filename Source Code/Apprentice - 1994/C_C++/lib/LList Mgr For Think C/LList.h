/************************************************************/
/*															*/
/* LList.h													*/
/*															*/
/* Version 1.3, April 1994, fixed bug w/no scroll bar,		*/
/*							LLDoDraw(theList, 1) no longer 	*/
/*							auto-redraws the list so call 	*/
/*							LLUpdate if necessary			*/
/* Version 1.2, December 1993, added column frame option,	*/
/*							   condenses type if needed,	*/
/*							   added LLRect function,		*/
/*							   fixed LLAddRow redraw bug	*/
/* Version 1.1, November 1993, fixed thumb scrolling bug	*/
/* Version 1.0, September 1993								*/
/* Tad Woods, T&T Software, 70312,3552 on CompuServe		*/
/*															*/
/* LList is an alternative list manager to the standard		*/
/* toolbox list manager. The advantages of LList are the	*/
/* abilities to have variable width	columns, a different 	*/
/* text style for each column, and lists larger than 32K.	*/
/* Unlike the standard list manager, LList only	scrolls		*/
/* vertically, requires a fixed number of columns to		*/
/* be set when the list is created via LLNew, and entire	*/
/* rows, not individual cells, can be highlighted.			*/
/* LList has about the same performance and the function	*/
/* calls are similar to the toolbox list manager.			*/
/* LLists may be unlimited total size, but individual		*/
/* cells may not contain data bigger than 32K. Currently 	*/
/* LList is totally pointer based.							*/
/*															*/
/* LList has been tested and used but comes with no			*/
/* guarantees. You may include LList with your own			*/
/* compiled code. You may not sell or distribute LList		*/
/* source code without permission of T&T Software.			*/
/* Send comments or problem reports to the address above.	*/
/*															*/
/************************************************************/

#pragma once

enum {						// Values for selFlags
    LLOnlyOne = -128,		// only one cell can be selected at a time
    LLExtendDrag = 64,		// drag extends without shift key down
    LLNoDisjoint = 32,		// click deselects all previous selections
    LLNoExtend = 16,		// shift won't extend selection
    LLUseSense = 4,			// shift senses state of initial cell
    LLNoNilHilite = 2		// don't highlight empty rows
};


typedef struct {			// Description of how to draw a column
	short		width;		// width in pixels, default is list width div by number of cols
	short		style;		// TextFace values, default is plain
	short		justify;	// 0=left justify in cell (default), 1=center, -1=right
	short		dodraw;		// 1=draw column (default), 0=don't draw column
	short		highlight;	// 1=highlight column (default), 0=don't highlight column
	short		frame;		// 0x0008 frame left, 0x0004 right, 0x0002 top, 0x0001 bottom, 0x000F all
} LColDesc;


typedef struct {			// Column data
	Ptr			data;
	short		dataLen;
} LCol;


typedef struct {			// Row data
	void		*nextRow;
	void		*prevRow;
	short		needDraw;
	short		selected;
	LCol		col[];		// array[0..numOfColumns] of column data records
} LRow;


typedef struct {
	Rect			view;		// display rectangle in local coords
	WindowPtr		window;		// window where list resides
	ControlHandle	scroll;		// ControlRecord for scroll bar
	short			font;		// list font
	short			size;		// list font size
	short			height;		// list line height in pixels
	short			baseline;	// pixels from bottom of text to bottom of row
	short			indent;		// pixels to indent from edge of column
	short			dodraw;
	char			selFlags;	// determines how selection is performed with mouse
	char			activeFlag;
	LRow			*row;		// pointer to first LRow
	LRow			*lastClkRow;	// pointer to last row clicked in
	long			lastClkTime;
	short			numOfRows;		// number of rows in list
	short			firstVisRowNum;	// number of first visible row in list
	short			numOfVisRows;	// number of visible rows in view Rect
	short			numOfColumns;	// number of columns in list
	long			refCon;		// for your use
	LColDesc		colDesc[];	// array[0..numOfColumns] of column description records
} LList;


/************************************************************/
/*															*/
/* LLActivate												*/
/*															*/
/*  -> theList		pointer to LList						*/
/*  -> activate		1=activate, 0=deactivate				*/
/*															*/
/* Activates or deactivates a LList. Call after an			*/
/* activate event.											*/
/*															*/
/************************************************************/
pascal void LLActivate(LList *theList, short activate);


/************************************************************/
/*															*/
/* LLAddRow			returns pointer to new row or NULL		*/
/*					(returns NULL if there's not enough 	*/
/*					memory)									*/
/*															*/
/*  -> theList		pointer to LList						*/
/*  -> beforeRow	pointer to row that new row will come	*/
/*					before									*/
/*															*/
/* Adds a new row to a LList. If beforeRow = NULL a row		*/
/* will be added to the end of the list.					*/
/*															*/
/************************************************************/
pascal LRow *LLAddRow(LList *theList, LRow *beforeRow);


/************************************************************/
/*															*/
/* LLClick			returns 1 if user double-clicks in list	*/
/*					otherwise returns 0						*/
/*															*/
/*  -> theList		pointer to LList						*/
/*  -> localPoint	click location in local coordinates		*/
/*  -> modifiers	shift, cmd, etc. from event record		*/
/*															*/
/* Processes mouse-down for list dragging and selection.	*/
/* Call LLClick when a mousedown event occurs inside the	*/
/* list view area of a window. LLClick performs row 		*/
/* selection according to criteria established by the		*/
/* bits of LList.selFlags.									*/
/*															*/
/************************************************************/
pascal short LLClick(LList *theList, Point localPoint, short modifiers);


/************************************************************/
/*															*/
/* LLDelRow													*/
/*															*/
/*  -> theList		pointer to LList						*/
/*  -> row			pointer to list row, or NULL to delete	*/
/*					all rows								*/
/*															*/
/* Deletes the row and releases all of its associated 		*/
/* memory.													*/
/*															*/
/************************************************************/
pascal void LLDelRow(LList *theList, LRow *row);


/************************************************************/
/*															*/
/* LLDispose												*/
/*															*/
/*  -> theList		pointer to LList						*/
/*															*/
/* Dispose of a LList and release all of its memory.		*/
/*															*/
/************************************************************/
pascal void LLDispose(LList *theList);


/************************************************************/
/*															*/
/* LLDoDraw													*/
/*															*/
/*  -> theList		pointer to LList						*/
/*  -> dodraw		1=changes to LList are displayed as 	*/
/*					they occur, 0=list will not be updated	*/
/*					until drawing is turned back on			*/
/*															*/
/* Turns list drawing on or off. After turning drawing 		*/
/* back on, you must call LLUpdate to force the list to		*/
/* be displayed.											*/
/*															*/
/************************************************************/
pascal void LLDoDraw(LList *theList, short dodraw);


/************************************************************/
/*															*/
/* LLGetCell												*/
/*															*/
/*  -> theList		pointer to LList						*/
/*  -> row			pointer to list row						*/
/*  -> colNum		column number of cell in row			*/
/*					(column numbers start at 0)				*/
/* <-> dataLen		on entry size in bytes of buffer at		*/
/*					data, on return actual length of data	*/
/*					transferred								*/
/*  -> data			pointer to buffer, on return this 		*/
/*					buffer contains the data from the cell	*/
/*															*/
/* Get a copy of a cell's data.								*/
/*															*/
/************************************************************/
pascal void LLGetCell(LList *theList, LRow *row, short colNum, short *dataLen, Ptr data);


/************************************************************/
/*															*/
/* LLGetSelect		returns 1 if a selected row is 			*/
/*					found, 0 if not							*/
/*															*/
/*  -> theList		pointer to LList						*/
/* <-> row			pointer to list row	pointer				*/
/*					on entry pointer to a row to test		*/
/*					first, on return pointer to a selected	*/
/*					row if a selected row was found			*/
/*  -> advanceit	0=examine one row, 1=keep looking		*/
/*															*/
/* Queries if a row is selected; gets the next selected		*/
/* row if advanceit = 1. Note that in searching for all		*/
/* selections in a list, you will need to advance row		*/
/* (use LLNextRow) after a selection is found.				*/
/*															*/
/************************************************************/
pascal short LLGetSelect(LList *theList, LRow **row, short advanceit);


/************************************************************/
/*															*/
/* LLNew			returns a pointer to a new LList		*/
/*					(returns NULL if there's not enough 	*/
/*					memory)									*/
/*															*/
/*  -> view			pointer to list display	rectangle in	*/
/*					local coordinates						*/
/*  -> window		pointer to window where list resides	*/
/*  -> rowHeight	height of a row in pixels				*/
/*					(if you change LList.height after the	*/
/*					LLNew call you also need to recalcualte	*/
/*					LList.numOfVisRows)						*/
/*  -> numOfColumns	number of columns in the list			*/
/*					(5 columns would be numbered 0 thru 4)	*/
/*  -> hasScroll	1=list has vertical scroll bar, 0=no	*/
/*					scroll bar								*/
/*  -> selFlags		determines how selection of rows is 	*/
/*					processed in LLClick calls				*/
/*					See Values for selFlags above.			*/
/*															*/
/* Allocates and initializes a LList record.				*/
/*															*/
/************************************************************/
pascal LList *LLNew(Rect *view, WindowPtr window, short rowHeight, short numOfColumns, short hasScroll, short selFlags);


/************************************************************/
/*															*/
/* LLNextRow		returns a pointer to next row			*/
/*					returns NULL if no next row				*/
/*															*/
/*  -> theList		pointer to LList						*/
/*  -> row			pointer to a row						*/
/*															*/
/* Returns a pointer to the row after row. Pass NULL for	*/
/* row and LLNextRow will return a pointer to the first		*/
/* row in the list.											*/
/*															*/
/************************************************************/
pascal LRow *LLNextRow(LList *theList, LRow *row);


/************************************************************/
/*															*/
/* LLPrevRow		returns a pointer to previous row		*/
/*					returns NULL if no previous row			*/
/*															*/
/*  -> theList		pointer to LList						*/
/*  -> row			pointer to a row						*/
/*															*/
/* Returns a pointer to the row before row. Pass NULL for	*/
/* row and LLPrevRow will return a pointer to the last		*/
/* row in the list.											*/
/*															*/
/************************************************************/
pascal LRow *LLPrevRow(LList *theList, LRow *row);


/************************************************************/
/*															*/
/* LLRect													*/
/*															*/
/*  -> theList		pointer to LList						*/
/*  -> row			pointer to a row						*/
/*  -> colFirst		first column to contain rectangle		*/
/*  -> colLast		last column to contain rectangle		*/
/* <-  theRect		local coordindates of rectangle that	*/
/*					encloses the specified row and columns	*/
/*															*/
/* Obtains the local coordinates of the rectangle that		*/
/* encloses the specified row and column(s).				*/
/*															*/
/************************************************************/
pascal void LLRect(LList *theList, LRow *row, short colFirst, short colLast, Rect *theRect);


/************************************************************/
/*															*/
/* LLScroll													*/
/*															*/
/*  -> theList		pointer to LList						*/
/*  -> rows			<0 scroll list down specific number of	*/
/*					rows, >0 scroll list up specific 		*/
/*					number of rows							*/
/*															*/
/* Scrolls the list a specific number of rows relative		*/
/* to the current displayed rows.							*/
/*															*/
/************************************************************/
pascal void LLScroll(LList *theList, short rows);


/************************************************************/
/*															*/
/* LLScrollToRow											*/
/*															*/
/*  -> theList		pointer to LList						*/
/*  -> row			pointer to a row						*/
/*															*/
/* Scrolls the list so that row is the first row displayed.	*/
/* If row is NULL the list scrolls to the bottom.			*/
/*															*/
/************************************************************/
pascal void LLScrollToRow(LList *theList, LRow *row);


/************************************************************/
/*															*/
/* LLSearch			returns 1 if data is found, 0 if not	*/
/*															*/
/*  -> theList		pointer to LList						*/
/* <-> row			pointer to pointer to list row			*/
/*					on entry row to start search at or NULL	*/
/*					to start search at first row, on return	*/
/*					row where data was found if found		*/
/* <-  colFound		on return pointer to column number of	*/
/*					row where data was found, if found		*/
/*  -> colFirst		colFirst and colLast define the range	*/
/*  -> colLast		of columns to search in					*/
/*  -> data			pointer to buffer containing data to	*/
/*					search for								*/
/*  -> dataLen		length of buffer						*/
/*															*/
/* Searches all columns of all rows beginning at *row 		*/
/* for data. The search is case insensitive.				*/
/* (Note: LLSearch calls IUMagIDString.)					*/
/*															*/
/************************************************************/
pascal short LLSearch(LList *theList, LRow **row, short *colFound, short colFirst, short colLast, Ptr data, short dataLen);


/************************************************************/
/*															*/
/* LLSetCell												*/
/*															*/
/*  -> theList		pointer to LList						*/
/*  -> row			pointer to list row						*/
/*  -> colNum		column number of cell in row that will 	*/
/*					receive data (col. nums. start at 0)	*/
/*  -> dataLen		length of buffer						*/
/*  -> data			pointer to buffer containing data to	*/
/*					be placed in the cell					*/
/*															*/
/* Stores a copy of data into a cell.						*/
/* (If there's not enough memory to copy data into this		*/
/* cell, the cell will be empty.)							*/
/*															*/
/************************************************************/
pascal void LLSetCell(LList *theList, LRow *row, short colNum, short dataLen, Ptr data);


/************************************************************/
/*															*/
/* LLSetSelect												*/
/*															*/
/*  -> theList		pointer to LList						*/
/*  -> row			pointer to list row	to select or		*/
/*					deselect								*/
/*  -> setit		1=select, 0=deselect					*/
/*															*/
/* Selects or deselects a cell.								*/
/*															*/
/************************************************************/
pascal void LLSetSelect(LList *theList, LRow *row, short setit);


/************************************************************/
/*															*/
/* LLUpdate													*/
/*															*/
/*  -> theList		pointer to LList						*/
/*															*/
/* Redraws the list. Call LLUpdate in response to an update	*/
/* event in the list's window.								*/
/*															*/
/************************************************************/
pascal void LLUpdate(LList *theList);
