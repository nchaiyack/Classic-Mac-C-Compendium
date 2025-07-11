/* CSpecificNumRepeats.c */

#include "CSpecificNumRepeats.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CArray.h"

#define MINNumRepeats (0)
#define MAXNumRepeats (32767)


void				CSpecificNumRepeats::ISpecificNumRepeats(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(SpecificRepeatTEID,&LocalStart,&LocalExtent);
		INumberText(LocalStart,LocalExtent,applFont,9,TheWindow,TheWindow);
	}


void				CSpecificNumRepeats::StoreValue(void)
	{
		long			Temp;
		SongRec		Song;

		if (Dirty && (Document->Selection != -1))
			{
				Temp = GetValue();
				if (Temp < MINNumRepeats)
					{
						Temp = MINNumRepeats;
					}
				if (Temp > MAXNumRepeats)
					{
						Temp = MAXNumRepeats;
					}
				Document->ListOfSongs->GetElement(Document->Selection,&Song);
				if (Song.NumRepeatsOverrideDefault && (Song.NumRepeats != Temp))
					{
						Document->SetNumRepeats(Temp);
					}
				SetValue(Temp);
			}
		inherited::StoreValue();
	}
