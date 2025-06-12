unit Dialogs;

interface
	uses
		Sound, Globals;

	var
		soundWndo, speedWndo, physicsWndo: WindowPtr;

	procedure DoSound;
	procedure DoSpeed;
	procedure DoHelpPlay;
	procedure DoHelpRules;
	procedure DoPhysics;

implementation

{=================================}

	var
		upRect, dnRect: Rect;

{=================================}

	procedure DoSound;
		const
			okayButton = 1;
			cancelButton = 2;
			soundOnRadio = 3;
			soundOffRadio = 4;
			soundCheck1 = 5;
			soundCheck2 = 6;
			soundCheck3 = 7;
			soundCheck4 = 8;
			soundCheck5 = 9;
			userItem = 11;
			userItem2 = 12;

		var
			theDlgPtr: DialogPtr;
			tempRect: Rect;
			itemT, itemHit, i: Integer;
			itemH: Handle;
			tempStr: Str255;
			tempBools: array[1..5] of Boolean;
			exitDialog, tempSoundOn: Boolean;

{----------------}

		procedure DrawDefaultAndFrame;
		begin
			ForeColor(redColor);
			GetDItem(theDlgPtr, userItem, itemT, itemH, tempRect);
			FrameRect(tempRect);
			GetDItem(theDlgPtr, userItem2, itemT, itemH, tempRect);
			FrameRect(tempRect);
			ForeColor(blackColor);
			GetDItem(theDlgPtr, okayButton, itemT, itemH, tempRect);
			PenSize(3, 3);
			InsetRect(tempRect, -4, -4);
			FrameRoundRect(tempRect, 16, 16);
			PenNormal;
		end;

{----------------}

		procedure UpdateChecks;
			var
				i: Integer;
		begin
			for i := 1 to 5 do
				if (tempBools[i]) then
					begin
						GetDItem(theDlgPtr, i + soundCheck1 - 1, itemT, itemH, tempRect);
						SetCtlValue(ControlHandle(itemH), 1);
					end
				else
					begin
						GetDItem(theDlgPtr, i + soundCheck1 - 1, itemT, itemH, tempRect);
						SetCtlValue(ControlHandle(itemH), 0);
					end;
		end;

{----------------}

		procedure UpdateRadios;
			var
				i: Integer;
		begin
			if (tempSoundOn) then
				begin
					GetDItem(theDlgPtr, soundOnRadio, itemT, itemH, tempRect);
					SetCtlValue(ControlHandle(itemH), 1);
					GetDItem(theDlgPtr, soundOffRadio, itemT, itemH, tempRect);
					SetCtlValue(ControlHandle(itemH), 0);

					for i := 1 to 5 do
						begin
							GetDItem(theDlgPtr, i + soundCheck1 - 1, itemT, itemH, tempRect);
							if (tempBools[i]) then
								SetCtlValue(ControlHandle(itemH), 1)
							else
								SetCtlValue(ControlHandle(itemH), 0);

							HiliteControl(ControlHandle(itemH), 0);
						end;
				end
			else
				begin
					GetDItem(theDlgPtr, soundOffRadio, itemT, itemH, tempRect);
					SetCtlValue(ControlHandle(itemH), 1);
					GetDItem(theDlgPtr, soundOnRadio, itemT, itemH, tempRect);
					SetCtlValue(ControlHandle(itemH), 0);

					for i := 1 to 5 do
						begin
							GetDItem(theDlgPtr, i + soundCheck1 - 1, itemT, itemH, tempRect);
							SetCtlValue(ControlHandle(itemH), 0);
							HiliteControl(ControlHandle(itemH), 255);
						end;
				end;
		end;

{----------------}

	begin
		CenterDialog(soundDialID);
		theDlgPtr := GetNewDialog(soundDialID, nil, Pointer(-1));
		ShowWindow(theDlgPtr);
		SetPort(GrafPtr(theDlgPtr));

		for i := 1 to 5 do
			begin
				tempBools[i] := soundArray[i];
				if (tempBools[i]) then
					begin
						GetDItem(theDlgPtr, i + soundCheck1 - 1, itemT, itemH, tempRect);
						SetCtlValue(ControlHandle(itemH), 1);
					end
				else
					begin
						GetDItem(theDlgPtr, i + soundCheck1 - 1, itemT, itemH, tempRect);
						SetCtlValue(ControlHandle(itemH), 0);
					end;
			end;
		tempSoundOn := soundOn;

		UpdateChecks;
		UpdateRadios;

		exitDialog := FALSE;
		DrawDefaultAndFrame;

		repeat
			ModalDialog(nil, itemHit);

			if (ItemHit = okayButton) then
				begin
					exitDialog := TRUE;
					soundOn := tempSoundOn;
					for i := 1 to 5 do
						soundArray[i] := tempBools[i];
				end;

			if (ItemHit = cancelButton) then
				exitDialog := TRUE;

			if (itemHit = soundOnRadio) then
				begin
					tempSoundOn := TRUE;
					UpdateRadios;
				end;

			if (itemHit = soundOffRadio) then
				begin
					tempSoundOn := FALSE;
					UpdateRadios;
				end;

			if ((itemHit >= soundCheck1) and (itemHit <= soundCheck5)) then
				begin
					tempBools[itemHit - soundCheck1 + 1] := not tempBools[itemHit - soundCheck1 + 1];
					GetDItem(theDlgPtr, itemHit, itemT, itemH, tempRect);
					if (tempBools[itemHit - soundCheck1 + 1]) then
						SetCtlValue(ControlHandle(itemH), 1)
					else
						SetCtlValue(ControlHandle(itemH), 0);
				end;

		until exitDialog;

		DisposDialog(theDlgPtr);
	end;

{=================================}

	procedure DoSpeed;
		const
			okayButton = 1;
			cancelButton = 2;
			fastRadio = 3;
			slowRadio = 7;
			userItem = 9;

		var
			theDlgPtr: DialogPtr;
			tempRect: Rect;
			itemT, itemHit, tempDelay, i: Integer;
			itemH: Handle;
			tempStr: Str255;
			exitDialog: Boolean;

{----------------}

		procedure DrawDefaultAndFrame;
		begin
			ForeColor(redColor);
			GetDItem(theDlgPtr, userItem, itemT, itemH, tempRect);
			FrameRect(tempRect);
			ForeColor(blackColor);
			GetDItem(theDlgPtr, okayButton, itemT, itemH, tempRect);
			PenSize(3, 3);
			InsetRect(tempRect, -4, -4);
			FrameRoundRect(tempRect, 16, 16);
			PenNormal;
		end;

{----------------}

	begin
		CenterDialog(speedDialID);
		theDlgPtr := GetNewDialog(speedDialID, nil, Pointer(-1));
		ShowWindow(theDlgPtr);
		SetPort(GrafPtr(theDlgPtr));

		tempDelay := delayTime;
		GetDItem(theDlgPtr, tempDelay + fastRadio, itemT, itemH, tempRect);
		SetCtlValue(ControlHandle(itemH), 1);

		exitDialog := FALSE;
		DrawDefaultAndFrame;

		repeat
			ModalDialog(nil, itemHit);

			if (ItemHit = okayButton) then
				begin
					exitDialog := TRUE;
					delayTime := tempDelay;
				end;

			if (ItemHit = cancelButton) then
				exitDialog := TRUE;

			if ((itemHit >= fastRadio) or (itemHit <= slowRadio)) then
				begin
					for i := fastRadio to slowRadio do
						begin
							GetDItem(theDlgPtr, i, itemT, itemH, tempRect);
							SetCtlValue(ControlHandle(itemH), 0);
						end;

					GetDItem(theDlgPtr, itemHit, itemT, itemH, tempRect);
					SetCtlValue(ControlHandle(itemH), 1);

					tempDelay := itemHit - fastRadio;
				end;

		until exitDialog;

		DisposDialog(theDlgPtr);
	end;

{=================================}

	procedure DoHelpPlay;
		const
			okayButton = 1;

		var
			theDlgPtr: DialogPtr;
			tempRect: Rect;
			itemT, itemHit: Integer;
			itemH: Handle;

{----------------}

		procedure DrawDefault;
		begin
			GetDItem(theDlgPtr, okayButton, itemT, itemH, tempRect);
			PenSize(3, 3);
			InsetRect(tempRect, -4, -4);
			FrameRoundRect(tempRect, 16, 16);
			PenSize(1, 1);
		end;

{----------------}

	begin
		CenterDialog(helpPlayDialID);
		theDlgPtr := GetNewDialog(helpPlayDialID, nil, Pointer(-1));
		ShowWindow(theDlgPtr);
		SetPort(GrafPtr(theDlgPtr));
		DrawDefault;

		repeat
			ModalDialog(nil, itemHit);
		until (ItemHit = okayButton);

		DisposDialog(theDlgPtr);
	end;

{=================================}

	procedure DoHelpRules;
		const
			okayButton = 1;

		var
			theDlgPtr: DialogPtr;
			tempRect: Rect;
			itemT, itemHit: Integer;
			itemH: Handle;

{----------------}

		procedure DrawDefault;
		begin
			GetDItem(theDlgPtr, okayButton, itemT, itemH, tempRect);
			PenSize(3, 3);
			InsetRect(tempRect, -4, -4);
			FrameRoundRect(tempRect, 16, 16);
			PenSize(1, 1);
		end;

{----------------}

	begin
		CenterDialog(helpRulesDialID);
		theDlgPtr := GetNewDialog(helpRulesDialID, nil, Pointer(-1));
		ShowWindow(theDlgPtr);
		SetPort(GrafPtr(theDlgPtr));
		DrawDefault;

		repeat
			ModalDialog(nil, itemHit);
		until (ItemHit = okayButton);

		DisposDialog(theDlgPtr);
	end;

{=================================}

	function idleFilter (theDialog: DialogPtr; var theEvent: EventRecord; var itemHit: integer): boolean;
		var
			mousePt: Point;
	begin
		idleFilter := FALSE;
		if ((theEvent.what = KeyDown) and (BitAnd(theEvent.message, CharCodeMask) = 13)) then
			begin
				itemHit := 1;
				idleFilter := TRUE;
			end;

		if (StillDown) then
			begin
				GetMouse(mousePt);
				if (PtInRect(mousePt, upRect)) then
					begin
						itemHit := 8;
						idleFilter := TRUE;
					end;
				if (PtInRect(mousePt, dnRect)) then
					begin
						itemHit := 9;
						idleFilter := TRUE;
					end;
			end;
	end;

{=================================}

	procedure DoPhysics;
		const
			okayButton = 1;
			cancelButton = 2;
			defaultButton = 3;
			noFricRadio = 4;
			strongFricRadio = 7;
			upGravIcon = 8;
			downGravIcon = 9;
			gravStat = 10;
			gravIcon = 11;
			userItem = 13;
			userItem2 = 14;
			userItem3 = 15;

		var
			theDlgPtr: DialogPtr;
			tempRect: Rect;
			itemT, itemHit, tempGrav, tempFrict: Integer;
			itemH: Handle;
			exitDialog: Boolean;

{----------------}

		procedure DrawDefaultAndFrame;
		begin
			ForeColor(redColor);
			GetDItem(theDlgPtr, userItem, itemT, itemH, tempRect);
			FrameRect(tempRect);
			GetDItem(theDlgPtr, userItem2, itemT, itemH, tempRect);
			FrameRect(tempRect);
			GetDItem(theDlgPtr, userItem3, itemT, itemH, tempRect);
			FrameRect(tempRect);
			ForeColor(blackColor);
			GetDItem(theDlgPtr, okayButton, itemT, itemH, tempRect);
			PenSize(3, 3);
			InsetRect(tempRect, -4, -4);
			FrameRoundRect(tempRect, 16, 16);
			PenNormal;
		end;

{----------------}

		procedure UpdateGrav;
			var
				whichIcon: Integer;
				tempStr: Str255;
				icnHand: Handle;
				cicnHand: CIconHandle;
		begin
			NumToString(tempGrav, tempStr);
			GetDItem(theDlgPtr, gravStat, itemT, itemH, tempRect);
			SetIText(itemH, tempStr);

			if (tempGrav = 0) then
				whichIcon := 138
			else
				whichIcon := ((tempGrav - 1) div 20) + 139;

			GetDItem(theDlgPtr, gravIcon, itemT, itemH, tempRect);
			if (inColor) then
				begin
					cicnHand := GetCIcon(whichIcon);
					if (cicnHand <> nil) then
						begin
							PlotCIcon(tempRect, cicnHand);
							DisposCIcon(cicnHand);
						end;
				end
			else
				begin
					icnHand := GetIcon(whichIcon);
					if (icnHand <> nil) then
						PlotIcon(tempRect, icnHand);
				end;
		end;

{----------------}

		procedure UpdateRadios;
			var
				i: Integer;
		begin
			for i := noFricRadio to strongFricRadio do
				begin
					GetDItem(theDlgPtr, i, itemT, itemH, tempRect);
					SetCtlValue(ControlHandle(itemH), 0);
				end;
			GetDItem(theDlgPtr, tempFrict + noFricRadio, itemT, itemH, tempRect);
			SetCtlValue(ControlHandle(itemH), 1);
		end;

{----------------}

	begin
		CenterDialog(physicsDialID);
		theDlgPtr := GetNewDialog(physicsDialID, nil, Pointer(-1));
		ShowWindow(theDlgPtr);
		SetPort(GrafPtr(theDlgPtr));

		tempFrict := frictState;
		tempGrav := mouseConst - 50;

		GetDItem(theDlgPtr, upGravIcon, itemT, itemH, upRect);
		GetDItem(theDlgPtr, downGravIcon, itemT, itemH, dnRect);

		UpdateRadios;
		UpdateGrav;

		exitDialog := FALSE;
		DrawDefaultAndFrame;

		repeat
			ModalDialog(@idleFilter, itemHit);

			if (itemHit = okayButton) then
				begin
					mouseConst := tempGrav + 50;
					if (mouseConst > 150) then
						mouseConst := 150;
					if (mouseConst < 50) then
						mouseConst := 50;

					frictState := tempFrict;
					if (frictState > 3) then
						frictState := 3;
					if (frictState < 0) then
						frictState := 0;

					exitDialog := TRUE;
				end;

			if (itemHit = cancelButton) then
				exitDialog := TRUE;

			if (itemHit = defaultButton) then
				begin
					tempFrict := normalFriction;
					tempGrav := 50;
					UpdateRadios;
					UpdateGrav;
				end;

			if ((itemHit >= noFricRadio) and (itemHit <= strongFricRadio)) then
				begin
					tempFrict := itemHit - noFricRadio;
					UpdateRadios;
				end;

			if ((itemHit = upGravIcon) and (tempGrav < 100)) then
				begin
					GetDItem(theDlgPtr, upGravIcon, itemT, itemH, tempRect);
					InvertRect(tempRect);
					tempGrav := tempGrav + 1;
					UpdateGrav;
					GetDItem(theDlgPtr, upGravIcon, itemT, itemH, tempRect);
					InvertRect(tempRect);
				end;

			if ((itemHit = downGravIcon) and (tempGrav > 0)) then
				begin
					GetDItem(theDlgPtr, downGravIcon, itemT, itemH, tempRect);
					InvertRect(tempRect);
					tempGrav := tempGrav - 1;
					UpdateGrav;
					GetDItem(theDlgPtr, downGravIcon, itemT, itemH, tempRect);
					InvertRect(tempRect);
				end;

		until exitDialog;

		DisposDialog(theDlgPtr);
	end;

{=================================}

end.