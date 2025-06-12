unit AboutIt;

interface

	procedure D_MoreAboutGlider;
	procedure Init_AboutIt;
	procedure Close_AboutIt;
	procedure Open_AboutIt;
	procedure Update_AboutIt (whichWindow: WindowPtr);
	procedure Do_AboutIt (myEvent: EventRecord; var theInput: TEHandle);

implementation

	const
		I_Help = 7;	{Button ID}
		I_More = 8;	{Button ID}
		I_Okay = 1;
		I_Static_Text = 2;
		I_Static_Text3 = 3;
		I_Static_Text5 = 4;
		I_Static_Text7 = 5;
		I_Static_Text9 = 6;
		I_Static_Text11 = 7;
		I_Static_Text13 = 8;
		I_Static_Text15 = 9;
		I_Static_Text17 = 10;
		I_Static_Text19 = 11;
		I_Static_Text21 = 12;
		I_Static_Text23 = 13;
		I_Static_Text25 = 14;
		I_Static_Text27 = 15;
		I_Static_Text29 = 16;
		I_Static_Text31 = 17;
		I_Static_Text33 = 18;
		I_Static_Text35 = 19;
		I_HelpPicture = 60;
		I_Rectanglex43 = 26;

	var
		ExitDialog: boolean;				{Flag used to exit the Dialog		}
		MyWindow: WindowPtr;			{Window pointer					}
		tempRect: Rect;					{Temporary rectangle			}
		Index: Integer;					{For looping						}
		CtrlHandle: controlhandle;		{Control handle					}
		sTemp: Str255;					{Get text entered, temp holding	}

{=================================}

	function MyFilter (theDialog: DialogPtr; var theEvent: EventRecord; var itemHit: integer): boolean;
		var
			DoubleClick: boolean;                {Flag to say that a double click on a list happened}
			MyPt: Point;                         {Current list selection point}
			tempRect: Rect;

	begin
		MyFilter := FALSE;
		if (theEvent.what = MouseDown) then   {Only do on a mouse click}
			begin
				MyPt := theEvent.where;             {Get the point where the mouse was clicked}
				with theDialog^.portBits.bounds do  {Do this way for speed}
					begin
						myPt.h := myPt.h + left;
						myPt.v := myPt.v + top;
					end;
			end;
	end;

{===========================================================}

	procedure D_Help;
		var
			GetSelection: DialogPtr;	{Name of dialog}
			tempRect: Rect;			{Temporary tectangle}
			DType: Integer;				{Type of dialog item}
			Index: Integer;				{For looping}
			DItem: Handle;				{Handle to the dialog item}
			CItem, CTempItem: controlhandle;	{Control handle}
			sTemp: Str255;			{Get text entered, temp holding}
			itemHit: Integer;			{Get selection}
			temp: Integer;				{Get selection, temp holding}
			ThisEditText: TEHandle;	{Handle to get the Dialogs TE record}
			TheDialogPtr: DialogPeek;{Pointer to Dialogs definition record, contains the TE record}

		procedure Refresh_Dialog;
			var
				rTempRect: Rect;
		begin
			PenNormal;
			GetDItem(GetSelection, I_Okay, DType, DItem, tempRect);{Get the item handle}
			PenSize(3, 3);  				{Change pen to draw thick default outline}
			InsetRect(tempRect, -4, -4);		{Draw outside the button by 1 pixel}
			FrameRoundRect(tempRect, 16, 16); {Draw the outline}
			PenSize(1, 1);
		end;

	begin
		GetSelection := GetNewDialog(4, nil, Pointer(-1)); {Bring in the dialog resource}
		tempRect := GetSelection^.portRect;	{Get window size, we will now center it}
		tempRect.Top := ((screenBits.Bounds.Bottom - screenBits.Bounds.Top) - (tempRect.Bottom - tempRect.Top)) div 2;
		tempRect.Left := ((screenBits.Bounds.Right - screenBits.Bounds.Left) - (tempRect.Right - tempRect.Left)) div 2;
		MoveWindow(GetSelection, tempRect.Left, tempRect.Top, TRUE);{Now move the window to the proper position}
		ShowWindow(GetSelection);             {Open a dialog box}
		SelectWindow(GetSelection);           {Lets see it}
		SetPort(GetSelection);                {Perpare to add conditional text}
		Refresh_Dialog;			{Draw any Lists, lines, or rectangles}
		ExitDialog := FALSE;	{Do not exit dialog handle loop yet}
		repeat
			ModalDialog(nil, itemHit);		{Wait until an item is hit}
			GetDItem(GetSelection, itemHit, DType, DItem, tempRect); {Get item information}
			CItem := Pointer(DItem);			{Get the control handle}
			if (ItemHit = I_Okay) then		{Handle the Button being pressed}
				begin
					ExitDialog := TRUE;				{Exit the dialog when this selection is made}
					Refresh_Dialog;
				end;								{End for this item selected}
		until ExitDialog;						{Handle dialog items until exit selected}
		DisposDialog(GetSelection);		{Flush the dialog out of memory}
	end;

{=================================}

	procedure D_MoreAboutGlider;
		var
			GetSelection: DialogPtr;             {Name of dialog}
			tempRect: Rect;                      {Temporary tectangle}
			DType: Integer;                      {Type of dialog item}
			Index: Integer;                      {For looping}
			DItem: Handle;                       {Handle to the dialog item}
			CItem, CTempItem: controlhandle;     {Control handle}
			sTemp: Str255;                       {Get text entered, temp holding}
			itemHit: Integer;                    {Get selection}
			temp: Integer;                       {Get selection, temp holding}
			ThisEditText: TEHandle;              {Handle to get the Dialogs TE record}
			TheDialogPtr: DialogPeek;            {Pointer to Dialogs definition record, contains the TE record}

		procedure Refresh_Dialog;               {Refresh the dialogs non-controls}
			var
				rTempRect: Rect;                       {Temp rectangle used for drawing}
		begin
			PenNormal;
			GetDItem(GetSelection, I_Okay, DType, DItem, tempRect);{Get the item handle}
			PenSize(3, 3);  				{Change pen to draw thick default outline}
			InsetRect(tempRect, -4, -4);		{Draw outside the button by 1 pixel}
			FrameRoundRect(tempRect, 16, 16); {Draw the outline}
			PenSize(1, 1);
		end;

	begin                                   {Start of dialog handler}
		GetSelection := GetNewDialog(3, nil, Pointer(-1)); {Bring in the dialog resource}
		tempRect := GetSelection^.portRect;	{Get window size, we will now center it}
		tempRect.Top := ((screenBits.Bounds.Bottom - screenBits.Bounds.Top) - (tempRect.Bottom - tempRect.Top)) div 2;
		tempRect.Left := ((screenBits.Bounds.Right - screenBits.Bounds.Left) - (tempRect.Right - tempRect.Left)) div 2;
		MoveWindow(GetSelection, tempRect.Left, tempRect.Top, TRUE);{Now move the window to the proper position}
		ShowWindow(GetSelection);             {Open a dialog box}
		SelectWindow(GetSelection);           {Lets see it}
		SetPort(GetSelection);                {Perpare to add conditional text}

		Refresh_Dialog;
		ExitDialog := FALSE;                    {Do not exit dialog handle loop yet}
		repeat                                {Start of dialog handle loop}
			ModalDialog(nil, itemHit);          {Wait until an item is hit}
			GetDItem(GetSelection, itemHit, DType, DItem, tempRect); {Get item information}
			CItem := Pointer(DItem);            {Get the control handle}
			if (ItemHit = I_Okay) then           {Handle the Button being pressed}
				begin
					ExitDialog := TRUE;                 {Exit the dialog when this selection is made}
				end;                                {End for this item selected}
		until ExitDialog;                     {Handle dialog items until exit selected}
		DisposDialog(GetSelection);           {Flush the dialog out of memory}
	end;

{=================================}

	procedure Init_AboutIt;
	begin
		MyWindow := nil;				{Make sure other routines know we are not valid yet}
	end;

{=================================}

	procedure Close_AboutIt;
	begin
		DisposeWindow(MyWindow);	{Clear window and controls}
		MyWindow := nil;				{Make sure other routines know we are open}
	end;

{=================================}

	procedure UpDate_AboutIt;
		var
			SavePort: WindowPtr;	{Place to save the last port}
			Icon_Handle: Handle;		{Icon handle for icon loaded and displayed}
			sTemp: Str255;			{Temporary string}
	begin
		if (MyWindow <> nil) and (MyWindow = whichWindow) then {Handle an open when already opened}
			begin
				GetPort(SavePort);							{Save the current port}
				SetPort(MyWindow);							{Set the port to my window}
				Icon_Handle := GetIcon(11);				{Get Icon into memory}
				SetRect(tempRect, 331, 10, 363, 42);	{left,top,right,bottom}
				if (Icon_Handle <> nil) then				{Only use handle if it is valid}
					PlotIcon(tempRect, Icon_Handle);		{Draw the icon in the window}
				TextFont(systemFont);						{Set the font to draw in}
				SetRect(tempRect, 11, 15, 90, 30);
				sTemp := 'Glider+ 3.14';
				TextBox(Pointer(ord(@sTemp) + 1), length(sTemp), tempRect, teJustLeft);
				SetRect(tempRect, 11, 50, 362, 66);
				sTemp := '¨1989, 90, 91 john calhoun - Soft Dorothy Software';
				TextBox(Pointer(ord(@sTemp) + 1), length(sTemp), tempRect, teJustLeft);
				DrawControls(MyWindow);								{Draw all the controls				}
				SetPort(SavePort);										{Restore the old port				}
			end;															{End for if (MyWindow<>nil)		}
	end;

{=================================}

	procedure Open_AboutIt;
	begin
		if (MyWindow = nil) then									{Handle an open when already opened}
			begin
				MyWindow := GetNewWindow(2, nil, Pointer(-1));	{Get the window from the resource file}
				MoveWindow(MyWindow, (screenBits.bounds.right - 384) div 2, (screenBits.bounds.bottom - 200) div 2 + 20, FALSE);
				ShowWindow(MyWindow);
				SelectWindow(MyWindow); 								{Bring our window to the front}
				SetPort(MyWindow);										{Prepare to write into our window}
				CtrlHandle := GetNewControl(I_Help, MyWindow);	{Make a new button						}
				CtrlHandle := GetNewControl(I_More, MyWindow);	{Make a new button						}
				UpDate_AboutIt(MyWindow);							{Do an update to draw rest of items	}
			end
		else
			SelectWindow(MyWindow);								{Already open, so show it				}
	end;

{=================================}

	procedure Do_AboutIt;
		var
			RefCon: integer;                       {RefCon for controls}
			code: integer;                         {Location of event in window or controls}
			theValue: integer;                     {Current value of a control}
			whichWindow: WindowPtr;                {Window pointer where event happened}
			myPt: Point;                           {Point where event happened}
			theControl: ControlHandle;             {Handle for a control}

		procedure Do_A_Button;                  {Handle a button being pressed}
		begin
			HiliteControl(theControl, 10);        {Darken the button}
			RefCon := GetCRefCon(theControl);     {get control refcon}
			case RefCon of                        {Select correct button}
				I_Help:							{Help, button		}
					begin
						D_Help;						{Call a dialog		}
					end;
				I_More:						{More, button	}
					begin
						D_MoreAboutGlider;		{Call a dialog		}
					end;
				otherwise
					begin
					end;
			end;								{end of case		}
			HiliteControl(theControl, 0);	{Lighten the button}
		end;

	begin
		if (MyWindow <> nil) then	{Handle only when the window is valid}
			begin
				code := FindWindow(myEvent.where, whichWindow); {Get where in window and which window}
				if (myEvent.what = MouseDown) and (MyWindow = whichWindow) then {}
					begin
						myPt := myEvent.where;				{Get mouse position}
						with MyWindow^.portBits.bounds do	{Make it relative}
							begin
								myPt.h := myPt.h + left;
								myPt.v := myPt.v + top;
							end;
					end;
				if (MyWindow = whichWindow) and (code = inContent) then {for our window	}
					begin
						code := FindControl(myPt, whichWindow, theControl);	{Get type of control	}
						if (code <> 0) then		{Check type of control}
							code := TrackControl(theControl, myPt, nil);			{Track the control	}
						if code = inButton then
							Do_A_Button;												{Do buttons			}
					end;										{End for if (MyWindow=whichWindow)	}
			end;											{End for if (MyWindow<>nil)				}
	end;

{=================================}

end.                                    {End of unit}