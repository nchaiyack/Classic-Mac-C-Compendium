/*/
     Project Arashi: VA.h
     Major release: Version 1.1, 7/22/92

     Last modification: Thursday, July 8, 1993, 23:25
     Created: Sunday, February 5, 1989, 17:24

     Copyright � 1989-1993, Juri Munkki
/*/

#define	NUMDISPLISTS		(3)

#define	ANGLES				120
#define	NUMCOLORS			10		/*	Currently 10 colors (3 background+7 animated)	*/

#define	MAXSAVED			1000L
#define MAXSAVEDPIX			1500L
#define	DEFAULT_FRAMESPEED	3
#define	QD32COMPATIBLE		0x0101

#define BGC					255
#define	BG2					(255-64)
#define	BG1					(255-128)

#define	VAPosRandom()		((unsigned int)VARandom())
#define	FastPosRandom()		((unsigned int)FastRandom())

typedef	struct
{
	WindowPtr		window;
	int				monochrome;
	Handle			palette;
	Handle			device;
	int				refnum;

	Ptr				base;
	int				row;
	Ptr				*quickrow;

	Rect			frame;
	Rect			DisplayFrame;

	long			offset;
	unsigned int	color;
	unsigned int	field;
	unsigned int	segmscale;
	int				curbuffer;	/*	Buffer to draw into.			*/
	int				visbuffer;	/*	Visible frame buffer number.	*/

	Rect			*lines[NUMDISPLISTS];
	int				*linecolors[NUMDISPLISTS];
	int				numlines[NUMDISPLISTS];
	
	long			*pix[NUMDISPLISTS];
	int				*pixcolors[NUMDISPLISTS];
	int				numpix[NUMDISPLISTS];
	
	long			*spots[NUMDISPLISTS];
	int				*spotcolors[NUMDISPLISTS];
	int				numspots[NUMDISPLISTS];

	unsigned long	ticker;
	int				FrameSpeed;
	int				FrameCounter;
	int				SkipFrames;
	int				Late;
	int				SkipCounter;
	int				CurrentX;
	int				CurrentY;

}	VAGlobalsRecord;


#define	VAMoveTo(x,y) {	VA.CurrentX=(x); VA.CurrentY=(y); }
#define	VApalette	((PaletteHandle)VA.palette)
#ifdef VECTOR_ANIMATION_MAIN
		VAGlobalsRecord		VA;
		int					Sins[ANGLES];
		int					Cosins[ANGLES];
		long				VARandSeed;
#else
extern	VAGlobalsRecord		VA;
extern	int					Sins[];
extern	int					Cosins[];
extern	long				VARandSeed;
#endif

void VASetColors(Handle theColors);
Handle VAGetColors();
void VAEraseBuffer();
void VALine(int x1, int y1, int x2, int y2);
void VALineTo(int x, int y);
void VASafeLineTo(int x, int y);
void VAPixel(int,int);
void VASafePixel(int x, int y);
void VASpot(int,int);
void VASafeSpot(int,int);
void VADrawNumber(long num, int x, int y);
void VADrawPadNumber(long num, int x, int y, int width);
void VAExplosion(int x, int y, int size, int color);
void VAStaticLine(int,int,int,int);
void VAStaticLineTo(int,int);
void VAStep(void);
void VAClose(void);
Handle	ScreenSelect();
int	 VAClip(Rect *ptrec);
void VADrawText(char *,long,long);
void *VAColorToQD(int);
