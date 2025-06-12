/* CAutoNextSong.c */

#include "CAutoNextSong.h"
#include "CMyDocument.h"
#include "LocationConstants.h"
#include "CWindow.h"


void				CAutoNextSong::IAutoNextSong(CMyDocument* TheDocument, CWindow* TheWindow)
	{
		LongPoint			LocalStart,LocalExtent;

		Document = TheDocument;
		GetRect(AutoNextSongID,&LocalStart,&LocalExtent);
		ICheckbox(LocalStart,LocalExtent,GetCString(AutoNextSongID),
			0,0,applFont,9,TheWindow,TheWindow);
	}


MyBoolean		CAutoNextSong::DoThang(void)
	{
		MyBoolean			Result;

		Result = inherited::DoThang();
		if (Document->AutoNextSong != State)
			{
				Document->SetAutoNextSong(State);
			}
		return Result;
	}
