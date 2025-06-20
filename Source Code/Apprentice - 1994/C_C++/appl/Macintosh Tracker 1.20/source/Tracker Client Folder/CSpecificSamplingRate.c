/* CSpecificSamplingRate.c */

#include "CSpecificSamplingRate.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CArray.h"

#define MINSAMPLINGRATE (8000)
#define MAXSAMPLINGRATE (65535)


void				CSpecificSamplingRate::ISpecificSamplingRate(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(SpecificFreqTEID,&LocalStart,&LocalExtent);
		INumberText(LocalStart,LocalExtent,applFont,9,TheWindow,TheWindow);
	}


void				CSpecificSamplingRate::StoreValue(void)
	{
		long			Temp;
		SongRec		Song;

		if (Dirty && (Document->Selection != -1))
			{
				Temp = GetValue();
				if (Temp < MINSAMPLINGRATE)
					{
						Temp = MINSAMPLINGRATE;
					}
				if (Temp > MAXSAMPLINGRATE)
					{
						Temp = MAXSAMPLINGRATE;
					}
				Document->ListOfSongs->GetElement(Document->Selection,&Song);
				if (Song.SamplingRateOverrideDefault && (Song.SamplingRate != Temp))
					{
						Document->SetSamplingRate(Temp);
					}
				SetValue(Temp);
			}
		inherited::StoreValue();
	}
