#include "program globals.h"
#include "generic open.h"
#include "jotto load-save.h"
#include "environment.h"
#include "dialogs.h"
#include "error.h"
#include "print meat.h"
#include "file utilities.h"
#include "graphics.h"
#include "window layer.h"

void OpenTheFile(FSSpec *myFSS)
{
	switch (GetFileType(myFSS))
	{
		case SAVE_TYPE:
			if (!IndWindowExistsQQ(kMainWindow))
			{
				LoadSaveDispatch(TRUE, TRUE, myFSS);
				gNeedToOpenWindow=FALSE;
			}
			else
			{
				RemoveHilitePatch();
				PositionDialog('ALRT', smallAlert);
				ParamText("\pPlease close the current game before opening another.","\p","\p","\p");
				StopAlert(smallAlert, 0L);
				InstallHilitePatch();
			}
			break;
	}
}

void PrintTheFile(FSSpec *myFSS)
{
	RemoveHilitePatch();
	PositionDialog('ALRT', smallAlert);
	ParamText("\pJotto ][ can not print games or documents.","\p","\p","\p");
	StopAlert(smallAlert, 0L);
	InstallHilitePatch();
}
