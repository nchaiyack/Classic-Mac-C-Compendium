#include <GestaltEqu.h>
#include "stdio.h"

ProcPtr oldGestaltFunction;

/* ResetGestalt() will reset the Gestalt selectors to the way they were before 		*/
/* starting.																		*/
void ResetGestalt()
{
	ProcPtr	junk;
	OSErr	myReplaceErr=0;
	Handle	gstFuncHandle;
	int		gstDummyFuncRsrcID = 129;
	int		SAVRTrouble = 130;
	
	if(oldGestaltFunction != 0)
		myReplaceErr = ReplaceGestalt('SAVR', oldGestaltFunction, &junk);
	else
	{
		gstFuncHandle = GetResource('GDEF', gstDummyFuncRsrcID);
		if(gstFuncHandle == 0)
			Alert(SAVRTrouble,0);
		else
		{
			DetachResource(gstFuncHandle);
			myReplaceErr = ReplaceGestalt('SAVR', *gstFuncHandle, &oldGestaltFunction);
		}
	}
	if(myReplaceErr != noErr)
		Alert(130,0);
}


/* DisableSuperclock will set the Gestalt selector 'savr' 	*/
/* to indicate that a screen saver, such as afterdark, 		*/
/* is enabled already, thus superclock will					*/
/* not attempt to draw on the playing surface.				*/
void DisableSuperClock()
{
	OSErr	myNewErr, myReplaceErr;
	int		gstFuncRsrcID = 128;
	Handle	gstFuncHandle;
	int		SAVRTrouble = 130;
	
	gstFuncHandle = GetResource('GDEF', gstFuncRsrcID);
	if(gstFuncHandle == 0)
		Alert(SAVRTrouble,0);
	else
	{
		DetachResource(gstFuncHandle);
		myNewErr = NewGestalt('SAVR',*gstFuncHandle);
		if(myNewErr != noErr)
		{
			myReplaceErr = ReplaceGestalt('SAVR', *gstFuncHandle, &oldGestaltFunction);
			if(myReplaceErr != noErr)
				Alert(SAVRTrouble,0);
		}
	}
	/* DisposHandle(gstFuncHandle); */
}

main()
{
	DisableSuperClock();
	printf("Superclock should now be gone \n");
	Alert(130,0);
	ResetGestalt();
	printf("Superclock should now be back \n");
	Alert(130,0);
	DisableSuperClock();
	printf("Superclock should now be gone \n");
	Alert(130,0);
	ResetGestalt();
	printf("Superclock should now be back \n");
	Alert(130,0);
}