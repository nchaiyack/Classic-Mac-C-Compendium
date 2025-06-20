{===============================================}
{================= Flypaper sprite unit ================}
{===============================================}

{ Example file for Ingemars Sprite Animation Toolkit. }
{ � Ingemar Ragnemalm 1992 }
{ See doc files for legal terms for using this code. }

unit sFlypaper;

{ Sprite unit. A sprite unit should include the following routines:}
{ Init-procedure, that initializes private bitmaps}
{ Setup-procedure, that sets variables other than the standard ones set by MakeSprite }
{ Handle-procedure, to be called once per iteration until the sprite dies }
{ Hittask-procedure (optional), for advanced collission handling. }

{ Flypaper object, the "enemies" in the HeartQuest game. }

interface

	uses
		SAT, scores, SoundConst, GameGlobals;

	var
		slowcount: integer;

	procedure InitFlypaper;
	procedure SetupFlypaper (mp: SpritePtr);
	procedure HandleFlypaper (me: SpritePtr);
	procedure HandleDyingFlypaper (me: SpritePtr); {Beh�ver inte vara extern}

implementation

	var
		FlypaperFace: array[1..6] of FacePtr;
		DyingFlypaperFace: array[1..3] of FacePtr;

	procedure InitFlypaper;
		var
			ii: integer;
			h: Handle;
	begin
		for ii := 1 to 6 do
			FlypaperFace[ii] := GetFace(156 + ii);
		for ii := 1 to 3 do
			DyingFlypaperFace[ii] := GetFace(143 + ii);
	end;

	procedure SetupFlypaper (mp: SpritePtr);
	begin
		mp^.face := FlypaperFace[1];
		mp^.mode := rand(3) + 1; { icon number }
		SetRect(mp^.hotRect, -14 + 16, -28 + 32, 14 + 16, 0 + 32);
	end;

	procedure HandleFlypaper (me: SpritePtr);
		var
			mp: Spriteptr;
			divmed: integer;
	begin
		if me^.kind <> -3 then
			begin
				me^.kind := -3;
				if slowcount < 10 then
					begin
						slowcount := 50;
						SATSoundPlay(Splatt2SndH, 1, true);
						me^.task := @HandleDyingFlypaper;
						me^.kind := 0;
					end;
			end;

		me^.mode := me^.mode + 1;
		if me^.mode = 7 then
			me^.mode := 1;
		me^.face := FlypaperFace[me^.mode];

		me^.position.h := me^.position.h + me^.speed.h;
		me^.position.v := me^.position.v + me^.speed.v;

		if Rand100 < 7 then
			me^.speed.h := rand(15) - 8;
		if Rand100 < 4 then
			me^.speed.v := rand(7) - 4;

		if (level > 4) then
			if rand100 < level then
				if slowcount = 0 then
					begin
						me^.speed.h := PlayerPos.h - me^.position.h;
						me^.speed.v := PlayerPos.v - me^.position.v;
						divmed := abs(me^.speed.h) + abs(me^.speed.v);
						if divmed > level then
							begin
								me^.speed.h := me^.speed.h * 2 * level div divmed;
								me^.speed.v := me^.speed.v * 2 * level div divmed;
							end;
					end
				else {Player stunned - keep away!}
					begin
						me^.speed.h := -(PlayerPos.h - me^.position.h);
						me^.speed.v := -(PlayerPos.v - me^.position.v);
						divmed := abs(me^.speed.h) + abs(me^.speed.v);
						if divmed > level then
							begin
								me^.speed.h := me^.speed.h * 2 * level div divmed;
								me^.speed.v := me^.speed.v * 2 * level div divmed;
							end;
					end;

		if me^.position.h < 0 then
			begin
				me^.position.h := 0;
				me^.speed.h := 1;
			end;
		if me^.position.h > gSAT.offSizeH - xsize then
			begin
				me^.position.h := gSAT.offSizeH - xsize;
				me^.speed.h := -1;
			end;
		if me^.position.v < 0 then
			begin
				me^.position.v := 0;
				me^.speed.v := 1;
			end;
		if me^.position.v > gSAT.offSizeV - 32 then
			begin
				me^.position.v := gSAT.offSizeV - 32;
				me^.speed.v := -1;
			end;
	end;

	procedure HandleDyingFlypaper (me: SpritePtr);
		var
			mp: Spriteptr;
			divmed: integer;
	begin
		if me^.kind <> 0 then
			me^.kind := 0;

		me^.mode := me^.mode + 1;
		if me^.mode >= 4 then
			me^.mode := 1;
		me^.face := DyingFlypaperFace[me^.mode];

		me^.position.v := me^.position.v + 8;

		if me^.position.h > gSAT.offSizeH then
			me^.task := nil;
	end;

end.