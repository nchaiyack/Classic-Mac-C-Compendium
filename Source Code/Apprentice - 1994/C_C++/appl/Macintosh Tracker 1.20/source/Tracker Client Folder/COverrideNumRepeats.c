/* COverrideNumRepeats.c */

#include "COverrideNumRepeats.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CArray.h"


void				COverrideNumRepeats::IOverrideNumRepeats(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(OverrideDefaultRepeatsID,&LocalStart,&LocalExtent);
		ICheckbox(LocalStart,LocalExtent,GetCString(OverrideDefaultRepeatsID),
			0,0,applFont,9,TheWindow,TheWindow);
	}


MyBoolean		COverrideNumRepeats::DoThang(void)
	{
		MyBoolean			Result;
		SongRec				Song;

		if (Document->Selection != -1)
			{
				Result = inherited::DoThang();
				Document->ListOfSongs->GetElement(Document->Selection,&Song);
				if (Song.NumRepeatsOverrideDefault != State)
					{
						Document->SetOverrideNumRepeats(State);
					}
			}
		return Result;
	}
