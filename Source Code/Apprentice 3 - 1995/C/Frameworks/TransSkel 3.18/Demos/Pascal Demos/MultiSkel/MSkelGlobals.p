unit MultiSkelGlobals;

interface

	uses
		TransSkel;

	const

		normalHilite = 0;
		dimHilite = 255;

		aboutAlrtRes = 1000;
		getInfoDlog = 1001;

		fileMenuRes = 1001;
		editMenuRes = 1002;

		helpWindRes = 1000;
		editWindRes = 1001;
		zoomWindRes = 1002;
		rgnWindRes = 1003;

		helpTextRes = 1000;

	var

		helpWind: WindowPtr;
		editWind: WindowPtr;
		zoomWind: WindowPtr;
		rgnWind: WindowPtr;

		editMenu: MenuHandle;

	procedure DrawGrowBox (wind: WindowPtr);
	procedure SetWindClip (wind: WindowPtr);
	procedure ResetWindClip;

implementation


	var

		oldClip: RgnHandle;


{ Draw grow box of window in lower right hand corner }

	procedure DrawGrowBox (wind: WindowPtr);
		var
			oldClip: RgnHandle;
			r: Rect;
	begin
		r := wind^.portRect;
		r.left := r.right - 15;			{ draw only in corner }
		r.top := r.bottom - 15;
		oldClip := NewRgn;
		GetClip(oldClip);
		ClipRect(r);
		DrawGrowIcon(wind);
		SetClip(oldClip);
		DisposeRgn(oldClip);
	end;


	procedure SetWindClip (wind: WindowPtr);
		var
			r: Rect;
	begin
		r := wind^.portRect;
		r.right := r.right - 15;
		oldClip := NewRgn;
		GetClip(oldClip);
		ClipRect(r);
	end;


	procedure ResetWindClip;
	begin
		SetClip(oldClip);
		DisposeRgn(oldClip);
	end;

end.