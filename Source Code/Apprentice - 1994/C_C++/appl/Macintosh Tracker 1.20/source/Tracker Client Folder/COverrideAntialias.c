/* COverrideAntiAliasing.c */

#include "COverrideAntialias.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CArray.h"


void				COverrideAntiAliasing::IOverrideAntiAliasing(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(OverrideDefaultAntiAliasingID,&LocalStart,&LocalExtent);
		ICheckbox(LocalStart,LocalExtent,GetCString(OverrideDefaultAntiAliasingID),
			0,0,applFont,9,TheWindow,TheWindow);
	}


MyBoolean		COverrideAntiAliasing::DoThang(void)
	{
		MyBoolean			Result;
		SongRec				Song;

		if (Document->Selection != -1)
			{
				Result = inherited::DoThang();
				Document->ListOfSongs->GetElement(Document->Selection,&Song);
				if (Song.AntiAliasingOverrideDefault != State)
					{
						Document->SetOverrideAntiAliasing(State);
					}
			}
		return Result;
	}
