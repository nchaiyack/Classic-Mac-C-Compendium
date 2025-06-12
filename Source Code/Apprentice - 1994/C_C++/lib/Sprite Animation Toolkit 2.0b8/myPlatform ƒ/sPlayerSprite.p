{ Player sprite for the Platform test}

unit sPlayerSprite;

interface

	uses
		SAT;

	var
		rightFaces, leftFaces: array[0..3] of FacePtr;
		fallFace: array[0..1] of FacePtr;
		done: Boolean; {Boolean that tells if we should quit}

	procedure InitPlayerSprite;
	procedure SetupPlayerSprite (me: SpritePtr);
	procedure HandlePlayerSprite (me: SpritePtr);
	procedure HitPlayerSprite (me, him: SpritePtr);

implementation

	procedure InitPlayerSprite;
		var
			i: integer;
	begin
		for i := 0 to 3 do
			rightFaces[i] := GetFace(128 + i);
		for i := 0 to 3 do
			leftFaces[i] := GetFace(132 + i);
		for i := 0 to 1 do
			fallFace[i] := GetFace(136 + i);
	end;

	procedure SetupPlayerSprite (me: SpritePtr);
	begin
		me^.mode := 0;
		me^.speed.h := 1;
		me^.kind := 1; {Friend kind}
		SetRect(me^.hotRect, 8, 0, 24, 32);
		me^.face := fallFace[0];
	end;

	procedure HandlePlayerSprite (me: SpritePtr);
		var
			theEvent: EventRecord;
	begin
		if me^.kind = 10 then
			begin
				me^.speed.h := me^.speed.h div 2;
			end;

		if GetOSEvent(keyDownMask + autoKeyMask, theEvent) then
			begin
				if BitAnd(theEvent.modifiers, cmdKey) <> 0 then
					case char(BitAnd(theEvent.message, charCodeMask)) of
						'q', 'Q': 
							done := true; {Command-Q: quit!}
					end; {case}
				if me^.kind <> 1 then{Touching something - then we can control!}
					case char(bitand(theEvent.message, charCodeMask)) of
						',': 
							me^.speed.h := -5;
						'.': 
							me^.speed.h := 5;
						' ': 
							me^.speed.v := -5;
						otherwise
					end;
			end;

		me^.position.h := me^.position.h + me^.speed.h;
		me^.position.v := me^.position.v + me^.speed.v;
		if me^.kind < 3 then {acceleration when in midair!}
			me^.speed.v := me^.speed.v + 1;
		if me^.speed.v > 30 then
			me^.speed.v := 30;

		if me^.kind > 1 then
			me^.kind := me^.kind - 1;

{Make sure we are always visible!}
		if me^.position.v < 0 then
			begin
				me^.position.v := 0;
				me^.speed.v := 0;
			end;
		if me^.position.h < 0 then
			begin
				me^.position.h := 0;
				me^.speed.h := abs(me^.speed.h);
			end;
		if me^.position.v > gSAT.offSizeV - 32 then
			begin
				me^.position.v := gSAT.offSizeV - 32;
				me^.speed.v := -abs(me^.speed.v) * 2 div 3;
				me^.kind := 7; {Allow movement for a short while!}
			end;
		if me^.position.h > gSAT.offSizeH - 32 then
			begin
				me^.position.h := gSAT.offSizeH - 32;
				me^.speed.h := -abs(me^.speed.h);
			end;

		me^.mode := abs(me^.mode + 1);

		if me^.kind > 1 then
			begin
				if me^.speed.h > 0 then
					me^.face := RightFaces[band(me^.mode div 3, 3)];
				if me^.speed.h < 0 then
					me^.face := LeftFaces[band(me^.mode div 3, 3)];
			end
		else
			me^.face := FallFace[band(me^.mode, 1)];

		me^.layer := -me^.position.v;
	end;

	procedure HitPlayerSprite;
	begin
{Hit something! We can take whatever action we need here, but in this case, we let OtherSprite decide.}
{(We could have omitted this function altogether and passed nil.)}
		if him^.kind = 1 then
		else if him^.kind = 2 then
	end;

end.