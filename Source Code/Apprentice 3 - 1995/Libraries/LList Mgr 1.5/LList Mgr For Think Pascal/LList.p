{************************************************************}
{*															*}
{* LList.p													*}
{*															*}
{* Version 1.5, June 1994, added LLSize function,			*}
{*						   added type to colDesc record,	*}
{*						   enhanced LLSort to support		*}
{*						   ascii and numeric types			*}
{*						   and use a quick sort algorithm	*}
{* Version 1.4, May 1994, added ignoreCase to LLSearch,		*}
{*						  added drawNow to LLDoDraw,		*}
{*						  added LLSort function				*}
{* Version 1.3, April 1994, fixed bug w/no scroll bar,		*}
{*							LLDoDraw(theList, 1) no longer 	*}
{*							auto-redraws the list so call 	*}
{*							LLUpdate if necessary			*}
{* Version 1.2, December 1993, added column frame option,	*}
{*							   condenses type if needed,	*}
{*							   added LLRect function,		*}
{*							   fixed LLAddRow redraw bug	*}
{* Version 1.1, November 1993, fixed thumb scrolling bug	*}
{* Version 1.0, September 1993								*}
{* Tad Woods, T&T Software, 70312,3552 on CompuServe		*}
{*															*}
{* LList is an alternative list manager to the standard		*}
{* toolbox list manager. The advantages of LList are the	*}
{* abilities to have variable width	columns, a different 	*}
{* text style for each column, and lists larger than 32K.	*}
{* Unlike the standard list manager, LList only	scrolls		*}
{* vertically, requires a fixed number of columns to		*}
{* be set when the list is created via LLNew, and entire	*}
{* rows, not individual cells, can be highlighted.			*}
{* LList has about the same performance and the function	*}
{* calls are similar to the toolbox list manager.			*}
{* LLists may contain up to 32767 rows, 32767 columns, 		*}
{* and individual cells may contain data up to 32K.			*}
{* LList is pointer (not handle) based.						*}
{*															*}
{* LList has been tested and used but comes with no			*}
{* guarantees. You may include LList with your own			*}
{* compiled code. You may not sell or distribute LList		*}
{* source code without permission of T&T Software.			*}
{* Send comments or problem reports to the address above.	*}
{*															*}
{************************************************************}

unit LList;

interface

	const
{Values for selFlags}
		LLOnlyOne = -128;		{only one cell can be selected at a time}
		LLExtendDrag = 64;	{drag extends without shift key down}
		LLNoDisjoint = 32;	{click deselects all previous selections}
		LLNoExtend = 16;		{shift won't extend selection}
		LLUseSense = 4;		{shift senses state of initial cell}
		LLNoNilHilite = 2;	{don't highlight empty rows}

	type
		LColDescPtr = ^LColDescRec;
		LColDescRec = packed record	{Description of how to draw a column}
				width: integer;		{width in pixels, default is list width div by number of cols}
				style: integer;		{TextFace values, default is plain}
				justify: integer;	{0=left justify in cell (default), 1=center, -1=right}
				dodraw: integer;	{1=draw column (default), 0=don't draw column}
				highlight: integer;	{1=highlight column (default), 0=don't highlight column}
				frame: integer;
		{$0008 frame left, $0004 right, $0002 top, $0001 bottom, $000F all}
				numeric: integer;	{0=sort column as ascii text (default), 1=sort column as numeric data}
			end;

		LColPtr = ^LColRec;	{Column data}
		LColRec = packed record
				data: Ptr;
				dataLen: integer;
			end;

		LRowPtr = ^LRowRec;	{Row data}
		LRowRec = packed record
				nextRow: LRowPtr;
				prevRow: LRowPtr;
				needDraw: integer;
				selected: integer;
				col: array[0..999] of LColRec;
			end;

		LListPtr = ^LListRec;
		LListRec = packed record
				view: Rect;				{display rectangle in local coords}
				window: WindowPtr;		{window where list resides}
				scroll: ControlHandle;	{ControlRecord for scroll bar}
				font: integer;			{list font}
				size: integer;			{list font size}
				height: integer;		{list line height in pixels}
				baseline: integer;		{pixels from bottom of text to bottom of row}
				indent: integer;		{pixels to indent from edge of column}
				dodraw: integer;
				selFlags: char;			{determines how selection is performed with mouse}
				activeFlag: char;
				row: LRowPtr;			{pointer to first LRow}
				lastClkRow: LRowPtr;	{pointer to last row clicked in}
				lastClkTime: longint;
				numOfRows: integer;			{number of rows in list}
				firstVisRowNum: integer;	{number of first visible row in list}
				numOfVisRows: integer;		{number of visible rows in view Rect}
				numOfColumns: integer;		{number of columns in list}
				refCon: longint;	{for your use}
				colDesc: array[0..999] of LColDescRec;
			end;

{************************************************************}
{*															*}
{* LLActivate												*}
{*															*}
{*  -> theList		pointer to LList						*}
{*  -> activate		1=activate, 0=deactivate				*}
{*															*}
{* Activates or deactivates a LList. Call after an			*}
{* activate event.											*}
{*															*}
{************************************************************}
	procedure LLActivate (theList: LListPtr; activate: integer);


{************************************************************}
{*															*}
{* LLAddRow			returns pointer to new row or NULL		*}
{*					(returns NULL if there's not enough 	*}
{*					memory)									*}
{*															*}
{*  -> theList		pointer to LList						*}
{*  -> beforeRow	pointer to row that new row will come	*}
{*					before									*}
{*															*}
{* Adds a new row to a LList. If beforeRow = NULL a row		*}
{* will be added to the end of the list.					*}
{*															*}
{************************************************************}
	function LLAddRow (theList: LListPtr; beforeRow: LRowPtr): LRowPtr;


{************************************************************}
{*															*}
{* LLClick			returns 1 if user double-clicks in 		*}
{*					list otherwise returns 0				*}
{*															*}
{*  -> theList		pointer to LList						*}
{*  -> localPoint	click location in local coordinates		*}
{*  -> modifiers	shift, cmd, etc. from event record		*}
{*															*}
{* Processes mouse-down for list dragging and selection.	*}
{* Call LLClick when a mousedown event occurs inside the	*}
{* list view area of a window. LLClick performs row 		*}
{* selection according to criteria established by the		*}
{* bits of LList.selFlags.									*}
{*															*}
{************************************************************}
	function LLClick (theList: LListPtr; localPoint: Point; modifiers: integer): integer;


{************************************************************}
{*															*}
{* LLDelRow													*}
{*															*}
{*  -> theList		pointer to LList						*}
{*  -> row			pointer to list row, or NULL to delete	*}
{*					all rows								*}
{*															*}
{* Deletes the row and releases all of its associated 		*}
{* memory.													*}
{*															*}
{************************************************************}
	procedure LLDelRow (theList: LListPtr; row: LRowPtr);


{************************************************************}
{*															*}
{* LLDispose												*}
{*															*}
{*  -> theList		pointer to LList						*}
{*															*}
{* Disposes of a LList and release all of its memory.		*}
{*															*}
{************************************************************}
	procedure LLDispose (theList: LListPtr);


{************************************************************}
{*															*}
{* LLDoDraw													*}
{*															*}
{*  -> theList		pointer to LList						*}
{*  -> dodraw		1=changes to LList are displayed as 	*}
{*					they occur, 0=list will not be 			*}
{*					updated until drawing is turned back on	*}
{*  -> drawNow		1=draw list items that need drawing		*}
{*					immediately, 0=do not draw now (call	*}
{*					LLUpdate to force the list to redraw).	*}
{*					Note: drawNow is ignored if dodraw = 0.	*}
{*															*}
{* Turns list drawing normally performed by other LList 	*}
{* Manager functions on or off.								*}
{*															*}
{************************************************************}
	procedure LLDoDraw (theList: LListPtr; dodraw: integer; drawNow: integer);


{************************************************************}
{*															*}
{* LLGetCell												*}
{*															*}
{*  -> theList		pointer to LList						*}
{*  -> row			pointer to list row						*}
{*  -> colNum		column number of cell in row			*}
{*					(column numbers start at 0)				*}
{* <-> dataLen		on entry size in bytes of buffer at		*}
{*					data, on return actual length of data	*}
{*					transferred								*}
{*  -> data			pointer to buffer, on return this 		*}
{*					buffer contains the data from the cell	*}
{*															*}
{* Get a copy of a cell's data.								*}
{*															*}
{************************************************************}
	procedure LLGetCell (theList: LListPtr; row: LRowPtr; colNum: integer; var dataLen: integer; data: Ptr);


{************************************************************}
{*															*}
{* LLGetSelect		returns true if a selected row is 		*}
{*					found, false if not						*}
{*															*}
{*  -> theList		pointer to LList						*}
{* <-> row			pointer to list row	pointer				*}
{*					on entry pointer to a row to test		*}
{*					first, on return pointer to a selected	*}
{*					row if a selected row was found			*}
{*  -> advanceit	1=examine one row,0=keep looking		*}
{*															*}
{* Queries if a row is selected; gets the next selected		*}
{* row if advanceit = true. Note that in searching for all	*}
{* selections in a list, you will need to advance row		*}
{* (use LLNextRow) after a selection is found.				*}
{*															*}
{************************************************************}
	function LLGetSelect (theList: LListPtr; var row: LRowPtr; advanceit: integer): integer;


{************************************************************}
{*															*}
{* LLNew			returns a pointer to a new LList		*}
{*					(returns NULL if there's not enough 	*}
{*					memory)									*}
{*															*}
{*  -> view			pointer to list display	rectangle in	*}
{*					local coordinates						*}
{*  -> window		pointer to window where list resides	*}
{*  -> rowHeight	height of a row in pixels				*}
{*					(if you change LList.height after the	*}
{*					LLNew call you also need to recalcualte	*}
{*					LList.numOfVisRows)						*}
{*  -> numOfColumns	number of columns in the list			*}
{*					(5 columns would be numbered 0 thru 4)	*}
{*  -> hasScroll	1=list has vertical scroll bar, 		*}
{*					0=no scroll bar							*}
{*  -> selFlags		determines how selection of rows is 	*}
{*					processed in LLClick calls				*}
{*					See Values for selFlags above.			*}
{*															*}
{* Allocates and initializes a LList record.				*}
{*															*}
{************************************************************}
	function LLNew (var view: Rect; window: WindowPtr; rowHeight: integer; numOfColumns: integer; hasScroll: integer; selFlags: integer): LListPtr;


{************************************************************}
{*															*}
{* LLNextRow		returns a pointer to next row			*}
{*					returns NULL if no next row				*}
{*															*}
{*  -> theList		pointer to LList						*}
{*  -> row			pointer to a row						*}
{*															*}
{* Returns a pointer to the row after row. Pass NULL for	*}
{* row and LLNextRow will return a pointer to the first		*}
{* row in the list.											*}
{*															*}
{************************************************************}
	function LLNextRow (theList: LListPtr; row: LRowPtr): LRowPtr;


{************************************************************}
{*															*}
{* LLPrevRow		returns a pointer to previous row		*}
{*					returns NULL if no previous row			*}
{*															*}
{*  -> theList		pointer to LList						*}
{*  -> row			pointer to a row						*}
{*															*}
{* Returns a pointer to the row before row. Pass NULL for	*}
{* row and LLPrevRow will return a pointer to the last		*}
{* row in the list.											*}
{*															*}
{************************************************************}
	function LLPrevRow (theList: LListPtr; row: LRowPtr): LRowPtr;


{************************************************************}
{*															*}
{* LLRect													*}
{*															*}
{*  -> theList		pointer to LList						*}
{*  -> row			pointer to a row						*}
{*  -> colFirst		first column to contain rectangle		*}
{*  -> colLast		last column to contain rectangle		*}
{* <-  theRect		local coordindates of rectangle that	*}
{*					encloses the specified row and columns	*}
{*															*}
{* Obtains the local coordinates of the rectangle that		*}
{* encloses the specified row and column(s).				*}
{*															*}
{************************************************************}
	procedure LLRect (theList: LListPtr; row: LRowPtr; colFirst: integer; colLast: integer; var theRect: Rect);


{************************************************************}
{*															*}
{* LLScroll													*}
{*															*}
{*  -> theList		pointer to LList						*}
{*  -> rows			<0 scroll list down specific number of	*}
{*					rows, >0 scroll list up specific 		*}
{*					number of rows							*}
{*															*}
{* Scrolls the list a specific number of rows relative		*}
{* to the current displayed rows.							*}
{*															*}
{************************************************************}
	procedure LLScroll (theList: LListPtr; rows: integer);


{************************************************************}
{*															*}
{* LLScrollToRow											*}
{*															*}
{*  -> theList		pointer to LList						*}
{*  -> row			pointer to a row						*}
{*															*}
{* Scrolls the list so that row is the first row displayed.	*}
{* If row is NULL the list scrolls to the bottom.			*}
{*															*}
{************************************************************}
	procedure LLScrollToRow (theList: LListPtr; row: LRowPtr);


{************************************************************}
{*															*}
{* LLSearch			returns 1 if data is found, 0 if not	*}
{*															*}
{*  -> theList		pointer to LList						*}
{* <-> row			pointer to pointer to list row			*}
{*					on entry row to start search at or NULL	*}
{*					to start search at first row, on return	*}
{*					row where data was found if found		*}
{* <-  colFound		on return pointer to column number of	*}
{*					row where data was found, if found		*}
{*  -> colFirst		colFirst and colLast define the range	*}
{*  -> colLast		of columns to search in					*}
{*  -> data			pointer to buffer containing data to	*}
{*					search for								*}
{*  -> dataLen		length of buffer						*}
{*  -> ignoreCase	1 = comparison is case-insensitive,		*}
{*					0 = comparison is case-sensitive		*}
{*															*}
{* Searches all columns of all rows beginning at *row 		*}
{* for data. 												*}
{*															*}
{************************************************************}
	function LLSearch (theList: LListPtr; var row: LRowPtr; var colFound: integer; colFirst: integer; colLast: integer; data: Ptr; dataLen: integer; ignoreCase: integer): integer;


{************************************************************}
{*															*}
{* LLSetCell												*}
{*															*}
{*  -> theList		pointer to LList						*}
{*  -> row			pointer to list row						*}
{*  -> colNum		column number of cell in row that will 	*}
{*					receive data (col. nums. start at 0)	*}
{*  -> dataLen		length of buffer						*}
{*  -> data			pointer to buffer containing data to	*}
{*					be placed in the cell					*}
{*															*}
{* Stores a copy of data into a cell.						*}
{* (If there's not enough memory to copy data into this		*}
{* cell, the cell will be empty.)							*}
{* To clear a cell, pass 0 and nil for dataLen and data.	*}
{*															*}
{************************************************************}
	procedure LLSetCell (theList: LListPtr; row: LRowPtr; colNum: integer; dataLen: integer; data: Ptr);


{************************************************************}
{*															*}
{* LLSetSelect												*}
{*															*}
{*  -> theList		pointer to LList						*}
{*  -> row			pointer to list row	to select or		*}
{*					deselect								*}
{*  -> setit		1=select, 0=deselect					*}
{*															*}
{* Selects or deselects a cell.								*}
{*															*}
{************************************************************}
	procedure LLSetSelect (theList: LListPtr; row: LRowPtr; setit: integer);


{************************************************************}
{*															*}
{* LLSize													*}
{*															*}
{*  -> theList		pointer to LList						*}
{*  -> listWidth	new width								*}
{*  -> listHeight	new height								*}
{*															*}
{* Changes the size of the list display rectangle, updates	*}
{* the scroll bar, and redraws the list appropriately.		*}
{* If drawing is off, LLsize will cause an update event		*}
{* to redraw the invalid regions.							*}
{* If you change the listWidth, you must make any necessary	*}
{* column width adjustments yourself.						*}
{*															*}
{************************************************************}
procedure LLSize(theList: LListPtr; listWidth: integer; listHeight: integer);


{************************************************************}
{*															*}
{* LLSort													*}
{*															*}
{*  -> theList		pointer to LList						*}
{*  -> col1			sort by data field of this column		*}
{*					number									*}
{*  -> col2			sort rows with the same col1 by data	*}
{*					field of this column number, or -1 for	*}
{*					no second or third sort field			*}
{*  -> col3			sort rows with the same col2 by data	*}
{*					field of this column number, or -1 for	*}
{*					no third sort field						*}
{*  -> ignoreCase	1 = comparison is case-insensitive,		*}
{*					0 = comparison is case-sensitive		*}
{*  -> order		1 = sort order is ascending,			*}
{*					-1 = sort order is descending			*}
{*															*}
{* Sorts and re-draws a list. 								*}
{* The data in rows is compared by ascii value unless the	*}
{* numeric flag in the colDesc record for the sort column 	*}
{* is 1, in which case the comparison is by numeric value.	*}
{* When sorting by numeric value, any characters other 		*}
{* than numbers, minuses, and pluses are stripped before 	*}
{* comparison.												*}
{*															*}
{************************************************************}
	procedure LLSort (theList: LListPtr; col1: integer; col2: integer; col3: integer; ignoreCase: integer; order: integer);


{************************************************************}
{*															*}
{* LLUpdate													*}
{*															*}
{*  -> theList		pointer to LList						*}
{*															*}
{* Redraws the list. Call LLUpdate in response to an update	*}
{* event in the list's window.								*}
{*															*}
{************************************************************}
	procedure LLUpdate (theList: LListPtr);


implementation
end. {LList}