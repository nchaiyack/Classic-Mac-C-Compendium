/* a commented version of this can be found in the document */
/* 'Tutorial, which you should have received with this code */

#include <QDOffscreen.h>

void InitToolBox(void);
void main(void);
void MarksMaskBlitter(long *, long , long *, long , long *, long , long *);

void InitToolBox()
{
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	InitCursor();
	TEInit();
	InitDialogs(nil);
	MaxApplZone();
	MoreMasters();
}

void main()
{
	GrafPort killMenuBar;
	WindowPtr theWindow;
	CWindowRecord myCWindowRecord;
	short i, frame, x, y;

	PixMapHandle iconPictPixMap, backdropPictPixMap;
	Byte *myCWindowMemPtr, *iconPictMemPtr, *backdropPictMemPtr, *maskMemPtr;
	long myCWindowRowOffSet, iconPictRowOffSet, backdropPictRowOffSet;

	GWorldPtr iconPictGWorld, backdropPictGWorld;

	PicHandle myPict, myBackdrop;

	Rect tempRect, fullScreenRect;

	CGrafPtr winPort;
	GDHandle winDevice;

	SignedByte mmuMode;

/* uncomment the next line (and some later) to slow things down */

/* long L; */

	InitToolBox();
	SetRect(&tempRect, 0,0,512,64);

/* this should be changed if you have a different screen size */

	SetRect(&fullScreenRect, 0, 0, 640, 480);

	myPict = GetPicture (128);
	myBackdrop = GetPicture(228);
	
	theWindow = NewCWindow(&myCWindowRecord, &fullScreenRect, nil,
								true, plainDBox, nil, false, 0L);
	
	OpenPort ((GrafPtr)&killMenuBar);
	EraseRect (&killMenuBar.portRect);

/* draw the backdrop picture in it */

	DrawPicture(myBackdrop, &fullScreenRect);
	
	GetGWorld(&winPort, &winDevice);

	NewGWorld(&backdropPictGWorld, 8, &fullScreenRect, nil, nil, 0L);
	backdropPictPixMap = GetGWorldPixMap(backdropPictGWorld);
	LockPixels(backdropPictPixMap);
	SetGWorld(backdropPictGWorld, nil);
	EraseRect(&(backdropPictGWorld->portRect));

	DrawPicture(myBackdrop, &fullScreenRect);
		
	NewGWorld(&iconPictGWorld, 8, &tempRect, nil, nil, 0L);
	iconPictPixMap = GetGWorldPixMap(iconPictGWorld);
	LockPixels(iconPictPixMap);
	SetGWorld(iconPictGWorld, nil);
	EraseRect(&(iconPictGWorld->portRect));
	DrawPicture(myPict, &tempRect);

/* reset the port and device to what they were before */

	SetGWorld(winPort, winDevice);

	myCWindowMemPtr = (Byte *)(GetPixBaseAddr(myCWindowRecord.port.portPixMap));
	myCWindowRowOffSet = (0x7FFF & (**myCWindowRecord.port.portPixMap).rowBytes);
	
	backdropPictMemPtr = (Byte *)(GetPixBaseAddr(backdropPictPixMap));
	backdropPictRowOffSet = (0x7FFF & (**backdropPictPixMap).rowBytes);
	
	iconPictMemPtr = (Byte *)(GetPixBaseAddr(iconPictPixMap));
	iconPictRowOffSet = (0x7FFF & (**iconPictPixMap).rowBytes);


/* set up a loop for timing purposes */
	
	y = 150;
	frame = 0;
	
	while (!Button())		/* wait for mouse press to help timing */
	{}

/* do the whole thing 100 times */
	
	for (i = 0; i < 100; i++)
	{

/* move the sprite from 0 to 599 horizontally, one pixel at a time */

		for (x = 0; x < 600; x++)
		{

/* uncomment for delay loop */

/* L = TickCount(); */


			maskMemPtr = iconPictMemPtr + (32 * iconPictRowOffSet);

/* for 24 bit compatibility add the next 2 lines */

	mmuMode = true32b;
	SwapMMUMode (&mmuMode);

	MarksMaskBlitter((long *)(backdropPictMemPtr + (y*backdropPictRowOffSet)+x),
					backdropPictRowOffSet/4, (long *)(iconPictMemPtr + (32 * frame)),
					iconPictRowOffSet/4, (long *)(myCWindowMemPtr +
					(y*myCWindowRowOffSet)+x),
					myCWindowRowOffSet/4, (long *)(maskMemPtr + (32 * frame)));

	SwapMMUMode(&mmuMode);

	frame = (++frame) %8;

/* uncomment for delay loop */

/* while (L > TickCount() -2L); */
			
		}
	}

/* wait for the mouse button to be clicked before exiting */

	while (!Button())
	{}
}



/* the actual mask blitter */
/* now all 'MemPtr's point to 'long's for speed on blitting */

void MarksMaskBlitter(long *backdropPictMemPtr, long backdropPictRowOffSet,
					long *iconPictMemPtr, long iconPictRowOffSet,
					long *myCWindowMemPtr, long myCWindowRowOffSet,
					long *maskMemPtr)
{
	short i;

/* 32 rows to blit. Change this for other heights of sprite */
	
	for (i = 0; i<= 31; i++)
	{
		*myCWindowMemPtr++ = (*backdropPictMemPtr++ & *maskMemPtr++) | *iconPictMemPtr++;
		*myCWindowMemPtr++ = (*backdropPictMemPtr++ & *maskMemPtr++) | *iconPictMemPtr++;
		*myCWindowMemPtr++ = (*backdropPictMemPtr++ & *maskMemPtr++) | *iconPictMemPtr++;
		*myCWindowMemPtr++ = (*backdropPictMemPtr++ & *maskMemPtr++) | *iconPictMemPtr++;
		*myCWindowMemPtr++ = (*backdropPictMemPtr++ & *maskMemPtr++) | *iconPictMemPtr++;
		*myCWindowMemPtr++ = (*backdropPictMemPtr++ & *maskMemPtr++) | *iconPictMemPtr++;
		*myCWindowMemPtr++ = (*backdropPictMemPtr++ & *maskMemPtr++) | *iconPictMemPtr++;
		*myCWindowMemPtr++ = (*backdropPictMemPtr++ & *maskMemPtr++) | *iconPictMemPtr++;

		myCWindowMemPtr += myCWindowRowOffSet - 8;
		backdropPictMemPtr += backdropPictRowOffSet - 8;
		maskMemPtr += iconPictRowOffSet - 8;
		iconPictMemPtr += iconPictRowOffSet - 8;
	}
}
