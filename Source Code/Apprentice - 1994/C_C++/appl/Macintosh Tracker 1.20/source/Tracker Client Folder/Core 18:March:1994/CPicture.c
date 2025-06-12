/* CIcon.c */

#include "CPicture.h"
#include "CWindow.h"


/* initialize the button */
void			CPicture::IPicture(LongPoint Start, LongPoint Extent, short ThePicture,
						short ThePictureSelected, char Key, short Modifiers, CWindow* TheWindow,
						CEnclosure* TheEnclosure)
	{
		ERROR(Initialized == True,PRERR(ForceAbort,
			"CPicture::IPicture called on already initialized object."));
		EXECUTE(Initialized = True);
		Picture = ThePicture;
		PictureSelected = ThePictureSelected;
		IButton(Start,Extent,Key,Modifiers,TheWindow,TheEnclosure);
	}


void			CPicture::RedrawNormal(void)
	{
		PicHandle		Pic;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CPicture::RedrawNormal called on uninitialized object."));
		SetUpPort();
		ERROR(ResLoad == 0,PRERR(ForceAbort,"Automatic resource loading is disabled."));
		Pic = (PicHandle)GetResource('PICT',Picture);
		ERROR(Pic==NIL,PRERR(ForceAbort,
			"CPicture::RedrawNormal passed an undefined PICT resource ID."));
		ERROR(ResErr != noErr,PRERR(ForceAbort,"Resource Error occurred."));
		Window->LDrawPicture(Pic,ZeroPoint,Extent);
		if (!Enabled)
			{
				Window->SetPen(1,1,patBic,gray);
				Window->LPaintRect(ZeroPoint,Extent);
			}
	}


void			CPicture::RedrawHilited(void)
	{
		PicHandle		Pic;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CPicture::RedrawHilited called on uninitialized object."));
		SetUpPort();
		ERROR(ResLoad == 0,PRERR(ForceAbort,"Automatic resource loading is disabled."));
		Pic = (PicHandle)GetResource('PICT',PictureSelected);
		ERROR(Pic==NIL,PRERR(ForceAbort,"CPicture::RedrawHilited passed an undefined PICT resource ID."));
		ERROR(ResErr != noErr,PRERR(ForceAbort,"Resource Error occurred."));
		Window->LDrawPicture(Pic,ZeroPoint,Extent);
		if (!Enabled)
			{
				Window->SetPen(1,1,patBic,gray);
				Window->LPaintRect(ZeroPoint,Extent);
			}
	}
