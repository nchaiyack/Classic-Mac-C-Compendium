/*
PrintfGWorld.c

	void PrintfGWorld(GWorldPtr our);
Uses "printf" to print out the GWorld as a bitmap, representing blank as '.' and
non-blank as '#'. The origin is designated '+'. Uses the upper lefthand pixel
as the definition of blank (a poor but convenient assumption). This is very crude, 
and intended only for debugging.

	void PrintStringAsBitmap(unsigned char *s);
Uses DrawString to render the string in a GWorld, and then calls PrintfGWorld to
render the GWorld onto the console. For debugging.

HISTORY:
1/17/94 dgp wrote it.
*/
#include <VideoToolbox.h>
void PrintfGWorld(GWorldPtr our);
void PrintStringAsBitmap(unsigned char *s);

void PrintfGWorld(GWorldPtr our)
// Print GWorld on the console as a bitmap.
// Assume upper lefthand pixel is blank, which is rendered as '.'. 
// Any other pixel value is represented as '#'.
{
	GWorldPtr old;
	GDHandle oldDevice;
    Rect r;
	register unsigned long *pix,blank;
	register int i,n;
	int x,y;
	char *bit;

	GetGWorld(&old,&oldDevice);
	r=our->portRect;
	r.left=0;
	#if THINK_C
		r.right=console_options.ncols;
	#else
		r.right=72;
	#endif
	CenterRectInRect(&r,&our->portRect);
	SectRect(&r,&our->portRect,&r);
	n=r.right-r.left;
	bit=(char *)malloc((1+n)*sizeof(char));
	if(bit==NULL)PrintfExit("PrintStringAsBitmap: Couldn't allocate %n bytes.\n",n+1);
	bit[n]=0;
	pix=(unsigned long *)malloc(n*sizeof(unsigned long));
	if(pix==NULL)PrintfExit("PrintStringAsBitmap: Couldn't allocate %ld bytes.\n"
		,n*sizeof(long));
	SetGWorld(our,NULL);
	GetPixelsQuickly(our->portRect.left,our->portRect.top,pix,1);
	blank=pix[0];
	for(y=r.top;y<r.bottom;y++){
		SetGWorld(our,NULL);
		GetPixelsQuickly(r.left,y,pix,n);
		SetGWorld(old,oldDevice);
		for(i=0;i<n;i++){
			if(pix[i]!=blank) bit[i]='#';					// non-blank
			else bit[i]='.';								// blank
		}
		if(y==0)bit[-r.left]='+';							// the origin
		printf("%s\n",bit);
	}
	free(pix);
	free(bit);
}

void PrintStringAsBitmap(unsigned char *s)
{
	GWorldPtr our,old;
	GDHandle oldDevice;
	FontInfo f;
    Rect r;
	OSErr error;

	// Draw string into a new GWorld
	GetFontInfo(&f);
	SetRect(&r,0,-f.ascent,StringWidth(s),f.descent);	// nominal size
	InsetRect(&r,-(f.widMax/2+2),-(f.leading+2));		// add margin
	error=NewGWorld(&our,1,&r,NULL,NULL,0);
	if(error)PrintfExit("PrintStringAsBitmap: NewGWorld error %d.\n",error);
	GetGWorld(&old,&oldDevice);
	SetGWorld(our,NULL);
	TextFace(old->txFace);
	TextFont(old->txFont);
	TextSize(old->txSize);
	EraseRect(&our->portRect);
	MoveTo(0,0);
	DrawString(s);
	SetGWorld(old,oldDevice);
	
	PrintfGWorld(our);
	
	DisposeGWorld(our);
}

