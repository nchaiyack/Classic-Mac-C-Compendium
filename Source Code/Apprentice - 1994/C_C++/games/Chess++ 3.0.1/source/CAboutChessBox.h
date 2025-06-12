/******************************************************************************
 CAboutChessBox.h

		Interface for the Chess++ About Box Class
		
		Copyright © 1993 Steven J. Bushell. All rights reserved.

 ******************************************************************************/
 
#define _H_CAboutChessBox

#include "CDirector.h"					/* Interface for its superclass		*/

#define WINDAboutChessBox 510

	// Forward class declarations
	
class CPane;

class CAboutChessBox : public CDirector {			/* Class Declaration				*/

public:
								/** Instance Variables **/
	CPane			*itsMainPane;
	short			pageWidth;
	short			pageHeight;
	
									/** Construction/Destruction **/

    void      		  	IAboutChessBox(CDirectorOwner *aSupervisor);
	void				DoAboutChessWiggle(void);
	void				DoAboutChessGradualWiggle();
	void				DoAboutChess(void);
};