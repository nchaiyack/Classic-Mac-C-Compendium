#pragma segment trash
// **********************************************************************
//	TTrash - Methods
// **********************************************************************
void TCTrash::ICTrash(Point cornor, short itsRsrsId,short itsVSize,short itsHSize,
	short itsMaxSize,short side)
	{
	fRsrsId = itsRsrsId;

	theIcon = GetCIcon(itsRsrsId);												// get icon from resource
	if (theIcon == NULL)																// not gotten
		{ return;}																			// LAM - error handling
	
	HLock((Handle) theIcon);														// lock the handle
	HLock((Handle) this);
	this->CalcLocation(&cornor,itsVSize,itsHSize,itsMaxSize,side);	// Center the icon over the point if possible
	HUnlock((Handle) this);
	
	fCTrash.top 		= cornor.v;													// create
	fCTrash.left		= cornor.h;
	fCTrash.bottom	= cornor.v + itsVSize;
	fCTrash.right	= cornor.h + itsHSize;
	return;
	}

// --------------------------------------------------------------------------------------------------
//	Free the icon handle
// --------------------------------------------------------------------------------------------------
pascal void TCTrash::Free(void)
	{
	DisposCIcon(theIcon);
	HUnlock((Handle) theIcon);
	inherited::Free();
	}
	
// --------------------------------------------------------------------------------------------------
//	Is mouse inside the trash can icon?
// --------------------------------------------------------------------------------------------------
Boolean TCTrash::InTheCan(Point theMouse)
	{
	Boolean t;
	HLock((Handle) this);
	t = PtInRect(theMouse,&fCTrash);
	HUnlock((Handle) this);
	
	return t;
	}
	
// --------------------------------------------------------------------------------------------------
//	Draw the Trash can icon
// --------------------------------------------------------------------------------------------------
pascal void TCTrash::DrawCan(void)
	{
	PenMode(srcXor);
	HLock((Handle) this);
	PlotCIcon(&fCTrash,theIcon);
	HUnlock((Handle) this);
	}

// --------------------------------------------------------------------------------------------------
//	Draw the Trash can icon
// --------------------------------------------------------------------------------------------------
Rect TCTrash::GetRect(void)
	{
	return fCTrash;
	}

// --------------------------------------------------------------------------------------------------
//	Calculate the location of the upper left cornor of the trash can
// --------------------------------------------------------------------------------------------------
void TCTrash::CalcLocation(Point * cornor,short itsVSize,short itsHSize,short itsMaxSize,short side)
	{
	short cVert, cHorz, test2;

	if (side == 1 || side == 3)														// point on right / left
		{
		cVert	= (itsVSize/2) + 1;												// find half way
		cornor->v -= cVert;															// center vertically
		test2	= itsMaxSize - (cornor->v + cVert);						// calc difference from bottom
		if (cornor->v < 0)																// beyond top ?
			cornor->v = 0;
		else if (test2 < 0)																// beyond bottom?
			cornor->v += test2;
		
		if (side == 3)																	// place point on the right
			{
			cornor->h -= (itsHSize - gPensize_H);							// calc far left cornor
			if (cornor->h < 0)
				cornor->h = 0;															// don't allow off window
			}
		else																					// place point on the left
			{
			cornor->h -= gPensize_H;
			test2 = itsMaxSize - (cornor->h + itsHSize);				// calc for right cornor
			if (test2 < 0)																// beyond right side
				cornor->h += test2;
			}
		}
	else																						// point on bottom / top
		{
		cHorz	= (itsHSize/2) + 1;												// find half way
		cornor->h -= cHorz;															// center horizontally
		test2	= itsMaxSize - (cornor->h + cHorz);						// calc difference from left
		if (cornor->h < 0)																// beyond top?
			cornor->h = 0;
		else if (test2 < 0)																// beyond bottom?
			cornor->h += test2;

		if (side == 0)																	// place point on the bottom
			{
			cornor->v -= (itsVSize - gPensize_V);							// calc far left cornor
			if (cornor->v < 0)
				cornor->v = 0;															// don't allow off window
			}
		else																					// place point on the left
			{
			cornor->v -= gPensize_V;
			test2 = itsMaxSize - (cornor->v + itsVSize);				// calc for right cornor
			if (test2 < 0)																// beyond right side
				cornor->v += test2;
			}
		}
	return;
	}