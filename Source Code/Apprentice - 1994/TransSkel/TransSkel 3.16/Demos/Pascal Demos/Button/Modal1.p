unit Modal1;

interface

	uses
		TransSkel, ButtonGlobals;


	procedure DoModal1;

implementation

	const

		iOK = 1;
		iCancel = 2;
		iEditText = 3;
		iOutline = 4;

	var

		defaultButton: Integer;


{--------------------------------------------------------------------}
{ Dialog 1 procedures }
{--------------------------------------------------------------------}

	procedure DoModal1;
		var
			filter: ModalFilterProcPtr;
			dlog: DialogPtr;
			savePort: GrafPtr;
			item: Integer;
			str: Str255;
			hilite: Integer;
			loop: Boolean;
	begin
		dlog := GetNewDialog(modal1Res, nil, WindowPtr(-1));
		if (dlog = DialogPtr(nil)) then
			begin
				SysBeep(1);
				exit(DoModal1);
			end;

		SkelPositionWindow(dlog, skelPositionOnMainDevice, horizRatio, vertRatio);

		GetPort(savePort);
		SetPort(dlog);

		SkelSetDlogButtonOutliner(dlog, iOutline);
		SkelSetDlogStr(dlog, iEditText, 'Default button is active only when this field is non-empty');
		SelItext(dlog, iEditText, 0, 32767);

		ShowWindow(dlog);

		loop := true;
		while (loop) do
			begin
				filter := SkelDlogFilter(nil, true);
				SkelDlogCancelItem(iCancel);
				SkelDlogTracksCursor(true);
				ModalDialog(filter, item);
				SkelRmveDlogFilter;
				if ((item = iOK) or (item = iCancel)) then
					loop := false
				else
					begin
						SkelGetDlogStr(dlog, iEditText, str);
						if (length(str) > 0) then
							hilite := normalHilite
						else
							hilite := dimHilite;
						if (SkelSetDlogCtlHilite(dlog, iOK, hilite)) then
							SkelDrawButtonOutline(SkelGetDlogCtl(dlog, iOK));
					end;
			end;

		DisposeDialog(dlog);
		SetPort(savePort);
	end;

end.