/******************************************************************************
 CBrowserScrollPane.h

		
 ******************************************************************************/
 
#define _H_CBrowserScrollPane

#include <CScrollPane.h>						/* Interface for its superclass		*/

	// Forward declarations
	
class CPanorama;
class CScrollBar;
class CBrowserSizeBox;

class CBrowserScrollPane : public CScrollPane {			/* CLASS DECLARATION				*/

public:
								/** Instance Variables **/
	CBrowserSizeBox	*itsBrowserSizeBox;			/* Grow box							*/
								/** Instance Methods **/
									/** Construct/Desctruction **/
	void		IBrowserScrollPane(
		CView			*anEnclosure,
		CBureaucrat		*aSupervisor,
		short			aWidth,
		short			aHeight,
		short			aHEncl,
		short			aVEncl,
		SizingOption	aHSizing,
		SizingOption	aVSizing,
		Boolean			hasHoriz,
		Boolean			hasVert,
		Boolean			hasSizeBox,
		Boolean			hasSICN);
		
	void	Calibrate();
	
};
