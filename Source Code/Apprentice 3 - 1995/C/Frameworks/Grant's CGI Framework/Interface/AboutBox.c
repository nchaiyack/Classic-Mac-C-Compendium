/*****
 *
 *	AboutBox.c
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright �1995 by Grant Neufeld
 *	grant@acm.com
 *	http://arpp1.carleton.ca/grant/
 *
 *****/

#include "constants.h"
#include "globals.h"

#include "DebugUtil.h"
#include "IconUtil.h"
#include "WindowInt.h"

#include "AboutBox.h"


/***  LOCAL VARIABLES ***/

static	DialogPtr	vAboutBoxDialog;


/***  LOCAL CONSTANTS ***/

#define kriAboutBoxAppIcon			2


/***  LOCAL PROTOTYPES ***/

	pascal void	aboutBoxAppIcon	( DialogPtr, short );


/***  FUNCTIONS  ***/

#pragma segment Startup
/* initialize the variables used for the about box */
void
AboutBoxInit ( void )
{
	vAboutBoxDialog = nil;
} /* AboutBoxInit */
#pragma segment Utilities


/* open the about box dialog window */ 
void
AboutBoxOpen ( void )
{
	windowInfoHdl	dlogWindInfo;
	short			itemType;
	Handle			item;
	Rect			box;
	GrafPtr			savePort;
	
	if ( vAboutBoxDialog == nil )
	{
		vAboutBoxDialog = GetNewDialog ( kAboutBoxDLOG, nil, (WindowPtr)-1L );
		
		if ( vAboutBoxDialog != nil )
		{
			dlogWindInfo = WindowNewInfoHdl ( Window_about, false, nil );
			
			/* store a reference to the status dialog's ID# */
			SetWRefCon ( vAboutBoxDialog, (long)dlogWindInfo );
			
			/* install the drawing procedure for the application icon user item */
			GetDItem ( vAboutBoxDialog, kriAboutBoxAppIcon, &itemType, &item, &box );
			SetDItem ( vAboutBoxDialog, kriAboutBoxAppIcon, itemType, (Handle)aboutBoxAppIcon, &box );
		}
	
		if ( vAboutBoxDialog != nil )
		{
			/* set the ^0 parameter to be the version string */
			ParamText ( gVersionStr, nil, nil, nil );
			
			/* change the dialog port font to Geneva */
			GetPort		( &savePort );
			SetPort		( (GrafPtr)vAboutBoxDialog );
			TextFont	( geneva );
			SetPort		( savePort );
		}
	}
	
	/* display the dialog window and bring it to front */
	if ( vAboutBoxDialog != nil )
	{
		ShowWindow		( vAboutBoxDialog );
		SelectWindow	( vAboutBoxDialog );
	}
} /* AboutBoxOpen */


/* update the contents of the about box dialog window */
void
AboutBoxUpdate ( void )
{
	my_assert ( vAboutBoxDialog != nil, "\pAboutBoxUpdate: vAboutBoxDialog is nil" );
	
	/* set the dialog strings */
	ParamText ( gVersionStr, nil, nil, nil );
	
	/* draw the dialog */
	DrawDialog ( vAboutBoxDialog );
} /* AboutBoxUpdate */


/* close the about box window */
void
AboutBoxClose ( void )
{
	my_assert ( vAboutBoxDialog != nil, "\pAboutBoxClose: vAboutBoxDialog is nil" );
	
	DisposeDialog ( vAboutBoxDialog );
	
	vAboutBoxDialog = nil;
} /* AboutBoxClose */


/* Draw the application icon using the ICN# or a color icon resource */
pascal void
aboutBoxAppIcon ( DialogPtr theDialog, short theItem )
{
	short		itemType;
	Rect		itemRect;
	Handle		itemHandle;
	PenState	curPen;
	WindowPtr	oldPort;
	Handle		iconSuiteHdl;
	
	HLock ( (Handle)(&theDialog) );
	
	GetDItem ( theDialog, theItem, &itemType, &itemHandle, &itemRect );
	
	GetPort ( &oldPort );
	SetPort ( (GrafPtr)theDialog );
	
	GetPenState	( &curPen );
	PenNormal	();
	
	IconDrawFromFamily ( kApplicationIconSuite, &itemRect, &iconSuiteHdl );

	SetPenState	( &curPen );
	SetPort		( oldPort );
	
	HUnlock ( (Handle)(&theDialog) );
} /* aboutBoxAppIcon */


/*****  EOF  *****/
