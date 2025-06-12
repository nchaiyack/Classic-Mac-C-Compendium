program Glypha;	{Glypha 3.01 ©1990, 1991, 1992 , 1993 Soft Dorothy}

	uses
		AboutWndo, Dialogs, Sound, GameUtils, Enemies, GlyphaGuts, MainWndo, Initialize, Menus;

	const
		kSpaceKey = $31;
		kLeftKey1 = $29;
		kLeftKey2 = $00;
		kRightKey1 = $27;
		kRightKey2 = $01;

	var
		myEvent: EventRecord;
		code, index, theMenu, theItem, chCode, flashes: integer;
		ch: char;
		dummyLong, mResult: LongInt;
		whichWindow: WindowPtr;
		tempRect: Rect;
		theInput: TEHandle;
		mousePt: Point;
		err: OSErr;

{===================================}

	procedure CheckOurEnvirons;
		var
			err: OSErr;
			thisWorld: SysEnvRec;
	begin
		rightOffset := (ScreenBits.bounds.right - 512) div 2;
		downOffset := (ScreenBits.bounds.bottom - 342) div 2;

		err := SysEnvirons(1, thisWorld);	{Check set up the Mac game is on	}
		with thisWorld do
			begin
				if (machineType < 1) then		{If less than a Mac 512KE (<1)		}
					ExitToShell									{quit to the Finder now!					}
				else
					begin
						if (machineType < 3) then
							onFastMachine := FALSE
						else
							onFastMachine := TRUE;
						if (systemVersion < $0602) then
							inhibitSound := TRUE		{Global to prevent sound on older	}
						else											{versions of the System.					}
							inhibitSound := FALSE;	{Or, we allow sound.							}
					end;
			end;
	end;

{===================================}

	procedure CheckTheMouse;
		var
			screenPos: Integer;
	begin
		GetMouse(mousePt);
		screenPos := (mousePt.h - 256 - rightOffset) div 8;
		if (screenPos > 16) then
			screenPos := 16;

		with thePlayer do
			begin
				if (facing = 0) then
					begin
						if (screenPos < 0) then
							facing := 1
						else
							begin
								if (screenPos > horiVel) then
									keyStillDown := TRUE
								else
									keyStillDown := FALSE;
							end;
					end
				else
					begin
						if (screenPos > 0) then
							facing := 0
						else
							begin
								if (screenPos < horiVel) then
									keyStillDown := TRUE
								else
									keyStillDown := FALSE;
							end;
					end;
			end;
	end;

{===================================}

	procedure CheckTheKeyboard;
		var
			keyState: KeyMap;

	begin
		keyStillDown := FALSE;
		GetKeys(keyState);
		if ((keyState[kLeftKey1]) or (keyState[kLeftKey2])) then
			keyStillDown := TRUE
		else
			begin
				if ((keyState[kRightKey1]) or (keyState[kRightKey2])) then
					keyStillDown := TRUE;
			end;
	end;

{===================================}

	procedure HandleGameEvent (myEvent: EventRecord);
		var
			wasPort: GrafPtr;
	begin
		case myEvent.what of
			KeyDown: 
				begin
					chCode := BitAnd(myEvent.message, CharCodeMask);
					ch := CHR(chCode);
					if (ODD(myEvent.modifiers div CmdKey)) then
						begin
							if (ch = 'p') or (ch = 'P') then
								DoPause;
							if (ch = 'e') or (ch = 'E') then
								DoEnd;
							if (ch = 'q') or (ch = 'Q') then
								DoEnd;
						end
					else if (keyboardControl) then
						with thePlayer do
							case chCode of
								32:		{flap}
									begin
										state := TRUE;
									end;
								34, 39, 83, 115:	{point right}
									begin
										facing := 0;
									end;
								58, 59, 65, 97:		{point left}
									begin
										facing := 1;
									end;
								82, 114:					{refresh}
									begin
										Update_MainWndo(mainWndo);
										DrawMenuBar;
										FlashMenuBar(0);
									end;
								70, 102:					{flush}
									begin
										GetPort(wasPort);
										SetPort(mainWndo);
										FillRect(mainWndo^.portBits.bounds, black);
										SetPort(wasPort);
									end;

								otherwise
									begin
									end;
							end;	{end case chCode of}
				end;		{end of KeyDown event}
			MouseDown: 
				begin
					thePlayer.state := TRUE;
				end;
			UpDateEvt:				{Update event for a window}
				begin						{Handle the update}
					whichWindow := WindowPtr(myEvent.message); {Get the window the update is for}
					BeginUpdate(whichWindow);     {Set the clipping to the update area}
					Update_MainWndo(whichWindow);
					EndUpdate(whichWindow);       {Return to normal clipping area}
					DrawMenuBar;
					FlashMenuBar(0);
				end;
			otherwise
				begin
				end;
		end;
	end;

{===================================}

begin
	MaxApplZone;
	MoreMasters;               	{This reserves space for more handles}
	MoreMasters;
	MoreMasters;
	MoreMasters;
	CheckOurEnvirons;
	FlushEvents(everyEvent, 0);	{Clear out all events}
	Init_LogoWindo;
	Open_LogoWindo(rightOffset, downOffset);
	SetCursor(GetCursor(watchCursor)^^);
	Init_My_Menus;							{Initialize menu bar}
	FlashMenuBar(0);
	doneFlag := FALSE;
	theInput := nil;
	InitVariables;
	NewLightning;
	ReadInScores;
	Init_MainWndo;							{Set mainWndo's pointer to nil}
	Open_MainWndo(rightOffset, downOffset);	{Open the window}
	RedoTheBackground;
	Close_LogoWindo;
	DoTheSound('music.snd', TRUE);
	ReDrawHiScores;
	InitCursor;
	FlushEvents(everyEvent, 0);
	Delay(160, dummyLong);
	CopyMask(offPlayerMap, offPlayerMap, mainWndo^.portBits, eyeRects[4], eyeRects[4 + 5], theEye.dest);
	DoTheSound('lightning.snd', TRUE);
	SetPort(mainWndo);
	for index := 1 to 3 do
		StrikeLightning(upperEye);
	CopyBits(offVirginMap, mainWndo^.portBits, theEye.dest, theEye.dest, srcCopy, playRgn);
	FlushEvents(everyEvent, 0);

	repeat
		if (theInput <> nil) then
			TEIdle(theInput);
		SystemTask;

		CopyBits(offVirginMap, offLoadMap, flameRect[0], flameRect[0], srcCopy, nil);
		CopyBits(offVirginMap, offVirginMap, flameRect[1], flameRect[0], srcCopy, nil);
		CopyBits(offLoadMap, offVirginMap, flameRect[0], flameRect[1], srcCopy, nil);
		CopyBits(offVirginMap, mainWndo^.portBits, flameRect[0], flameRect[0], srcCopy, nil);
		CopyBits(offVirginMap, mainWndo^.portBits, flameRect[1], flameRect[1], srcCopy, nil);
		Delay(4, dummyLong);
		if DoRandom(200) = 0 then
			begin
				SetPort(mainWndo);
				flashes := DoRandom(4) + 1;
				CopyMask(offPlayerMap, offPlayerMap, mainWndo^.portBits, eyeRects[4], eyeRects[4 + 5], theEye.dest);
				DoTheSound('lightning.snd', TRUE);
				for index := 1 to flashes do
					StrikeLightning(upperEye);
				CopyBits(offVirginMap, mainWndo^.portBits, theEye.dest, theEye.dest, srcCopy, playRgn);
			end;

		if GetNextEvent(everyEvent, myEvent) then
			begin
				code := FindWindow(myEvent.where, whichWindow); {Get which window the event happened in}
				case myEvent.what of						{Decide type of event					}
					MouseDown:									{Mouse button pressed					}
						begin										{Handle the pressed button				}
							if (code = inMenuBar) then			{See if a menu selection				}
								begin									{Get the menu selection and handle it	}
									mResult := MenuSelect(myEvent.Where);				{Do menu selection	}
									theMenu := HiWord(mResult);			{Get the menu list number			}
									theItem := LoWord(mResult);			{Get the menu list item number	}
									Handle_My_Menu(theMenu, theItem, theInput); 		{Handle the menu	}
								end;											{End of inMenuBar					}
							if (code = inSysWindow) then				{See if a DA selection		}
								SystemClick(myEvent, whichWindow); {Let other programs in		}
						end;												{End of MouseDown			}
					KeyDown:											{Handle key inputs			}
						begin
							with myevent do
								begin
									chCode := BitAnd(message, CharCodeMask);	{Get character}
									ch := CHR(chCode);					{Change to ASCII}
									if (Odd(modifiers div CmdKey)) then		{See if Command key is down}
										begin
											mResult := MenuKey(ch);	   	{See if menu selection}
											theMenu := HiWord(mResult); {Get the menu list number}
											theItem := LoWord(mResult); {Get the menu item number}
											if (theMenu <> 0) then    	{See if a list was selected}
												Handle_My_Menu(theMenu, theItem, theInput); {Do the menu selection}
										end
									else
										case ch of
											'l': 
												begin
													SetPort(mainWndo);
													for index := 1 to 10 do
														StrikeLightning(DoRandom(4));
												end;
											'L': 
												begin
													SetPort(mainWndo);
													flashes := DoRandom(4) + 1;
													CopyMask(offPlayerMap, offPlayerMap, mainWndo^.portBits, eyeRects[4], eyeRects[4 + 5], theEye.dest);
													DoTheSound('lightning.snd', TRUE);
													for index := 1 to flashes do
														StrikeLightning(upperEye);
													CopyBits(offVirginMap, mainWndo^.portBits, theEye.dest, theEye.dest, srcCopy, playRgn);
												end;
											'm', 'M': 
												DoTheSound('music.snd', TRUE);
											otherwise
												begin
												end;
										end;
								end;				{End for with}
						end;						{End for KeyDown,AutoKey}
					UpDateEvt:				{Update event for a window}
						begin						{Handle the update}
							whichWindow := WindowPtr(myEvent.message); {Get the window the update is for}
							BeginUpdate(whichWindow);     {Set the clipping to the update area}
							Update_MainWndo(whichWindow);
							EndUpdate(whichWindow);       {Return to normal clipping area}
							DrawMenuBar;
							FlashMenuBar(0);
						end;                            {End of UpDateEvt}
					ActivateEvt:                   {Window activated event}
						begin                           {Handle the activation}
							whichWindow := WindowPtr(myevent.message); {Get the window to be activated}
							if odd(myEvent.modifiers) then {Make sure it is Activate and not DeActivate}
								SelectWindow(whichWindow);    {Activate the window by selecting it}
						end;                            {End of ActivateEvt}
					otherwise
						begin
						end;
				end;                              {End of case}

				while (playing) do
					begin
						gameCycle := gameCycle + 1;
						if keyboardControl then
							CheckTheKeyboard
						else
							CheckTheMouse;
						if (stonesSliding) then
							SlideTheStones;
						MoveThePlayer;
						HandleTheEnemies;
						UpdateEye;
						with thePlayer do
							begin
								if ((dest.bottom > 281) and (dest.left > 347) and (not otherState)) then
									UpdateTheHand
								else if (theHand.state) then
									RetractTheHand;
								DrawBeasts;
								DrawPlayer(dest, oldDest);
								oldDest := dest;
							end;
						if (deadAndGone) then
							ExitAMortal;
						if (onward) then
							AdvanceALevel;
						repeat
							if GetNextEvent(everyEvent, myEvent) then
								HandleGameEvent(myEvent);
						until (not pausing);
					end;
			end;
	until doneFlag;                       {End of the event loop}

	HUnlock(Handle(playRgn));
	DisposeRgn(playRgn);
	HUnlock(Handle(obeliskRgn1));
	DisposeRgn(obeliskRgn1);
	HUnlock(Handle(obeliskRgn2));
	DisposeRgn(obeliskRgn2);
	if (chanPtr <> nil) then
		err := SndDisposeChannel(chanPtr, FALSE);
	Close_MainWndo;

	ClosePort(offPlayerPort);
	DisposPtr(Ptr(offPlayerPort));
	ClosePort(offEnemyPort);
	DisposPtr(Ptr(offEnemyPort));
	ClosePort(offLoadPort);
	DisposPtr(Ptr(offLoadPort));
	ClosePort(offVirginPort);
	DisposPtr(Ptr(offVirginPort));

	WriteOutScores;

end.                                    {End of the program}