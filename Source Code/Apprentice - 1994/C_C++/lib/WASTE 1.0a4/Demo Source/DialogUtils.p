unit DialogUtils;

{ WASTE DEMO PROJECT: }
{ Dialog Utilities }

{ Copyright © 1993-1994 Merzwaren }
{ All Rights Reserved }

interface

	function DialogFilter (dialog: DialogPtr;
									var event: EventRecord;
									var item: Integer): Boolean;

implementation
	uses
		DialogExtensions;

	procedure DoWindowEvent (var event: EventRecord);
	external;		{ defined in DemoEvents.p }

	function CallStdFilterProc (dialog: DialogPtr;
									var event: EventRecord;
									var item: Integer;
									filterProc: ProcPtr): Boolean;
	inline
		$205F,					{ movea.l (sp)+, a0 }
		$4E90;					{ jsr (a0) }

	function DialogFilter (dialog: DialogPtr;
									var event: EventRecord;
									var item: Integer): Boolean;
		var
			savePort: GrafPtr;
			standardFilter: ProcPtr;
			err: OSErr;
	begin
		DialogFilter := false;

{ set up thePort }
		GetPort(savePort);
		SetPort(dialog);

{ intercept window events directed to windows behind the dialog }
		if (event.what in [updateEvt, activateEvt]) then
			if (WindowPtr(event.message) <> dialog) then
				DoWindowEvent(event);

{ tell the Dialog Manager to care about the default item }
		err := SetDialogDefaultItem(dialog, DialogPeek(dialog)^.aDefItem);

{ call the standard Dialog Manager filter procedure }
		if (GetStdFilterProc(standardFilter) = noErr) then
			DialogFilter := CallStdFilterProc(dialog, event, item, standardFilter);

{ restore thePort }
		SetPort(savePort);

	end;  { DialogFilter }

end.