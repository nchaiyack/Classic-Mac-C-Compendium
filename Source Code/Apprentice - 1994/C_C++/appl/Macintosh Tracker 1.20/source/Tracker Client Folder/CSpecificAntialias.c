/* CSpecificAntiAliasing.c */

#include "CSpecificAntiAlias.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CArray.h"


void				CSpecificAntiAliasing::ISpecificAntiAliasing(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(SpecificAntiAliasingID,&LocalStart,&LocalExtent);
		ICheckbox(LocalStart,LocalExtent,GetCString(SpecificAntiAliasingID),
			0,0,applFont,9,TheWindow,TheWindow);
	}


MyBoolean		CSpecificAntiAliasing::DoThang(void)
	{
		MyBoolean			Result;
		SongRec				Song;

		if (Document->Selection != -1)
			{
				Result = inherited::DoThang();
				Document->ListOfSongs->GetElement(Document->Selection,&Song);
				if (Song.AntiAliasingOverrideDefault && (Song.AntiAliasing != State))
					{
						Document->SetAntiAliasing(State);
					}
			}
		return Result;
	}
