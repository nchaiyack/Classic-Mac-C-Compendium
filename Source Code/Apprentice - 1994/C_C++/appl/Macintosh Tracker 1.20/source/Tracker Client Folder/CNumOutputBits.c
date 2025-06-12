/* CNumOutputBits.c */

#include "CNumOutputBits.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CArray.h"


void				CNumOutputBits::INumOutputBits(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(NumOutputBitsID,&LocalStart,&LocalExtent);
		ICheckbox(LocalStart,LocalExtent,GetCString(NumOutputBitsID),
			0,0,applFont,9,TheWindow,TheWindow);
	}


MyBoolean		CNumOutputBits::DoThang(void)
	{
		MyBoolean			Result;

		Result = inherited::DoThang();
		if (Document->UseSixteenBitMode != State)
			{
				Document->SetNumOutputBits(State);
			}
		return Result;
	}
