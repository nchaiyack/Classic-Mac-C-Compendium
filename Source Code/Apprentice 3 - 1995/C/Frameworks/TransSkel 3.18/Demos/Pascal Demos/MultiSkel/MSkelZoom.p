unit MultiSkelZoom;

interface

	uses
		TransSkel, MultiSkelGlobals;

	procedure ZoomWindInit;

implementation

	const

		zoomSteps = 15;
		maxZoomStep = zoomSteps - 1;

	var

		zRect: array[0..maxZoomStep] of Rect;
		zSrcRect: Rect;
		sizeX: Integer;
		sizeY: Integer;

	procedure ZDrawGrowBox (wind: WindowPtr);
	forward;


	procedure SetZoomSize;
		var
			r: Rect;
	begin
		r := zoomWind^.portRect;
		r.right := r.right - 15;
		sizeX := r.right;
		sizeY := r.bottom;
	end;


	function Rand (max: Integer): Integer;
		var
			t: Integer;
	begin
		t := Random;
		if (t < 0) then
			t := -t;
		Rand := t mod (max + 1);
	end;


	procedure ZoomRect (r1: Rect;
									r2: Rect);
		var
			r1left: Integer;
			r1top: Integer;
			l: Integer;
			t: Integer;
			j: Integer;
			hDiff: Integer;
			vDiff: Integer;
			widDiff: Integer;
			htDiff: Integer;
			r: Integer;
			b: Integer;
			rWid: Integer;
			rHt: Integer;
	begin
		r1left := r1.left;
		r1top := r1.top;
		hDiff := r2.left - r1left;		{ positive if moving to right }
		vDiff := r2.top - r1top;		{ positive if moving down }
		rWid := r1.right - r1left;
		rHt := r1.bottom - r1top;
		widDiff := (r2.right - r2.left) - rWid;
		htDiff := (r2.bottom - r2.top) - rHt;
		for j := 1 to zoomSteps do
			begin
				FrameRect(zRect[j - 1]);	{ erase a rectangle }
				l := r1left + (hDiff * j) div zoomSteps;
				t := r1top + (vDiff * j) div zoomSteps;
				r := l + rWid + (widDiff * j) div zoomSteps;
				b := t + rHt + (htDiff * j) div zoomSteps;
				SetRect(zRect[j - 1], l, t, r, b);
				FrameRect(zRect[j - 1]);
			end;
	end;


	procedure ZDrawGrowBox (wind: WindowPtr);
		var
			r: Rect;
	begin
		r := wind^.portRect;
		r.left := r.right - 15;
		r.top := r.bottom - 15;
		r.right := r.right + 1;
		r.bottom := r.bottom + 1;
		Eraserect(r);
		FrameRect(r);
		if (WindowPeek(wind)^.hilited) then
			begin
				r.right := r.right - 6;
				r.bottom := r.bottom - 6;
				OffsetRect(r, 4, 4);
				FrameRect(r);
				r.right := r.right - 3;
				r.bottom := r.bottom - 3;
				OffsetRect(r, -1, -1);
				EraseRect(r);
				FrameRect(r);
			end;
	end;


	procedure Mouse (pt: Point;
									t: LongInt;
									mods: Integer);
	begin
		while (StillDown) do
			begin
			{ wait until mouse button is released }
			end;
	end;


	procedure Update (resized: Boolean);
		var
			i: Integer;
	begin
		EraseRect(zoomWind^.portRect);
		ZDrawGrowBox(zoomWind);
		SetWindClip(zoomWind);
		for i := 0 to maxZoomStep do
			FrameRect(zRect[i]);
		ResetWindClip;
		if (resized) then
			SetZoomSize;
	end;


	procedure Activate (active: Boolean);
	begin
		ZDrawGrowBox(zoomWind);
		if (active) then
			DisableItem(editMenu, 0)
		else
			EnableItem(editMenu, 0);
		DrawMenuBar;
	end;


	procedure Clobber;
	begin
		DisposeWindow(zoomWind);
	end;


	procedure Idle;
		var
			i: Integer;
			pt1: Point;
			pt2: Point;
			dstRect: Rect;
	begin
		SetPt(pt1, Rand(sizeX), Rand(sizeY));
		SetPt(pt2, Rand(sizeX), Rand(sizeY));
		Pt2Rect(pt1, pt2, dstRect);
		SetWindClip(zoomWind);
		ZoomRect(zSrcRect, dstRect);
		ResetWindClip;
		zSrcRect := dstRect;
	end;


	procedure ZoomWindInit;
		var
			i: Integer;
			ignore: Boolean;
	begin
		if (SkelQuery(skelQHasColorQD) <> 0) then
			zoomWind := GetNewCWindow(zoomWindRes, nil, WindowPtr(-1))
		else
			zoomWind := GetNewWindow(zoomWindRes, nil, WindowPtr(-1));
		if (zoomWind = nil) then
			exit(ZoomWindInit);
		ignore := SkelWindow(zoomWind, @Mouse, nil, @Update, @Activate, nil, @Clobber, @Idle, false);
		SetZoomSize;
		BackPat(black);
		PenMode(patXor);
		SetRect(zSrcRect, 0, 0, 0, 0);
		for i := 0 to maxZoomStep do
			zRect[i] := zSrcRect;
	end;

end.