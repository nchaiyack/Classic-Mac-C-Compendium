program Orbits;

{ demo program for grafsys, © 1993 by C. Franz }
{}
{ this demo demonstrates inheritance and arbitrary rotation }

{this demo program shows three spheres that orbit each other. They are }
{essentially three objects. Sun, Planet and moon are one object each       }

	uses
		Matrix, Transformations, GrafSysCore, GrafSysScreen, GrafSysObject, Resources;

	const
		degrees = 0.01745329; (* ¹/180 *)
		cTheWindow = 400;

	type
		SolarSystem = object
				sun: TSGenericObject3D;
				planet: TSGenericObject3D;
				moon: TSGenericObject3D;
				SunRad: real;
				PlanetRad: real;
				MoonRad: Real;
				frame: rect;
				procedure Init;
				procedure Draw;
				procedure Animate;
			end;

	var
		Sol: SolarSystem;

{CalcObjectFrame : calculates the rect that will contain the sphere 		}
{						uses the same perspective projection that usually is	}
{						used by grafsys												}
{ 	v 			contains the (transformed) wc of the sphere's center			}
{ 	radius 	is the size of the shpere 											}
{ 	theRect	returns the rect used for drawing the sphere 					}

	procedure CalcObjectFrame (v: vector4; radius: real; var theRect: Rect; var z: real);

		var
			zbyd: real;
			x, y: real;
			sx, sy, sr: integer;

	begin
		GetVector4(v, x, y, z); (* get the sphere's z *)
		ProjectPoint(v, sx, sy);(* use this as center for the rect *)
		SetRect(theRect, sx, sy, sx, sy);

		zbyd := 1 / (z / current3DPort^.d + 1);
		sr := Trunc(radius * zbyd); (* project the radius *)
		InsetRect(theRect, -sr, -sr);
	end;

{ Methods }
	procedure SolarSystem.Init;
		var
			thePass: TMatrixPass;
			OK: LongInt;

	begin
		New(Sun);
		Sun.Init;
		OK := Sun.AddPoint(0, 0, 0);
		New(Planet);
		Planet.Init;
		OK := Planet.AddPoint(0, 0, 0);
		New(Moon);
		Moon.Init;
		OK := Moon.AddPoint(0, 0, 0);
		SetRect(frame, 32000, 32000, -32000, -32000); (* almost-minimum empty rect *)

	(* Now set up the radius *)
		SunRad := 50;
		PlanetRad := 20;
		MoonRad := 10;

	(* now lets move the planets and moons where they belong *)
		Planet.Translate(100, 0, 0);
		Moon.Translate(30, 0, 0);

    (* Now lets make them inheritances *)
		thePass := Sun.FFPassOn;
		Planet.FFInherit(thePass);
		thePass := Planet.FFPassOn;
		Moon.FFInherit(thePass);
	end;

	procedure SolarSystem.Draw;
		var
			sunR: rect;
			planetR: Rect;
			moonR: Rect;
			sunZ, planetZ, moonZ: Real;
			theVector: Vector4;
			priority: array[1..3] of record
					theRect: rect;
					deepZ: real
				end;

	begin
(* Step 1: transform the object and all the points in it *)
		sun.transform(FALSE);
		planet.transform(FALSE); (* not the order! *)
		moon.transform(TRUE);
(* Step 2 : access the data base, get the transformed point and *)
(*              calculate the frame for the solar object. We use a little trick: *)
(*              we always access the first point (buffer 0, offset 0 ) so we   *)
(*              don't need to calculate an offset                                             *)
		theVector := sun.theBufs[0]^[0].transformed;
		CalcObjectFrame(theVector, sunRad, sunR, sunZ);
		theVector := Planet.theBufs[0]^[0].transformed;
		CalcObjectFrame(theVector, planetRad, planetR, planetZ);
		theVector := Moon.theBufs[0]^[0].transformed;
		CalcObjectFrame(theVector, moonRad, moonR, moonZ);

(* step 3: sort the rects in their z-priority *)
		priority[1].theRect := sunR;
		priority[1].deepZ := sunZ;

		if planetZ > Priority[1].deepZ then
			begin
				priority[2] := priority[1];
				priority[1].theRect := planetR;
				priority[1].deepZ := planetZ;
			end
		else
			begin
				priority[2].theRect := planetR;
				priority[2].deepZ := planetZ
			end;

		if moonZ > Priority[1].deepZ then
			begin
				priority[3] := priority[2];
				priority[2] := priority[1];
				priority[1].theRect := moonR;
				priority[1].deepZ := moonZ;
			end
		else if moonZ > Priority[2].deepZ then
			begin
				priority[3] := priority[2];
				priority[2].theRect := moonR;
				priority[2].deepZ := moonZ;
			end
		else
			begin
				priority[3].theRect := moonR;
				priority[3].deepZ := moonZ
			end;

(* Step 4 : Erase frame and redraw *)
		EraseRect(frame);
		FrameOval(Priority[1].theRect);
		EraseOval(Priority[2].theRect);
		FrameOval(Priority[2].theRect);
		EraseOval(Priority[3].theRect);
		FrameOval(Priority[3].theRect);

(* Step 5: calc new frame *)
		UnionRect(sunR, planetR, frame);
		UnionRect(frame, moonR, frame);
	end;

	procedure SolarSystem.Animate;
	begin
		Sun.Rotate(0 * degrees, 5 * degrees, 2 * degrees);
		Planet.Rotate(0 * degrees, 5 * degrees, 0 * degrees);
	end;

	var
		EyeLoc: Vector4;
		theWindow: WindowPtr;
		dummyL: LongInt;

begin
	InitGrafSys;
	theWindow := GetNew3DWindow(cTheWindow, pointer(-1));
	SetVector4(EyeLoc, 0, 0, -1);
	SetEye(TRUE, EyeLoc, 0, 0, 0, 90 * degrees, none);
	New(Sol);
	Sol.Init;
	Sol.Draw;
	repeat
		Sol.Animate;
		Sol.Draw;
		Delay(10, dummyL);
	until button;
end.