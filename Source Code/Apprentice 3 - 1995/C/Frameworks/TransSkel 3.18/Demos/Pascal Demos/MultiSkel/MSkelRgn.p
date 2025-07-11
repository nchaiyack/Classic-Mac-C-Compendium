unit MultiSkelRgn;

interface

	uses
		TransSkel, MultiSkelGlobals;

	procedure RgnWindInit;

implementation

	var

		rgnPortRect: Rect;
		selectRgn: RgnHandle;
		selectWhen: LongInt;
		selectWhere: Point;

		marqueePat: Pattern;


	procedure MarqueeRgn (r: RgnHandle);
		var
			p: PenState;
			b: Byte;
			i: Integer;
	begin
		GetPenState(p);
		PenPat(marqueePat);
		PenMode(patCopy);
		FrameRgn(r);
		SetPenState(p);
		b := marqueePat[0];			{ shift pattern for next call }
		for i := 0 to 7 do
			marqueePat[i] := marqueePat[i + 1];
		marqueePat[7] := b;
	end;


	procedure DoSelectRect (startPoint: Point;
									var dstRect: Rect);
		var
			pt: Point;
			dragPt: Point;
			rClip: Rect;
			port: GrafPtr;
			result: Boolean;
			ps: PenState;
			i: Integer;
			loop: Boolean;
	begin
		GetPort(port);
		rClip := port^.portRect;
		rClip.right := rClip.right - 15;
		GetPenState(ps);
		PenPat(gray);
		PenMode(patXor);
		dragPt := startPoint;
		Pt2Rect(dragPt, dragPt, dstRect);
		FrameRect(dstRect);
		loop := true;
		while (loop) do
			begin
				GetMouse(pt);
				if (not EqualPt(pt, dragPt)) then	{ mouse has moved, change region }
					begin
						FrameRect(dstRect);
						dragPt := pt;
						Pt2Rect(dragPt, startPoint, dstRect);
						result := SectRect(dstRect, rClip, dstRect);
						FrameRect(dstRect);
						for i := 0 to 999 do
							begin
								{ empty }
							end;
					end;
				if (not StillDown) then
					loop := false;
			end;
		FrameRect(dstRect);			{ erase last rect }
		SetPenState(ps);
	end;


	procedure Mouse (pt: Point;
									t: LongInt;
									mods: Integer);
		var
			r: Rect;
			rgn: RgnHandle;
	begin
		r := rgnWind^.portRect;
		if (pt.h >= r.right - 15) then
			exit(Mouse);
		if ((t - selectWhen) <= GetDblTime) then	{ it's a double-click }
			begin
				selectWhen := 0;				{ don't take next click as double-click }
				SetWindClip(rgnWind);
				EraseRgn(selectRgn);
				ResetWindClip;
				SetEmptyRgn(selectRgn);		{ clear region }
			end
		else
			begin
				selectWhen := t;			{ update click variables }
				selectWhere := pt;
				DoSelectRect(pt, r);		{ draw selection rectangle }
				if (not EmptyRect(r)) then
					begin
						EraseRgn(selectRgn);
						selectWhen := 0;
						rgn := NewRgn;
						RectRgn(rgn, r);
						if (BitAnd(mods, shiftKey) <> 0) then	{ test shift key }
							DiffRgn(selectRgn, rgn, selectRgn)
						else
							UnionRgn(selectRgn, rgn, selectRgn);
						DisposeRgn(rgn);
					end;
			end;

	end;


	procedure Idle;
		var
			i: Integer;
	begin
		SetWindClip(rgnWind);
		MarqueeRgn(selectRgn);
		ResetWindClip;
	end;


	procedure Update (resized: Boolean);
		var
			r: Rect;
	begin
		r := rgnWind^.portRect;
		EraseRect(r);
		if (resized) then
			begin
				rgnPortRect.right := rgnPortRect.right - 15;
				r.right := r.right - 15;
				MapRgn(selectRgn, rgnPortRect, r);
				rgnPortRect := rgnWind^.portRect;
			end;
		DrawGrowBox(rgnWind);
		Idle;
	end;


	procedure Activate (active: Boolean);
	begin
		DrawGrowBox(rgnWind);
		if (active) then
			DisableItem(editMenu, 0)
		else
			EnableItem(editMenu, 0);
		DrawMenuBar;
	end;


	procedure Clobber;
	begin
		DisposeRgn(selectRgn);
		DisposeWindow(rgnWind);
	end;


	procedure RgnWindInit;
		var
			ignore: Boolean;
	begin
		StuffHex(@marqueePat, '0f87c3e1f0783c1e');

		if (SkelQuery(skelQHasColorQD) <> 0) then
			rgnWind := GetNewCWindow(rgnWindRes, nil, WindowPtr(-1))
		else
			rgnWind := GetNewWindow(rgnWindRes, nil, WindowPtr(-1));
		if (rgnWind = nil) then
			exit(RgnWindInit);
		ignore := SkelWindow(rgnWind, @Mouse, nil, @Update, @Activate, nil, @Clobber, @Idle, false);

		rgnPortRect := rgnWind^.portRect;
		selectRgn := NewRgn;
		selectWhen := 0;

	end;

end.