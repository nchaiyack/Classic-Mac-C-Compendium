/*
*  File fdlog.c
*
*  Take care of the dialog actions required for NCSA DataScope.
*
*/


# define aboutMeDLOG	139
# define openDLOG  		131
# define attrDLOG  		129
# define nomemDLOG  	132
# define pattDLOG  		133
# define isizeDLOG  	135

#define okButton 		1
#define aboutHELP		2
#define aboutOK			1

#include "macfview.h"
#include "DSversion.h"
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	OutlineItem		Outline a dialog item
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
pascal void OutlineItem( theDialog, theItem)
	DialogPtr	theDialog;
	short		theItem;
{
	Rect		ibox;
	char		**ihndl;
	short		typ;
	
	GetDItem( theDialog, theItem, &typ, &ihndl, &ibox);
	PenSize((short) 3,(short) 3);
	InsetRect( &ibox, (short) -4,(short) -4);
	FrameRoundRect( &ibox, (short) 16,(short) 16);
}

/********************************************************************/
/*  checkmem
*   Use CompactMem to see if there is enough memory.  If not, call
*   nomem to inform the user.
*/
checkmem(amount)
	int amount;
	{
	char *p;
	
	if (NULL == (p = (char *)NewPtr(amount))) {		/* try to allocate it */
		nomem();
		return(1);
	}
	
	DisposPtr(p);								/* turn it back */
#ifdef OLDM
	if (amount > CompactMem(amount)) {
		nomem();
		return(1);
	}
#endif

	return(0);
	
}

/********************************************************************/
/*  nomem
*  Inform the user that there is not enough memory for what they
*  want to do.
*/
nomem()
{

	StopAlert(1002, nil);
	
	return;
}

/********************************************************************/
/*  modal dialog filter for drawing the viewport rectangle.
*/

#define hoffset 30
#define voffset 55

#define pOK 1
#define pCANCEL 15
#define pnorth 2
#define psouth 4
#define peast 5
#define pwest 6
#define pexpand 9
#define pwidth 10
#define pheight 8
#define pdec 13
#define pinc 14
#define pcircle 27
#define poutl 16
#define pincircle 20

#define igOK 1
#define igCANCEL 10
#define ighexpand 6
#define igvexpand 14
#define igwidth 7
#define igheight 5
#define ighdec 8
#define ighinc 9
#define igvdec 12
#define igvinc 13
#define igbox 27
#define igoutl 2
#define iginbox 14


Rect prect;

/********************************************************************/
pascal void drawview( dlog, itm)
	DialogPtr dlog;
	short itm;
{

	frameit();
}

frameit()
{
	Rect tr;
	
	tr = prect;
	OffsetRect(&tr,hoffset,voffset);
	
	PenSize(3,3);
	PenMode(patXor);
	FrameRect(&tr);					/* draw a box */

	PenMode(patCopy);
}

/********************************************************************/
pascal Boolean
polardraw(td,tdevent,itemhit)
	DialogPtr td;
	EventRecord *tdevent;
	short int *itemhit;
	{
	Point		where,newpt,anchorp;
	int c;
	
	switch(tdevent->what) {
		case keyDown:
		case autoKey:
			if ((c = (tdevent->message & charCodeMask)) == 13) {
				*itemhit = 1;
				return(true);			/* else let dialog handle it */
			}
			break;
			
		case mouseDown:
			where = tdevent->where;						/* anchor point */
			where.h -= hoffset;						/* position within frame */
			where.v -= voffset;
			
			GlobalToLocal(&where);						/* Get location */
			if (where.h < 220 && where.v < 220) {
				if (where.h < 0) where.h = 0;
				if (where.v < 0) where.v = 0;
				if (where.h > 202) where.h = 202;			/* protect the range */
				if (where.v > 202) where.v = 202;
					
				frameit();							/* un-draw a box */
				SetRect(&prect, where.h, where.v, where.h, where.v);
				anchorp = where;							/* save anchor */
				while (StillDown()) {
					GetMouse(&newpt);
					newpt.h -= hoffset;
					newpt.v -= voffset;
					
					if (newpt.h < 0) newpt.h = 0;
					if (newpt.v < 0) newpt.v = 0;
					if (newpt.h > 202) newpt.h = 202;
					if (newpt.v > 202) newpt.v = 202;
					if (where != newpt) {	/* something changed */
						where = newpt;
						PenMode(patXor);
						frameit();					/* un-draw a box */
						
						if (newpt.v <= anchorp.v) {			/* above, move bottom to anchor, top to point */
							prect.bottom = anchorp.v;
							prect.top = newpt.v;
						}
						else {								/* below, move top to anchor, bottom to point */
							prect.top = anchorp.v;
							prect.bottom = newpt.v;
						}
						if (newpt.h <= anchorp.h) {			/* left, move right to anchor, left to point */
							prect.right = anchorp.h;
							prect.left = newpt.h;
						}
						else {
							prect.left = anchorp.h;
							prect.right = newpt.h;
						}
						
						frameit();					/* draw a new box */
					}
				}
				*itemhit=pcircle;
				return(true);
			}
			break;
		default:
			break;
	}
	
	*itemhit=0;
	return(false);				

}

/********************************************************************/
/*  polar attributes
*   Display and work on the polar attributes dialog.
*/
dopatt(tw)
	struct Mwin *tw;
	{
	DialogPtr	theDialog;
	short		itemType;
	Handle		itemHdl;
	Rect		itemRect,tr;
	short		itemHit;
	GrafPtr		savep;
	int 	i,denom,ashift,exx;
	char  temp[40];
	
	GetPort(&savep);
		
	theDialog = GetNewDialog(pattDLOG, nil, (WindowPtr) -1);
	
	SetPort(theDialog);

	UItemAssign( theDialog, (short) pincircle, drawview);
	UItemAssign( theDialog, (short) poutl, OutlineItem);

	tr = tw->dat->viewport;
	denom = tw->dat->ydim;
	
	prect.top = 100 + 200*tr.top/denom;
	prect.left = 100 + 200*tr.left/denom;
	prect.right = 100 + 200*tr.right/denom;
	prect.bottom = 100 + 200*tr.bottom/denom;
		
	exx = tw->dat->exx;
	ashift = tw->dat->angleshift;
	
	GetDItem(theDialog, pexpand, &itemType, &itemHdl, &itemRect);
	sprintf(temp,"%d",exx);
	setitext(itemHdl, temp);

	do {
		PenMode(patCopy);
		GetDItem(theDialog, pnorth, &itemType, &itemHdl, &itemRect);	/* radio buttons */
		SetCtlValue( (ControlHandle) itemHdl, ashift == 3 ? 1 : 0);
		GetDItem(theDialog, psouth, &itemType, &itemHdl, &itemRect);
		SetCtlValue( (ControlHandle) itemHdl, ashift == 1 ? 1 : 0);
		GetDItem(theDialog, peast, &itemType, &itemHdl, &itemRect);
		SetCtlValue( (ControlHandle) itemHdl, ashift == 0 ? 1 : 0);
		GetDItem(theDialog, pwest, &itemType, &itemHdl, &itemRect);
		SetCtlValue( (ControlHandle) itemHdl, ashift == 2 ? 1 : 0);
		
		i = (prect.right - prect.left)*denom/200*exx;
		GetDItem(theDialog, pwidth, &itemType, &itemHdl, &itemRect);
		sprintf(temp,"%d",i);
		setitext(itemHdl, temp);
		i = (prect.bottom - prect.top)*denom/200*exx;
		GetDItem(theDialog, pheight, &itemType, &itemHdl, &itemRect);
		sprintf(temp,"%d",i);
		setitext(itemHdl, temp);

		ModalDialog(polardraw, &itemHit);
		switch (itemHit) {
		case pCANCEL:
			DisposDialog(theDialog);
			SetPort(savep);
			return(0);
		case pnorth:
			ashift = 3;
			break;
		case peast:
			ashift = 0;
			break;
		case pwest:
			ashift = 2;
			break;
		case psouth:
			ashift = 1;
			break;
		case pinc:
			GetDItem(theDialog, pexpand, &itemType, &itemHdl, &itemRect);
			sprintf(temp,"%d",++exx);
			setitext(itemHdl, temp);
			break;
		case pdec:
			if (exx <= 1) break;
			GetDItem(theDialog, pexpand, &itemType, &itemHdl, &itemRect);
			sprintf(temp,"%d",--exx);
			setitext(itemHdl, temp);
			break;
		case pexpand:
			GetDItem(theDialog, pexpand, &itemType, &itemHdl, &itemRect);
			getitext(itemHdl, temp);
			exx = max(1,atoi(temp));
			sprintf(temp,"%d",exx);
			setitext(itemHdl, temp);
			break;			
		case 1:
			itemHit = pOK;
			break;
		default:
			break;
		}
		
	} while (itemHit != pOK);

	tw->dat->angleshift = ashift;
	tw->dat->exx = tw->dat->exy = exx;
	
	DisposDialog(theDialog);
	
	tr.top = (prect.top - 100)*denom/200;
	tr.left = (prect.left - 100)*denom/200;
	tr.right = (prect.right - 100)*denom/200;
	tr.bottom = (prect.bottom - 100)*denom/200;
	tw->dat->viewport = tr;
	
	SetPort(savep);
}

/********************************************************************/
/*  Image size attributes
*   Display and work on the image size dialog.
*/
dosize(tw)
	struct Mwin *tw;
	{
	DialogPtr	theDialog;
	short		itemType;
	Handle		itemHdl;
	Rect		itemRect;
	short		itemHit;
	GrafPtr		savep;
	int 	exx,exy,xbig,ybig;
	char  temp[40];
	
	GetPort(&savep);
		
	theDialog = GetNewDialog(isizeDLOG, nil, (WindowPtr) -1);
	
	SetPort(theDialog);

	UItemAssign( theDialog, (short) igoutl, OutlineItem);

	xbig = tw->dat->xsize;
	ybig = tw->dat->ysize;
		
	exx = tw->dat->exx;
	exy = tw->dat->exy;
	
	GetDItem(theDialog, igwidth, &itemType, &itemHdl, &itemRect);
	sprintf(temp,"%d",xbig);
	setitext(itemHdl, temp);
	GetDItem(theDialog, igheight, &itemType, &itemHdl, &itemRect);
	sprintf(temp,"%d",ybig);
	setitext(itemHdl, temp);
	GetDItem(theDialog, ighexpand, &itemType, &itemHdl, &itemRect);
	sprintf(temp,"%d",exx);
	setitext(itemHdl, temp);
	GetDItem(theDialog, igvexpand, &itemType, &itemHdl, &itemRect);
	sprintf(temp,"%d",exy);
	setitext(itemHdl, temp);

	do {
		PenMode(patCopy);
		
		ModalDialog(nil, &itemHit);
		
		switch (itemHit) {
		case igCANCEL:
			DisposDialog(theDialog);
			SetPort(savep);
			return(0);
		case ighinc:
			GetDItem(theDialog, ighexpand, &itemType, &itemHdl, &itemRect);
			sprintf(temp,"%d",++exx);
			setitext(itemHdl, temp);
			break;
		case ighdec:
			if (exx <= 1) break;
			GetDItem(theDialog, ighexpand, &itemType, &itemHdl, &itemRect);
			sprintf(temp,"%d",--exx);
			setitext(itemHdl, temp);
			break;
		case ighexpand:
			GetDItem(theDialog, ighexpand, &itemType, &itemHdl, &itemRect);
			getitext(itemHdl, temp);
			exx = max(1,atoi(temp));
			break;			
		case igvinc:
			GetDItem(theDialog, igvexpand, &itemType, &itemHdl, &itemRect);
			sprintf(temp,"%d",++exy);
			setitext(itemHdl, temp);
			break;
		case igvdec:
			if (exy <= 1) break;
			GetDItem(theDialog, igvexpand, &itemType, &itemHdl, &itemRect);
			sprintf(temp,"%d",--exy);
			setitext(itemHdl, temp);
			break;
		case igvexpand:
			GetDItem(theDialog, igvexpand, &itemType, &itemHdl, &itemRect);
			getitext(itemHdl, temp);
			exy = max(1,atoi(temp));
			break;			
		case igwidth:
			GetDItem(theDialog, igwidth, &itemType, &itemHdl, &itemRect);
			getitext(itemHdl, temp);
			xbig = max(tw->dat->xdim,atoi(temp));
			exx = xbig / tw->dat->xdim;
			GetDItem(theDialog, ighexpand, &itemType, &itemHdl, &itemRect);
			sprintf(temp,"%d",exx);
			setitext(itemHdl, temp);
			break;
		case igheight:
			GetDItem(theDialog, igheight, &itemType, &itemHdl, &itemRect);
			getitext(itemHdl, temp);
			ybig = max(tw->dat->ydim,atoi(temp));
			exy = ybig / tw->dat->ydim;
			GetDItem(theDialog, igvexpand, &itemType, &itemHdl, &itemRect);
			sprintf(temp,"%d",exy);
			setitext(itemHdl, temp);
			break;
		case 1:
			itemHit = igOK;
			break;
		default:
			break;
		}

/*
*  adjust dependent fields, depending on what was hit.
*/
		switch (itemHit) {
		case ighexpand:
		case ighdec:
		case ighinc:
			xbig = exx * tw->dat->xdim;
			GetDItem(theDialog, igwidth, &itemType, &itemHdl, &itemRect);
			sprintf(temp,"%d",xbig);
			setitext(itemHdl, temp);
			GetDItem(theDialog, ighexpand, &itemType, &itemHdl, &itemRect);
			sprintf(temp,"%d",exx);
			setitext(itemHdl, temp);
			break;
		case igvexpand:
		case igvdec:
		case igvinc:
			ybig = exy * tw->dat->ydim;
			GetDItem(theDialog, igheight, &itemType, &itemHdl, &itemRect);
			sprintf(temp,"%d",ybig);
			setitext(itemHdl, temp);
			GetDItem(theDialog, igvexpand, &itemType, &itemHdl, &itemRect);
			sprintf(temp,"%d",exy);
			setitext(itemHdl, temp);
			break;
		}
		
	} while (itemHit != igOK);

	tw->dat->exx = exx;
	tw->dat->exy = exy;
	tw->dat->xsize = xbig;
	tw->dat->ysize = ybig;
	
	DisposDialog(theDialog);
		
	SetPort(savep);
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	openingdialog		Show and remove the openingdialog
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
DialogPtr	openDialog;
openingdialog(mode)
	int			mode;
{
	pascal void VersionNumber();

	if (mode) 
		{
		Rect		ibox;
		char		**ihndl;
		short		typ;
		openDialog = GetNewDialog(openDLOG, nil, (WindowPtr) -1);

		GetDItem(openDialog, 3, &typ, &ihndl, &ibox);
		setitext(ihndl, VERSION);
		DrawDialog(openDialog);
	}
	else
		DisposDialog(openDialog);

	return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	showAboutMeDialog		standard about dialog processing
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
showAboutMeDialog()
{
	DialogPtr	theDialog;
	GrafPtr		savePort;
	short		itemHit;
	pascal void VersionNumber(),
				OutlineItem();
	extern char netfailed;
	Boolean		done = false;
	
	if (netfailed)
		paramtext("Network Disabled","","","");
	else
		paramtext("Network Enabled","","","");

	GetPort(&savePort);
	theDialog = GetNewDialog(aboutMeDLOG, nil, (WindowPtr) -1);
	SetPort(theDialog);
	UItemAssign(theDialog,(short)(aboutOK), OutlineItem);
	UItemAssign(theDialog,(short) 3, VersionNumber);

	do
	{
		ModalDialog(NULL, &itemHit);
		   if		(itemHit == aboutOK)
		   			{DisposDialog(theDialog);
					 done = true;
					}
		   else if	(itemHit == aboutHELP)
		   			{DisposDialog(theDialog);
					 done = true;
					 ShowHelp();
					}
		   else;
	}
	while (done == false);
	
	SetPort(savePort);
	return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ShowHelp		Access the Help Compiler routine which
					displays the on-line help.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
#include "resources.h"

ShowHelp()
{
	Handle	rsrcH;
	ProcPtr pp;

	if (rsrcH = GetResource('HCOD', 2000))
	{
		HLock(rsrcH);
		pp = (ProcPtr) *rsrcH;
		(*pp)();
		HUnlock(rsrcH);
		ReleaseResource(rsrcH);
	}
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	VersionNumber		Prints a version number string into a
						dialog box.  Requires an item number
						to be present to define where to
						center the string.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
pascal void VersionNumber( theDialog, theItem)
	DialogPtr	theDialog;
	short		theItem;
{
	Rect		ibox;
	char		**ihndl;
	short		typ;

	GetDItem( theDialog, (int)(theItem), &typ, &ihndl, &ibox);
	TextFont( 3);
	TextSize( 9);
	MoveTo( ibox.left					 +
			((ibox.right-ibox.left) / 2) -
			(stringwidth( VERSION)/2),
			ibox.bottom-2);
	drawstring( VERSION );
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	UItemAssign
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
UItemAssign( dlog, item, proc)
	DialogPtr	dlog;
	short			item;
	int			(*proc)();
{
	Rect		ibox;
	char		**ihndl;
	short		typ;

	GetDItem( dlog, item, &typ, &ihndl, &ibox);
	SetDItem( dlog, item,  typ,  (Handle) proc, &ibox);
}

/***************************************************************************/
/*  checksave
*   See if the window which is about to go away really should be saved first.
*/
checksave(tw)
	struct Mwin *tw;
	{
	DialogPtr	theDialog;
	short itemHit;

	if (!tw->dat->needsave)
		return(0);
	
	if (!tw->dat->fname[0])
		paramtext(tw->dat->dvar,"","","");
	else
		paramtext(tw->dat->fname,"","","");

	theDialog = GetNewDialog(134, nil, (WindowPtr) -1);
	
	UItemAssign( theDialog, 4, OutlineItem );
	do {
		ModalDialog(nil, &itemHit);
	} while (itemHit < 1 || itemHit > 5);

	DisposDialog(theDialog);
	
	if (itemHit == 1) {					/* go for the save */
		if (saveit(tw))
			return(-1);
	}
	else if (itemHit == 5)
		return(-1);						/* Cancel return */

	return(0);
}


/***************************************************************************/
/*  findfunction
*   Work with the user to find an external function which may be in another
*   file.  Returns 0 if another attempt should be made, -1 if the user gave up.
*/

extern char netfailed;

findfunction(s)
	char *s;
{
	DialogPtr	theDialog;
	short		itemType;
	Handle		itemHdl;
	Rect		itemRect;
	short itemHit;

	paramtext(s,"","","");

	SetCursor(&qd.arrow);

	theDialog = GetNewDialog(137, nil, (WindowPtr) -1);
	
	UItemAssign( theDialog, 3, OutlineItem );
	
	if (netfailed) {			/* no networking, don't allow this option */
		GetDItem(theDialog, 8, &itemType, &itemHdl, &itemRect);
		HiliteControl((ControlHandle)itemHdl,255);
	}
	
	do {
		ModalDialog(nil, &itemHit);
	} while (itemHit != 1 && itemHit != 2 && itemHit != 8);

	DisposDialog(theDialog);
	
	if (itemHit == 1) {					/* go for the function file */
		 SFReply reply;
		 SFTypeList tlst;
		 Point wh;
		 int ret;
	
		wh.h = wh.v = 50;
		tlst[0] = 'DSff';
		sfgetfile(&wh, "Open External Function Library", nil, 1, tlst, nil, &reply);
		if (reply.good) {
			p2cstr(reply.fName);
			setvol(NULL, reply.vRefNum);			/* set to this volume (dir) */
			ret = openresfile(reply.fName);			/* Open its resource fork */
			if (ret < 0)
				return(-1);
			else
				return(0);
		}
		else
			return(-1);					/* user cancel */
	}
	else if (itemHit == 8) {
		return(1);						/* wants network function */
	}
	
	
	return(-1);							/* Cancel return - back to notebook */

}

/************************************************************************/
/*  remote execution setup.
*   We need to get all of the fields required by rexec because we use
*   the rexec protocol to get going.  
*
*	passwords are not echoed to the screen.
*
*/

char *exuser=NULL,						/* user name */
	*expass=NULL,						/* password */
	*exmachine=NULL;					/* host name */

int 
	exconnect=512;						/* compute port # is rexec */
	
pascal Boolean pfilt();					/* non-echoing modal dialog processor for password */

exremote(fnp)
	char *fnp;
	{
	char s[256];
	DialogPtr	theDialog;
	short		itemType;
	Handle		itemHdl;
	Rect		itemRect;
	short itemHit,i;

	
	if (exuser == NULL) {
		exuser = (char *)NewPtr(256);
		exuser[0] = 0;
	}
	if (exmachine == NULL) {
		exmachine = (char *)NewPtr(256);
		exmachine[0] = 0;
	}
	if (expass == NULL) {
		expass = (char *)NewPtr(256);
		expass[0] = 0;
	}
	
	if (!exuser || !exmachine || !expass)
		return(-1);
		
	paramtext(fnp,"","","");

	SetCursor(&qd.arrow);

	theDialog = GetNewDialog(138, nil, (WindowPtr) -1);
	
	GetDItem(theDialog, 6, &itemType, &itemHdl, &itemRect);
	for (i=0; i< strlen(expass); i++)		/* echo Æ instead of chars */
		s[i] = 'Æ';
	s[i] = 0;
	setitext(itemHdl, s);
	GetDItem(theDialog, 5, &itemType, &itemHdl, &itemRect);
	setitext(itemHdl, exuser);
	GetDItem(theDialog, 4, &itemType, &itemHdl, &itemRect);
	setitext(itemHdl, exmachine);
	SelIText(theDialog, 4, 0,32767);

	UItemAssign( theDialog, 3, OutlineItem );
	do {
		ModalDialog(pfilt, &itemHit);
	} while (itemHit < 1 || itemHit > 2);

	if (itemHit == 1) {
		GetDItem(theDialog, 5, &itemType, &itemHdl, &itemRect);
		getitext(itemHdl, exuser);
		GetDItem(theDialog, 4, &itemType, &itemHdl, &itemRect);
		getitext(itemHdl, exmachine);
	}
	
	DisposDialog(theDialog);
	
	if (itemHit != 1)
		return(-1);

	return(0);
}

/*****************************************************************/
/*   Modalproc for prompting which is tied to exremote()
*    This proc makes sure that key echoes are suppressed for item #6
*/

pascal Boolean
pfilt(td,tdevent,itemhit)
	DialogPtr td;
	EventRecord *tdevent;
	short int *itemhit;
	{
	short		itemType;
	Handle		itemHdl;
	Rect		itemRect;
	TEHandle	te;
	char s[256];
	int c,i,len,ret;
	
	if (tdevent->what != keyDown && tdevent->what != autoKey)
		return(false);

	switch(tdevent->what) {
		case keyDown:
		case autoKey:
			len = strlen(expass);
/*
*  If it is CR, take it as OK button, as per Inside Mac
*/
			if ((c = (tdevent->message & charCodeMask)) == 13) {
				*itemhit = 1;
				return(true);
			}
			
			if (*itemhit != 6)				/* limit my actions to item #6 */
				return(false);
/*
*  If it is tab, pass it through to modaldialog untouched
*/
			if (c == 9)
				return(false);
#ifdef DONTWORK
/*
*  Check selection start and delete password if not at end
*/
			if ((*te)->selStart < len) {
				*expass = '\0';
				len = 0;
				ret = false;
			}
#endif				
/*
*  If an ASCII char, take it as part of password and echo Æ
*/
			if (c > 31) {
				if (len < 16) {
					expass[len] = c;
					expass[++len] = '\0';
				}
				ret = true;
			}
			

			else {		/* anything else is illegal, terminate string */
				*expass = '\0';
				len = 0;
				ret = false;
			}
							
				
/*
*  Clean up after myself, set the Æ echo to the right length.
*/
			GetDItem(td, *itemhit, &itemType, &itemHdl, &itemRect);
			for (i=0; i< len; i++)
				s[i] = 'Æ';
			s[len] = 0;
			setitext(itemHdl, s);
			SelIText(td, *itemhit, 32767, 32767);
			return(ret);
			
			break;
			

		default:
			break;
	}

	return(false);				/* default return, unknown key or mouse pressed */		

}

/************************************************************************/
/*  ncstrcmp
*   No case string compare.
*   Only returns 0=match, 1=no match, does not compare greater or less
*   There is a tiny bit of overlap with the | 32 trick, but shouldn't be
*   a problem.  It causes some different symbols to match.
*/
ncstrcmp(sa,sb)
	char *sa,*sb;
	{

	while (*sa && *sa < 33)		/* don't compare leading spaces */
		sa++;
	while (*sb && *sb < 33)
		sb++;

	while (*sa && *sb) {
		if ((*sa != *sb) && ((*sa | 32) != (*sb | 32)))
			return(1);
		sa++;sb++;
	}
	if (!*sa && !*sb)		/* if both at end of string */
		return(0);
	else
		return(1);
}

/***************************************************************************/
max(i,j)
	int i,j;
{
	if (i < j)
		return(j);
	else
		return(i);
}

/***************************************************************************/
/*  askvar
*   Ask the user if he spelled the name right.
*   Returns 1 on a valid selection (OK), 0 if the user hits CANCEL.
*   The parameter s must contain 255 characters of space for the textedit string.
*/
askvar(s)
	char *s;
	{
	DialogPtr	theDialog;
	short		itemType;
	Handle		itemHdl;
	Rect		itemRect;
	short		itemHit;

	theDialog = GetNewDialog(136, nil, (WindowPtr) -1);
	
	SetCursor(&qd.arrow);

	UItemAssign( theDialog, 5, OutlineItem );
	GetDItem(theDialog, 4, &itemType, &itemHdl, &itemRect);
	setitext(itemHdl, s);
	SelIText(theDialog, 4, 0,32767);
	
	do {
		ModalDialog(nil, &itemHit);
		if (itemHit == 2) {					/* Cancel, don't care about return */
			DisposDialog(theDialog);
			return(0);
		}
	} while (itemHit < 1 || itemHit > 2);

	GetDItem(theDialog, 4, &itemType, &itemHdl, &itemRect);
	getitext(itemHdl, s);

	DisposDialog(theDialog);
	
	return(1);
}

/********************************************************************/
/*  netalert
*  Inform the user of a network problem.
*/
netalert(s)
	char *s;
{
	char *p;
	
	p = s;
	
	while (*p) {
		if (*p < 32)
			*p = 13;					/* returns are mostly harmless */
		p++;
	}
			
	SetCursor(&qd.arrow);
	
	paramtext(s,"","","");
	
	StopAlert(1008, nil);
	
	return;
}
