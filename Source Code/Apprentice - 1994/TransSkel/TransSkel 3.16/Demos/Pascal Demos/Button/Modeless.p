unit Modeless;

interface

	uses
		TransSkel, ButtonGlobals;


	procedure SetupModeless;

implementation

	const

		okBtn = 1;
		cancelBtn = 2;

{--------------------------------------------------------------------}
{ Dialog handling procedures }
{--------------------------------------------------------------------}


	function Filter (dlog: DialogPtr;
									evt: EventRecord;
									var item: Integer): Boolean;
		var
			hilite: Integer;
			ignore: Boolean;
	begin
		Filter := false;
		case evt.what of
			updateEvt: 
				SkelDrawButtonOutline(SkelGetDlogCtl(dlog, okBtn));
			activateEvt: 
				begin
					if (BitAnd(evt.modifiers, activeFlag) <> 0) then
						hilite := normalHilite
					else
						hilite := dimHilite;
					ignore := SkelSetDlogCtlHilite(dlog, okBtn, hilite);
					SkelDrawButtonOutline(SkelGetDlogCtl(dlog, okBtn));
					ignore := SkelSetDlogCtlHilite(dlog, cancelBtn, hilite);
				end;
			keyDown: 
				Filter := SkelDlogMapKeyToButton(dlog, evt, item, okBtn, cancelBtn);
		end;
	end;


	procedure Clobber;
		var
			dlog: DialogPtr;
	begin
		GetPort(dlog);
		HideWindow(dlog);
		DisposeDialog(dlog);
	end;


	procedure SetupModeless;
		var
			dlog: DialogPtr;
			ignore: Boolean;
	begin

		dlog := GetNewDialog(modelessRes, nil, WindowPtr(-1));
		if (dlog = nil) then
			begin
				SysBeep(1);
				exit(SetupModeless);
			end;
		ignore := SkelDialog(dlog, @filter, nil, nil, @Clobber);

		ShowWindow(dlog);
		SkelDoEvents(activMask + updateMask);
	end;

end.