/* CIcon.h */

#pragma once

#include "CButton.h"

/* forward */
class CWindow;
class CEnclosure;

struct	CPicture	:	CButton
	{
		short		Picture;
		short		PictureSelected;
		EXECUTE(MyBoolean Initialized;)

		void			IPicture(LongPoint Start, LongPoint Extent, short ThePicture,
								short ThePictureSelected, char Key, short Modifiers, CWindow* TheWindow,
								CEnclosure* TheEnclosure);
		void			RedrawNormal(void);
		void			RedrawHilited(void);
	};
