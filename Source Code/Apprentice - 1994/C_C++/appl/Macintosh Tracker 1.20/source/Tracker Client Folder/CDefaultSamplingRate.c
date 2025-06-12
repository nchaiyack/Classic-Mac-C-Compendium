/* CDefaultSamplingRate.c */

#include "CDefaultSamplingRate.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"

#define MINSAMPLINGRATE (8000)
#define MAXSAMPLINGRATE (65535)


void				CDefaultSamplingRate::IDefaultSamplingRate(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(DefaultFrequencyTEID,&LocalStart,&LocalExtent);
		INumberText(LocalStart,LocalExtent,applFont,9,TheWindow,TheWindow);
	}


void				CDefaultSamplingRate::StoreValue(void)
	{
		long			Temp;

		if (Dirty)
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
				if (Temp != Document->SamplingRate)
					{
						Document->SetDefaultSamplingRate(Temp);
					}
				SetValue(Temp);
			}
		inherited::StoreValue();
	}
