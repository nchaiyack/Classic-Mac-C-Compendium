/* CSpecificVolume.c */

#include "CSpecificVolume.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CArray.h"

#define MINVolume (0)
#define MAXVolume (255)


void				CSpecificVolume::ISpecificVolume(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(SpecificVolumeTEID,&LocalStart,&LocalExtent);
		INumberText(LocalStart,LocalExtent,applFont,9,TheWindow,TheWindow);
	}


void				CSpecificVolume::StoreValue(void)
	{
		long			Temp;
		SongRec		Song;

		if (Dirty && (Document->Selection != -1))
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
				Document->ListOfSongs->GetElement(Document->Selection,&Song);
				if (Song.VolumeOverrideDefault && (Song.Volume != Temp))
					{
						Document->SetVolume(Temp);
					}
				SetValue(Temp);
			}
		inherited::StoreValue();
	}
