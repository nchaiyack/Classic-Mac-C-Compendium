#pragma segment Boundry
// **********************************************************************
//	TObstacle Class
//		The obstacle objects
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	TObstacle	:	Initialize the Class
// --------------------------------------------------------------------------------------------------
void TObstacle::IObstacle(PointInfo * info)
	{
	fUpLeft		=	info->oPt;
	fUpRight	= 	info->above;
	fLwLeft	=	info->left;
	fLwRight	= 	info->below;
	
	this->SetObstacle();
	return;
	}

// --------------------------------------------------------------------------------------------------
//	TObstacle	:	Initialize the Class
// --------------------------------------------------------------------------------------------------
void TObstacle::SetCorners(PointInfo * info)
	{
	fUpLeft		=	info->oPt;
	fUpRight	= 	info->above;
	fLwLeft	=	info->left;
	fLwRight	= 	info->below;
	this->SetObstacle();
	}
	
// --------------------------------------------------------------------------------------------------
//	TObstacle	:	Draw the obstacle
// --------------------------------------------------------------------------------------------------
void TObstacle::Draw()
	{
	RgnHandle rgn;

	PenMode (srcXor);
	PenSize (1,1);
	HLock((Handle) this);
	RGBForeColor(&ObstacleColor);
	HUnlock((Handle) this);
	PenPixPat(ObstaclePat);

	rgn = NewRgn();
	OpenRgn();
	MoveTo (fUpLeft->fTrans.h, fUpLeft->fTrans.v);
	LineTo (fUpRight->fTrans.h, fUpRight->fTrans.v);
	LineTo (fLwRight->fTrans.h, fLwRight->fTrans.v);
	LineTo (fLwLeft->fTrans.h, fLwLeft->fTrans.v);
	LineTo (fUpLeft->fTrans.h, fUpLeft->fTrans.v);
	CloseRgn (rgn);
	PaintRgn (rgn);
	DisposeRgn (rgn);
	}
	
// --------------------------------------------------------------------------------------------------
//	TObstacle	:	Release the Points
// --------------------------------------------------------------------------------------------------
void TObstacle::ReleaseObstacle(void)
	{
	fUpLeft->SetObstacle(false,4,NULL);
	fUpRight->SetObstacle(false,3,NULL);
	fLwLeft->SetObstacle(false,2,NULL);
	fLwRight->SetObstacle(false,1,NULL);
	}
	
// --------------------------------------------------------------------------------------------------
//	TObstacle	:	Set the Points
// --------------------------------------------------------------------------------------------------
void TObstacle::SetObstacle(void)
	{
	fUpLeft->SetObstacle(true,4,(TObject *) this);
	fUpRight->SetObstacle(true,3,(TObject *) this);
	fLwLeft->SetObstacle(true,2,(TObject *) this);
	fLwRight->SetObstacle(true,1,(TObject *) this);
	}
	
// --------------------------------------------------------------------------------------------------
//	TObstacle	:	Release the Points
// --------------------------------------------------------------------------------------------------
pascal void TObstacle::Free(void)
	{
	if (fUpLeft->GetObsQuad(4) != NULL)
		this->ReleaseObstacle();
	inherited::Free();
	}
	
