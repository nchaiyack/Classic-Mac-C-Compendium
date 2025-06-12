program Glypha;	{Glypha II 1991 Soft Dorothy}

{  This code is for all to disect and laugh at.  It is fairly      }
{  sloppy and very, ver underdocumented.  Still, I have no time    }
{  to go back and improve upon this code, so I offer it as is.     }
{  It nonetheless reflects the abilities of one in the middle of   }
{  learning how to write a game in Pascal on the Mac.  Almost      }
{  every function can be improved - if only to make each function  }
{  more elegant.  There are no overwhelming "bugs" though.  Still, }
{  for example, you shouldn't write to your own resource fork as   }
{  this game does - and if you decide to write to your resource    }
{  fork anyway, you could do it more elegantly than I did here.    }

{$I-}

	uses
		Sound, Palettes, Globals, GameUtils, Enemies, GlyphaGuts, Initialize, Menus;

	var
		code, index, theMenu, theItem, chCode, flashes: integer;
		ch: char;
		mResult, tickWait, dummyLong: LongInt;
		whichWindow: WindowPtr;
		tempRect: Rect;
		theInput: TEHandle;
		mousePt: Point;
		err: OSErr;
		eventHappened: Boolean;

{===================================}

	procedure CheckTheMouse;
		var
			screenPos: Integer;

{--------------------}

		procedure HorizontalMouse;
			const
				MBState = $172;
				MTemp = $828;
				RawMouse = $82C;
				Mouse = $830;
				CrsrNew = $8CE;
				CrsrCouple = $8CF;
				Couple = $FF;
				Uncouple = $00;
			var
				center: Point;
				lowGlob: Integer;
				lowMem: Ptr;
				pointPtr: ^Point;
		begin
			SetPt(center, mousePt.h, 240);
			lowMem := Pointer(rawMouse);
			pointPtr := @lowMem^;
			pointPtr^ := center;
			lowMem := Pointer(MTemp);
			pointPtr := @lowMem^;
			pointPtr^ := center;
			lowMem := Pointer(CrsrNew);
			lowMem^ := $FFFF;
		end;

{--------------------}

	begin
		GetMouse(mousePt);
		HorizontalMouse;
		screenPos := (mousePt.h - 320 - rightOffset) div 8;
		if (screenPos > 16) then
			screenPos := 16;
		if (screenPos < -16) then
			screenPos := -16;

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
			begin
				keyStillDown := TRUE;
				thePlayer.facing := kLeftFace;
			end;
		if ((keyState[kRightKey1]) or (keyState[kRightKey2])) then
			begin
				keyStillDown := TRUE;
				thePlayer.facing := kRightFace;
			end;
	end;

{===================================}

	procedure HandleGameEvent;
		var
			wasPort: GrafPtr;
	begin
		case theEvent.what of
			KeyDown: 
				begin
					chCode := BitAnd(theEvent.message, CharCodeMask);
					if (ODD(theEvent.modifiers div CmdKey)) then
						begin
							mResult := MenuKey(CHR(chCode));
							theMenu := HiWord(mResult);
							theItem := LoWord(mResult);
							if (theMenu <> 0) then
								Handle_My_Menu(theMenu, theItem, theInput); {Do the menu selection}
						end
					else if (keyboardControl) then
						with thePlayer do
							case chCode of
								kSpaceBar:		{flap}
									begin
										state := TRUE;
									end;
								82, 114:					{refresh}
									begin
										CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, wholeArea, wholeArea, srcCopy, mainWndo^.visRgn);
										ShowScore;
										ShowMortals;
										ShowLevel;
									end;
								70, 102:					{flush}
									begin
										GetPort(GrafPtr(wasPort));
										SetPort(GrafPtr(mainWndo));
										FillRect(mainWndo^.portBits.bounds, black);
										SetPort(GrafPtr(wasPort));
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
					whichWindow := WindowPtr(theEvent.message); {Get the window the update is for}
					if (whichWindow = mainWndo) then
						begin
							SetPort(GrafPtr(mainWndo));
							BeginUpdate(mainWndo);     {Set the clipping to the update area}
							CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, wholeArea, wholeArea, srcCopy, mainWndo^.visRgn);
							ShowScore;
							ShowMortals;
							ShowLevel;
							EndUpdate(mainWndo);       {Return to normal clipping area}
							SetPort(GrafPtr(virginCPtr));
						end;
				end;
			otherwise
		end;
	end;

{===================================}

begin
	InitVariables;
	UnloadSeg(@InitVariables);
	tickWait := TickCount;
	theInput := nil;

	repeat

		CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, BitMapPtr(loadCPtr^.portPixMap^)^, flameRect[0], flameRect[0], srcCopy, nil);
		CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, BitMapPtr(virginCPtr^.portPixMap^)^, flameRect[1], flameRect[0], srcCopy, nil);
		CopyBits(BitMapPtr(loadCPtr^.portPixMap^)^, BitMapPtr(virginCPtr^.portPixMap^)^, flameRect[0], flameRect[1], srcCopy, nil);
		CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, flameRect[0], flameRect[0], srcCopy, mainWndo^.visRgn);
		CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, flameRect[1], flameRect[1], srcCopy, mainWndo^.visRgn);
		Delay(4, dummyLong);
		if DoRandom(200) = 0 then
			begin
				SetPort(GrafPtr(mainWndo));
				flashes := DoRandom(4) + 1;
				CopyMask(BitMapPtr(objectCPtr^.portPixMap^)^, offMaskMap, mainWndo^.portBits, eyeRects[4], eyeRects[4], theEye.dest);
				DoTheSound('lightning.snd', highPriority);
				for index := 1 to flashes do
					StrikeLightning(upperEye);
				CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, theEye.dest, theEye.dest, srcCopy, playRgn);
			end;

		if (hasWNE) then
			eventHappened := WaitNextEvent(everyEvent, theEvent, sleep, nil)
		else
			begin
				SystemTask;
				eventHappened := GetNextEvent(everyEvent, theEvent);
			end;

		if (eventHappened) then
			begin
				code := FindWindow(theEvent.where, whichWindow); {Get which window the event happened in}
				case theEvent.what of							{Decide type of event}
					MouseDown:												{Mouse button pressed}
						begin														{Handle the pressed button}
							if (code = inMenuBar) then		{See if a menu selection}
								begin									{Get the menu selection and handle it	}
									mResult := MenuSelect(theEvent.Where);				{Do menu selection	}
									theMenu := HiWord(mResult);			{Get the menu list number			}
									theItem := LoWord(mResult);			{Get the menu list item number	}
									Handle_My_Menu(theMenu, theItem, theInput); 		{Handle the menu	}
								end;											{End of inMenuBar					}
							if (code = inSysWindow) then				{See if a DA selection		}
								SystemClick(theEvent, whichWindow); {Let other programs in		}
						end;												{End of MouseDown			}
					KeyDown:											{Handle key inputs			}
						begin
							with theEvent do
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
													SetPort(GrafPtr(mainWndo));
													for index := 1 to 10 do
														StrikeLightning(DoRandom(4));
												end;
											'L': 
												begin
													SetPort(GrafPtr(mainWndo));
													flashes := DoRandom(4) + 1;
													CopyMask(BitMapPtr(objectCPtr^.portPixMap^)^, offMaskMap, mainWndo^.portBits, eyeRects[4], eyeRects[4], theEye.dest);
													DoTheSound('lightning.snd', highPriority);
													for index := 1 to flashes do
														StrikeLightning(upperEye);
													CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, theEye.dest, theEye.dest, srcCopy, playRgn);
												end;
											'm', 'M': 
												DoTheSound('music.snd', highPriority);
											otherwise
												begin
												end;
										end;
								end;				{End for with}
						end;						{End for KeyDown,AutoKey}
					UpDateEvt:				{Update event for a window}
						begin						{Handle the update}
							whichWindow := WindowPtr(theEvent.message); {Get the window the update is for}
							BeginUpdate(whichWindow);     {Set the clipping to the update area}
							CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, wholeArea, wholeArea, srcCopy, mainWndo^.visRgn);
							ShowScore;
							ShowMortals;
							ShowLevel;
							EndUpdate(whichWindow);       {Return to normal clipping area}
						end;                            {End of UpDateEvt}
					ActivateEvt:                   {Window activated event}
						begin                           {Handle the activation}
							whichWindow := WindowPtr(theEvent.message); {Get the window to be activated}
							if odd(theEvent.modifiers) then {Make sure it is Activate and not DeActivate}
								SelectWindow(whichWindow);    {Activate the window by selecting it}
						end;                            {End of ActivateEvt}
					App4Evt: 
						case BSR(theEvent.message, 24) of	{high byte of message}
							1:  						{suspendResumeMessage}
								if (BitAnd(theEvent.message, suspendResumeBit) = resuming) then
									inBackground := FALSE
								else
									begin
										inBackground := TRUE;		{it was a suspend event}
										if (chanPtr <> nil) then
											err := SndDisposeChannel(chanPtr, TRUE);
										chanPtr := nil;
									end;
							otherwise
								;
						end; {CASE}
					otherwise
				end;                              {End of case}

				while (playing) do
					begin
						gameCycle := gameCycle + 1;
						if keyboardControl then
							CheckTheKeyboard
						else
							CheckTheMouse;
						HideCursor;
						if (stonesSliding) then
							SlideTheStones;
						MoveThePlayer;
						HandleTheEnemies;
						UpdateEye;
						with thePlayer do
							begin
								if ((dest.bottom > handTop - 20) and (dest.left > handLeft) and (not otherState)) then
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
							if (hasWNE) then
								eventHappened := WaitNextEvent(everyEvent, theEvent, 0, nil)
							else
								begin
									SystemTask;
									eventHappened := GetNextEvent(everyEvent, theEvent);
								end;
							if eventHappened then
								HandleGameEvent;
						until (not pausing);

						repeat
						until (TickCount >= tickWait);
						tickWait := TickCount + gameSpeed;
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

	DisposePalette(mainPalette);

	DisposeWindow(GrafPtr(mainWndo));
	CloseCPort(objectCPtr);
	DisposPtr(objectCBits);
	CloseCPort(virginCPtr);
	DisposPtr(virginCBits);
	CloseCPort(loadCPtr);
	DisposPtr(loadCBits);

	WriteOutScores;

end.                                    {End of the program}