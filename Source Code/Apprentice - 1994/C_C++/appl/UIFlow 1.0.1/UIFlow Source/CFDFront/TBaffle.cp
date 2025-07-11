#pragma segment Boundry
// **********************************************************************
//	TBaffle Class
//		The Baffle objects
// **********************************************************************
//	direction indicates 	Vertical Baffle		(1)
//									Horizontal Baffle	(2)
// --------------------------------------------------------------------------------------------------
//	TBaffle	:	Initialize the Class
// --------------------------------------------------------------------------------------------------
void TBaffle::IBaffle(PointInfo * info, short tDirection)
	{
	if (tDirection == 1)
		{
		fPt1 = info->above;
		fPt2 = info->below;
		}
	else
		{
		fPt1 = info->left;
		fPt2 = info->right;
		}

	fDirection = tDirection;	
	this->BaffleSet();
	return;
	}

// --------------------------------------------------------------------------------------------------
//	TBaffle	:	Draw the baffle
// --------------------------------------------------------------------------------------------------
void TBaffle::Draw()
	{
	if (fPt1->IsBoundryPt() && fPt2->IsBoundryPt())
		PenSize(gPensize_H+2,gPensize_V+2);
	else
		PenSize(gGridsize_H+2, gGridsize_V+2);
		
	HLock((Handle) this);
	RGBForeColor(&BaffleColor);																// set color
	HUnlock((Handle) this);
	PenPixPat(BafflePat);
	PenMode(srcXor);																					// draw xor
	MoveTo(fPt1->fTrans.h,fPt1->fTrans.v);												// move to this point
	LineTo(fPt2->fTrans.h,fPt2->fTrans.v);												// draw to baffle stop
	return;	
	}

// --------------------------------------------------------------------------------------------------
//	TBaffle	:	Release the Points
// --------------------------------------------------------------------------------------------------
void TBaffle::BaffleRelease(void)
	{
	if (fDirection == 1)
		{
		fPt1->SetBaffle (false,3, NULL);
		fPt2->SetBaffle (false,1, NULL);
		}
	else
		{
		fPt1->SetBaffle (false,2, NULL);
		fPt2->SetBaffle (false,4, NULL);
		} 
	}
	
// --------------------------------------------------------------------------------------------------
//	TBaffle	:	Set the Points
// --------------------------------------------------------------------------------------------------
void TBaffle::BaffleSet(void)
	{
	if (fDirection == 1)
		{
		fPt1->SetBaffle (true,3, (TObject *) this);
		fPt2->SetBaffle (true,1, (TObject *) this);
		}
	else
		{
		fPt1->SetBaffle (true,2, (TObject *) this);
		fPt2->SetBaffle (true,4, (TObject *) this);
		}
	}
	
// --------------------------------------------------------------------------------------------------
//	TBaffle	:	Release the Points
// --------------------------------------------------------------------------------------------------
pascal void TBaffle::Free(void)
	{
//	short i;
//	if (fDirection == 1)
//		i = 3;
//	else
//		i = 4;
//	if (fPt1->GetBaffQuad(i) != NULL)
//		this->BaffleRelease();
	inherited::Free();
	}
	
