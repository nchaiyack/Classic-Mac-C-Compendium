// **********************************************************************
//	TFortranView Methods.
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	Method:		IRes
// --------------------------------------------------------------------------------------------------
pascal void TFortranView::IRes (TDocument * itsDocument, TView * itsSuperView, Ptr * itsParams)
	{
	inherited::IRes(itsDocument, itsSuperView, itsParams);
	return;
	}

// --------------------------------------------------------------------------------------------------
//	Method:		PutLine
// 	This routine takes a line of text received from the DTM connection and will insert
// 		it into the text edit record.
// --------------------------------------------------------------------------------------------------
void TFortranView::PutLine (char * message)
	{
	Handle itsText = fText; 									// what is in there already
	long size = GetHandleSize (itsText); 			// how long is it (handle is always full so handle size == # of chars)
	long add = strlen (message); 							// how long will it be after we print
	
	if (add + size > fMaxChars) 							// cannot exceed its size limit
		return; 																// Failure (minErr, 0);

	TEInsert((const void *) message, strlen (message), fHTE);
	this->SynchView(TRUE);
	this->Update();
	this->ForceRedraw();
	}

pascal void TFortranView::Close()
	{
	gFortranView = NULL;
	inherited::Close();
	}
