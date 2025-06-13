unit DialogUtils;

{ WASTE DEMO PROJECT: }
{ Dialog Utilities }

{ Copyright © 1993-1995 Marco Piovanelli }
{ All Rights Reserved }

interface
	uses
		Dialogs;

	function GetMyStandardDialogFilter: ModalFilterUPP;

	function GetDialogItemType (dialog: DialogRef;
									item: Integer): Integer;
	function GetDialogItemHandle (dialog: DialogRef;
									item: Integer): Handle;
	procedure GetDialogItemRect (dialog: DialogRef;
									item: Integer;
									var itemRect: Rect);
	procedure SetDialogItemProc (dialog: DialogRef;
									item: Integer;
									proc: UserItemUPP);

implementation

	procedure DoWindowEvent ({const} var event: EventRecord);
	external;		{ defined in DemoEvents.p }

	function MyStandardDialogFilter (dialog: DialogRef;
									var event: EventRecord;
									var item: Integer): Boolean;
		var
			savePort: GrafPtr;
			standardFilter: ProcPtr;
	begin
		MyStandardDialogFilter := false;

{ set up thePort }
		GetPort(savePort);
		SetPort(dialog);

{ intercept window events directed to windows behind the dialog }
		if (event.what in [updateEvt, activateEvt]) then
			if (DialogRef(event.message) <> dialog) then
				DoWindowEvent(event);

{ is the default item a pushbutton? }
		if (GetDialogItemType(dialog, DialogPeek(dialog)^.aDefItem) = kButtonDialogItem) then

{ yes, so tell the Dialog Manager to care about its outline }
			if (SetDialogDefaultItem(dialog, DialogPeek(dialog)^.aDefItem) <> noErr) then
				;

{ call the standard Dialog Manager filter procedure }
		if (GetStdFilterProc(standardFilter) = noErr) then
			MyStandardDialogFilter := CallModalFilterProc(dialog, event, item, standardFilter);

{ restore thePort }
		SetPort(savePort);

	end;  { MyStandardDialogFilter }

	var

{ static variable for GetMyStandardDialogFilter }
		sMyStandardDialogFilterProc: ModalFilterUPP;

	function GetMyStandardDialogFilter: ModalFilterUPP;
	begin
		if (sMyStandardDialogFilterProc = nil) then
			sMyStandardDialogFilterProc := NewModalFilterProc(@MyStandardDialogFilter);
		GetMyStandardDialogFilter := sMyStandardDialogFilterProc;
	end;  { GetMyStandardDialogFilter }

	function GetDialogItemType (dialog: DialogRef;
									item: Integer): Integer;
		var
			itemType: Integer;
			itemHandle: Handle;
			itemRect: Rect;
	begin
		GetDialogItem(dialog, item, itemType, itemHandle, itemRect);
		GetDialogItemType := itemType;
	end;  { GetDialogItemType }

	function GetDialogItemHandle (dialog: DialogRef;
									item: Integer): Handle;
		var
			itemType: Integer;
			itemHandle: Handle;
			itemRect: Rect;
	begin
		GetDialogItem(dialog, item, itemType, itemHandle, itemRect);
		GetDialogItemHandle := itemHandle;
	end;  { GetDialogItemHandle }

	procedure GetDialogItemRect (dialog: DialogRef;
									item: Integer;
									var itemRect: Rect);
		var
			itemType: Integer;
			itemHandle: Handle;
	begin
		GetDialogItem(dialog, item, itemType, itemHandle, itemRect);
	end;  { GetDialogItemRect }

	procedure SetDialogItemProc (dialog: DialogRef;
									item: Integer;
									proc: UserItemUPP);
		var
			itemType: Integer;
			itemHandle: Handle;
			itemRect: Rect;
	begin
		GetDialogItem(dialog, item, itemType, itemHandle, itemRect);
		if (BAND(itemType, $007F) = userItem) then
			SetDialogItem(dialog, item, itemType, Handle(proc), itemRect);
	end;  { SetDialogItemProc }

end.