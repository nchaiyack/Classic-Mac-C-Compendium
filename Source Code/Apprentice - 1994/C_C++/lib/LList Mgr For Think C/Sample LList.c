/************************************************************/
/*															*/
/* Sample LList.c											*/
/*															*/
/* The following code demonstrates the use of an LList in 	*/
/* a modal dialog. See LList.h for documentation on each	*/
/* LList function.											*/
/*															*/
/************************************************************/

#include "LList.h"

#define DIALOG_RSRC_ID 128
#define DIALOG_LIST_ITEM 3
#define DIALOG_OK_ITEM 1
#define DIALOG_CANCEL_ITEM 2

/************************************************************/

pascal Boolean myDialogFilter(DialogPtr theDialog, EventRecord *theEvent, short *itemHit)
{
	GrafPtr		remPort;
	Point		thePt;
	Rect		theRect;
	char		theChar;
	short		doubleClick;
	
	switch (theEvent->what)
	{
		case keyDown:
			// if key pressed, handle return key
			theChar = (theEvent->message) & charCodeMask;
			if ((theChar == 0x0D) || (theChar == 0x03))
			{
				*itemHit = ((DialogPeek)theDialog)->aDefItem;
				return TRUE;
			}
			return FALSE;
			
		case mouseDown:
			// get where the click occured in global coords
			thePt = theEvent->where;
			
			// save the current port
			GetPort(&remPort);
			SetPort(theDialog);
			
			// convert click to local coords
			GlobalToLocal(&thePt);
			
			// get a copy of the list's view rectangle
			theRect.top = ((LList *)(((WindowPeek)theDialog)->refCon))->view.top;
			theRect.bottom = ((LList *)(((WindowPeek)theDialog)->refCon))->view.bottom;
			theRect.left = ((LList *)(((WindowPeek)theDialog)->refCon))->view.left;
			theRect.right = ((LList *)(((WindowPeek)theDialog)->refCon))->view.right + 16;
			
			if (!PtInRect(thePt, &theRect))
			{
				// mouse was not clicked in list
				SetPort(remPort);
				return FALSE;
			}
			
			// mouse was clicked in list
			*itemHit = DIALOG_LIST_ITEM;
			doubleClick = LLClick((LList *)(((WindowPeek)theDialog)->refCon), thePt, theEvent->modifiers);
			
			// automatically push OK if user double-clicks
			if (doubleClick)
				*itemHit = ((DialogPeek)theDialog)->aDefItem;
			else 
				*itemHit = DIALOG_LIST_ITEM;
				
			// restore port and let ModalDialog know we handled event
			SetPort(remPort);
			return TRUE;
			
		case updateEvt:
			BeginUpdate(theDialog);
			LLUpdate((LList *)(((WindowPeek)theDialog)->refCon));
			DrawDialog(theDialog);
			EndUpdate(theDialog);
			return FALSE;

		default:
			return FALSE;
	}
} // myDialogFilter


/************************************************************/

void DoSampleLList(Str255 city, Str255 state, Str255 someNumber)
{
	DialogPtr	theDialog;
	short		itemHit;
	short		iType;
	Handle		iHandle;
	Rect		iRect;
	LList		*theList;
	LRow		*row;
	short		dataLen;
	
	// make city,state,someNumber initially empty;
	// if the user selects a row and presses OK, 
	// they will contain data from the selected row
	city[0] = '\0';
	state[0] = '\0';
	someNumber[0] = '\0';
	
	// get the sample dialog
	theDialog = GetNewDialog(DIALOG_RSRC_ID, NULL, (WindowPtr)-1);

	// get Rect of list dialog item
	GetDItem(theDialog, DIALOG_LIST_ITEM, &iType, &iHandle, &iRect);
	
	// create a list in the dialog 16 pixels high with 3 columns
	theList = LLNew(&iRect, theDialog, 16, 3, true, LLOnlyOne);
	
	theList->colDesc[1].justify = 1;	// center column 2
	theList->colDesc[2].justify = -1;	// right justify column 3

	// remember theList for myDialogFilter
	((WindowPeek)theDialog)->refCon = (long)theList;

	// turn off list drawing while we add rows of data to the list
	LLDoDraw(theList, 0);
	
	// add some rows with data to the list
	row = LLAddRow(theList, NULL);
	LLSetCell(theList, row, 0, 7, "Georgia");
	LLSetCell(theList, row, 1, 7, "Atlanta");
	LLSetCell(theList, row, 2, 1, "1");
	
	row = LLAddRow(theList, NULL);
	LLSetCell(theList, row, 0, 8, "Virginia");
	LLSetCell(theList, row, 1, 7, "Norfolk");
	LLSetCell(theList, row, 2, 2, "22");
	
	row = LLAddRow(theList, NULL);
	LLSetCell(theList, row, 0, 8, "Virginia");
	LLSetCell(theList, row, 1, 7, "Roanoke");
	LLSetCell(theList, row, 2, 3, "333");

	row = LLAddRow(theList, NULL);
	LLSetCell(theList, row, 0, 7, "Indiana");
	LLSetCell(theList, row, 1, 9, "Ft. Wayne");
	LLSetCell(theList, row, 2, 4, "4444");

	// turn list drawing back on
	LLDoDraw(theList, 1);
	
	// initially disable OK button
	GetDItem(theDialog, DIALOG_OK_ITEM, &iType, &iHandle, &iRect);
	HiliteControl((ControlHandle)iHandle, 255);
	
	do {
		ModalDialog(myDialogFilter, &itemHit);
		
		// dim OK button if no row is selected
		row = LLNextRow(theList, NULL);
		if (!LLGetSelect(theList, &row, 1))
		{
			HiliteControl((ControlHandle)iHandle, 255);
			if (itemHit == 1)
				itemHit = 0; // in case user hit return
		}
		else
			HiliteControl((ControlHandle)iHandle, 0);		
		
	} while ((itemHit != DIALOG_OK_ITEM)  && (itemHit != DIALOG_CANCEL_ITEM));

	// if user presses ok get the city name from the selected row
	if (itemHit == 1)
	{
		// get pointer to first row
		row = LLNextRow(theList, NULL);
		
		// find first selected row; start looking in first row
		if (LLGetSelect(theList, &row, 1))
		{
			// get selected state
			dataLen = 255;
			LLGetCell(theList, row, 0, &dataLen, (Ptr)&(state[1]));
			state[0] = (char)dataLen;
			
			// get selected city
			dataLen = 255;
			LLGetCell(theList, row, 1, &dataLen, (Ptr)&(city[1]));
			city[0] = (char)dataLen;

			// get selected someNumber
			dataLen = 255;
			LLGetCell(theList, row, 2, &dataLen, (Ptr)&(someNumber[1]));
			someNumber[0] = (char)dataLen;
		}
	}
	
	// dispose of list
	LLDispose(theList);
	
	// dispose of dialog
	DisposDialog(theDialog);
} // DoSampleLList

/************************************************************/

main()
{
	Str255		city;
	Str255		state;
	Str255		someNumber;

	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	InitDialogs(0L);
	FlushEvents(everyEvent, 0);
	InitCursor();
	DoSampleLList(city, state, someNumber);
} // main

/************************************************************/
