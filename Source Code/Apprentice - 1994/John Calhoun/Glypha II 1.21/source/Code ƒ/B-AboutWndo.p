unit AboutWndo;

interface
	uses
		Palettes, Sound, Globals;

	procedure AboutUs;
	procedure CloseLogoWindo;
	procedure OpenLogoWindo (rightOff, downOff: Integer);

implementation

	var
		ExitDialog: boolean;			{Flag used to exit the Dialog	}
		logoWindo: WindowPtr;			{Ptr to Soft Dorothy logo	}
		tempRect: Rect;
		Pic_Handle: PicHandle;

{============================================}

	procedure AboutUs;
		const						{These are the item numbers for controls in the Dialog	}
			I_Okay = 1;		{They are all found in the .RSRC file							}

		var
			GetSelection: DialogPtr;				{Name of dialog	}
			tempRect: Rect;									{Temporary rectangle}
			DType, itemHit: Integer;
			DItem: Handle;									{Handle to the dialog item}
			CItem, CTempItem: controlhandle;{Control handle}

			dummyInt: Integer;
			dialHandle: DialogTHndl;
			dialRect: Rect;

{--------------------}

		procedure Refresh_Dialog;		{Refresh the dialogs non-controls	}
		begin
			SetPort(GetSelection);
			GetDItem(GetSelection, I_Okay, DType, DItem, tempRect);
			PenSize(3, 3);
			InsetRect(tempRect, -4, -4);
			FrameRoundRect(tempRect, 16, 16);
			PenSize(1, 1);
		end;

{--------------------}

	begin
		dialHandle := DialogTHndl(Get1Resource('DLOG', aboutDialID));
		if (dialHandle <> nil) then
			begin
				HNoPurge(Handle(dialHandle));
				dialRect := dialHandle^^.boundsRect;
				OffsetRect(dialRect, -dialRect.left, -dialRect.top);
				dummyInt := (screenBits.bounds.right - dialRect.right) div 2;
				OffsetRect(dialRect, dummyInt, 0);
				dummyInt := (screenBits.bounds.bottom - dialRect.bottom) div 3;
				OffsetRect(dialRect, 0, dummyInt);
				dialHandle^^.boundsRect := dialRect;
				HPurge(Handle(dialHandle));
			end;

		GetSelection := GetNewDialog(aboutDialID, nil, Pointer(-1));	{Get dialog from rsrc	}

		ShowWindow(GetSelection);
		SelectWindow(GetSelection);
		SetPort(GetSelection);
		Refresh_Dialog;
		ExitDialog := FALSE;					{Do not exit dialog yet		}
		repeat
			ModalDialog(nil, itemHit);	{Wait until an item is hit	}
			GetDItem(GetSelection, itemHit, DType, DItem, tempRect); {Get info	}
			CItem := Pointer(DItem);		{Get the control handle		}
			if (ItemHit = I_Okay) then	{Handle Button being pressed	}
				begin
					ExitDialog := TRUE;			{Exit when this is made		}
					Refresh_Dialog;
				end;
		until ExitDialog;							{Do items til OKAY selected	}
		DisposDialog(GetSelection);		{Flush dialog out of memory	}
	end;

{=================================}

	procedure CloseLogoWindo;
	begin
		DisposeWindow(GrafPtr(logoWindo));
		logoWindo := nil;
	end;

{=================================}

	procedure OpenLogoWindo;
		var
			tempRect: Rect;
	begin
		logoWindo := nil;
		logoWindo := GetNewCWindow(1999, nil, Pointer(-1));
		SetRect(tempRect, 0, 0, 325, 318);
		SelectWindow(GrafPtr(logoWindo));
		SetPort(GrafPtr(logoWindo));
		ShowWindow(GrafPtr(logoWindo));
		Pic_Handle := GetPicture(1999);					{Get PICT (ID=1999) from .RSRC fork}
		if (Pic_Handle <> nil) then						{If the PICT (ID=1999) exists then.}
			begin
				HLock(Handle(Pic_Handle));
				tempRect := Pic_Handle^^.picFrame;
				DrawPicture(Pic_Handle, tempRect);
				HUnLock(Handle(Pic_Handle));
				ReleaseResource(Handle(Pic_Handle));{Dump PICT from RAM}
			end;
	end;

{=================================}

end.                                    {End of unit}