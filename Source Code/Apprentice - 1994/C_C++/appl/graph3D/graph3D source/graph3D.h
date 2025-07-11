/*
	Copyright '89	Christopher Moll
	all etceteras reserved
*/

#define	_MC68881_

//#define	FIXED_MATH

typedef double	Real;

#include	<math.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdio.h>
#include <Palettes.h>



#define	PI (3.14159265358979)


/*pascal	Boolean	WaitNextEvent() = 0xa860;
*/

/* windows */
#define	WIND_FUNCT	128
#define WIND_GRAPH	129

#define SCREEN_WIDTH		512
#define SCREEN_HEIGHT		342
#define SCREEN_MARGIN		4
#define TITLE_BAR_HEIGHT	18
#define	SCBAR_WIDTH			15


/* Dialogs */
#define DIAL_SCALE_XY	128
#define DIAL_SCALE_RP	138
#define	DIALI_S_SX		1
#define	DIALI_S_EX		2
#define	DIALI_S_SY		3
#define	DIALI_S_EY		4
#define	DIALI_S_REDRWB	9

#define DIAL_LIMT		129
#define	DIALI_L_ZS		1
#define	DIALI_L_C		2
#define	DIALI_L_F		3

#define DIAL_METER		131

#define DIAL_FUNCT		134
#define	DIALI_F_SWTCH	1
#define	DIALI_F_FUNCT	2

#define DIAL_DERIV		135
#define	DIALI_D_SWTCH	1
#define	DIALI_D_FUNC1	4
#define	DIALI_D_FUNC2	5

#define DIAL_INITVL		136
#define	DIALI_I_VALUE	2

#define DIAL_CYLIND		137

/* Alerts */
#define ALRT_FORM		130
#define ALRT_FILE		132
#define ALRT_MEMRY		133
#define	ALERT_020		139
#define	ALERT_881		140
#define ALRT_ABOUT		141
#define ALERT_GENRAL	200

/* menus */
#define MENU_BAR_HEIGHT		20

#define MENU_APPLE	1
#define MENU_FILE	2
	enum {
		FILE_OPEN = 1, FILE_OPEN_MAND, FILE_CLOSE, FILE_SVBITM, FILE_SVOBJ, FILE_QUIT };

#define MENU_EDIT	3
	enum {
		EDIT_UNDO = 1, EDIT_CUT = 3, EDIT_COPY, EDIT_PASTE, EDIT_CLEAR };

#define MENU_PLOT	4
	enum {
		PLOT_WIREFRAM = 1, PLOT_SHDSURF, PLOT_SHDHITE, PLOT_DRWSCRN, PLOT_SETLITE = 6,
		PLOT_ZEROROT = 8, PLOT_NORMVIEW, PLOT_TOPVIEW, PLOT_REDRAW = 12 };

#define MENU_GTYP	5
/* graph types */
	enum {
		FUNCT_GTYPE = 1, CYLIND_GTYPE, DERIV_GTYPE, FDATA_GTYPE };

#define MENU_WIND	6
	enum {
		WINDMEN_GRAPH = 1, WINDMEN_FUNCT, WINDMEN_SPECS, WINDMEN_LIMITS };


	/* Switcher stuff */
#define		IS_SUSPND(x)	(!((x)&0x1))
#define		LOAD_CLIP(x)	((x)&0x2)
#define		resumEvt		15




/* Boolean operations */
#define NOT			!
#define AND			&&
#define NEQ			!=
#define EQ			==
#define OR			||
#define COMP		~

/* Math macros */
#define MOD			%
#define Abs(x)		((x>0)?(x):(-(x)))
#define round(x)	((int)((x)+((x)>0)-.5))
#define CLOSE(x,y)	(Abs((x)-(y))<1.0E-15)
#define SQ(x)		((x)*(x))
#define SQ_L(x)		((long)(x)*(long)(x))
		/* convert to longs when squaring large integers */
#define	MAX(x,y)	((x>y)?(x):(y))
#define	MIN(x,y)	((x<y)?(x):(y))
#define	IS_DIGIT(x)		((x)>='0'&&(x)<='9')
#define NIL			((long)0)
#define	DIV_2(x)	((x)>>1)
#define	DIV_4(x)	((x)>>2)
#define	MUL_2(x)	((x)<<1)
#define	MUL_4(x)	((x)<<2)

#define	DeBug()	asm {dc.w 43519}

#define Pstrcpy(x,y)	BlockMove(x,y,(long)((x)[0]+1))
#define	HIWORD(x)	((int)((x)>>16))
#define	LOWORD(x)	((int)(x))
#define	HIBYTE(x)	(((x)&0xff000000)>>24)

#define	pass(x)		(x).h,(x).v
#define	ptoc(x)		PtoCstr(x)
#define	ctop(x)		CtoPstr(x)
#define	cotan(x)	(1.0/tan(x))

#define	LOG2		(.6931471806)
#define	RADtoDEG	(180.0)/PI
#define	DEGtoRAD	PI/(180.0)
#define	SMALL		(1.0e-10)


#ifdef	_MC68881_
	Real	mldexp(Real, int);
#else
	#define	mldexp(x)	ldexp(x)
#endif


/* Fixed math */
#define	FixToD(x)	mldexp((Real)(x),-16)
#define	DToFix(x)	((long)mldexp((x),16))
#define	FixToi(x)	((int)((x)>>16))
#define	iToFix(x)	(((long)(x))<<16)
#define	FIX_0		(0L)
#define	FIX_1		(0x00010000)
#define	DToRFix(x)	(long)mldexp((x),16+rangeScale)



#define	MAX_SYMBS		11

#define	END_SYMB		0
#define	ADD_SYMB		1
#define	SUBT_SYMB		2
#define	MULT_SYMB		3
#define	DIV_SYMB		4
#define	POWR_SYMB		5

#define	L_PAREN_SYMB	6
#define	R_PAREN_SYMB	7
#define	FUNC_SYMB		8
#define	NUMB_SYMB		9
#define	NEG_SYMB		10


#define	MAX_OPS		30

#define	END_OP		0
#define	ADD_OP		1
#define	SUBT_OP		2
#define	MULT_OP		3
#define	DIV_OP		4
#define	POWER_OP	5
#define	MOD_OP		6

#define	NEG_OP		7
#define	EXP_OP		8
#define	LOG_OP		9
#define	LOG10_OP	10
#define	SQ_OP		11
#define	SQRT_OP		12
#define	SIN_OP		13
#define	ASIN_OP		14
#define	COS_OP		15
#define	ACOS_OP		16
#define	TAN_OP		17
#define	ATAN_OP		18
#define	COTAN_OP	19
#define	SINH_OP		20
#define	COSH_OP		21
#define	TANH_OP		22


#define	PUSH_X_OP		-1
#define	PUSH_Y_OP		-2
#define	PUSH_Z_OP		-3
#define	PUSH_NUM_OP		-4
#define	OP_L_PAREN		-5
#define	OP_R_PAREN		-6

#define	ACT_OPtTEX		1
#define	ACT_TEXtF		2
#define	ACT_DELOPT		3
#define	ACT_STPDN		4
#define	ACT_STPERR		5
#define	ACT_OPtF		6
#define	ACT_DELOP		7

#define	IS_FUNC(x)	((x)>MOD_OP)
#define	IS_PUSH(x)	((x)<0)







typedef unsigned char	Uchar;


typedef struct  {
	Real	x;
	Real	y;
	Real	z;
} Vector;

typedef struct {
	Fixed	x;
	Fixed	y;
	Fixed	z;
} FxVector;

typedef	struct {
	Real	v;
	Real	h;
} DoubPoint;

typedef struct {
	Fixed	v;
	Fixed	h;
} FixPoint;

typedef struct {
	Real	top;
	Real	left;
	Real	bottom;
	Real	right;
} DoubRect;

typedef	struct {
	Fixed	conts[3][3];
} cpyFx3Matrx;

typedef	struct {
	Real	conts[3][3];
} cpy3Matrx;

typedef	struct {
	int		length;
	Pattern	patList[64];
} PatternList, **PattListHand;


#	define MoveToTrapNum   0x93
#	define LineToTrapNum   0x91
#	define FixMulTrapNum   0x68
	pascal	void	RMoveTo(...);
	pascal	void	RLineTo(...);
	pascal	void	MLineTo(...);
#	define MoveTo		RMoveTo
#	define LineTo		RLineTo


/*
The following is (c) Dave Platt, 1988
	taken from MandelZot
*/


/* Save-file section header */

typedef struct FileSection {
  long int          signature;
  int               type;
  int               version;
  long int          byteCount;
} FileSection, *FileSectionPtr, **FileSectionHandle;

/* Coordinate save block, Version 0 */

typedef struct CoordBlock {
  int               width;
  int               height;
  int               maxDwell;
  short double      top;
  short double      left;
  short double      bottom;
  short double      right;
} CoordBlock, CoordBlockPtr, CoordBlockHandle;
