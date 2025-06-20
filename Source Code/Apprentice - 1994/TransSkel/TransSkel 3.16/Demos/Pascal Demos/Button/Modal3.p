unit Modal3;

interface

	uses
		TransSkel, ButtonGlobals;


	procedure DoModal3;

implementation

	const

		iPushDismiss = 1;
		iPushButton1 = 2;
		iPushButton2 = 3;
		iPushButton3 = 4;
		iRadioStaticText = 5;
		iRadioButton1 = 6;
		iRadioButton2 = 7;
		iRadioButton3 = 8;
		iRadioNone = 9;
		iCheckStaticText = 10;
		iCheckButton1 = 11;
		iCheckButton2 = 12;
		iCheckButton3 = 13;
		iOutline = 14;


	var

		defaultButton: Integer;


{--------------------------------------------------------------------}
{ Dialog 3 procedures }
{--------------------------------------------------------------------}

	procedure OutlineButton (dlog: DialogPtr;
									item: Integer);
	begin
		SkelDrawButtonOutline(SkelGetDlogCtl(dlog, defaultButton));
	end;


	procedure InstallOutliner (dlog: DialogPtr;
									item: Integer);
		var
			r: Rect;
	begin
		SkelGetDlogRect(dlog, item, r);
		InsetRect(r, -4, -4);
		SkelSetDlogRect(dlog, iOutline, r);
		SkelSetDlogProc(dlog, iOutline, @OutlineButton);
		SkelDrawButtonOutline(SkelGetDlogCtl(dlog, defaultButton));
	end;


	procedure RemoveOutliner (dlog: DialogPtr);
	begin
		SkelSetDlogProc(dlog, iOutline, nil);
		SkelEraseButtonOutline(SkelGetDlogCtl(dlog, defaultButton));
	end;


	procedure SetDefaultButton (dlog: DialogPtr;
									item: Integer);
	begin
		if (defaultButton <> 0) then
			RemoveOutliner(dlog);
		defaultButton := item;
		if (defaultButton <> 0) then
			InstallOutliner(dlog, defaultButton);
	end;


	procedure DoModal3;
		var
			filter: ModalFilterProcPtr;
			dlog: DialogPtr;
			savePort: GrafPtr;
			item: Integer;
			value: Integer;
			hilite: Integer;
			loop: Boolean;
	begin
		dlog := GetNewDialog(modal3Res, nil, WindowPtr(-1));
		if (dlog = DialogPtr(nil)) then
			begin
				SysBeep(1);
				exit(DoModal3);
			end;

		SkelPositionWindow(dlog, skelPositionOnMainDevice, horizRatio, vertRatio);

		GetPort(savePort);
		SetPort(dlog);

		SetDefaultButton(dlog, iPushButton1);
		SkelSetDlogCtlValue(dlog, iCheckButton1, 1);
		SkelSetDlogCtlValue(dlog, iCheckButton2, 1);
		SkelSetDlogCtlValue(dlog, iCheckButton3, 1);
		SkelSetDlogRadioButtonSet(dlog, iRadioButton1, iRadioNone, iRadioButton1);

		ShowWindow(dlog);

		loop := true;
		while (loop) do
			begin
				filter := SkelDlogFilter(nil, false);
				SkelDlogDefaultItem(defaultButton); { turns off if zero }
				ModalDialog(filter, item);
				SkelRmveDlogFilter;
				case item of
					iPushDismiss: 
						loop := false;
					iPushButton1, iPushButton2, iPushButton3: 
						begin
							{ ignore hits in these items }
						end;
					iRadioButton1, iRadioButton2, iRadioButton3: 
						begin
							SkelSetDlogRadioButtonSet(dlog, iRadioButton1, iRadioNone, item);
							{ remap item number from radio button range into pushbutton range }
							item := item + iPushButton1 - iRadioButton1;
							SetDefaultButton(dlog, item);
						end;
					iRadioNone: 
						begin
							SkelSetDlogRadioButtonSet(dlog, iRadioButton1, iRadioNone, item);
							SetDefaultButton(dlog, 0);	{ no default button }
						end;
					iCheckButton1, iCheckButton2, iCheckButton3: 
						begin
							value := SkelToggleDlogCtlValue(dlog, item);
							if (value <> 0) then
								hilite := normalHilite
							else
								hilite := dimHilite;
							{ remap item number from checkbox range into pushbutton range }
							item := item + iPushButton1 - iCheckButton1;
							if (SkelSetDlogCtlHilite(dlog, item, hilite) and (item = defaultButton)) then
								SkelDrawButtonOutline(SkelGetDlogCtl(dlog, item));
						end;
				end;
			end;
		DisposeDialog(dlog);
		SetPort(savePort);
	end;

end.