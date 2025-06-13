{************************************************************}
{*															*}
{* Sample LList												*}
{*															*}
{* The following code demonstrates the use of an LList in 	*}
{* a modal dialog. See LList.p for documentation on each	*}
{* LList procedure.											*}
{*															*}
{************************************************************}

program SampleLList;

	uses
		LList;

	const
		DIALOG_RSRC_ID = 128;
		DIALOG_LIST_ITEM = 3;
		DIALOG_SORT_FWD_ITEM = 4;
		DIALOG_SORT_BKWD_ITEM = 5;
		DIALOG_OK_ITEM = 1;
		DIALOG_CANCEL_ITEM = 2;

	var
		city: Str255;
		state: Str255;
		someNumber: Str255;

{************************************************************}

	function myDialogFilter (theDialog: DialogPtr; var theEvent: EventRecord; var itemHit: integer): Boolean;

		var
			remPort: GrafPtr;
			thePt: Point;
			theRect: Rect;
			theChar: char;
			doubleClick: integer;

	begin
		case theEvent.what of
			keyDown: 
				begin
	{if key pressed, handle return key}
					theChar := char(BitAnd(theEvent.message, charCodeMask));
					if (theChar = char($0D)) or (theChar = char($03)) then
						begin
							itemHit := DialogPeek(theDialog)^.aDefItem;
							myDialogFilter := true;
							Exit(myDialogFilter);
						end;
					myDialogFilter := false;
					Exit(myDialogFilter);
				end;

			mouseDown: 
				begin
	{get where the click occured in global coords}
					thePt := theEvent.where;

	{save the current port}
					GetPort(remPort);
					SetPort(theDialog);

	{convert click to local coords}
					GlobalToLocal(thePt);

	{get a copy of the list's view rectangle}
					theRect.top := LListPtr(WindowPeek(theDialog)^.refCon)^.view.top;
					theRect.bottom := LListPtr(WindowPeek(theDialog)^.refCon)^.view.bottom;
					theRect.left := LListPtr(WindowPeek(theDialog)^.refCon)^.view.left;
					theRect.right := LListPtr(WindowPeek(theDialog)^.refCon)^.view.right + 16;

					if not PtInRect(thePt, theRect) then
						begin
	{mouse was not clicked in list}
							SetPort(remPort);
							myDialogFilter := false;
							Exit(myDialogFilter);
						end;

	{mouse was clicked in list}
					itemHit := DIALOG_LIST_ITEM;
					doubleClick := LLClick(LListPtr(WindowPeek(theDialog)^.refCon), thePt, theEvent.modifiers);

	{automatically push OK if user double-clicks}
					if (doubleClick = 1) then
						itemHit := DialogPeek(theDialog)^.aDefItem
					else
						itemHit := DIALOG_LIST_ITEM;

	{restore port and let ModalDialog know we handled event}
					SetPort(remPort);
					myDialogFilter := true;
					Exit(myDialogFilter);
				end;

			updateEvt: 
				begin
					BeginUpdate(theDialog);
					LLUpdate(LListPtr(WindowPeek(theDialog)^.refCon));
					DrawDialog(theDialog);
					EndUpdate(theDialog);
					myDialogFilter := false;
					Exit(myDialogFilter);
				end;

			otherwise
				myDialogFilter := false;

		end;
	end; {myDialogFilter}


{************************************************************}

	procedure DoSampleLList (var city: Str255; var state: Str255; var someNumber: Str255);

		var
			theDialog: DialogPtr;
			itemHit: integer;
			iType: integer;
			iHandle: Handle;
			iRect: Rect;
			theList: LListPtr;
			row: LRowPtr;
			dataLen: integer;
			text: Str255;

	begin
{if the user selects a row and presses OK, city, state,}
{and someNumber will contain data from the selected row}
		city := '';
		state := '';
		someNumber := '';

{get the sample dialog}
		theDialog := GetNewDialog(DIALOG_RSRC_ID, nil, WindowPtr(-1));

{get Rect of list dialog item}
		GetDItem(theDialog, DIALOG_LIST_ITEM, iType, iHandle, iRect);

{create a list in the dialog 16 pixels high with 3 columns}
		theList := LLNew(iRect, theDialog, 16, 3, 1, LLOnlyOne);

		theList^.colDesc[1].justify := 1;		{center column 2}
		theList^.colDesc[2].justify := -1;	{right justify column 3}
		theList^.colDesc[2].numeric := 1;		{if we want to sort column 3 numerically}

{remember theList for myDialogFilter}
		WindowPeek(theDialog)^.refCon := longint(theList);

{turn off list drawing while we add rows of data to the list}
		LLDoDraw(theList, 0, 0);

{add some rows with data to the list}
		row := LLAddRow(theList, nil);
		text := 'Georgia';
		LLSetCell(theList, row, 0, length(text), pointer(ord(@text) + 1));
		text := 'Atlanta';
		LLSetCell(theList, row, 1, length(text), pointer(ord(@text) + 1));
		text := '1';
		LLSetCell(theList, row, 2, length(text), pointer(ord(@text) + 1));

		row := LLAddRow(theList, nil);
		text := 'Virginia';
		LLSetCell(theList, row, 0, length(text), pointer(ord(@text) + 1));
		text := 'Roanoke';
		LLSetCell(theList, row, 1, length(text), pointer(ord(@text) + 1));
		text := '22';
		LLSetCell(theList, row, 2, length(text), pointer(ord(@text) + 1));

		row := LLAddRow(theList, nil);
		text := 'Virginia';
		LLSetCell(theList, row, 0, length(text), pointer(ord(@text) + 1));
		text := 'Norfolk';
		LLSetCell(theList, row, 1, length(text), pointer(ord(@text) + 1));
		text := '333';
		LLSetCell(theList, row, 2, length(text), pointer(ord(@text) + 1));

		row := LLAddRow(theList, nil);
		text := 'Indiana';
		LLSetCell(theList, row, 0, length(text), pointer(ord(@text) + 1));
		text := 'Ft. Wayne';
		LLSetCell(theList, row, 1, length(text), pointer(ord(@text) + 1));
		text := '4444';
		LLSetCell(theList, row, 2, length(text), pointer(ord(@text) + 1));

{turn list drawing back on}
		LLDoDraw(theList, 1, 0);

{initially disable OK button}
		GetDItem(theDialog, DIALOG_OK_ITEM, iType, iHandle, iRect);
		HiliteControl(ControlHandle(iHandle), 255);

		repeat
			ModalDialog(@myDialogFilter, itemHit);

			case itemHit of
				DIALOG_SORT_FWD_ITEM: 
					begin
	{sort the list by State, city, case-insensitive, in-order}
						LLSort(theList, 0, 1, -1, 1, 1);
					end;

				DIALOG_SORT_BKWD_ITEM: 
					begin
	{sort the list by State, City, case-insensitive, reverse-order}
						LLSort(theList, 0, 1, -1, 1, -1);
					end;

				otherwise
					begin
	{dim OK button if no row is selected}
						row := LLNextRow(theList, nil);
						if LLGetSelect(theList, row, 1) = 0 then
							begin
								HiliteControl(ControlHandle(iHandle), 255);
								if itemHit = DIALOG_OK_ITEM then
									itemHit := 0; {in case user hit return}
							end
						else
							HiliteControl(ControlHandle(iHandle), 0);
					end;

			end;

		until (itemHit = DIALOG_OK_ITEM) or (itemHit = DIALOG_CANCEL_ITEM);

{if user presses ok get the city name from the selected row}
		if itemHit = DIALOG_OK_ITEM then
			begin
	{get pointer to first row}
				row := LLNextRow(theList, nil);

	{find first selected row; start looking in first row}
				if LLGetSelect(theList, row, 1) = 1 then
					begin
	{get selected state}
						dataLen := 255;
						LLGetCell(theList, row, 0, dataLen, pointer(ord(@state) + 1));
	{$PUSH}
	{$R-}
						state[0] := char(dataLen);
	{$POP}

	{get selected city}
						dataLen := 255;
						LLGetCell(theList, row, 1, dataLen, pointer(ord(@city) + 1));
	{$PUSH}
	{$R-}
						city[0] := char(dataLen);
	{$POP}

	{get selected someNumber}
						dataLen := 255;
						LLGetCell(theList, row, 2, dataLen, pointer(ord(@someNumber) + 1));
	{$PUSH}
	{$R-}
						someNumber[0] := char(dataLen);
	{$POP}
					end;
			end;

{dispose of list}
		LLDispose(theList);

{dispose of dialog}
		DisposDialog(theDialog);
	end; {DoSampleLList}

{************************************************************}

begin
	FlushEvents(everyEvent, 0);
	InitCursor;
	DoSampleLList(city, state, someNumber);
end. {SampleLList}

{************************************************************}