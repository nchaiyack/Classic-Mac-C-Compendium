/* COverrideVolume.c */

#include "COverrideVolume.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CArray.h"


void				COverrideVolume::IOverrideVolume(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(OverrideDefaultVolumeID,&LocalStart,&LocalExtent);
		ICheckbox(LocalStart,LocalExtent,GetCString(OverrideDefaultVolumeID),
			0,0,applFont,9,TheWindow,TheWindow);
	}


MyBoolean		COverrideVolume::DoThang(void)
	{
		MyBoolean			Result;
		SongRec				Song;

		if (Document->Selection != -1)
			{
				Result = inherited::DoThang();
				Document->ListOfSongs->GetElement(Document->Selection,&Song);
				if (Song.VolumeOverrideDefault != State)
					{
						Document->SetOverrideVolume(State);
					}
			}
		return Result;
	}
