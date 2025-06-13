/* DocDialogs - Implementation of the configuration dialogs                   */

#include "DocDialogs.h"


		// ¥ Toolbox
#ifndef __ALIASES__
#include "Aliases.h"
#endif

#ifndef NUM_ALT_ADDRS
#include "AddressXlation.h"
#endif

#ifndef __GESTALTEQU__
#include "GestaltEqu.h"
#endif

#ifndef __RESOURCES__
#include "Resources.h"
#endif

#ifndef __STRINGS__
#include "Strings.h"
#endif

		// ¥ Implementation use
#ifndef __GLOBALS__
#include "Globals.h"
#endif

#ifndef __MTPSTUFF__
#include "MTPStuff.h"
#endif


//------------------------------------------------------------------------------

const int kS7GetDirButton = 10;
const int kS7GetDirNowButton = 11;

const int kGetDirButton = 11;
const int kGetDirNowButton = 12;

const unsigned char kBullet = '¥';
const unsigned char kDummyVirtualCode = 49;

//------------------------------------------------------------------------------

pascal void GetCommValues (TMUDDoc *theDoc, TWindow *aWindow);
pascal void GetPrefValues (TMUDDoc *theDoc, TWindow *aWindow);
pascal void GetSetupValues (TMUDDoc *theDoc, TWindow *aWindow);
pascal void GetTCPValues (TMUDDoc *theDoc, TWindow *aWindow);

pascal void SetCommValues (TMUDDoc *theDoc, TWindow *aWindow);
pascal void SetPrefValues (TMUDDoc *theDoc, TWindow *aWindow);
pascal void SetSetupValues (TMUDDoc *theDoc, TWindow *aWindow);
pascal void SetTCPValues (TMUDDoc *theDoc, TWindow *aWindow);

//------------------------------------------------------------------------------

#pragma segment SDocDialogs

class TSelButton: public TButton {
public:
	virtual pascal void DoChoice (TView *origView, short itsChoice);
};

static TMUDDoc *theSelDoc;
static Boolean curDirValid;
static short myRefNum;
static long myCurDir;
static StandardFileReply s7reply;
static SFReply reply;

pascal short S7MyGetDirHook (short item, DialogPtr, void *)
{
	Str255 tmp;
	CInfoPBRec pb;
	FSSpec spec;
	Boolean isFolder, wasAliased;

	switch (item) {
	case kS7GetDirButton:
		spec = s7reply.sfFile;
		ResolveAliasFile (&spec, TRUE, &isFolder, &wasAliased);
		if (isFolder) {
			CopyStr255 (spec.name, tmp);
			pb.dirInfo.ioNamePtr = tmp;
			pb.dirInfo.ioVRefNum = spec.vRefNum;
			pb.dirInfo.ioDrDirID = spec.parID;
			pb.dirInfo.ioFDirIndex = 0;
			PBGetCatInfo (&pb, FALSE);
			myCurDir = pb.dirInfo.ioDrDirID;
			myRefNum = spec.vRefNum;
			curDirValid = true;
			return sfItemCancelButton;
		}
		break;
	case kS7GetDirNowButton:
		myCurDir = *(long *)CurDirStore;
		myRefNum = -(*(short *) SFSaveDisk);
		curDirValid = true;
		return sfItemCancelButton;
	}
	return item;
}

pascal Boolean S7FoldersOnly (ParmBlkPtr p, void *)
{
	return (p->fileParam.ioFlAttrib & 0x10) == 0;
}

pascal short MyGetDirHook (short item, DialogPtr)
{
	switch (item) {
	case kGetDirButton:
		if (reply.fType != 0) {
			myRefNum = -(*(short *) SFSaveDisk);
			myCurDir = reply.fType;
			curDirValid = true;
			return getCancel;
		}
		break;
	case kGetDirNowButton:
		myRefNum = -(*(short *) SFSaveDisk);
		myCurDir = *(long *)CurDirStore;
		curDirValid = true;
		return getCancel;
	}
	return item;
}

pascal Boolean FoldersOnly (ParmBlkPtr p)
{
	return (p->fileParam.ioFlAttrib & 0x10) == 0;
}

pascal void TSelButton::DoChoice (TView *, short)
{
	Rect r;
	DialogTHndl dlg;
	Point loc;
	Str255 tmp;
	Boolean sys7;
	long response;
	OSErr err;
	
	err = Gestalt (gestaltAliasMgrAttr, &response);
	sys7 = (err == noErr) && (response & (1 << gestaltAliasMgrPresent)) != 0;
	if (sys7) {
		err = Gestalt (gestaltStandardFileAttr, &response);
		sys7 = (err == noErr) && (response & (1 << gestaltStandardFile58)) != 0;
	}
	if (sys7)
		dlg = (DialogTHndl) GetResource ('DLOG', phS7GetDirID);
	else
		dlg = (DialogTHndl) GetResource ('DLOG', phGetDirID);
	if (dlg != NULL) {
		r = (**dlg).boundsRect;
		CenterRectOnScreen (&r, TRUE, TRUE, TRUE);
		loc.h = r.left;
		loc.v = r.top;
	} else {
		loc.h = 100;
		loc.v = 100;
	}
	gApplication->UpdateAllWindows ();
	curDirValid = FALSE;
	if (sys7)
		CustomGetFile (S7FoldersOnly, -1, (SFTypeList) NULL, &s7reply,
			phS7GetDirID, loc, S7MyGetDirHook, NULL, NULL, NULL, NULL);
	else
		SFPGetFile (loc, "\p", FoldersOnly, -1, (SFTypeList) NULL, MyGetDirHook,
			&reply, phGetDirID, NULL);
	if (curDirValid) MTPDirSetup (theSelDoc, myRefNum, myCurDir);
	SetText (theSelDoc->fMTPDir, kDontRedraw);
	ForceRedraw ();
}


//------------------------------------------------------------------------------

#pragma segment MACommandRes

class TTypeButton: public TButton {
public:
	unsigned long fFileType;
	
	virtual pascal void DoChoice (TView *origView, short itsChoice);
};

pascal void TTypeButton::DoChoice (TView *, short)
{
	AppFile anAppFile;
	FInfo info;
	
	if (gApplication->ChooseDocument (cPrefs, &anAppFile)) {
		FailOSErr (GetFInfo (anAppFile.fName, anAppFile.vRefNum, &info));
		fFileType = info.fdCreator;
		SetText (anAppFile.fName, kDontRedraw);
		ForceRedraw ();
	}
}


//------------------------------------------------------------------------------

#pragma segment DlgRes

class TVT100Box: public TCheckBox {
public:
	virtual pascal void DoChoice (TView *origView, short itsChoice);
};

pascal void TVT100Box::DoChoice (TView *origView, short itsChoice)
{
	TWindow *aWindow;
	TCheckBox *aCheckBox;

	inherited::DoChoice (origView, itsChoice);
	if (itsChoice == mCheckBoxHit) {
		aWindow = GetWindow ();
		aCheckBox = (TCheckBox *) aWindow->FindSubView ('skip');
		aCheckBox->DimState (IsOn (), kRedraw);
	}
}


//------------------------------------------------------------------------------

#pragma segment SDocDialogs

pascal void Communication (TMUDDoc *theDoc)
{
	TWindow *aWindow;
	Boolean okPressed;
	TDialogView *aDialogView;
	
	aWindow = NewTemplateWindow (kCommID, NULL);
	FailNIL (aWindow);
	SetCommValues (theDoc, aWindow);
	aDialogView = (TDialogView *) aWindow->FindSubView ('DLOG');
	okPressed = aDialogView->PoseModally () == 'OK  ';
	if (okPressed) GetCommValues (theDoc, aWindow);
	aWindow->Close ();
	if (! okPressed) Failure (noErr, msgCancelled);
	theDoc->fChangeCount += 1;
}

//------------------------------------------------------------------------------

#pragma segment SDocDialogs

pascal void ConfigTCPHandler (short , long , void *Handler_StaticLink)
{
	((TWindow *) Handler_StaticLink)->Close ();
}

pascal void ConfigTCP (TMUDDoc *theDoc)
{
	TWindow *aWindow;
	Boolean okPressed;
	TDialogView *aDialogView;
	FailInfo fi;
	
	aWindow = NewTemplateWindow (kConfigTCPID, NULL);
	FailNIL (aWindow);
	CatchFailures (&fi, ConfigTCPHandler, (void *) aWindow);
	SetTCPValues (theDoc, aWindow);
	aDialogView = (TDialogView *) aWindow->FindSubView ('DLOG');
	okPressed = aDialogView->PoseModally () == 'OK  ';
	if (okPressed) GetTCPValues (theDoc, aWindow);
	Success (&fi);
	aWindow->Close ();
	if (! okPressed) Failure (noErr, msgCancelled);
	theDoc->fChangeCount += 1;
}

//------------------------------------------------------------------------------

#pragma segment SDocDialogs

pascal void GetCommValues (TMUDDoc *theDoc, TWindow *aWindow)
{
//	TNumberText *aNumberText;
	TCluster *aCluster;
	ResType aType;
	TCheckBox *aCheckBox;
	
//	aNumberText = (TNumberText *) aWindow->FindSubView ('intc');
//	theDoc->fInterrupt = (unsigned char) aNumberText->GetValue ();
	aCluster = (TCluster *) aWindow->FindSubView ('eolc');
	aType = aCluster->ReportCurrent ();
	switch (aType) {
	case 'eol1':
		theDoc->fEndLine = 1;
		break;
	case 'eol2':
		theDoc->fEndLine = 2;
		break;
	default:
		theDoc->fEndLine = 0;
		break;
	}
	aCheckBox = (TCheckBox *) aWindow->FindSubView ('teln');
	theDoc->fUseTelnet = aCheckBox->IsOn ();
	aCheckBox = (TCheckBox *) aWindow->FindSubView ('echo');
	theDoc->fEcho = aCheckBox->IsOn ();
	theDoc->fDoEcho = theDoc->fEcho;
	aCheckBox = (TCheckBox *) aWindow->FindSubView ('vt10');
	theDoc->fParseVT100 = aCheckBox->IsOn ();
	aCheckBox = (TCheckBox *) aWindow->FindSubView ('skip');
	theDoc->fSkipCR = aCheckBox->IsOn ();
}

//------------------------------------------------------------------------------

#pragma segment SDocDialogs

pascal void GetPrefValues (TMUDDoc *theDoc, TWindow *aWindow)
{
	TEditText *aEditText;
	TNumberText *aNumberText;
	TPopup *aPopup;
	TTypeButton *aTypeButton;
	short item;
	int i;
	Str255 tmp;
	TextStyle aStyle;
	Boolean setStyle;
	
	aEditText = (TEditText *) aWindow->FindSubView ('name');
	aEditText->GetText (tmp);
	CopyStr255 (tmp, theDoc->fPlayer);
	aEditText = (TEditText *) aWindow->FindSubView ('pass');
	aEditText->GetText (tmp);
	CopyStr255 (tmp, theDoc->fPasswd);
	aPopup = (TPopup *) aWindow->FindSubView ('font');
	item = aPopup->GetCurrentItem ();
	GetItem (aPopup->fMenuHandle, item, tmp);
	CopyStr255 (tmp, theDoc->fFontName);
	item = GetFontNum (tmp);
	setStyle = (theDoc->fFontNum != item);
	theDoc->fFontNum = item;
	aNumberText = (TNumberText *) aWindow->FindSubView ('tabs');
	i = (int) aNumberText->GetValue ();
	setStyle |= (theDoc->fTabSize != i);
	theDoc->fTabSize = i;
	aNumberText = (TNumberText *) aWindow->FindSubView ('size');
	i = (int) aNumberText->GetValue ();
	setStyle |= (theDoc->fFontSize != i);
	theDoc->fFontSize = i;
	aNumberText = (TNumberText *) aWindow->FindSubView ('logs');
	theDoc->fLogSize = (int) aNumberText->GetValue ();
	aNumberText = (TNumberText *) aWindow->FindSubView ('hist');
	theDoc->fHistSize = (int) aNumberText->GetValue ();
	aTypeButton = (TTypeButton *) aWindow->FindSubView ('ftyp');
	aTypeButton->GetText (tmp);
	CopyStr255 (tmp, theDoc->fCreatorApp);
	theDoc->fTextSig = aTypeButton->fFileType;
	if (setStyle) {
		aStyle.tsFont = theDoc->fFontNum;
		aStyle.tsFace = 0;
		aStyle.tsSize = theDoc->fFontSize;
		aStyle.tsColor = gRGBBlack;
		theDoc->fLogWindow->fLogView->SetBaseStyle (&aStyle, kRedraw);
		theDoc->fLogWindow->fCommandView->SetBaseStyle (&aStyle, kRedraw);
	}
}

//------------------------------------------------------------------------------

#pragma segment SDocDialogs

pascal void GetSetupValues (TMUDDoc *theDoc, TWindow *aWindow)
{
	TCheckBox *aCheckBox;
	TNumberText *aNumberText;
	TCluster *aCluster;
	ResType aType;
	
	aCluster = (TCluster *) aWindow->FindSubView ('prot');
	aType = aCluster->ReportCurrent ();
	switch (aType) {
	case 'pro1':
		theDoc->fProtocol = 1;
		break;
	default:
		theDoc->fProtocol = 0;
		break;
	}
	aCheckBox = (TCheckBox *) aWindow->FindSubView ('ctab');
	theDoc->fCvtTab = aCheckBox->IsOn ();
	aNumberText = (TNumberText *) aWindow->FindSubView ('tabw');
	theDoc->fSTab = (int) aNumberText->GetValue ();
	aCheckBox = (TCheckBox *) aWindow->FindSubView ('updt');
	theDoc->fUpdate = aCheckBox->IsOn ();
}

//------------------------------------------------------------------------------

#pragma segment SDocDialogs

pascal void GetTCPValues (TMUDDoc *theDoc, TWindow *aWindow)
{
	TEditText *aEditText;
	TNumberText *aNumberText;
	Str255 tmp;
	
	aEditText = (TEditText *) aWindow->FindSubView ('addr');
	aEditText->GetText (tmp);
	CopyStr255 (tmp, theDoc->fHostName);
	aNumberText = (TNumberText *) aWindow->FindSubView ('port');
	theDoc->fTCPPort = (int) aNumberText->GetValue ();
	aNumberText = (TNumberText *) aWindow->FindSubView ('mtpp');
	theDoc->fMTPPort = (int) aNumberText->GetValue ();
}

//------------------------------------------------------------------------------

#pragma segment SDocDialogs

pascal void Preferences (TMUDDoc *theDoc)
{
	TWindow *aWindow;
	Boolean okPressed;
	TDialogView *aDialogView;
	
	aWindow = NewTemplateWindow (kPrefsID, NULL);
	FailNIL (aWindow);
	SetPrefValues (theDoc, aWindow);
	aDialogView = (TDialogView *) aWindow->FindSubView ('DLOG');
	okPressed = aDialogView->PoseModally () == 'OK  ';
	if (okPressed) GetPrefValues (theDoc, aWindow);
	aWindow->Close ();
	if (!okPressed) Failure (noErr, msgCancelled);
	theDoc->fLogWindow->fLogView->SetMaxSize (theDoc->fLogSize * 1024);
	theDoc->fLogWindow->fCommandView->SetHistSize (theDoc->fHistSize);
	theDoc->fChangeCount += 1;
}

//------------------------------------------------------------------------------

#pragma segment SDocDialogs

pascal void SetCommValues (TMUDDoc *theDoc, TWindow *aWindow)
{
//	TNumberText *aNumberText;
	TRadio *aRadio;
	ResType aType;
	TCheckBox *aCheckBox;
	
//	aNumberText = (TNumberText *) aWindow->FindSubView ('intc');
//	aNumberText->SetValue (theDoc->fInterrupt, kDontRedraw);
	switch (theDoc->fEndLine) {
	case 1:
		aType = 'eol1';
		break;
	case 2:
		aType = 'eol2';
		break;
	default:
		aType = 'eol0';
		break;
	}
	aRadio = (TRadio *) aWindow->FindSubView (aType);
	aRadio->SetState (TRUE, kDontRedraw);
	aCheckBox = (TCheckBox *) aWindow->FindSubView ('teln');
	aCheckBox->SetState (theDoc->fUseTelnet, kDontRedraw);
	aCheckBox = (TCheckBox *) aWindow->FindSubView ('echo');
	aCheckBox->SetState (theDoc->fEcho, kDontRedraw);
	aCheckBox = (TCheckBox *) aWindow->FindSubView ('vt10');
	aCheckBox->SetState (theDoc->fParseVT100, kDontRedraw);
	aCheckBox = (TCheckBox *) aWindow->FindSubView ('skip');
	aCheckBox->SetState (theDoc->fSkipCR, kDontRedraw);
	aCheckBox->DimState (theDoc->fParseVT100, kDontRedraw);
}

//------------------------------------------------------------------------------

#pragma segment SDocDialogs

pascal void SetPrefValues (TMUDDoc *theDoc, TWindow *aWindow)
{
	TEditText *aEditText;
	TNumberText *aNumberText;
	TPopup *aPopup;
	TTypeButton *aTypeButton;
	short num, item;
	Str255 tmp, aFontName;
	
	aEditText = (TEditText *) aWindow->FindSubView ('name');
	CopyStr255 (theDoc->fPlayer, tmp);
	aEditText->SetText (tmp, kDontRedraw);
	aEditText = (TEditText *) aWindow->FindSubView ('pass');
	CopyStr255 (theDoc->fPasswd, tmp);
	aEditText->SetText (tmp, kDontRedraw);
	aPopup = (TPopup *) aWindow->FindSubView ('font');
	AddResMenu (aPopup->fMenuHandle, 'FONT');
	aPopup->AdjustBotRight ();
	CopyStr255 (theDoc->fFontName, tmp);
	num = CountMItems (aPopup->fMenuHandle);
	for (item = 1; item < num; item++) {
		GetItem (aPopup->fMenuHandle, item, aFontName);
		if (CompareStrings (tmp, aFontName) == 0) {
			aPopup->SetCurrentItem (item, kDontRedraw);
			break;
		}
	}
	aNumberText = (TNumberText *) aWindow->FindSubView ('size');
	aNumberText->SetValue (theDoc->fFontSize, kDontRedraw);
	aNumberText = (TNumberText *) aWindow->FindSubView ('tabs');
	aNumberText->SetValue (theDoc->fTabSize, kDontRedraw);
	aNumberText = (TNumberText *) aWindow->FindSubView ('logs');
	aNumberText->SetValue (theDoc->fLogSize, kDontRedraw);
	aNumberText = (TNumberText *) aWindow->FindSubView ('hist');
	aNumberText->SetValue (theDoc->fHistSize, kDontRedraw);
	aTypeButton = (TTypeButton *) aWindow->FindSubView ('ftyp');
	CopyStr255 (theDoc->fCreatorApp, tmp);
	aTypeButton->SetText (tmp, kDontRedraw);
	aTypeButton->fFileType = theDoc->fTextSig;
}

//------------------------------------------------------------------------------

#pragma segment SDocDialogs

pascal void SetSetupValues (TMUDDoc *theDoc, TWindow *aWindow)
{
	TCheckBox *aCheckBox;
	TNumberText *aNumberText;
	TButton *aButton;
	Str255 tmp;
	TRadio *aRadio;
	ResType aType;
	
	switch (theDoc->fProtocol) {
	case 1:
		aType = 'pro1';
		break;
	default:
		aType = 'pro0';
		break;
	}
	aRadio = (TRadio *) aWindow->FindSubView (aType);
	aRadio->SetState (TRUE, kDontRedraw);
	theSelDoc = theDoc;
	aCheckBox = (TCheckBox *) aWindow->FindSubView ('ctab');
	aCheckBox->SetState (theDoc->fCvtTab, kDontRedraw);
	aNumberText = (TNumberText *) aWindow->FindSubView ('tabw');
	aNumberText->SetValue (theDoc->fSTab, kDontRedraw);
	aCheckBox = (TCheckBox *) aWindow->FindSubView ('updt');
	aCheckBox->SetState (theDoc->fUpdate, kDontRedraw);
	aButton = (TButton *) aWindow->FindSubView ('dsel');
	CopyStr255 (theDoc->fMTPDir, tmp);
	aButton->SetText (tmp, kDontRedraw);
}

//------------------------------------------------------------------------------

#pragma segment SDocDialogs

pascal void SetTCPValues (TMUDDoc *theDoc, TWindow *aWindow)
{
	TEditText *aEditText;
	TNumberText *aNumberText;
	Str255 tmp;
	
	CopyStr255 (theDoc->fHostName, tmp);
	aEditText = (TEditText *) aWindow->FindSubView ('addr');
	aEditText->SetText (tmp, kDontRedraw);
	aNumberText = (TNumberText *) aWindow->FindSubView ('port');
	aNumberText->SetValue (theDoc->fTCPPort, kDontRedraw);
	aNumberText = (TNumberText *) aWindow->FindSubView ('mtpp');
	aNumberText->SetValue (theDoc->fMTPPort, kDontRedraw);
}

//------------------------------------------------------------------------------

#pragma segment SDocDialogs

static Str31 oldVol, oldDir;
static long oldDirID;
static short oldVRefNum;

pascal void SetupHandler (short , long , void *theDoc)
{
	CopyStr255 (oldVol, ((TMUDDoc *)theDoc)->fMTPVol);
	CopyStr255 (oldDir, ((TMUDDoc *)theDoc)->fMTPDir);
	((TMUDDoc *)theDoc)->fMTPDirID = oldDirID;
	((TMUDDoc *)theDoc)->fMTPVRefNum = oldVRefNum;
}

pascal void Setup (TMUDDoc *theDoc)
{
	TWindow *aWindow;
	Boolean okPressed;
	TDialogView *aDialogView;
	FailInfo fi;
	
	aWindow = NewTemplateWindow (kSetupID, NULL);
	FailNIL (aWindow);
	SetSetupValues (theDoc, aWindow);
	CopyStr255 (theDoc->fMTPVol, oldVol);
	CopyStr255 (theDoc->fMTPDir, oldDir);
	oldDirID = theDoc->fMTPDirID;
	oldVRefNum = theDoc->fMTPVRefNum;
	aDialogView = (TDialogView *) aWindow->FindSubView ('DLOG');
	CatchFailures (&fi, SetupHandler, (void *) theDoc);
	okPressed = aDialogView->PoseModally () == 'OK  ';
	if (okPressed) GetSetupValues (theDoc, aWindow);
	aWindow->Close ();
	if (!okPressed) Failure (noErr, msgCancelled);
	Success (&fi);
	theDoc->fChangeCount += 1;
}

//------------------------------------------------------------------------------

#pragma segment MAInit

pascal void InitDocDialogs (void)
{
	if (gDeadStripSuppression) {
		TSelButton *aSelButton = new TSelButton;
		TTypeButton *aTypeButton = new TTypeButton;
		TVT100Box *aVT100Box = new TVT100Box;
	}
}

//------------------------------------------------------------------------------
