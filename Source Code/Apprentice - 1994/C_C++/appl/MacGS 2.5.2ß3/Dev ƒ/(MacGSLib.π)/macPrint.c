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

#include <stdio.h>
#include "macPrint.h"


static THPrint	hPrint = (THPrint) NULL;

void			MacOpenPrint (void);
void			MacClosePrint (void);


	static void
MacOpenPrint (void)

{
	if (hPrint == (THPrint) NULL)
	{
		hPrint = (THPrint) NewHandle ((Size) sizeof (TPrint));

		if (hPrint /* != (THPrint) NULL */)
		{
			TGetRslBlk getRslBlk;


			PrOpen ();

			PrintDefault (hPrint);

			getRslBlk.iOpCode = getRslDataOp;
			PrGeneral ((Ptr) &getRslBlk);

			if (PrError () == noErr)
			{
			}
		}
	}
}


	static void
MacClosePrint (void)

{
	if (hPrint /* != (THPrint) NULL */)
	{
		DisposHandle ((Handle) hPrint);
		hPrint == (THPrint) NULL;

		PrClose ();
	}
}


	void
MacDoPrStyleDlg (void)

{
	MacOpenPrint ();

	InitCursor ();

	(void) PrStlDialog (hPrint);
}


	void
MacPrintOnePage (PicHandle hPicture, Rect *pRect)

{
	TPPrPort	hPrPort;


	if (hPicture == (PicHandle) NULL || pRect == (Rect *) NULL)
		goto premature_exit;

	MacOpenPrint ();

	InitCursor ();

	if (!PrJobDialog (hPrint))
		goto premature_exit;

	if ((hPrPort = PrOpenDoc (hPrint, (TPPrPort) NULL, (Ptr) NULL))
			/* != (TPPrPort) NULL */)
	{
		PrOpenPage (hPrPort, (Rect *) NULL);

		if (PrError () == noErr)
			DrawPicture (hPicture, pRect);

		PrClosePage (hPrPort);
	}

	PrCloseDoc (hPrPort);

	if ((*hPrint)->prJob.bJDocLoop == bSpoolLoop)
	{
		TPrStatus	prStatus;


		PrPicFile (hPrint, (TPPrPort) NULL, (Ptr) NULL, (Ptr) NULL, &prStatus);
	}

premature_exit:

	MacClosePrint ();
}


	OSErr
MacGetRslData (TGetRslBlk *pGetRslBlk)

{
	pGetRslBlk->iOpCode = getRslDataOp;
	PrGeneral ((Ptr) pGetRslBlk);

	return PrError ();
}


	OSErr
MacSetRslData (short iXRsl, short iYRsl)

{
	TSetRslBlk	setRslBlk;


	setRslBlk.iOpCode = setRslOp;
	setRslBlk.hPrint  = hPrint;
	setRslBlk.iXRsl	  = iXRsl;
	setRslBlk.iYRsl	  = iYRsl;
	PrGeneral ((Ptr) &setRslBlk);
	(void) PrValidate (hPrint);

	return PrError ();
}


	void
MacGetPrintRect (short iXRsl, short iYRsl, Rect *pRect)

{
	Boolean	fHaveHPrint = (hPrint != (THPrint) NULL);


	if (!fHaveHPrint)
		MacOpenPrint ();

	if (hPrint /* != (THPrint) NIL */)
	{
		(void) MacSetRslData (iXRsl, iYRsl);
		*pRect = (*hPrint)->prInfo.rPage;
	}

	if (!fHaveHPrint)
		MacClosePrint ();
}


	void
MacPrintPages (short iXRsl, short iYRsl, PrintProcPtr pPrintProc)

{
	TPPrPort	hPrPort;


	MacOpenPrint ();

	InitCursor ();

	if (!PrJobDialog (hPrint) ||
		MacSetRslData (iXRsl, iYRsl) /* != noErr */)
		goto premature_exit;

	(void) (*pPrintProc) (iDoInitPrintDoc, &(*hPrint)->prJob);

	if ((hPrPort = PrOpenDoc (hPrint, (TPPrPort) NULL, (Ptr) NULL))
			/* != (TPPrPort) NULL */)
	{
		while ((*pPrintProc) (iDoCheckThisPage))
		{
			PrOpenPage (hPrPort, (Rect *) NULL);

			if (PrError () == noErr)
				(void) (*pPrintProc) (iDoPrintThisPage);

			PrClosePage (hPrPort);
		}
	}

	PrCloseDoc (hPrPort);

	if ((*hPrint)->prJob.bJDocLoop == bSpoolLoop)
	{
		TPrStatus	prStatus;


		PrPicFile (hPrint, (TPPrPort) NULL, (Ptr) NULL, (Ptr) NULL, &prStatus);
	}

premature_exit:

	MacClosePrint ();
	(void) MacSetRslData (0, 0);
}
