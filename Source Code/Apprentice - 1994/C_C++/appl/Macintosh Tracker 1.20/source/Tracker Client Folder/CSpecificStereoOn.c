/* CSpecificStereoOn.c */

#include "CSpecificStereoOn.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CArray.h"


void				CSpecificStereoOn::ISpecificStereoOn(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(SpecificStereoID,&LocalStart,&LocalExtent);
		ICheckbox(LocalStart,LocalExtent,GetCString(SpecificStereoID),
			0,0,applFont,9,TheWindow,TheWindow);
	}


MyBoolean		CSpecificStereoOn::DoThang(void)
	{
		MyBoolean			Result;
		SongRec				Song;

		if (Document->Selection != -1)
			{
				Result = inherited::DoThang();
				Document->ListOfSongs->GetElement(Document->Selection,&Song);
				if (Song.StereoOnOverrideDefault && (Song.StereoOn != State))
					{
						Document->SetStereoOn(State);
					}
			}
		return Result;
	}
