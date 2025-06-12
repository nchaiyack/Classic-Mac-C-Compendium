unit ResourceAccess;

(* this unit handles loading the 3D resources for objects  *)
(* and converting them to the format used by the grafsys *)
interface
	uses
		Matrix, Transformations, OffscreenCore, GrafSysCore, GrafSysScreen, GrafSysObject;

(* LoadObjRes loads the '3Dob' Resource with the ID given and converts the data found there to *)
(* the data structure the Grafsys requires                                                                                *)

	procedure LoadObjRes (ID: integer; var theObject: TSObject3D);
	procedure LoadNamedObjRes (name: str255; var theObject: TSObject3D);
	procedure SaveObjRes (ID: integer; name: Str255; theObject: TSObject3D);
	procedure SaveNamedObjRes (var ID: integer; name: Str255; theObject: TSObject3D);

implementation

	const
(* formula for maxpoly is not correct *)
{MaxByte = 2 + MXP * 4 * 3 + 2 + MXL * 2 * 2 + 2 + MXPoly * MaxPolyLine * 2;}
(* how did we get MaxByte ? *)
(*    2 Byte # of Points  *)
(*    3 Coords * 4 Byte * MaxPoints for point data *)
(*    2 Byte # of lines *)
(*    2 Pointindex * 2 Bytes * MaxLines *)
(*    2 Byte # of Polygons *)
(*    MaxPolyLine * 2 * MaxPolygons *)

		dummyConst = 0;

	type
		TypeMagic = record (* used for type casting... very dirty *)
				case integer of
					1: (
							leftWord: integer;
							rightWord: integer;
					);

					2: (
							theReal: real
					);

			end;

		Point3D = array[1..4] of real;

		ResBufH = ^ResBufPtr;
		ResBufPtr = ^ResBuf;
		ResBuf = array[1..1] of integer; (* size is irrelevant *)

	var
		theRes: ResBufH;

	function GetInt (theBuf: ResBufH; var index: integer): integer;

	begin
		GetInt := theBuf^^[index];
		index := index + 1;
	end;

	procedure PutInt (theBuf: ResBufH; var index: integer; theValue: integer);

	begin
		theBuf^^[index] := theValue;
		index := index + 1;
	end;

	procedure DoResLoad (var theObject: TSObject3D);

		var
			PointCount: integer;
			BigHack: TypeMagic;
			thePoint: Point3D;
			fromIndex, toIndex: integer;
			LineCount: integer;
			index: integer;
			PolyCount: integer;
			Poly: Polygon;
			dummyLong: Longint;
			pt, ln, py: integer;

	begin

		index := 1; (* index is our byte counter. now points to first byte (# of points ) *)
		Pt := GetInt(theRes, index);
		if Pt > 0 then
			for PointCount := 1 to pt do (* read in Pt Points *)
				begin
					BigHack.leftWord := GetInt(theRes, index);
					BigHack.rightWord := GetInt(theRes, index);
					thePoint[1] := (BigHack.theReal);
					BigHack.leftWord := GetInt(theRes, index);
					BigHack.rightWord := GetInt(theRes, index);
					thePoint[2] := (BigHack.theReal);
					BigHack.leftWord := GetInt(theRes, index);
					BigHack.rightWord := GetInt(theRes, index);
					thePoint[3] := (BigHack.theReal);
					thePoint[4] := 1;
					dummyLong := theObject.AddPoint(thePoint[1], thePoint[2], thePoint[3]);
				end;

		Ln := GetInt(theRes, index);
		if Ln > 0 then
			for LineCount := 1 to Ln do
				begin
					fromIndex := GetInt(theRes, index);
					toIndex := GetInt(theRes, index);
					dummyLong := theObject.AddLine(fromIndex, toIndex);
				end;

		py := GetInt(theRes, index);
		if py > 0 then
			for PolyCount := 1 to py do
				begin
(* do nothing *)
				end;

(* now that resource is loaded, we must deallocate it, since it is only a template *)
		ReleaseResource(Handle(theRes));
	end;


(* GetResColor : Try to load a 'lClr' resource with given ID into object *)
(* 					lClr is zero-delimited list. *)
(*						first word	 	:	misc info				*)
(*						second word	: 	line number or zero	*)
(*                       	third word	:	Red value				*)
(*						fourth word	:	Green value			*)
(*						fifth word	:	Blue value				*)


	procedure GetResColor (var theObject: TSObject3D; theID: integer);

		var
			index, i: integer;
			theColor: RGBColor;
			lineIndex: integer;
			dummyBool: Boolean;
			ChangeCount: Integer;

	begin
		theRes := ResBufH(GetResource(Res3DColor, theID));
		if theRes = nil then
			Exit(GetResColor);

		index := 1; (* index is counter for resource access *)
		ChangeCount := GetInt(theRes, index); (* ignored *)
		i := 1;

		while i <= ChangeCount do
			begin
				lineIndex := GetInt(theRes, index);
				theColor.Red := GetInt(theRes, index);
				theColor.Green := GetInt(theRes, index);
				theColor.Blue := GetInt(theRes, index);
				dummyBool := theObject.ChangeLineColor(lineIndex, theColor);
				i := 1 + 1;
			end;
		ReleaseResource(Handle(theRes));
	end;

	procedure LoadObjRes (ID: integer; var theObject: TSObject3D);

	begin
		theRes := ResBufH(GetResource(Res3D, ID));
		if theRes <> nil then
			begin
				DoResLoad(theObject); (* releases resource, too *)
				GetResColor(theObject, ID);
			end
		else
			begin
				theObject.ErrorCode := cCantLoadRes;
			end;
	end;

	procedure LoadNamedObjRes (name: str255; var theObject: TSObject3D);
		var
			aName: Str255;
			theID: Integer;
			aType: ResType;

	begin
		theRes := ResBufH(GetNamedResource(Res3D, name));
		if theRes <> nil then
			begin
				GetResInfo(Handle(theRes), theID, aType, aName);
				DoResLoad(theObject);
				GetResColor(theObject, theID);
			end
		else
			begin
				theObject.ErrorCode := cCantLoadRes;
			end;
	end;

(* Save in current open resourcefile the object's definition as a resource *)

	procedure DoSaveRes (theObject: TSObject3D);

		var
			theSize: longInt;
			LineCount, PointCount: integer;
			fromIndex, toIndex: longint;
			thePoint: Point3D;
			index: integer;
			BigHack: TypeMagic;
			dummyBool: Boolean;
			pt, ln, py: integer;

	begin

(* begin with calculating the required size *)
		Pt := theObject.numPoints;
		Ln := theObject.numLines;
		py := 0;

		theSize := 2 + Pt * 12 + 2 + Ln * 4 + 2 + py * 10 * 2;
		theRes := ResBufH(NewHandle(theSize)); (* NOTE: HACK HACK HACK! The size is not the size of *)
		index := 1; (*                                                            the original data structure!!!!                         *)
		if theRes = nil then
			begin
				theObject.ErrorCode := cOutOfMem;
				Exit(DoSaveRes);
			end;

		PutInt(theRes, index, theObject.numPoints); (* save # of points *)
		if Pt > 0 then
			for PointCount := 1 to pt do (* read in Pt Points *)
				begin
					with BigHack do
						begin
							theObject.GetPoint(PointCount, thePoint[1], thePoint[2], thePoint[3]);
							theReal := (thePoint[1]);
							PutInt(theRes, index, leftWord);
							PutInt(theRes, index, rightWord);
							theReal := (thePoint[2]);
							PutInt(theRes, index, leftWord);
							PutInt(theRes, index, rightWord);
							theReal := (thePoint[3]);
							PutInt(theRes, index, leftWord);
							PutInt(theRes, index, rightWord);
						end (* with bighack *)
				end;


		PutInt(theRes, index, theObject.numLines);
		if Ln > 0 then
			for LineCount := 1 to Ln do
				begin
					theObject.GetLine(LineCount, fromIndex, toIndex);
					PutInt(theRes, index, fromIndex);
					PutInt(theRes, index, toIndex);
				end;

		PutInt(theRes, index, 0); (* zero polygons until now *)

	end;

(* Save rescolor: build memory structure for lClr resource *)
	procedure SaveResColor (theObject: TSObject3D);

		var
			lineIndex: Integer;
			changeCount: integer;
			index: integer;
			theColor: RGBColor;
			isChange: Boolean;
			theSize: longint;
			dummyBool: Boolean;
			MiscInfo: integer;

	begin
(* first, count the number of changes to calculate the required memsize *)
		lineIndex := 1;
		changeCount := 0;
		while lineIndex <= theObject.numLines do
			begin
				dummyBool := theObject.GetLineColor(lineIndex, theColor, isChange);
				if isChange then
					changeCount := changeCount + 1;
				lineIndex := lineIndex + 1;
			end;

		theSize := changeCount * (sizeOf(lineIndex) + sizeOf(theColor)) + sizeOf(changeCount);
		theRes := ResBufH(NewHandle(theSize));
		lineIndex := 1;
		index := 1;
		PutInt(theRes, index, changeCount);
		while lineIndex <= theObject.numLines do
			begin
				dummyBool := theObject.GetLineColor(lineIndex, theColor, isChange);
				if isChange then
					begin
						PutInt(theRes, index, lineIndex);
						PutInt(theRes, index, theColor.red);
						PutInt(theRes, index, theColor.green);
						PutInt(theRes, index, theColor.blue);
					end;
				lineIndex := lineIndex + 1;
			end;
	end;

(* replaces resource with same id *)
	procedure SaveObjRes (ID: integer; name: Str255; theObject: TSObject3D);

		var
			aRes: Handle;
			theErr: integer;

	begin
		DoSaveRes(theObject);
		if theRes = nil then
			Exit(SaveObjRes);

	(* check if we have to delete a resource of the same id before saving *)
		aRes := GetResource(Res3D, ID);
		if aRes <> nil then
			begin
				RmveResource(aRes);
				DisposHandle(aRes);
			end;
		AddResource(Handle(theRes), Res3D, ID, name);
		UpdateResFile(CurResFile);
		ReleaseResource(Handle(theRes)); (* Deallocate Mem *)
		theRes := nil;

(* handle color information *)
		SaveResColor(theObject); (* build color information resource in global theRes *)
		if theRes = nil then
			Exit(SaveObjRes);
	(* check if we have to delete a resource of the same id before saving *)
		aRes := GetResource(Res3DColor, ID);
		if aRes <> nil then
			begin
				RmveResource(aRes);
				DisposHandle(aRes);
			end;

		AddResource(Handle(theRes), Res3DColor, ID, name);
		theErr := ResError;
		UpdateResFile(CurResFile);
		theErr := ResError;
		ReleaseResource(Handle(theRes)); (* Deallocate Mem *)
	end;

(* replace res with same name *)
	procedure SaveNamedObjRes (var ID: integer; name: Str255; theObject: TSObject3D);
		var
			aRes: Handle;
			aType: ResType;
			aName: Str255;
			theErr: Integer;

	begin
		DoSaveRes(theObject);
		if theRes = nil then
			Exit(SaveNamedObjRes);

(* check if we have to delete a resource of the same id before saving *)
		aRes := GetNamedResource(Res3D, name);
		if aRes <> nil then
			begin
				GetResInfo(aRes, ID, aType, aName); (* get ID for later save of replacement *)
				RmveResource(aRes);
				DisposHandle(aRes);
			end
		else
			ID := UniqueID(Res3D);

		AddResource(Handle(theRes), Res3D, ID, name);
		UpdateResFile(CurResFile);
		ReleaseResource(Handle(theRes)); (* Deallocate Mem *)

(* handle color information *)
		SaveResColor(theObject); (* build color information resource in global theRes *)
		if theRes = nil then
			Exit(SaveNamedObjRes);
	(* check if we have to delete a resource of the same id before saving *)
		aRes := GetResource(Res3DColor, ID);
		if aRes <> nil then
			begin
				RmveResource(aRes);
				DisposHandle(aRes);
			end;
		AddResource(Handle(theRes), Res3DColor, ID, name);
		theErr := ResError;
		UpdateResFile(CurResFile);
		theErr := ResError;
		ReleaseResource(Handle(theRes)); (* Deallocate Mem *)
	end;

end.