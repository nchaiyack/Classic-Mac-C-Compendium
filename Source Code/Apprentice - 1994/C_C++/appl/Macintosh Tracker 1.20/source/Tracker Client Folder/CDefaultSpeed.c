/* CDefaultSpeed.c */

#include "CDefaultSpeed.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"

#define MINSpeed (20)
#define MAXSpeed (150)


void				CDefaultSpeed::IDefaultSpeed(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(DefaultSpeedTEID,&LocalStart,&LocalExtent);
		INumberText(LocalStart,LocalExtent,applFont,9,TheWindow,TheWindow);
	}


void				CDefaultSpeed::StoreValue(void)
	{
		long			Temp;

		if (Dirty)
			{
				Temp = GetValue();
				if (Temp < MINSpeed)
					{
						Temp = MINSpeed;
					}
				if (Temp > MAXSpeed)
					{
						Temp = MAXSpeed;
					}
				if (Temp != Document->Speed)
					{
						Document->SetDefaultSpeed(Temp);
					}
				SetValue(Temp);
			}
		inherited::StoreValue();
	}
