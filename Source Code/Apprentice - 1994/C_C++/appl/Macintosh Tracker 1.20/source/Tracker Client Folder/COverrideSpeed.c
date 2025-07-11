/* COverrideSpeed.c */

#include "COverrideSpeed.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CArray.h"


void				COverrideSpeed::IOverrideSpeed(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(OverrideDefaultSpeedID,&LocalStart,&LocalExtent);
		ICheckbox(LocalStart,LocalExtent,GetCString(OverrideDefaultSpeedID),
			0,0,applFont,9,TheWindow,TheWindow);
	}


MyBoolean		COverrideSpeed::DoThang(void)
	{
		MyBoolean			Result;
		SongRec				Song;

		if (Document->Selection != -1)
			{
				Result = inherited::DoThang();
				Document->ListOfSongs->GetElement(Document->Selection,&Song);
				if (Song.SpeedOverrideDefault != State)
					{
						Document->SetOverrideSpeed(State);
					}
			}
		return Result;
	}
