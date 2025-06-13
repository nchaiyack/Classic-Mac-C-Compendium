#include "save dialog.h"
#include "dialog layer.h"
#include "kant load-save.h"
#include "window layer.h"

short DisplaySaveAlert(WindowRef theWindow, Str255 fileName, Str255 verb)
{
	short			result;
	FSSpec			fs;
	
	if (WindowIsModifiedQQ(theWindow))
	{
		SetCursor(&qd.arrow);
		result=DisplayTheAlert(kCautionAlert, kSaveAlertID, fileName, verb, "\p", "\p",
			(UniversalProcPtr)SaveModalFilter);
		switch (result)
		{
			case 1:
				fs=GetWindowFS(theWindow);
				LoadSaveDispatch(&fs, FALSE, TRUE);
				return (WindowIsModifiedQQ(theWindow)) ? kUserCanceled : kUserSaved;
			case 2:
				return kUserCanceled;
			case 3:
				return kUserDidntSave;
		}
	}
	
	return kUserSaved;
}
