unit sZprite;

interface

	uses
		SAT;

	procedure initZprite;
	procedure SetupZprite (me: SpritePtr);
	procedure HandleZprite (me: SpritePtr);

implementation

	const
		zpeedH = 1;
		zpeedV = 1;
	var
		theSound: Handle;
		Zfaces: array[0..1] of FacePtr;

	procedure initZprite;
		var
			i: integer;
	begin
		theSound := SATGetSound(128);
		for i := 0 to 1 do
			Zfaces[i] := GetFace(128 + i);
	end;

	procedure SetupZprite (me: SpritePtr);
	begin
		me^.speed.h := zpeedH;
		me^.speed.v := -zpeedV;
		me^.face := Zfaces[1];
	end;

	procedure HandleZprite (me: SpritePtr);
	begin
		with me^ do
			begin
{if mode < 1 then Face := nil;{test av faceless}

				position.h := position.h + speed.h;
				position.v := position.v + speed.v;
				if position.h > gSAT.offSizeH then {400}
					begin
						speed.h := -zpeedH;
						SATSoundPlay(theSound, 1, false);
						face := Zfaces[0];
					end;
				if position.h < -20 then {50}
					begin
						speed.h := zpeedH;
						SATSoundPlay(theSound, 1, true);
						face := Zfaces[1];
					end;
				if position.v > gSAT.offSizeV then {400}
					begin
						speed.v := -zpeedV;
						SATSoundPlay(theSound, 1, false);
					end;
				if position.v < -20 then {50}
					begin
						speed.v := zpeedV;
						SATSoundPlay(theSound, 1, true);
					end;
			end;
	end;

end.