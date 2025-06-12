{===============================================}
{================= Player sprite unit ================}
{===============================================}

{ Example file for Ingemars Sprite Animation Toolkit. }
{ © Ingemar Ragnemalm 1992 }
{ See doc files for legal terms for using this code. }

{ This file is the first of several sprite units, units that holds the full}
{description of the objects to be animated. }

unit sPlayer;

{ Sprite unit. A sprite unit should include the following routines:}
{ Init-procedure, that initializes private bitmaps}
{ Setup-procedure, that sets variables other than the standard ones set by MakeSprite }
{ Handle-procedure, to be called once per iteration until the sprite dies }
{ Hittask-procedure (optional), for advanced collission handling. }

{ This is the sprite unit for the player object, in this case a butterfly. }

interface

	uses
		GameGlobals, SAT, SoundConst, sShot;

	var
		stillRunning: boolean;

	procedure InitPlayer;
	procedure SetupPlayer (player: SpritePtr);
	procedure HandlePlayer (me: SpritePtr);

implementation

	const
		playerspeed = 16; {How fast may the player object move per frame, max?}

	var
		playerFace: FacePtr;

	procedure InitPlayer;
	begin
		playerFace := GetFace(134); {Preload the player face (from cicn resource)}
	end;

	procedure SetupPlayer (player: SpritePtr);
	begin
		player^.face := playerFace;
		SetRect(player^.hotRect, 1, 7, 31, 32);
	end;

	procedure HandlePlayer (me: SpritePtr);
		var
			pt: point;
			shot: SpritePtr;
	begin
{We detect collisions with change in "kind". We could also have chosen to do this}
{with a "hitTask", a callback proceure.}
		if me^.kind <> 2 then
			begin
				SATSoundPlay(kraschH, 10, true);		{Play a sound}
				stillrunning := false;						{Tell MoveIt that the game is over, to quit the game loop.}
{ Real games make an explosion before quitting!}
			end;

{Where is the mouse pointer?}
		SATSetPortScreen;
		GetMouse(pt);

		me^.speed.h := pt.h - me^.position.h; {How far from the previous position?}

{Change the position to the new position, but not by more than platerspeed!}
		if me^.speed.h < -playerspeed then
			me^.position.h := me^.position.h - playerspeed
		else if me^.speed.h > playerspeed then
			me^.position.h := me^.position.h + playerspeed
		else
			me^.position.h := me^.position.h + me^.speed.h;

{Make sure we don't go out of sight.}
		if me^.position.h > gSAT.offSizeH - 32 then
			begin
				me^.position.h := gSAT.offSizeH - 32;
			end;
		if me^.position.h < 0 then
			begin
				me^.position.h := 0;
			end;

{ Create shots }
		if me^.mode > 0 then
			me^.mode := pred(me^.mode);
		if me^.mode = 0 then {We may only shoot every 10 frames!}
			if Button then
				begin
					shot := NewSprite(1, me^.position.h + 12, me^.position.v, @HandleShot, @SetupShot, nil);
					me^.mode := 10;
					SATSoundPlay(toffH, 1, false);
				end;
	end;

end.