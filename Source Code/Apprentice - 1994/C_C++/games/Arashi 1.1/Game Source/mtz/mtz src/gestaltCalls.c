#include <GestaltEQU.h>
#include <stdio.h>

#define BOTH
/* #define DEBUG_GESTALT */

ProcPtr oldSAVRProc;
Handle	savrHandle;
Handle	savrDumHandle;
ProcPtr oldASHIProc;
Handle	ASHIHandle;
Handle	ASHIDumHandle;

void ReplaceASHI(){
	ProcPtr	tempProc;
	OSErr	myErr = 0;
	long	response;
	OSErr myReplaceErr = 0;
	
	/* if a selector was in place when ARASHI started, reinstall it					*/
	if(ASHIHandle && oldASHIProc){
		if(myErr = ReplaceGestalt('ASHI',(ProcPtr)oldASHIProc, &tempProc)){
			ParamText("\pAn error has occurred in removing the ASHI Gestalt selector.  You may have to reboot before restarting ARASHI.",nil,nil,nil);
			Alert(131,0);
		}
#ifdef DEBUG_GESTALT
		else{
				ParamText("\pThe old ASHI Gestalt selector has been reinstalled. ",nil,nil,nil);
				Alert(131,0);
			} 
#endif
	}
	/* else install a dummy selector, or better remove it altogether if possible	*/
	/* not possible yet, so replace it with the dummy one #131						*/
	else{
		if(ASHIDumHandle = GetResource('GDEF',131)){
			DetachResource(ASHIDumHandle);
			if(myReplaceErr = ReplaceGestalt('ASHI', (ProcPtr)*ASHIDumHandle, &oldASHIProc)){
				DisposHandle(ASHIDumHandle);
				ASHIDumHandle = 0L;
				ParamText("\pAn error has occurred in removing the ASHI Gestalt selector.  You may have to reboot before restarting ARASHI. ",nil,nil,nil);
				Alert(131,0);
			}
#ifdef DEBUG_GESTALT			
			else{
				ParamText("\pA dummy 0 ASHI Gestalt selector has been  installed. ",nil,nil,nil);
				Alert(131,0);
			} 
#endif
		}
	}
	Gestalt('ASHI',&response);
#ifdef DEBUG_GESTALT
	if(response == 1)
		ParamText("\pASHI returns a 1 when exiting.",nil,nil,nil);
	else
		ParamText("\pASHI returns a 0 when exiting.",nil,nil,nil);
	Alert(131,0);
#endif
	DisposHandle(ASHIHandle);
}

#ifdef BOTH
/* ResetGestalt() will reset the Gestalt selectors to the way they were before 		*/
/* starting.																		*/
void ResetGestalt(){
	ProcPtr	tempProc;
	OSErr	myErr;
	long	response;
#ifdef BEEP	
	if(Gestalt('SAVR',&response))
		SysBeep(1);
	if(response == 0x12)
		SysBeep(1);
#endif
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
				ParamText("\pAn error has occurred in removing the ASHI Gestalt selector.  You may have to reboot before restarting ARASHI. ",nil,nil,nil);
				Alert(131,0);
			}
		}
	}
	DisposHandle(savrHandle);
#ifdef BEEP	
	if(Gestalt('SAVR',&response))
		SysBeep(1);
	if(response == 0)
		SysBeep(1);
#endif
}
#endif

int installASHI()
{
	
	int	error = 0;
	long	response;
	OSErr	myErr;
	OSErr 	myNewErr = 0;
	OSErr 	myReplaceErr = 0;
	
	myErr = Gestalt('ASHI',&response);
	if( (myErr == 0) && (response == 1)){  /* only one copy at a time please.. */
		ParamText("\pOnly one copy of ARASHI may be active at a time.",nil,nil,nil);
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
					ParamText("\pAn error has occurred trying to install ARASHI.",nil,nil,nil);
					Alert(131,0);
					error = 1;
				}
		}
	}
	return error;
}

#ifdef BOTH
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
				ParamText("\pAn error has occurred trying to re-activate Superclock",nil,nil,nil);
				Alert(131,0);
			}
	}
#ifdef BEEP	
	if(Gestalt('SAVR',&response))
		SysBeep(1);
	if(response == 0x12)
		SysBeep(1);
#endif
}
#endif

