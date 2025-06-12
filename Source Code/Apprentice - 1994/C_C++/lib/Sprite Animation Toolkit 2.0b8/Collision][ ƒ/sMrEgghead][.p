{ Player sprite for SATcollision][ }

unit sMrEgghead;

interface

	uses
		SAT;

	var
		mrEggheadFaces: array[0..3] of FacePtr;
		yuckFace: FacePtr;

	procedure InitMrEgghead;
	procedure SetupMrEgghead (me: SpritePtr);
	procedure HandleMrEgghead (me: SpritePtr);
	procedure HitMrEgghead (me, him: SpritePtr);

implementation

	procedure InitMrEgghead;
		var
			i: integer;
	begin
		for i := 0 to 3 do
			mrEggheadFaces[i] := GetFace(128 + i);
		yuckFace := GetFace(134);
	end;

	procedure SetupMrEgghead (me: SpritePtr);
	begin
		me^.mode := 0;
		me^.speed.h := 1;
		me^.kind := 1; {Friend kind}
		SetRect(me^.hotRect, 0, 0, 32, 32);
	end;

	procedure HandleMrEgghead (me: SpritePtr);
	begin
		GetMouse(me^.position);

{This time, let's make sure Mr Egghead is always visible!}
		if me^.position.v < 0 then
			me^.position.v := 0;
		if me^.position.h < 0 then
			me^.position.h := 0;
		if me^.position.v > offSizeV - 32 then
			me^.position.v := offSizeV - 32;
		if me^.position.h > offSizeH - 32 then
			me^.position.h := offSizeH - 32;

		if me^.mode < 0 then {we have taken a bite in a bad apple recently}
			begin
				me^.face := yuckFace;
				me^.mode := me^.mode + 1;
			end
		else if me^.mode = 0 then {nothing special recently}
			me^.face := mrEggheadFaces[2]
		else
			begin {ate an apple recently - chew!}
				me^.mode := me^.mode - 1;
				me^.face := mrEggheadFaces[me^.mode mod 4];
			end;
	end;

	procedure HitMrEgghead;
	begin
{Hit something! We can take whatever action we need here, but in this case, we let sApple decide.}
{(We could have omitted this function altogether and passed nil.)}
	end;

end.