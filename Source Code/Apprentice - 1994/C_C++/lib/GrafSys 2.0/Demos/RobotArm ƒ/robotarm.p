program RobotArm;

{demonstrates inheritance of transformation on a 2D robot hand }

	uses
		Matrix, Transformations, GrafSysCore, GrafSysScreen, GrafSysObject, Resources;

	const
		degrees = 0.01745329; (* ¹/180 *)
		cTheWindow = 400;


	type
		robot = object
				Arm: TSObject3D;
				Hand: TSObject3D;
				joint: TMatrixList; (* used to translate hand to end of arm *)
				jointRect: Rect;
				jointRadius: integer;
				procedure Init;
				procedure draw;
			end;

	var
		theRobot: robot;
		EyeLoc: Vector4;
		theWindow: WindowPtr;


	procedure BuildArm (Obj: TSObject3D);

		var
			OK: longint;

	begin
		OK := Obj.AddPoint(0, 15, 0);
		OK := Obj.AddPoint(70, 15, 0);
		OK := Obj.AddPoint(70, -15, 0);
		OK := Obj.AddPoint(0, -15, 0);
		OK := Obj.AddLine(1, 2);
		OK := Obj.AddLine(2, 3);
		OK := Obj.AddLine(3, 4);
		OK := Obj.AddLine(4, 1);

		OK := Obj.AddPoint(85, 0, 0); (* joint is circle with this center  *)
		OK := Obj.AddPoint(70, 0, 0); (* joint radius calculated with this *)

	end;

	procedure BuildHand (Obj: TSObject3D);
		var
			OK: longint;

	begin
		OK := Obj.AddPoint(15, 15, 0);
		OK := Obj.AddPoint(45, 15, 0);
		OK := Obj.AddPoint(35, 5, 0);
		OK := Obj.AddPoint(25, 5, 0);
		OK := Obj.AddPoint(25, -5, 0);
		OK := Obj.AddPoint(35, -5, 0);
		OK := Obj.AddPoint(45, -15, 0);
		OK := Obj.AddPoint(15, -15, 0);
		OK := Obj.AddLine(1, 2);
		OK := Obj.AddLine(2, 3);
		OK := Obj.AddLine(3, 4);
		OK := Obj.AddLine(4, 5);
		OK := Obj.AddLine(5, 6);
		OK := Obj.AddLine(6, 7);
		OK := Obj.AddLine(7, 8);
		OK := Obj.AddLine(8, 1);
	end;

	procedure robot.Init;
		var
			tmp: TMatrixPass;
			h1, v1, h, v: integer;

	begin
		New(Arm);
		Arm.Init;
		BuildArm(Arm);
		Arm.SetUseBounds(TRUE);
		New(Hand);
		Hand.Init;
		BuildHand(Hand);
		Hand.SetUseBounds(TRUE);
		joint := Hand.FFNewPostConcat;
		tmp := Arm.FFPassOn;
		Hand.FFInherit(tmp);
		joint.TMTranslate(85, 0, 0);
		ProjectPoint(Arm.transformedPoint(5), h, v);
		ProjectPoint(Arm.transformedPoint(6), h1, v1);
		jointRadius := abs(h1 - h);
	end;

	procedure robot.draw;

		var
			thePoint: Vector4;
			dummy, z: real;

	begin
(* erase old image *)
		EraseRect(Arm.Bounds);
		EraseRect(Hand.Bounds);
		EraseRect(jointRect);
		Arm.fDraw;
		Hand.fDraw; (* now draw the joint *)
		thePoint := Arm.transformedPoint(5);
		GetVector4(thePoint, dummy, dummy, z);
		if z >= 0 then
			begin
				ProjectPoint(thePoint, jointRect.right, jointRect.bottom);
				jointRect.left := jointRect.right;
				jointRect.top := jointRect.bottom;
				InsetRect(jointRect, -jointRadius, -jointRadius);
				FrameOval(jointRect);
			end;
	end;

	const
		leftArrow = $7B;
		rightArrow = $7C;
		upArrow = $7E;
		downArrow = $7D;

	var
		dummyLong: longint;
		dummyBool: Boolean;
		theEvent: EventRecord;
		theKeys: KeyMap;
		update: Boolean;

begin
	InitGrafSys;
	theWindow := GetNew3DWindow(cTheWindow, pointer(-1));
	SetVector4(EyeLoc, 0, 0, -1);
	SetEye(TRUE, EyeLoc, 0, 0, 0, 90 * degrees, arithmetic);
	New(theRobot);
	theRobot.Init;
	theRobot.Draw;
	repeat
		GetKeys(theKeys);
		if theKeys[leftArrow] then (* rotate hand counter-clockwise *)
			begin
				theRobot.Hand.Rotate(0, 0, 2.5 * degrees);
				update := TRUE;
			end;

		if theKeys[rightArrow] then (* rotate hand clockwise *)
			begin
				theRobot.Hand.Rotate(0, 0, -2.5 * degrees);
				update := true;
			end;

		if theKeys[upArrow] then (* rotate arm counter-clockwise *)
			begin
				theRobot.Arm.Rotate(0, 1 * degrees, 2.5 * degrees);
				update := true;
			end;

		if theKeys[downArrow] then
			begin
				theRobot.Arm.Rotate(0, 0, -2.5 * degrees);
				update := true;
			end;

		if update then
			theRobot.Draw;
		update := False;

		dummyBool := GetNextEvent(everyevent, theEvent);
		SystemTask;
	until button;
end.