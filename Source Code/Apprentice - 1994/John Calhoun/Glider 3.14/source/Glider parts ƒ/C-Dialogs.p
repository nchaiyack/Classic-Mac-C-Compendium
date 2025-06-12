unit Dialogs;

interface

	procedure D_BeginWhere (var levelBegin: Integer);
	procedure D_Controls (var leftControl, rightControl: Char);
	procedure D_Number_of_Gliders (var gliderStart: Integer);
	procedure WhosHiScore (var theName: Str255);

implementation

	const                                   {These are the item numbers for controls in the Dialog}
		I_Okay = 1;
		I_Static_Text = 2;
		I_Static_Text3 = 3;
		I_Edit_Text = 4;
		I_Edit_Text6 = 5;

	var
		ExitDialog: boolean;						{Flag used to exit the Dialog}
		GetSelection: DialogPtr;					{Name of dialog}
		tempRect: Rect;							{Temporary rectangle}
		DType: Integer;							{Type of dialog item}
		Index: Integer;							{For looping}
		iTemp: LongInt;
		DItem: Handle;							{Handle to the dialog item}
		CItem, CTempItem: controlhandle;		{Control handle}
		sTemp: Str255;							{Get text entered, temp holding	}
		cTemp: Char;
		itemHit: Integer;							{Get selection}
		temp: Integer;							{Get selection, temp holding}
		ThisEditText: TEHandle;					{Handle to get the Dialogs TE record}
		TheDialogPtr: DialogPeek;				{Pointer to Dialogs definition record, contains the TE record}

{=========================================}

	procedure D_BeginWhere;

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
		GetSelection := GetNewDialog(7, nil, Pointer(-1)); {Bring in the dialog resource}
		tempRect := GetSelection^.portRect;	{Get window size, we will now center it}
		tempRect.Top := ((screenBits.Bounds.Bottom - screenBits.Bounds.Top) - (tempRect.Bottom - tempRect.Top)) div 2;
		tempRect.Left := ((screenBits.Bounds.Right - screenBits.Bounds.Left) - (tempRect.Right - tempRect.Left)) div 2;
		MoveWindow(GetSelection, tempRect.Left, tempRect.Top, TRUE);{Now move the window to the proper position}
		ShowWindow(GetSelection);			{Open a dialog box}
		SelectWindow(GetSelection);		{Lets see it}
		SetPort(GetSelection);				{Perpare to add conditional text}
  {Setup initial conditions}
		GetDItem(GetSelection, I_Edit_Text, DType, DItem, tempRect);{Get item handle}
		NumToString(levelBegin, sTemp);
		SetIText(DItem, sTemp);			{Set the default text string}
		SelIText(GetSelection, I_Edit_Text, 0, 2);
		Refresh_Dialog;
		ExitDialog := FALSE;					{Do not exit dialog handle loop yet}
		repeat
			ModalDialog(nil, itemHit);			{Wait until an item is hit}
			GetDItem(GetSelection, itemHit, DType, DItem, tempRect);		{Get item information	}
			CItem := Pointer(DItem);				{Get the control handle									}
			if (ItemHit = I_Okay) then			{Handle the Button being pressed						}
				begin
					GetDItem(GetSelection, I_Edit_Text, DType, DItem, tempRect);	{Get handle		}
					GetIText(DItem, sTemp);			{Get the text entered									}
					StringToNum(sTemp, iTemp);		{Covert this text into an integer first.				}
					if (iTemp >= 1) and (iTemp <= 9) then		{First, is the number in proper range?	}
						begin
							ExitDialog := TRUE;				{Exit the dialog when this selection is made			}
						end
					else
						begin
							SysBeep(1);             			{Whoops, they're breaking the rules (or trying to)}
							SysBeep(1);						{Two beeps to you, Mr. or Ms. cheater!				}
							NumToString(levelBegin, sTemp);		{Reset to the original value					}
							SetIText(DItem, sTemp);
						end;
				end;									{End for this item selected								}
		until ExitDialog;							{Handle dialog items until exit selected				}
		GetDItem(GetSelection, I_Edit_Text, DType, DItem, tempRect);	{Get the item handle	}
		GetIText(DItem, sTemp);				{Get the text entered}
		StringToNum(sTemp, iTemp);
		levelBegin := iTemp;
		DisposDialog(GetSelection);			{Flush the dialog out of memory}
	end;											{End of procedure										}

{=========================================}

	procedure D_Controls;

		procedure Refresh_Dialog;               {Refresh the dialogs non-controls}
			var
				tempRect: Rect;                       {Temp rectangle used for drawing}
		begin
			PenNormal;
			GetDItem(GetSelection, I_Okay, DType, DItem, tempRect);{Get the item handle}
			PenSize(3, 3);  				{Change pen to draw thick default outline}
			InsetRect(tempRect, -4, -4);		{Draw outside the button by 1 pixel}
			FrameRoundRect(tempRect, 16, 16); {Draw the outline}
			PenSize(1, 1);
		end;

	begin                                   {Start of dialog handler}
		GetSelection := GetNewDialog(6, nil, Pointer(-1)); {Bring in the dialog resource}
		tempRect := GetSelection^.portRect;	{Get window size, we will now center it}
		tempRect.Top := ((screenBits.Bounds.Bottom - screenBits.Bounds.Top) - (tempRect.Bottom - tempRect.Top)) div 2;
		tempRect.Left := ((screenBits.Bounds.Right - screenBits.Bounds.Left) - (tempRect.Right - tempRect.Left)) div 2;
		MoveWindow(GetSelection, tempRect.Left, tempRect.Top, TRUE);{Now move the window to the proper position}
		ShowWindow(GetSelection);				{Open a dialog box}
		SelectWindow(GetSelection);			{Lets see it}
		SetPort(GetSelection);					{Perpare to add conditional text}
  {Setup initial conditions}
		GetDItem(GetSelection, I_Edit_Text, DType, DItem, tempRect); {Get the item handle}
		SetIText(DItem, leftControl);			{Set the default text string}
		GetDItem(GetSelection, I_Edit_Text6, DType, DItem, tempRect); {Get the item handle}
		SetIText(DItem, rightControl);			{Set the default text string}
		SelIText(GetSelection, I_Edit_Text6, 0, 2);
		Refresh_Dialog;
		ExitDialog := FALSE;                    {Do not exit dialog handle loop yet}
		repeat                                {Start of dialog handle loop}
			ModalDialog(nil, itemHit);          {Wait until an item is hit}
			GetDItem(GetSelection, itemHit, DType, DItem, tempRect); {Get item information}
			CItem := Pointer(DItem);            {Get the control handle}
{Handle it real time}
			if (ItemHit = I_Okay) then           {Handle the Button being pressed}
				begin
					GetDItem(GetSelection, I_Edit_Text, DType, DItem, tempRect); {Get the item handle}
					GetIText(DItem, sTemp);
					cTemp := COPY(sTemp, 1, 1);
					if (ORD(cTemp) < 127) and (ORD(cTemp) > 31) then
						begin
							ExitDialog := TRUE;
						end
					else
						begin
							SysBeep(1);
							SysBeep(1);
							SetIText(DItem, leftControl);
						end;
					GetDItem(GetSelection, I_Edit_Text6, DType, DItem, tempRect);
					GetIText(DItem, sTemp);
					cTemp := Copy(sTemp, 1, 1);
					if (Ord(cTemp) < 127) and (Ord(cTemp) > 31) then
						begin
							ExitDialog := TRUE;
						end
					else
						begin
							SysBeep(1);
							SysBeep(1);
							SetIText(DItem, rightControl);
						end;
				end;                                {End for this item selected}
		until ExitDialog;                     {Handle dialog items until exit selected}
 {Get results after dialog}
		GetDItem(GetSelection, I_Edit_Text, DType, DItem, tempRect); {Get the item handle}
		GetIText(DItem, sTemp);
		leftControl := Copy(sTemp, 1, 1);
		GetDItem(GetSelection, I_Edit_Text6, DType, DItem, tempRect); {Get the item handle}
		GetIText(DItem, sTemp);
		rightControl := Copy(sTemp, 1, 1);
		DisposDialog(GetSelection);           {Flush the dialog out of memory}
	end;                                    {End of procedure}

{=========================================}

	procedure D_Number_of_Gliders;

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

	begin
		GetSelection := GetNewDialog(5, nil, Pointer(-1)); {Bring in the dialog resource}
		tempRect := GetSelection^.portRect;	{Get window size, we will now center it}
		tempRect.Top := ((screenBits.Bounds.Bottom - screenBits.Bounds.Top) - (tempRect.Bottom - tempRect.Top)) div 2;
		tempRect.Left := ((screenBits.Bounds.Right - screenBits.Bounds.Left) - (tempRect.Right - tempRect.Left)) div 2;
		MoveWindow(GetSelection, tempRect.Left, tempRect.Top, TRUE);{Now move the window to the proper position}
		ShowWindow(GetSelection);			{Open a dialog box}
		SelectWindow(GetSelection);		{Lets see it}
		SetPort(GetSelection);				{Perpare to add conditional text}

		GetDItem(GetSelection, I_Edit_Text, DType, DItem, tempRect);
		NumToString(gliderStart, sTemp);
		SetIText(DItem, sTemp);				{Set the default text string}
		SelIText(GetSelection, I_Edit_Text, 0, 2);
		Refresh_Dialog;
		ExitDialog := FALSE;					{Do not exit dialog handle loop yet}
		repeat								{Start of dialog handle loop}
			ModalDialog(nil, itemHit);			{Wait until an item is hit}
			GetDItem(GetSelection, itemHit, DType, DItem, tempRect); {Get item information}
			CItem := Pointer(DItem);			{Get the control handle}
			if (ItemHit = I_Okay) then		{Handle the Button being pressed}
				begin
					GetDItem(GetSelection, I_Edit_Text, DType, DItem, tempRect); {Get the item handle}
					GetIText(DItem, sTemp);		{Get the text entered}
					StringToNum(sTemp, iTemp);
					if (iTemp >= 1) and (iTemp <= 6) then
						begin
							gliderStart := iTemp;
							ExitDialog := TRUE;		{Exit the dialog when this selection is made}
						end
					else
						begin
							SysBeep(1);
							SysBeep(1);
							NumToString(gliderStart, sTemp);
							SetIText(DItem, sTemp);	{Set the default text string}
						end;
				end;								{End for this item selected}
		until ExitDialog;						{Handle dialog items until exit selected}
		DisposDialog(GetSelection);			{Flush the dialog out of memory}
	end;

{===========================================}

	procedure WhosHiScore;
		var
			excessSpace: Integer;
			dotFiller, space: Str255;

		procedure Refresh_Dialog;				{This draws the rounded-rectangular}
			var											{border around the Okay button so that}
				rTempRect: Rect;						{the user knows it is the default button.}
		begin
			PenNormal;
			GetDItem(GetSelection, I_Okay, DType, DItem, tempRect);{Get the item handle}
			PenSize(3, 3);  				{Change pen to draw thick default outline}
			InsetRect(tempRect, -4, -4);		{Draw outside the button by 1 pixel}
			FrameRoundRect(tempRect, 16, 16); {Draw the outline}
			PenSize(1, 1);
		end;

	begin												{Start of dialog handler}
		GetSelection := GetNewDialog(999, nil, Pointer(-1));{Bring in the dialog resource}
		tempRect := GetSelection^.portRect;	{Get window size, we will now center it}
		tempRect.Top := ((screenBits.Bounds.Bottom - screenBits.Bounds.Top) - (tempRect.Bottom - tempRect.Top)) div 2;
		tempRect.Left := ((screenBits.Bounds.Right - screenBits.Bounds.Left) - (tempRect.Right - tempRect.Left)) div 2;
		MoveWindow(GetSelection, tempRect.Left, tempRect.Top, TRUE);{Now move the window to the proper position}
		ShowWindow(GetSelection);						{Open a dialog box}
		SelectWindow(GetSelection);					{Lets see it}
		SetPort(GetSelection);							{Perpare to add conditional text}

		GetDItem(GetSelection, I_Edit_Text, DType, DItem, tempRect);
		SetIText(DItem, theName);						{Set the default text string}
		SelIText(GetSelection, I_Edit_Text, 0, 15);
		ExitDialog := FALSE;								{Do not exit dialog handle loop yet}
		Refresh_Dialog;
		repeat											{Start of dialog handle loop}
			ModalDialog(nil, itemHit);						{Wait until an item is hit}
			GetDItem(GetSelection, itemHit, DType, DItem, tempRect); {Get item information}
			CItem := Pointer(DItem);						{Get the control handle}
			if (ItemHit = I_Okay) then					{Handle the Button being pressed}
				begin
					GetDItem(GetSelection, I_Edit_Text, DType, DItem, tempRect); {Get the item handle}
					GetIText(DItem, sTemp);					{Get the text entered}
					if (LENGTH(sTemp) > 15) then			{Make sure it's less than 15 characters}
						theName := COPY(sTemp, 1, 15)		{Just clip the first 15 if it is too long}
					else
						begin
							if (LENGTH(sTemp) < 15) then
								begin
									space := '               ';
									excessSpace := 15 - LENGTH(sTemp);
									dotFiller := COPY(space, 1, excessSpace);
									sTemp := CONCAT(sTemp, dotfiller);
								end;
							theName := sTemp;							{Otherwise, take it as is}
						end;
					ExitDialog := TRUE;								{Exit the dialog when this selection is made}
				end;												{End for this item selected}
		until ExitDialog;										{Handle dialog items until exit selected}
		DisposDialog(GetSelection);						{Flush the dialog out of memory}
	end;														{End of procedure}

{===========================================}

end.                                    {End of unit}