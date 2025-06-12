/* Code adapted from Tech Note 95. */


/* NOTE: Apple reserves the top half of the screen (where the current DITL
	items are located). Applications may use the bottom half of the screen
	to add items, but should not change any items in the top half of the
	screen.  An application should expand the print dialogs only as much
	as is absolutely necessary.
*/

/* Note: A global search and replace of 'Job' with 'Stl' will produce 
	code that modifies the style dialogs */

#include <PrintTraps.h>

#include "mac_printing.h"
#define PRJOB_DITL_ID		2323
#define EDIT_ITEM		2	/* Second new dialog item is editable text. */
#define	JobDlgID		-8191	/* resource ID of 'Job' or 'Print' DLOG, DITL, & hdlg		 */
#define Myhdlg 			8191	/* resource ID of my hdlg to be spliced on to job/stl dialog */

extern int	g_printer_dpi;		/* dpi set in mac-specific */
extern THPrint	g_print_rec_h;	/* initialized in mac-specific */

Boolean	get_bool_resource( char *rsrc_name ); /* declared in mac-specific */
void Append2hdlg( short srcResID, short dstResID );

static TPPrDlg PrtJobDialog;		/* pointer to job dialog */

/*	This points to the following structure:
	
	  struct { 
		  DialogRecord	Dlg;	  	(The Dialog window)
		  ProcPtr		pFltrProc;	(The Filter Proc.)
		  ProcPtr		pItemProc;	(The Item evaluating proc. -- 							we'll change this)
		  THPrint		hPrintUsr;	(The user's print record.)
		  Boolean		fDoIt;	
		  Boolean		fDone;	
		  	(Four longs -- reserved by Apple Computer)
		  long			lUser1;		 
		  long			lUser2;		
		  long			lUser3;	
		  long			lUser4;		
   } TPrDlg; *TPPrDlg;  		
*/



/*	Declare ÔpascalÕ functions and procedures */
pascal TPPrDlg MyJobDlgInit(THPrint hp);	/* Our extention to PrJobInit */
pascal void MyJobItems(TPPrDlg d, short item); /* Our modal item handler */

long prFirstItem;				/* save our first item here */
long prPItemProc;			/* we need to store the old itemProc here */

/*-----------------------------------------------------------------------*/

OSErr Special_job_dialog( THPrint hPrintRec )
/* Input: a handle to a print record, already allocated and modified.
   Output:  a string containing the "other commands" and an error code. 
*/
{
	PrValidate(hPrintRec);
	if (PrError() != noErr)
		return PrError();		

	/* call PrJobInit to get pointer to the invisible job dialog */
	PrtJobDialog = PrJobInit(hPrintRec);
	if (PrError() != noErr)
		return PrError();		

	Append2hdlg(Myhdlg, JobDlgID);

	if (!PrDlgMain(hPrintRec, &MyJobDlgInit))	/* this line does all the 
	stuff */
		return Cancel;

	if (PrError() != noErr)
		return PrError();		
	else
		return( noErr );
/* that's all for now */
		
} /* Special_job_dialog */

/*------------------------------------------------------------------------*/

pascal TPPrDlg MyJobDlgInit (hPrint)
THPrint hPrint;
/* this routine appends items to the standard job dialog and sets up the
	user fields of the printing dialog record TPRDlg 
	This routine will be called by PrDlgMain */
{
	short		firstItem;		/* first new item number */
		
	firstItem = AppendDITL ((DialogPtr)PrtJobDialog,
		PRJOB_DITL_ID); /*call routine to do this */
	
	prFirstItem = firstItem; /* save this so MyJobItems can find it */
	
/* Now comes the part where we patch in our item handler.  We have to save
	the old item handler address, so we can call it if one of the standard items is hit, and put our item handler's address
	in pItemProc field of the TPrDlg struct
*/

	prPItemProc = (long)PrtJobDialog->pItemProc;
	
/* Now we'll tell the modal item handler where our routine is */
	PrtJobDialog->pItemProc = (ProcPtr)&MyJobItems;

	
/* PrDlgMain expects a pointer to the modified dialog to be returned.... */
	return PrtJobDialog;
	
} /*myJobDlgInit*/


/*-----------------------------------------------------------------------*/

/* here's the analogue to the SF dialog hook */

pascal void MyJobItems(TPPrDlg theDialog, short itemNo)
{	
	short		itemType;		/* needed for GetDItem/SetDItem call */
	Handle		itemH;
	Rect		itemBox;
	
	if (itemNo == 1) /* OK */
	{
		GetDItem( (DialogPtr)theDialog, EDIT_ITEM + prFirstItem - 1,
			&itemType, &itemH, &itemBox );
		GetIText( itemH, g_other_commands );
	}
	CallPascal( theDialog, itemNo, prPItemProc );
} /* MyJobItems */

