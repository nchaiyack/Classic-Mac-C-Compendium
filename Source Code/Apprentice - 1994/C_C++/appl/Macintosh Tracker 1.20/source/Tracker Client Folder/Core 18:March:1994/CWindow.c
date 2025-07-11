/* CWindow.c */

#define COMPILING_CWINDOW_C
#include "CWindow.h"
#include "Memory.h"
#include "CSack.h"
#include "CApplication.h"
#include "Compatibility.h"
#include "MenuController.h"


CWindow*					ActiveWindow;

LongPoint					ZeroPoint = {0,0};

#define MAXTEXTSIZING (32767)

#define DEBUGSCROLL(Rgn)  InvertRgn(Rgn); { long Start; Start = TickCount(); while ((signed long)TickCount() - Start < 4); } InvertRgn(Rgn);

/* setting this to 1 enables manual munging of the update region for the */
/* scrollrect routines */
#define DOTHENAUGHTY (0)

EXECUTE(static char GrafErrMsg[]
	= {"CWindow drawing function called while other window's GrafPort was active."};)


/* */		CWindow::CWindow()
	{
		CSack*		Temp;

		OurKeyReceiver = NIL;
		Temp = new CSack;
		Temp->ISack(sizeof(CViewRect*),128);
		ListOfWindowClients = Temp;
		MyGrafPtr = NIL;
		ZoomStatePartCode = inZoomOut; /* we start zoomed in */
	}


/* */		CWindow::~CWindow()
	{
		CViewRect*	Thang;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::~CWindow called on uninitialized object."));
		ListOfWindowClients->ResetScan();
		while (ListOfWindowClients->GetNext(&Thang))
			{
				Thang->Window = NIL;
			}
		delete ListOfWindowClients;
		if (ActiveWindow == this)
			{
				ActiveWindow = NIL;
			}
		DeleteMyGrafPtr();
	}


/* this creates the window record.  Override & call this last */
void		CWindow::IWindow(LongPoint Start, LongPoint Extent, MyBoolean ModalFlag,
					MyBoolean GrowFlag, MyBoolean ZoomFlag)
	{
		Rect		ShortWhere;

		ERROR(Initialized == True,PRERR(ForceAbort,
			"CWindow::IWindow called on already initialized object."));
		EXECUTE(Initialized = True);
		Modality = ModalFlag;
		Growable = GrowFlag;
		Zoomable = ZoomFlag;
		Origin = ZeroPoint; /* this never changes, since the window is what everyone */
		/* else references their origins from. */
		MakeMyGrafPtr(Start,Extent);
		IEnclosure(Start,Extent,this,NIL);
		DoFindPosition();
		VisRectStart = ZeroPoint;
		VisRectExtent = Extent;
		Origin = ZeroPoint;
		SetWRefCon(MyGrafPtr,(long)this);
	}


/* user should override this & create his own MyWindowPtr */
void		CWindow::MakeMyGrafPtr(LongPoint Start, LongPoint Extent)
	{
		Rect		BoundsRect;
		short		WindowCode;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::MakeMyGrafPtr called on uninitialized object."));
		ERROR(MyGrafPtr!=NIL,PRERR(ForceAbort,
			"CWindow::MakeMyGrafPtr called, but MyGrafPtr is not NIL."));
		BoundsRect = RectOf(Start.x,Start.y,Start.x+Extent.x,Start.y+Extent.y);
		if (Growable)
			{
				if (Zoomable)
					{
						WindowCode = zoomDocProc;
					}
				 else
					{
						WindowCode = documentProc;
					}
			}
		 else
			{
				if (Zoomable)
					{
						WindowCode = zoomNoGrow;
					}
				 else
					{
						WindowCode = noGrowDocProc;
					}
			}
		MyGrafPtr = NewWindow(NIL,&BoundsRect,"\p",True,WindowCode,(void*)-1,
			True,(long)this);
		if (MyGrafPtr == NIL)
			{
				PRERR(ForceAbort,"NewWindow failed to allocate memory.");
			}
	}


void			CWindow::DeleteMyGrafPtr(void)
	{
		ERROR(MyGrafPtr==NIL,PRERR(ForceAbort,
			"CWindow::DeleteMyGrafPtr called but MyGrafPtr is already NIL."));
		DisposeWindow(MyGrafPtr);
	}


void			CWindow::DoUpdate(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::DoUpdate called on uninitialized object."));
		ERROR(MyGrafPtr==NIL,PRERR(ForceAbort,
			"CWindow::DoUpdate called but MyGrafPtr is already NIL."));
		BeginUpdate(MyGrafPtr);  /* begin the process */
		inherited::DoUpdate();
		if (Growable)
			{
				LongPoint			LocalExtent = {16,16};

				SetUpPort();
				if (Suspended)
					{
						LDrawPicture(GetPic(EmptyGrowIconPictID),
							LongPointOf(Extent.x-15,Extent.y-15),LocalExtent);
					}
				 else
					{
						LDrawPicture(GetPic(GrowIconPictID),
							LongPointOf(Extent.x-15,Extent.y-15),LocalExtent);
					}
			}
		EndUpdate(MyGrafPtr);  /* stop updating */
	}


void			CWindow::DoSuspend(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::DoSuspend called on uninitialized object."));
		OurKeyReceiver = KeyReceiverViewRect;
		if (KeyReceiverViewRect != NIL)
			{
				KeyReceiverViewRect->RelinquishKeyReceivership();
			}
		if (Growable)
			{
				LongPoint			LocalExtent = {16,16};

				SetUpPort();
				LDrawPicture(GetPic(EmptyGrowIconPictID),
					LongPointOf(Extent.x-15,Extent.y-15),LocalExtent);
			}
		inherited::DoSuspend();
		KeyReceiverViewRect = NIL;
	}


void			CWindow::DoResume(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::DoResume called on uninitialized object."));
		if (Growable)
			{
				LongPoint			LocalExtent = {16,16};

				SetUpPort();
				LDrawPicture(GetPic(GrowIconPictID),
					LongPointOf(Extent.x-15,Extent.y-15),LocalExtent);
			}
		inherited::DoResume();
		if (OurKeyReceiver != NIL)
			{
				OurKeyReceiver->BecomeKeyReceiver();
			}
	}


void			CWindow::SetDefaultKeyReceiver(CViewRect* TheReceiver)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::SetDefaultKeyReceiver called on uninitialized object."));
		ERROR(!Suspended,PRERR(ForceAbort,
			"CWindow::SetDefaultKeyReceiver attempted to set default key Receiver while not suspended."));
		OurKeyReceiver = TheReceiver;
	}


/* track the go-away thing */
void			CWindow::DoGoAway(Point GlobalStart)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::DoGoAway called on uninitialized object."));
		ERROR(MyGrafPtr==NIL,PRERR(ForceAbort,
			"CWindow::DoGoAway called but MyGrafPtr is already NIL."));
		if (TrackGoAway(MyGrafPtr,GlobalStart))
			{
				GoAway();
			}
	}


/* override to provide a go-away check function */
void			CWindow::GoAway(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::GoAway called on uninitialized object."));
		DoMenuCommand(mFileClose);
	}


/* get the size constraints for the window */
Rect		CWindow::GetConstraint(void)
	{
		Rect		Temp = {100,100,32767,32767};

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::GetConstraint called on uninitialized object."));
		return Temp;
	}


LongPoint	CWindow::SnapToGrid(LongPoint NewWindowSize)
	{
		return NewWindowSize;
	}


void			CWindow::DoFindPosition(void)
	{
		Point		Delta = {0,0};

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::DoFindPosition called on uninitialized object."));
		SetMyPort();
		LocalToGlobal(&Delta); /* find top-left with respect to screen */
		Start.x = Delta.h;
		Start.y = Delta.v;
	}


void			CWindow::SetNewSize(LongPoint NewWindowSize)
	{
		LongPoint		DeltaExtent;
		CViewRect*	Thang;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::SetNewSize called on uninitialized object."));
		DeltaExtent.x = NewWindowSize.x - Extent.x;
		DeltaExtent.y = NewWindowSize.y - Extent.y;
		Extent = NewWindowSize;
		SizeWindow(MyGrafPtr,Extent.x,Extent.y,True);
		DoFindPosition();
		VisRectStart = ZeroPoint;
		VisRectExtent = Extent;
		Origin = ZeroPoint;
		SetUpPort();
		Window->LEraseRect(ZeroPoint,Extent);
		Window->InvalidateLong(ZeroPoint,Extent);
		ListOfObjects->ResetScan();
		while (ListOfObjects->GetNext(&Thang))
			{
				Thang->DoEnclosureResized(DeltaExtent);
			}
	}


void			CWindow::DoGrow(Point GlobalStart)
	{
		long				Result;
		Rect				Limits;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::DoGrow called on uninitialized object."));
		Limits = GetConstraint();
		Result = GrowWindow(MyGrafPtr,GlobalStart,&Limits);
		if (Result != 0)
			{
				SetNewSize(SnapToGrid(LongPointOf(0x0000ffff & Result/*X coord*/,
					0x0000ffff & (Result >> 16)/*Ycoord*/)));
			}
	}


void			CWindow::DoZoomIn(Point GlobalStart)
	{
		if (TrackBox(MyGrafPtr,GlobalStart,inZoomIn))
			{
				ZoomIn();
			}
	}


void			CWindow::DoZoomOut(Point GlobalStart)
	{
		if (TrackBox(MyGrafPtr,GlobalStart,inZoomOut))
			{
				ZoomOut();
			}
	}


void			CWindow::ZoomIn(void)
	{
		ZoomStatePartCode = inZoomOut; /* opposite--state BEFORE THIS zoom occurs */
		SetUpPort();
		LEraseRect(ZeroPoint,Extent);
		InvalidateLong(ZeroPoint,Extent);
		ZoomWindow(MyGrafPtr,inZoomIn,False);
		SetNewSize(SnapToGrid(
			LongPointOf(MyGrafPtr->portRect.right - MyGrafPtr->portRect.left,
			MyGrafPtr->portRect.bottom - MyGrafPtr->portRect.top)));
	}


void			CWindow::ZoomOut(void)
	{
		ZoomStatePartCode = inZoomIn; /* opposite--state BEFORE THIS zoom occurs */
		SetUpPort();
		LEraseRect(ZeroPoint,Extent);
		InvalidateLong(ZeroPoint,Extent);
		ZoomWindow(MyGrafPtr,inZoomOut,False);
		SetNewSize(SnapToGrid(
			LongPointOf(MyGrafPtr->portRect.right - MyGrafPtr->portRect.left,
			MyGrafPtr->portRect.bottom - MyGrafPtr->portRect.top)));
	}


void			CWindow::DoDrag(Point GlobalStart)
	{
		Rect				BoundsRect;
		RgnHandle		BoundsRegion;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::DoDrag called on uninitialized object."));
		BoundsRegion = GetGrayRgn();
		BoundsRect = (**BoundsRegion).rgnBBox;
		InsetRect(&BoundsRect,4,4);
		DragWindow(MyGrafPtr,GlobalStart,&BoundsRect);
		DoFindPosition();
	}


void			CWindow::DoMouseDown(MyEventRec Event)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::DoMouseDown called on uninitialized object."));
		if (ActiveWindow == this)
			{
				inherited::DoMouseDown(Event);
			}
		 else
			{
				SelectWindow(MyGrafPtr);
			}
	}


void			CWindow::BecomeActiveWindow(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::BecomeActiveWindow called on uninitialized object."));
		if ((ActiveWindow == NIL) || ((ActiveWindow != NIL)
			&& (ActiveWindow->Modality == ModelessWindow)))
			{
				SelectWindow(MyGrafPtr);
			}
	}


void			CWindow::RegisterClient(CViewRect* Client)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::RegisterClient called on uninitialized object."));
		ListOfWindowClients->PushElement(&Client);
	}


void			CWindow::DeregisterClient(CViewRect* Client)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CWindow::DeregisterClient called on uninitialized object."));
		ListOfWindowClients->KillElement(&Client);
	}


/********************************************************************************/
/* LongRect versions of QuickDraw routines */


void			CWindow::SetMyPort(void)
	{
		Rect			Temp;

		if (thePort != MyGrafPtr)
			{
				SetPort(MyGrafPtr);
			}
		ClipStart = ZeroPoint;
		ClipExtent = Extent;
		ResetPen();
		EXECUTE(SetText(systemFont,outline,srcOr,36,0));
	}


void			CWindow::SetOrigin(LongPoint TheOrigin)
	{
		DrawingOrigin = TheOrigin;
	}


void			CWindow::SetClipRect(LongPoint Start, LongPoint Extent)
	{
		Rect				Temp;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		Start.x += DrawingOrigin.x;
		Start.y += DrawingOrigin.y;
		SectLongRect(ClipStart,ClipExtent,Start,Extent,&ClipStart,&ClipExtent);
		Temp.left = ClipStart.x;
		Temp.top = ClipStart.y;
		Temp.right = ClipStart.x + ClipExtent.x;
		Temp.bottom = ClipStart.y + ClipExtent.y;
		ClipRect(&Temp);
	}


void			CWindow::SetPen(short Width, short Height, short Mode, Pattern ThePattern)
	{
		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		PenSize(Width,Height);
		PenMode(Mode);
		PenPat(ThePattern);
	}


void			CWindow::ResetPen(void)
	{
		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		PenNormal();
	}


void			CWindow::SetPenMode(short Mode)
	{
		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		PenMode(Mode);
	}


void			CWindow::SetPenSize(short Width, short Height)
	{
		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		PenSize(Width,Height);
	}


void			CWindow::SetGreyishTextOr(void)
	{
		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		ResetPen();
		PenMode(srcCopy);
		PenPat(gray);
		TextMode(grayishTextOr);
	}


void			CWindow::DrawLine(LongPoint Start, LongPoint Extent)
	{
		Rect		Place;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		if (LongRectToShort(Start,Extent,&Place))
			{
				MoveTo(Place.left,Place.top);
				LineTo(Place.right,Place.bottom);
			}
	}


void			CWindow::SetText(short FontID, Style Face, short Mode, short PointSize, Fixed Space)
	{
		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		TextFont(FontID);
		TextFace(Face);
		TextMode(Mode);
		TextSize(PointSize);
		SpaceExtra(Space);
	}


void			CWindow::SetTextMode(short Mode)
	{
		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		TextMode(Mode);
	}


void			CWindow::LTextBox(LongPoint Start, LongPoint Extent, Handle Text, short Justification)
	{
		Rect		Temp;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		LongRectToShort(Start,Extent,&Temp);
		HLock(Text);
		TextBox(*Text,HandleSize(Text),&Temp,(short)Justification);
		HUnlock(Text);
	}


void			CWindow::LDrawText(LongPoint Start, LongPoint Extent, Handle Text, short Justification)
	{
		Rect			Temp;
		short			NumPixels;
		short			YPos;
		FontInfo	FInfo;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		LongRectToShort(Start,Extent,&Temp);
		GetFontInfo(&FInfo);
		HLock(Text);
		NumPixels = TextWidth(*Text,0,HandleSize(Text));
		YPos = (Temp.top+Temp.bottom-(FInfo.ascent+FInfo.descent+FInfo.leading))/2
			+ FInfo.ascent + FInfo.leading;
		switch (Justification)
			{
				case JustifyLeft:
					MoveTo(Temp.left,YPos);
					break;
				case JustifyCenter:
					MoveTo((Temp.left+Temp.right-NumPixels)/2,YPos);
					break;
				case JustifyRight:
					MoveTo((Temp.right-NumPixels),YPos);
					break;
				default:
					EXECUTE(PRERR(AllowResume,
						"Illegal text justification code passed to CWindow::LDrawText."));
					break;
			}
		DrawText(*Text,0,HandleSize(Text));
		HUnlock(Text);
	}


void			CWindow::LDrawTextPtr(LongPoint Start, LongPoint Extent, char* Text,
						long Length, short Justification)
	{
		Rect			Temp;
		short			NumPixels;
		short			YPos;
		FontInfo	FInfo;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		LongRectToShort(Start,Extent,&Temp);
		GetFontInfo(&FInfo);
		NumPixels = TextWidth(Text,0,Length);
		YPos = (Temp.top+Temp.bottom-(FInfo.ascent+FInfo.descent+FInfo.leading))/2
			+ FInfo.ascent + FInfo.leading;
		switch (Justification)
			{
				case JustifyLeft:
					MoveTo(Temp.left,YPos);
					break;
				case JustifyCenter:
					MoveTo((Temp.left+Temp.right-NumPixels)/2,YPos);
					break;
				case JustifyRight:
					MoveTo((Temp.right-NumPixels),YPos);
					break;
				default:
					EXECUTE(PRERR(AllowResume,
						"Illegal text justification code passed to CWindow::LDrawText."));
					break;
			}
		DrawText(Text,0,Length);
	}


long			CWindow::LMeasureText(char* Text, long NumChars)
	{
		long			CurrentLinePixels;
		long			IndexIntoBuffer;

		CurrentLinePixels = 0;
		IndexIntoBuffer = 0;
		while (NumChars > 0)
			{
				if (NumChars > MAXTEXTSIZING)
					{
						CurrentLinePixels += TextWidth(&(Text[IndexIntoBuffer]),0,MAXTEXTSIZING);
						NumChars -= MAXTEXTSIZING;
						IndexIntoBuffer += MAXTEXTSIZING;
					}
				 else
					{
						CurrentLinePixels += TextWidth(&(Text[IndexIntoBuffer]),0,NumChars);
						NumChars = 0;
					}
			}
		return CurrentLinePixels;
	}


void			CWindow::LDrawPicture(PicHandle ThePicture, LongPoint Start, LongPoint Extent)
	{
		Rect		Temp;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		if (LongRectToShort(Start,Extent,&Temp))
			{
				DrawPicture(ThePicture,&Temp);
			}
	}


void			CWindow::LFrameRect(LongPoint Start, LongPoint Extent)
	{
		Rect		Temp;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		LongRectToShort(Start,Extent,&Temp);
		FrameRect(&Temp);
	}


void			CWindow::LPaintRect(LongPoint Start, LongPoint Extent)
	{
		Rect		Temp;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		LongRectToShort(Start,Extent,&Temp);
		PaintRect(&Temp);
	}


void			CWindow::LEraseRect(LongPoint Start, LongPoint Extent)
	{
		Rect		Temp;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		LongRectToShort(Start,Extent,&Temp);
		EraseRect(&Temp);
	}


void			CWindow::LFrameOval(LongPoint Start, LongPoint Extent)
	{
		Rect		Temp;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		LongRectToShort(Start,Extent,&Temp);
		FrameOval(&Temp);
	}


void			CWindow::LPaintOval(LongPoint Start, LongPoint Extent)
	{
		Rect		Temp;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		LongRectToShort(Start,Extent,&Temp);
		PaintOval(&Temp);
	}


void			CWindow::LEraseOval(LongPoint Start, LongPoint Extent)
	{
		Rect		Temp;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		LongRectToShort(Start,Extent,&Temp);
		EraseOval(&Temp);
	}


void			CWindow::LFrameRoundRect(LongPoint Start, LongPoint Extent, short OvalWidth, short OvalHeight)
	{
		Rect		Temp;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		LongRectToShort(Start,Extent,&Temp);
		FrameRoundRect(&Temp,OvalWidth,OvalHeight);
	}


void			CWindow::LPaintRoundRect(LongPoint Start, LongPoint Extent, short OvalWidth, short OvalHeight)
	{
		Rect		Temp;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		LongRectToShort(Start,Extent,&Temp);
		PaintRoundRect(&Temp,OvalWidth,OvalHeight);
	}


void			CWindow::LEraseRoundRect(LongPoint Start, LongPoint Extent, short OvalWidth, short OvalHeight)
	{
		Rect		Temp;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		LongRectToShort(Start,Extent,&Temp);
		EraseRoundRect(&Temp,OvalWidth,OvalHeight);
	}


void			CWindow::SetPattern(Pattern ThePattern)
	{
		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		PenPat(ThePattern);
	}


void			CWindow::InvalidateLong(LongPoint Start, LongPoint Extent)
	{
		Rect			ShortOne;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		LongRectToShort(Start,Extent,&ShortOne);
		Start.x = ShortOne.left;
		Start.y = ShortOne.top;
		Extent.x = ShortOne.right - ShortOne.left;
		Extent.y = ShortOne.bottom - ShortOne.top;
		SectLongRect(Start,Extent,ClipStart,ClipExtent,&Start,&Extent);
		ShortOne.left = Start.x;
		ShortOne.top = Start.y;
		ShortOne.right = Start.x + Extent.x;
		ShortOne.bottom = Start.y + Extent.y;
		InvalRect(&ShortOne);
	}


void			CWindow::ScrollLong(LongPoint Start, LongPoint Extent, LongPoint Change)
	{
		RgnHandle		UpdateRegion;
		RgnHandle		WhiteRegion;
		Rect				TheRect;
		RgnHandle		InvalidRegion;
		RgnHandle		TempRgn;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		/* we constrain the range into a short integer.  This will not cause */
		/* problems until we start getting REALLY BIG monitors. */
		if ((Change.x < -32767) || (Change.x > 32767) || (Change.y < -32767)
			|| (Change.y > 32767))
			{
				/* erase screen and invalidate nothing */
				LEraseRect(Start,Extent);
				return;
			}
		/* converting to a regular rect */
		LongRectToShort(Start,Extent,&TheRect);
		/* create update region */
		UpdateRegion = NewRgn();
		/* do the scroll */
		ScrollRect(&TheRect,Change.x,Change.y,UpdateRegion);
		/* we want to add to the update region any area that was already invalid */
		InvalidRegion = NewRgn();
		TempRgn = NewRgn();
		RectRgn(InvalidRegion,&TheRect);
		CopyRgn(InvalidRegion,TempRgn);
		SectRgn(((WindowPeek)MyGrafPtr)->updateRgn,InvalidRegion,InvalidRegion);
		OffsetRgn(InvalidRegion,Change.x,Change.y);
		SectRgn(InvalidRegion,TempRgn,InvalidRegion);
		DisposeRgn(TempRgn);
		UnionRgn(InvalidRegion,UpdateRegion,UpdateRegion);
		DisposeRgn(InvalidRegion);
		/* it is assumed that the program will redraw the empty area created by scrolling. */
		/* we only want to add the area created by overlapping windows. */
		WhiteRegion = NewRgn();
		if (Change.x >= 0)
			{
				SetRectRgn(WhiteRegion,TheRect.left,TheRect.top,
					TheRect.left + Change.x,TheRect.bottom);
			}
		 else
			{
				SetRectRgn(WhiteRegion,TheRect.right + Change.x,
					TheRect.top,TheRect.right,TheRect.bottom);
			}
		DiffRgn(UpdateRegion,WhiteRegion,UpdateRegion);
		if (Change.y >= 0)
			{
				SetRectRgn(WhiteRegion,TheRect.left,TheRect.top,
					TheRect.right,TheRect.top + Change.y);
			}
		 else
			{
				SetRectRgn(WhiteRegion,TheRect.left,TheRect.bottom + Change.y,
					TheRect.right,TheRect.bottom);
			}
		DiffRgn(UpdateRegion,WhiteRegion,UpdateRegion);
#if DOTHENAUGHTY
		/* I have to do this since InvalRgn isn't working properly. */
		UnionRgn(UpdateRegion,((WindowPeek)MyGrafPtr)->updateRgn,
			((WindowPeek)MyGrafPtr)->updateRgn);
#endif
		InvalRgn(UpdateRegion);
		/* get rid of update region */
		DisposeRgn(UpdateRegion);
		DisposeRgn(WhiteRegion);
	}


void			CWindow::ScrollLongAndInvalidate(LongPoint Start, LongPoint Extent, LongPoint Change)
	{
		RgnHandle		UpdateRegion;
		Rect				TheRect;
		RgnHandle		InvalidRegion;
		RgnHandle		TempRgn;

		ERROR(MyGrafPtr != thePort,PRERR(ForceAbort,GrafErrMsg));
		if ((Change.x < -32767) || (Change.x > 32767) || (Change.y < -32767)
			|| (Change.y > 32767))
			{
				/* if we scroll out of qd space, just erase & return */
				LEraseRect(Start,Extent);
				return;
			}
		/* converting to a regular rect */
		LongRectToShort(Start,Extent,&TheRect);
		/* create update region */
		UpdateRegion = NewRgn();
		/* do the scroll */
		ScrollRect(&TheRect,Change.x,Change.y,UpdateRegion);
		/* we want to add to the update region any area that was already invalid */
		InvalidRegion = NewRgn();
		TempRgn = NewRgn();
		RectRgn(InvalidRegion,&TheRect);
		CopyRgn(InvalidRegion,TempRgn);
		SectRgn(((WindowPeek)MyGrafPtr)->updateRgn,InvalidRegion,InvalidRegion);
		OffsetRgn(InvalidRegion,Change.x,Change.y);
		SectRgn(InvalidRegion,TempRgn,InvalidRegion);
		DisposeRgn(TempRgn);
		UnionRgn(InvalidRegion,UpdateRegion,UpdateRegion);
		DisposeRgn(InvalidRegion);
		/* invalidate all bad areas */
#if DOTHENAUGHTY
		/* I have to do this since InvalRgn isn't working properly. */
		UnionRgn(UpdateRegion,((WindowPeek)MyGrafPtr)->updateRgn,
			((WindowPeek)MyGrafPtr)->updateRgn);
#endif
		InvalRgn(UpdateRegion);
		/* get rid of update region */
		DisposeRgn(UpdateRegion);
	}


/********************************************************************************/
/* utility routines */


MyBoolean	CWindow::RectVisible(LongPoint Start, LongPoint Extent)
	{
		Rect		Temp;

		LongRectToShort(Start,Extent,&Temp);
		return RectInRgn(&Temp,MyGrafPtr->visRgn);
	}


MyBoolean	CWindow::LongRectToShort(LongPoint Start, LongPoint Extent, Rect* ShortOne)
	{
		register	MyBoolean	Flag;

		Start.x += DrawingOrigin.x;
		Start.y += DrawingOrigin.y;
		Extent.x += Start.x;
		Extent.y += Start.y;
		Flag = True;
		if (Start.x < -32767) {Start.x = -32767; Flag = False;}
		if (Start.x > 32767) {Start.x = 32767; Flag = False;}
		if (Extent.x < -32767) {Extent.x = -32767; Flag = False;}
		if (Extent.x > 32767) {Extent.x = 32767; Flag = False;}
		if (Start.y < -32767) {Start.y = -32767; Flag = False;}
		if (Start.y > 32767) {Start.y = 32767; Flag = False;}
		if (Extent.y < -32767) {Extent.y = -32767; Flag = False;}
		if (Extent.y > 32767) {Extent.y = 32767; Flag = False;}
		ShortOne->left = Start.x;
		ShortOne->right = Extent.x;
		ShortOne->top = Start.y;
		ShortOne->bottom = Extent.y;
		return Flag;
	}


/****************************************************************/
/* Non-window object utility routines */


Rect			RectOf(short x1, short y1, short x2, short y2)
	{
		Rect	Temp;

		Temp.left = x1;
		Temp.top = y1;
		Temp.right = x2;
		Temp.bottom = y2;
		return Temp;
	}


Point			PointOf(short x, short y)
	{
		Point	Temp;

		Temp.v = y;
		Temp.h = x;
		return Temp;
	}


LongPoint	LongPointOf(long x, long y)
	{
		LongPoint	Temp;

		Temp.x = x;
		Temp.y = y;
		return Temp;
	}


/* get a Rect from the resource file. */
/* high word of RectID = ID of resource */
/* low word of RectID = ID of rect within that resource */
/* the resources are expected to be in big-endian format.  This code */
/* should work even on little-endian processors */
void		GetRect(ulong RectID, LongPoint* Start, LongPoint* Extent)
	{
		Handle		RectRes;
		char*			RectList;
		ulong			Scan;  /* scans the list by byte */
		ushort		Count;
		ushort		IDToFind;
		LongPoint	Temp;

		ERROR(ResLoad == 0,PRERR(ForceAbort,"Automatic resource loading is disabled."));
		RectRes = GetResource('�Rct',(RectID >> 16) & 0x0000ffff);
		if (RectRes == NIL)
			{
				PRERR(ForceAbort,"GetRect couldn't load resource.");
			}
		ERROR(ResErr != noErr,PRERR(ForceAbort,"Resource Error occurred."));
		HLock(RectRes);
		RectList = *RectRes;  /* dereference */
		IDToFind = 0x0000ffff & RectID;  /* isolate local ID */
		Scan = 2;  /* start with the 2nd word */
		/* getting number of items to do (big-endian) */
		Count = ((uchar*)RectList)[1] + (256 * ((uchar*)RectList)[0]);
		while (Count > 0)
			{
				if ((((uchar*)RectList)[Scan] * 256) + (((uchar*)RectList)[Scan + 1])
					== IDToFind)
					{
						char*			BaseAddr;

						/* we've found it--copy it out */
						BaseAddr = (char*)((((StrLen((char*)(RectList + Scan + sizeof(short))))
							& (~1)) + 2) + (char*)RectList +	Scan + sizeof(short));
						HUnlock(RectRes);
						(*Start).x = ((signed char)*(BaseAddr++) * 256) + (uchar)*(BaseAddr++);
						(*Start).y = ((signed char)*(BaseAddr++) * 256) + (uchar)*(BaseAddr++);
						(*Extent).x = ((signed char)*(BaseAddr++) * 256) + (uchar)*(BaseAddr++);
						(*Extent).y = ((signed char)*(BaseAddr++) * 256) + (uchar)*(BaseAddr++);
						return;
					}
				/* skip to next item */
				Scan +=
					((   (StrLen((char*)(RectList + Scan + sizeof(short))))
					& (~1)) + 2) + sizeof(short) + 4*sizeof(short);
				Count -= 1;  /* decrement the counter */
			}
		HUnlock(RectRes);
		PRERR(ForceAbort,"GetRect couldn't find rectangle in resource.");
	}


MyBoolean	LongPtInRect(LongPoint ThePoint, LongPoint Start, LongPoint Extent)
	{
		if (ThePoint.x < Start.x) return False;
		if (ThePoint.x >= Start.x + Extent.x) return False;
		if (ThePoint.y < Start.y) return False;
		if (ThePoint.y >= Start.y + Extent.y) return False;
		return True;
	}


/* returns new Start */
LongPoint	CenterRect(LongPoint Extent, LongPoint ContainerExtent)
	{
		LongPoint	NewStart;

		NewStart.x = (ContainerExtent.x - Extent.x) / 2;
		NewStart.y = (ContainerExtent.y - Extent.y) / 3;
		if (NewStart.y < 20 + GetMBarHeight())
			{
				NewStart.y = 20 + GetMBarHeight();
			}
		return NewStart;
	}


LongPoint	AlertCenterRect(LongPoint Extent, LongPoint ContainerExtent)
	{
		LongPoint	NewStart;

		NewStart.x = (ContainerExtent.x - Extent.x) / 2;
		NewStart.y = (ContainerExtent.y / 3) - Extent.y;
		if (NewStart.y < 40)
			{
				NewStart.y = 40;
			}
		return NewStart;
	}


LongPoint	GetLongMouseLoc(void)
	{
		Point				MouseLoc;
		LongPoint		Result;

		GetMouse(&MouseLoc);
		LocalToGlobal(&MouseLoc);
		Result.x = MouseLoc.h;
		Result.y = MouseLoc.v;
		return Result;
	}


short			GetFontID(PString FontName)
	{
		short			FontID;

		GetFNum(FontName,&FontID);
		return FontID;
	}


void			SectLongRect(LongPoint Start1, LongPoint Extent1, LongPoint Start2,
						LongPoint Extent2, register LongPoint* S, register LongPoint* E)
	{
		/* normalize the rectangles */
		if (Extent1.x < 0)
			{
				Start1.x += Extent1.x;
				Extent1.x = -Extent1.x;
			}
		if (Extent1.y < 0)
			{
				Start1.y += Extent1.y;
				Extent1.y = -Extent1.y;
			}
		if (Extent2.x < 0)
			{
				Start2.x += Extent2.x;
				Extent2.x = -Extent2.x;
			}
		if (Extent2.y < 0)
			{
				Start2.y += Extent2.y;
				Extent2.y = -Extent2.y;
			}
		/* constrain topleft corner */
		if (Start1.x > Start2.x)
			{
				S->x = Start1.x;
			}
		 else
			{
				S->x = Start2.x;
			}
		if (Start1.y > Start2.y)
			{
				S->y = Start1.y;
			}
		 else
			{
				S->y = Start2.y;
			}
		/* constrain bottomright corner */
		if (Start1.x+Extent1.x > Start2.x+Extent2.x)
			{
				E->x = (Start2.x+Extent2.x) - S->x;
			}
		 else
			{
				E->x = (Start1.x+Extent1.x) - S->x;
			}
		if (Start1.y+Extent1.y > Start2.y+Extent2.y)
			{
				E->y = (Start2.y+Extent2.y) - S->y;
			}
		 else
			{
				E->y = (Start1.y+Extent1.y) - S->y;
			}
	}


LongPoint	MainScreenSize(void)
	{
		return LongPointOf(screenBits.bounds.right - screenBits.bounds.left,
			screenBits.bounds.bottom - screenBits.bounds.top);
	}


PicHandle	GetPic(short PicID)
	{
		PicHandle		Temp;

		ERROR(ResLoad == 0,PRERR(ForceAbort,"Automatic resource loading is disabled."));
		Temp = (PicHandle)GetResource('PICT',PicID);
		ERROR(Temp==NIL,PRERR(ForceAbort,"GetPic: Couldn't find PICT"));
		ERROR(ResErr != noErr,PRERR(ForceAbort,"Resource Error occurred."));
		return Temp;
	}
