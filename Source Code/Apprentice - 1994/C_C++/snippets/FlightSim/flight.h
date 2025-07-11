/*
#include <Quickdraw.h>
#include <Events.h>
#include <Windows.h>
#include <Files.h>
*/

#include <math.h>
#include <FixMath.h>


#define NOT		!
#define AND		&&
#define NEQ		!=
#define EQ		==
#define OR		||
#define COMP	~
#define MOD		%
#define SQ(x)	((x)*(x))
#define NIL		(0L)

#define ASCIItoNUM(x)	((x)-48)
#define NUMtoASCII(x)	((x)+48)

//char	*PtoCstr();
//char	*CtoPstr();
#define	ptoc(x)		PtoCstr(x)
#define	ctop(x)		CtoPstr(x)

#define	DeBug()	asm{dc.w 43519}



/* menus */
#define MENU_APPLE	1
#define MENU_FILE	2
#define MENU_EDIT	3


/* windows */
#define MENU_BAR_HEIGHT	20
#define SCREEN_WIDTH	512
#define SCREEN_HEIGHT	342
#define SCREEN_MARGIN	4
#define TITLE_BAR_HEIGHT	18	
#define inZoomIn	7
#define inZoomOut	8

#define WIND_MAIN	128
#define ALRT_ABOUT	128


#define	MOUSE_POS	(*(Point *)(0x830))

#define	GRID_SZ		10

struct Vector {
	double	x;
	double	y;
	double	z;
};
typedef struct Vector Vector;

struct FxVector {
	Fixed	x;
	Fixed	y;
	Fixed	z;
};
typedef struct FxVector FxVector;

typedef double ThreeMatrx[3][3];
typedef Fixed FxThreeMatrx[3][3];
typedef Fract FracThreeMatrx[3][3];

struct FracThreeMatrxStrc {
	Fract	entrys[3][3];
};
typedef struct FracThreeMatrxStrc FracThreeMatrxStrc;

struct FxGridMatrx {
	FxVector	entrys[GRID_SZ][GRID_SZ];
};
typedef struct FxGridMatrx FxGridMatrx;


#define	FIX_HALF	(0x8000)
#define	FIX_ONE		(0x10000)
#define	FRAC_ONE	(0x40000000)
#define	FIX_NG_ONE	(0xffff0000)
#define	FixToD(x)	ldexp((double)(x),-16)
#define	DToFix(x)	((long)ldexp((x),16))
#define	FixToi(x)	((int)((x)>>16))
#define	RndFixToi(x)	((int)((x)+FIX_HALF>>16))

#define	iToFix(x)	(((long)(x)<<16))
#define	FixRound(x)	FixToi(x)
#define	Frac2Fix(x)	((x)>>14)
#define	Fix2Frac(x)	((x)<<14)
#define	FracToD(x)	ldexp((double)(x),-30)
#define	DToFrac(x)	((long)ldexp((x),30))
#define	Fx_DEGtoRAD	((long)0x477)

Fixed	_FixMul(), _FixDiv();



#define		stepSize	2
#define		startHeight		4
