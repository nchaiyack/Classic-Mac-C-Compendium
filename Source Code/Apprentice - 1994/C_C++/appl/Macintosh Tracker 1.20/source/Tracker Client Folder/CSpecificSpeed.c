/* CSpecificSpeed.c */

#include "CSpecificSpeed.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CArray.h"

#define MINSpeed (20)
#define MAXSpeed (150)


void				CSpecificSpeed::ISpecificSpeed(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(SpecificSpeedTEID,&LocalStart,&LocalExtent);
		INumberText(LocalStart,LocalExtent,applFont,9,TheWindow,TheWindow);
	}


void				CSpecificSpeed::StoreValue(void)
	{
		long			Temp;
		SongRec		Song;

		if (Dirty && (Document->Selection != -1))
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
				Document->ListOfSongs->GetElement(Document->Selection,&Song);
				if (Song.SpeedOverrideDefault && (Song.Speed != Temp))
					{
						Document->SetSpeed(Temp);
					}
				SetValue(Temp);
			}
		inherited::StoreValue();
	}
