/*****
 * CShellApp.h
 *
 *	Application class for a typical application.
 *
 *****/

#define	_H_CShellApp		/* Include this file only once */
#include "CApplication.h"


struct CShellApp : CApplication {

	/* No instance variables */

	void	IShellApp(void);
	void	SetUpFileParameters(void);
    
    void    SetUpMenus(void); 
    void    UpdateMenus(void); 

	void	DoCommand(long theCommand);
    
	void	Exit(void);

	void	ForceClassReferences( void);

	CDirector *DoShellDialogClass(Boolean modal,short ShellDialogID);
	
};
