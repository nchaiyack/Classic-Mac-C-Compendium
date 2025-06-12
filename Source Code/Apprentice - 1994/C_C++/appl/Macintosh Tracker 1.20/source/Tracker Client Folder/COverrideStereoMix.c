/* COverrideStereoMix.c */

#include "COverrideStereoMix.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CArray.h"


void				COverrideStereoMix::IOverrideStereoMix(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(OverrideDefaultMixID,&LocalStart,&LocalExtent);
		ICheckbox(LocalStart,LocalExtent,GetCString(OverrideDefaultMixID),
			0,0,applFont,9,TheWindow,TheWindow);
	}


MyBoolean		COverrideStereoMix::DoThang(void)
	{
		MyBoolean			Result;
		SongRec				Song;

		if (Document->Selection != -1)
			{
				Result = inherited::DoThang();
				Document->ListOfSongs->GetElement(Document->Selection,&Song);
				if (Song.StereoMixOverrideDefault != State)
					{
						Document->SetOverrideStereoMix(State);
					}
			}
		return Result;
	}
