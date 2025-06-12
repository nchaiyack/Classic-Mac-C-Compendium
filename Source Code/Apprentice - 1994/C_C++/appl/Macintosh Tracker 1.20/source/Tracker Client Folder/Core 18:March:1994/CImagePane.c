/* CImagePane.c */

#include "CImagePane.h"
#include "CWindow.h"


void		CImagePane::IImagePane(LongPoint Start, LongPoint Extent, CWindow* TheWindow,
					CEnclosure* TheEnclosure, short ThePictID)
	{
		PictID = ThePictID;
		IViewRect(Start,Extent,TheWindow,TheEnclosure);
	}


void		CImagePane::DoUpdate(void)
	{
		PicHandle		Image;

		SetUpPort();
		ERROR(ResLoad == 0,PRERR(ForceAbort,"Automatic resource loading is disabled."));
		Image = (PicHandle)GetResource('PICT',PictID);
		ERROR(Image==NIL,PRERR(ForceAbort,"'PICT' resource not present."));
		ERROR(ResErr != noErr,PRERR(ForceAbort,"Resource Error occurred."));
		HLock((Handle)Image);
		Window->LDrawPicture(Image,ZeroPoint,Extent);
		HUnlock((Handle)Image);
	}
