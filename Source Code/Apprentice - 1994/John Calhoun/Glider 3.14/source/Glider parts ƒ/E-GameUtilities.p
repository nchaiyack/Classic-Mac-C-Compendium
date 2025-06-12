unit GameUtilities;

interface

	uses
		Dialogs, Sound;

	const
		maxLevel = 15;				{Here follow all the global constants.		}
		defaultNumber = 4;
		defaultRoomBegin = 1;
		dartForwardFast = 8;
		dartBackwardFast = -2;
		gliderForwardFast = 4;
		gliderBackwardFast = -4;
		gravityFast = 2;
		ceilingFast = 4;
		floorFast = -6;
		startHori = 0;
		startVert = 20;
		shadowVert = 288;

		fastSpeed = 1;
		slowSpeed = 2;

		noSound = 0;
		highPriority = 1;
		lowPriority = 2;

		L_Apple = 201;				{Menu list}
		C_About_Glider_2_0 = 1;
		L_Game = 202;				{Menu list}
		C_Start_Game = 1;
		C_Start_in_Room = 2;
		C_Pause = 4;
		C_End_Game = 5;
		C_Quit = 7;
		L_Options = 203;			{Menu list}
		C_Glider = 1;
		C_Dart = 2;
		C_Sound_On = 4;
		C_Number_of_Gliders = 6;
		C_Controls = 7;
		C_Fast = 9;
		C_Slow = 10;

	var		{And here are the global variables.  Many variables, huh?	}
		leftControl, rightControl: Char;
		moveHori, moveVert, soundPriority, gliderNumber, numberOfCandles: Integer;
		thrust, stall, gravity, ductGravity, ventLift, totalObjects, levelOn: Integer;
		switchH, switchV, gameTime, numberBegin, levelBegin, timeOfDeath: Integer;
		burnMode, demoIndex, nextSwitch, levelHolding, numberHolding: Integer;
		numberOfCopters, dripMode, dripCycle, advanceRate, catMode, gameSpeed: Integer;
		theScore, oldScore, totalTime: LongInt;
		stalling, doneFlag, Pausing, Playing, gliderCraft, soundOn: Boolean;
		extraTime, darkOn, switchUntouched, extraGlider, candleThere: Boolean;
		highScoresOut, burning, crushed, craftHolding, scoresChanged: Boolean;
		soundPlaying, inhibitSound: Boolean;
		nameUsing: Str255;
			{Here follow the various offscreen and onscreen bitmap variables.	}
		mainWindow: WindowPtr;
		virginPort, loadPort, objectsPort, oldPort: GrafPtr;
		virginBits, loadBits, objectsBits: Ptr;
		virginMap, loadMap, objectsMap: BitMap;
		virginArea, loadArea, objectsArea: Rect;
			{Some rectangles...			}
		playerDropRect, oldPlayerRect, shadowDropRect, oldShadowRect: Rect;
		screenArea, horizonArea, catBodyRect, dangerRect, pawRect: Rect;
		oldPawRect, vitalArea, playArea, dripDropRect, oldDripRect, escapeRect: Rect;
			{The only sound global.		}
		chanPtr: sndchannelptr;
			{And all the arrays...		}
		levelArray: packed array[1..15, 0..7, 1..4] of Integer;
		eventArray: array[1..9] of Rect;
		objectsRects: array[0..57, 0..1] of Rect;
		objectArray, backgroundArray: array[1..15] of Integer;
		candleInfo: array[0..2, 0..2] of Integer;
		hiStrings: array[1..10] of Str255;
		hiScores: array[1..10] of LongInt;
		coptersRects: array[1..4, 0..1] of Rect;
		copterMode: array[1..4, 0..1] of Integer;

			{Here are all the forwarded procedures and functions...		}
	procedure DoTheSound (whichOne: Str255; priority: Integer);
	function DoRandom (range: Integer): Integer;
	procedure ResizeARect (var whichRect: Rect; whatNumber: Integer);
	procedure ResetGliderPos;
	procedure Init_GliderMain;
	procedure Close_GliderMain;
	procedure DrawAnObject (whichObject, hori, vert: Integer);
	procedure SetUpFurniture;
	procedure FoldTheGlider;
	procedure Open_GliderMain;
	procedure Update_GliderMain;
	procedure ReDrawHiScores;
	procedure ReadInScores (var prefs: Str255);
	procedure WriteOutScores;

implementation

{=================================}

	function GetA5: LONGINT;
	inline
		$2E8D; {MOVE.L A5,(A7)}

{=================================}

	function LoadA5 (newA5: LONGINT): LONGINT;
	inline
		$2F4D, $0004, $2A5F;

{=================================}

	procedure ChanCallBack (chan: SndChannelPtr; cmd: SndCommand);
		var
			oldA5: LongInt;
	begin
		if cmd.param1 <> 12345 then
			Exit(ChanCallBack); 			{ Skip err callBackCmd w/ System 6.0.4 }
		oldA5 := LoadA5(cmd.param2);		{ get the application's A5 and set it }
		soundPlaying := FALSE;
		soundPriority := noSound;
		oldA5 := LoadA5(oldA5);			{ restore old A5 }
	end;

{=================================}

	procedure DoTheSound;
		var
			aCommand: SndCommand;
			theSnd: Handle;
			err: OSErr;
	begin
		if (not soundOn) then
			Exit(DoTheSound);

		if (priority < soundPriority) then
			Exit(DoTheSound);
		soundPriority := priority;

		theSnd := GetNamedResource('snd ', whichOne);

		if (theSnd^ = nil) then
			Exit(DoTheSound);

		MoveHHi(theSnd);
		HLock(theSnd);

		if (chanPtr <> nil) then
			begin
				with aCommand do
					begin
						cmd := quietCmd;
						param1 := 0;
						param2 := 0;
					end;
				err := SndDoImmediate(chanPtr, aCommand);
				err := SndDisposeChannel(chanPtr, TRUE);
				chanPtr := nil;
			end;

		err := SndNewChannel(chanPtr, 0, 0, @ChanCallBack);
		if (err = noErr) then
			err := SndPlay(chanPtr, theSnd, TRUE);
		with aCommand do
			begin
				cmd := callBackCmd;
				param1 := 12345;
				param2 := GetA5;
			end;
		if (err = noErr) then
			err := SndDoCommand(chanPtr, aCommand, FALSE);
		if (err = noErr) then
			soundPlaying := TRUE;

		HUnlock(theSnd);
	end;

{=================================}

	function DoRandom;
		var
			rawResult: LongInt;
	begin
		rawResult := Abs(Random);						{Straight out of "Macintosh Revealed."		}
		DoRandom := (rawResult * range) div 32768	{Use it in your programs (esp. games).		}
	end;

{=================================}

	procedure ResizeARect;
		var
			leftIs, topIs: Integer;
	begin
		leftIs := whichRect.left;							{This is a routine to automatically resize	}
		topIs := whichRect.top;								{any rectangle passed to it.  For example,	}
		whichRect := objectsRects[whatNumber, 0];	{a burning glider is > than a nonburning one.}
		OffsetRect(whichRect, -whichRect.left + leftIs, -whichRect.top + topIs);
	end;

{=================================}

	procedure ResetGliderPos;
	begin
		SetRect(playerDropRect, 0, 0, 0, 0);	{Here we set up the initial position of the glider	}
		SetRect(shadowDropRect, 0, 0, 0, 0);	{We first predefine the rects at 0,0,0,0 ... 		}
		if (gliderCraft) then
			begin
				ResizeARect(playerDropRect, 0);		{...and then we rely on ResizeARect to set the		}
				ResizeARect(shadowDropRect, 53);	{right side and bottom of the rectangle proper.		}
			end
		else
			begin
				ResizeARect(playerDropRect, 2);		{The dart is larger and thus will have a larger		}
				ResizeARect(shadowDropRect, 54);	{bounding rectangle.									}
			end;
		OffsetRect(playerDropRect, startHori, startVert);		{Note that before being offset, the	}
		OffsetRect(shadowDropRect, startHori, shadowVert);	{rectangles still reside in the upper	}
		oldPlayerRect := playerDropRect;							{left hand corner of the screen.		}
		oldShadowRect := shadowDropRect;		{Here we've set the old rects equal to the new.	}
	end;

{=================================}

	procedure Init_GliderMain;
	begin
		MainWindow := nil;					{Cute, huh?  Prototyper does this.				}
	end;

{=================================}

	procedure Close_GliderMain;
	begin
		DisposeWindow(MainWindow);		{Clear window and controls						}
		MainWindow := nil;					{Make sure other routines know we are open	}
	end;

{=================================}

	procedure DrawAnObject;						{This routine is specifically designed for drawing	}
		var												{the various furniture items.  Except for the case	}
			tempRect, tempRect2, tempRect3: Rect;	{statement, the CopyBits & CopyMask routines	}
	begin												{Are pretty generic.  These aren't for the flicker-}
		case whichObject of							{free animation we'll be doing when the game is	}
			0..6, 8..13: 										{underway, these calls simply draw the various	}
				begin											{objects from the objectMap to the virginMap.		}
					SetRect(tempRect, 0, 0, 0, 0);			{In effect, the objects become part of the back	-	}
					ResizeARect(tempRect, whichObject);	{ground.  Note: none of these procedures are seen	}
					OffsetRect(tempRect, hori, vert);		{by the player - we're copying from offscreen to	}
					CopyMask(objectsMap, objectsMap, virginMap, objectsRects[whichObject, 0], objectsRects[whichObject, 1], tempRect);
				end;											{offscreen.  Candles are the exception.				}
			7, 14: 		{Candles get special treatment}
				begin
					SetRect(tempRect, 0, 0, 0, 0);			{Why special?  We have a flickering flame to deal	}
					ResizeARect(tempRect, whichObject);	{with.  So, we have to CopyBits the bits behind	}
					OffsetRect(tempRect, hori, vert);		{the flame directly to the onscreen (thus hiding it)}
					tempRect2 := tempRect;					{And then redraw the flame in it's "newly flick-	}
					OffsetRect(tempRect2, 0, -5);			{ered" location.  It's a rectangle nightmare!		}
					tempRect2.bottom := tempRect2.top + 20;
					CopyBits(virginMap, mainWindow^.portBits, tempRect2, tempRect2, srcCopy, nil);
					CopyMask(objectsMap, objectsMap, mainWindow^.portBits, objectsRects[whichObject, 0], objectsRects[whichObject, 1], tempRect);
				end;
			99:		{this is the special case when the candle is first drawn - only}
				begin
					SetRect(tempRect, 0, 0, 0, 0);			{We do however want something of the candle to	}
					ResizeARect(tempRect, 7);				{be represented in the virginMap.  Otherwise, the	}
					OffsetRect(tempRect, hori, vert);		{candles won't be there when the room is first		}
					tempRect.top := tempRect.top + 13;		{revealed to the player (because we just CopyBits}
					tempRect2 := objectsRects[7, 0];		{the entire virginMap to the mainWindow).  So,	}
					tempRect2.top := tempRect2.top + 13;	{we draw the candle minus any specific flame to	}
					tempRect3 := objectsRects[7, 1];		{the virginMap.  Again, some sloppy rectangle		}
					tempRect3.top := tempRect3.top + 13;	{manipulation is required (to lose the flame).		}
					CopyMask(objectsMap, objectsMap, virginMap, tempRect2, tempRect3, tempRect);
				end;
			otherwise
				begin
				end;
		end;		{case whichObject of}
	end;

{=================================}

	procedure SetUpFurniture;
		var
			Index, Ind2, xPos, yPos, anotherPos, addIt: Integer;
			tempRect: Rect;
			tempPoly: PolyHandle;
	begin
		candleThere := FALSE;									{Initially, there's no candle in the room	}
		numberOfCandles := 0;									{And, initially, they total zero.			}
		totalObjects := levelArray[levelOn, 0, 1];			{Get # of objects in room from array	}
		addIt := 0;												{Additional items begins at zero.			}
		for Index := 1 to totalObjects do					{Now loop thru for each object in room	}
			begin
				Ind2 := Index;											{We may need a second index				}
				case levelArray[levelOn, Index, 1] of			{One big long case statement handles	 all}
					1: 														{the objects in the room.					}
						begin												{1= a Wall socket							}
							objectArray[Ind2] := 2;						{2=glider will burn if in contact with	}
							xPos := levelArray[levelOn, Index, 2];		{Extract the horizontal and vertical		}
							yPos := levelArray[levelOn, Index, 3];		{from the data array for this room.		}
							DrawAnObject(3, xPos, yPos);				{Call drawing routine giving #, h and v.	}
							SetRect(eventArray[Ind2], xPos, yPos, xPos + 16, yPos + 24);	{Define it's rect	}
						end;
					2: 
						begin												{Floor vent									}
							objectArray[Ind2] := 1;						{1=glider will be blown upwards					}
							xPos := levelArray[levelOn, Index, 2];		{get vent's hori. pos. from data array	}
							DrawAnObject(4, xPos, 290);					{object 4=the floor vent					}
							SetRect(eventArray[Ind2], xPos + 12, levelArray[levelOn, Index, 3], xPos + 28, 295);
						end;
					3: 
						begin												{Ceiling suction vent						}
							objectArray[Ind2] := 7;						{7=glider is sucked back a few rooms	}
							xPos := levelArray[levelOn, Index, 2];		{get vent's hori. pos. from data array	}
							SetRect(eventArray[Ind2], xPos, 0, xPos + 48, 28);
							DrawAnObject(5, xPos, 10);					{object 5=the ceiling suction vent				}
						end;
					4: 
						begin												{Ceiling blower								}
							objectArray[Ind2] := 6;						{6=glider will be blown downwards		}
							xPos := levelArray[levelOn, Index, 2];
							DrawAnObject(6, xPos, 10);
							SetRect(eventArray[Ind2], xPos + 12, 20, xPos + 28, levelArray[levelOn, Index, 3]);
						end;
					5: 
						begin												{Candle										}
							candleThere := TRUE;
							numberOfCandles := numberOfCandles + 1;
							objectArray[Ind2] := 2;						{this area will cause glider to catch fire}
							xPos := levelArray[levelOn, Index, 2];
							yPos := levelArray[levelOn, Index, 3];
							candleInfo[numberOfCandles, 0] := xPos;			{Keep a global variable on the candle's	}
							candleInfo[numberOfCandles, 1] := yPos - 34;		{position and "mode".  This way we can	}
							candleInfo[numberOfCandles, 2] := DoRandom(6);	{redraw the "flickering flame" later.	}
							SetRect(eventArray[Ind2], xPos + 8, yPos - 45, xPos + 16, yPos);
							addIt := addIt + 1;								{candles have 2 rects, so keep track of extra	}
							objectArray[totalObjects + addIt] := 1;		{this rectangle (if intersected) will lift glider	}
							SetRect(eventArray[totalObjects + addIt], xPos + 8, yPos - 100, xPos + 16, yPos - 65);
							DrawAnObject(99, xPos, yPos - 34)			{7 tells the procedure to draw a candle			}
						end;
					6: 
						begin												{Clock													}
							objectArray[Ind2] := 8;						{8=player gets a bonus in her/his score			}
							xPos := levelArray[levelOn, Index, 2];		{Get horizontal and vertical position from the	}
							yPos := levelArray[levelOn, Index, 3];		{data array (levelArray).							}
							DrawAnObject(8, xPos, yPos - 28);			{8 means draw the clock							}
							SetRect(eventArray[Ind2], xPos, yPos - 23, xPos + 23, yPos);
						end;
					7: 
						begin												{Folded paper										}
							objectArray[Ind2] := 9;						{9=player gets an extra glider for touching it	}
							xPos := levelArray[levelOn, Index, 2];		{note: the first dimension of the array is 		}
							yPos := levelArray[levelOn, Index, 3];		{indexed by the "levelOn" or room # now at.	}
							DrawAnObject(9, xPos, yPos - 21);			{Rectangle 9 in objectsRects = folded paper	}
							SetRect(eventArray[Ind2], xPos, yPos - 20, xPos + 48, yPos);
						end;
					8: 
						begin												{Thermostat}
							objectArray[Ind2] := 10;						{10 means contact will turn on floor vents		}
							xPos := levelArray[levelOn, Index, 2];		{note: the 2nd dimension in levelArray is 		}
							yPos := levelArray[levelOn, Index, 3];		{the # of the object we're on in the room		}
							DrawAnObject(10, xPos, yPos);
							SetRect(eventArray[Ind2], xPos, yPos, xPos + 16, yPos + 22);
							ventLift := 0;										{initially turn the vents off for this room		}
						end;
					9: 
						begin												{Shelf											}
							objectArray[Ind2] := 12;						{crushed if glider intersects with it		}
							xPos := levelArray[levelOn, Index, 2];
							yPos := levelArray[levelOn, Index, 3];
							anotherPos := levelArray[levelOn, Index, 4];
							SetRect(eventArray[Ind2], xPos, yPos, anotherPos, yPos + 4);
							FillRect(eventArray[Ind2], gray);
							FrameRect(eventArray[Ind2]);
							tempPoly := OpenPoly;							{draw shadow under shelf					}
							MoveTo(xPos, yPos + 4);						{start poly in one corner of shelf			}
							LineTo(anotherPos, yPos + 4);				{sweep to right under shelf				}
							LineTo(anotherPos, yPos);						{move up right side a little ways			}
							LineTo(anotherPos + 12, ypos + 12);			{move down diagonally to the right		}
							LineTo(anotherPos + 12, yPos + 16);		{move down tiny bit for shadow of edge	}
							LineTo(xPos + 12, ypos + 16);				{sweep way left for bottom edge of shadow}
							LineTo(xPos, yPos + 4);						{come up diagonally and close off the poly	}
							ClosePoly;										{stop using moves to define poly: close it	}
							FillPoly(tempPoly, dkGray);					{throw the dark fray color into the poly			}
							KillPoly(tempPoly);								{end of shadow drawing					}
							DrawAnObject(11, xPos + 16, yPos + 4);	{draw the shelf brackets last				}
							DrawAnObject(11, anotherPos - 20, yPos + 4)
						end;
					10: 
						begin												{Table}
							objectArray[Ind2] := 12;						{death by being crushed (fragile glider - eh?)	}
							xPos := levelArray[levelOn, Index, 2];
							yPos := levelArray[levelOn, Index, 3];
							anotherPos := levelArray[levelOn, Index, 4];
							SetRect(eventArray[Ind2], xPos, yPos, anotherPos, yPos + 7);
							FillRect(eventArray[Ind2], ltgray);
							FrameRect(eventArray[Ind2]);
							SetRect(tempRect, xPos, 305 - ((anotherPos - xPos) div 20), anotherPos, 305 + ((anotherPos - xPos) div 20));
							PenMode(srcOr);
							FillOval(tempRect, black);
							PenNormal;
							DrawAnObject(12, (xPos + anotherPos) div 2 - 24, 297);
							SetRect(tempRect, (xPos + anotherPos) div 2 - 2, yPos + 7, (xPos + anotherPos) div 2 + 1, 303);
							FillRect(tempRect, black)
						end;
					11: 
						begin												{Light switch}
							objectArray[Ind2] := 11;   {light switch has been encountered}
							switchH := levelArray[levelOn, Index, 2];
							switchV := levelArray[levelOn, Index, 3];
							DrawAnObject(13, switchH, switchV);
							SetRect(eventArray[Ind2], switchH, switchV, switchH + 16, switchV + 22);
							darkOn := TRUE;
						end;
				end  {end of case }
			end;   {end of for loop}
		totalObjects := totalObjects + addIt;	{Make sure totalObjects reflects the additional rects	}
	end;											{due to candles dual rects (one burns & one lifts).		}

{=================================}

	procedure FoldTheGlider;
		var
			tempRect, tempRect2: Rect;
			tempString: Str255;
			index: Integer;
			dummyLong: LongInt;
	begin
		SetPort(loadPort);						{A tedious animation sequence to give us that wonderful	}
		PenNormal;								{folding plane.  First, we must set port to the loadPort	}
		PenSize(2, 2);							{to draw the white and framed rectangle that will slip	}
		SetRect(tempRect, 0, 10, 80, 70);	{onto the scene.  We define it first...		}
		EraseRect(tempRect);					{Make it white (erase it's contents)...	}
		FrameRect(tempRect);					{Frame it with the 2 x 2 pen size...		}
		SetRect(tempRect2, 0, 0, 0, 0);		{Define a blank "drop rect" for the sheet of paper...		}
		ResizeARect(tempRect2, 41);			{And call ResizeARect to get it to the proper dimensions	}
		OffsetRect(tempRect2, 20, 30);		{And now we move it to lie in the center of the 1st rect	}
		CopyBits(objectsMap, loadMap, objectsRects[41, 0], tempRect2, srcCopy, nil);	{Draw it	}
		MoveTo(25, 25);						{And then align the pen - here comes the text.				}
		TextFont(0);								{Set font to the everyone-has Chicago 12					}
		TextSize(12);
		NumToString(gliderNumber, tempString);		{Convert the # of glider to a character string	}
		tempString := CONCAT(tempString, ' left');		{Add the characters ' left' (as in '5 left')		}
		DrawString(tempString);							{And finally draw it onto the first rectangle	}
		tempRect2 := tempRect;				{We have now assembled the art on the loadMap and can	}
		OffsetRect(tempRect2, -80, 0);		{scroll it out.  So first set up the drop rect (way left)	}
		for index := 1 to 39 do				{and now we'll move the drop rect right, copy the art,	}
			begin									{move the drop rect a little more right, copy the art,	}
				OffsetRect(tempRect2, 2, 0);		{etc... until the whole image has "scrolled" into view.	}
				CopyBits(loadMap, mainWindow^.portBits, tempRect, tempRect2, srcCopy, mainWindow^.visRgn);
			end;
		SetRect(tempRect2, 0, 0, 0, 0);		{Now that the "little window" appears on the left side of	}
		ResizeARect(tempRect2, 41);			{the screen, we can begin erasing it, drawing the more-	}
		OffsetRect(tempRect2, 18, 30);		{completely-folded glider, erase it, draw the more....	}
		SetPort(mainWindow);					{But first set the port to the on screen so that EraseRect}
		InsetRect(tempRect, 2, 2);				{has a visible effect (else we would be erasing the		}
		tempRect.top := tempRect.top + 17;	{area in the loadMap and not causing any visible erasing).}
		tempRect.right := tempRect.right - 2;	{We rearrange tempRect's size so as not to erase the	}
		for index := 41 to 52 do				{'5 left' message when we call EraseRect.					}
			begin
				Delay(6, dummyLong);				{Pause...	}
				ResizeARect(tempRect2, index);	{Get the proper size rect for the more-folded glider...	}
				if (index = 49) then
					OffsetRect(tempRect2, 0, 11);	{Folding phase 49, 50 and 52 need special adjusting.		}
				if (index = 50) then					{Otherwise, the folding glider would jerk around on		}
					OffsetRect(tempRect2, 0, -1);	{the screen (the upper-left corner of the bounding rects	}
				if (index = 52) then					{don't line up with the previous phases).					}
					OffsetRect(tempRect2, 0, -3);
				EraseRect(tempRect);				{Here we go.  Erase all but the '5 left' (or '4 left' etc..).	}
				CopyBits(objectsMap, mainWindow^.portBits, objectsRects[index, 0], tempRect2, srcCopy, nil);
				DoTheSound('fold.snd', highPriority);		{And call the folding sound up "fwip"!						}
			end;
		Delay(10, dummyLong);				{When done folding the glider, wait 1/3 of a second...		}
		tempRect.top := tempRect.top - 17;	{Pop that tempRect back to it's original size...				}
		tempRect.right := tempRect.right + 2;{Enlarge it to include the "frame" we drew and then		}
		InsetRect(tempRect, -3, -3);			{CopyBits from the virginMap to screen (cover it all up).}
		CopyBits(virginMap, mainWindow^.portBits, tempRect, tempRect, srcCopy, nil);
	end;

{=================================}

	procedure UpDate_GliderMain;
		var
			whichBack: Integer;
			Pic_Handle: PicHandle;
			tempRect: Rect;
	begin
		SetPort(virginPort);
		EraseRect(virginPort^.portRect);
		whichBack := backgroundArray[levelOn];
		SetRect(tempRect, 0, 0, 512, 322);
		ClipRect(tempRect);
		Pic_Handle := GetPicture(whichBack);		{Get Picture into memory			}
		if (Pic_Handle <> nil) then					{Only use handle if it is valid		}
			begin
				HLock(Handle(Pic_Handle));					{Lock the handle before using it	}
				tempRect := Pic_Handle^^.picFrame;
				DrawPicture(Pic_Handle, tempRect);		{And draw it to the virginMap		}
				ReleaseResource(Handle(Pic_Handle));		{Free up the memory we had held}
				HUnLock(Handle(Pic_Handle));				{Unlock the picture again}
			end
		else
			SysBeep(3);
		SetRect(tempRect, 0, 0, 1023, 1023);		{Widen the clip area again			}
		ClipRect(tempRect);								{Set the clip area					}
		SetUpFurniture;									{Draw the furniture there also..	}
		if (darkOn) and (switchUntouched) then
			begin
				FillRect(virginMap.bounds, black);			{Unless the lights are off, then		}
				DrawAnObject(13, switchH, switchV);	{we want blackness and the switch.	}
			end;
{And finally dump the whole virginMap collage to the mainWindow for player's perusal}
		CopyBits(virginMap, mainWindow^.portBits, virginMap.bounds, mainWindow^.portRect, srcCopy, mainWindow^.visRgn);
		if (levelOn = 15) then
			begin
				CopyBits(virginMap, loadMap, catBodyRect, catBodyRect, srcCopy, nil);
				CopyMask(objectsMap, objectsMap, loadMap, objectsRects[28, 0], objectsRects[28, 1], catBodyRect);
				CopyBits(loadMap, mainWindow^.portBits, catBodyRect, catBodyRect, srcCopy, nil);
			end;
	end;

{=================================}

	procedure Open_GliderMain;
	begin
		if (mainWindow = nil) then		{Handle an open when already opened	}
			begin
				mainWindow := GetNewWindow(1, nil, Pointer(-1));
				MoveWindow(mainWindow, (screenBits.bounds.right - 512) div 2, (screenBits.bounds.bottom - 342) div 2 + 20, FALSE);
				ShowWindow(mainWindow);
				SelectWindow(mainWindow);
				SetPort(mainWindow);
				UpDate_GliderMain;
			end									{End for if (mainWindow<>nil)				}
		else
			SelectWindow(mainWindow);		{Already open, so show it					}
	end;

{=================================}

	procedure ReDrawHiScores;
		var
			index: Integer;
			tempRect, dummyRect, dummyRect2, dummyRect3: Rect;
			dummyString: Str255;
	begin
		SetPort(loadPort);						{Set all drawing operations to go to the load Port offscreen	}
		EraseRect(loadMap.bounds);			{What we're going to do here is move a copy of the glider	}
		SetRect(dummyRect, 0, 0, 0, 0);		{with the banner to the load port from the object port, then	}
		ResizeARect(dummyRect, 34);		{we'll write a message on it there and animate the whole	}
		CopyBits(objectsMap, loadMap, objectsRects[34, 0], dummyRect, srcCopy, nil);
		PenNormal;								{banner-with-message across the "on-screen".				}
		TextFont(0);								{First we'll animate the one moving across the bottom with	}
		TextSize(12);							{my name on it, then we'll animate one higher up the screen	}
		MoveTo(60, 14);						{with the High Scores message.  Finally we'll draw a white	}
		dummyString := '© 1989 - 93 john calhoun';
		DrawString(dummyString);			{rectangle between the two and write all the hi scores on	}
		dummyRect2 := dummyRect;			{it.  This kind of stuff is time consuming to write and takes	}
		OffsetRect(dummyRect2, 512, 229);{up a good chunk of memory, but for the final product, but	}
		dummyRect3 := dummyRect2;
		dummyRect2.right := dummyRect2.right + 10;		{it seems to be worth it.							}
		for index := 90 downto 0 do			{These steps I'm passing over are all repeated and			}
			begin									{documented below.  It's the same routine.						}
				OffsetRect(dummyRect2, -3, 0);
				OffsetRect(dummyRect3, -3, 0);
				CopyBits(virginMap, loadMap, dummyRect2, dummyRect2, srcCopy, nil);
				CopyMask(loadMap, objectsMap, loadMap, dummyRect, objectsRects[34, 1], dummyRect3);
				CopyBits(loadMap, mainWindow^.portBits, dummyRect2, dummyRect2, srcCopy, mainWindow^.visRgn);
			end;
		SetRect(dummyRect, 0, 0, 0, 0);			{Set up an initially null rectangle								}
		ResizeARect(dummyRect, 34);			{Pass to ResizeARect to get the proper dimensions		}
		CopyBits(objectsMap, loadMap, objectsRects[34, 0], dummyRect, srcCopy, nil);	{Copy to load	}
		MoveTo(60, 14);							{Get ready to write on the banner now in the load port	}
		dummyString := 'Glider+ 3.14 High Scores...';	{Set dummyString to this string on banner			}
		DrawString(dummyString);				{Draw it															}
		dummyRect2 := dummyRect;				{We'll need another ("drop") rect that's the same size	}
		OffsetRect(dummyRect2, 512, 61);		{Start "drop" rect all the way right and down 61 pixels	}
		dummyRect3 := dummyRect2;
		dummyRect2.right := dummyRect2.right + 10;		{Add extra pixels to cover trail left behind		}
		for index := 0 to 90 do					{This loop will animate the glider-with-banner across	}
			begin										{the top of the screen.											}
				OffsetRect(dummyRect2, -3, 0);		{Move the "drop" rect 3 pixels to left every loop			}
				OffsetRect(dummyRect3, -3, 0);		{Also move this "drop" rect.  MacII's scale CopyMask	}
				CopyBits(virginMap, loadMap, dummyRect2, dummyRect2, srcCopy, nil);
				CopyMask(loadMap, objectsMap, loadMap, dummyRect, objectsRects[34, 1], dummyRect3);
				CopyBits(loadMap, mainWindow^.portBits, dummyRect2, dummyRect2, srcCopy, mainWindow^.visRgn);
			end;
		SetPort(mainWindow);		{Make all drawing operations go to the screen		}
		PenNormal;					{Reset the pen to normal in case it's wigged out	}
		PenSize(2, 2);				{Now make a thicker pen for a bolder line			}
		TextFont(0);					{Set font to Chicago (everyone has this font)		}
		TextSize(12);				{Make it 12 point in size so that it isn't cruddy	}
		SetRect(tempRect, 310, 80, 512, 232);	{Define rectangle for scores		}
		EraseRect(tempRect);		{Erase this area on the screen (tabla rasa)			}
		MoveTo(310, 80);			{Move the pen to the top left edge of the rect		}
		LineTo(310, 230);			{Draw a line down along the left edge of the rect	}
		for index := 1 to 10 do	{Now we're going to loop through all the scores	}
			begin
				MoveTo(320, index * 15 + 77);			{Moving the pen down each time	}
				DrawString(hiStrings[index]);				{Draw the name of person		}
				MoveTo(450, index * 15 + 77);			{Move pen over to the right		}
				NumToString(hiScores[index], dummyString);
				DrawString(dummyString);					{And draw their score			}
			end;							{Copy the screen to the virgin port for updates	}
		CopyBits(mainWindow^.portBits, virginMap, screenArea, screenArea, srcCopy, nil);
	end;

{=================================}

	procedure ReadInScores;

		type
			scoreHandle = ^scorePtr;
			scorePtr = ^score;
			score = record
					data: array[0..9] of LongInt;
				end;

			nameHandle = ^namePtr;
			namePtr = ^name;
			name = record
					data: array[0..9, 0..14] of Char;
				end;

			prefHandle = ^prefPtr;
			prefPtr = ^pref;
			pref = record
					isFast, isNoisy: Boolean;
					isLeft, isRight: Char;
					isName: string[15];
				end;

		var
			index, index2, tempVolume: Integer;
			dummyStr: Str255;
			theScores: scoreHandle;
			theNames: nameHandle;
			thePrefs: prefHandle;

	begin
		theScores := scoreHandle(NewHandle(SIZEOF(score)));
		MoveHHi(Handle(theScores));
		HLock(Handle(theScores));
		Handle(theScores) := GetResource('scrs', 128);
		for index := 0 to 9 do
			hiScores[index + 1] := theScores^^.data[index];
		HUnlock(Handle(theScores));
		ReleaseResource(Handle(theScores));
		DisposHandle(Handle(theScores));

		theNames := nameHandle(NewHandle(SIZEOF(name)));
		MoveHHi(Handle(theNames));
		HLock(Handle(theNames));
		Handle(theNames) := GetResource('name', 128);
		for index := 0 to 9 do
			begin
				dummyStr := '';
				for index2 := 0 to 14 do
					dummyStr := CONCAT(dummyStr, theNames^^.data[index, index2]);
				hiStrings[index + 1] := dummyStr;
			end;
		HUnlock(Handle(theNames));
		ReleaseResource(Handle(theNames));
		DisposHandle(Handle(theNames));

		thePrefs := prefHandle(NewHandle(SIZEOF(pref)));
		MoveHHi(Handle(thePrefs));
		HLock(Handle(thePrefs));
		Handle(thePrefs) := GetResource('pref', 128);

		if ((thePrefs <> nil) and (resError = noErr)) then
			begin

				with thePrefs^^ do
					begin
						if (isFast) then
							gameSpeed := fastSpeed
						else
							gameSpeed := slowSpeed;

						if (isNoisy) then
							soundOn := TRUE
						else
							soundOn := FALSE;
						if (inhibitSound) then
							soundOn := FALSE;
						GetSoundVol(tempVolume);
						if (tempVolume = 0) then
							soundOn := FALSE;

						leftControl := isLeft;
						rightControl := isRight;

						nameUsing := isName;
					end;

				HUnlock(Handle(thePrefs));
				ReleaseResource(Handle(thePrefs));
				DisposHandle(Handle(thePrefs));
			end
		else
			begin
				gameSpeed := fastSpeed;
				soundOn := not inhibitSound;
				leftControl := ',';
				rightControl := '.';
				nameUsing := 'Anonymous      ';
			end;
	end;

{=================================}

	procedure WriteOutScores;
		type
			scoreHandle = ^scorePtr;
			scorePtr = ^score;
			score = record
					data: array[0..9] of LongInt;
				end;

			nameHandle = ^namePtr;
			namePtr = ^name;
			name = record
					data: array[0..9, 0..14] of Char;
				end;

			prefHandle = ^prefPtr;
			prefPtr = ^pref;
			pref = record
					isFast, isNoisy: Boolean;
					isLeft, isRight: Char;
					isName: string[15];
				end;

		var
			index, index2: Integer;
			dummyStr: Str255;
			theScores: scoreHandle;
			theNames: nameHandle;
			thePrefs: prefHandle;

	begin
		thePrefs := prefHandle(NewHandle(SIZEOF(pref)));
		HLock(Handle(thePrefs));
		Handle(thePrefs) := GetResource('pref', 128);

		if ((thePrefs = nil) or (ResError <> noErr)) then
			AddResource(Handle(thePrefs), 'pref', 128, 'added');

		with thePrefs^^ do
			begin
				isFast := (gameSpeed = fastSpeed);

				isNoisy := soundOn;

				isLeft := leftControl;
				isRight := rightControl;

				isName := COPY(nameUsing, 1, 15);
			end;

		ChangedResource(Handle(thePrefs));
		WriteResource(Handle(thePrefs));
		HUnlock(Handle(thePrefs));
		ReleaseResource(Handle(thePrefs));
		DisposHandle(Handle(thePrefs));

		ChangedResource(Handle(thePrefs));
		WriteResource(Handle(thePrefs));
		HUnlock(Handle(thePrefs));
		ReleaseResource(Handle(thePrefs));
		DisposHandle(Handle(thePrefs));

		theScores := scoreHandle(NewHandle(SIZEOF(score)));
		MoveHHi(Handle(theScores));
		HLock(Handle(theScores));
		Handle(theScores) := GetResource('scrs', 128);
		for index := 0 to 9 do
			theScores^^.data[index] := hiScores[index + 1];
		ChangedResource(Handle(theScores));
		WriteResource(Handle(theScores));
		HUnlock(Handle(theScores));
		ReleaseResource(Handle(theScores));
		DisposHandle(Handle(theScores));

		theNames := nameHandle(NewHandle(SIZEOF(name)));
		HLock(Handle(theNames));
		Handle(theNames) := GetResource('name', 128);
		for index := 0 to 9 do
			for index2 := 0 to 14 do
				theNames^^.data[index, index2] := COPY(hiStrings[index + 1], index2 + 1, 1);
		ChangedResource(Handle(theNames));
		WriteResource(Handle(theNames));
		HUnlock(Handle(theNames));
		ReleaseResource(Handle(theNames));
		DisposHandle(Handle(theNames));
	end;


{=================================}

end.