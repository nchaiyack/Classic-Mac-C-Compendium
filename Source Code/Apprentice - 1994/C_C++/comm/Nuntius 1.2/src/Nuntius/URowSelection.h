// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// URowSelection.h

#define __UROWSELECTION__


#ifndef __UGRIDVIEW__
#include <UGridView.h>
#endif

class TImprovedRowSelectCommand : public TRowSelectCommand 
{
	public:
		virtual pascal void ComputeNewSelection(GridCell &clickedCell);

		TImprovedRowSelectCommand();
		pascal void IImprovedRowSelectCommand(TGridView* itsView,
						  const VPoint& itsMouse, Boolean theShiftKey, Boolean theCommandKey);
};

//-------------------------------------------------------------------

class TStickySelectCommand : public TImprovedRowSelectCommand 
{
	public:
		pascal void ComputeNewSelection(GridCell &clickedCell);
		pascal void ComputeAnchorCell(GridCell &clickedCell);
		
		TStickySelectCommand();
		void IStickySelectCommand(TGridView *gridView, 
										   const VPoint &itsMouse,
										   Boolean theShiftKey,
										   Boolean theCommandKey);
	protected:
		Boolean fSpecialMethodActivated;
};

