/*
	PassODocs.c
	
	Code for a small application that, when it receives an ODoc apple event, it will
	rebundle it and pass it to another application.
	
	06/23/95 dn - Created.
*/

#include <Icons.h>

#include "PassODocs.h"
#include "LaunchWithDocs.h"

#define kDefaultTargetSignature 'SPM '

OSType gTargetType;
Boolean gQuit;
AEEventHandlerUPP gOappUpp,gOdocUpp,gPdocUpp,gQuitUpp;
Boolean gOApp=false;

Str255 gHelpStr1,gHelpStr2;

void main(void){
	Handle rh;
	
	gOApp=false;
	gQuit=false;
	
	InitToolbox();
	Initialize();
	
	gTargetType=kDefaultTargetSignature;
	
	rh=GetResource('ATyp',128);
	if (rh!=(Handle)0L){
		// handle points to the target OSType
		gTargetType=**((OSType**)rh);
		ReleaseResource(rh);
	}
	
	MainLoop();
	
	CleanUp();
}

OSErr InitializeAppleEvents(void){
	OSErr err=noErr;
	
	gOappUpp=NewAEEventHandlerProc(HandleOApp);
	gOdocUpp=NewAEEventHandlerProc(HandleODoc);
	gPdocUpp=NewAEEventHandlerProc(HandlePDoc);
	gQuitUpp=NewAEEventHandlerProc(HandleQuit);
	
	err=AEInstallEventHandler(kCoreEventClass,kAEOpenApplication,gOappUpp, 0, FALSE);
	
	if (err!=noErr)
		return err;
	
	err=AEInstallEventHandler(kCoreEventClass,kAEOpenDocuments,gOdocUpp, 0, FALSE);
	
	if (err!=noErr)
		return err;
	
	err=AEInstallEventHandler(kCoreEventClass,kAEPrintDocuments,gPdocUpp, 0, FALSE);
	
	if (err!=noErr)
		return err;
	
	err=AEInstallEventHandler(kCoreEventClass,kAEQuitApplication,gQuitUpp, 0, FALSE);
	
	return err;
}

pascal OSErr HandleOApp(AppleEvent* ae,AppleEvent* reply,long refCon){
	OSErr err=noErr;
	
	err=HandleRequiredParams(ae);
	
	if (err==noErr){
		// do any open application stuff here.
		gOApp=true;
		
		// nothing to do, it has been done in the initialization routines...
	} else
		HandleMissedParams(reply);
	
	return err;
}

pascal OSErr HandleODoc(AppleEvent* ae,AppleEvent* reply,long refCon){
	FSSpec spec;
	AEDescList docList;
	OSErr err;
	long index,items;
	Size actualSize;
	AEKeyword keywd;
	DescType returnedType;
	
	// get the direct parameter--a descriptor list--and put it into a docList
	err = AEGetParamDesc (ae, keyDirectObject, typeAEList,&docList);
	
	if (err)
		return err;
	
	// check for missing parameters
	err = HandleRequiredParams (ae);
	
	if (err)
		return err;
	
	// count the number of descriptor records in the list
	err = AECountItems (&docList, &items);
	
	if (err==noErr){
		// now get each descriptor record from the list, coerce the returned
		// data to an FSSpec record, and open the associated file
		ProcessSerialNumber psn;
		
		for (index = 1; index <= items; index++) {
			err = AEGetNthPtr (&docList, index, typeFSS, &keywd,&returnedType,
					(Ptr)&spec, sizeof(FSSpec),&actualSize);
			if (err)
				break;
			
			err=OpenSpecifiedDocumentWithCreator(&spec,gTargetType);
			
			if (err!=noErr)
				HandleError("\pError returned when trying to pass the ODOC event.",err);
		}
		
		// By now the target application has been opened, bring it to the fore-front.
		if (CreatorToProcessID(gTargetType,&psn)){
			// it's open, bring it to the front...
			SetFrontProcess(&psn);
		}
		
		if (!gOApp) // stay open if we have previously received the OApp event...
			gQuit=true;
	}
	
	err = AEDisposeDesc (&docList);
	
	return noErr;
}

OSErr NotHandledEvent(AppleEvent* ae){
	OSErr err;
	
	if (ae->dataHandle!=nil){
		Str255 errStr="\pPassODocs can't process the event";
		
		err=AEPutParamPtr(ae,keyErrorString,typeChar,(Ptr)&errStr[1],errStr[0]);
	}
	
	return errAEEventNotHandled;
}

OSErr HandleMissedParams(AppleEvent* ae){
	OSErr err=noErr;
	
	if (ae->dataHandle!=nil){
		Str255 errStr="\pRequired parameters were not received";
		
		err=AEPutParamPtr(ae,keyErrorString,typeChar,(Ptr)&errStr[1],errStr[0]);
	}
	
	return err;
}

pascal OSErr HandlePDoc(AppleEvent* ae,AppleEvent* reply,long refCon){
	OSErr err=noErr;
	
	err=HandleRequiredParams(ae);
	
	if (err==noErr){
		// do any print document stuff here
		err=NotHandledEvent(reply);
	} else
		HandleMissedParams(reply);
	
	return err;
}

pascal OSErr HandleQuit(AppleEvent* ae,AppleEvent* reply,long refCon){
	OSErr err=noErr;
	
	err=HandleRequiredParams(ae);
	
	if (err==noErr){
		// do any error free quit stuff here.
	} else
		HandleMissedParams(reply);
	
	gQuit=true;
	
	return noErr; // force to noErr because we don't care about errors...
}

OSErr HandleRequiredParams(AppleEvent* ae){
	DescType type;
	Size size;
	OSErr err;

	err=AEGetAttributePtr(ae, keyMissedKeywordAttr,typeWildCard, &type,nil, 0, &size);

	if (err==errAEDescNotFound){	// you got all the required parameters
		err=noErr;
	} else {
		if (err == noErr)  		// you missed a required parameter
			err=errAEParamMissed;
		// else the call to AEGetAttributePtr failed
	}
	
	return err;
}

OSErr Initialize(void){
	OSErr err=noErr,serr;
	Handle mbar;
	MenuHandle mh;
	
	// Initialize our application...
	
	// install our menu bar
	mbar=GetNewMBar(kMBar);
	SetMenuBar(mbar);
	DrawMenuBar();
	mh=GetMHandle(kMApple);
	AddResMenu(mh,'DRVR');
	
	// Initialize the apple events...
	err=InitializeAppleEvents();
	
	if (err!=noErr)
		HandleError("\pInitializeAppleEvents returns an error",err);
	return err;
}

OSErr CleanUp(void){
	OSErr err=noErr;
	
	// Get rid of the UPPs for our apple event handlers...
	DisposeRoutineDescriptor(gOappUpp);
	DisposeRoutineDescriptor(gOdocUpp);
	DisposeRoutineDescriptor(gPdocUpp);
	DisposeRoutineDescriptor(gQuitUpp);
	
	return err;
}

OSErr MainLoop(void){
	OSErr err=noErr;
	EventRecord er;
	Boolean gotEvent;
	
	while (!gQuit){
		gotEvent=WaitNextEvent(everyEvent,&er,0xffffffff,(RgnHandle)0L); // no null events
		
		if (gotEvent){
			err=HandleEvent(&er);
			
			if (err!=noErr){
				//gQuit=true;
				HandleError("\pHandleEvent Returns Error",err);
			}
		}
	}
	
	return err;
}

// handle any events we get...
OSErr HandleEvent(EventRecord* erp){
	OSErr err=noErr;
	
	switch (erp->what){
		case mouseDown:
			err=HandleMouse(erp);
			break;
		case keyDown:
		case autoKey:
			err=HandleKey(erp);
			break;
		case diskEvt:
			err=HandleDisk(erp);
			break;
		case updateEvt:
			err=HandleUpdate(erp);
			break;
		case activateEvt:
			err=HandleActivateEvt(erp);
			break;
		case kHighLevelEvent:
			err=HandleHighLevelEvent(erp);
			break;
		case osEvt:
			if (erp->message & 0x01000000){
				if (erp->message & 0x00000001){ // resume event
					err=HandleActivate(erp);
				} else {
					err=HandleDeactivate(erp);
				}
			}
			
			if ((erp->message & 0xff000000)==0xfa000000){ // mouse moved
				// you only get this when a non-nil value is passed to WNE
				err=HandleMouseMoved(erp);
			}
			
			break;
		default:
			break;
	}
	
	return err;
}

OSErr HandleLimitedMouse(EventRecord* erp,short* ppart,Boolean* handled){
	OSErr err=noErr;
	short part;
	WindowPtr wind;
	Point pt,lpt;
	long ms;
	
	pt=erp->where;
	lpt=pt;
	LocalToGlobal(&pt);
	
	part=FindWindow(erp->where,&wind);
	
	*ppart=part;
	
	if (wind==(WindowPtr)0L)
		return noErr;
	
	switch (part){
		case inMenuBar:
		case inDesk:
		case inSysWindow:
			SysBeep(5);
			break;
		case inContent:
			break;
		case inDrag:
			{
				Rect br;
				
				BlockMoveData((Ptr)(&qd.screenBits.bounds),(Ptr)&br,sizeof(Rect));
				
				InsetRect(&br,4,4);
				
				DragWindow(wind,lpt,&br);
				*handled=true;
			}
			break;
		case inGoAway:
			// perhaps we should close the window?
			break;
		case inZoomIn:
		case inZoomOut:
			{
				Boolean sel=true;
				
				if (erp->modifiers & cmdKey)
					sel=false; // don't select if the cmd key is down
				
				ZoomWindow(wind,part,sel);
				*handled=true;
			}
			break;
		default:
			// some weird part was clicked
			// we ignore it...
			SysBeep(5);
			break;
	}
	
	return err;
}

// use this routine to handle mouse clicks.
OSErr HandleMouse(EventRecord* erp){
	OSErr err=noErr;
	short part;
	WindowPtr wind;
	Point pt,lpt;
	long ms;
	
	pt=erp->where;
	lpt=pt;
	LocalToGlobal(&pt);
	
	part=FindWindow(erp->where,&wind);
	
	switch (part){
		case inMenuBar:
			err=HandleMenuEnabling();
			ms=MenuSelect(lpt);
			err=HandleMenu(ms);
			break;
		case inDesk:
		case inSysWindow:
			SystemClick(erp,wind);
			break;
		case inContent:
			SelectWindow(wind);
			break;
		case inDrag:
			{
				Rect br;
				
				BlockMoveData((Ptr)(&qd.screenBits.bounds),(Ptr)&br,sizeof(Rect));
				
				InsetRect(&br,4,4);
				
				DragWindow(wind,pt,&br);
			}
			break;
		case inGoAway:
			// we shouldn't get these...
			DisposeWindow(wind);
			break;
		case inZoomIn:
		case inZoomOut:
			{
				Boolean sel=true;
				
				if (erp->modifiers & cmdKey)
					sel=false; // don't select if the cmd key is down
				
				ZoomWindow(wind,part,sel);
			}
			break;
		default:
			// some weird part was clicked
			// we ignore it...
			break;
	}
	
	return err;
}

OSErr HandleMenuEnabling(void){
	OSErr err=noErr;
	WindowPtr wind;
	Boolean closeEnabled=true;
	MenuHandle mh;
	
	wind=FrontWindow();
	
	mh=GetMHandle(kMFile);
	
	if (wind==(WindowPtr)0L){
		// there are no active windows to close
		// DisableItem(mh,kMClose);
	} else {
		// there are active windows to close
		// EnableItem(mh,kMClose);
	}
	
	return err;
}

OSErr HandleMenu(long ms){
	OSErr err=noErr;
	short menu,item;
	
	menu=HiWord(ms);
	item=LoWord(ms);
	
	if (menu!=0){
		// a valid menu choice
		switch (menu){
			case kMApple:
				err=HandleApple(item);
				break;
			case kMFile:
				err=HandleFile(item);
				break;
			default:
				// could be an error, but I am ignoring them...
				break;
		}
	} else {
		ms=MenuChoice();
		
		// ms represents a disabled choice the user picked.
	}
	
	HiliteMenu(0);
	
	return err;
}

OSErr HandleError(StringPtr errStr,OSErr err){
	OSErr ierr=noErr;
	Str255 numErr;
	long lerr;
	
	// build the error number string
	lerr=(long)err;
	NumToString(lerr,numErr);
	
	// have paramtext fill in the alert
	ParamText(errStr,numErr,"\p","\p");
	
	// let stopalert display the error message.
	StopAlert(130,(ModalFilterUPP)0L);
	
	return ierr;
}

OSErr HandleApple(short item){
	OSErr err=noErr;
	
	switch (item){
		case kMAbout:
			err=HandleAbout();
			break;
		default:
			// handle da's here
			{
				MenuHandle mh=GetMHandle(kMApple);
				Str255 da;
				GrafPtr savePort;
				
				GetItem(mh,item,da);
				
				GetPort(&savePort);
				
				OpenDeskAcc(da);
				
				SetPort(savePort);
			}
			break;
	}
	
	return err;
}

OSErr HandleFile(short item){
	OSErr err=noErr;
	
	switch (item){
		case kMOpen:
			err=HandleOpen();
			break;
		case kMPrefs:
			// we shouldn't get one of these...
			err=HandlePrefs();
			break;
		case kMQuit:
			gQuit=true;
			break;
		default:
			break;
	}
	
	if (err!=noErr)
		HandleError("\pHandleFile returning error",err);
	
	return err;
}

OSErr HandleEdit(short item){
	OSErr err=noErr;
	
	// This application doesn't use any editing features so they
	// are being ignored...
	
	return err;
}

// use this routine to handle key events.
OSErr HandleKey(EventRecord* erp){
	OSErr err=noErr;
	char ch,key;
	long mk;
	
	ch=erp->message&charCodeMask;
	key=(erp->message&keyCodeMask)>>8;
	
	if (erp->modifiers & cmdKey){
		// menu event
		err=HandleMenuEnabling();
		mk=MenuKey(ch);
		err=HandleMenu(mk);
	} else {
		// normal key events
		
		// currently ignored
	}
	
	return err;
}

// Handle window activate events here
OSErr HandleActivateEvt(EventRecord* erp){
	OSErr err=noErr;
	WindowPtr wind;
	
	wind=(WindowPtr)erp->message;
	
	if (wind!=(WindowPtr)0L){
		if (erp->modifiers&activeFlag){
			BringToFront(wind); // make it the frontmost
			HiliteWindow(wind,true); // activate the window...
		} else {
			HiliteWindow(wind,false); // deactivate the window...
		}
	}
	
	return err;
}

// Handle Window Updates
OSErr HandleUpdate(EventRecord* erp){
	OSErr err=noErr;
	WindowPtr wind;
	
	wind=(WindowPtr)erp->message;
	
	BeginUpdate(wind);
	EndUpdate(wind);
	
	return err;
}

// Handle Disk Events here...
OSErr HandleDisk(EventRecord* erp){
	OSErr err=noErr;
	Point pt;
	
	SetPt(&pt,100,100);
	err=(OSErr)DIBadMount(pt,erp->message);
	
	return err;
}

// Entry point for Apple Event Handling
OSErr HandleHighLevelEvent(EventRecord* erp){
	OSErr err=noErr;
	
	err=AEProcessAppleEvent(erp);
	
	return err;
}

// use this routine to handle activate events...
OSErr HandleActivate(EventRecord* erp){
	OSErr err=noErr;
	WindowPtr wind;
	
	wind=FrontWindow();
	
	if (wind!=(WindowPtr)0L)
		HiliteWindow(wind,true); // hilite the window
	
	return err;
}

// use this routine to handle the deactivate events
OSErr HandleDeactivate(EventRecord* erp){
	OSErr err=noErr;
	WindowPtr wind;
	
	wind=FrontWindow();
	if (wind!=(WindowPtr)0L)
		HiliteWindow(wind,false); // deactivate the window
	
	return err;
}

// use this routine to change the mouse cursor depending on where the mouse is located.
OSErr HandleMouseMoved(EventRecord* erp){
	OSErr err=noErr;
	
	return err;
}

// Initialize the Mac Toolbox
OSErr InitToolbox(void){
	OSErr err=noErr;
	
	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	FlushEvents(everyEvent,0);
	TEInit();
	InitDialogs(0L);
	InitCursor();
	
	return err;
}

OSErr HandleAbout(void){
	OSErr err=noErr;
	
	NoteAlert(128,(ModalFilterUPP)0L);
	
	return err;
}

OSErr HandleOpen(void){
	OSErr err=noErr;
	FSSpec spec;
	SFTypeList list;
	StandardFileReply reply;
	ProcessSerialNumber psn;
	
	list[0]='****';
	list[1]='fold';
	list[2]='disk';
	list[3]='text';
	
	StandardGetFile((FileFilterUPP)0L,-1,list,&reply);
	
	if (reply.sfGood){
		// hand the doc to the target application...
		
		err=OpenSpecifiedDocumentWithCreator(&(reply.sfFile),gTargetType);
		
		if (err==noErr){
			// By now the target application has been opened, bring it to the fore-front.
			if (CreatorToProcessID(gTargetType,&psn)){
				// it's open, bring it to the front...
				err=SetFrontProcess(&psn);
			}
		} else
			HandleError("\pError returned while trying to open the document.",err);
	}
	
	return err;
}

OSErr HandleAppSelection(void){
	OSErr err=noErr;
	FSSpec spec;
	SFTypeList list;
	StandardFileReply reply;
	ProcessSerialNumber psn;
	
	list[0]='APPL';
	
	StandardGetFile((FileFilterUPP)0L,1,list,&reply);
	
	if (reply.sfGood){
		// hand the doc to the target application...
		FInfo info;
		
		err=FSpGetFInfo(&(reply.sfFile),&info);
		
		if (err==noErr){
			gTargetType=info.fdCreator;
		}
	}
	
	return err;
}

OSErr ReplacePrefs(void){
	OSErr err=noErr;
	Handle rh;
	OSType tmp;
	OSType* rp;
	OSType** th;
	
	rh=GetResource('ATyp',128);
	
	if (rh==(Handle)0L){
		// then there isn't one already...
		rh=NewHandle(sizeof(OSType));
		
		AddResource(rh,'ATyp',128,"\pPassODocs Target Application Creator");
	}
	
	HLock(rh);
	
	th=(OSType**)rh;
	tmp=**th;
	
	**th=gTargetType;
	
	HUnlock(rh);
	
	ChangedResource(rh);
	WriteResource(rh);
	ReleaseResource(rh);
	
	return err;
}

OSErr HandlePrefs(void){
	OSErr err=noErr;
	DialogPtr dlg;
	short i;
	Rect r;
	Handle h;
	UserItemUPP frame,icons,box,help,htext,title;
	ModalFilterUPP custom;
	GrafPtr savePort;
	short itemHit;
	OSType type;
	Str255 typeStr;
	
	GetPort(&savePort);
	
	dlg=GetNewDialog(129,(Ptr)0L,(WindowPtr)-1);
	if (dlg!=(DialogPtr)0L){
		// use the dialog
		
		SetPort((GrafPtr)dlg);
		
		SetHelp("\pHold down the shift key while clicking on an item to see",
			"\pa description of the item.");
		
		// first frame the ok button...
		GetDItem(dlg,12,&i,&h,&r);
		frame=NewUserItemProc(ItemOvalFrame);
		SetDItem(dlg,12,i,(Handle)frame,&r);
		
		// now do the icon family...
		GetDItem(dlg,4,&i,&h,&r);
		icons=NewUserItemProc(ItemIconFamily);
		SetDItem(dlg,4,i,(Handle)icons,&r);
		
		// and the outline
		GetDItem(dlg,9,&i,&h,&r);
		box=NewUserItemProc(ItemBox);
		SetDItem(dlg,9,i,(Handle)box,&r);
		
		// and the text
		GetDItem(dlg,10,&i,&h,&r);
		help=NewUserItemProc(ItemHelpText);
		SetDItem(dlg,10,i,(Handle)help,&r);
		
		GetDItem(dlg,8,&i,&h,&r);
		title=NewUserItemProc(ItemHelpTitle);
		SetDItem(dlg,8,i,(Handle)title,&r);
		
		GetDItem(dlg,11,&i,&h,&r);
		htext=NewUserItemProc(ItemHelpStrings);
		SetDItem(dlg,11,i,(Handle)htext,&r);
		
		OSTypeToStr255(gTargetType,typeStr);
		GetDItem(dlg,5,&i,&h,&r);
		SelIText(dlg,5,0,100);
		SetIText(h,typeStr);
		SelIText(dlg,5,0,100);
		
		itemHit=5;
		
		custom=NewModalFilterProc(CustomFilter);
		
		ShowWindow((WindowPtr)dlg);
		SetDialogTracksCursor(dlg,true);
		
		while ((itemHit<1)||(itemHit>3)){ // while not the ok button
			ModalDialog(custom,&itemHit);
			
			switch (itemHit){
				case 1:
					// save the new app sig
					GetDItem(dlg,5,&i,&h,&r);
					GetIText(h,typeStr);
					Str255ToOSType(typeStr,&type);
					gTargetType=type;
					
					err=ReplacePrefs();
					
					break;
				case 2:
					// do not save the sig
					break;
				case 3:
					// only copy it to the global
					GetDItem(dlg,5,&i,&h,&r);
					GetIText(h,typeStr);
					Str255ToOSType(typeStr,&type);
					gTargetType=type;
					break;
				case 5:
					// text editing
					break;
				case 7:
					// select application
					err=HandleAppSelection();
					
					if (err==noErr){
						OSTypeToStr255(gTargetType,typeStr);
						GetDItem(dlg,5,&i,&h,&r);
						SelIText(dlg,5,0,100);
						SetIText(h,typeStr);
						SelIText(dlg,5,0,100);
					}
					break;
				default:
					// do nothing special
					break;
			}
		}
		
		DisposeDialog(dlg);
		
		DisposeRoutineDescriptor(custom);
		DisposeRoutineDescriptor(frame);
		DisposeRoutineDescriptor(icons);
		DisposeRoutineDescriptor(help);
		DisposeRoutineDescriptor(htext);
		DisposeRoutineDescriptor(title);
		DisposeRoutineDescriptor(box);
	} else {
		err=ResError();
	}
	
	SetPort(savePort);
	
	return err;
}

pascal void ItemOvalFrame(WindowPtr wind,short item){
	DialogPtr dlg;
	short i;
	Rect r;
	Handle h;
	
	dlg=(DialogPtr)wind;
	
	GetDItem(dlg,item,&i,&h,&r);
	
	PenSize(2,2);
	
	FrameRoundRect(&r,8,8);
	
	PenNormal();
}

pascal void ItemBox(WindowPtr wind,short item){
	DialogPtr dlg;
	short i;
	Rect r;
	Handle h;
	
	dlg=(DialogPtr)wind;
	
	GetDItem(dlg,item,&i,&h,&r);
	
	FrameRect(&r);
}

pascal void ItemHelpText(WindowPtr wind,short item){
	DialogPtr dlg;
	short i;
	Rect r;
	Handle h;
	short cf;
	short sz;
	short st;
	
	dlg=(DialogPtr)wind;
	
	cf=((GrafPtr)dlg)->txFont;
	sz=((GrafPtr)dlg)->txSize;
	st=((GrafPtr)dlg)->txFace;
	
	GetDItem(dlg,item,&i,&h,&r);
	
	EraseRect(&r);
	InsetRect(&r,2,2);
	
	MoveTo(r.left+6,r.bottom-6);
	
	TextFont(geneva);
	TextFace(normal);
	TextSize(9);
	
	DrawString("\pHelp");
	
	TextFont(cf);
	TextFace(st);
	TextSize(sz);
}

pascal void ItemHelpTitle(WindowPtr wind,short item){
	DialogPtr dlg;
	short i;
	Rect r;
	Handle h;
	short cf;
	short sz;
	short st;
	
	dlg=(DialogPtr)wind;
	
	cf=((GrafPtr)dlg)->txFont;
	sz=((GrafPtr)dlg)->txSize;
	st=((GrafPtr)dlg)->txFace;
	
	GetDItem(dlg,item,&i,&h,&r);
	
	EraseRect(&r);
	InsetRect(&r,2,2);
	
	MoveTo(r.left,r.bottom-3);
	
	TextFont(times);
	TextFace(bold);
	TextSize(18);
	
	DrawString("\pPassODocs Application Specification");
	
	TextFont(cf);
	TextFace(st);
	TextSize(sz);
}

void SetHelp(StringPtr s1,StringPtr s2){
	
	BlockMoveData((Ptr)s1,(Ptr)gHelpStr1,s1[0]+1);
	BlockMoveData((Ptr)s2,(Ptr)gHelpStr2,s2[0]+1);
}

pascal void ItemHelpStrings(WindowPtr wind,short item){
	DialogPtr dlg;
	short i;
	Rect r;
	Handle h;
	short cf;
	short sz;
	short st;
	
	dlg=(DialogPtr)wind;
	
	cf=((GrafPtr)dlg)->txFont;
	sz=((GrafPtr)dlg)->txSize;
	st=((GrafPtr)dlg)->txFace;
	
	GetDItem(dlg,item,&i,&h,&r);
	
	EraseRect(&r);
	InsetRect(&r,2,2);
	
	TextFont(geneva);
	TextFace(normal);
	TextSize(9);
	
	MoveTo(r.left,r.top+12);
	DrawString(gHelpStr1);
	
	MoveTo(r.left,r.top+24);
	DrawString(gHelpStr2);
	
	TextFont(cf);
	TextFace(st);
	TextSize(sz);
}

pascal void ItemAboutText(WindowPtr wind,short item){
}

#define kDarkValue 10000
#define kLightValue 63000

pascal void Item3DBox(WindowPtr wind,short item){
	DialogPtr dlg;
	short i;
	Rect r;
	Handle h;
	RGBColor ltg,dkg;
	RGBColor save;
	
	GetForeColor(&save);
	
	dlg=(DialogPtr)wind;
	
	GetDItem(dlg,item,&i,&h,&r);
	
	PenNormal();
	
	PenSize(2,2);
	
	ltg.red=kLightValue;
	ltg.green=kLightValue;
	ltg.blue=kLightValue;
	
	dkg.red=kDarkValue;
	dkg.green=kDarkValue;
	dkg.blue=kDarkValue;
	
	RGBForeColor(&ltg);
	MoveTo(r.left,r.bottom);
	LineTo(r.left,r.top);
	LineTo(r.right,r.top);
	
	RGBForeColor(&dkg);
	LineTo(r.right,r.bottom);
	LineTo(r.left+2,r.bottom);
	
	PenNormal();
	
	RGBForeColor(&save);
}

pascal void ItemIconFamily(WindowPtr wind,short item){
	DialogPtr dlg;
	short i;
	Rect r;
	Handle h;
	
	dlg=(DialogPtr)wind;
	
	GetDItem(dlg,item,&i,&h,&r);
	
	PlotIconID(&r,atVerticalCenter+atHorizontalCenter,ttNone,128);
}

short DlgFindItem(DialogPtr dlg,Point pt){
	Handle h;
	short i,loop;
	Rect r;
	
	GetDItem(dlg,1,&i,&h,&r);
	if (PtInRect(pt,&r))
		return 1;
	
	GetDItem(dlg,2,&i,&h,&r);
	if (PtInRect(pt,&r))
		return 2;
	
	GetDItem(dlg,3,&i,&h,&r);
	if (PtInRect(pt,&r))
		return 3;
	
	GetDItem(dlg,7,&i,&h,&r);
	if (PtInRect(pt,&r))
		return 7;
	
	GetDItem(dlg,5,&i,&h,&r);
	if (PtInRect(pt,&r))
		return 5;
	
	return -1;
}

pascal Boolean CustomFilter(DialogPtr dlg,EventRecord* erp,short* item){
	Boolean hand=false;
	Handle h;
	short i;
	Rect r;
	GrafPtr savePort;
	short part;
	ControlHandle c;
	Point pt;
	
	GetPort(&savePort);
	SetPort((GrafPtr)dlg);
	
	switch (erp->what){
		case keyDown:
		case autoKey:
			if (erp->modifiers & cmdKey){
				// check for cmd key messages
				switch ((erp->message)&charCodeMask){
					case 'S':
					case 's':
					case 0x0d: // return key
					case 0x03: // enter key
						// hit the save button
						GetDItem(dlg,1,&i,(Handle*)&c,&r);
						HiliteControl(c,inButton);
						HiliteControl(c,0);
						break;
					case 'T':
					case 't':
						// hit the temporary button
						GetDItem(dlg,3,&i,(Handle*)&c,&r);
						HiliteControl(c,inButton);
						HiliteControl(c,0);
						break;
					case 'C':
					case 'c':
					case 0x1b: // escape key
						// hit the cancel button
						GetDItem(dlg,2,&i,(Handle*)&c,&r);
						HiliteControl(c,inButton);
						HiliteControl(c,0);
						break;
				}
			} else {
				switch ((erp->message)&charCodeMask){
					case 0x0d: // return key
					case 0x03: // enter key
						*item=1;
						GetDItem(dlg,1,&i,(Handle*)&c,&r);
						HiliteControl(c,inButton);
						HiliteControl(c,0);
						hand=true;
					case 0x1b: // escape key
						*item=2;
						hand=true;
						GetDItem(dlg,2,&i,(Handle*)&c,&r);
						HiliteControl(c,inButton);
						HiliteControl(c,0);
						break;
				}
			}
			break;
		case mouseDown:
			// movable modal dlg...
			
			HandleLimitedMouse(erp,&part,&hand);
			
			if (part==inContent){
				if (erp->modifiers & shiftKey){
					// want to display help text...
					
					pt=erp->where;
					GlobalToLocal(&pt);
					
					i=DlgFindItem(dlg,pt);
					
					if (i!=-1){
						switch (i){
							case 1:
								SetHelp("\pSaves the application signature for this PassODocs application.",
									"\pThe signature is saved in the resource fork so it only applies to this application.");
								GetDItem(dlg,1,&i,(Handle*)&c,&r);
								HiliteControl(c,inButton);
								HiliteControl(c,0);
								break;
							case 2:
								SetHelp("\pExit from the dialog without making any changes.","\p");
								GetDItem(dlg,2,&i,(Handle*)&c,&r);
								HiliteControl(c,inButton);
								HiliteControl(c,0);
								break;
							case 3:
								SetHelp("\pMake the application signature the temporary target only while",
									"\pthis PassODocs application is running.");
								GetDItem(dlg,3,&i,(Handle*)&c,&r);
								HiliteControl(c,inButton);
								HiliteControl(c,0);
								break;
							case 4:
								SetHelp("\pPassODocs icon.","\p");
								break;
							case 5:
								SetHelp("\pThe application signature specifies which application PassODocs should",
									"\ppass all ODOC events to.");
								break;
							case 7:
								SetHelp("\pStandard dialog for choosing an application instead of entering the application's",
									"\psignature.");
								GetDItem(dlg,7,&i,(Handle*)&c,&r);
								HiliteControl(c,inButton);
								HiliteControl(c,0);
								break;
						}
					}
					// invalidate the rectangle so that we get an update event...
					GetDItem(dlg,11,&i,&h,&r);
					InvalRect(&r);
					
					*item=4; // set the item to the icon
					hand=true;
				}
			}
			break;
		case updateEvt:
			{
				WindowPtr wind=(WindowPtr)erp->message;
				
				if ((wind!=(WindowPtr)0)&&((DialogPtr)(wind)!=dlg)){
					// got an update event for a background window!
					
					// By calling BeginUpdate/EndUpdate, the update events will stop coming to us.
					
					BeginUpdate(wind);
					
					// call the window's drawing procedure here...
					
					EndUpdate(wind);
				}
			}
			break;
			
		default:
			// do nothing special
			break;
	}
	
	SetPort(savePort);
	
	return hand;
}

void OSTypeToStr255(OSType type,StringPtr sp){
	register unsigned char* tp = (unsigned char*) &type;
	register unsigned char* cp = sp;
	register short  n = sizeof(OSType);
	
	*cp++ = n;
	while ( n-- )
		*cp++ = *tp++;
}

void Str255ToOSType(StringPtr sp,OSType* type){
	register unsigned char* tp=(unsigned char*) type;
	register unsigned char* cp=sp;
	register short n=sizeof(OSType);
	
	cp++;
	
	while (n--)
		*tp++ = *cp++;
}





