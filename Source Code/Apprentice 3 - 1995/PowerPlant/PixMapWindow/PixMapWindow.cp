#include "UGWorld.h"
#include "PixMapWindow.h"

PixMapWindow* PixMapWindow::CreatePixMapWindowStream(LStream	*inStream)
{
	 return (new PixMapWindow(inStream));
}

PixMapWindow::PixMapWindow(LStream *inStream)
	: LWindow(128, windAttr_Regular + windAttr_CloseBox + windAttr_TitleBar, nil)
{
	Rect	frame;
	CalcLocalFrameRect(frame);
	mGWorld = new LGWorld(frame, 0);
	mScaleToWindow = 0;
	SetDescriptor((ConstStr255Param)"\pPixMapWindow");
	Show();
	Select();
}

PixMapWindow::PixMapWindow(LGWorld *inGWorld, Rect frame, Boolean inScale)
	: LWindow(128, windAttr_Regular + windAttr_CloseBox + windAttr_TitleBar, nil)
{
	mGWorld = inGWorld;
	SetDescriptor((ConstStr255Param)"\pPixMapWindow");
	SendAESetBounds(&frame, 1);
	mScaleToWindow = inScale;
	Show();
	Select();
}

PixMapWindow::PixMapWindow(LGWorld *inGWorld, Boolean inScale)
	: LWindow(128, windAttr_Regular + windAttr_CloseBox + windAttr_TitleBar, nil)
{
	Rect	frame;

	mGWorld = inGWorld;
	SetDescriptor((ConstStr255Param)"\pPixMapWindow");
	frame = mGWorld->GetMacGWorld()->portRect;
	OffsetRect(&frame,	50-(mGWorld->GetMacGWorld()->portRect.top),
						50-(mGWorld->GetMacGWorld()->portRect.left));
	SendAESetBounds(&frame, 1);
	mScaleToWindow = inScale;
	Show();
	Select();
}

void
PixMapWindow::SetmGWorld(LGWorld *theGWorld)
{
	Rect frame;

	if (mGWorld != NULL)
		delete mGWorld;
	mGWorld = theGWorld;
	if (theGWorld == NULL)
		ResizeImageTo(0, 0, false);
	else {
		frame = theGWorld->GetMacGWorld()->portRect;
		ResizeImageTo(	frame.right - frame.left,
						frame.bottom - frame.top, false);
	}
}

LGWorld *
PixMapWindow::GetmGWorld()
{
	return mGWorld;
}

PixMapWindow::~PixMapWindow()
{
	delete mGWorld;
}

void
PixMapWindow::DrawSelf()
{
	if (mGWorld != NULL) {
		Rect theBounds;
		
		if (mScaleToWindow)
			CalcLocalFrameRect(theBounds);
		else {
			theBounds = mGWorld->GetMacGWorld()->portRect;
			OffsetRect(&theBounds, 0-theBounds.left, 0-theBounds.top);
		}
		
		ForeColor(blackColor);
		BackColor(whiteColor);
		mGWorld->CopyImage(GetMacPort(), theBounds);
	}
}
