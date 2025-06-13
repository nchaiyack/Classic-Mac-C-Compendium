//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CParametersArrayPane.cp
//|
//| This file contains the interface to the CParametersArrayPane class.  The
//| CParametersArrayPane class implements the the scrolling list in the
//| equation window.
//|________________________________________________________________________________


#include <CArrayPane.h>


class CParametersArrayPane : public CArrayPane
	{

  public:

	void IParametersArrayPane( CView *anEnclosure, CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing);

	virtual void DrawCell(Cell cell, Rect *rect);
	virtual void GetCellText(Cell cell, short availableWidth, StringPtr text);
	
	};
