/* COverrideSamplingRate.c */

#include "COverrideSamplingRate.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CArray.h"


void				COverrideSamplingRate::IOverrideSamplingRate(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(OverrideDefaultFrequecyID,&LocalStart,&LocalExtent);
		ICheckbox(LocalStart,LocalExtent,GetCString(OverrideDefaultFrequecyID),
			0,0,applFont,9,TheWindow,TheWindow);
	}


MyBoolean		COverrideSamplingRate::DoThang(void)
	{
		MyBoolean			Result;
		SongRec				Song;

		if (Document->Selection != -1)
			{
				Result = inherited::DoThang();
				Document->ListOfSongs->GetElement(Document->Selection,&Song);
				if (Song.SamplingRateOverrideDefault != State)
					{
						Document->SetOverrideSamplingRate(State);
					}
			}
		return Result;
	}
