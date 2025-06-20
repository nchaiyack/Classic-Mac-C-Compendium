/******************************************************************************
 CPasswordTextDemoDir.c
	
	To install this Showcase demo, follow these two easy steps:
	
		1)  Add both source files to the CSApplication project.
		
			 *	CPasswordText
				CPasswordTextDemoDir
				
			a whole bunch of dialog support
	
				CButton
				CCheckBox
				CDialog
				CDialogDirector
				CDLOGDialog
				CDLOGDirector
				CIconPane
				CIntegerText
				CRadioControl
				CRadioGroupPane
				SANE (library)
			
			[ Those marked with an * are the files necessary when using
			  the CPasswordText class.  The others are only for this demo. ]
	
		2)	Add all resources in "CPasswordTextDemo.rsrc" to the
			Showcase resource file "CSApplication �.rsrc".

		3) Recompile and run.

	NOTE: This example uses the CDialog classes. Please see its documentation
	on how to use an overloaded static text dialog item. The password dialog 
	uses such an item to present the CPasswordText instance.
	
	SUPERCLASS: CShowcaseDemoDir	
	REQUIRES:   CPasswordText	
	AUTHOR:     Andrew_Gilmartin@Brown.Edu
	MODIFIED:   93-05-21

******************************************************************************/

#include <TCLHeaders>
#include "CPasswordTextDemoDir.h"
#include "CPasswordText.h"



/*============================================================================
 GetUserNameAndPassword

	Ask the user for a name and password.
============================================================================*/

#define GET_ITEM_STRING( _dialog, _item, _string )\
	((CDialogText*)_dialog->itsWindow->FindViewByID(_item))->GetTextString(_string)

#define SET_ITEM_STRING( _dialog, _item, _string )\
	((CDialogText*)_dialog->itsWindow->FindViewByID(_item))->SetTextString(_string)

#define kPasswordDialogID 1024

#define kNameItem 4
#define kPasswordItem 3

static Boolean GetUserNameAndPassword( StringPtr aName, StringPtr aPassword )
{
	CDLOGDirector* theDialog = NULL;
	long theResult = cmdNull;

	TRY
	{	
		theDialog = new CDLOGDirector;
		theDialog->IDLOGDirector( kPasswordDialogID, gApplication );
		
		SET_ITEM_STRING( theDialog, kNameItem, aName );
		SET_ITEM_STRING( theDialog, kPasswordItem, aPassword );
		
		theDialog->BeginDialog();
	
		if ( ( theResult = theDialog->DoModalDialog( cmdOK ) ) == cmdOK )
		{
			GET_ITEM_STRING( theDialog, kNameItem, aName );
			GET_ITEM_STRING( theDialog, kPasswordItem, aPassword );
		}
	
		ForgetObject( theDialog );
	}
	CATCH
	{
		ForgetObject( theDialog );
	}
	ENDTRY

	return theResult == cmdOK;

} /* GetUserNameAndPassword */


/******************************************************************************
 INewDemo

******************************************************************************/

void CPasswordTextDemoDir::INewDemo( CDirectorOwner *aSupervisor )
{
	Str255 theName;
	Str255 thePassword;

	inherited::INewDemo( aSupervisor);

	CopyPString( "\pAndrew Gilmartin", theName );
	CopyPString( "\p", thePassword );

	if ( GetUserNameAndPassword( theName, thePassword ) )
	{
		/* The user pressed OK */
	}

		/* Finished with the demo */

	Close( FALSE );
		
} /* INewDemo */
