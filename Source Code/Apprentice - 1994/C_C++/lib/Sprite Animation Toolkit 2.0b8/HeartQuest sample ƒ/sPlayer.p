{===============================================}
{================= Player sprite unit ================}
{===============================================}

{ Example file for Ingemars Sprite Animation Toolkit. }
{ � Ingemar Ragnemalm 1992 }
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
		GameGlobals, SAT, sHeart, sFlypaper, scores, SoundConst;

	var
		stillRunning: boolean;

	procedure InitPlayer;
	procedure SetupPlayer (player: SpritePtr);
	procedure HandlePlayer (me: SpritePtr);
	procedure HitPlayer (me, him: SpritePtr);

implementation

	const
{playerspeed = 16; { Might become a variable one day }
		shift = 4;

	var
		playerFace: array[0..3] of FacePtr;
		posh, posv: longint;

	procedure InitPlayer;
		var
			ii: integer;
	begin
		for ii := 0 to 3 do
			begin
				playerFace[ii] := GetFace(140 + ii);
			end;
	end;

	procedure SetupPlayer (player: SpritePtr);
	begin
		player^.face := playerFace[0];
		SetRect(player^.hotRect, -15 + 16, -25 + 32, 15 + 16, 0 + 32);
		posh := bsl(player^.position.h, shift);
		posv := bsl(player^.position.v, shift);
		slowcount := 0;
	end;


	procedure HandlePlayer (me: SpritePtr);
		var
			pt: point;
	begin
		if me^.kind <> 2 then
			me^.kind := 2;

		if slowcount > 0 then
			begin
				slowcount := pred(slowcount);
				me^.speed := Point(0);
{SetMouse(Point($00AB0100));???? {256,171}
			end
		else
			begin
{Note about the controls:}
{(posh,posv) is the position scaled up by 16 (2**shift)}
{This is my way to implement fixed-point arithmetics efficiently.}
{If we used only integers, we wouldn't get the smooth movement the}
{bufferfly has.}
				SetPort(gSAT.wind);
				GetMouse(pt);

				if pt.h < 0 then
					pt.h := 0;
				if pt.h > 512 then
					pt.h := 512;
				if pt.v < 0 then
					pt.v := 0;
				if pt.v > 342 then
					pt.v := 342;

				me^.speed.h := 15 * me^.speed.h div 16 + pt.h - 256;
				me^.speed.v := 15 * me^.speed.v div 16 + pt.v - 171;

{If we rather want the direct velocity-from-position that Crystal Quest has,}
{we can use the following:}
{me^.speed.h := pt.h - 256;}
{me^.speed.v := pt.v - 171;}

				SetMouse(Point($00AB0100));{256,171}

				posh := posh + me^.speed.h;
				posv := posv + me^.speed.v;

				me^.position.h := bsr(posh, shift);
				me^.position.v := bsr(posv, shift);

				if me^.position.h > gSAT.offSizeH - xsize then
					begin
						me^.position.h := gSAT.offSizeH - xsize; { ev. xsize - 10? }
						posh := bsl(gSAT.offSizeH - xsize, shift);
					end;
				if me^.position.h < 0 then
					begin
						me^.position.h := 0;
						posh := 0;
					end;
				if me^.position.v > gSAT.offSizeV - 32 then
					begin
						me^.position.v := gSAT.offSizeV - 32; { ev. xsize - 10? }
						posv := bsl((gSAT.offSizeV - 32), shift);
					end;
				if me^.position.v < 0 then
					begin
						me^.position.v := 0;
						posv := 0;
					end;
			end; { if slowcount... }

		me^.mode := me^.mode + 1;
		if slowcount = 0 then
			begin
				if me^.mode > 6 then
					begin
						me^.mode := 0;
					end;
				me^.face := playerFace[abs(me^.mode - 3)];
			end
		else
			begin
				me^.face := playerFace[band(slowcount, 1) * 3];
				if slowcount = 1 then
					SATSoundPlay(Splatt3SndH, 1, true);
			end;
		if bonus > 0 then
			begin
				bonus := bonus - 1;
				if bonus mod 10 = 0 then
					begin
						addscore(0);
						if features^^.macho then
							if bonus < 110 then
								SATSoundPlay(TickSndH, 0, false);
					end
				else if features^^.macho then
					if bonus < 26 then
						if bonus mod 5 = 0 then
							SATSoundPlay(TickSndH, 0, false);
			end
		else if features^^.macho then
			stillrunning := false;

		playerPos := me^.position;
	end;

	procedure HitPlayer (me, him: SpritePtr);
	begin
{This would be an alternative place to handle collisions. Not used in this program.}
	end;

end.