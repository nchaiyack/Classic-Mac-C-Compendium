#define JDEB 0
#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "egblvars.h"
#include "m72.h"
#include "mac-specific.h"
#include "Scale_rect.h"
#include "Copy_banded.h"

void rollbeachball( void );
/**********************************************************************/
/****************************** prtpage *******************************/
/**********************************************************************/

void
prtpage(bytepos)		/* print page whose BOP is at bytepos */
long bytepos;

{
    struct stack_entry
    {
		INT32 h;
		COORDINATE hh;
		INT32 v;
		COORDINATE vv;
		INT32 w, x, y, z;	/* what's on stack */
    };
    register BYTE command;  /* current command				*/
    register INT16 i;	    /* command parameter; loop index		*/
    char tc;		    /* temporary character			*/
    UNSIGN32 ht_rule;	    /* rule height                              */
    UNSIGN32 wd_rule;	    /* rule width                               */
    INT32 k,m;		    /* temporary parameter			*/
    BOOLEAN seen_bop;	    /* flag for noting processing of BOP	*/
    register INT16 sp;	    /* stack pointer				*/
    struct stack_entry stack[STACKSIZE];    /* stack			*/
    char specstr[MAXSPECIAL+1];		/* \special{} string		*/
    INT32 w;		    /* current horizontal spacing		*/
    INT32 x;		    /* current horizontal spacing		*/
    INT32 y;		    /* current vertical spacing			*/
    INT32 z;		    /* current vertical spacing			*/

	Rect	dest, print_dest, preview_dest;
	
	if (g_draw_offscreen)
	{
		SetPort( &g_offscreen_GrafPort );
		EraseRect( &g_offscreen_GrafPort.portRect );
	}
	else
	{
		SetPort( g_page_window );
		EraseRect( &g_page_window->portRect );
	}
	SetPort( (GrafPtr) g_print_port_p );
	PrOpenPage( g_print_port_p, nil );
	g_print_status = 3;
	if (PrError() != noErr)
	{
		printf("PrOpenPage error %d.\n", PrError());
		/* (void)fflush(stdout); */
	}
	(**g_print_rec_h).prJob.pIdleProc = (ProcPtr)Printer_idle;
	if (!g_draw_offscreen)
		Set_pic_procs();
	if ((**g_print_rec_h).prStl.feed == feedCut) /* refresh cancel dialog */
	{
		DrawDialog( g_cancel_dialog );
		SetPort( (GrafPtr) g_print_port_p );
	}
	if ((**g_print_rec_h).prJob.bJDocLoop == bSpoolLoop)
		/* Tell the ImageWriter driver not to */
		PicComment(1000, 0, nil);	/* ..."thin the bits" */

/***********************************************************************
Process all commands  between the  BOP at bytepos  and the  next BOP  or
POST.  The page is  printed when the  EOP is met,  but font changes  can
also happen between EOP and BOP, so we have to keep going after EOP.
***********************************************************************/

    seen_bop = FALSE;			/* this is first time through */
    (void) FSEEK(dvifp,bytepos,0);	/* start at the desired position */

    for (;;)	/* "infinite" loop - exits when POST or second BOP met */
    {
#if OS_THINKC
		if (User_wants_out())
			return;
#endif /* OS_THINKC */
	command = (BYTE)nosignex(dvifp,(BYTE)1);
	rollbeachball(); /* OS_THINKC */
	switch (command)
	{

	case SET1:
	case SET2:
	case SET3:
	case SET4:
	    (void)setchar((BYTE)nosignex(dvifp,(BYTE)(command-SET1+1)),TRUE);
	    break;

	case SET_RULE:
	    ht_rule = nosignex(dvifp,(BYTE)4);
	    wd_rule = nosignex(dvifp,(BYTE)4);
	    (void)setrule(ht_rule,wd_rule,TRUE);
	    break;

	case PUT1:
	case PUT2:
	case PUT3:
	case PUT4:
	    (void)setchar((BYTE)nosignex(dvifp,(BYTE)(command-PUT1+1)),FALSE);
	    break;

	case PUT_RULE:
	    ht_rule = nosignex(dvifp,(BYTE)4);
	    wd_rule = nosignex(dvifp,(BYTE)4);
	    (void)setrule(ht_rule,wd_rule,FALSE);
	    break;

	case NOP:
	    break;

	case BOP:
	    if (seen_bop)
			goto close_page;			/* we have been here already */
	    seen_bop = TRUE;

	    for (i=0; i<=9; i++)
		tex_counter[i] = (INT32)signex(dvifp,(BYTE)4);

	    if (!quiet)
	    {
	        (void)printf("{%s}",tctos()); /* TeX page counters */
	        /* (void) fflush(stderr); */
	    }
	    (void) nosignex(dvifp,(BYTE)4);	/* skip prev. page ptr */

	    hxxxx = vxxxx = w = x = y = z = 0;
	    hh = lmargin;
	    vv = tmargin;
	    sp = 0;
	    fontptr = (struct font_entry*)NULL;
	    break;

	case EOP:

	    goto close_page;

	case PUSH:
	    if (sp >= STACKSIZE)
	    {
	    	Kill_dvi("prtpage():  stack overflow");
	    	return;
	    }
	    stack[sp].h = hxxxx;
	    stack[sp].hh = hh;
	    stack[sp].v = vxxxx;
	    stack[sp].vv = vv;
	    stack[sp].w = w;
	    stack[sp].x = x;
	    stack[sp].y = y;
	    stack[sp].z = z;
	    sp++;
	    break;

	case POP:
	    --sp;
	    if (sp < 0)
	    {
	    	Kill_dvi("prtpage():  stack underflow");
	    	return;
	    }
	    hxxxx = stack[sp].h;
	    hh = stack[sp].hh;
	    vxxxx = stack[sp].v;
	    vv = stack[sp].vv;
	    w = stack[sp].w;
	    x = stack[sp].x;
	    y = stack[sp].y;
	    z = stack[sp].z;
	    break;

	case RIGHT1:
	case RIGHT2:
	case RIGHT3:
	case RIGHT4:
	    (void)moveover(signex(dvifp,(BYTE)(command-RIGHT1+1)));
	    break;

	case W0:
	    (void)moveover(w);
	    break;

	case W1:
	case W2:
	case W3:
	case W4:
	    w = (INT32)signex(dvifp,(BYTE)(command-W1+1));
	    (void)moveover(w);
	    break;

	case X0:
	    (void)moveover(x);
	    break;

	case X1:
	case X2:
	case X3:
	case X4:
	    x = (INT32)signex(dvifp,(BYTE)(command-X1+1));
	    (void)moveover(x);
	    break;

	case DOWN1:
	case DOWN2:
	case DOWN3:
	case DOWN4:
	    (void)movedown(signex(dvifp,(BYTE)(command-DOWN1+1)));
	    break;

	case Y0:
	    (void)movedown(y);
	    break;

	case Y1:
	case Y2:
	case Y3:
	case Y4:
	    y = signex(dvifp,(BYTE)(command-Y1+1));
	    (void)movedown(y);
	    break;

	case Z0:
	    (void)movedown(z);
	    break;

	case Z1:
	case Z2:
	case Z3:
	case Z4:
	    z = signex(dvifp,(BYTE)(command-Z1+1));
	    (void)movedown(z);
	    break;

	case FNT1:
	case FNT2:
	case FNT3:
	case FNT4:
	    (void)setfntnm((INT32)nosignex(dvifp,
		(BYTE)(command-FNT1+1)));
		if (g_abort_dvi)
			return;
	    break;

	case XXX1:
	case XXX2:
	case XXX3:
	case XXX4:
	    k = (INT32)nosignex(dvifp,(BYTE)(command-XXX1+1));
	    if (k > MAXSPECIAL)
	    {
		(void)sprintf(message,
		    "prtpage():  \\special{} string of %d characters longer \
than DVI driver limit of %d -- truncated.",
		k,MAXSPECIAL);
		(void)warning(message);
	    }
	    m = 0;
	    while (k--)
	    {
		 tc = (char)nosignex(dvifp,(BYTE)1);
		 if (m < MAXSPECIAL)
		     specstr[m++] = tc;
	    }
	    specstr[m] = '\0';
 	    (void) special(specstr);
 	    /* a PICT might turn off our picture comment? */
		if ((**g_print_rec_h).prJob.bJDocLoop == bSpoolLoop)
			/* Tell the ImageWriter driver not to */
			PicComment(1000, 0, nil);	/* ..."thin the bits" */
	    break;

	case FNT_DEF1:
	case FNT_DEF2:
	case FNT_DEF3:
	case FNT_DEF4:
	    if (preload)
		(void)skipfont ((INT32) nosignex(dvifp,
		    (BYTE)(command-FNT_DEF1+1)));
	    else
		(void)readfont ((INT32) nosignex(dvifp,
		    (BYTE)(command-FNT_DEF1+1)));
	    break;

	case PRE:
	    Kill_dvi("prtpage():  PRE occurs within file");
	    return;

	case POST:
		goto close_page;
#if NOTNOW	/* Should this stuff ever happen? */
	    (void)dviterm();		/* terminate DVI file processing */
	    (void)Close_printer();		/* terminate device output */
	    (void)alldone();		/* this will never return */
#endif
	    break;

	case POST_POST:
	    Kill_dvi("prtpage():  POST_POST with no preceding POST");
	    return;

	default:
	    if (command >= FONT_00 && command <= FONT_63)
	    {
			(void)setfntnm((INT32)(command - FONT_00));
	    	if (g_abort_dvi)
	    		return;
	    }
	    else if (command >= SETC_000 && command <= SETC_127)

#if    (HPJETPLUS | POSTSCRIPT | IMPRESS | CANON_A2)
		(void)setstr((BYTE)command); /* this sets several chars */
#else
		(void)setchar((BYTE)(command-SETC_000), TRUE);
#endif /* (HPJETPLUS | POSTSCRIPT | IMPRESS | CANON_A2) */

	    else
	    {
		(void)sprintf(message,"prtpage():  %d is an undefined command",
		    command);
			Kill_dvi(message);
			return;
	    }
	    break;
	}
    }

close_page:
	if (g_draw_offscreen)
	{
		
		Scale_rect( &g_offscreen_GrafPort.portRect,
			&print_dest, &preview_dest );
		if (g_preview)
		{
			SetPort( g_page_window );
			/* We use srcCopy instead of srcOr here for speed. */
			CopyBits( &g_offscreen_GrafPort.portBits,
				&g_page_window->portBits,
				&g_offscreen_GrafPort.portRect,
				&preview_dest, srcCopy, NIL );
		}
		SetPort( (GrafPtr) g_print_port_p );
		Copy_banded( &g_offscreen_GrafPort.portBits,
			&g_print_port_p->gPort.portBits,
			&g_offscreen_GrafPort.portRect, &print_dest );
	}
	if ((**g_print_rec_h).prJob.bJDocLoop == bSpoolLoop)
		PicComment(1001, 0, nil);
	PrClosePage( g_print_port_p );
	g_print_status = 2;
	if (PrError() != noErr)
	{
		g_abort_dvi = TRUE;
		printf("PrClosePage error %d.\n", PrError());
	}
}
