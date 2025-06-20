/*
StringBounds.c

	void StringBounds(unsigned char *s,Rect *bounds,long *count);
Counts, and computes the minimum bounding rectangle for, all the pixels that
would be set black by calling DrawString with the given pascal string, in the
current port, font, size, and style. (Assumes ForeColor is black and that
TextMode is srcOr, which are the default settings.) Coordinates in the *bounds
rect are relative to the current pen position. If no pixels would be set then
*bounds is set to the empty rect (0,0,0,0). The current port is left untouched.

The bounds and count arguments are optional; they may be replaced by NULL.

	void CharBounds(char a,Rect *bounds,long *count);
Creates a one-char string and calls StringBounds().

NOTES

The string argument "s" is a Pascal string, because that seems most convenient to
the caller, who will be almost immediately calling DrawString.

bounds->right need not exactly equal the value returned by StringWidth(s),
because StringWidth returns the displacement of the pen position, and some (e.g.
italic) characters extend beyond that, while others (e.g. comma) extend less
far. Similarly, bounds->left need not always be zero.

By QuickDraw's convention, each pixel is considered to lie below and to the
right of the point that is used to address it. The bounding rectangle for a
pixel at x,y, is SetRect(&r,x,y,x+1,y+1).

HISTORY:
1/17/94 dgp wrote it, based on discussion with Bart Farell and Manoj Gunwani.
2/22/94	dgp	added count argument, and made both bounds and count optional.
2/27/94	dgp since this routine is typically called repeatedly I save time
			(0.2 s per call) by only allocating the GWorld once, and never
			freeing it. It is reused on subsequent calls, resized appropriately
			if necessary.
*/
#include <VideoToolbox.h>
void CharBounds(char a,Rect *bounds,long *count);
void StringBounds(unsigned char *s,Rect *bounds,long *count);

void CharBounds(char a,Rect *bounds,long *countPtr)
{
	unsigned char string[]="\pA";
	
	string[1]=a;
	StringBounds(string,bounds,countPtr);
}

void StringBounds(unsigned char *s,Rect *boundsPtr,long *countPtr)
{
	static GWorldPtr our=NULL;
	GWorldPtr old;
	GDHandle oldDevice;
	FontInfo f;
    Rect r;
	register unsigned long *pix,blank;
	register int x,y;
	register long count=0;
	int n;
	OSErr error;
	Rect bounds;

	// draw string into a new GWorld
	GetFontInfo(&f);
	SetRect(&r,0,-f.ascent,StringWidth(s),f.descent);	// nominal size
	InsetRect(&r,-(f.widMax+2),-(f.leading+2)); 		// add margin

	if(our==NULL)
		error=NewGWorld(&our,1,&r,NULL,NULL,0);
	else
		error=UpdateGWorld(&our,1,&r,NULL,NULL,0);
	if(error)PrintfExit("StringBounds: NewGWorld/UpdateGWorld error %d.\n",error);
	GetGWorld(&old,&oldDevice);
	SetGWorld(our,NULL);
	TextFace(old->txFace);
	TextFont(old->txFont);
	TextSize(old->txSize);
	EraseRect(&our->portRect);
	MoveTo(0,0);
	DrawString(s);
	SetGWorld(old,oldDevice);
	
	// measure the bounding box
	r=our->portRect;
	SetRect(&bounds,r.right,r.bottom,r.left,r.top);
	n=r.right-r.left;
	pix=(unsigned long *)malloc(n*sizeof(*pix));
	if(pix==NULL)PrintfExit("StringBounds: Couldn't allocate %ld bytes.\n"
		,n*sizeof(long));
	SetGWorld(our,NULL);
	OffsetRect(&bounds,-r.left,-r.top);
	for(y=0;y<r.bottom-r.top;y++){
		GetPixelsQuickly(r.left,r.top+y,pix,n);
		if(y==0)blank=pix[0];
		for(x=n-1;x>=0;x--)if(pix[x]!=blank){
			count++;
			if(x<bounds.left)bounds.left=x;
			if(x>=bounds.right)bounds.right=x+1;
			if(y<bounds.top)bounds.top=y;
			if(y>=bounds.bottom)bounds.bottom=y+1;
		}
	}
	OffsetRect(&bounds,r.left,r.top);
	SetGWorld(old,oldDevice);
	if(EmptyRect(&bounds))SetRect(&bounds,0,0,0,0);
	free(pix);	
//	DisposeGWorld(our);
	if(boundsPtr!=NULL)*boundsPtr=bounds;
	if(countPtr!=NULL)*countPtr=count;
}
