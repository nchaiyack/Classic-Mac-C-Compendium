/* CRandomize.c */

#include "CRandomize.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"


void				CRandomize::IRandomize(CMyDocument* TheDocument, CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(RandomizeID,&LocalStart,&LocalExtent);
		ICheckbox(LocalStart,LocalExtent,GetCString(RandomizeID),
			0,0,applFont,9,TheWindow,TheWindow);
	}


MyBoolean		CRandomize::DoThang(void)
	{
		MyBoolean			Result;

		Result = inherited::DoThang();
		if (Document->Randomize != State)
			{
				Document->SetRandomize(State);
			}
		return Result;
	}
