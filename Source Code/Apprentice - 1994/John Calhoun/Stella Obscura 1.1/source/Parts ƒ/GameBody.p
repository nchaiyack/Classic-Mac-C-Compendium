program Game;

	uses
		LogoWindo, Sound, SomeGlobals, Utilities, mainWndo, Initialize, TheMenus;
	var
		theEvent: EventRecord;
		doneFlag: boolean;
		code, superFix, suffix: integer;
		whichWindow: WindowPtr;
		mResult, dummyLong: longint;
		theMenu, theItem: integer;
		chCode: integer;
		ch: char;
		theInput: TEHandle;
		thisWorld: SysEnvRec;
		err: OSErr;

{===================================}

begin
	MoreMasters;
	MoreMasters;
	MoreMasters;
	InitCursor;
	doneFlag := FALSE;
	SetCursor(GetCursor(WatchCursor)^^);
	Init_My_Menus;
	Init_LogoWindo;
	Open_LogoWindo;
	err := SysEnvirons(1, thisWorld);
	with thisWorld do
		begin
			if (machineType < 2) then
				ExitToShell
			else
				begin
					superFix := systemVersion div 256;
					suffix := systemVersion - superFix;
					if (superFix >= 6) and (suffix >= 2) then
						forgetSound := FALSE
					else
						forgetSound := TRUE;
				end;
		end;
	if forgetSound then
		begin
			SetItem(GetMenu(OptionsM), 1, 'Sound needs Sys 6.02 or >');
			DisableItem(GetMenu(OptionsM), 1);
		end;
	InitVariables;
	theInput := nil;
	InitCursor;
	Close_LogoWindo;
	Init_mainWndo;
	BringUpStartUp;
	DoTheVirgin;
	UpDate_mainWndo(mainWndo);
	SetEventMask(idleMask);
	FlushEvents(everyEvent, 0);

	repeat
		SystemTask;

		if GetNextEvent(everyEvent, theEvent) then
			begin
				code := FindWindow(theEvent.where, whichWindow);
				case theEvent.what of
					MouseDown: 
						begin
							if (code = inMenuBar) then
								begin
									mResult := MenuSelect(theEvent.Where);
									theMenu := HiWord(mResult);
									theItem := LoWord(mResult);
									Handle_My_Menu(doneFlag, theMenu, theItem, theInput);
								end;
							if (code = inContent) then
								begin
								end;
							if (code = inSysWindow) then
								SystemClick(theEvent, whichWindow);
						end;
					KeyDown, AutoKey: 
						begin
							with theEvent do
								begin
									chCode := BitAnd(message, CharCodeMask);
									ch := CHR(chCode);
									if (Odd(modifiers div CmdKey)) then
										begin
											mResult := MenuKey(ch);
											theMenu := HiWord(mResult);
											theItem := LoWord(mResult);
											if (theMenu <> 0) then
												Handle_My_Menu(doneFlag, theMenu, theItem, theInput);
										end;
								end;
						end;
					UpDateEvt: 
						if (not playing) then
							begin
								whichWindow := WindowPtr(theEvent.message);
								BeginUpdate(whichWindow);
								Update_mainWndo(whichWindow);
								EndUpdate(whichWindow);
							end;
					otherwise
						begin
						end;
				end;
			end;

		if playing and (not pausing) then
			begin
				SetShipsPos;
				if (hoopOut) then
					ComputeHoops
				else
					UpDateEnemy;
				DrawScene;
				if GameOver then
					WrapItUp;
				if slowOn then
					Delay(10, dummyLong);
			end;

	until doneFlag;                       {End of the event loop}

	if (chanPtr <> nil) then
		err := SndDisposeChannel(chanPtr, FALSE);

	HUnlock(Handle(leftScreenRgn));
	DisposeRgn(leftScreenRgn);
	HUnlock(Handle(rightScreenRgn));
	DisposeRgn(rightScreenRgn);

	ClosePort(offPlayerPort);
	DisposPtr(Ptr(offPlayerPort));
	ClosePort(offRightPort);
	DisposPtr(Ptr(offRightPort));
	ClosePort(offLeftPort);
	DisposPtr(Ptr(offLeftPort));
	ClosePort(offLeftVirginPort);
	DisposPtr(Ptr(offLeftVirginPort));
	ClosePort(offRightVirginPort);
	DisposPtr(Ptr(offRightVirginPort));

end.                                    {End of the program}