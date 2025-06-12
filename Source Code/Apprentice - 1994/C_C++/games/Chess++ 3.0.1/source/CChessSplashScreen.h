/******************************************************************************
 CChessSplashScreen.h

		Interface for the Chess++ Splash Screen Class
		
		Copyright © 1993 Steven J. Bushell.

 ******************************************************************************/
 
#define _H_CChessSplashScreen

#include "CDirector.h"					/* Interface for its superclass		*/

#define WINDChessSplashScreen 520

	// Forward class declarations
	
class CPane;

class CChessSplashScreen : public CDirector {			/* Class Declaration				*/

public:
								/** Instance Variables **/
	CPane			*itsMainPane;
	short			pageWidth;
	short			pageHeight;
	
									/** Construction/Destruction **/

    void      		  	IChessSplashScreen(CDirectorOwner *aSupervisor);
	void				DoChessSplashScreen(void);
};