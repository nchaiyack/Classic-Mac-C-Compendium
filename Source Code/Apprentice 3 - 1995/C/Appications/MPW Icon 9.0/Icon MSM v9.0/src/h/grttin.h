/*
 * Group of include files for input to rtt.
 *   rtt reads these files for preprocessor directives and typedefs, but
 *   does not output any code from them.
 */
#include "::h:define.h"
#if VMS
/* don't need path.h */
#else					/* VMS */
#include "::h:path.h"
#endif					/* VMS */
#include "::h:config.h"
#ifndef NoTypeDefs
#include "::h:typedefs.h"
#endif					/* NoTypeDefs */
#include "::h:version.h"

/*
 * Macros that must be expanded by rtt.
 */

/*
 * Declaration for library routine.
 */
#begdef LibDcl(nm,n,pn)
   #passthru OpBlock(nm,n,pn,0)

   int O##nm(nargs,cargp)
   int nargs;
   register dptr cargp;
#enddef

/*
 * Error exit from non top-level routines. Set tentative values for
 *   error number and error value; these errors will but put in
 *   effect if the run-time error routine is called.
 */
#begdef ReturnErrVal(err_num, offending_val, ret_val)
   {
   t_errornumber = err_num;
   t_errorvalue = offending_val;
   t_have_val = 1;
   return ret_val;
   }
#enddef

#begdef ReturnErrNum(err_num, ret_val)
   {
   t_errornumber = err_num;
   t_errorvalue = nulldesc;
   t_have_val = 0;
   return ret_val;
   }
#enddef

/*
 * Code expansions for exits from C code for top-level routines.
 */
#define Fail		return A_Resume
#define Return		return A_Continue

/*
 * RunErr encapsulates a call to the function err_msg, followed
 *  by Fail.  The idea is to avoid the problem of calling
 *  runerr directly and forgetting that it may actually return.
 */

#define RunErr(n,dp) {\
   err_msg((int)n,dp);\
   Fail;\
   }

/*
 * Protection macro.
 */
#define Protect(notnull,orelse) if ((notnull)==NULL) orelse

/* Get RTT past some OS2-32 types */
#if OS2_32
typedef int HFILE, ULONG;
#endif					/* OS2_32 */

#ifdef EventMon
#define MMMark(block, type)
#define MMSMark(saddr, slen)
#define MMOut(prefix, msg)
#define MMShow(d, colr)
#define evnewline()
#define evcmd(addr, len, c)
#define etvalue(n, c)
#define evdec(n)

/*
 * perform what amounts to "function inlining" of EVVal
 */
#begdef EVVal(value,event)
   {
   if (!is:null(curpstate->eventmask) &&
       Testb((word)ToAscii(event), curpstate->eventmask)
       ) {
      MakeInt(value, &(curpstate->parent->eventval));
      actparent(event);
      }
   }
#enddef					/* EVVal */
#begdef EVValD(dp,event)
   {
   if (!is:null(curpstate->eventmask) &&
       Testb((word)ToAscii(event), curpstate->eventmask)
       ) {
      curpstate->parent->eventval = *(dp);
      actparent(event);
      }
   }
#enddef					/* EVValD */
#begdef EVValX(bp,event)
   {
   struct progstate *parent = curpstate->parent;
   if (!is:null(curpstate->eventmask) &&
       Testb((word)ToAscii(event), curpstate->eventmask)
       ) {
      parent->eventval.dword = D_Coexpr;
      BlkLoc(parent->eventval) = (union block *)(bp);
      actparent(event);
      }
   }
#enddef					/* EVValX */
#if UNIX
#begdef EVTick()
/*
 * EVTick() - record a Tick event reflecting a clock advance.
 *
 *  The interpreter main loop has detected a change in the profile counters.
 *  This means that the system clock has ticked.  Record an event and update
 *  the records.
 */
   {
   word sum, nticks;

   oldtick = ticker.l[0] + ticker.l[1];
   sum = ticker.s[0] + ticker.s[1] + ticker.s[2] + ticker.s[3];
   nticks = sum - oldsum;
   EVVal(nticks, E_Tick);
   oldsum = sum;
   }
#enddef					/* EVTick */
#else					/* UNIX */
#define EVTick()
#endif					/* UNIX */
#define InterpEVVal(arg1,arg2)  { ExInterp; EVVal(arg1,arg2) EntInterp }
#define InterpEVValD(arg1,arg2) { ExInterp; EVValD(arg1,arg2) EntInterp }
#define InterpEVValX(arg1,arg2) { ExInterp; EVValX(arg1,arg2) EntInterp }
#define InterpEVTick()          { ExInterp; EVTick() EntInterp }

/*
 * Macro with construction of event descriptor.
 */

#begdef Desc_EVValD(bp, code, type)
   {
   eventdesc.dword = type;
   eventdesc.vword.bptr = (union block *)(bp);
   EVValD(&eventdesc, code);
   }
#enddef					/* Desc_EVValD */

#else					/* EventMon */
#define EVVal(arg1,arg2)
#define EVValD(arg1,arg2)
#define EVValX(arg1,arg2)
#define EVTick()
#define InterpEVVal(arg1,arg2)
#define InterpEVValD(arg1,arg2)
#define InterpEVValX(arg1,arg2)
#define InterpEVTick()
#define Desc_EVValD(bp, code, type)
#endif					/* EventMon */


#ifdef Graphics
typedef int wbp, wsp, wcp, wdp, wclrp, wfp;
typedef int wbinding, wstate, wcontext, wfont;
typedef int siptr, stringint;
typedef int XRectangle, XPoint, XSegment, XArc, SysColor, LinearColor;
typedef int LONG, SHORT;

#ifdef MacGraph
typedef int Rect, PolyHandle;
#endif					/* MacGraph */

#ifdef XWindows
typedef int XGCValues, XColor, XFontStruct, XWindowAttributes;
typedef int XEvent, XExposeEvent, XKeyEvent, XButtonEvent, XConfigureEvent;
typedef int XSizeHints, XWMHints, XTextProperty;
typedef int Colormap, XVisualInfo;
typedef int *Display, Cursor, GC, Window, Pixmap, Visual, KeySym;
typedef int WidgetClass, XImage, XpmAttributes;
#endif					/* XWindows */

#ifdef MSWindows
typedef int int_PASCAL, LRESULT_CALLBACK, LOGBRUSH, MSG, WORD, DWORD;
typedef int HINSTANCE, LPSTR, HBITMAP, WNDCLASS, PAINTSTRUCT, POINT, RECT;
typedef int HWND, HDC, UINT, WPARAM, LPARAM, HANDLE, HPEN, HBRUSH, SIZE;
#endif					/* MSWindows */

#ifdef PresentationManager
/* OS/2 PM specifics */
typedef int HAB, HPS, QMSG, HMQ, HWND, USHORT, MRESULT, ULONG, MPARAM;
typedef int PFNWP, HMODULE, SHORT, BOOL, TID, RECTL, ERRORID;
typedef int MRESULT_N_EXPENTRY, SIZEL, HDC, POINTL, HMTX, HBITMAP;
typedef int VOID_APIENTRY, UCHAR, HEV;
typedef int LONG, BITMAPINFOHEADER2, PBITMAPINFO2, PSZ, RGB2, BITMAPINFO2;
typedef int FONTMETRICS, PRECTL, PCHARBUNDLE, PLINEBUNDLE, PIMAGEBUNDLE;
typedef int AREABUNDLE, PAREABUNDLE, PPOINTL, POLYGON, CHARBUNDLE, LINEBUNDLE;
typedef int lclIdentifier, BYTE, PBYTE, PRGB2, FATTRS, PFATTRS, PULONG;
typedef int PBITMAPINFOHEADER2, BITMAPFILEHEADER2, BITMAPARRAYFILEHEADER2;
typedef int colorEntry, ARCPARAMS, threadargs, HPOINTER, CURSORINFO;
typedef int PCURSORINFO, DEVOPENSTRUCT, PDEVOPENDATA, SIZEF, HRGN, PSWP;
typedef int va_list, BITMAPINFOHEADER, BITMAPFILEHEADER, BITMAPARRAYFILEHEADER;
typedef int PBITMAPINFOHEADER, MinBitmapHeader, RGB;
#endif					/* PresentationManager */

/*
 * Convenience macros to make up for RTL's long-windedness.
 */
#begdef CnvShortInt(desc, s, max)
	{
	struct descrip tmp;
	if (!cnv:integer(desc,tmp)) runerr(101,desc);
	if ((tmp.dword != D_Integer) || (IntVal(tmp) > max))
	   MakeInt(max, &tmp);
	s = (short) IntVal(tmp);
	}
#enddef					/* CnvShortInt */

#define CnvCShort(d,s) CnvShortInt(d,s,32767)
#define CnvCUShort(d,s) CnvShortInt(d,s,65535)

#define CnvCInteger(d,i) \
  if (!cnv:C_integer(d,i)) runerr(101,d);

#define DefCInteger(d,default,i) \
  if (!def:C_integer(d,default,i)) runerr(101,d);

#define CnvString(din,dout) \
  if (!cnv:string(din,dout)) runerr(103,din);

#define CnvTmpString(din,dout) \
  if (!cnv:tmp_string(din,dout)) runerr(103,din);

/*
 * conventions supporting optional initial window arguments:
 *
 * All routines declare argv[argc] as their parameters
 * Macro OptWindow checks argv[0] and assigns _w_ and warg if it is a window
 * warg serves as a base index and is added everywhere argv is indexed
 * n is used to denote the actual number of "objects" in the call
 * Macro ReturnWindow returns either the initial window argument, or &window
 */
#begdef OptWindow(w)
   if (argc>warg && is:file(argv[warg])) {
      if ((BlkLoc(argv[warg])->file.status & Fs_Window) == 0)
	 runerr(140,argv[warg]);
      if ((BlkLoc(argv[warg])->file.status & (Fs_Read|Fs_Write)) == 0)
	 runerr(142,argv[warg]);
      (w) = (wbp)BlkLoc(argv[warg])->file.fd;
      warg++;
      }
   else {
      if (!(is:file(kywd_xwin[XKey_Window]) &&
	    (BlkLoc(kywd_xwin[XKey_Window])->file.status & Fs_Window)))
	runerr(140,kywd_xwin[XKey_Window]);
      if (!(BlkLoc(kywd_xwin[XKey_Window])->file.status & (Fs_Read|Fs_Write)))
	 runerr(142,kywd_xwin[XKey_Window]);
      w = (wbp)BlkLoc(kywd_xwin[XKey_Window])->file.fd;
      }
#enddef					/* OptWindow */

#begdef ReturnWindow
      if (!warg) return kywd_xwin[XKey_Window];
      else return argv[0]
#enddef					/* ReturnWindow */

#begdef CheckArgMultiple(mult)
{
  if ((argc-warg) % (mult)) runerr(146);
  n = (argc-warg)/mult;
  if (!n) runerr(146);
}
#enddef					/* CheckArgMultiple */

/*
 * calloc to make sure uninit'd entries are zeroed.
 */
#begdef GRFX_ALLOC(var,type)
   var = (struct type *)calloc(1, sizeof(struct type));
   if (var == NULL) ReturnErrNum(305, NULL);
   var->refcount = 1;
#enddef					/* GRFX_ALLOC */

#begdef GRFX_LINK(var, chain)
   var->next = chain;
   var->previous = NULL;
   if (chain) chain->previous = var;
   chain = var;
#enddef					/* GRFX_LINK */

#begdef GRFX_UNLINK(var, chain)
   if (var->previous) var->previous->next = var->next;
   else chain = var->next;
   if (var->next) var->next->previous = var->previous;
   free(var);
#enddef					/* GRFX_UNLINK */

#endif					/* Graphics */
