/******************************************************************************
 CChessInfoPane.h

		Interface for the CChessInfoPane Class
		
		Copyright � 1993 Steven J. Bushell. All rights reserved.

 ******************************************************************************/
 
#define _H_CChessInfoPane

#include "CPane.h"						/* Interface for its superclass		*/

class CChessInfoPane : public CPane {			/* Class Declaration				*/

public:
								/** Instance Variables **/
	short			infoString;

									/** Drawing **/
	virtual void	Draw(Rect *area);
};