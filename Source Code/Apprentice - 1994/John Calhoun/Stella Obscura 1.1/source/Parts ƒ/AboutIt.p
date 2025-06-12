unit aboutIt;

interface

	procedure D_aboutIt;

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
		I_Iconx29 = 16;
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

	procedure D_aboutIt;
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
			ThisEditText: TEHandle;              {Handle to get the Dialogs TE record}
			TheDialogPtr: DialogPeek;            {Pointer to Dialogs definition record, contains the TE record}

		procedure Refresh_Dialog;
			var
				rTempRect: Rect;
		begin
		end;

	begin
		GetSelection := GetNewDialog(5, nil, Pointer(-1));
		ShowWindow(GetSelection);
		SelectWindow(GetSelection);
		SetPort(GetSelection);

		TheDialogPtr := DialogPeek(GetSelection);
		ThisEditText := TheDialogPtr^.textH;
		HLock(Handle(ThisEditText));
		ThisEditText^^.txSize := 12;
		TextSize(12);
		ThisEditText^^.txFont := systemFont;
		TextFont(systemFont);
		ThisEditText^^.txFont := 0;
		ThisEditText^^.fontAscent := 12;
		ThisEditText^^.lineHeight := 12 + 3 + 1;
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
			if (ItemHit = I_Iconx29) then
				begin
					Icon_Handle := GetIcon(10046);
					if (Icon_Handle <> nil) then
						begin
							EraseRect(tempRect);
							PlotIcon(tempRect, Icon_Handle);
						end;
					InIcon := TRUE;
					repeat
						GetMouse(NewMouse);
						if (PtInRect(NewMouse, tempRect)) then
							begin
								if not (InIcon) then
									begin
										Icon_Handle := GetIcon(10046);
										if (Icon_Handle <> nil) then
											begin
												EraseRect(tempRect);
												PlotIcon(tempRect, Icon_Handle);
											end;
										InIcon := TRUE;
									end;
							end
						else if InIcon then
							begin
								Icon_Handle := GetIcon(46);
								if (Icon_Handle <> nil) then
									begin
										EraseRect(tempRect);
										PlotIcon(tempRect, Icon_Handle);
									end;
								InIcon := FALSE;
							end;
					until not (StillDown);
					if (PtInRect(NewMouse, tempRect)) then
						begin
							Refresh_Dialog;
						end;
					Icon_Handle := GetIcon(46);
					if (Icon_Handle <> nil) then
						begin
							EraseRect(tempRect);
							PlotIcon(tempRect, Icon_Handle);
						end;
				end;
		until ExitDialog;
		DisposDialog(GetSelection);
	end;
end.                                    {End of unit}
