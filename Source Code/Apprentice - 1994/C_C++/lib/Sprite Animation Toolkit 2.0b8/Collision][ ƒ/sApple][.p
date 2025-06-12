{ Apple sprite for SATcollision][ }
{ No, it has nothing to do with old Apple computers :-) }

unit sApple;

interface

	uses
		SAT;

	var
		nammSound, bliaehSound: Handle;
		goodFace, badFace: FacePtr;
		coreDump: FacePtr;

	procedure InitApple;
	procedure SetupApple (me: SpritePtr);
	procedure HandleApple (me: SpritePtr);
	procedure HitApple (me, him: SpritePtr);

implementation

	procedure InitApple;
		var
			i: integer;
	begin
		nammSound := SATGetNamedSound('namm');
		bliaehSound := SATGetNamedSound('bliaeh');
		goodFace := GetFace(132);
		badFace := GetFace(133);
		coreDump := GetFace(135);
	end;

	procedure SetupApple (me: SpritePtr);
	begin
		me^.speed.h := 1 + Rand(3);
		me^.kind := -2; {Enemy kind}
		me^.face := GoodFace;
		SetRect(me^.hotRect, 0, 0, 32, 32);
	end;

{We use kind -2 for fresh apples and kind -3 for bad apples. We avoid -1, since it doesn't count for the "anymonsters" flag.}
{Note that the "kind" field is not modified my SAT since we are not using KindCollision!}

	procedure HandleApple (me: SpritePtr);
	begin

		case me^.kind of
			-2: 
				me^.face := goodFace;
			-3: 
				me^.face := badFace;
		end;

		me^.position.h := me^.position.h + me^.speed.h;
		if me^.position.h > offSizeH - 16 then
			begin
				me^.speed.h := -1 - Rand(3);
				if Rand(2) = 0 then
					me^.kind := -3
				else
					me^.kind := -2;
			end;
		if me^.position.h < -16 then
			begin
				me^.speed.h := 1 + Rand(3);
				if Rand(2) = 0 then
					me^.kind := -3
				else
					me^.kind := -2;
			end;
	end;

	procedure HitApple;
	begin
		if him^.task = @HandleApple then
			me^.kind := -3 {Colliding apples corrupt each other!}
		else
{If "him" is not an apple, then it must be the player!}
			if him^.mode >= 0 then {if the player feels bad, don't eat}
				case me^.kind of
					-2: 
						begin
							SATSoundPlay(nammSound, 1, false);
							me^.task := nil;
							him^.mode := 25;
							SATPlotFace(coreDump, nil, nil, me^.position, true);
						end;
					-3: {Bad apple, make player feel bad.}
						begin
							him^.mode := -20;
							SATSoundPlay(bliaehSound, 2, false);
						end;
					otherwise {This shouldn't happend!}
				end;
	end;

end.