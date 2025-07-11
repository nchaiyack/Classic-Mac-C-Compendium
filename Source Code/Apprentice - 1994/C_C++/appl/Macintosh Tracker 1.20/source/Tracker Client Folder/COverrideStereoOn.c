/* COverrideStereoOn.c */

#include "COverrideStereoOn.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CArray.h"


void				COverrideStereoOn::IOverrideStereoOn(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(OverrideDefaultStereoID,&LocalStart,&LocalExtent);
		ICheckbox(LocalStart,LocalExtent,GetCString(OverrideDefaultStereoID),
			0,0,applFont,9,TheWindow,TheWindow);
	}


MyBoolean		COverrideStereoOn::DoThang(void)
	{
		MyBoolean			Result;
		SongRec				Song;

		if (Document->Selection != -1)
			{
				Result = inherited::DoThang();
				Document->ListOfSongs->GetElement(Document->Selection,&Song);
				if (Song.StereoOnOverrideDefault != State)
					{
						Document->SetOverrideStereoOn(State);
					}
			}
		return Result;
	}
