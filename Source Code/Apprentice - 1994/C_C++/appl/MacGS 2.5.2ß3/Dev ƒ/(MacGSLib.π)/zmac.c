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


/* zmac.c */
/* Macintosh operators for Ghostscript */
#include "ghost.h"
#include "errors.h"
#include "oper.h"
#include "stream.h"
#include "estack.h"
#include "store.h"
#include "files.h"
#include "gsmatrix.h"			/* for gxdevice.h */
#include "gxdevice.h"
#include "gxdevmem.h"


#define MAC_GS_VERSION		2523


/* .macconfirm 		-  ->  true	(Resume)	*/
/*						or false (Abort)	*/

	private int
zMacConfirm (register os_ptr op)

{
	Boolean			fWasResume;
	extern	Boolean	doMacConfirm (void);


	fWasResume = doMacConfirm ();

	push (1);
	make_bool (op, fWasResume ? 1 : 0);

	return 0;
}


/* .macopenfile 		-  ->  file true	*/
/*							or false		*/

	private int
zMacOpenFile (register os_ptr op)

{
	ref				runFile;
	Boolean			fHaveFile;
	extern	short	doMacOpenFile (ref *pRunFile);


	if (fHaveFile = (doMacOpenFile (&runFile) == 0))
	{
		push (1);
		*op = runFile;
		SetCursor (*GetCursor (watchCursor));
	}

	push (1);
	make_bool (op, fHaveFile ? 1 : 0);

	return 0;
}


/* .macGSversion 		-  ->  version	*/

	private int
zMacGSversion (register os_ptr op)

{
	push (1);
	make_int (op, MAC_GS_VERSION);

	return 0;
}


/*	.macAlert			string  ->	true	(alert displayed)	*/
/*					or	int		->	boolean						*/

	private int
zMacAlert (register os_ptr op)

{
	Boolean	fSuccess = FALSE;
	Boolean gp_mac_alert (char *msg, short lenStr, short index);


	switch (r_type (op))
	{
		default:
			return e_typecheck;

		case t_integer:
			if (op->value.intval < 1)
				return e_undefinedresult;
			else
				fSuccess = gp_mac_alert ((char *) NULL, 0, op->value.intval);

			break;

		case t_string:
			fSuccess = gp_mac_alert ((char *) op->value.bytes, r_size (op), 0);
			
			break;
	}

	make_bool (op, fSuccess ? 1 : 0);

	return 0;
}


/* ------ Initialization procedure ------ */

op_def zmac_op_defs[] =
{
	{"0.macconfirm"  , zMacConfirm},
	{"0.macopenfile" , zMacOpenFile},
	{"0.macGSversion", zMacGSversion},
	{"1.macalert"	 , zMacAlert},
	op_def_end (0)
};


