{ Platform sprite, experimental faceless sprite }

unit sPlatform;

interface

	uses
		SAT;

	procedure InitPlatform;
	procedure SetupPlatform (me: SpritePtr);
	procedure HandlePlatform (me: SpritePtr);
	procedure HitPlatform (me, him: SpritePtr);

implementation

	procedure InitPlatform;
		var
			i: integer;
	begin
{The platform has no faces!}
	end;

	procedure SetupPlatform (me: SpritePtr);
		var
			r: Rect;
			pol: PolyHandle;
	begin
		{me^.kind := -2; {Enemy kind}
		me^.face := nil; {=faceless!}
		SetRect(me^.hotRect, 0, 0, 100, 16);
		r := me^.hotRect;
		offSetRect(r, me^.position.h, me^.position.v);
		SetPort(gSAT.backScreen);
{Perhaps we should SATSetPortBackScreen nowadays…}
{but it works anyway - we're in no hurry here, and we don't CopyBits.}
		FillRect(r, dkgray);

		pol := OpenPoly;
		MoveTo(r.left, r.top);
		LineTo(r.left + 5, r.top - 5);
		LineTo(r.right + 5, r.top - 5);
		LineTo(r.right, r.top);
		LineTo(r.left, r.top);
		LineTo(r.right, r.top);

		LineTo(r.right, r.bottom);
		LineTo(r.right + 5, r.bottom - 5);
		LineTo(r.right + 5, r.top - 5);
		LineTo(r.right, r.top);

		ClosePoly;
		ErasePoly(pol);
		FramePoly(pol);
		KillPoly(pol);

		r.top := r.top - 5;
		r.right := r.right + 5;
		SATBackChanged(r); {Tell SAT to draw it when appropriate}

		me^.layer := -me^.position.v;
	end;

	procedure HandlePlatform (me: SpritePtr);
	begin
		{me^.face := nil; {Really not needed}
	end;

	procedure HitPlatform;
		var
			mini, i, min: integer;
			diff: array[1..4] of integer;
	begin
		if him^.Task <> @HandlePlatform then
			begin
				diff[1] := -me^.hotRect2.top + (him^.hotRect2.bottom);	{TtoB}
				diff[2] := -him^.hotRect2.top + (me^.hotRect2.bottom);	{BtoT}
				diff[3] := -me^.hotRect2.left + (him^.hotRect2.right);	{LtoR}
				diff[4] := -him^.hotRect2.left + (me^.hotRect2.right);	{RtoL}
				mini := 0;
				min := 10000;
				for i := 1 to 4 do
					if min > diff[i] then
						begin
							min := diff[i];
							mini := i;
						end;
				case mini of
					1: {floor}
						begin
							him^.position.v := him^.position.v - diff[1] + 1;
							him^.kind := 10; {Signal to him, as if we used KindCollision}
							if him^.speed.v > 0 then
								him^.speed.v := 0;
						end;
					2: {ceiling}
						begin
							him^.position.v := him^.position.v + diff[2] + 1;{me^.position.v + 17}
{We don't signal here. A hit in the ceiling should just send him back down again.}
							if him^.speed.v < 0 then
								him^.speed.v := -him^.speed.v;
						end;
					3: {left}
						begin
							him^.position.h := him^.position.h - diff[3] - 1;{me^.position.h - 32}
							him^.kind := 10; {Signal to him, as if we used KindCollision}
							if him^.speed.h > 0 then
								him^.speed.h := -him^.speed.h;
						end;
					4: {right}
						begin
							him^.position.h := him^.position.h + diff[4] + 1;{me^.position.h + 100}
							him^.kind := 10; {Signal to him, as if we used KindCollision}
							if him^.speed.h < 0 then
								him^.speed.h := -him^.speed.h;
						end;
				end;{case}
			end; {if}
	end; {HitPlatform}
end.{of unit}