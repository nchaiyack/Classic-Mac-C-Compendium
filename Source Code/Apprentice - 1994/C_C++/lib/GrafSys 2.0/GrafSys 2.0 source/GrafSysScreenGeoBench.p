unit GrafSysScreen;

interface
uses
	Matrix, Transformations, OffscreenCore, GrafSysCore;

const
{$IFC UseFixedMath = FALSE}
	MaxPointPerBuf = 1024 - 1; (* Maximum Points per Object : 256K *)
{$ELSEC}
{ code for fixed math routines }
	MaxPointPerBuf = 1024 - 1;
{$ENDC}
	MaxBuffers = 256 - 1;
(* GrafSys constants *)
	cPort3DType = '3Prt';
	cGrafSysVersion = $00000001; (* = Version 0.01 *)

type
	ProjectionTypes = (parallel, perspective);
	clippingType = (none, arithmetic, fast);

	TPort3DPtr = ^TPort3D;
	TPort3D = record
			theWindow: CWindowRecord; (* piggy-back riding on the window's data we'll put everything else *)
			mx, my, dx, dy: real; (* used for GrafSys adaption *)
			versionType: OSType; (* used to veryfy that this record is really a 3dPort *)
			theOffscreen: TOffscreenRec; (* for later use in off-screen drawing *)
			ProjectionPlane: rect;
			ViewPlane: rect;
			left, right, top, bottom: integer;
			center: point;
			useEye: Boolean;	{if false, no eye transform necessary }
			EyeKoord: Vector4;
			ViewPoint: Vector4;
			phi, theta, pitch: real;
			ViewAngle: real;
			d: real; {Perspective param set by viewangle }
			MasterTransform: Matrix4; {matrix to transform all objects according to eye settings }
			projection: ProjectionTypes;
			clipping: ClippingType;
			versionsID: longint; (* ID so objects can detect if the eye changed its specifications *)
		end;

	TSPoint3D = object(TPoint3D)
			size: integer; (* size of point *)
			procedure Init;
			override;
			procedure Reset;
			override;
			procedure Draw;	{draw point as seen from the eye. xForm (and Eye) are  applied. No Transform call required }
			override;				{xForm is automatically calculated if neccessary. A Graf3DPort must be active! }
		end;

	TSLine3D = object(TLine3D)
			procedure Draw; 	{draw line as seen from the eye. xForm (and Eye) are  applied. No Transform call required }
			override;				{xForm is automatically calculated if neccessary. A Graf3DPort must be active! }
		end;

	Point3DEntry = record
			koords: Vector4;
			transformed: vector4; (* transformed point, used with clipping *)
			Screenx: integer;
			Screeny: integer;
{transformedZ: real;}
		end;

	Point3DBufPtr = ^Point3DBufRec;
	Point3DBufRec = array[0..MaxPointperBuf] of Point3DEntry;

	TSGenericObject3D = object(Tabstract3DObject) 	{ this is the real 3D object that will be extended }
			theBufs: array[0..MaxBuffers] of Point3DBufPtr;		{ it contains only the points, nothing else }
			currentBuf: Point3DBufPtr;
			currentIndex: integer; (* index of current buffer *)
			numPoints: longint; (* number of points in object *)
			Bounds: rect; (* for auto-erase data gathering *)
			oldBounds: Rect;
			screenXform: Matrix4;								{final xformation matrix including eye if useseye else}
																{equal to xForm                                                         }
			procedure Init;
			override;
			function Clone: TGenericObject;					{duplicate point buffers as well }
			override;
			procedure Reset;									{reset all rots and attributes to default }
			override;
			procedure Kill; 										{deallocate mem. Will kill all sons that inherit}
			override;
			procedure GenIndex (pointIndex: longint; var BufIndex, bufOffset: integer); {pointindex -> (buffer, offset) conversion }
			function AddPoint (x, y, z: real): longint;		{add a point to the object's database. It returns the }
																{points reference number if successful or -1 otherwise}
																{point count is one-based, i.e. fist point is index 1. Note	 }
																{that this differs from the internal representation where }
																{point count is zero-based. COMMON ERROR SOURCE!		 }
			function DeletePoint (index: longint): boolean;	{delete point with passed index from database. returns }
																{false if operation could not be completed }
																{delete does not deallocate mem if buffer is freed }
																{all points beyond the one deleted will be moved to }
																{compact mem}
																{passing index <1 as index means delete all points}
			procedure GetPoint (index: longint; var x, y, z: real); 		{get points coordinate in model coords }
																				{if point illegal, it returns 0,0,0}
			function ChangePoint (index: longint; x, y, z: real): boolean; 	{change points coords. true on success }
			procedure Transform (forceCalc: boolean);		{calc trafo-matrix (if necessary) and convert   }
																{all points to their screen representation           }
                                    										{WARNING: A 3D GrafPort must be open. The     }
																{transformations are done for the currently ac-}
																{tive 3D grafport's eye settings.}
			procedure Transform2 (forceCalc: boolean);	(* calc trafo-matrix (if necessary) and convert    *)
																(*all points to their screen representation. gather *)
																(* information for auto-erasure                            *)

			function TransformedPoint (index: longint): Vector4; 	{get transformed coordinates of point 	}
																		{with index. if illegal proc will return 	}
																		{0,0,0. Note that eye is not considered	}
																		{you have to use ToScreen for that    	}
			function ForeignPoint (p: Vector4): Vector4;	{as inherited except that eye trafo is included if}
			override;												{useEye is set										  }
			function WorldToModel (wc: Vector4): Vector4;	{as inherited except that eye trafo is included}
			override;													{if useEye is set.									}
			procedure CalcBounds;								{calc bounds of object on screen and place it in	}
																{the oldBounds variable.								}
		end;


var
	current3Dport: TPort3Dptr; (* currently active 3D port *)


(* procedures to intialize GrafSys and return version numbers *)
procedure InitGrafSysScreen; (* initialize the GrafSys and local variables such as current graf 3D port *)
function GrafSysVersion: longint; (* higword: major release, lower word : minor release. Hex 00010001 means version 1.01 *)
(* procedure to create new 3D windows and grafports *)

function GetNew3DWindow (ID: integer; behind: ptr): WindowPtr; (* allocate a new window from resource *)
function New3DWindow (boundsRect: Rect; title: Str255; visible: BOOLEAN; procID: Integer; behind: WindowPtr; goAwayFlag: BOOLEAN; refCon: LongInt): WindowPtr;
procedure Dispos3DWindow (theWindow: WindowPtr); (* close and release mem occupied by the window *)

(* procedures affecting CURRENT  3D GrafPort *)
procedure Set3DPort (the3DPort: WindowPtr); 	(* tells grafsys in which port to draw. This port MUST have been *)
															(* previously allocated with New3DWindow or GetNew3DWindow  *)
procedure Get3DPort (var the3DPort: WindowPtr); 	(* returns current 3D GrafPort *)
function Is3DPort (thePort: WindowPtr): Boolean;	(* returns TRUE if thePort is a 3D Port *)
procedure SetView (ProjectPlaneSize, ViewPlaneSize: Rect); 	(* sets the viewing and projection plane parameters *)
																			(* of the currently active 3D GrafPort                     *)
																			(* this of course affects the center location             *)
procedure SetCenter (x, y: Integer);				(* Sets center of current 3D grafport to given params  *)
procedure SetEyeChar (UsesEye: Boolean; location: Vector4; thePhi, theTheta, thePitch, theViewangle: real; clipType: clippingType);
															(* Sets eye characteristics and calculates window's master trans- *)
															(* form                                     										 *)
procedure GetEye (var UsesEye: Boolean; var location: Vector4; thePhi, theTheta, thePitch, theViewangle: real; var clipType: clippingType);
															(* returns eye characteristic of current active 3d grafport *)

procedure ToScreen (thePoint: Vector4; var h, v: INTEGER); { transforms a point with x,y,z to screen as seen under	}
																	{current eye settings										}
procedure ProjectPoint (thePoint: Vector4; var h, v: integer);
		{project 3D pointz to screen using projection type}
implementation

(* global variables for the 3D package *)
var
{versionCount: longint; 	(* used for syncing Master Transform of eye. If <> 0 a change to *)
							 	(* the eye has occured and the eye has to be recalculated 		   *)

	lgMaxPoints: longint; (* maximum # of points in model *)

(* GetNew3DWindow will load the WIND template as specified but will also put the *)
(* 3D eye data required on piggyback *)

procedure Init3DPort (var the3DPort: TPort3DPtr);
	begin
		with the3DPort^ do begin
			versionType := cPort3DType;
			theOffscreen.thePort := nil;
			theOffscreen.theDevice := nil;
			ProjectionPlane := theWindow.port.portRect;
			ViewPlane := theWindow.port.portRect; (* the viewplane and project plane are set to the whole window *)
			left := theWindow.port.portRect.left;
			right := theWindow.port.portRect.right;
			top := theWindow.port.portRect.top;
			bottom := theWindow.port.portRect.bottom;
			center.h := 0;
			center.v := 0; (* GeoBench uses topleft as origin *)
			center.h := (left + right) div 2;
			center.v := (top + bottom) div 2; (* center is center of window *)

			useEye := False;
			SetVector4(EyeKoord, 0, 0, 0);
			phi := 0;
			theta := 0;
			pitch := 0;
			ViewAngle := 0; (* parallel projection *)
			d := 0;
			MasterTransform := Identity;
			projection := parallel;
			clipping := none;
			versionsID := 0; (* fresh eye *)
{versionCount := versionCount + 1;}
		end;
	end;

procedure InitGrafSysScreen; (* initialize the GrafSys and local variables such as current graf 3D port *)
	begin
{versionCount := -1;}
		current3Dport := nil;
		lgMaxPoints := LongInt((MaxPointPerBuf + 1)) * LongInt((MaxBuffers + 1));
	end;

function GrafSysVersion: longint; (* higword: major release, lower word : minor release. Hex 00010001 means version 1.01 *)
	begin
		GrafSysVersion := cGrafSysVersion;
	end;

(* places 3D eye data piggyback to window data *)
(* sets current 3D port to newly allocated window if allocated *)
function GetNew3DWindow (ID: integer; behind: ptr): WindowPtr;
	var
		wStorage: Ptr;
		theWindow: WindowPtr;

	begin
		theWindow := nil;
		wStorage := NewPtr(SizeOf(TPort3D));
		if wStorage = nil then begin
			GetNew3DWindow := nil;
			Exit(GetNew3DWindow);
		end;
		theWindow := GetNewCWindow(ID, wStorage, Pointer(behind));
		GetNew3DWindow := theWindow;
		if theWindow = nil then
			Exit(GetNew3DWindow); (* do not init the 3D data since window not valid. Don't set current3Dport to it *)
		Init3Dport(TPort3DPtr(wStorage));
		current3Dport := TPort3DPtr(wStorage);
		SetPort(theWindow);
	end;

(* places 3D eye data piggyback to window data *)
(* sets current 3D port to newly allocated window if allocated *)
function New3DWindow (boundsRect: Rect; title: Str255; visible: BOOLEAN; procID: Integer; behind: WindowPtr; goAwayFlag: BOOLEAN; refCon: LongInt): WindowPtr;
	var
		wStorage: Ptr;
		theWindow: WindowPtr;

	begin
		theWindow := nil;
		wStorage := NewPtr(SizeOf(TPort3D));
		if wStorage = nil then begin
			New3DWindow := nil;
			Exit(New3DWindow);
		end;
		theWindow := NewCWindow(wStorage, boundsRect, title, visible, procID, Pointer(behind), goAwayFlag, refCon);
		New3DWindow := theWindow;
		if theWindow = nil then
			Exit(New3DWindow); (* do not init the 3D data since window not valid. Don't set current3Dport to it *)
		Init3Dport(TPort3DPtr(wStorage));
		current3Dport := TPort3DPtr(wStorage);
		SetPort(theWindow);
	end;

procedure Dispos3DWindow (theWindow: WindowPtr); (* close and release mem occupied by the window *)
	var
		the3DWindow: TPort3DPtr;

	begin
		the3DWindow := TPort3DPtr(theWindow);
		if is3DPort(theWindow) then begin
			if the3DWindow^.theOffscreen.thePort <> nil then
				DisposeOffScreen(the3DWindow^.theOffscreen.thePort, the3DWindow^.theOffscreen.theDevice);
		end;
		CloseWindow(theWindow);
		DisposPtr(ptr(theWindow));
		theWindow := nil;
	end;

procedure Set3DPort (the3DPort: WindowPtr); 	(* tells grafsys in which port to draw. This port MUST have been *)
															(* previously allocated with New3DWindow or GetNew3DWindow  *)
															(* otherwise Set3DPort does nothing								   *)
	begin
		if (GetPtrSize(Ptr(thePort)) = SizeOf(TPort3D)) and (TPort3DPtr(thePort)^.versionType = cPort3DType) then begin
			current3Dport := TPort3Dptr(thePort);
		end;
		SetPort(the3DPort);
	end;

procedure Get3DPort (var the3DPort: WindowPtr); 	(* returns current 3D GrafPort *)
	begin
		the3Dport := WindowPtr(current3Dport);
	end;


function Is3DPort (thePort: WindowPtr): Boolean;	(* returns TRUE if thePort is a 3D Port *)
	var
		theResult: boolean;

	begin
		theResult := GetPtrSize(Ptr(thePort)) = SizeOf(TPort3D);
		if theResult then
			theResult := Tport3Dptr(thePort)^.versionType = cPort3DType;
		Is3Dport := theResult;
	end;

procedure SetView (ProjectPlaneSize, ViewPlaneSize: Rect); 	(* sets the viewing and projection plane parameters *)
																			(* of the currently active 3D GrafPort                     *)
	begin
		with current3Dport^ do begin
			ProjectionPlane := ProjectPlaneSize;
			ViewPlane := ViewPlaneSize;
			left := ProjectPlaneSize.left;
			top := ProjectPlaneSize.top;
			bottom := ProjectPlaneSize.bottom;
			right := ProjectPlaneSize.right;
			center.h := (left + right) div 2;
			center.v := (top + bottom) div 2;
			versionsID := versionsID + 1; (* changes increment. Indicate we must redraw  *)
		end; (* with *)
		ClipRect(ViewPlaneSize);
	end;


procedure SetCenter (x, y: Integer);
	begin
		current3Dport^.center.h := x;
		current3Dport^.center.v := y;
		current3Dport^.versionsID := current3Dport^.versionsID + 1; (* changes increment. Indicate we must redraw  *)
	end;

procedure SetEyeChar (UsesEye: Boolean; location: Vector4; thePhi, theTheta, thePitch, theViewangle: real; clipType: clippingType);
															(* Sets eye characteristics and calculates window's master trans- *)
															(* form                                     										 *)
	var
		theMatrix, theMatrixd: Matrix4;
		p1, p2: Vector4;

	const
		dist = 100;


	begin
		with current3Dport^ do begin
(* first, assign new version ID for eye *)
			versionsID := versionsID + 1;
{versionCount := versionCount + 1;}
			UseEye := UsesEye;
			clipping := clipType;
(* now we calculate the viewange. it derives the d from the viewangle and the HIGHTH of the viewwindow *)
(* a setting of viewangle of zero (= nothing) or 2¹  means NO PERSPECTIVE                                              *)
			theViewangle := theViewangle / 2;
			if (theViewangle >= 2 * Pi) or (theViewangle <= 0) then
				theViewangle := 0;
			if theViewangle = 0 then
				projection := parallel
			else begin
				d := 200 / tan(theViewangle);
				d := center.v / tan(theViewangle);{}
				projection := perspective;
			end;
			theMatrix := Identity; (* start from scratch with 0,0,0, 0,0,0 *)


(* Zero step : translate eye to world viewpoint *)

			MTranslate(theMatrix, -location[1], -location[2], -location[3]); (* since we move world and not eye, negative sign *)

(* 1st step : direction of looking is straight up, through the XY plane *)
(*                 now deviate towards Y-achsis by phi radiants . after    *)
(*                 transformation z-vector lies in XZ plane                       *)

			RotX(theMatrix, -thePhi);

(* 2nd step: rotate around y so the z-vector comes to coincide with *)
(*                world coordinate system                                              *)

			RotY(theMatrix, -theTheta);


(* 3rd step: ratate according to pitch *)

			RotZ(theMatrix, -thePitch);

			MasterTransform := theMatrix;

			phi := thePhi;
			theta := theTheta;
			pitch := thePitch;
			viewangle := theViewangle;
			EyeKoord := location;
(* necessary for eye position and direction *)
		end; (* with *)
	end;


procedure GetEye (var UsesEye: Boolean; var location: Vector4; thePhi, theTheta, thePitch, theViewangle: real; var clipType: clippingType);
															(* returns eye characteristic of current active 3d grafport *)
	begin
		with current3Dport^ do begin
			thePhi := phi;
			theTheta := theta;
			thePitch := pitch;
			location := EyeKoord;
			theViewangle := viewangle;
			UsesEye := UseEye;
			clipType := Clipping;
		end;
	end;

procedure ToScreen (thePoint: Vector4; var h, v: INTEGER); { transforms a point with x,y,z to screen as seen under	}
																	{current eye settings										}

	var
		x, y, z: Real;
		zbyd: Real;

	begin
		if current3DPort^.useEye then
			thePoint := VMult(thePoint, current3DPort^.MasterTransform); (* transform point so we can project *)
		GetVector4(thePoint, x, y, z); (* return to real *)

		if current3DPort^.projection = parallel then begin
			h := Trunc(x) + current3DPort^.center.h;
			v := -Trunc(y) + current3DPort^.center.v
		end
		else begin
			zbyd := 1 / (z / current3DPort^.d + 1);
			h := Trunc(x * zbyd) + current3DPort^.center.h; (* do perspective transformation *)
			v := -Trunc(y * zbyd) + current3DPort^.center.v;
		end

	end;


procedure ProjectPoint (thePoint: Vector4; var h, v: integer);
	var
		x, y, z: real;
		zbyd: Real;

	begin
		GetVector4(thePoint, x, y, z); (* return to real *)

		if current3DPort^.projection = parallel then begin
			h := Trunc(x) + current3DPort^.center.h;
			v := -Trunc(y) + current3DPort^.center.v
		end
		else begin
			zbyd := 1 / (z / current3DPort^.d + 1);
			h := Trunc(x * zbyd) + current3DPort^.center.h; (* do perspective transformation *)
			v := -Trunc(y * zbyd) + current3DPort^.center.v;
		end
	end;


(* procedures for the objects *)

procedure TSGenericObject3D.Init;
	override;
	var
		i: integer;

	begin
		inherited Init; (* init all inherited fields *)
		if errorCode <> 0 then
			Exit(init);
		SetRect(Bounds, 0, 0, 0, 0);
		SetRect(oldBounds, 0, 0, 0, 0);
		for i := 0 to MaxBuffers do
			theBufs[i] := nil;
		theBufs[0] := Point3DBufPtr(NewPtr(SIZEOF(Point3DBufRec)));
		currentBuf := theBufs[0];
		currentIndex := -1;
		numPoints := 0;
		if currentBuf = nil then
			ErrorCode := cOutOfMem; (* flag error condition *)
	end;

procedure TSGenericObject3D.Reset;
	override;
	begin
		inherited Reset;
		SetRect(Bounds, 0, 0, 0, 0);
		SetRect(oldBounds, 0, 0, 0, 0);
	end;

(* extend inherited clone method to duplicate all allocated line buffers *)
function TSGenericObject3D.Clone: TGenericObject;					{duplicate point buffers as well }
	override;

	var
		i: integer;
		theClone: TSGenericObject3D;

	begin
		theClone := TSGenericObject3D(inherited Clone);
		for i := 1 to MaxBuffers do begin
			if theBufs[i] <> nil then begin
				theClone.theBufs[i] := Point3DBufPtr(NewPtr(SIZEOF(Point3DBufRec)));
				theClone.theBufs[i]^ := theBufs[i]^; (* copy contents *)
			end
			else
				theClone.theBufs[i] := nil;
		end;
		Clone := theClone;
	end;

procedure TSGenericObject3D.Kill; 										{deallocate mem. Will kill all sons that inherit}
	override;
	var
		i: integer;

	begin (* first, deallocate all buffers that have been allocated *)
		for i := 0 to MaxBuffers do begin
			if theBufs[i] <> nil then
				DisposPtr(Ptr(theBufs[i]));
		end;
		inherited Kill; (* suicide with proven method, thereby killing all sons that inherit *)
	end;

{GenIndex does no sanity check. Use with caution}
procedure TSGenericObject3D.GenIndex (pointIndex: longint; var BufIndex, bufOffset: integer);
	begin
		BufIndex := pointIndex div (MaxPointPerBuf + 1);
		bufOffset := pointIndex mod (MaxPointPerBuf + 1);
	end;

function TSGenericObject3D.AddPoint (x, y, z: real): longint;		{add a point to the object's database. It returns the }
																				{points reference number if successful or -1 otherwise}
																				{check ErrorCode if function returns -1. }
																				{point count is one-based, i.e. fist point is index 1. Note	 }
																				{that this differs from the internal representation where }
																				{point count is zero-based. COMMON ERROR SOURCE!		 }

	var
		theBufIndex: integer;
		offsetIntoBuf: integer;
		pointIndex: longint;

	begin
		if self.NumPoints >= lgMaxPoints then {exit with index -1 because database is full}
			begin
			AddPoint := -1;
			ErrorCode := cTooManyPoints; (* flag error condition *)
			Exit(AddPoint);
		end;
		pointIndex := self.numPoints; (* self.numPoints is one-based count, pointIndex zero-based *)
		self.genIndex(pointIndex, theBufIndex, offSetIntoBuf);
		if theBufs[theBufIndex] = nil then begin {we need to allocate a new buffer}
		{checkpoint1 : the offset into buffer must be zero. check it }
			if offsetIntoBuf > 0 then
				DebugStr('ATTENTION: Addpoint req. new buffer and offset > 0');
			theBufs[theBufIndex] := Point3DBufPtr(NewPtr(SIZEOF(Point3DBufRec)));
			if theBufs[theBufIndex] = nil then begin
				AddPoint := -1;
				ErrorCode := cOutOfMem; (* flag error condition *)
				Exit(AddPoint);
			end;
		end;
 	{if we are here, buffer has been allocated and all is well}
		SetVector4(theBufs[theBufIndex]^[pointIndex].koords, x, y, z);
		self.numPoints := self.numPoints + 1;  (* update the number of points in model. always 1 greater that currently *)
												(* active point since pointCount is 1 based, pointIndex zero-based *)
		self.objChanged := TRUE;
		AddPoint := self.numPoints;
	end;


function TSGenericObject3D.DeletePoint (index: longint): boolean;	{delete point with passed index from database. returns }
																				{false if operation could not be completed }
																				{delete does not deallocate mem if buffer is freed }
																				{all points beyond the one deleted will be moved to }
																				{compact mem}
																				{when specifying a point, its index is zero-based}
																				{passing 0 as index means delete all points}
	var
		temp: Point3DEntry;
		buffer, offset: integer;

	begin
		index := index - 1; (* make zero-based count *)
		if index >= self.numPoints then begin
			ErrorCode := cIllegalPointIndex;
			DeletePoint := FALSE;
			Exit(DeletePoint);
		end;
		if index <= 0 then begin
			numPoints := 0;
			DeletePoint := TRUE; (* remember, no memory is deallocated *)
			Exit(DeletePoint);
		end;
		while index < self.numPoints - 1 do begin
			self.GenIndex(index + 1, buffer, offset);
			if theBufs[buffer] = nil then
				DebugStr('WARNING: about to access nil buffer while moving::read');
			temp := theBufs[buffer]^[offset]; (* read entry *)
			self.GenIndex(index, buffer, offset);
			if theBufs[buffer] = nil then
				DebugStr('WARNING: about to access nil buffer while moving::write');
			theBufs[buffer]^[offset] := temp; (* read entry *)
			index := index + 1;
		end;
		self.numPoints := self.numPoints - 1;
		DeletePoint := TRUE;
	end;

procedure TSGenericObject3D.GetPoint (index: longint; var x, y, z: real); 		{get points coordinate in model coords }
	var
		temp: Point3DEntry;
		buffer, offset: integer;

	begin
		index := index - 1; (* make zero-based count *)
		if (index < 0) or (index > self.numPoints - 1) then begin
			x := 0;
			y := 0;
			z := 0;
		end
		else begin
			self.GenIndex(index, buffer, offset);
			if theBufs[buffer] = nil then
				DebugStr('WARNING: about to access nil buffer in GetPoint');
			temp := theBufs[buffer]^[offset]; (* read entry *)
			GetVector4(temp.koords, x, y, z);
		end;
	end;


function TSGenericObject3D.ChangePoint (index: longint; x, y, z: real): boolean; 	{change points coords. true on success }
	var
		buffer, offset: integer;

	begin
		index := index - 1; (* make zero-based count *)
		if (index < 0) or (index > self.numPoints - 1) then begin
			ChangePoint := FALSE;
			ErrorCode := cIllegalPointIndex;
			Exit(changePoint);
		end;
		self.GenIndex(index, buffer, offset);
		SetVector4(theBufs[buffer]^[offset].koords, x, y, z);
		self.objChanged := TRUE;
		ChangePoint := TRUE;
	end;


procedure TSGenericObject3D.Transform (forceCalc: boolean);	(* calc trafo-matrix (if necessary) and convert *)
																			(*all points to their screen representation         *)

	var
		buffer, offset: integer;
		pointIndex: longint;
		thePoint: Vector4;
		x, y, z: real;
		zbyd: real;
		dx, dy, mx, my: real; (* geobench adaption parameters for zoom *)

	begin
		dx := current3DPort^.dx;
		dy := current3DPort^.dy;
		mx := current3DPort^.mx;
		my := current3DPort^.my;

		if forceCalc or objChanged then (* object has changed. we must recalc it *)
			begin
			self.CalcTransform; (* resets versionID to zero and calcs xForm, passes on etc *)
			self.screenXform := self.xForm; (* transfer it in case we don't use the eye *)
			versionsID := -1; (* force check for useEye later on *)
		end; (* if forcecalc or version *)

		if versionsID <> current3DPort^.versionsID then begin
			self.screenXform := xForm; (* failsafe in case object didn't change but eye was switched off *)
			versionsID := current3DPort^.versionsID; {sync eye and object. even if we don't use the eye anymore }
			if current3DPort^.useEye then begin
			(* PostConcat the Eye matrix to xForm *)
				self.screenXform := MMult(xForm, current3DPort^.MasterTransform);
			end; (* if useEye fails we still have last xForm in screenXform *)
		end;

(* transformation is pretty simple: for all points: fetch and multiply with xForm Matrix *)
(* we will use currentBuf for faster access. If index mod pointsPerBuf returnes zero we will *)
(* access a new buffer, otherwise we don't calculate anything since this is all straightforward *)

		pointIndex := 0;
		offset := 0;
		buffer := -1; (* will be incremented to 0 right away *)
		while pointIndex < self.numPoints do begin
			if offset mod (MaxPointPerBuf + 1) = 0 then (* advance buffer 1 *)
				begin
				buffer := buffer + 1;
				currentBuf := theBufs[buffer];
				if currentBuf = nil then
					DebugStr('About to access nil buffer in TSGenericObject3D.Transform');
				offset := 0;
			end;
		(* get the point *)
			thePoint := currentBuf^[offset].koords;

		(* Transform the point *)
{thePoint := VMult(thePoint, screenXform); (* transform point so we can project *)
			thePoint := MatrixVectorMult(screenXform, thePoint); (* geoBench adaption *)
			currentBuf^[offset].transformed := thePoint; (* copy transformed point *)
			GetVector4(thePoint, x, y, z); (* return to real *)

{currentBuf^[offset].transformedZ := z;}
			if current3DPort^.projection = parallel then begin
				currentBuf^[offset].Screenx := Trunc(dx + mx * (x + current3DPort^.center.h)); (* geobench adaption *)
				currentBuf^[offset].Screeny := +Trunc(dy + my * (y + current3DPort^.center.v)); (* geobench adaption: down is up *)
			end
			else begin
				zbyd := 1 / (z / current3DPort^.d + 1);
				currentBuf^[offset].Screenx := Trunc(dx + mx * (x * zbyd + current3DPort^.center.h)); (* do perspective transformation *)
				currentBuf^[offset].Screeny := +Trunc(dy + my * (y * zbyd + current3DPort^.center.v));(* geobench adaption: down is up *)
{DebugStr('Not yet adapted to geobench: mx and dx paramteres');{}
			end;

			offset := offset + 1;
			pointIndex := pointIndex + 1;
		end; (* while *)
	end; (* Transform *)

procedure TSGenericObject3D.Transform2 (forceCalc: boolean);	(* calc trafo-matrix (if necessary) and convert    *)
																				(*all points to their screen representation. gather *)
																				(* information for auto-erasure                            *)
																				(* points with negative z will not be collected into *)
																				(* bounds rectangle                                               *)

	var
		buffer, offset: integer;
		pointIndex: longint;
		thePoint: Vector4;
		x, y, z: real;
		zbyd: real;
		sx, sy: integer;

	begin
		if forceCalc or objChanged then (* object has changed. we must recalc it *)
			begin
			self.CalcTransform; (* resets versionID to zero and calcs xForm, passes on etc *)
			self.screenXform := self.xForm; (* transfer it in case we don't use the eye *)
			versionsID := -1; (* force check for useEye later on *)
		end; (* if forcecalc or version *)

		if versionsID <> current3DPort^.versionsID then (* we must postconcat eye to xForm. result in screenXform *)
			begin
			self.screenXform := xForm; (* failsafe in case object didn't change but eye was switched off *)
			versionsID := current3DPort^.versionsID; {sync eye and object. even if we don't use the eye anymore }
			if current3DPort^.useEye then begin
			(* PostConcat the Eye matrix to xForm *)
				self.screenXform := MMult(xForm, current3DPort^.MasterTransform);
			end; (* if useEye fails we still have last xForm in screenXform *)
		end;

(* transformation is pretty simple: for all points: fetch and multiply with xForm Matrix *)
(* we will use currentBuf for faster access. If index mod pointsPerBuf returnes zero we will *)
(* access a new buffer, otherwise we don't calculate anything since this is all straightforward *)
		oldBounds := Bounds;
		SetRect(Bounds, 32760, 32760, -32760, -32760); (* set bounds to minimal empty rect *)
		pointIndex := 0;
		offset := 0;
		buffer := -1; (* will be incremented to 0 right away *)
		while pointIndex < self.numPoints do begin
			if offset mod (MaxPointPerBuf + 1) = 0 then (* advance buffer 1 *)
				begin
				buffer := buffer + 1;
				currentBuf := theBufs[buffer];
				if currentBuf = nil then
					DebugStr('About to access nil buffer in TSGenericObject3D.Transform');
				offset := 0;
			end;
		(* get the point *)
			thePoint := currentBuf^[offset].koords;

		(* Transform the point *)
{thePoint := VMult(thePoint, screenXform); (* transform point so we can project *)
			thePoint := MatrixVectorMult(screenXform, thePoint); (* geoBench adaption *)

			currentBuf^[offset].transformed := thePoint;
			GetVector4(thePoint, x, y, z); (* return to real *)

{currentBuf^[offset].transformedZ := z;}
			if current3DPort^.projection = parallel then begin
				sx := Trunc(x) + current3DPort^.center.h;
				currentBuf^[offset].Screenx := sx;
{sy := -Trunc(y) + current3DPort^.center.v;{}
				sy := +Trunc(y) + current3DPort^.center.v; { GeoBench adaption: View-up is down }
				currentBuf^[offset].Screeny := sy
			end
			else begin
				zbyd := 1 / (z / current3DPort^.d + 1);
				sx := Trunc(x * zbyd) + current3DPort^.center.h; (* do perspective transformation *)
				currentBuf^[offset].Screenx := sx;
{sy := -Trunc(y * zbyd) + current3DPort^.center.v;{}
				sy := +Trunc(y * zbyd) + current3DPort^.center.v; { GeoBench adaption: View-up is down }
				currentBuf^[offset].Screeny := sy;
			end;

			if z >= 0 then (* do this only if point is drawn *)
				begin
				if sx < Bounds.left then (* gather data for autoerase *)
					Bounds.left := sx - 1;
				if sx > Bounds.right then
					Bounds.right := sx + 1;
				if sy < Bounds.top then
					Bounds.top := sy - 1;
				if sy > Bounds.bottom then
					Bounds.bottom := sy + 1;
			end;

			offset := offset + 1;
			pointIndex := pointIndex + 1;
		end; (* while *)
{insetRect(Bounds, -1, -1); {now done in draw and GetLineData }
	end; (* Transform *)

procedure TSGenericObject3D.CalcBounds;			{calc bounds of object on screen and place it in }
																{the oldBounds variable							}
	var
		buffer, offset: integer;
		pointIndex: longint;
		x, y: integer;

	begin
		SetRect(oldBounds, 32760, 32760, -32760, -32760); (* set bounds to minimal empty rect *)
		pointIndex := 0;
		offset := 0;
		buffer := -1; (* will be incremented to 0 right away *)
		while pointIndex < self.numPoints do begin
			if offset mod (MaxPointPerBuf + 1) = 0 then (* advance buffer 1 *)
				begin
				buffer := buffer + 1;
				currentBuf := theBufs[buffer];
				if currentBuf = nil then
					DebugStr('About to access nil buffer in TSGenericObject3D.Transform');
				offset := 0;
			end;

			x := currentBuf^[offset].Screenx; (* access them screen-coords *)
			y := currentBuf^[offset].Screeny;
			if x < oldBounds.left then (* gather data for autoerase *)
				oldBounds.left := x - 1;
			if x > oldBounds.right then
				oldBounds.right := x + 1;
			if y < oldBounds.top then
				oldBounds.top := y - 1;
			if y > oldBounds.bottom then
				oldBounds.bottom := y + 1;

			offset := offset + 1;
			pointIndex := pointIndex + 1;
		end; (* while *)
		insetRect(oldBounds, -1, -1);
	end;

function TSGenericObject3D.TransformedPoint (index: longint): Vector4;
																					{get transformed coordinates of point 	}
																					{with index. if illegal proc will return 	}
																					{0,0,0.  	}
																					{does not apply eye settings to it         }
																					{you have to use ToScreen for that    	}
																					{calls transform if versionId <> 0        }

	var
		buffer, offset: integer;
		theVector: Vector4;

	begin
		if objChanged then (* object has changed. we must recalc it *)
			begin
			self.CalcTransform; (* resets versionID to zero and calcs xForm, passes on etc *)
			self.screenXform := self.xForm; (* transfer it in case we don't use the eye *)
			versionsID := -1; (* force check for useEye later on *)
		end; (* if forcecalc or version *)

		if versionsID <> current3DPort^.versionsID then (* we must postconcat eye to xForm. result in screenXform *)
			begin
			self.screenXform := xForm; (* failsafe in case object didn't change but eye was switched off *)
			versionsID := current3DPort^.versionsID; {sync eye and object. even if we don't use the eye anymore }
			if current3DPort^.useEye then begin
			(* PostConcat the Eye matrix to xForm *)
				self.screenXform := MMult(xForm, current3DPort^.MasterTransform);
			end; (* if useEye fails we still have last xForm in screenXform *)
		end;


		SetVector4(theVector, 0, 0, 0);
		if (index < 1) or (index > self.numPoints) then begin
			ErrorCode := cIllegalPointIndex;
			TransformedPoint := theVector;
			Exit(TransformedPoint);
		end;
		self.GenIndex(index - 1, buffer, offset);
		theVector := theBufs[buffer]^[offset].koords;
		TransformedPoint := VMult(theVector, self.screenXform);
	end;

function TSGenericObject3D.ForeignPoint (p: Vector4): Vector4;
	override;

	begin
		if objChanged then (* object has changed. we must recalc it *)
			begin
			self.CalcTransform; (* resets versionID to zero and calcs xForm, passes on etc *)
			self.screenXform := self.xForm; (* transfer it in case we don't use the eye *)
			versionsID := -1; (* force check for useEye later on *)
		end; (* if forcecalc or version *)

		if versionsID <> current3DPort^.versionsID then (* we must postconcat eye to xForm. result in screenXform *)
			begin
			self.screenXform := xForm; (* failsafe in case object didn't change but eye was switched off *)
			versionsID := current3DPort^.versionsID; {sync eye and object. even if we don't use the eye anymore }
			if current3DPort^.useEye then begin
			(* PostConcat the Eye matrix to xForm *)
				self.screenXform := MMult(xForm, current3DPort^.MasterTransform);
			end; (* if useEye fails we still have last xForm in screenXform *)
		end;
		ForeignPoint := VMult(p, self.screenXform)
	end;


function TSGenericObject3D.WorldToModel (wc: Vector4): Vector4; {xform world coordinates to model coordinates}
	override;

	var
		wcOrigin: Vector4;
		Origin: vector4;

	begin
	{we don't have to change anything because Foreign point is overriden and checks itself }
	{for changes in the object and sceenXform}
		SetVector4(Origin, 0, 0, 0); (* model coordinate origin *)
		wcOrigin := ForeignPoint(Origin); (* get the origin in wc *)
		WorldToModel := VSub(wc, wcOrigin); (* subtract global origin from global point to get local point *)
	end;

procedure TSPoint3D.Reset;
	begin
		size := 2;
		SetVector4(Koord, 0, 0, 0);
		inherited Reset;
	end;

procedure TSPoint3D.Init;
	begin
		size := 2;
		SetVector4(Koord, 0, 0, 0);
		inherited Init;
	end;

procedure TSPoint3D.Draw;
	override;
	var
		thePoint: Vector4;
		x, y, z: real;
		h, v: integer;
		theRect: rect;
		zbyd: real;

	begin
		if (objChanged) then begin
			self.CalcTransform; (* resets versionID to zero and calcs xForm, passes on etc *)
		end;
		if current3DPort^.useEye then begin
			(* first check to see if the eye needs a recalc -- NOT IMPLEMENTED SINCE EYE ALWAYS RECALCS IMMEDIATELY *)
			(* PostConcat the Eye matrix to xForm *)
			self.xForm := MMult(xForm, current3DPort^.MasterTransform);
		end;

		thePoint := VMult(Koord, xForm);
		GetVector4(thePoint, x, y, z);
		if current3DPort^.projection = parallel then

			begin
			h := Trunc(x) + current3DPort^.center.h;
			v := -Trunc(y) + current3DPort^.center.v
		end
		else begin
			zbyd := 1 / (z / current3DPort^.d + 1);
			h := Trunc(x * zbyd) + current3DPort^.center.h; (* do perspective transformation *)
			v := -Trunc(y * zbyd) + current3DPort^.center.v;
		end;
		SetRect(theRect, h, v, h, v);
		InsetRect(theRect, -size, -size);
		PaintRect(theRect);
	end;


procedure TSLine3D.Draw;
	override;
	var
		thePoint: Vector4;
		x, y, z: real;
		h, v: integer;
		zbyd: real;

	begin
		if (objChanged) then begin
			self.CalcTransform; (* resets versionID to zero and calcs xForm, passes on etc *)
		end;
		if current3DPort^.useEye then begin
			(* first check to see if the eye needs a recalc -- NOT IMPLEMENTED SINCE EYE ALWAYS RECALCS IMMEDIATELY *)
			(* PostConcat the Eye matrix to xForm *)
			self.xForm := MMult(xForm, current3DPort^.MasterTransform);
		end;

		thePoint := VMult(FromLoc, xForm);
		GetVector4(thePoint, x, y, z);
		if current3DPort^.projection = parallel then

			begin
			h := Trunc(x) + current3DPort^.center.h;
			v := -Trunc(y) + current3DPort^.center.v
		end
		else begin
			zbyd := 1 / (z / current3DPort^.d + 1);
			h := Trunc(x * zbyd) + current3DPort^.center.h; (* do perspective transformation *)
			v := -Trunc(y * zbyd) + current3DPort^.center.v;
		end;
		MoveTo(h, v);
		thePoint := VMult(ToLoc, xForm);
		GetVector4(thePoint, x, y, z);
		if current3DPort^.projection = parallel then

			begin
			h := Trunc(x) + current3DPort^.center.h;
			v := -Trunc(y) + current3DPort^.center.v
		end
		else begin
			zbyd := 1 / (z / current3DPort^.d + 1);
			h := Trunc(x * zbyd) + current3DPort^.center.h; (* do perspective transformation *)
			v := -Trunc(y * zbyd) + current3DPort^.center.v;
		end;
		LineTo(h, v);
	end;
end. {impl }