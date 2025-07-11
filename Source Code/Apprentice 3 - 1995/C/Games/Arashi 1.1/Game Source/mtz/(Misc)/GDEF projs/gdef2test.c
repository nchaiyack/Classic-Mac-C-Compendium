#include <GestaltEQu.h>

ProcPtr oldSAVRProc;
Handle	savrHandle;
Handle	savrDumHandle;
ProcPtr oldASHIProc;
Handle	ASHIHandle;
Handle	ASHIDumHandle;

void ReplaceASHI(){
	ProcPtr	tempProc;
	OSErr	myErr;
	long	response;
	
	/* if a selector was in place when ARASHI started, replace it					*/
	if(ASHIHandle && oldASHIProc){
		if(myErr = ReplaceGestalt('ASHI',(ProcPtr)oldASHIProc, &tempProc))
				Alert(133,0);
	}
	/* else install a dummy selector, or better remove it altogether if possible	*/
	/* not possible yet, so replace it with the dummy one #131						*/
	else{
		if(ASHIDumHandle = GetResource('GDEF',131)){
			DetachResource(ASHIDumHandle);
			if(ReplaceGestalt('SAVR', (ProcPtr)*ASHIDumHandle, &oldSAVRProc)){
				DisposHandle(ASHIDumHandle);
				ASHIDumHandle = 0L;
				Alert(133,0);
			}
		}
	}
	DisposHandle(ASHIHandle);
}

/* ResetGestalt() will reset the Gestalt selectors to the way they were before 		*/
/* starting.																		*/
void ResetGestalt(){
	ProcPtr	tempProc;
	OSErr	myErr;
	long	response;
	
	if(Gestalt('SAVR',&response))
		SysBeep(1);
	if(response == 0x12)
		SysBeep(1);
	/* if a selector was in place when ARASHI started, replace it					*/
	if(savrHandle && oldSAVRProc){
		if(myErr = ReplaceGestalt('SAVR',(ProcPtr)oldSAVRProc, &tempProc))
				Alert(130,0);
	}
	/* else install a dummy selector, or better remove it altogether if possible	*/
	/* not possible yet, so replace it with the dummy one #129						*/
	else{
		if(savrDumHandle = GetResource('GDEF',129)){
			DetachResource(savrDumHandle);
			if(ReplaceGestalt('SAVR', (ProcPtr)*savrDumHandle, &oldSAVRProc)){
				DisposHandle(savrDumHandle);
				savrDumHandle = 0L;
				Alert(133,0);
			}
		}
	}
	if(Gestalt('SAVR',&response))
		SysBeep(1);
	if(response == 0)
		SysBeep(1);
	DisposHandle(savrHandle);
	if(Gestalt('SAVR',&response))
		SysBeep(1);
	if(response == 0)
		SysBeep(1);
}

int installASHI()
{
	
	int	error = 0;
	long	response;
	OSErr	myErr;
	OSErr 	myNewErr = 0;
	OSErr 	myReplaceErr = 0;
	
	myErr = Gestalt('ASHI',&response);
	if( (myErr == 0) && (response == 1)){  /* only one copy at a time please.. */
		Alert(131,0);
		return 1;
	}
	else{
		oldASHIProc = 0L;						/* mark none installed so far */
		if(ASHIHandle = GetResource('GDEF',130)){
			DetachResource(ASHIHandle);
			if(myNewErr = NewGestalt('ASHI',*ASHIHandle))
				if(myReplaceErr = ReplaceGestalt('ASHI', *ASHIHandle, &oldASHIProc)){
					DisposHandle(ASHIHandle);
					ASHIHandle = 0L;
					Alert(132,0);
					error = 1;
				}
		}
	}
	return error;
}


/* DisableSuperclock will set the Gestalt selector 'savr' 	*/
/* to indicate that a screen saver, such as afterdark, 		*/
/* is enabled already, thus superclock will					*/
/* not attempt to draw on the playing surface.				*/
void DisableSuperClock(){
	
	long	response = 0;
	OSErr 	myNewErr = 0;
	OSErr 	myReplaceErr = 0;
	
	oldSAVRProc = 0L;
	if(savrHandle = GetResource('GDEF',128)){
		DetachResource(savrHandle);
		if(myNewErr = NewGestalt('SAVR',(ProcPtr)(*savrHandle) ))
			if(myReplaceErr = ReplaceGestalt('SAVR',(ProcPtr)(*savrHandle), &oldSAVRProc)){
				DisposHandle(savrHandle);
				savrHandle = 0L;
				Alert(129,0);
			}
	}
	if(Gestalt('SAVR',&response))
		SysBeep(1);
	if(response == 0x12)
		SysBeep(1);
}

void main()
{
	installASHI();
	DisableSuperClock();
	ResetGestalt();
	ReplaceASHI();
}