unit AboutWndo;

interface

	procedure D_AboutWndo (rightOff, downOff: Integer);
	procedure Init_LogoWindo;
	procedure Close_LogoWindo;
	procedure Open_LogoWindo (rightOff, downOff: Integer);
	procedure Update_LogoWindo;

implementation

	const						{These are the item numbers for controls in the Dialog	}
		I_Okay = 1;				{They are all found in the .RSRC file							}
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
		I_Iconx29 = 16;
	var
		ExitDialog: boolean;			{Flag used to exit the Dialog	}
		logoWindo: WindowPtr;		{Ptr to Soft Dorothy logo	}
		tempRect: Rect;
		Pic_Handle: PicHandle;

{===========================================================}

	function MyFilter (theDialog: DialogPtr; var theEvent: EventRecord; var itemHit: integer): boolean;
		var
			tempRect: Rect;
			thePt: Point;
	begin
		MyFilter := FALSE;
		if (theEvent.what = MouseDown) then	{Only do on a mouse click}
			begin
				thePt := theEvent.where;			{Get the point where the mouse was clicked}
				with theDialog^.portBits.bounds do	{Do this way for speed}
					begin
						thePt.h := thePt.h + left;
						thePt.v := thePt.v + top;
					end;
			end;
	end;

{===========================================================}

	procedure D_AboutWndo;
		var
			GetSelection: DialogPtr;		{Name of dialog				}
			tempRect: Rect;				{Temporary rectangle			}
			DType: Integer;				{Type of dialog item			}
			Index: Integer;				{For looping					}
			DItem: Handle;				{Handle to the dialog item		}
			CItem, CTempItem: controlhandle;	{Control handle				}
			sTemp: Str255;				{Get text entered, temp holding	}
			itemHit: Integer;				{Get selection					}
			temp: Integer;				{Get selection, temp holding		}
			TheDialogPtr: DialogPeek;		{Pointer to Dialogs definition record, contains the TE record}
			ThisEditText: TEHandle;

		procedure Refresh_Dialog;		{Refresh the dialogs non-controls	}
			var
				rTempRect: Rect;				{Temp rectangle used for drawing	}
		begin
			SetRect(tempRect, 172, 140, 252, 172);
			PenSize(3, 3);
			FrameRoundRect(tempRect, 17, 17);
			PenSize(1, 1);
		end;

	begin								{Start of dialog handler		}
		GetSelection := GetNewDialog(2, nil, Pointer(-1));	{Get dialog from rsrc	}
		with GetSelection^.portBits do
			begin
				rightOff := rightOff - bounds.left;
				downOff := downOff - bounds.top;
			end;
		MoveWindow(GetSelection, rightOff, downOff, FALSE);
		ShowWindow(GetSelection);			{Open a dialog box	}
		SelectWindow(GetSelection);		{Lets see it				}
		SetPort(GetSelection);				{}
		Refresh_Dialog;
		ExitDialog := FALSE;					{Do not exit dialog yet		}
		repeat							{Start of dialog handle loop	}
			ModalDialog(nil, itemHit);	{Wait until an item is hit	}
			GetDItem(GetSelection, itemHit, DType, DItem, tempRect); {Get info	}
			CItem := Pointer(DItem);		{Get the control handle		}
			if (ItemHit = I_Okay) then	{Handle Button being pressed	}
				begin
					ExitDialog := TRUE;			{Exit when this is made		}
					Refresh_Dialog;
				end;
		until ExitDialog;					{Do items til OKAY selected	}
		DisposDialog(GetSelection);			{Flush dialog out of memory	}
	end;

{=================================}

	procedure Init_LogoWindo;
	begin
		logoWindo := nil;
	end;

{=================================}

	procedure Close_LogoWindo;
	begin
		if (logoWindo <> nil) then
			begin
				DisposeWindow(logoWindo);
				logoWindo := nil;
			end;
	end;

{=================================}

	procedure UpDate_LogoWindo;
		var
			SavePort: WindowPtr;
	begin
		if (logoWindo <> nil) then
			begin
				GetPort(SavePort);				{Keep track of current port		}
				SetPort(logoWindo);				{Set the port to logo window		}
				Pic_Handle := GetPicture(1999);		{Get PICT (ID=1999) from .RSRC fork}
				SetRect(tempRect, 0, 0, 321, 308);	{Set the size of the bounds of PICT}
				if (Pic_Handle <> nil) then		{If the PICT (ID=1999) exists then.}
					begin
						ClipRect(tempRect);
						HLock(Handle(Pic_Handle));
						tempRect.Right := tempRect.Left + (Pic_Handle^^.picFrame.Right - Pic_Handle^^.picFrame.Left);
						tempRect.Bottom := tempRect.Top + (Pic_Handle^^.picFrame.Bottom - Pic_Handle^^.picFrame.Top);
						HUnLock(Handle(Pic_Handle));
						DrawPicture(Pic_Handle, tempRect);	{Draw the PICT into current port	}
					end;
				SetRect(tempRect, 0, 0, 1023, 1023);	{Widen the clip area again}
				ClipRect(tempRect);
				ReleaseResource(Handle(Pic_Handle));	{Dump PICT from RAM				}
				SetPort(SavePort);				{Return port to old "current" port	}
			end;
	end;

{=================================}

	procedure Open_LogoWindo;
	begin
		if (logoWindo = nil) then
			begin
				logoWindo := GetNewWindow(1999, nil, Pointer(-1));	{Get wind defs	}
				with logoWindo^.portBits do
					begin
						rightOff := rightOff - bounds.left;
						downOff := downOff - bounds.top;
					end;
				MoveWindow(logoWindo, rightOff, downOff, FALSE);
				SelectWindow(logoWindo);
				SetPort(logoWindo);
				ShowWindow(logoWindo);
				UpDate_LogoWindo;				{Get the logo faerie PICT	}
			end
		else
			SelectWindow(logoWindo);
	end;

{=================================}

end.                                    {End of unit}