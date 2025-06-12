/*
>>	Dizzy 0.0	Dizzy.h
>>
>>	A digital circuit simulator & design program for the X Window System
>>
>>	Copyright 1990 Juri Munkki, all rights reserved
>>
>>	Please read the included file called "DizzyDoc" for information on
>>	what your rights are concerning this product.
*/

/*	If this program is compiled with Think C, it's probably on a Macintosh. */
#ifdef	THINK_C
#define MACINTOSH
#endif

#ifdef	MAIN_PROGRAM
#define GLOBAL
#else
#define GLOBAL	extern
#endif

#ifdef	MACINTOSH
#define HILITEMODE		asm {	BCLR	#7,0x938	}
#define PenXor()		PenMode(patXor)
#define PenCopy()		PenMode(patCopy)
#define PenGray()		PenPat(gray)
#define PenBlack()		PenPat(black)
#define LCENTERING	2
#define	MAXFILENAME	256						/*	Maximum length of a file name for Mac.	*/
#else
#include "macstuff.h"
#define	MAXFILENAME	2048						/*	Maximum length of a path for unix.	*/
#define LCENTERING	0
#endif

#include "elementdefs.h"

typedef struct
{
	long	Chip;		/*	Usually offset to the output element	*/
	int 	Pin;		/*	Pin number to use for input 			*/

}	Input;

typedef struct
{
	int 	Data;		/*	Output data bits from this pin. 			*/
	int 	RefCount;	/*	Number of references to this output pin.	*/
}	Output;

typedef struct
{	
	long	PrevLength; /*	Length of previous object (or 0, if first)	*/
	long	Length; 	/*	Lenght of this object (or 0, if last)		*/
	long	Type;		/*	Object type magic number (4 chars, really)	*/
	int 	Number; 	/*	Identifying number of object				*/
	int 	Flags;		/*	Flags: private and public.					*/
	int 	PrivData;	/*	Private data for element.					*/
	Rect	Body;		/*	Rectangle containing main body of element	*/
	Rect	InRect; 	/*	Rectangle containing inputs of element		*/
	Rect	OutRect;	/*	Rectangle containing outputs of element 	*/
	int 	Outputs;	/*	Number of outputs							*/
	int 	Inputs; 	/*	Number of inputs							*/
	Output	Out[1]; 	/*	Outputs 									*/	
}	Element;


typedef struct
{
	long	First;		/*	Offset to first element 				*/
	long	Last;		/*	Offset to last element					*/
	int 	XOrig;		/*	X origin (screen coordinates)			*/
	int 	YOrig;		/*	Y origin								*/
	int 	TitleWidth; /*	Width of title in pixels.				*/
	int 	TitleLen;	/*	Length of title in characters.			*/
	int 	PathLen;	/*	Length of whole filename/path in chars. */
}	TableHeader;

typedef struct
{						/*	Function is negative, if NOT is also on.*/
	int 	Function;	/*	1..19. 1 is NOT, 19 is zapper tool. 	*/
	Rect	Prime;		/*	Rectangle of selected button.			*/
	Rect	Secondary;	/*	If Function<0, invert this rect as well.*/
}	ToolSelection;

#define MOUSE_BUTTONS	3		/*	Number of mouse button functions	*/

/*	User interface variables:	*/
#ifdef	MACINTOSH
GLOBAL	WindowPtr		MyWind; 		/*	We use just one window. 			*/
GLOBAL	EventRecord 	MyEvent;		/*	Globally used event record. 		*/
GLOBAL	PicHandle		ToolP,MenuP;	/*	Handles to pictures of "menus"		*/
GLOBAL	BitMap			ButtonBits; 	/*	Bitmap to misc stuff.				*/
GLOBAL	RgnHandle		BackRegion; 	/*	Gray area in our window.			*/
#endif
GLOBAL	Rect			PortRect;		/*	Rectangle containing our window 	*/
GLOBAL	int 			QuitNow;		/*	Exit program, if this is true.		*/
GLOBAL	Rect			ToolR,MenuR;	/*	Rectangle containing menus. 		*/
GLOBAL	Rect			MenuHilite; 	/*	Contains menu item being executed.	*/
GLOBAL	ToolSelection	ToolButtons[MOUSE_BUTTONS]; /*	Tool functions. 		*/
GLOBAL	ToolSelection	ToolLocks[MOUSE_BUTTONS];	/*	Locked tools.			*/
GLOBAL	int 			InputSelector;	/*	Number of input pins in AND_s etc.	*/
GLOBAL	Rect			InputFrame; 	/*	Frame around input selector.		*/
GLOBAL	Rect			EditR,EditOutline;	/*	Editing area rectangles.		*/
GLOBAL	Rect			EditClipper;	/*	Editing area in circuit coordinates.*/
GLOBAL	int 			ConnectorFrames;/*	true=>connectors are always framed. */
GLOBAL	Rect			NilRect;		/*	left=0,top=0,right=0,bottom=0		*/
GLOBAL	int 			SplashVisible;	/*	Is splash screen is visible?		*/

/*	Simulation globals: 		*/
#ifdef	MACINTOSH
GLOBAL	Handle			SimHandle;		/*	Memory spool for chips. 			*/
#endif
GLOBAL	Ptr 			SimPtr; 		/*	Pointer to memory spool.			*/
GLOBAL	long			SimSize;		/*	Size of allocated memory spool. 	*/
GLOBAL	long			SimEnd; 		/*	First unused byte of memory spool.	*/
GLOBAL	int 			SimLevel;		/*	Subchip recursion level counter.	*/
GLOBAL	int 			SimTimer;		/*	A clock for the clock component.	*/
GLOBAL	long			SimSpeed;		/*	Time between two simulation steps.	*/
GLOBAL	TableHeader 	*CurHeader; 	/*	Pointer to currently simulated header.*/
GLOBAL	TableHeader 	*MainHeader;	/*	Pointer to top level header.		*/
GLOBAL	Ptr 			SimBase;		/*	Start of current level components.	*/

/*	File globals:					*/
#ifdef	MACINTOSH
GLOBAL	SFReply 	NameOfOpenFile; /*	Contains info needed to access the file.	*/
#endif
GLOBAL	int 		FileIsNamed;	/*	True, if file has been previously named.	*/


#define RAMCHUNK		32768L		/*	Amount of memory to allocate with one call. */

/*	Simulation constants			*/
#define NEWVALUE			1		/*	Output & 1 == new value. May be invalid.	*/
#define CURRENTVALUE		2		/*	Output & 2 == current value. Always valid.	*/
#define OLDVALUE			4		/*	Output & 4 == old value for edge detection. */

/*	Flag bits						*/
#define INVERTED			1		/*	Output pin #1 is inverted as in NAND.		*/
#define RESOLVED			2		/*	This element has already been simulated.	*/
#define DISPLAYEDVALUE		4		/*	Value currently displayed on screen. OUTP_	*/

/*	Misc. defines					*/
#define MAX_WIRES			100 	/*	Max number of wires to rubberband at once.	*/
#define MAX_ZAPS			5		/*	Number of zaps to do before garbage collect.*/

#include	"protos.h"				/*	Include prototypes for functions.			*/
