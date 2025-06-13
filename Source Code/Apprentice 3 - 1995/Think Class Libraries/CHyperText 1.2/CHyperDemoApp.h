/******************************************************************************
 CHyperDemoApp.cp

		
	SUPERCLASS = CApplication
	
	Copyright © 1994 Johns Hopkins University. All rights reserved.
	
	Original Author: 	Martin R. Wachter		email:	mrw@welchgate.welch.jhu.edu
	Created:			4/4/94					by:		mrw			TCL Version:	1.1.3
	Modified:			4/5/94					by:		mrw			TCL Version:	1.1.3

 ******************************************************************************/

#define	_H_CHyperDemoApp		/* Include this file only once */
#include "CApplication.h"


struct CHyperDemoApp : CApplication {

	/* No instance variables */

	void	IHyperDemoApp(void);
	void	SetUpFileParameters(void);
    
    void    SetUpMenus(void); 
    void    UpdateMenus(void); 

	void	DoCommand(long theCommand);
    
	void	Exit(void);

	void	ForceClassReferences( void);

	CDirector *DoDemoDialog( short resID, Boolean modal);
	CDirector *DoDemoDialogClass(Boolean modal);
	
};
