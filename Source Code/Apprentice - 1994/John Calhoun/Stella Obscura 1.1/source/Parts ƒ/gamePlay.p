unit gamePlay;

interface

	procedure D_gamePlay;

implementation

	const
		I_Okay = 1;
		I_Static_Text = 2;
		I_Static_Text3 = 3;
		I_Static_Text5 = 4;
		I_Static_Text7 = 5;
		I_Static_Text9 = 6;
		I_Static_Text11 = 7;
		I_Static_Text13 = 8;
		I_Static_Text15 = 9;
		I_Static_Text17 = 10;
		I_Static_Text19 = 11;
		I_Static_Text21 = 12;
		I_Static_Text23 = 13;
		I_Static_Text25 = 14;
		I_Static_Text27 = 15;
		I_Static_Text29 = 16;
		I_Static_Text31 = 17;
		I_Static_Text33 = 18;
		I_Picturex35 = 19;
	var
		ExitDialog: boolean;

{===========================================================}

	function MyFilter (theDialog: DialogPtr; var theEvent: EventRecord; var itemHit: integer): boolean;
		var
			DoubleClick: boolean;
			MyPt: Point;
			tempRect: Rect;
	begin
		MyFilter := FALSE;
		if (theEvent.what = MouseDown) then
			begin
				MyPt := theEvent.where;
				with theDialog^.portBits.bounds do
					begin
						myPt.h := myPt.h + left;
						myPt.v := myPt.v + top;
					end;
			end;
	end;

{===========================================================}

	procedure D_gamePlay;
		var
			GetSelection: DialogPtr;
			tempRect: Rect;
			DType: Integer;
			Index: Integer;
			DItem: Handle;
			CItem, CTempItem: controlhandle;
			sTemp: Str255;
			itemHit: Integer;
			temp: Integer;
			Icon_Handle: Handle;
			NewMouse: Point;
			InIcon: boolean;
			ThisEditText: TEHandle;
			TheDialogPtr: DialogPeek;
		procedure Refresh_Dialog;
			var
				rTempRect: Rect;
		begin
		end;

	begin
		GetSelection := GetNewDialog(6, nil, Pointer(-1));
		ShowWindow(GetSelection);
		SelectWindow(GetSelection);
		SetPort(GetSelection);
		TheDialogPtr := DialogPeek(GetSelection);
		ThisEditText := TheDialogPtr^.textH;
		HLock(Handle(ThisEditText));
		ThisEditText^^.txSize := 9;
		TextSize(9);
		ThisEditText^^.txFont := geneva;
		TextFont(geneva);
		ThisEditText^^.txFont := 3;
		ThisEditText^^.fontAscent := 10;
		ThisEditText^^.lineHeight := 10 + 2 + 0;
		HUnLock(Handle(ThisEditText));
		ExitDialog := FALSE;
		repeat
			ModalDialog(nil, itemHit);
			GetDItem(GetSelection, itemHit, DType, DItem, tempRect);
			CItem := Pointer(DItem);
			if (ItemHit = I_Okay) then
				begin
					ExitDialog := TRUE;
					Refresh_Dialog;
				end;
			if (ItemHit = I_Picturex35) then
				begin
				end;
		until ExitDialog;
		DisposDialog(GetSelection);
	end;

end.                                    {End of unit}
