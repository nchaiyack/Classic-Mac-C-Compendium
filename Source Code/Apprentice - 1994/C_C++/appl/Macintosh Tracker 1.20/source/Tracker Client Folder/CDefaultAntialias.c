/* CDefaultAntiAliasing.c */

#include "CDefaultAntialias.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"


void				CDefaultAntiAliasing::IDefaultAntiAliasing(CMyDocument* TheDocument, CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(DefaultAntiAliasingID,&LocalStart,&LocalExtent);
		ICheckbox(LocalStart,LocalExtent,GetCString(DefaultAntiAliasingID),
			0,0,applFont,9,TheWindow,TheWindow);
	}


MyBoolean		CDefaultAntiAliasing::DoThang(void)
	{
		MyBoolean			Result;

		Result = inherited::DoThang();
		if (Document->AntiAliasing != State)
			{
				Document->SetDefaultAntiAliasing(State);
			}
		return Result;
	}
