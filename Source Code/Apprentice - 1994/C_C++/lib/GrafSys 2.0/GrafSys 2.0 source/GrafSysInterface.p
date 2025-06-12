unit GrafSysInterface;

{this unit declares globals and procedures used to interface the GrafSys with geobench}
{ written 1993 by C. Franz }

interface

uses
	Matrix, Transformations, OffscreenCore, GrafSysCore, GrafSysScreen, GrafSysObject, Resources, OffScreenGraphics, GrafSysC, GeoBenchUtility, threeDVision;

const
	MaxPolyLayer = 100;

type
	TPolygon3dObj = object(TSGenericObject3D)
			procedure Draw;
			override;
			procedure Setz (z: extended);
		end;

var
	EyeVector: Vector4;
	CurrentEyeTrafo: Matrix4;
	useQD: Boolean;
	theCoordSys: TSObject3D;
	visibility3d: Boolean;

procedure InitGrafSysInterface;
{initialize the interface routines }


procedure SetCurrentEye (M: trafoMatrT);
{Get GeoBench's Trafo Matrix and convert it to GrafSys Trafo Matrix,}
{set the current eye trafo matrix to it }


implementation

procedure InitGrafSysInterface;
	begin
		SetVector4(EyeVector, 0, 0, 0);
		CurrentEyeTrafo := Identity;
		theCoordSys := GetNewNamedObject('XYZ Coord');
		failNil(Handle(theCoordSys));
		theCoordSys.Reset;
	end;

procedure SetCurrentEye (M: trafoMatrT);
	var
		i: integer;
		j: integer;
	begin
		for i := 1 to 4 do
			for j := 1 to 4 do
				CurrentEyeTrafo.M[i, j] := M[i, j];
		CurrentEyeTrafo.identityFlag := FALSE; (* always!!! *)
		if current3DPort <> nil then begin
			current3DPort^.MasterTransform := CurrentEyeTrafo;
			current3DPort^.versionsID := current3Dport^.versionsID + 1; (* changes increment. Indicate we must redraw  *)
		end
		else
			DebugStr('Trying to set current eye trafo without having 3D window allocated.');
	end;


procedure TPolygon3dObj.Draw;
	override;

	var
		theQDPoly: PolyHandle;
		firstX, firstY: integer;
		lastX, lasty, currX, currY: integer;
		currPoly: integer;
		pointIndex: longint;
		offset: integer;
		buffer: integer;

	begin
		self.Transform(false); (* transform if we need to. Now all points are ready to draw *)
(* all points are accessed incrementally until we are done *)
(* Remember, GragSys internal index is ZCNT, PolyCount is OCNT! *)

		currPoly := 1;
		pointIndex := 0;
		offset := 0;
		buffer := -1; (* will be incremented to 0 right away *)
		if numPoints = 0 then (* no points to be drawn *)
			exit(Draw);


		theQDPoly := OpenPoly; (* open a new polygon *)

	(* calculate point address in memory and access first point in buffer *)
		if offset mod (MaxPointPerBuf + 1) = 0 then (* advance buffer 1 *)
			begin
			buffer := buffer + 1;
			currentBuf := theBufs[buffer];
			if currentBuf = nil then
				DebugStr('About to access nil buffer in TSGenericObject3D.Transform');
			offset := 0;
		end;
		(* get the point *)
		firstX := currentBuf^[offset].screenX;
		firstY := currentBuf^[offset].screenY;
		MoveTo(firstX, firstY);
		offset := offset + 1;
		pointIndex := pointIndex + 1;

		while pointIndex < numPoints do begin (* nur '<' wegen ZCNT compare OCNT *)
		(* calculate point address in memory *)
			if offset mod (MaxPointPerBuf + 1) = 0 then (* advance buffer 1 *)
				begin
				buffer := buffer + 1;
				currentBuf := theBufs[buffer];
				if currentBuf = nil then
					DebugStr('About to access nil buffer in TSGenericObject3D.Transform');
				offset := 0;
			end;
		(* get the point *)
			LineTo(currentBuf^[offset].screenX, currentBuf^[offset].screenY);
			offset := offset + 1;
			pointIndex := pointIndex + 1;
		end; (* while pointindex < polygon endpoint *)

		LineTo(firstX, firstY); (* back to first point *)
		ClosePoly;
	(* now draw it *)
		if visibility3d then (* hidden line *)
			FillPoly(theQDPoly, white);
		FramePoly(theQDPoly);
		KillPoly(theQDPoly);

	end; (* TPolygon3dObj.Draw *)

procedure TPolygon3dObj.SetZ (z: extended);

	var
		i: longint;
		x, y, dummy: real;
		theErr: Boolean;

	begin
		if numPoints < 1 then
			exit(SetZ);
		for i := 1 to numPoints do begin
			GetPoint(i, x, y, dummy);
			theErr := ChangePoint(i, x, y, z);
		end;
	end;


end.