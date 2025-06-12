{ Apple sprite for SATcollision }

unit sApple;

interface

	uses
		SAT;

	var
		TheSound: Handle;
		AppleFace: FacePtr;

	procedure InitApple;
	procedure SetupApple (me: SpritePtr);
	procedure HandleApple (me: SpritePtr);

implementation

	procedure InitApple;
	begin
		TheSound := SATGetSound(128);
		AppleFace := GetFace(132);
	end;

	procedure SetupApple (me: SpritePtr);
	begin
		me^.speed.h := 1 + Rand(3);
		me^.kind := -1; {Enemy kind}
		me^.face := AppleFace;
		SetRect(me^.hotRect, 0, 0, 32, 32);
	end;

	procedure HandleApple (me: SpritePtr);
	begin
		if me^.kind <> -1 then {Something hit us!}
			begin
				SATSoundPlay(TheSound, 1, false);
				me^.task := nil; {Go away}
			end;
{Move}
		me^.position.h := me^.position.h + me^.speed.h;
		if me^.position.h > gSAT.offSizeH - 16 then
			me^.speed.h := -1 - Rand(3);
		if me^.position.h < -16 then
			me^.speed.h := 1 + Rand(3);
	end;

end.