/* CDefaultVolume.c */

#include "CDefaultVolume.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"

#define MINVolume (0)
#define MAXVolume (255)


void				CDefaultVolume::IDefaultVolume(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(DefaultVolumeTEID,&LocalStart,&LocalExtent);
		INumberText(LocalStart,LocalExtent,applFont,9,TheWindow,TheWindow);
	}


void				CDefaultVolume::StoreValue(void)
	{
		long			Temp;

		if (Dirty)
			{
				Temp = GetValue();
				if (Temp < MINVolume)
					{
						Temp = MINVolume;
					}
				if (Temp > MAXVolume)
					{
						Temp = MAXVolume;
					}
				if (Temp != Document->Volume)
					{
						Document->SetDefaultVolume(Temp);
					}
				SetValue(Temp);
			}
		inherited::StoreValue();
	}
