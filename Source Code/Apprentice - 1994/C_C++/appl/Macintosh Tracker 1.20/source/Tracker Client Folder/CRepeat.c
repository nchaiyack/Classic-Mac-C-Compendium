/* CRepeat.c */

#include "CRepeat.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"


void				CRepeat::IRepeat(CMyDocument* TheDocument, CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(RepeatID,&LocalStart,&LocalExtent);
		ICheckbox(LocalStart,LocalExtent,GetCString(RepeatID),
			0,0,applFont,9,TheWindow,TheWindow);
	}


MyBoolean		CRepeat::DoThang(void)
	{
		MyBoolean			Result;

		Result = inherited::DoThang();
		if (Document->Repeat != State)
			{
				Document->SetRepeat(State);
			}
		return Result;
	}
