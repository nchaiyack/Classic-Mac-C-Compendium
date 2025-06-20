/*
VideoToolbox.h
This file contains the necessary prototypes for use of all the VideoToolbox
files except Luminance.c, which has its own header file, Luminance.h, and
GDInfo.c and GDTestClut.c, which use GDInfo.h.

Precompilation of this header, producing either VideoToolbox.pre or
VideoToolboxMATLAB.pre, will reduce your compilation time tenfold. See
VideoToolbox.c or VideoToolboxMATLAB.c for instructions.

Header files whose precompilation depends on the mc68881 or mc68020 THINK C
compiler options (i.e. math.h and mc68881.h) were omitted from VideoToolbox.h so
that the resulting precompiled header file can be used in projects regardless of
those options. (You�ll want them enabled for speed in your lab, but you�ll
usually want them disabled for portability on the road, e.g. the PowerBook 160
has no floating point unit.) If you change other options that affect the
pre-compilation of the headers (e.g. size of double or size of int) you should
rebuild the precompiled header file, to reflect the new settings.

MAC_C:

The Apple Macintosh header files use some extensions to C that are not part of
Standard C, e.g. "pascal", "short double", enums larger than ints, and function
prototypes that specify absolute addresses. These keywords and constructions
cannot be compiled in Standard C. The macro MAC_C is defined as true, below,
only on Macintosh C compilers that incorporate these extensions, and is false
otherwise. The MAC_C symbol is used to prevent inclusion of all the
Macintosh-dependent header files and prototypes when the compiler doesn't
support them. However, lacking any general test for these extensions, a test
must be added, below, for each compiler. At present we only test Symantec's THINK C
compiler and Apple's MPW C compiler.

MATLAB:

The VideoToolbox sources can be linked to create a stand-alone C application, or
a MATLAB external code resource (a "MEX" file), to be invoked by typing the
function name from within MATLAB. In that case we call the THINK C project a
"MATLAB project". A MATLAB project must #define the symbol MATLAB as true for
every C file, must #include the VideoToolbox.h header at the beginning of every
C file, and must #include MATLABMain.h in the main C file.

Don't #define MATLAB within your C file; use the Edit:Options:THINK C:Prefix
window instead. E.g. you could type
	#define MATLAB 1
into the Prefix window, or, more conveniently (because it speeds up your
compilations enormously), type
	#include "VideoToolboxMATLAB.pre"
See VideoToolboxMATLAB.c for instructions.

The MATLABMain.h header allocates space for QuickDraw globals for use by THINK C
MATLAB projects. It should be #included in the main C file of each THINK C
MATLAB project, i.e. in the C file that contains "main" or "user_fcn". It must
be included in exactly one source file of each project, as the THINK C Linker
will otherwise complain.

dgp & dhb.
Copyright 1989-1993 � Denis G. Pelli

HISTORY: (omitting changes documented in C files)
2/20/93 dhb	Added various commonly used headers.  
			Restructured so that only VideoToolbox.h has to be included,
			whether one is creating stand-alone THINK C or code resources
			to be called by MATLAB. Everything that cares is properly conditioned
			by the symbol MATLAB.
7/9/93	dgp	Changed our convention for the MATLAB symbol, defining it as
			1 or 0 instead of defining or leaving undefined. The advantage of
			this approach is that "MATLAB" can now be used as an argument in an
			ordinary if statement, which is much more readable than #if.
8/21/93	dhb	Added undefs in #if MATLAB so that memory allocation goes through THINK C 
			library, not as callback. Callbacks would be OK except that the Mathworks 
			forgot to give us one for realloc. This leads to mixing memory managers
 			on the same pointers -- bad bad bad.
9/2/93	dhb	commented out #include <TranslateLinefeeds.h> since it's
			not presently compatible with MATLAB.
9/2/93	dgp	Moved the NL and NEWLINE macros from Assign.c into this file.
9/8/93	dgp	Mention VideoToolbox.c.
9/11/93	dhb,dgp	Define STDIO_INT and STDIO_DOUBLE.
9/12/93	dhb	Change STDIO_... to PRINTF_... and SCANF_...
			Insert conditional for MATLAB 3.5 vs. 4.x
9/14/93	dhb	Conditional for MATLAB changed to make symbol match major version number.
			Removed traces of old trying to force 4 byte ints for MATLAB.
9/14/93	dgp	Moved up inclusion of QuickDraw.h before MATLAB stuff, since cmex.h
			requires it. Fixed typo, changing "#elseif" to "#elif".
9/15/93	dgp	Edited the comments above, referring to VideoToolbox.c and the new 
			VideoToolboxMATLAB.c.
9/16/93	dgp	Added contents of former Assign.h.
			Defined MAC_C so that the VideoToolbox.h header may be used with
			Standard C.
9/16/93	dhb Removed prototype for obsolete GetDateTimeString.
10/2/93	dgp	Added #include <Errors.h>.
12/1/93 dgp Now always write "MATLAB" in all caps, since that's the trademarked name.
12/16/93 dhb Somewhere things got munged.  Conditional for MATLAB version appeared
           before inclusion of cmex, which won't work.  I fixed the order.
12/16/93 dhb Commented out inclusion of TranslateLineFeeds.h down below.  This
           generates a link error from MATLAB.
12/29/93 dgp added #include <Packages.h>
*/
#pragma once			/*  suppress multiple inclusions of this file */
#ifndef _VIDEOTOOLBOX_	/*  suppress multiple inclusions of this file */
#define _VIDEOTOOLBOX_
/*
Header files that are required for successful compilation of the rest of this
file are marked "required".
*/
#if defined(THINK_C) && __option(thinkc)	/* THINK C with Mac extensions */	\
	|| defined(applec)						/* MPW C, presumably with Mac extensions */
	#define MAC_C 1		/* allow Macintosh C extensions: "pascal", "short double", etc. */
#else
	#define MAC_C 0
#endif
#ifndef SHORT_DOUBLE_OK
	#define SHORT_DOUBLE_OK MAC_C	/* "short double" not allowed by Standard C */
#endif
#include <stdio.h>			/*  required */
#include <stdlib.h>
#if MAC_C
	#include <QuickDraw.h>	/*  required */
#endif
#ifndef MATLAB
	#define MATLAB 0
#endif
#if MATLAB
	/*
	This must come after stdlib.h, stdio.h, and QuickDraw.h and before everything
	else. If MATLAB is true then we include the header file cmex.h.  Otherwise we
	define the data types that are normally provided by that header, so that code
	that uses these types may be compiled for use in applications.  It must come
	before we test for version 3 or 4.  The version 3 cmex.h does not define the
	symbol mex_h; version 4 does.  This is what we use to figure out which
	version we are compiling for.  Sound complicated?  Sigh!
	*/
	
	#include <cmex.h>
	// Test for MATLAB 4 or MATLAB 3, and set symbol appropriately.
	#undef MATLAB
	#ifdef mex_h
		#define MATLAB 4
	#else
		#define MATLAB 3
	#endif
	#ifndef MAC_C
		#error "MATLAB requires the Macintosh extensions to C."
	#endif

    // Undefine the macros that override memory allocation.  MATLAB's implementation
    // doesn't work if you use realloc.
	#undef malloc
	#undef calloc
	#undef free
	
	// Define data types for printf, scanf.
	#if (MATLAB == 4)						/* Version 4 */
		typedef int PRINTF_INT;				/* type of arguments to printf et al. */
		typedef long double PRINTF_DOUBLE;
		typedef int SCANF_INT;				/* type of arguments to scanf et al. */
		typedef double SCANF_DOUBLE;
	#elif (MATLAB == 3)						/* Version 3.5 */
		typedef long int PRINTF_INT;		/* type of arguments to printf et al. */
		typedef double PRINTF_DOUBLE;
		typedef short int SCANF_INT;		/* type of arguments to scanf et al. */
		typedef long double SCANF_DOUBLE;	
	#endif
	#ifdef THINK_C	/* Declare our own QuickDraw globals, allocated by MATLABMain.h */
		extern THINK_C_QD mex_qd;
		#define qd mex_qd
	#else
		#define mex_qd qd
	#endif
	#if defined(MAIN)						/* Some old programs #define MAIN. */
		#include "MATLABMain.h"				/* New programs just #include directly. */
	#endif
#else
	typedef long int INT;
	#if SHORT_DOUBLE_OK
		typedef short double DOUBLE;/* not allowed by Standard C */
	#endif
	typedef int PRINTF_INT;			/* type of arguments to printf et al. */
	typedef double PRINTF_DOUBLE;
	typedef int SCANF_INT;			/* type of arguments to scanf et al. */
	typedef double SCANF_DOUBLE;
	#if SHORT_DOUBLE_OK
		typedef struct {
			char *name;
			INT type;
			INT m;
			INT n;
			DOUBLE *pr;
			DOUBLE *pi;
		} Matrix;
	#endif
#endif

/*  Standard C headers */
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <string.h>
#include <time.h>

/*  Macintosh headers */
#if MAC_C
	#if THINK_C
		#include <console.h>
	#endif
	#include <Devices.h>
	#include <Errors.h>
	#include <Events.h>
	#include <FixMath.h>	/*  Must come before mc68881.h */
	#include <Fonts.h>
	#include <GestaltEqu.h>
	#include <Memory.h>
	#include <OSEvents.h>
	#include <OSUtils.h>	/*  required */
	#include <Packages.h>
	#include <Palettes.h>	/*  formerly Palette.h */
	#if THINK_C
		#include <pascal.h>	/*  c2pstr(),CtoPStr() */
		#include <profile.h>/* only needed if you want timing info */
	#endif
	#include <QDOffscreen.h>/*  formerly QuickDraw32Bit.h */
	#include <Resources.h>
	#include <Retrace.h>	/*  required */
	#include <Slots.h>
	#include <Sound.h>
	#ifndef THINK_C			/*  e.g. MPW C */
		#include <Strings.h>/*  c2pstr(),CtoPStr() */
	#endif
	#include <Timer.h>		/*  required */
	#include <ToolUtils.h>
	#include <Video.h>
	#include <Windows.h>	/*  required */
#endif

/*
MATLAB AND THE NEWLINE CHARACTER
In most C environments the newline character is '\n' (ascii 10). In Apple's
wisdom they chose to separate lines in Macintosh files with the ascii 13
character, which is usually called the return character '\r'. Standard C
specifies that the stdio library will use '\n' as the newline character. This
potentially creates a compatibility problem between Mac C programs and C
programs written for the rest of the world's computers, since one wants any text
files they produce to be appropriate to the local environment. Both MPW and
THINK C resolve the problem behind the scenes, but they do it in different ways.
MPW redefines the association between symbolic names and character codes, making
'\n' be 13 and '\r' be 10. THINK C instead modifies the stdio library so that
reads and writes to a "text" (i.e. not binary) stream are translated, exchanging
character codes 10 and 13. Both solutions work. However, when we use THINK C to
create code resources for MATLAB we are forced to use the MPW stdio library that
is built into MATLAB. Unfortunately this bypasses both compilers' fixes to the
newline problem. The problem affects every single text i/o operation, e.g. every
printf and scanf. Alas, there is no way to get the pre-processor to redefine /n
and /r.

We have attempted two solutions: a quick fix, using macros NL and NEWLINE, and a
complete clean fix, using TranslateLinefeeds.c. At present only the quick fix
works.

Our quick fix is to create a macro called NL that is normally defined as the
newline string "\n", but which is instead defined as "\015" (i.e. ascii 13) when
MATLAB is true. Programs such as Assign.c that have replaced all instances of
"\n" by NL will then work whether or not MATLAB is true. The obvious drawback is
that this quick solution requires these modifications to the source files.

The clean relatively complete solution is to insert a filter,
TranslateLinefeeds.c, that intercepts the calls to the stdio library and
translates /r to /n and vice versa for i/o to text files. The header file
TranslateLinefeeds.h redefines all the stdio calls as macros that call
corresponding routines within TranslateLinefeeds.c. Note that scanf is not
intercepted and that for printf (and fprintf, ...) only the format string
is translated; linefeeds hiding in char or string arguments in the variable
part of the argument list are not be translated.
*/
#if MATLAB && THINK_C			/* Clean, relatively complete solution */
	//#include <TranslateLinefeeds.h>
#endif
#if !defined(_TranslateLinefeeds_) && MATLAB && THINK_C	/* Quick fix */
	#define NL "\r"
	#define NEWLINE '\r'
#else
	#define NL "\n"		/* the standard new-line string */
	#define NEWLINE '\n'
#endif

/*
TRUE and FALSE
*/
#ifndef TRUE
	#define FALSE	0
	#define TRUE	1
#endif

/*
NAN & INF
If you're using NAN and INF you'll be interested in the IsNan(), IsInf(), and
IsFinite() definitions & prototypes below. Note that the THINK C compiler is
timid, worried about possible exception events, so the INF and NAN expressions
below are evaluated at runtime. (Drat!) If you use these expressions in a tight
loop, you'd be better off putting the value in a variable (preferably a
register) and using that instead.
*/
#if !defined(INF)
	#define INF	(1.0/0.0)		/* Infinity */
#endif
#if !defined(NAN)
	#define NAN	(0.0/0.0)		/* Not a Number */
#endif

/*
NUMERICAL CONSTANTS
It's sad to say, but THINK C 5.02 loses two bits of precision converting between
doubles and ascii text, in either direction. Thus you will obtain more accurate
results by computing the numerical constants below at runtime rather than using
these predefined constants. The ANSI Numerical C Extensions group is moving to
require C compilers to preserve precision, so there is hope for the future. Of
course most applications would never notice a loss of two bits precision out of
the total double precision given by the 64 bits in the mantissa.
*/
#if !defined(PI)
	#define PI		3.1415926535897932385	/* computed in Mathematica */
#endif
#if !defined(LOGPI)
	#define LOGPI	1.14472988584940017414	/* computed in Mathematica */
#endif
#if !defined(LOG2)
	#define LOG2	0.69314718055994530942	/* computed in Mathematica */
#endif
#if !defined(LOG10)
	#define LOG10	2.30258509299404568402	/* computed in Mathematica */
#endif

#if MAC_C
	/*
	CLUTSIZE
	Inline code that returns the same answer as GDClutSize(device).
	*/
	#define GDCLUTSIZE(device) (gdClutSizeTable[(**(**(device)).gdPMap).pixelSize])
	
	/*  Commented out, because its answer is misleading when in 16-bit or 32-bit mode. */
	/* #define GDCOLORS(device) ((**(**(**device).gdPMap).pmTable).ctSize+1) */
#endif

/*
FIXED POINT ARITHMETIC
Apple defines a handy data type called Fixed that is stored in a long, but is
assumed to have a decimal point in the middle. Many operations, e.g. adding two
Fixed numbers or multiplying or dividing a Fixed by an integer, can be performed
directly. To multiply or divide two Fixed numbers use Apple's FixMul() and
FixDiv(). FixRatio(n,m) returns the Fixed ratio of two integers. Macintosh C
compilers define double in various ways, depending on whether a floating point
unit is to be used. The Apple-provided routines for doing type conversion to and
from Fixed are only appropriate if you are NOT using the floating point unit.
The ones defined below are faster and work with or without the FPU.
*/
#define LongToFix(x) ((long)(x)<<16)
#define FixToLong(x) ((x)>>16)
#define DoubleToFix(x) ((long)((x)*65536.+0.5))
#define FixToDouble(x) ((double)(x)*(1./65536.))

#if MAC_C
	/*
	STACK
	StackGrow(n) increases the stack allocation by n bytes. You'll also want to use
	Apple's StackSpace(), declared in Memory.h, that returns the number of bytes 
	allocated for the stack.
	*/
	#define StackGrow(extraBytes) SetApplLimit(GetApplLimit()-(extraBytes))
#endif

/* Assign.c */

#define ASSIGN_DIMS 3		/* Feel free to increase or decrease ASSIGN_DIMS. */
typedef struct {
	short type;
	unsigned sizedOnce:1;	/* dim[] is meaningful */
	unsigned sized:1;		/* dim[] is final */
	unsigned malloced:1;	/* allocated by malloc */
	char *name;
	void *ptr;				/* for array, address of element zero */
	long dim[ASSIGN_DIMS];	/* zero indicates a scalar */
	long firstElement;		/* for 1-d array, index of first array element. Usually 0 */
	const char *comment;	/* text string, or NULL */
} Description;
enum{assignReportUnknown=1,assignNoHexInts=2,assignHexFloats=4
	,assignNoComment=8,assignNoPrintfExit=0x10
	,assignEchoAssignments=0x20,assignEchoComments=0x40,assignEchoFile=0x80
};
enum {assignMemoryError=-1,assignTypeError=-2
	,assignVariableError=-3,assignUnknownVariableError=-4
	,assignSubscriptError=-5,assignSubscriptBoundsError=-6
	,assignEqualsError=-7,assignConstantError=-8,assignHexError=-9
	,assignSemicolonError=-10,assignFileError=-11
	,assignInconsistentDescriptionsError=-12,assignUnequalDataError=-13
	,assignInconsistentDimensionsError=-14
	,assignCouldntFindDescription=-15
};
enum{charType=1,unsignedCharType,shortType,unsignedShortType
	,longType,unsignedLongType
	,floatType
	#if SHORT_DOUBLE_OK
		,shortDoubleType
	#endif
	,doubleType
	,charPtrType,unsignedCharPtrType,shortPtrType,unsignedShortPtrType
	,longPtrType,unsignedLongPtrType
	,floatPtrType
	#if SHORT_DOUBLE_OK
		,shortDoublePtrType
	#endif
	,doublePtrType
	,stringType,unknownType
};
FILE *OpenCalFileRead(char *filename);
FILE *OpenCalFileReadAndCheck(char *filename);
FILE *OpenCalFileWrite(char *filename);
void AppendDescriptions(Description **d,Description *s);
void CopyDescriptions(Description *d,Description *s);
Description *AllocateDescriptions(long n);
void FreeDescriptions(Description *d);
long NumberOfDescriptions(Description *d);
Description NullDescription(void);
int IsNullDescription(Description d);
#define IsNullDescription(d) ((d).type==0)
int AssignmentLineWasBlank(void);
int UnequalDescribedVars(Description d1[],Description d2[],short flags);
int UnequalDescribedVarPair(Description *d1,Description *d2,short flags);
long InitializeADescribedVar(Description *d,short flags);
long InitializeDescribedVars(Description d[],short flags);
long FindDescribedDim(Description d[],void *ptr,int i,short flags);
long FindDescription(Description d[],void *ptr,short flags);
void FreeADescribedVar(Description *d,short flags);
void FreeDescribedPtrVars(Description d[],short flags);
void FreeDescribedVars(Description d[],short flags);
void KeepDescribedVars(Description d[],short flags);
void KeepADescribedVar(Description *d,short flags);
long PrintAnAssignment(FILE *stream,Description *d,short flags);
long PrintAssignments(FILE *stream,Description d[],short flags);
long PrintAssignmentsToFile(const char *filename,Description d[],short flags);
long ReadAssignmentLine(FILE *stream,Description d[],short flags);
long ReadAssignmentBlock(FILE *stream,Description d[],short flags);
long ReadAssignmentStream(FILE *stream,Description d[],short flags);
long ReadAssignmentFile(const char *filename,Description d[],short flags);
Description Describe(short type,void *ptr,char *name,const char *comment);
Description DescribeArray(short type,void *ptr,char *name,const char *comment,...);
Description DescribeFirstLast(short type,void *ptr,char *name
	,const char *comment,long firstElement,long lastElement);

/* Binomial.c */

long	BinomialSample(double p,long n);
int		BinomialSampleQuickly(int n);
double	BinomialLowerBound(double P,long k,long n);
double	BinomialUpperBound(double P,long k,long n);
double	BinomialPdf(double p,long k,long n);
double	Binomial(double p,long k,long n);
double	IncompleteBeta(double x,double a,double b);
double	InverseBinomial(double P,long k,long n);
double	InverseIncompleteBeta(double p,double a,double b);

/* BreakLines.c */

char *BreakLines(char *string,long lineLength);
void PrintWrappedText(FILE *stream,const char *s);
void PrintWrappedTextToFile(const char *filename,const char *s);
void PrintWrappedComment(FILE *stream,const char *s);
void PrintWrappedCommentToFile(const char *filename,const char *s);

#if MAC_C
	/* CardSlot.c */
	
	OSErr	CardSlot(char *cardName);
	
	/* CenterRectInRect.c */
	
	void	CenterRectInRect(Rect *moveableRectPtr,Rect *fixedRectPtr);
	void	OffsetRectTile(Rect *r,int nx,int ny);
	Boolean	RectInRect(Rect *r,Rect *R);
#endif

/* ChiSquare.c */

double	PChiSquare (double chiSquare,int n);

#if MAC_C
	/* ConvolveX.c */
	
	void	ConvolveX(double f[],int dim,BitMap *srcBits,BitMap *dstBits,
				Rect *srcRectPtr,Rect *dstRectPtr);

	/* ConvolveY.c */
	
	void ConvolveY(double f[],int dim,BitMap *srcBits,BitMap *dstBits,
				Rect *srcRectPtr,Rect *dstRectPtr);
	
	/* CopyBitsQuickly.c */
	
	enum{mulOver=128};
	int	CopyBitsQuickly(BitMap *srcBits,BitMap *dstBits,
				Rect *srcRectPtr,Rect *dstRectPtr,int srcMode,RgnHandle maskRgn);
	
	/* CopyQuickDrawGlobals.c */
	
	void CopyQuickDrawGlobals(void);
	#define CopyQuickdrawGlobals CopyQuickDrawGlobals	/*  old spelling */
	
	/* CreateTrialSnds.c */
	
	void CreateTrialSnds(void);
#endif

/* DateString.c */

char	*DateString(unsigned long seconds);

#if MAC_C
	/* DrawPrintf.c */
	
	void DrawPrintf(char *s, ...);
#endif

/* Exponential.c */

double ExponentialPdf(double x);

/* ffprintf.c */

int ffprintf(FILE *stream[2],char *format,...);

#if MAC_C
	/* FlushCacheRange.c */
	
	void FlushCacheRange (void *address, unsigned long count);
	
	/* GDOpenWindow.c */
	
	void AddExplicitPalette(WindowPtr window);
	void RemovePalette(WindowPtr window);
	WindowPtr GDOpenWindow1(GDHandle device);
	void GDDisposeWindow1(WindowPtr window);
	CWindowPtr GDOpenWindow(GDHandle device);					/*  Old. Use GD...1 instead. */
	void GDDisposeWindow(GDHandle device,CWindowPtr myWindow);	/*  Old. Use GD...1 instead. */
	
	/* GDPrintf.c */
	
	void	GDPrintf(char *s, ...);
	
	/* GDTime.c */
	
	double GDFrameRate(GDHandle device);
	double GDMovieRate(GDHandle device,int quickly);
	double GDMovieSize(GDHandle device,int quickly);
	double GDVBLRate(GDHandle device);
	double TickRate(void);
	typedef OSErr (*SetEntriesFunction)(GDHandle device,short start,short count
		,ColorSpec *aTable);
	OSErr GDTimeClut(GDHandle device,SetEntriesFunction function,short clutEntries
		,double *sPtr,double *framesPtr,double *missingFramesPtr,double *frameRatePtr);
	
	/* GDVideo.c */
	
	#define MAX_SCREENS 8
	char  *GDCardName(GDHandle device);
	extern short gdClutSizeTable[33];
	short GDClutSize(GDHandle device);
	long GDColors(GDHandle device);
	OSErr GDControl(int refNum,int csCode,Ptr csParamPtr);
	short GDDacSize(GDHandle device);
	OSErr GDDirectSetEntries(GDHandle device,short start,short count,ColorSpec *aTable);
	OSErr GDGetDefaultGamma(GDHandle device,GammaTbl **gammaTbl);
	OSErr GDGetDefaultMode(GDHandle device,short *modePtr);
	OSErr GDGetEntries(GDHandle device,short start,short count,ColorSpec *aTable);
	OSErr GDGetGamma(GDHandle device,GammaTbl **myGammaTblHandle);
	OSErr GDGetGray(GDHandle device,Boolean *flagPtr);
	OSErr GDGetInterrupt(GDHandle device,Boolean *flagPtr);
	OSErr GDGetMode(GDHandle device,short *modePtr,short *pagePtr,Ptr *baseAddrPtr);
	OSErr GDGetPageBase(GDHandle device,short page,Ptr *baseAddrPtr);
	OSErr GDGetPageCnt(GDHandle device,short mode,short *pagesPtr);
	OSErr GDGrayPage(GDHandle device,short page);
	Boolean GDHasMode(GDHandle device,short mode,short *pixelSizePtr,short *pagesPtr);
	#define GDLinearGamma GDUncorrectedGamma /* old name, before 12/16/92 */
	short GDModePixelSize(GDHandle device,short mode);
	unsigned char *GDName(GDHandle device);
	ColorSpec *GDNewLinearColorTable(GDHandle device);
	short GDPixelSize(GDHandle device);
	OSErr GDPrintGammaTable(FILE *o,GDHandle device);
	OSErr GDReset(GDHandle device,short *modePtr,short *pagePtr,Ptr *baseAddrPtr);
	OSErr GDRestoreDeviceClut(GDHandle device);
	OSErr GDRestoreGamma(GDHandle device);
	OSErr GDSaveGamma(GDHandle device);
	OSErr GDSetDefaultMode(GDHandle device,short mode);
	OSErr GDSetEntries(GDHandle device,short start,short count,ColorSpec *aTable);
	OSErr GDSetEntriesByType(GDHandle device,short start,short count,ColorSpec *table);
	OSErr GDSetEntriesByTypeHighPriority(GDHandle device,short start,short count
		,ColorSpec *table);
	OSErr GDSetGamma(GDHandle device,GammaTbl *myGammaTblPtr);
	OSErr GDSetGray(GDHandle device,Boolean flag);
	OSErr GDSetInterrupt(GDHandle device,Boolean flag);
	OSErr GDSetMode(GDHandle device,short mode,short page,Ptr *baseAddrPtr);
	OSErr GDSetPageDrawn(GDHandle device,short page);
	OSErr GDSetPageShown(GDHandle device,short page);
	OSErr GDStatus(int refNum,int csCode,Ptr csParamPtr);
	short GDType(GDHandle device);
	OSErr GDUncorrectedGamma(GDHandle device);
	int  GDVersion(GDHandle device);
	#define ModeName GDModeName   /* old name, before 1/18/93 */
	int  PatchMacIIciVideoDriver(void);
	
	/* GetClicks.c */
	
	short	GetClicks(void);
#endif

/* GetTimeDateString.c */

char *GetTimeDateString(time_t t);

#if MAC_C
	/* GetVersionString.c */

	char *GetVersionString(void);

	/* GetScreenDevice.c */
	
	GDHandle	GetScreenDevice(int n);
	int			GetScreenIndex(GDHandle device);
	GDHandle	SlotToScreenDevice(int n);
	short int	GetDeviceSlot(GDHandle device);
	GDHandle	GetWindowDevice(WindowPtr theWindow);
	GDHandle	GetRectDevice(Rect *r);
	void		LocalToGlobalRect(Rect *r);
	void		GlobalToLocalRect(Rect *r);

	/* GetVoltage.c */
	
	double	VoltsDuringFrame(double frames);
	double	GetVoltage(short channel,double *gainPtr,double *frequencyPtr,long n
				,double *sdPtr);
	short	GetVoltages(short channel,double *gainPtr,double *frequencyPtr,long *nPtr
				,unsigned short readings[],double *voltDeltaPtr,double *voltZeroPtr);
	short	InitiateVoltageSampling(short channel,double *gainPtr,double *frequencyPtr
				,double *voltDeltaPtr,double *voltZeroPtr);
	short	RetrieveVoltages(long *nPtr,unsigned short readings[]);
	int		ForeRunnerSlot(void);
	enum{voltageBufferOverflow=1<<0,voltageOverflow=1<<1,voltageUnderflow=1<<2};
#endif

/* HexToBinary.c */

int HexToBinary(char *string,void *ptr);
char *BinaryToHex(size_t n,void *ptr,char *string);

#if MAC_C
	/* HideMenuBar.c */
	
	void	HideMenuBar(void);
	void	ShowMenuBar(void);
	void SquareCorners(GDHandle device);
	void RestoreCorners(GDHandle device);
	void UnclipScreen(GDHandle device);
	void RestoreScreenClipping(GDHandle device);
	
	/* Identify.c */
	
	char *IdentifyCompiler(void);
	char *IdentifyMachine(void);
	char *IdentifyModel(void);
	char *IdentifyOwner(void);
	char *IdentifyVideo(GDHandle device);
	
	/* IsCmdPeriod.c */
	
	Boolean IsCmdPeriod(EventRecord *theEvent);
#endif

/* IsNan.c */

int IsNan(double x);
int IsInf(double x);
#ifndef IsFinite
	#define IsFinite(x) ((*(short *)&(x) & 32767)!=32767)	/* neither NAN nor �INF */
#endif

#if MAC_C
	/* kbhit.c */
	
	int	kbhit(void);
	int	getcharUnbuffered(void);
#endif

/* Log2L.c */

long Log2L(unsigned long j);

#if MAC_C
	/* MakeNoise.c */
	
	Boolean MakeNoise1(double dx,double dy,Boolean randomPhase,PixMap *frame);
	void	MakeNoise(double dx,double dy,PixMap frame[],short *maxFramesPtr);
	
	/* MaximizeConsoleHeight.c */
	
	void MaximizeConsoleHeight(void);
#endif

/* Mean.c */

double Mean(double x[],long n,double *sdPtr);
double MeanF(float x[],long n,double *sdPtr);
double MeanB(char x[],long n,double *sdPtr);
double MeanUB(unsigned char x[],long n,double *sdPtr);
double MeanW(short x[],long n,double *sdPtr);
double MeanUW(unsigned short x[],long n,double *sdPtr);
double MeanL(long x[],long n,double *sdPtr);
double MeanUL(unsigned long x[],long n,double *sdPtr);

/* MultipleChoice.c */

#if !MAC_C
	#define Boolean unsigned char
#endif
int MultipleChoice(short defaultChoice,short n,char *answer[]);
Boolean YesOrNo(Boolean defaultAnswer);
#if !MAC_C
	#undef Boolean
#endif

/* Nan.c */
extern const short __NAN[];
#define Nan (*(double *)__NAN)

/* Normal.c */

double NormalPdf(double x);
double Normal(double x);
double InverseNormal(double p);
double NormalSample(void);
double Normal2DPdf(double r);
double Normal2D(double r);
double InverseNormal2D(double p);
double Normal2DSample(void);
double InverseNormal2DPdf(double p);
void BoundedNormalIntegers(short *distribution,long n,double mean,double sd
	,short min,short max);

/* nrand.c */

unsigned long nrand(unsigned long n);

/* OpenDataFiles.c */

unsigned long OpenDataFiles(FILE **inPtr,FILE **outPtr
					,char *inName,char *outName);

/* OpenPreferencesFolder.c */

#if !MAC_C
	#define OSErr short
#endif
OSErr OpenPreferencesFolder(void);
OSErr ClosePreferencesFolder(void);
OSErr OpenPreferencesWD(short *wdRefNumPtr);
#if !MAC_C
	#undef OSErr
#endif

#if MAC_C
	/*	PixMapToPICT.c */

	void PixMapToPICT(char *filename,PixMap **pm,Rect *rectPtr
		,int pixelSize,ColorTable **cTable);

	/*	PixMapToPostScript.c */
	
	void	PixMapToPostScript(char *filename,PixMap **pm,Rect *rectPtr
				,Rect *pageRectPtr,double cellsPerInch,int grayLevels);
	void	AppendToFile(char *filename,char *string);
	
	/* PlotXY.c */
	
	typedef struct{
		Boolean continuing;	/* zero to start a new curve */
		long color;			/* e.g. blackColor, blueColor */
		short lineWidth;	/* in pixels, zero for none */
		short symbolWidth;	/* in pixels, zero for none */
		short dashOffset;	/* in pixels */
		short dash[5];		/* in pixels. The array is terminated by a zero element */
		short h,v;			/* reserved for internal use */
		Fixed pathLengthF;	/* reserved for internal use */
	} PlotXYStyle;
	void PlotXY(WindowPtr window,double x,double y,PlotXYStyle *style);
#endif

/* PrintfExit.c */

int PrintfExit(const char *format,...);

#if MAC_C

	/* PrintfGWorld.c */

	void PrintfGWorld(GWorldPtr our);
	void PrintStringAsBitmap(unsigned char *s);

	/* QD32Exists.c */
	
	Boolean 		QD8Exists(void);
	Boolean			QD32Exists(void);
	Boolean			NewPaletteManager(void);
#endif

/* randU.c */

unsigned short	randU(void);
unsigned long	randUL(void);
void			srandU(unsigned n);
void			RandFill(void *address,long bytes);

/* ReadMATLABFile.c */

int LoadNamedMatIntoDoubles(FILE *f,const char *name,long *rows,long *cols
	,double **real,double **imag);
int LoadNamedMatIntoShorts(FILE *f,const char *name,long *rows,long *cols
	,short **real,short **imag);
int LoadNextMatIntoDoubles(FILE *f,char *name,long *rows,long *cols
	,double **real,double **imag);
int LoadNextMatIntoShorts(FILE *f,char *name,long *rows,long *cols
	,short **real,short **imag);
int SaveMatDoubles(FILE *f,char *name,long rows,long cols,double *real,double *imag);
int SaveMatShorts(FILE *f,char *name,long rows,long cols,short *real,short *imag);

#if MAC_C
	/* RectToAddress.c */
	
	unsigned char *RectToAddress(PixMap *pixMapPtr,Rect *rectPtr,short *rowBytesPtr, 
		short *pixelSizePtr,short *bitsOffsetPtr);
	Ptr GetPixBaseAddr32(PixMap **pm);
#endif
	
/*	Require.c	*/

void Require(long quickDrawVersion);
struct PrecompileSizeofInt{
	int i;
};
struct PrecompileSizeofDouble{
	double d;
};

#if MAC_C
	/* RestoreCluts.c */
	
	void RestoreCluts(void);
	
	/*	SetEntriesQuickly.c */
	
	OSErr SetEntriesQuickly(GDHandle device,short start,short count,ColorSpec *table);
	short macltset(GDHandle device,register short start
		,unsigned short *red,unsigned short *green,unsigned short *blue,short count1);
	short GetCardType(GDHandle device);
	char *GetCardBase(GDHandle device);
	OSErr WaitForNextBlanking(GDHandle device);
	
	/* SetFileInfo.c */
	
	void	SetFileInfo(char *fileName,OSType fileType,OSType fileCreator);
	
	/* SetMouse.c */
	
	void	SetMouse(Point where);
	
	/* SetOnePixel.c */
	
	void SetPixmapPixel(PixMap *pmHandle,int x,int y,unsigned long value);
	unsigned long GetPixmapPixel(PixMap *pmHandle,int x,int y);
	unsigned char *GetPixmapPixelAddress(PixMap *pmHandle,int x,int y);
	void SetDevicePixel(GDHandle device,int x,int y,unsigned long value);
	unsigned long GetDevicePixel(GDHandle device,int x,int y);
	void SetOnePixel(int x,int y,unsigned long value);
	unsigned long GetOnePixel(int x,int y);
	#define SetIPixel SetDevicePixel	/*  So that old programs won't break. */
	#define GetIPixel GetDevicePixel	/*  So that old programs won't break. */
	
	/* SetPixelsQuickly.c */
	
	int SetPixelsQuickly(int x,int y,unsigned long value[],short n);
	int GetPixelsQuickly(int x,int y,unsigned long value[],short n);
	int SetWindowPixelsQuickly(WindowPtr window,int x,int y,unsigned long value[],short n);
	int GetWindowPixelsQuickly(WindowPtr window,int x,int y,unsigned long value[],short n);
	int SetDevicePixelsQuickly(GDHandle device,int x,int y,unsigned long value[],short n);
	int GetDevicePixelsQuickly(GDHandle device,int x,int y,unsigned long value[],short n);
	int SetPixmapPixelsQuickly(PixMapPtr pmPtr,int x,int y,unsigned long value[],short n);
	int GetPixmapPixelsQuickly(PixMapPtr pmPtr,int x,int y,unsigned long value[],short n);
	
	/* SetPriority.c */
	
	void SwapPriority(char *priority);
	void SetPriority(int i);
	int GetPriority(void);
#endif

/* Shuffle.c */

void Shuffle(void *array,long elements,size_t elementSize);

#if MAC_C
	/* SndPlay1.c */
	
	OSErr SndPlay1(Handle snd);
	void SndStop1(void);
	short SndDone1(void);

	/* StringToDate.c */
	
	double StringToDate(char *string,DateTimeRec *date);

	/* StringBounds.c */
	
	void CharBounds(char a,Rect *bounds,long *count);
	void StringBounds(unsigned char *s,Rect *bounds,long *count);

	/* Timer.c */
	
	struct Timer{
		TMTask time;
		long ourA5;
		long interval,elapsed,elapsedIntervals;
		long timeToStartTimer;			/*  minimum time in �s */
		long stopDelay;					/*  �s from call to stop, re from call to start */
		long timeManagerVersion;
		struct Timer *next,*previous;	/*  doubly linked list of Timers */
	};
	typedef struct Timer Timer;
	
	Timer *NewTimer(void);
	void DisposeTimer(Timer *t);
	void StartTimer(Timer *t);
	long StopTimer(Timer *t);					/*  �s */
	double StopTimerSecs(Timer *t);				/*  s */
	
	/* TitleBarHeight.c */
	
	int TitleBarHeight(WindowPtr window);
	
	/* TrapAvailable.c */
	
	Boolean	TrapAvailable(short theTrap);
#endif

/* Uniform.c */

double UniformSample(void);

#if MAC_C
	/* VBLInstall.c */
	
	struct VBLTaskAndA5 {
	    volatile VBLTask vbl;
	    long ourA5;
	    void (*subroutine)(struct VBLTaskAndA5 *vblData);
	    GDHandle device;
	    long slot;
	    volatile long newFrame;				/*  Boolean */
	    volatile long framesLeft;			/*  count down to zero */
	    long framesDesired;
	    Timer *frameTimer;					/*  time ms since last VBL interrupt, see Timer.c */
	    void *ptr;							/*  use this for whatever you want */
	};
	typedef struct VBLTaskAndA5 VBLTaskAndA5;	
	OSErr VBLInstall(VBLTaskAndA5 *vblData,GDHandle device,long frames);
	OSErr VBLRemove(VBLTaskAndA5 *vblData);
	void VBLInterruptServiceRoutine(void);
	void SimpleVBLSubroutine(VBLTaskAndA5 *vblData);
	
	/* VideoTFB.c */
	
	Boolean TFBInSlot(int slot);
	void	SetUpTFB(int slot);
	void	RampClutTFB(int slot);
	void	GrayClutTFB(int slot);
	void	LoadClutTFB(int slot,unsigned char rgb[256][3]);
	void	NewBlankingTFB(int slot);
	void	NewFieldTFB(int slot);
	int		BlankingTFB(int slot);
	void	SetDepthTFB(int slot,short int bits);
	void	SynchSetDepthTFB(int masterSlot,int slot,short int bits);
	void	SynchToMainDeviceTFB(GDHandle device);
	void	HaltTFB(int slot);
	void	RestartTFB(int slot,short int bits);
	void	HaltDeviceTFB(GDHandle device);
	void	RestartDeviceTFB(GDHandle device);
	void	ScrollTFB(int slot,short int bits,long x,long y);
	void	PanTFB(int slot,long int x);
#endif

/* VLambda.c */

double VLambda(double nm);
double VLambdaPrime(double nm);

#if MAC_C
	/* Zoom.c */
	
	void Zoom(WindowPtr theWindow,int zoomDir);
#endif

#endif /* _VIDEOTOOLBOX_ */
