#ifndef H_macPrint
#define H_macPrint

/* Copyright (C) 1993 Aladdin Enterprises.  All rights reserved.
   Distributed by Free Software Foundation, Inc.

This file is part of Ghostscript.

Ghostscript is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the Ghostscript General Public License for full details.

Everyone is granted permission to copy, modify and redistribute
Ghostscript, but only under the conditions described in the Ghostscript
General Public License.  A copy of this license is supposed to have been
given to you along with Ghostscript so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.  */


#ifndef lint
static char macPrint_h_rcs_id[] = "$Header:$";
#endif /* lint */

#include <PrintTraps.h>


typedef Boolean (*PrintProcPtr) (short iAction, ...);

enum PrintProcConsts
{
	iDoInitPrintDoc,
	iDoCheckThisPage,
	iDoPrintThisPage,

	__iLastPrintProcConst__
};


OSErr	MacGetRslData (TGetRslBlk *pGetRslBlk);
OSErr	MacSetRslData (short iXRsl, short iYRsl);
void	MacDoPrStyleDlg (void);
void	MacPrintOnePage (PicHandle hPict, Rect *pRect);
void	MacGetPrintRect (short iXRsl, short iYRsl, Rect *pRect);
void	MacPrintPages (short iXRsl, short iYRsl, PrintProcPtr pPrintProc);

#endif /* H_macPrint */
