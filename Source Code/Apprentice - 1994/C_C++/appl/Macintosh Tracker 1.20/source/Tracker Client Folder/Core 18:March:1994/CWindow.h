/* CWindow.h */

#pragma once

#include "CEnclosure.h"

class CSack;

#define JustifyLeft (teJustLeft)
#define JustifyCenter (teJustCenter)
#define JustifyRight (teJustRight)

#define ModelessWindow (False)
#define ModalWindow (True)
#define YesGrowable (True)
#define NoGrowable (False)
#define YesZoomable (True)
#define NoZoomable (False)

#define GrowIconPictID (139)
#define EmptyGrowIconPictID (140)

struct	CWindow	:	CEnclosure
	{
		GrafPtr				MyGrafPtr;
		LongPoint			DrawingOrigin;
		MyBoolean			Modality;
		MyBoolean			Growable;
		MyBoolean			Zoomable;
		LongPoint			ClipStart;
		LongPoint			ClipExtent;
		CViewRect*		OurKeyReceiver;
		CSack*				ListOfWindowClients;
		short					ZoomStatePartCode;
		EXECUTE(MyBoolean Initialized;)

		/* */			~CWindow();
		/* */			CWindow();
		void			IWindow(LongPoint Start, LongPoint Extent, MyBoolean Modality,
								MyBoolean GrowFlag, MyBoolean ZoomFlag);
		void			MakeMyGrafPtr(LongPoint Start, LongPoint Extent);
		void			DeleteMyGrafPtr(void);
		void			DoUpdate(void);
		void			DoSuspend(void);
		void			DoResume(void);
		Rect			GetConstraint(void);
		LongPoint	SnapToGrid(LongPoint NewWindowSize);
		void			DoGoAway(Point GlobalStart);
		void			GoAway(void);
		void			DoZoomIn(Point GlobalStart);
		void			DoZoomOut(Point GlobalStart);
		void			ZoomIn(void);
		void			ZoomOut(void);
		void			DoMouseDown(MyEventRec Event);
		void			DoGrow(Point GlobalStart);
		void			SetNewSize(LongPoint NewWindowSize);
		void			DoDrag(Point GlobalStart);
		void			DoFindPosition(void);
		void			BecomeActiveWindow(void);
		void			SetDefaultKeyReceiver(CViewRect* TheReceiver);
		void			RegisterClient(CViewRect* Client);
		void			DeregisterClient(CViewRect* Client);

		/* video stuff */
		void			SetMyPort(void);
		void			InvalidateLong(LongPoint Start, LongPoint Extent);
		void			ScrollLong(LongPoint Start, LongPoint Extent, LongPoint Change);
		void			ScrollLongAndInvalidate(LongPoint Start, LongPoint Extent, LongPoint Change);
		void			SetOrigin(LongPoint TheOrigin);
		void			SetClipRect(LongPoint Start, LongPoint Extent);
		void			ResetPen(void);
		void			SetPen(short Width, short Height, short Mode, Pattern ThePattern);
		void			SetPenMode(short Mode);
		void			SetPattern(Pattern ThePattern);
		void			SetPenSize(short Width, short Height);
		void			SetGreyishTextOr(void);
		void			DrawLine(LongPoint Start, LongPoint Extent);
		void			SetText(short FontID, Style Face, short Mode, short PointSize, Fixed Space);
		void			SetTextMode(short Mode);
		void			LTextBox(LongPoint Start, LongPoint Extent, Handle Text, short Justification);
		void			LDrawText(LongPoint Start, LongPoint Extent, Handle Text, short Justification);
		void			LDrawTextPtr(LongPoint Start, LongPoint Extent, char* Text, long Length, short Justification);
		long			LMeasureText(char* Text, long NumChars);
		MyBoolean	RectVisible(LongPoint Start, LongPoint Extent);
		MyBoolean	LongRectToShort(LongPoint Start, LongPoint Extent, Rect* ShortOne);
		void			LFrameRect(LongPoint Start, LongPoint Extent);
		void			LPaintRect(LongPoint Start, LongPoint Extent);
		void			LEraseRect(LongPoint Start, LongPoint Extent);
		void			LFrameOval(LongPoint Start, LongPoint Extent);
		void			LPaintOval(LongPoint Start, LongPoint Extent);
		void			LEraseOval(LongPoint Start, LongPoint Extent);
		void			LFrameRoundRect(LongPoint Start, LongPoint Extent, short OvalWidth, short OvalHeight);
		void			LPaintRoundRect(LongPoint Start, LongPoint Extent, short OvalWidth, short OvalHeight);
		void			LEraseRoundRect(LongPoint Start, LongPoint Extent, short OvalWidth, short OvalHeight);
		void			LDrawPicture(PicHandle ThePicture, LongPoint Start, LongPoint Extent);
	};

#ifndef COMPILING_CWINDOW_C
	extern CWindow*	ActiveWindow;
	extern LongPoint ZeroPoint;
#endif

Rect			RectOf(short x1, short y1, short x2, short y2);
Point			PointOf(short x, short y);
LongPoint	LongPointOf(long x, long y);
void			GetRect(ulong RectID, LongPoint* Start, LongPoint* Extent);
MyBoolean	LongPtInRect(LongPoint ThePoint, LongPoint Start, LongPoint Extent);
LongPoint	CenterRect(LongPoint Extent, LongPoint ContainerExtent);
LongPoint	AlertCenterRect(LongPoint Extent, LongPoint ContainerExtent);
void			SectLongRect(LongPoint Start1, LongPoint Extent1, LongPoint Start2,
						LongPoint Extent2, register LongPoint* S, register LongPoint* E);
LongPoint	GetLongMouseLoc(void);
short			GetFontID(PString FontName);
LongPoint	MainScreenSize(void);
PicHandle	GetPic(short PicID);
