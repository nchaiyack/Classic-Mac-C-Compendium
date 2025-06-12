unit MainWndo;

interface

	uses
		Sound, GameUtils, Enemies, GlyphaGuts;

	procedure Init_MainWndo;
	procedure Close_MainWndo;
	procedure Open_MainWndo (rightOff, downOff: Integer);
	procedure Update_MainWndo (whichWindow: WindowPtr);

implementation

	var
		tempRect: Rect;                      {Temporary rectangle}
		Index: Integer;                      {For looping}
		CtrlHandle: controlhandle;           {Control handle}
		sTemp: Str255;                       {Get text entered, temp holding}

{=================================}

	procedure Init_MainWndo;

	begin                                   {Start of Window initialize routine}
		mainWndo := nil;                        {Make sure other routines know we are not valid yet}
	end;                                    {End of procedure}

{=================================}

	procedure Close_MainWndo;

	begin
		if (mainWndo <> nil) then
			begin
				DisposeWindow(mainWndo);	{Clear window and controls}
				mainWndo := nil;			{Make sure other routines know we are open}
			end;					{End for if (mainWndo<>nil)}
	end;

{=================================}

	procedure UpDate_MainWndo;
		var
			leftEdge, topEdge, rightEdge, bottomEdge: Integer;
			tempRect: Rect;
	begin
		if (mainWndo <> nil) and (mainWndo = whichWindow) then {Handle an open when already opened}
			begin
				SetPort(mainWndo);
				PenNormal;
				leftEdge := mainWndo^.portBits.bounds.left;
				topEdge := mainWndo^.portBits.bounds.top;
				rightEdge := mainWndo^.portBits.bounds.right;
				bottomEdge := mainWndo^.portBits.bounds.bottom;
				SetRect(tempRect, leftEdge, topEdge, 0, bottomEdge);
				FillRect(tempRect, black);
				SetRect(tempRect, 512, topEdge, rightEdge, bottomEdge);
				FillRect(tempRect, black);
				SetRect(tempRect, 0, topEdge, 512, 0);
				FillRect(tempRect, black);
				SetRect(tempRect, 0, 342, 512, bottomEdge);
				FillRect(tempRect, black);
				SetRect(tempRect, 0, 0, 512, 342);
				CopyBits(offVirginMap, mainWndo^.portBits, tempRect, tempRect, srcCopy, nil);
				PenMode(patXOr);
				InsetRect(tempRect, -1, -1);
				FrameRect(tempRect);
				PenNormal;
			end;							{End for if (mainWndo<>nil)}
	end;

{=================================}

	procedure Open_MainWndo;
		var
			tempRect: Rect;
	begin
		if (mainWndo = nil) then		{Handle an open when already opened}
			begin
				mainWndo := GetNewWindow(1, nil, Pointer(-1)); {Get window from resource}
				SelectWindow(mainWndo);	{Bring our window to the front}
				SetPort(mainWndo);		{Prepare to write into our window}
				SizeWindow(mainWndo, screenBits.bounds.right, screenBits.bounds.bottom, FALSE);
				SetOrigin(-rightOff, -downOff);
				ShowWindow(mainWndo);
				UpDate_MainWndo(mainWndo);	{Do an update to draw rest of items}
			end						{End for if (mainWndo<>nil)}
		else
			SelectWindow(mainWndo);		{Already open, so show it}
	end;

{=================================}

end.                                    {End of unit}