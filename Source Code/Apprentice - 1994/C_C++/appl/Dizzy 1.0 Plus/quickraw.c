#include "dizzy.h"
#ifndef MACINTOSH
#include "xstuff.h"

Pixmap	gridpix;

static	int 	CurrentPenMode;
static	char	GrayData[]={0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA};
static	Pixmap	graypix;

void	FrameRect(r)
Rect	*r;
{
	if(NonEmpty(r))
		XDrawRectangle(Disp,Wind,Mode,
						orig_x+(r)->left,
						orig_y+(r)->top,
						(r)->right-(r)->left-1,(r)->bottom-(r)->top-1);
}
void	PaintRect(r)
Rect	*r;
{
	if(NonEmpty(r))
		XFillRectangle(Disp,Wind,Mode,
						orig_x+(r)->left,
						orig_y+(r)->top,
						(r)->right-(r)->left,(r)->bottom-(r)->top);
}

void	SetRect(r,x1,y1,x2,y2)
Rect	*r;
int 	x1,y1,x2,y2;
{
	r->left=x1;
	r->right=x2;
	r->top=y1;
	r->bottom=y2;	
}

void	EraseRect(r)
Rect	*r;
{
	XSetForeground(Disp,Mode,White);
	PaintRect(r);
	XSetForeground(Disp,Mode,Black);
}

void	InvertRect(r)
Rect	*r;
{
	XSetFunction(Disp,Mode,GXxor);
	XSetForeground(Disp,Mode,Black ^ White);
	PaintRect(r);
	XSetFunction(Disp,Mode,CurrentPenMode);
	XSetForeground(Disp,Mode,Black);
}

void	LineTo(x,y)
int 	x,y;
{
	XDrawLine(Disp,Wind,Mode,macpen_x+orig_x,macpen_y+orig_y,x+orig_x,y+orig_y);
	MoveTo(x,y);
}
void	Line(x,y)
int 	x,y;
{
	x+= macpen_x;
	y+= macpen_y;
	XDrawLine(Disp,Wind,Mode,macpen_x+orig_x,macpen_y+orig_y,x+orig_x,y+orig_y);
	MoveTo(x,y);
}

void	InsetRect(r,h,v)
Rect	*r;
int 	h,v;
{
	r->left+=h;
	r->right-=h;
	r->top+=v;
	r->bottom-=v;
}

void	OffsetRect(r,h,v)
Rect	*r;
int 	h,v;
{
	r->left+=h;
	r->right+=h;
	r->top+=v;
	r->bottom+=v;
}

int 	PtInRect(pt,r)
Point	pt;
Rect	*r;
{
	if(r->left>pt.h)	return FALSE;
	if(r->right<=pt.h)	return FALSE;
	if(r->top>pt.v) 	return FALSE;
	if(r->bottom<=pt.v) return FALSE;
	return TRUE;
}

void	DrawText(s,start,len)
char	*s;
long	start;
long	len;
{
	XDrawString(Disp,Wind,Mode,macpen_x+orig_x,macpen_y+orig_y,s+start,len);
	macpen_x+=XTextWidth(MyFont,s+start,len);
}

void	DrawChar(c)
char	c;
{
	XDrawString(Disp,Wind,Mode,macpen_x+orig_x,macpen_y+orig_y,&c,1);
	macpen_x+=XTextWidth(MyFont,&c,1);
}

int 	TextWidth(s,start,len)
char	*s;
long	start;
long	len;
{
	return	XTextWidth(MyFont,s+start,len);
}
/*
>>	Mark a rectangular area as invalid. Causes an expose event
>>	on that area.
*/
void	InvalRect(r)
Rect	*r;
{
	if(NonEmpty(r))
	{	XClearArea(Disp,Wind, r->left+orig_x, r->top+orig_y,
						  r->right-r->left, r->bottom-r->top, TRUE);
	}
}

/*
>>	Create a pixmap from bitmap data.
*/
Pixmap	CreatePixMap(bits,width,height)
char	*bits;
int 	width,height;
{
	Pixmap	bitsis,pixis;
	
	bitsis=XCreateBitmapFromData(Disp,RootWindowOfScreen(XtScreen(Canvas)),
								 bits,width,height);
	pixis=XCreatePixmap(Disp,RootWindowOfScreen(XtScreen(Canvas)),
						width,height,DefaultDepthOfScreen(XtScreen(Canvas)));
	XCopyPlane(Disp,bitsis,pixis,Mode,0,0,width,height,0,0,1);
	XFreePixmap(Disp,bitsis);
	
	return pixis;
}

void	SetOrigin(x,y)
int 	x,y;
{
	orig_x= -x;
	orig_y= -y;
}

void	PenSize(m,n)
int 	m,n;
{
	if(m==1) m=0;	/*	Use HW line drawing, if possible.	*/
	XSetLineAttributes(Disp,Mode,m,LineSolid,CapButt,JoinMiter);
}

void	PenXor()
{
	CurrentPenMode=GXxor;
	XSetFunction(Disp,Mode,GXxor);
}
void	PenCopy()
{
	CurrentPenMode=GXcopy;
	XSetFunction(Disp,Mode,GXcopy);
}

/*
>>	Copy a pixmap portion to our window.
*/
void	PixCopy(pixels,srcx,srcy,dest)
Pixmap	pixels;
int 	srcx,srcy;
Rect	*dest;
{
	XCopyPlane(Disp,pixels,Wind,Mode,srcx,srcy,
				dest->right-dest->left,
				dest->bottom-dest->top,
				dest->left+orig_x,dest->top+orig_y,1);
}
void	MiscPixCopy(srcx,srcy,dest)
int 	srcx,srcy;
Rect	*dest;
{
	PixCopy(MiscPix,srcx,srcy,dest);
}
/*
>>	Find out last position of the mouse cursor.
*/
void	GetMouseDownPoint(pt)
Point	*pt;
{
	pt->h=MyEvent.xbutton.x-orig_x;
	pt->v=MyEvent.xbutton.y-orig_y;
}
/*
>>	Find out which button is/was down.
*/
int 	GetDownButton()
{
	switch(MyEvent.xbutton.button)
	{	case Button1:	return 0;
		case Button2:	return 1;
		case Button3:	return 2;
		default:		return 0;
	}
}


/*
>>	Gets the mouse position while in a tracking loop.
>>	Returns true when the mouse button comes up.
*/
int GetMouseTrackEvent(pt)
Point	*pt;
{
	int 	waitflag;
	
	if(DownTrigger)
	{	GetMouseDownPoint(pt);
		DownTrigger=FALSE;
		return	TRUE;
	}

	XMaskEvent(Disp, ButtonPressMask | ButtonMotionMask | ButtonReleaseMask, &MyEvent);
	waitflag= (MyEvent.type==MotionNotify);
	while(waitflag)
	{	if(XCheckMaskEvent(Disp,ButtonPressMask | ButtonMotionMask |
							 ButtonReleaseMask, &MyEvent))
		{	waitflag= (MyEvent.type==MotionNotify);
		}
		else
		{	waitflag= 0;
		}
	}
	GetMouseDownPoint(pt);

	if(MyEvent.type==ButtonRelease)
		return	FALSE;
	return	TRUE;
}
/*
>>	Use a rectangle for clipping. The rectangle is in local coordinates.
*/
void	ClipRect(r)
Rect	*r;
{
	XRectangle	Clipper;
	
	Clipper.x=r->left+orig_x;
	Clipper.y=r->top+orig_y;
	Clipper.width=r->right-r->left;
	Clipper.height=r->bottom-r->top;
	if(Clipper.width<0) Clipper.width=0;
	if(Clipper.height<0) Clipper.height=0;
	
	XSetClipRectangles(Disp,Mode,0,0,&Clipper,1,Unsorted);
}
/*
>>	Using a stippled pen to draw rectangle outlines and
>>	lines didn't work with MacX and was slow on Sony
>>	News workstations, so I just replaced it with a solid
>>	black. It looks ok.
*/
void	PenGray()	
{
	XSetFillStyle(Disp,Mode,FillSolid);
}
/*
>>	The background still needs to be drawn with a
>>	gray tiled pattern, so I created this mode for
>>	just that purpose. Filling rectangles with a
>>	tile or stipple worked ok even with MacX.
*/
void	PenBGray()
{
	XSetFillStyle(Disp,Mode,FillTiled);
	XSetTile(Disp,Mode,graypix);
	XSetTSOrigin(Disp,Mode,0,0);
}
/*
>>	This tile pattern draws a grid with 64 pixels
>>	between each gridline and lines with just one
>>	pixel out of four drawn. Looks nice.
*/
void	PenGrid()
{
	XSetFillStyle(Disp,Mode,FillTiled);
	XSetTile(Disp,Mode,gridpix);
	XSetTSOrigin(Disp,Mode,orig_x,orig_y);
}

/*
>>	Just the dull black pen to draw most of
>>	the things that need to be drawn. We assume
>>	that the foreground color is already correctly
>>	set.
*/
void	PenBlack()
{
	XSetFillStyle(Disp,Mode,FillSolid);
}
void	InitXGraf()
{	
	int 	i;
	
	SetRect(&NilRect,0,0,0,0);
	Wind=XtWindow(Canvas);
	Disp=XtDisplay(Canvas);

	graypix=CreatePixMap(GrayData,8,8);
	gridpix=XCreatePixmap(Disp,RootWindowOfScreen(XtScreen(Canvas)),
						64,64,DefaultDepthOfScreen(XtScreen(Canvas)));
	
	XSetForeground(Disp,Mode,WhitePixel(Disp,DefaultScreen(Disp)));
	XFillRectangle(Disp,gridpix,Mode,0,0,64,64);
	XSetForeground(Disp,Mode,BlackPixel(Disp,DefaultScreen(Disp)));


	for(i=2;i<64;i+=4)
	{	XDrawPoint(Disp,gridpix,Mode,i,0);
		XDrawPoint(Disp,gridpix,Mode,0,i);
	}
}

/*
>>	This routines handles the scrolling of the drawing area.
>>	It doesn't work quite as nicely as the Macintosh version,
>>	but the basic idea is the same.
*/
void	DoHandScroller()
{
	Rect		dest;
	Pixmap		temp;
	int 		dh,dv;
	Point		StartSpot,OldSpot,MousePoint;
	int 		downflag;

	ClipEditArea();
	
	GetMouseDownPoint(&StartSpot);
	OldSpot=StartSpot;

	do
	{	downflag=GetMouseTrackEvent(&MousePoint);
		
		if(MousePoint.h!=OldSpot.h || MousePoint.v != OldSpot.v)
		{	dest=EditClipper;
			dh=MousePoint.h-OldSpot.h;
			dv=MousePoint.v-OldSpot.v;
			OffsetRect(&dest,dh,dv);
			PixCopy(Wind,EditR.left,EditR.top,&dest);

			dest=EditClipper;
			if(dh<0)	dest.left=dest.right+dh;
			else		dest.right=dest.left+dh;
			EraseRect(&dest);

			dest=EditClipper;
			if(dv<0)	dest.top=dest.bottom+dv;
			else		dest.bottom=dest.top+dv;
			EraseRect(&dest);
			
			OldSpot=MousePoint;
		}
	}	while(downflag);
	
	CurHeader->XOrig-=OldSpot.h-StartSpot.h;
	CurHeader->YOrig-=OldSpot.v-StartSpot.v;
	
	InvalRect(&EditClipper);	/*	Redraw everything.	*/
	RestoreClipping();
}
#endif
