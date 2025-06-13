unit Sicn;

interface

	uses
		TransSkel;

	const

		sitmSize = 16;

	type

		Sitm = array[1..sitmSize] of Integer;
		Sicn = array[0..0] of Sitm;			{ actually variable-length }
		SicnPtr = ^Sicn;
		SicnHandle = ^SicnPtr;
	procedure PlotSicn (r: Rect;
									s: SicnHandle;
									i: Integer);

	procedure PlotSitm (r: Rect;
									s: Sitm);

implementation

	procedure PlotSicn (r: Rect;
									s: SicnHandle;
									i: Integer);
		var
			state: SignedByte;
	begin

		state := HGetState(Handle(s));
		HLock(Handle(s));
		PlotSitm(r, s^^[i]);
		HSetState(Handle(s), state);
	end;


	procedure PlotSitm (r: Rect;
									s: Sitm);
		var
			port: GrafPtr;
			bm: BitMap;
	begin
		{ create a small bitmap }
		bm.baseAddr := Ptr(@s);
		bm.rowBytes := 2;
		SetRect(bm.bounds, 0, 0, sitmSize, sitmSize);
		GetPort(port);
		CopyBits(bm, port^.portBits, bm.bounds, r, srcCopy, nil);
	end;

end.