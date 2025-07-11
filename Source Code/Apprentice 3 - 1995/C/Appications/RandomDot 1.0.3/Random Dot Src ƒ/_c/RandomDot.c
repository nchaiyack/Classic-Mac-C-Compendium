/* RandomDot.c
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */
#include "RandomDotMain.h"
#include "RandomDotRes.h"

#include "RandomDot.h"
#include "Progress.h"

#define round(X)	((int) ((X) + 0.5))
#define DPI			72				/* assume 72 pixels per inch */
#define	E			round(2.5*DPI)	/* eye separation is assumed to be 2.5 inches */
#define mu			(1/3.0)			/* depth of field (fraction of viewing dist.) */
#define separation(Z)	round((1-mu*Z)*E/(2-mu*Z))	/* stereo sep corresponding to position z */
#define farPlane	separation(0)	/* the far plane, z = 0 */

#define kRadius		5


/* DrawCircle - draw a circle, centered at x, y. radius 
 */
static void DrawCircle(Integer x, Integer y){
	Rect	r;

	r.left  = x - kRadius;
	r.right = x + kRadius;
	r.top   = y - kRadius;
	r.bottom= y + kRadius;
	PaintOval(&r);
}


/* ComputeAutoStereogram - See the October 1994 issue of Computer for more info.
	Note: there is a bug in System 7.1 with the LaserWriter driver such that if
		you CopyBits() to the printer, with an 8-bit indexed offscreen gworld
		as the source, the first and last entries of the color table of the
		offscreen gworld get set so the first is white and the last is black.
		So, we follow that scheme in this program: for an 8-bit pixmap index,
		the gray scale value is the complement of the index.
		Watch out, expressions evaluate to ints in C, so ~(unsigned char) 1
		if 0xFFFE not 0xFE
 */
OSErr ComputeAutoStereogram(GWorldPtr src, GWorldPtr dest, Boolean isGray){
	OSErr			errCode;
	Str255			buf;
	CGrafPtr		savePort;
	GDHandle		saveGD;
	PixMapHandle	pmSrc;
	PixMapHandle	pmDest;
	LongInt			y, yMax;
	LongInt			x, xMax;
	Integer			sameBuffer[512];
	Integer			*same;
	Integer			s;
	Integer			left, right;
	unsigned char	*srcP, *destP;
	unsigned char	colorBuffer[512];
	unsigned char	*color;
	Boolean			visible;
	LongInt			t;
	float			z;
	float			zt;
	Integer			k;
	
	GetIndString(buf, kMainStrs, kComputingStereogramS);
	SetProgressLabelText(buf);

	errCode = noErr;
	same = NIL;
	color = NIL;
	GetGWorld(&savePort, &saveGD);
	pmSrc = GetGWorldPixMap(src);
	LockPixels(pmSrc);
	pmDest = GetGWorldPixMap(dest);
	LockPixels(pmDest);
	SetGWorld(dest, NIL);

	yMax = src->portRect.bottom - src->portRect.top;
	xMax = src->portRect.right - src->portRect.left;
	if(xMax < 512){
		same = sameBuffer;
		color = colorBuffer;
	}else{
		if(NIL == (same = (Integer *) NewPtr(sizeof(Integer) * xMax))){
			errCode = memFullErr;
			goto errSkip;
		}
		if(NIL == (color = (unsigned char *) NewPtr(xMax))){
			errCode = memFullErr;
			goto errSkip;
		}
	}

	for(y = 0 ; y < yMax ; y++){
		SetGWorld(savePort, saveGD);
		if(noErr != (errCode = ShowProgress(y, yMax))){
			goto errSkip;
		}
		SetGWorld(dest, NIL);
		for(x = 0 ; x < xMax ; x++){
			same[x] = x;				/* each pixel is initially linked ot itself */
		}
		srcP = (unsigned char *) GetPixBaseAddr(pmSrc) + y * ((**pmSrc).rowBytes & 0x7FFF);
		destP = (unsigned char *) GetPixBaseAddr(pmDest) + y * ((**pmDest).rowBytes & 0x7FFF);
		for(x = 0 ; x < xMax ; x++){
			z = (0xFF & ~srcP[x])/255.0;
			s = separation(z);
			left = x - (s+(s&y&1))/2;
			right = left + s;
			if(0 <= left && right < xMax){
				t = 1;
				do{
					zt = z + 2*(2 - mu*z)*t/(mu*E);
					visible = (0xFF & ~srcP[x-t])/255.0 < zt && (0xFF & ~srcP[x+t])/255.0 < zt ;
					t++;
				}while(visible && zt < 1.0);
				if(visible){
					for(k = same[left] ; k != left && k != right ; k = same[left]){
						if(k < right){
							left = k;
						}else{
							left = right;
							right = k;
						}
					}
					same[left] = right;
				}
			}
		}
		for( x = xMax - 1 ; x >= 0 ; x--){
			if(same[x] == x){
				if(isGray){
					color[x] = (Random() & 0xFF);	/*Note 1*/	/* free choice, do it randomly */
				}else{
					color[x] = (Random() & 1) ? 0x00 : 0xFF;	/*Note 1*/	/* free choice, do it randomly */
				}
			}else{
				color[x] = color[same[x]];					/* constrained */
			}
			destP[x] = color[x];
		}
	}
	DrawCircle(xMax/2 - farPlane/2, (yMax*19)/20);
	DrawCircle(xMax/2 + farPlane/2, (yMax*19)/20);
errSkip:
	SetGWorld(savePort, saveGD);
	ShowProgress(yMax, yMax);
	if(same != sameBuffer && NIL != same){
		DisposePtr((Ptr) same);
	}
	if(color != colorBuffer && NIL != color){
		DisposePtr((Ptr) color);
	}
	UnlockPixels(pmSrc);
	UnlockPixels(pmDest);
	SetGWorld(savePort, saveGD);
	return errCode;
}

