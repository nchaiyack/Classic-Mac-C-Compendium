//#ifndef __FSPCOMPAT__
//#include "FSpCompat.h"
//#endif

#if defined(FabSystem7orlater)

#define	FabStandardGetFile	StandardGetFile
#define	FabStandardPutFile	StandardPutFile

#else

#include	"Utils6.h"

#endif


//#if GENERATING68K
//#pragma parameter MyNumToString(__D0,__A0)
//extern pascal void MyNumToString(long theNum,Str255 theString)
// TWOWORDINLINE(0x4267, 0xA9EE);
//#else
#define	MyNumToString	NumToString
//#endif

/* WantThisHandleSafe locks high the handle and returns the previous state */

#if GENERATING68K
#pragma parameter __D0 WantThisHandleSafe(__A0)
extern pascal SignedByte WantThisHandleSafe(Handle myH) = {0xA069, 0x1F00, 0xA064, 0xA029, 0x101F};
#else
SignedByte WantThisHandleSafe(Handle myH);
#endif

/* ClearHiliteMode clears the HiliteMode bit with BCLR */

/*pascal void ClearHiliteMode(void) = {0x08B8, 0x0007, 0x0938};
*/

#define numOfLongs(s)	(((s)>>2) - 1L)

#if GENERATING68K
#pragma parameter MyZeroBuffer(__A0,__D1)
extern pascal void MyZeroBuffer(long *p, long s) = {0x7000, 0x20C0, 0x51C9, 0xFFFC };

#pragma parameter MyFillBuffer(__A0,__D1,__D0)
extern pascal void MyFillBuffer(long *p, long s, long filler) = {0x20C0, 0x51C9, 0xFFFC };

#pragma parameter __D0 mySwap(__D0)
extern pascal long mySwap(long s) = {0x4840 };
#else
void MyZeroBuffer(long *p, long s);
void MyFillBuffer(long *p, long s, long filler);
long mySwap(long s);
#endif


#define toMenu(m, i)	(((long)m << 16) + i)
#define	CHARFROMMESSAGE(mess)	(*(((unsigned char *)(&(mess))) + 3))
#define ISAPPWINDOW(w)	(w && ((WindowPeek) w)->windowKind >= userKind)
#define ISDAWINDOW(w)	(w && ((WindowPeek) w)->windowKind < 0)
#define isMovableModal(w)	(GetWVariant(w) == movableDBoxProc)

#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define MIN(a, b)	((a) < (b) ? (a) : (b))


Handle NewHandleGeneral(Size blockSize);
