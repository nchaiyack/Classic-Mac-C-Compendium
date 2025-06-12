/******************************************************************************
 CBrowserSizeBox.h

		Interface for the SizeBox Class
		
		Copyright © 1989 Symantec Corporation. All rights reserved.
		
	TCL 1.1 CHANGES
	[
		- added useSICN instance variable. Determines whether a SICN resource
		  or the DrawGrowIcon trap is used to draw the size box.
	]
		
 ******************************************************************************/
 
#define _H_CCBrowserSizeBox

#include <CSizeBox.h>						/* Interface for its superclass		*/

class CBrowserSizeBox : public CSizeBox {		/* Class Declaration				*/

public:
								/** Instance Methods **/
	virtual void	DoClick(Point hitPt,short modifierKeys,long when);
			void	Draw(Rect *area);
};