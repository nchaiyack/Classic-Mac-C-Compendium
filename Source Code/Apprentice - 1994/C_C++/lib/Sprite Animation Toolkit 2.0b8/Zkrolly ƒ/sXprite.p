unit sXprite;

interface

	uses
		SAT;

	var
		Xface: FacePtr;

	procedure initXprite;
	procedure SetupXprite (me: SpritePtr);
	procedure HandleXprite (me: SpritePtr);

implementation

	const
		xpeed = 2;

	procedure initXprite;
	begin
		Xface := GetFace(130);
	end;

	procedure SetupXprite (me: SpritePtr);
	begin
		me^.mode := 0;
		me^.speed.h := xpeed;
		me^.face := Xface;
	end;

	procedure HandleXprite (me: SpritePtr);
	begin
		with me^ do
			begin
{if mode < 1 then Face := nil;{test av faceless}

				position.h := position.h + speed.h;
				if position.h > gSAT.offSizeH then {400}
					begin
						speed.h := -xpeed;
					end;
				if position.h < -20 then {50}
					begin
						speed.h := xpeed;
					end;
			end;
	end;

end.