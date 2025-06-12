/* CAutoStartSongs.c */

#include "CAutoStartSongs.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"


void				CAutoStartSongs::IAutoStartSongs(CMyDocument* TheDocument, CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(AutoStartSongsID,&LocalStart,&LocalExtent);
		ICheckbox(LocalStart,LocalExtent,GetCString(AutoStartSongsID),
			0,0,applFont,9,TheWindow,TheWindow);
	}


MyBoolean		CAutoStartSongs::DoThang(void)
	{
		MyBoolean			Result;

		Result = inherited::DoThang();
		if (Document->AutoStartSongs != State)
			{
				Document->SetAutoStartSongs(State);
			}
		return Result;
	}
