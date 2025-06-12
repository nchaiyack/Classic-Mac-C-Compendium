unit triangles;

interface

	uses
		Matrix, Transformations, OffscreenCore, GrafSysCore, GrafSysScreen, GrafSysObject, Resources, OffScreenGraphics, GrafSysC;

	function Triangle (p1, p2, p3: point; theColor: integer): integer;


implementation

(* Top-Down :	Pascal version of the triangle top-down fill algorithm *)
(*					just provided to test and demonstrate the algorithm *)
(*					calling convention : 	b is highest point, a and d have same 	*)
(*											vertical component 						*)

	procedure TPTopDown (a, b, d: point);
		var
			deltaxleft, deltaxright: fixed;
			dx, dy: integer;
			ys, ye: integer;
			xs, xe: fixed;

	begin
		dy := a.v - b.v;
		dx := a.h - b.h;
		deltaXleft := FixRatio(dx, dy); (* ATTN: dy may be zero ! *)
		dx := d.h - b.h;
		deltaXright := FixRatio(dx, dy);
(* Not implemented : plot b itself *)
		xs := FixRatio(b.h, 1);
		ys := b.v;
		xe := FixRatio(b.h, 1);
		ye := b.v; (* now both point to the start point *)
		while ye < a.v do
			begin (* go down one line and plot from s to e horizontal line *)
				ys := ys + 1;
				ye := ye + 1;
				xs := xs + deltaXleft;
				xe := xe + deltaXright;
				MoveTo(HiWord(xs), ys);
				LineTo(HiWord(xe), ye);
			end;
	end;


	procedure TPBottomUp (a, d, c: point);

		var
			deltaxleft, deltaxright: fixed;
			dx, dy: integer;
			ys, ye: integer;
			xs, xe: fixed;


	begin
		dy := a.v - c.v;
		dx := a.h - c.h;
		deltaXleft := FixRatio(dx, dy); (* ATTN: dy may be zero ! *)
		dx := d.h - c.h;
		deltaXright := FixRatio(dx, dy);
(* Not implemented : plot b itself *)
		xs := FixRatio(c.h, 1);
		ys := c.v;
		xe := FixRatio(c.h, 1);
		ye := c.v; (* now both point to the start point *)
		while ye > a.v do
			begin (* go down one line and plot from s to e horizontal line *)
				ys := ys - 1;
				ye := ye - 1;
				xs := xs - deltaXleft;
				xe := xe - deltaXright;
				MoveTo(HiWord(xs), ys);
				LineTo(HiWord(xe), ye);
			end;
	end;

	procedure ButtonWait;
	begin
		repeat
		until button;
		repeat
		until not button;
	end;

	function Triangle (p1, p2, p3: point; theColor: integer): integer;

		var
			a, b, c, d: point; (* the four points that define the two scan-parallel triangles *)
			temp: point;

			dx, dy: integer;
			deltax: fixed;
			deltaY: fixed;
			thePort: GrafPtr;


	begin
(* sanity check : do we have a pixmap allocated ? *)
(* if currOSPixMap = nil then *)
		if false then
			begin
				Triangle := cNoActiveOSPixMap;
				Exit(Triangle);
			end;

		GetPort(thePort);

(* Step 1 : sort p1, p2, p3 so that they conform with the scan-parallel definition *)
	(* Step 1.1 : look for b (highest) and place a and c accordingly *)
		if (p1.v <= p2.v) and (p1.v <= p3.v) then
			begin
				b := p1; (* p1 is highest *)
				if p2.v >= p3.v then (* p2 is lowest, p3 is middle *)
					begin
						c := p2;
						a := p3;
					end
				else
					begin (* p3 is lowest, p2 is middle *)
						c := p3;
						a := p2;
					end;
			end
		else if (p2.v <= p1.v) and (p2.v <= p3.v) then
			begin
				b := p2; (* p2 is highest *)
				if p1.v >= p3.v then
					begin
						c := p1;
						a := p3;
					end
				else
					begin
						c := p3;
						a := p1;
					end;
			end
		else (* p3.v is highest *)
			begin
				b := p3;
				if p1.v >= p2.v then
					begin
						c := p1;
						a := p2;
					end
				else
					begin
						c := p2;
						a := p1;
					end;
			end;

(* step 2 : calculate point d : it's v is the same as a's but it resides on bc *)
		dx := c.h - b.h;
		dy := c.v - b.v;
		deltaX := FixRatio(dx, dy);
		deltaY := FixRatio(a.v - b.v, 1);
		d.v := a.v;
		d.h := b.h + HiWord(FixMul(deltaY, deltax));

(* step 4 : sanity check --> exchange a and d if d.h < a.h *)
		if d.h < a.h then
			begin
				temp := a;
				a := d;
				d := temp;
			end;

(* step 3 : draw it. this takes two steps*)
(*				step 3.1 draw upper triangle top-down *)
(*				step 3.2 draw lower triangle bottom-up *)
		ButtonWait;
		MoveTo(a.h, a.v); (* jst draw outline. Draw upper scan-parallel *)
		LineTo(b.h, b.v);
		LineTo(d.h, d.v);
		MoveTo(a.h, a.v); (* draw lower scan-parallel *)
		LineTo(c.h, c.v);
		LineTo(d.h, d.v);
		ButtonWait;
		FillTri(a, b, d, c, CGrafPtr(thePort)^.portPixMap, theColor);
(* Finish up *)
		Triangle := noErr;
	end; (* Triangle *)

end.