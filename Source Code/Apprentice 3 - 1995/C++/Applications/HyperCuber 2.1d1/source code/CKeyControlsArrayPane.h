//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyControlsArrayPane.cp
//|
//| This file contains the interface to the CKeyControlsArray class.  The
//| CKeyControlsArrayPane class implements the the scrolling list in the key
//| controls window.
//|________________________________________________________________________________


#include <CArrayPane.h>


class CKeyControlsArrayPane : public CArrayPane
	{

  public:

	void IKeyControlsArrayPane( CView *anEnclosure, CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing);

	void DrawCell(Cell theCell, Rect *cellRect);
	
	};
