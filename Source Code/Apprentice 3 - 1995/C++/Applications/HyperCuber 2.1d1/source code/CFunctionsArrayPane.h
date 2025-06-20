//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CFunctionsArrayPane.h
//|
//| This file contains the interface to the CFunctionsArrayPane class.  The
//| CFunctionsArrayPane class implements the scrolling list of functions in the
//| equation window.
//|________________________________________________________________________________


#include <CArrayPane.h>


class CFunctionsArrayPane : public CArrayPane
	{

  public:

	void IFunctionsArrayPane( CView *anEnclosure, CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing);

	virtual void DrawCell(Cell cell, Rect *rect);
	virtual void GetCellText(Cell cell, short availableWidth, StringPtr text);
	
	};
