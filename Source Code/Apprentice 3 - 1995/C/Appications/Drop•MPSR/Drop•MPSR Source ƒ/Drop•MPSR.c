/*
	Drop�MPSR.c
	
	This is the code for a small application which handles the modification of the MPSR resource for
	editor systems.
	
	Many times I will download code which I want to use with (insert your editor's name here).  However,
	the code is personalized for the person that used it (i.e. I like geneva-9 for programming, others prefer
	something different).  Well, all of this information is stored in the MPSR resource in each of the text
	files.
	
	So I wanted an appl that I could drop the downloaded files on to set them up the way I want them
	right away; therefore I created this application.
	
	But I took the code one step further; not only does the MPSR resource get fixed, but this appl will set
	up a bunch of things for you (i.e. creator).  Another thing it does is stack all of the windows for you.
	If you drop a bunch of files on the app at one time, the windows will be staggered automatically.
	
*/

#include <LowMem.h>

#include "Drop�MPSR.h"
#include "MPSR Resource.h"

#include "DSUtils.h"

#include "SpinLib.h"

Rect gMainScrn;

Rect gOutRect,gInRect; // for storing the sizes of the zoom in and out rects.
short gSelf;
unsigned long gModTime;
Handle gMPSR;
unsigned short gLeft,gRight,gTop,gBottom,gGoDown,gGoLeft;
OSType gCreator;

/*
	MPSR_Init
	
	Initializes our stuff.
*/
Boolean MPSR_Init(){
	Boolean ret;
	OSErr err;
	GDHandle mainDev;
	Handle h;
	PrefHand pref;
	
	ret=true;
	
	gSelf=CurResFile();
	
	SpinInit();
	
	// get our prefs...
	h=Get1Resource('�prf',editResID);
	if (h!=(Handle)0){
		DetachResource(h);
		HLock(h);
		
		pref=(PrefHand)h;
		gLeft=(*pref)->left;
		gRight=(*pref)->right;
		gTop=(*pref)->top;
		gBottom=(*pref)->bottom;
		
		gGoDown=(*pref)->shiftdown;
		gGoLeft=(*pref)->shiftleft;
		
		gCreator=(*pref)->creator;
		
		HUnlock(h);
		DisposeHandle(h);
	} else {
		gCreator='KAHL';
		gLeft=gRight=gTop=gBottom=2;
		
	}
	
	// get the main screen's rectangle
	mainDev=GetMainDevice();
	HLock((Handle)mainDev);
	CopyRect(&((**((*mainDev)->gdPMap)).bounds),&gMainScrn);
	
	if (gMainScrn.top<0)
		gMainScrn.top=0;
		
	// cut out the menu bar
	gMainScrn.top += LMGetMBarHeight();
	gMainScrn.top += 16;
	
	// add the prefered gaps...
	gMainScrn.left+=gLeft;
	gMainScrn.right-=gRight;
	gMainScrn.top+=gTop;
	gMainScrn.bottom-=gBottom;
	
	return ret;
}

/*
	MPSR_Cleanup
	
	Cleans up our stuff.
*/
void MPSR_Cleanup(){
	
	SpinCleanup();
}

/*
	CopyRect
	
	Copies one rect to another.
*/
void CopyRect(Rect* from,Rect* to){
	to->top=from->top;
	to->bottom=from->bottom;
	to->left=from->left;
	to->right=from->right;
}

/*
	MPSR_Preflight
	
	Preflight handling of the documents.  We reset the rectangles for each group of files.
*/
void MPSR_Preflight(short count,Handle* dataHdl){
	OSErr err;
	short current;
	
	SpinStart(1);
	
	// get the MPSR resource from the application
	current=CurResFile();
	UseResFile(gSelf);
	gMPSR=Get1Resource(editResType,editResID);
	err=ResError();
	UseResFile(current);
	
	if (err!=noErr){
		gMPSR=(Handle)0;
	} else {
		// got the handle, ok to continue...
		DetachResource(gMPSR);
		MoveHHi(gMPSR);
		HLock(gMPSR);
		
		// set up the zooming rectangles.
		CopyRect(&gMainScrn,&gOutRect);
		gOutRect.top +=4;
		gOutRect.right -=51;
		
		// now set the zoom-in rect
		CopyRect(&gOutRect,&gInRect);
		
		// modify the zoom-in rect some more...
		gInRect.right=gInRect.left +80;
		gInRect.bottom=gInRect.top+60;
		
		ReadDateTime(&gModTime);
		
		// now we're all set...
	}
}

/*
	MPSR_Postflight
	
	Postflight handling of the documents.
*/
void MPSR_Postflight(short count,Handle dataHdl){
	
	// ditch the resource now...
	if (gMPSR!=(Handle)0){
		HUnlock(gMPSR);
		DisposeHandle(gMPSR);
	}
	
	SpinStop();
}

/*
	MPSR_Process
	
	Process one document.
*/
OSErr MPSR_Process(FSSpecPtr theSpec){
	OSErr err=noErr;
	FInfo info;
	EditRSRCHdl mpsr=(EditRSRCHdl)gMPSR;
	
	err=FSpGetFInfo(theSpec,&info);
	
	if (err==noErr){
		if (info.fdType!='TEXT'){
			return noErr; // don't do anything to this file...
		}
	} else
		return err;
	
	// now update file for the changes...
	
	if (info.fdCreator!=gCreator)
		SetCreator(theSpec,gCreator);
	
	// make the changes to the local MPSR resource handle...
	CopyRect(&gInRect,&((*mpsr)->windrsrc.openrect));
	CopyRect(&gOutRect,&((*mpsr)->windrsrc.closerect));
	
	// fix up the in and out rects for the next file...
	gOutRect.top+=gGoDown;
	
	if (gOutRect.top>gOutRect.bottom-60){
		gOutRect.left+=gGoLeft;
		
		gOutRect.right+=gGoLeft;
		
		if (gOutRect.right>gMainScrn.right)
			gOutRect.right=gMainScrn.right;
			
		gOutRect.top=gMainScrn.top+4;
	}
	
	gInRect.left+=85;
	gInRect.right+=85;
	if (gInRect.right>gMainScrn.right)
		gInRect.right=gMainScrn.right;
	
	if (gInRect.left>gInRect.right-60){
		gInRect.left=gMainScrn.left;
		gInRect.top+=65;
		gInRect.right=gInRect.left+80;
		gInRect.bottom=gInRect.top+60;
	}
	
	// change the file's MPSR resource...
	SetMPSRRes(theSpec);
	
	err=ForceFinderUpdate(theSpec,false);
	return err;
}

/*
	SetCreator
	
	Sets the creator for a file.
*/
void SetCreator(FSSpec* fsp,OSType type){
	FInfo info;
	OSErr err;
	
	err=FSpGetFInfo(fsp,&info);
	
	if (info.fdCreator!=type){
		
		info.fdCreator=type;
		
		err=FSpSetFInfo(fsp,&info);
		
		if (err!=noErr){
			DSLocalError("\pError changing the file's creator.",err);
		}
	}
}

/*
	SetMPSRRes
	
	Set the MPSR resource to the one out of the application's resource fork.
*/
void SetMPSRRes(FSSpecPtr myFSSPtr){
	Handle h;
	short fref;
	OSErr rerr;
	EditRSRCHdl mpsr=(EditRSRCHdl)gMPSR;
	long date=gModTime;
	
	fref=FSpOpenResFile(myFSSPtr,fsRdWrPerm);
	rerr=ResError();
	
	if (fref==-1){ // there was an error...
		
		if ((rerr==mapReadErr)||(rerr==-39)){// no resource fork.  Create the res fork and re-open
			FSpCreateResFile(myFSSPtr,gCreator,'TEXT',smSystemScript);
			rerr=ResError();
			
			if (rerr!=noErr){
				DSLocalError("\pUnable to create the resource fork for this file.",rerr);
				return;
			}
			
			fref=FSpOpenResFile(myFSSPtr,fsRdWrPerm);
			rerr=ResError();
			
			if (fref==-1){
				DSLocalError("\pSerious program malfunction.",rerr);
				return;
			}
		} else {
			 if (rerr!=-39){
				// some kind of weird funky error.
				DSLocalError("\pWeird error encountered...",rerr);
				return;
			} else { // weird number -39
				SysBeep(10);
			}
		}
	} else { // it opened ok.  This could mean that there is already an MPSR resource.
		// check to see if there is one and remove it if so
		
		UseResFile(fref);
		
		h=Get1Resource(editResType,editResID);
		rerr=ResError();
		
		if ((h!=(Handle)0) && (rerr==noErr)) {	// then the resource exists
			EditRSRCHdl tmp=(EditRSRCHdl)h;
			
			RmveResource(h); // mark it removed from the res file...
			
			// we must use the date out of this file, otherwise it could prompt some compilers to recompile
			// unchanged source code files.  So get the handle's date before disposing of it...
			HLock(h);
			date=(*tmp)->selrsrc.date;
			HUnlock(h);
			
			DisposeHandle(h);
			
			UpdateResFile(fref);
		}
	}
	// make sure we are using the correct file
	UseResFile(fref);
	
	// set the correct modification time
	(*mpsr)->selrsrc.date=date;
	
	// temporarily remove the lock on the handle...
	HUnlock(gMPSR);
	
	// add the resource to the file
	AddResource(gMPSR,editResType,editResID,"\pDrop�MPSR Resource");
	rerr=ResError();
	
	if (rerr!=noErr){
		DSLocalError("\pErrors adding MPSR resource.",rerr);
		CloseResFile(fref);
		return;
	}
	
	// update the file
	UpdateResFile(fref);
	
	// detach the resource again (so we can re-use it)...
	DetachResource(gMPSR);
	
	// replace the lock
	MoveHHi(gMPSR);
	HLock(gMPSR);
	
	// close the res file and exit...
	CloseResFile(fref);
}

/*
	DSLocalError
	
	Local error function, should put up an alert with the error message.
*/
void DSLocalError(StringPtr sp,OSErr err){
	#define	kAlertID	200
	Str255	errorStr;

	NumToString ( err, errorStr );
	ParamText ( sp,  errorStr, NULL, NULL );
	CenterAlert ( kAlertID );
	(void) Alert ( kAlertID, NULL );
}

