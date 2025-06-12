/* CSpecificStereoMix.c */

#include "CSpecificStereoMix.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"
#include "CArray.h"

#define MINStereoMix (0)
#define MAXStereoMix (100)


void				CSpecificStereoMix::ISpecificStereoMix(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(SpecificMixTEID,&LocalStart,&LocalExtent);
		INumberText(LocalStart,LocalExtent,applFont,9,TheWindow,TheWindow);
	}


void				CSpecificStereoMix::StoreValue(void)
	{
		long			Temp;
		SongRec		Song;

		if (Dirty && (Document->Selection != -1))
			{
				Temp = GetValue();
				if (Temp < MINStereoMix)
					{
						Temp = MINStereoMix;
					}
				if (Temp > MAXStereoMix)
					{
						Temp = MAXStereoMix;
					}
				Document->ListOfSongs->GetElement(Document->Selection,&Song);
				if (Song.StereoMixOverrideDefault && (Song.StereoMix != Temp))
					{
						Document->SetStereoMix(Temp);
					}
				SetValue(Temp);
			}
		inherited::StoreValue();
	}
