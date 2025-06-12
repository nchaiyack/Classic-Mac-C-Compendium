/* CDefaultStereoMix.c */

#include "CDefaultStereoMix.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"

#define MINStereoMix (0)
#define MAXStereoMix (100)


void				CDefaultStereoMix::IDefaultStereoMix(CMyDocument* TheDocument,
							CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(DefaultMixTEID,&LocalStart,&LocalExtent);
		INumberText(LocalStart,LocalExtent,applFont,9,TheWindow,TheWindow);
	}


void				CDefaultStereoMix::StoreValue(void)
	{
		long			Temp;

		if (Dirty)
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
				if (Temp != Document->StereoMix)
					{
						Document->SetDefaultStereoMix(Temp);
					}
				SetValue(Temp);
			}
		inherited::StoreValue();
	}
