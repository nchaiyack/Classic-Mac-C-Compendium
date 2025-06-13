/******************************************************************************
 CShellDialog.cp

		
	SUPERCLASS = CDLOGDirector
	
	Copyright © 1994 Johns Hopkins University. All rights reserved.
	

 ******************************************************************************/

#include "CShellDialog.h"
#include "CDialog.h"
#include "CMovieController.h"

#define	kShellDialogIDwo			1024

enum		/* window item numbers	*/
{
	kOKBtn = 1
};

extern CApplication	*gApplication;

/******************************************************************************
 IShellDialog
******************************************************************************/

void CShellDialog::IShellDialog( short ShellDialogID)
{
	
	CDLOGDirector::IDLOGDirector( ShellDialogID, gApplication);
	
	((CDialog*)itsWindow)->SetDefaultCmd( cmdOK);
	
	// if there is not a CMovieController in the DITL then create it on the fly
	if (ShellDialogID == kShellDialogIDwo){
		itsMC = new CMovieController;
		itsMC->IMovieController(itsWindow, this, 0, 0, 0, 0,sizELASTIC, sizELASTIC);
		itsMC->OpenMovie();
		if (itsMC->itsMovie)
			SizeWindow(itsWindow->macPort, itsMC->width, itsMC->height, TRUE);
	}
	
}	/* CShellDialog::IShellDialog */

/******************************************************************************
 DoCommand
******************************************************************************/

void CShellDialog::DoCommand( long aCmd)
{
	switch (aCmd)
	{
		case cmdOK:
		case cmdCancel:
			dismissCmd = aCmd;
			Close(FALSE);
			break;
			
		default: 
				inherited::DoCommand( aCmd);
				break;
	}
				
}	/* CShellDialog::DoCommand */

/******************************************************************************
 ProviderChanged
******************************************************************************/

void CShellDialog::ProviderChanged( CCollaborator *aProvider, long reason,
										void *info)
{
	inherited::ProviderChanged( aProvider, reason, info);

}	/* CShellDialog::ProviderChanged */

/******************************************************************************
 Dispose
******************************************************************************/

void CShellDialog::Dispose( void)
{
	ForgetObject(itsMC);
	inherited::Dispose();
}

