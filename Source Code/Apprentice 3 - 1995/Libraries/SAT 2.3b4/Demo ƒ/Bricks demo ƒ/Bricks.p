{Bricks}
{}
{Demo program for SAT 2.3, by Ingemar Ragnemalm november 1994}
{}
{The purpose of this demo is to test and demonstrate the support for what I call "resting" sprites.}
{If a sprite neither moves, changes face, overlaps a changing sprite nor changes its place in the}
{sprite list, then SAT can avoid drawing it.}
{}
{For programs where all sprites change in every frame (which is the case in many arcade games)}
{then this is not of any interest at all, and will just slow things down. For other programs, for}
{example board games, Tetris-style games etc, this technique is very useful.}
{}
{For the moment (SAT 2.3d3), you get the new system by calling SATRun2 instead of SATRun. The name}
{of SATRun2 is likely to change in the future.}
{}
{When you start Bricks, the sprites will not be initialied in order, so since VPositionSort}
{is active, they will sort during the first frames. This slows the program down for the first}
{few seconds. If this is a problem, it can be avoided by either turning off sorting or}
{creating the sprite in the proper places from the start.}
{}
{Bug note: When using "fast graphics", the cursor will cause some "mouse droppings". This is best avoided}
{by hiding the cursor and repllacing it with a SAT sprite. The "proper" way to avoid the problem is to}
{call ShieldCursor, but that will make the cursor flicker a lot.}

program Bricks;
	uses
{$ifc UNDEFINED THINK_PASCAL}
		Types, QuickDraw, Events, Windows, Dialogs, Fonts, DiskInit, TextEdit, Traps,{}
		Memory, SegLoad, Scrap, ToolUtils, OSUtils, Menus, Resources, StandardFile,{}
		GestaltEqu, Files, Errors, Devices, 
{$elsec}
		InterfacesUI, 
{$endc}
		SAT;

	const
		kNumBricks = 30;
	var
		brickFace: array[0..kNumBricks] of FacePtr;
		myEvent: EventRecord;
		whichWindow: WindowPtr;
		appleMenu, fileMenu: MenuHandle;
		gUseStagger, gDone, gUseFast, gAllowBackground: Boolean;
		i: integer;
		sp, found: SpritePtr;
		theSelection: Longint;
		where: Point;
		theKey: Char;
		whichPart: Integer;
		hasEvent: Boolean;

	procedure InitBricks;
		var
			i: integer;
	begin
		for i := 0 to kNumBricks - 1 do
			brickFace[i] := SATGetFace(i + 128);
	end;

	procedure HandleRestingBrick (me: SpritePtr);
	begin
	end;

	procedure HandleFollowMouse (me: SpritePtr);
		var
			mousePos: Point;
	begin
		if Button then
			begin
				GetMouse(mousePos);
				me^.position.h := me^.position.h + mousePos.h - me^.speed.h;
				me^.position.v := me^.position.v + mousePos.v - me^.speed.v;
				me^.speed := mousePos;
			end
		else
			me^.task := @HandleRestingBrick;
	end;

	procedure SetupBrick (me: SpritePtr);
	begin
		me^.task := @HandleRestingBrick;
		me^.face := brickFace[me^.kind];
	end;

	procedure SynchMenus;
	begin
		CheckItem(fileMenu, 1, not gUseStagger);
		CheckItem(fileMenu, 2, gUseStagger);
		CheckItem(fileMenu, 3, gUseFast);
		CheckItem(fileMenu, 4, gAllowBackground);
	end;

	procedure MenuSelection (theSelection: Longint);
		var
			name: Str255;
			saveport: GrafPtr;
	begin
		case HiWord(theSelection) of
			128: 
				begin
					if LoWord(theSelection) = 1 then
						SATReportStr('Experimental SAT demo program. Resting sprites allow good speed with many sprites!')
					else
						begin
							GetPort(saveport);
							GetMenuItemText(appleMenu, 1, name);	(* get name *)
							if OpenDeskAcc(name) = 0 then (* run the desk accessory *)
								;
							SetPort(saveport);
						end;
				end;
			129: 
				case LoWord(theSelection) of
					1: 
						gUseStagger := false;
					2: 
						gUseStagger := true;
					3: 
						gUseFast := not gUseFast;
					4: 
						gAllowBackground := not gAllowBackground;
					6: 
						gDone := true;
				end; {case MenuSelect}
			otherwise
		end;
		SynchMenus;
		HiLiteMenu(0);
	end;

begin
{CodeWarrior needs inits here}
{$IFC UNDEFINED THINK_PASCAL}
	SATInitToolbox;
{$ENDC}

	InitBricks;

	SATInit(128, 129, 512, 342);
	SATSetPortScreen;
	gUseStagger := true;
	gAllowBackground := true;
	gUseFast := false;

	for i := 0 to kNumBricks - 1 do
		sp := SATNewSprite(i, SATRand(gSAT.offSizeH - 48), SATRand(gSAT.offSizeV - 64), @SetupBrick);
	for i := 0 to kNumBricks - 1 do
		sp := SATNewSprite(i, SATRand(gSAT.offSizeH - 48), SATRand(gSAT.offSizeV - 64), @SetupBrick);

	appleMenu := NewMenu(128, stringof(char($14)));
	AppendMenu(appleMenu, 'About Bricks demo…;(-');
	AppendResMenu(appleMenu, 'DRVR');
	InsertMenu(appleMenu, 0);			{ put apple menu at end of menu bar }

	fileMenu := NewMenu(129, 'File');
	AppendMenu(fileMenu, 'RunSAT/1;RunSAT2/2;Fast graphics/F;Allow background tasks;(-;Quit/Q');
	InsertMenu(fileMenu, 0);			{ put file menu at end of menu bar }
	DrawMenuBar;
	SynchMenus;

	InitCursor;

	repeat
{if gUseFast then}
{ShieldCursor(gSAT.bounds, Point(0));}
		if gUseStagger then
			SATRun2(gUseFast)
		else
			SATRun(gUseFast);
{if gUseFast then}
{ShowCursor;}

		if gAllowBackground then
			begin
				SystemTask;
				hasEvent := GetNextEvent(everyEvent, myEvent)
			end
		else
			hasEvent := GetOSEvent(everyEvent, myEvent);		{or perhaps mDownMask + updateMask + keyDownMask}
		if hasEvent then
			case myEvent.what of
				updateEvt: 
					if WindowPtr(myEvent.message) = gSAT.wind.port then
						begin
							BeginUpdate(gSAT.wind.port);
							SATRedraw;
							EndUpdate(gSAT.wind.port);
						end;
				keyDown: 
					begin
						theKey := char(BitAnd(myEvent.message, charCodeMask));
						if (BitAnd(myEvent.modifiers, cmdKey) <> 0) then
							MenuSelection(MenuKey(theKey))
						else
{DoKey(theKey, theEvent.modifiers)}
							;
					end;
				mouseDown: 
					begin
						whichPart := FindWindow(myEvent.where, whichWindow);
						case whichPart of
							inMenuBar: 
								begin
									theSelection := MenuSelect(myEvent.where);
									MenuSelection(theSelection);
								end;
							inSysWindow: 
								SystemClick(myEvent, whichWindow);
							inContent: 
								begin
									found := nil;
									sp := gSAT.sRoot;
									GetMouse(where);
									while sp <> nil do
										begin
											if PtInRect(where, sp^.r) then
												found := sp;
											sp := sp^.next;
										end;
									if found <> nil then
										if myEvent.modifiers <> 0 then
											found^.task := nil
										else
											begin
												found^.task := @HandleFollowMouse;
												found^.speed := where; {not speed, but storage for old mouse position}
											end;
								end;
							otherwise
						end; {case whichPart}
					end;

				otherwise
			end;{case myEvent.what}
	until gDone;

	SATSoundShutup;
end.