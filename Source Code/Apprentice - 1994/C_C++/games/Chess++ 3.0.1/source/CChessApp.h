/*****
 * CChessApp.h
 *
 *	Application class for a typical application.
 *
 *	Copyright � 1993 Steven J. Bushell. All rights reserved.
 *
 *****/

#define	_H_CChessApp		/* Include this file only once */
#include <CApplication.h>

struct CChessApp : CApplication {

	/* No instance variables */

	void	IChessApp(void);
	void	SetUpFileParameters(void);
    
    void    SetUpMenus(void); 
    void    UpdateMenus(void); 

	void	DoCommand(long theCommand);
    
	void	Exit(void);

	void	CreateDocument(void);
	void	OpenDocument(SFReply *macSFReply);
};
