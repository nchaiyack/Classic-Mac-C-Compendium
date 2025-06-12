unit GrafSysObject;

interface
uses
	Matrix, Transformations, OffscreenCore, GrafSysCore, GrafSysScreen;

const
	MaxLine = 8000;

type

	LineEntry = record
			fromP, toP: longint; (* max 8000 lines per model supported in this incarnation.  *)
			hs, vs, he, ve: integer; (* for fast drawing. buffers transformed locations *)
			newline: boolean; (* for optimization. if true, no MoveTo required *)
			newLineColor: boolean;
			LineColor: RGBColor;
		end;

	LineBufPtr = ^LineBufRec;
	LineBufRec = array[1..MaxLine] of LineEntry;

	TSObject3D = object(TSGenericObject3D)
			Lines: LineBufPtr;
			numLines: integer;
			AutoErase: Boolean;
			UseBounds: Boolean;
			procedure Init;
			override;
			function Clone: TGenericObject;	{also clone line description buffer}
			override;
			procedure Reset;
			override;
			procedure Kill;
			override;
			function AddLine (fIndex, tIndex: longint): integer;		{add line to objects database. returns line index or -1}
			function ChangeLine (LineIndex, fIndex, tIndex: longint): boolean;	{change line description of line with index }
																					{lineIndex. True if successful				}
			function ChangeLineColor (LineIndex: longint; theColor: RGBColor): boolean;
																		{change the color from this line on for all following }
																		{until the next ChangeColor command                      }
			function GetLineColor (LineIndex: longint; var theColor: RGBColor; var ChangeHere: boolean): Boolean;
																		{returns the currently active color of specified line}
			function KeepLineColor (LineIndex: longint): boolean;	{deletes change linecolor information. This line and }
																		{all following will have the same color as the pre-  }
																		{vious													  }
			function DeleteLine (LineIndex: integer): Boolean;		{delete whole line from model. True on success}
			function DeletePoint (index: longint): boolean;			{override inherited proc of this kind. This one checks}
			override;														{first if point is referenced to by a point. If so, it }
																		{returns false and doesn't delete the point            }
			procedure GetLine (lineIndex: integer; var src, tgt: LongInt); {returns start and endpoint of line}
			procedure BuildNewLines;	{should not be called from the outside}
			procedure CollectLineData; {internal use only. fill the screen vals from point definition into line array}
			procedure SetAutoerase (TurnOn: Boolean);				{controls setting of autoerase flag if switched on, }
																		{this procedure will initialize the oldBounds var    }
			procedure SetUseBounds (TurnOn: Boolean);				{tells Draw and fDraw to collect bouding box data}
			procedure Draw;											{recalcs if neccessary, erases old image if auto- }
			override;														{erase on, redraws all objects lines                     }
			procedure fDraw;											{like Draw but it collects data prior to drawing }
																		{thus making the actual drawing process a bit  }
																		{faster but the whole call is slower than Draw }
			procedure Erase;											{erase image of myself. this calcs and uses bounds}
		end;

{Global Procedures for GrafSys}
procedure InitGrafSys;
procedure ArithmeticClip (var startV, endV: Point3DEntry; var skipThis, clippedThis: boolean; var sx, sy, ex, ey: integer);																		{arithmetically clips a line that connects startV,endV }
																		{if it intersects the Z=0 plane. If it is completely behind }
																		{the Z=0 plane, skipThis is TRUE, if it intersects with }
																		{the plane, clippedThis becomes true and sx..ey contain}
																		{the new screen coordinates                                        }


implementation

type
	screenBuffer = array[1..MaxLine] of record
			sx, sy: integer;
			ex, ey: integer;
			newLine: boolean;
			newLineColor: boolean;
			LineColor: RGBColor;
		end;
	screenBufPtr = ^screenBuffer;


var
	theBlack: RGBColor;
	lineBuffer: screenBufPtr;
	center: Point; (* screen center in local coords of current 3d grafport *)
	thed: real;
	screenBufNumLines: integer; (* number of lines in scren buffer *)

procedure InitGrafSys;
	begin
		InitMatrix; (* initialize the Matrix Package *)
		lineBuffer := screenBufPtr(NewPtr(SIZEOF(screenBuffer)));
		InitGrafSysScreen;
		theBlack.red := 0;
		theBlack.green := $0000;
		theBlack.blue := 0;
	end;

(* Clipping works the following way:  Eye orientation is looking in direction of positive z !!!!                               *)
(*    - if both start and endpoint are behind the xy plane (have negative z-vals)  then the line is not shown at all. *)
(*    - if both points have negative z-vals, the line is drawn entirely, no clipping required                                   *)
(*    - otherwise the line is intersected with the xy plane and drawn from the point with positive z value to the  *)
(*       intersection point                                                                                                                                    *)

(* new clipping algorithm :                                                                                                     *)
(* first get start and endpoint                                                                                                 *)
(* clipping only required if on opposite sides of the projection screen                                       *)
(* if on opposite sides then we have to clip. the point to clip is always the endpoint of line, so   *)
(*     we have to switch the two points if the endpoint is on the POSITIVE (=legal) side of plane  *)

(* the vars have the folloving meaning :                 *)
(* s    : vector -- startpoint                                   *)
(* e    : vector -- endpoint                                     *)
(* dir : vector -- direction                                    *)
(* t    : real -- parameter to calculate intersection *)
(* d    : vector -- Intersection Point                       *)


procedure ArithmeticClip (var startV, endV: Point3DEntry; var skipThis, clippedThis: boolean; var sx, sy, ex, ey: integer);

	type
		realV = array[1..3] of real;

	var
		xform, xform2: Matrix4;
		thePoint, dir, d, dummyV: realV;
		zbyd: Real;
		lineCount: integer;
		clipstart: boolean;
		t: Real;
		eyeSafetyDist: real;
		s, e: realV;

	begin
		skipThis := FALSE;
		clippedthis := FALSE;
{startV := theScrnObj^.Point[sp];}
{endV := theScrnObj^.Point[ep];       (* now we have start & endpoint for clipping in 3D *)
		GetVector4(startV.transformed, s[1], s[2], s[3]);
		GetVector4(endV.transformed, e[1], e[2], e[3]);
		if ((s[3] <= 0) and (e[3] <= 0)) or ((s[3] > 0) and (e[3] > 0)) then begin
			if ((s[3] <= 0) and (e[3] <= 0)) then (* no line is drawn *)
				skipThis := TRUE
			else begin (* whole line can be drawn, transfer it to the line buffer *)
				sx := startV.screenx; (* perspective xform has been applied already *)
				sy := startV.screeny;
				ex := endV.screenx;
				ey := endV.screeny;
			end;
		end
		else (* we have to clip. will always clip endpoint *)
			begin
			clippedThis := TRUE;
			if s[3] < 0 then (* we have to switch start and endpoint since endpoint is legal one *)
				begin
				dummyV := s;
				s := e;
				e := dummyV;
				sx := endV.screenx; (* these screen coords don't have to be *)
				sy := endV.screeny; (* recalculated *)
			end
			else begin
				sx := startV.screenX;
				sy := startV.screenY;
			end; (* no switch *)

			dir[1] := e[1] - s[1]; (* now calc direction vector *)
			dir[2] := e[2] - s[2];
			dir[3] := e[3] - s[3];

			t := (0 - s[3]) / dir[3]; (* calc parameter for intersection *)
			d[1] := s[1] + (t * dir[1]); (* calc intersection Point *)
			d[2] := s[2] + (t * dir[2]);
			d[3] := 0;

(* now we have to perspective-project the intersection point *)
			if current3Dport^.projection = perspective then begin
				zbyd := 1 / (d[3] / thed + 1);
				ex := Trunc((d[1] * zbyd)) + center.h; (* do perspective transformation *)
				ey := -Trunc((d[2] * zbyd)) + center.v;
			end
			else begin
				ex := Trunc(d[1]) + center.h; (* do parallel projection *)
				ey := -Trunc(d[2]) + center.v;
			end; (* parallel *)
		end; (* else we have to clip *)

	end; (* arithmetic clip *)




procedure TSObject3D.Init;
	begin
		inherited Init;
		if ErrorCode <> noErr then
			Exit(Init);
		numLines := 0;
		SetRect(Bounds, 0, 0, 0, 0);
		oldBounds := Bounds;
		AutoErase := False;
		UseBounds := FALSE;
		Lines := LineBufPtr(NewPtr(SIZEOF(LineBufRec)));
		if Lines = nil then
			ErrorCode := cOutOfMem;
	end;

procedure TSObject3D.Reset;
	override;
	begin
		inherited Reset;
		AutoErase := FALSE;
		UseBounds := False;
	end;

{Clone: extend this procedure to also allocate a line buffer and copy all data from }
{          the original																		}

function TSObject3D.Clone: TGenericObject;
	override;

	var
		theClone: TSObject3D;

	begin
		theClone := TSObject3D(inherited Clone);
		theClone.Lines := LineBufPtr(NewPtr(SIZEOF(LineBufRec)));
		if theClone.Lines = nil then
			theClone.ErrorCode := cOutOfMem;
		theClone.Lines^ := self.Lines^; (* copy the whole structure *)
		Clone := theClone;
	end;

procedure TSObject3D.Kill;
	override;
	begin
		DisposPtr(Ptr(Lines));
		inherited Kill;
	end;

procedure TSObject3D.BuildNewLines;	{should not be called from the outside}
	var
		index: integer;

	begin
		index := 2; (* check all lines starting with line two *)
		while index <= numLines do begin
			if Lines^[index].fromP = Lines^[index - 1].toP then
				Lines^[index].newLine := False
			else
				Lines^[index].newline := TRUE;
			index := index + 1;
		end;
		if numLines > 0 then
			Lines^[1].newLine := TRUE; (* first line always true *)
	end;

function TSObject3D.AddLine (fIndex, tIndex: longint): integer; {add line to objects database. returns line index or -1 }
	begin
		fIndex := fIndex - 1;
		if (fIndex < 0) or (fIndex > numPoints) then begin
			ErrorCode := cIllegalPointIndex;
			AddLine := -1;
			Exit(AddLine);
		end;
		tIndex := tIndex - 1; (* make f and t zero-based *)
		if (tIndex < 0) or (tIndex > numPoints) then begin
			ErrorCode := cIllegalPointIndex;
			AddLine := -1;
			Exit(AddLine);
		end;

		if numLines < MaxLine then begin
			numLines := numLines + 1;
			Lines^[numLines].fromP := fIndex;
			Lines^[numLines].toP := tIndex;
			Lines^[numlines].newLineColor := FALSE;
			if numLines > 1 then
				if Lines^[numLines].fromP = Lines^[numLines - 1].toP then
					Lines^[numLines].newLine := False
				else
					Lines^[numLines].newline := TRUE
			else (* numLines = 1 *)
				Lines^[numLines].newline := TRUE;
			AddLine := numLines;
			objChanged := TRUE;
		end
		else begin
			ErrorCode := cTooManyLines;
			AddLine := -1;
		end;
	end;

function TSObject3D.ChangeLineColor (LineIndex: longint; theColor: RGBColor): boolean;
																		{change the color from this line on for all following }
																		{until the next ChangeColor command                      }
	begin
		if LineIndex <= numLines then begin
			Lines^[LineIndex].newLineColor := TRUE;
			Lines^[LineIndex].LineColor := theColor;
			ChangeLineColor := TRUE;
		end
		else begin
			ErrorCode := cIllegalLineIndex;
			ChangeLineColor := FALSE;
		end;
	end;

function TSObject3D.GetLineColor (LineIndex: longint; var theColor: RGBColor; var ChangeHere: boolean): Boolean;
																		{returns the currently active color of specified line}
	var
		index: longint;

	begin
		GetLineColor := TRUE;
		theColor.red := 0;
		theColor.green := 0;
		theColor.blue := 0;
		if LineIndex <= numLines then begin
			index := 1;
			while index <= LineIndex do (* walk down all lines and change line color if neccessary *)
				begin
				ChangeHere := Lines^[LineIndex].newLineColor;
				if ChangeHere then
					theColor := Lines^[LineIndex].LineColor;
				index := index + 1;
			end;
		end
		else begin
			ErrorCode := cIllegalLineIndex;
			GetLineColor := FALSE;
		end;
	end;


function TSObject3D.KeepLineColor (LineIndex: longint): boolean;
																		{deletes change linecolor information. This line and }
																		{all following will have the same color as the pre-  }
																		{vious													  }
	begin
		if LineIndex <= numLines then begin
			Lines^[LineIndex].newLineColor := FALSE;
			KeepLineColor := TRUE;
		end
		else begin
			ErrorCode := cIllegalLineIndex;
			KeepLineColor := FALSE;
		end;
	end;

function TSObject3D.ChangeLine (LineIndex, fIndex, tIndex: longint): boolean;	{change line description of line with index }
																							{lineIndex. True if successful				}
	begin
		fIndex := fIndex - 1;
		if (fIndex < 0) or (fIndex > numPoints) then begin
			ErrorCode := cIllegalPointIndex;
			ChangeLine := FALSE;
			Exit(ChangeLine);
		end;
		tIndex := tIndex - 1; (* make f and t zero-based *)
		if (tIndex < 0) or (tIndex > numPoints) then begin
			ErrorCode := cIllegalPointIndex;
			ChangeLine := FALSE;
			Exit(ChangeLine);
		end;

		if LineIndex <= numLines then begin
			Lines^[LineIndex].fromP := fIndex;
			Lines^[LineIndex].toP := tIndex;
			if LineIndex > 1 then
				if Lines^[LineIndex].fromP = Lines^[LineIndex - 1].toP then
					Lines^[LineIndex].newLine := False
				else
					Lines^[LineIndex].newline := TRUE
			else (* LineIndex = 1 *)
				Lines^[LineIndex].newline := TRUE;
			ChangeLine := TRUE;
			objChanged := TRUE;
		end
		else begin
			ErrorCode := cIllegalLineIndex;
			ChangeLine := FALSE;
		end;
	end;

function TSObject3D.DeleteLine (LineIndex: integer): Boolean;	{delete whole line from model. True on success}
	var
		index: integer;

	begin
		if (LineIndex > numLines) or (LineIndex < 0) then begin
			ErrorCode := cIllegalLineIndex;
			DeleteLine := FALSE;
			Exit(DeleteLine);
		end;
(* now move all line descs from above down once *)
		index := lineIndex;
		while index < numLines - 1 do begin
			Lines^[index] := Lines^[index + 1];
			index := index + 1;
		end;

(* rebuild newline at the deleted spot *)
		if LineIndex > 1 then
			if Lines^[LineIndex].fromP = Lines^[LineIndex - 1].toP then
				Lines^[LineIndex].newLine := False
			else
				Lines^[LineIndex].newline := TRUE
		else (* LineIndex = 1 *)
			Lines^[LineIndex].newline := TRUE;
		numLines := numLines - 1;
		objChanged := TRUE;
		DeleteLine := TRUE;
	end;

procedure TSObject3D.GetLine (lineIndex: integer; var src, tgt: LongInt); {returns start and endpoint of line or -1,-1}
	begin
		if (LineIndex > numLines) or (LineIndex < 0) then begin
			ErrorCode := cIllegalLineIndex;
			src := -1;
			tgt := -1;
			Exit(GetLine);
		end;
		src := Lines^[lineIndex].fromP + 1;
		tgt := Lines^[lineIndex].toP + 1;
	end;

function TSObject3D.DeletePoint (index: longint): boolean;	{override inherited proc of this kind. This one checks}
	override;															{first if point is referenced to by a point. If so, it }
																		{returns false and doesn't delete the point            }
	var
		hasRef: boolean;
		lineIndex: integer;

	begin
(* look if a point is referenced by any of the lines *)
		hasRef := FALSE;
		lineIndex := 1;
		while not hasRef and (lineIndex <= numLines) do begin
			if Lines^[lineIndex].fromP = index then
				hasRef := TRUE;
			if Lines^[lineIndex].toP = index then
				hasRef := TRUE;
			lineIndex := lineIndex + 1;
		end;
		if not hasRef then
			DeletePoint := inherited DeletePoint(index)
		else begin
			ErrorCode := cCantDeletePoint;
			DeletePoint := FALSE;
		end;
		objChanged := TRUE;
		DeletePoint := TRUE;
	end;



procedure TSObject3D.SetAutoerase (TurnOn: Boolean);
	begin
		Autoerase := TurnOn;
		if AutoErase then
			CalcBounds;
	end;

procedure TSObject3D.SetUseBounds (TurnOn: Boolean);		{tells Draw and fDraw to collect bouding box data}
	begin
		UseBounds := TurnOn;
	end;


procedure TSObject3D.CollectLineData; {internal use only. fill the screen vals from point definition into line array}

	var
		index: integer;
		lBufIndex: integer; (* index to the line number in linebuf. always <= numLines *)
		BufIndex, bufOffset: integer;
		tempS, tempE: Point3DEntry;
		clippedLast: Boolean; (* if this is true, the next line must have NewLine set to true *)
		newLine: boolean;
		skipThis: boolean;
		ClipMode: ClippingType;
		startx, starty, startz: real;
		endx, endy, endz: real;
		sx, sy, ex, ey: integer;
		ClippedThis: Boolean;

	begin
		index := 1;
		lBufIndex := 1;
		skipThis := FALSE;
		ClipMode := current3DPort^.clipping;
		clippedLast := FALSE;
		center := current3DPort^.center;
		thed := current3DPort^.d;
		while index <= numLines do begin
			newLine := Lines^[index].newLine or clippedLast;
			GenIndex(Lines^[index].toP, BufIndex, bufOffset); (* this is executed anyways *)
			tempE := theBufs[BufIndex]^[bufOffset]; (* read entry *)
			GenIndex(Lines^[index].fromP, BufIndex, bufOffset);
			tempS := theBufs[BufIndex]^[bufOffset]; (* read entry *)
			LineBuffer^[lBufIndex].newLineColor := Lines^[index].newLineColor;
			LineBuffer^[lBufIndex].lineColor := Lines^[index].lineColor;
(* do clipping *)

			case clipMode of
				none:  begin (* do nothing, just copy *)
					LineBuffer^[lBufIndex].sx := tempS.screenx;
					LineBuffer^[lBufIndex].sy := tempS.screeny;
					LineBuffer^[lBufIndex].ex := tempE.screenx;
					LineBuffer^[lBufIndex].ey := tempE.screeny;
					LineBuffer^[lBufIndex].newLine := newLine;
					lBufIndex := lBufIndex + 1;
				end;

				arithmetic:  begin
					ArithmeticClip(tempS, tempE, skipThis, clippedThis, sx, sy, ex, ey);
					if skipThis then
						clippedLast := TRUE
					else begin
						LineBuffer^[lBufIndex].sx := sx; (* copy data to buffer *)
						LineBuffer^[lBufIndex].sy := sy;
						LineBuffer^[lBufIndex].ex := ex;
						LineBuffer^[lBufIndex].ey := ey;
						LineBuffer^[lBufIndex].newLine := newLine or clippedThis;
						lBufIndex := lBufIndex + 1;
					(* if something was clipped we might have to update the bounds array 	*)
					(* the point that didn't get included was always the endpoint that got 		*)
					(* clipped to the projection plane                                                         	*)
						if newLine or clippedThis then begin
							if ex < Bounds.left then (* gather data for autoerase *)
								Bounds.left := ex - 1;
							if ex > Bounds.right then
								Bounds.right := ex + 1;
							if ey < Bounds.top then
								Bounds.top := ey - 1;
							if ey > Bounds.bottom then
								Bounds.bottom := ey + 1;
						end;
						clippedLast := clippedThis;
					end;
				end;


				fast: (* very simple clipping method : remove all lines that fall at least partwise off the screen *)
					begin
					clippedLast := FALSE;
					GetVector4(tempS.transformed, startx, starty, startz);
					GetVector4(tempE.transformed, endx, endy, endz);
					if (startz < 0) or (endz < 0) then begin
						clippedLast := TRUE; (* don't copy line *)
					end
					else begin
						LineBuffer^[lBufIndex].sx := tempS.screenx;
						LineBuffer^[lBufIndex].sy := tempS.screeny;
						LineBuffer^[lBufIndex].ex := tempE.screenx;
						LineBuffer^[lBufIndex].ey := tempE.screeny;
						LineBuffer^[lBufIndex].newLine := newLine;
						lBufIndex := lBufIndex + 1;
						clippedLast := FALSE;
					end;
				end;

				otherwise
					DebugStr('Unknown clipping method. TSObject3D.CollectLineDat')
			end; (* case clipMode *)

(* end of clipping *)

			index := index + 1;
		end; (* while index *)
		screenBufNumLines := lBufIndex - 1; (* store number of lines in screenBuf *)
		insetRect(Bounds, -1, -1); (* just do it anyways *)
	end;


procedure TSObject3D.Draw;
	override;
	var
		index: integer;
		BufIndex, bufOffset: integer;
		temp, tempS, tempE: point3DEntry;
		theColor: RGBColor;
		skippedLast: Boolean;
		needMoveTo: Boolean;
		skipThis, clippedThis: boolean;
		sx, sy, ex, ey: integer;
		ClipMode: ClippingType;
		startx, starty, startz: real;
		endx, endy, endz: real;

	begin
(* first, set the current color to black *)
		RGBForeColor(theBlack);
		if Autoerase or UseBounds then begin
			Transform2(FALSE); (* calc transform (if neccessary), transfor and gather autoerase data as well *)
			if Autoerase then
				EraseRect(oldBounds); (* erase old image. Its rect was stored in oldRect *)
		end
		else
			Transform(FALSE);

	(* now begin drawing all lines of the object *)
		ClipMode := current3DPort^.clipping;
		center := current3DPort^.center;
		thed := current3DPort^.d;
		skippedLast := false;
		index := 1;
		while index <= numLines do begin

			if Lines^[index].newLineColor then
				RGBForeColor(LineBuffer^[index].LineColor);
			needMoveTo := skippedLast or Lines^[index].newLine; (* test if we need to use MoveTo call *)
			skippedLast := False; (* reset for this round of clipping *)

(* new we calculate clipping *)

			case clipMode of
				none:  begin (* do nothing, just draw *)
					if needMoveTo then (* do only if we have to do a moveTo *)
						begin
						GenIndex(Lines^[index].fromP, BufIndex, bufOffset);
						temp := theBufs[BufIndex]^[bufOffset]; (* read entry *)
						Lines^[index].hs := temp.screenx;
						Lines^[index].vs := temp.screeny;
						MoveTo(temp.screenx, temp.screeny);
					end;
					GenIndex(Lines^[index].toP, BufIndex, bufOffset);
					temp := theBufs[BufIndex]^[bufOffset]; (* read entry *)
					Lines^[index].he := temp.screenx;
					Lines^[index].ve := temp.screeny;
					LineTo(temp.screenx, temp.screeny);
				end;

				arithmetic:  begin
					GenIndex(Lines^[index].fromP, BufIndex, bufOffset);
					tempS := theBufs[BufIndex]^[bufOffset]; (* read entry *)
					GenIndex(Lines^[index].toP, BufIndex, bufOffset);
					tempE := theBufs[BufIndex]^[bufOffset]; (* read entry *)
					ArithmeticClip(tempS, tempE, skipThis, clippedThis, sx, sy, ex, ey);
					if skipThis then
						skippedLast := TRUE
					else begin
						if clippedThis then (* do this only if point is drawn *)
							begin
							if ex < Bounds.left then 	(* gather data for autoerase. only if clipped 		*)
								Bounds.left := ex - 1;	(* the point that needs to be checked is always 	*)
							if ex > Bounds.right then	(* the endpoint (sx,sy)							*)
								Bounds.right := ex + 1;
							if ey < Bounds.top then
								Bounds.top := ey - 1;
							if ey > Bounds.bottom then
								Bounds.bottom := ey + 1;
						end;
						if needMoveTo or clippedThis then (* we need move to *)
							MoveTo(sx, sy);
						LineTo(ex, ey); (* draw it *)
						skippedLast := clippedThis; (* indicate we need a moveto *)
					end;
				end;


				fast: (* very simple clipping method : remove all lines that fall at least partwise off the screen *)
					begin
					GenIndex(Lines^[index].fromP, BufIndex, bufOffset);
					tempS := theBufs[BufIndex]^[bufOffset]; (* read entry *)
					GenIndex(Lines^[index].toP, BufIndex, bufOffset);
					tempE := theBufs[BufIndex]^[bufOffset]; (* read entry *)
					GetVector4(tempS.transformed, startx, starty, startz);
					GetVector4(tempE.transformed, endx, endy, endz);
					if (startz < 0) or (endz < 0) then begin
						skippedLast := TRUE; (* don't copy line *)
					end
					else begin (* draw line *)
						if needMoveTo then begin
						{Lines^[index].hs := tempS.screenx;}
						{Lines^[index].vs := tempS.screeny;}
							MoveTo(tempS.screenx, tempS.screeny);
						end;

					{Lines^[index].he := tempE.screenx;}
					{Lines^[index].ve := tempE.screeny;}
						LineTo(tempE.screenx, tempE.screeny);
						skippedLast := FALSE;
					end;
				end;
				otherwise
					DebugStr('Unknown clipping method');
			end; (* case *)

(* end of clipping *)

			index := index + 1;
		end; (* while index *)
		insetRect(Bounds, -1, -1);
		hasChanged := FALSE;
	end;


procedure TSObject3D.fDraw;
	override;
	var
		index: integer;

	begin
(* first, set the current color to black *)
		RGBForeColor(theBlack);
		if Autoerase or useBounds then begin
			Transform2(FALSE); (* calc transform (if neccessary), transfor and gather autoerase data as well *)
								(* transform2 will move bounds -> oldBounds for ersure of old image *)
		end
		else
			Transform(FALSE);

	(* now begin drawing all lines of the object *)

		CollectLineData; (* pre-gather all line-data for faster drawing. This includes clipping *)
		index := 1;

		if AutoErase then
			EraseRect(oldBounds); (* erase old image. Its rect was stored in oldBounds *)

		while index <= screenBufNumLines do begin
			if LineBuffer^[index].newLineColor then (* first check the selected color *)
				RGBForeColor(LineBuffer^[index].LineColor);
			if LineBuffer^[index].newLine then
				MoveTo(LineBuffer^[index].sx, LineBuffer^[index].sy);
			LineTo(LineBuffer^[index].ex, LineBuffer^[index].ey);
			index := index + 1;
		end;

		hasChanged := FALSE;
	end;

procedure TSObject3D.Erase;
	begin
		self.CalcBounds;
		EraseRect(oldBounds);
	end;

end.