/* CDefaultNumRepeats.c */

#include "CDefaultNumRepeats.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"

#define MINNumRepeats (0)
#define MAXNumRepeats (32767)


void				CDefaultNumRepeats::IDefaultNumRepeats(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(DefaultRepeatsTEID,&LocalStart,&LocalExtent);
		INumberText(LocalStart,LocalExtent,applFont,9,TheWindow,TheWindow);
	}


void				CDefaultNumRepeats::StoreValue(void)
	{
		long			Temp;

		if (Dirty)
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
				if (Temp != Document->NumRepeats)
					{
						Document->SetDefaultNumRepeats(Temp);
					}
				SetValue(Temp);
			}
		inherited::StoreValue();
	}
