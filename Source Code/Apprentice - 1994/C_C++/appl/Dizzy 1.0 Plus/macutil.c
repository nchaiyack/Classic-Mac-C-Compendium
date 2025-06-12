#include "dizzy.h"

#ifdef	MACINTOSH
/*	PictBit reads a picture resource, creates
>>	a large enough bitmap and draws the picture
>>	into it. The Bits bitmap is supplied to the
>>	routine. Space for the actual bits is reserved
>>	with NewPtr. Be sure to deallocate it once
>>	it is no longer needed!
>>
>>	No error checking is made.
*/
void	PictBit(Bits,PictId)
BitMap	*Bits;
int 	PictId;
{
	GrafPort	AnyPort;
	GrafPtr 	SavedPort;
	PicHandle	ThePic;
	Rect		TempRect;
	long		RAMNeeded;
	
	GetPort(&SavedPort);
	OpenPort(&AnyPort);
	
	ThePic=(PicHandle)GetResource('PICT',PictId);
	TempRect=(*ThePic)->picFrame;

	OffsetRect(&TempRect,-TempRect.left,-TempRect.top);
	Bits->bounds=TempRect;

	Bits->rowBytes=((TempRect.right + 15) >> 4) << 1;	/*	Round to word boundary	*/
	RAMNeeded=Bits->rowBytes*(long)TempRect.bottom; 	/*	Calculate RAM for bits	*/
	Bits->baseAddr=NewPtr(RAMNeeded);
	
	SetPortBits(Bits);
	AnyPort.portRect=TempRect;
	RectRgn(AnyPort.visRgn,&TempRect);
	RectRgn(AnyPort.clipRgn,&TempRect);

	EraseRect(&TempRect);
	DrawPicture(ThePic,&TempRect);

	ReleaseResource(ThePic);
	ClosePort(&AnyPort);
	SetPort(SavedPort);
}

/*
>>	Gets the mouse position while in a tracking loop.
>>	Returns true when the mouse button comes up.
*/

int GetMouseTrackEvent(pt)
Point	*pt;
{
	int 	r=0;
	
	r=GetNextEvent(mUpMask+mDownMask,&MyEvent);
	*pt=MyEvent.where;
	GlobalToLocal(pt);
	
	if(r)	r=(MyEvent.what==mouseUp);
	return !r;
}

void	GetMouseDownPoint(pt)
Point	*pt;
{
	*pt=MyEvent.where;
	GlobalToLocal(pt);
}

void	DoInits()
{
	char	StackSpace[32767];

	InitGraf(&thePort);
	InitCursor();
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	MaxApplZone();
}

int GetDownButton()
{
	int 	WhichButton=0;

	if(MyEvent.modifiers & optionKey)
	{	WhichButton=1;
	}
	else if(MyEvent.modifiers & cmdKey)
	{	WhichButton=2;
	}
	
	return	WhichButton;
}

void	DoHandScroller()
{
	BitMap		EditBuffer;
	GrafPort	TempClipper;
	Rect		dest;
	Point		StartSpot,OldSpot,MousePoint;
	RgnHandle	Eraser;
	int 		downflag;

	ClipEditArea();
	SetCursor(*GetCursor(128));
	
	EditBuffer.bounds=EditClipper;
	EditBuffer.rowBytes=((EditClipper.right-EditClipper.left + 15) >> 4) << 1;
	EditBuffer.baseAddr=NewPtrClear(EditBuffer.rowBytes*(long)(EditClipper.bottom-EditClipper.top));
	OpenPort(&TempClipper);
	SetPort(&TempClipper);
	SetPortBits(&EditBuffer);
	RectRgn(TempClipper.visRgn,&EditBuffer.bounds);
	UpdateSim();
	ClosePort(&TempClipper);
	SetPort(MyWind);

	GetMouseDownPoint(&StartSpot);
	OldSpot=StartSpot;

	Eraser=NewRgn();
	do
	{	downflag=GetMouseTrackEvent(&MousePoint);
		
		if(MousePoint.h!=OldSpot.h || MousePoint.v != OldSpot.v)
		{	dest=EditClipper;
			OldSpot=MousePoint;
			OffsetRect(&dest,OldSpot.h-StartSpot.h,OldSpot.v-StartSpot.v);
			CopyBits(&EditBuffer,&MyWind->portBits,&EditBuffer.bounds,&dest,srcCopy,0L);

			OpenRgn();
			FrameRect(&EditClipper);
			FrameRect(&dest);
			CloseRgn(Eraser);
			EraseRgn(Eraser);
		}
	}	while(downflag);
	
	CurHeader->XOrig-=OldSpot.h-StartSpot.h;
	CurHeader->YOrig-=OldSpot.v-StartSpot.v;
	
	InvalRgn(Eraser);
	DisposeRgn(Eraser);
	DisposPtr(EditBuffer.baseAddr);
	InitCursor();
	RestoreClipping();
}
#endif
