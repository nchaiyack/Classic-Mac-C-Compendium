// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UPreferencesView.cp

#include "UPreferencesView.h"
#include "UPrefsDatabase.h"
#include "UNntp.h"
#include "UFolderSelectView.h"
#include "Tools.h"
#include "UArticleTextCache.h"
#include "NetAsciiTools.h"
#include "ViewTools.h"

#include <Rsrcglobals.h>
#include <ErrorGlobals.h>

#include <Folders.h>
#include <UDialog.h>
#include <ToolUtils.h>

#ifndef __STDIO__
#include <stdio.h>
#endif

#pragma segment MyDialogs

#if qDebug
#define macroCheckSubView(view, x) if (!view) {ProgramBreak(CStr255(x) + " is missing");Failure(minErr, 0);}
#else
#define macroCheckSubView(view, x)
#endif

TPrefRammeView::TPrefRammeView()
{
}

pascal void TPrefRammeView::Initialize()
{
	inherited::Initialize();
}

pascal void TPrefRammeView::ReadFields(TStream *aStream)
{
	inherited::ReadFields(aStream);
	FailInfo fi;
	if (fi.Try())
	{
		short id = aStream->ReadInteger();
		TextStyle itsTextStyle;
		MAGetTextStyle(id, itsTextStyle);
		fTextStyle = itsTextStyle;
		fRammeVertOffset = aStream->ReadInteger();
		fTextHorzOffset = aStream->ReadInteger();
		fTextVertOffset = aStream->ReadInteger();
		id = aStream->ReadInteger();
		short index = aStream->ReadInteger();
		CStr255 s;
		GetIndString(s, id, index);
		fText = s;
		fi.Success();
	}
	else // fail
	{
		Free();
		fi.ReSignal();
	}
}

void pascal TPrefRammeView::Free()
{
	inherited::Free();
}

pascal void TPrefRammeView::Draw(const VRect& /* area */)
{
	CRect r;
	ViewToQDRect(VRect(VPoint(0, 0), fSize), r);
	Lock(true);
	SetPortTextStyle(fTextStyle);
	MoveTo(fTextHorzOffset, fTextVertOffset);
	DrawChar(' ');
	DrawString(fText);
	DrawChar(' ');
	Lock(false);
	PenPat(&qd.gray);
	Move(0, fRammeVertOffset - fTextVertOffset);
	LineTo(r.right - 1, fRammeVertOffset);
	LineTo(r.right - 1, r.bottom - 1);
	LineTo(r.left, r.bottom - 1);
	LineTo(0, fRammeVertOffset);
	LineTo(fTextHorzOffset, fRammeVertOffset);
}

// =====================================================================
void DoNewsServerPreferencesDialog()
{
	TWindow *window = gViewServer->NewTemplateWindow(kNewsServerPrefsView, nil);
	macroCheckSubView(window, "Could not create NewsServerPrefs window");
	if (!window) 
		return;
// set all the control-states

	window->Center(true, true, window->IsModal() );
	gPrefs->GetSilentWindowPosPrefs('WNPe', window);
	CStr255 s, pre, post;
	
	TCheckBox *alwaysAuth = (TCheckBox*) window->FindSubView('AlAu');
	alwaysAuth->SetState(gPrefs->GetBooleanPrefs('AlAu'), kRedraw);

	
// News-Server
	TStaticText *serverIPST = (TStaticText*) window->FindSubView('SeIP');
	macroCheckSubView(serverIPST, "Missing server IP static text");
	TStaticText *serverNameST = (TStaticText*) window->FindSubView('SeNa');
	macroCheckSubView(serverNameST, "Missing server name static text");
	
	char cs[40];
	gPrefs->GetStringPrefs('SvNa', s);
	MyGetIndString(pre, kNewsServerPreName);
	MyGetIndString(post, kNewsServerPostName);
	serverNameST->SetText(pre + s + post, kRedraw);

	long ip = gPrefs->GetLongPrefs('SvIP');
	sprintf(cs, "%ld.%ld.%ld.%ld",
		(ip >> 24) & 255,
		(ip >> 16) & 255, 
		(ip >>  8) & 255,
		(ip      ) & 255);
	MyGetIndString(pre, kNewsServerPreIP);
	MyGetIndString(post, kNewsServerPostIP);
	serverIPST->SetText(pre + cs + post, kRedraw);

// Timeout:
	TNumberText *timeoutNT = (TNumberText*) window->FindSubView('Time');
	macroCheckSubView(timeoutNT, "Missing timeout TNumberText");
	timeoutNT->SetValue(gPrefs->GetLongPrefs('STio'), kRedraw);

// Max idle nntp connections
	TNumberText *maxIdleConnectionsNT = (TNumberText*) window->FindSubView('Mntp');
	macroCheckSubView(maxIdleConnectionsNT, "Missing max # idle TNumberText");
	maxIdleConnectionsNT->SetValue(gPrefs->GetLongPrefs('Mntp'), kRedraw);
	
// Max idle time for nntp connections
	TNumberText *maxIdleTimeNT = (TNumberText*) window->FindSubView('Tntp');
	macroCheckSubView(maxIdleTimeNT, "Missing max idle time TNumberText");
	long maxIdle = gPrefs->GetLongPrefs('Tntp') / 60 / 60;
	maxIdleTimeNT->SetValue(maxIdle, kRedraw);

// Max articles in XHDR command
	TNumberText *maxXHDRArticlesNT = (TNumberText*) window->FindSubView('XHSz');
	macroCheckSubView(maxXHDRArticlesNT, "Missing maxXHDRArticlesNT TNumberText");
	maxXHDRArticlesNT->SetValue(gPrefs->GetLongPrefs('XHSz'), kRedraw);

// Max articles in XHDR command
	TNumberText *poolNewNT = (TNumberText*) window->FindSubView('CNAT');
	macroCheckSubView(poolNewNT, "Missing poolNewNT TNumberText");
	poolNewNT->SetValue(gPrefs->GetLongPrefs('CNAT'), kRedraw);

// TCHR
	TPopup *transPU = (TPopup*)window->FindSubView('Tran');
	macroCheckSubView(transPU, "Missing transPU TPopup");
	MenuHandle transMenuH = MAGetMenu(mNewsServerTranslateMenu);
	FailNILResource(Handle(transMenuH));
	if (!CountMItems(transMenuH))
	{
		AddResMenu(transMenuH, 'TCHR');
		NeedCalcMenuSize(transMenuH);
		transPU->SetLongMax(transPU->GetNumberOfItems(), kDontRedraw);
	}
	short itemNo = CountMItems(transMenuH);
	CStr255 currentTransName;
	gPrefs->GetStringPrefs('Tran', currentTransName);
	while (itemNo >= 1)
	{
		CStr255 itemName;
		GetItem(transMenuH, itemNo, itemName);
		if (itemName == currentTransName)
		{
			transPU->SetCurrentItem(itemNo, !kRedraw);
			break;
		}
		itemNo--;
	}

// show the thing
	window->Open();
	IDType dismisser = MyPoseModally(window);
	gPrefs->SetWindowPosPrefs('WNPe', window);
	if (dismisser != 'ok  ')
	{
		window->CloseByUser();
		return;
	}
	gPrefs->SetBooleanPrefs('AlAu', alwaysAuth->IsOn());
	GetItem(transMenuH, transPU->GetVal(), currentTransName);
	gPrefs->SetStringPrefs('Tran', currentTransName);
	LoadTranslateTable(currentTransName, gMac2NetAscii, gNetAscii2Mac);

	gPrefs->SetLongPrefs('CNAT', poolNewNT->GetValue());
	gPrefs->SetLongPrefs('XHSz', maxXHDRArticlesNT->GetValue());
	gPrefs->SetLongPrefs('STio', timeoutNT->GetValue());
	long maxIdleConnections = maxIdleConnectionsNT->GetValue();
	gPrefs->SetLongPrefs('Mntp', maxIdleConnections);
	long maxIdleTime = maxIdleTimeNT->GetValue() * 60 * 60;
	gPrefs->SetLongPrefs('Tntp', maxIdleTime);
	gNntpCache->UpdateTiming();

	window->Show(false, kDontRedraw);	
	gArticleTextCache->FlushCache(); // changes translate table
	window->CloseByUser();
}

// =====================================================================
// --------------------------------------------------------------------------
// now some _realy_ ugly code that checks the names
void ShowTheNameError(short rsrcID, short errorIndex)
{
	CStr255 msg;
	GetIndString(msg, rsrcID, errorIndex);
	ParamText(gEmptyString, gEmptyString, gEmptyString, msg);
	StdAlert(phNameError);
}

Boolean CheckIt(TEditText *et,
								short minNoAT, short maxNoAT, 
								Boolean allowLB, // <> allowed
								short rsrcID)
{
	CStr255 text;
	et->GetText(text);
	if (text.Length() == 0)
	{
		et->BecomeTarget();
		et->SetSelection(0, 0, kRedraw);
		ShowTheNameError(rsrcID, 1);
		return false;
	}
	short numAT = 0;
	short numC = 0;
	for (short i = 1; i <= text.Length(); i++)
	{
		char ch = text[i];
		if (ch == '@')
		{
			numAT++;
			if (numAT > maxNoAT)
			{
				et->BecomeTarget();
				et->SetSelection(i - 1, i, kRedraw);
				ShowTheNameError(rsrcID, 2);
				return false;
			}
		}
		if (ch == '(')
			numC++;
		if (ch == ')')
		{
			numC--;
			if (numC < 0)
			{
				et->BecomeTarget();
				et->SetSelection(i - 1, i, kRedraw);
				ShowTheNameError(rsrcID, 5);
				return false;
			}
		}
		if (!allowLB && (ch == '<' || ch == '>'))
		{
			et->BecomeTarget();
			et->SetSelection(i - 1, i, kRedraw);
			ShowTheNameError(rsrcID, 4);
			return false;
		}
	}
	if (numAT < minNoAT)
	{
		et->BecomeTarget();
		et->SetSelection(0, text.Length(), kRedraw);
		ShowTheNameError(rsrcID, 3);
		return false;
	}
	if (numC)
	{
		et->BecomeTarget();
		et->SetSelection(0, text.Length(), kRedraw);
		ShowTheNameError(rsrcID, 5);
		return false;
	}
	return true;
}

void DoYourNamePreferencesDialog()
{
	TWindow *window = gViewServer->NewTemplateWindow(kYourNamePrefsView, nil);
	if (!window) 
		return;
// set all the control-states

	window->Center(true, true, window->IsModal() );
	gPrefs->GetSilentWindowPosPrefs('WYPe', window);
	CStr255 s;
	
// Names
	TEditText *realNameET = (TEditText*)window->FindSubView('Name');
	TEditText *snailNameET = (TEditText*)window->FindSubView('@adr');
	TEditText *organizationET = (TEditText*)window->FindSubView('Orga');

	CStr255 realName;
	gPrefs->GetStringPrefs('Name', realName);
	realNameET->SetText(realName, kRedraw);

	CStr255 snailName;
	gPrefs->GetStringPrefs('@adr', snailName);
	snailNameET->SetText(snailName, kRedraw);

	CStr255 orgaName;
	gPrefs->GetStringPrefs('Orga', orgaName);
	organizationET->SetText(orgaName, kRedraw);

// show the thing
	window->Open();
	while (true)
	{
		IDType dismisser = MyPoseModally(window);
		gPrefs->SetWindowPosPrefs('WYPe', window);
		if (dismisser != 'ok  ')
		{
			window->CloseByUser();
			return;
		}
		if (!CheckIt(realNameET, 0, 0, false, kYourNameErrorStrings))
			continue;
		if (!CheckIt(snailNameET, 1, 1, false, kYourSnailErrorStrings))
			continue;
		if (!CheckIt(organizationET, 0, 32767, true, kOrganizErrorStrings))
			continue;
		break;
	}
	realNameET->GetText(realName);
	gPrefs->SetStringPrefs('Name', realName);
	snailNameET->GetText(snailName);
	gPrefs->SetStringPrefs('@adr', snailName);
	organizationET->GetText(orgaName);
	gPrefs->SetStringPrefs('Orga', orgaName);

	window->Show(false, kDontRedraw);	
	window->CloseByUser();
	gPrefs->SetBooleanPrefs('NaOK', true);
}

// =====================================================================
// =====================================================================
void AskFolderDoEvent(void *, TView *view, EventNumber, TEventHandler*, TEvent*)
{
	TWindow *w = view->GetWindow();
	TCheckBox *askFilenameCB = (TCheckBox*) w->FindSubView('AsFn');
	TFolderSelectView *folSel = (TFolderSelectView*)w->FindSubView('ChFo');
	Boolean askFilename = askFilenameCB->IsOn();
	folSel->DimState(askFilename, kRedraw);
	TCheckBox *useUUname = (TCheckBox*) w->FindSubView('_UU_');
	if (askFilename)
		useUUname->SetState(false, !kRedraw);
	useUUname->DimState(askFilename, kRedraw);
}

void AskLaunchExtractorDoEvent(void *, TView *view, EventNumber, TEventHandler*, TEvent*)
{
	TWindow *w = view->GetWindow();
	TCheckBox *doLaunchCB = (TCheckBox*) w->FindSubView('BLau');
	TFileSelectView *applSel = (TFileSelectView*)w->FindSubView('BLID');
	Boolean doLaunch = doLaunchCB->IsOn();
	applSel->DimState(!doLaunch, kRedraw);
	TCheckBox *odocCB = (TCheckBox*) w->FindSubView('BLOP');
	odocCB->DimState(!doLaunch, kRedraw);
}

void DoBinariesPreferencesDialog()
{
	TWindow *window = gViewServer->NewTemplateWindow(kBinariesPrefsView, nil);
	if (!window) 
		return;
// set all the control-states

	window->Center(true, true, window->IsModal() );
	gPrefs->GetSilentWindowPosPrefs('WBPe', window);
	CStr255 s;
	
// Extracting binaries
	TCheckBox *askFilenameCB = (TCheckBox*) window->FindSubView('AsFn');
	TCheckBox *doLaunchCB = (TCheckBox*) window->FindSubView('BLau');
	TCheckBox *odocCB = (TCheckBox*) window->FindSubView('BLOP');
	TCheckBox *useUUname = (TCheckBox*) window->FindSubView('_UU_');
	TFolderSelectView *folSel = (TFolderSelectView*)window->FindSubView('ChFo');
	folSel->InitializeFromPreferences('FBin');

	TFileSelectView *applSel = (TFileSelectView*)window->FindSubView('BLID');
	macroCheckSubView(applSel, "applSel");
	applSel->SpecifyFileTypes('APPL', '????');
	applSel->InitializeFromPreferences('BLid');

	Boolean doLaunch = gPrefs->GetBooleanPrefs('BLau');
	doLaunchCB->SetState(doLaunch, kRedraw);
	AddActionBehaviour(doLaunchCB, AskLaunchExtractorDoEvent, nil);
	odocCB->SetState(gPrefs->GetBooleanPrefs('BLOP'), kRedraw);
	applSel->DimState(!doLaunch, kRedraw);
	odocCB->DimState(!doLaunch, kRedraw);
	
	Boolean askFilename = gPrefs->GetBooleanPrefs('BiAs');
	askFilenameCB->SetState(askFilename, kRedraw);
	AddActionBehaviour(askFilenameCB, AskFolderDoEvent, nil);
	
	folSel->DimState(askFilename, kRedraw);
	useUUname->DimState(askFilename, kRedraw);
	useUUname->SetState(gPrefs->GetBooleanPrefs('UUna'), kRedraw);
	
// show the thing
	window->Open();
	IDType dismisser = MyPoseModally(window);
	gPrefs->SetWindowPosPrefs('WBPe', window);
	if (dismisser != 'ok  ')
	{
		window->CloseByUser();
		return;
	}
	window->Show(false, kDontRedraw);
	
	gPrefs->SetBooleanPrefs('BLau', doLaunchCB->IsOn());
	gPrefs->SetBooleanPrefs('BLOP', odocCB->IsOn());
	gPrefs->SetBooleanPrefs('BiAs', askFilenameCB->IsOn());
	gPrefs->SetBooleanPrefs('UUna', useUUname->IsOn());
	folSel->StoreInPreferences('FBin');
	applSel->StoreInPreferences('BLid');
	window->CloseByUser();
}
// =====================================================================
void DoEditorPreferencesDialog()
{
	TWindow *window = gViewServer->NewTemplateWindow(kEditorPrefsView, nil);
	if (!window) 
		return;
// set all the control-states

	window->Center(true, true, window->IsModal() );
	gPrefs->GetSilentWindowPosPrefs('WEPe', window);
	CStr255 s;
	
// Editor
	TCheckBox *defaultUseSignatureCB = (TCheckBox*) window->FindSubView('UsSi');
	macroCheckSubView(defaultUseSignatureCB, "defaultUseSignatureCB");
	defaultUseSignatureCB->SetState(gPrefs->GetBooleanPrefs('DUSi'), kRedraw);

	TFileSelectView *applSel = (TFileSelectView*)window->FindSubView('ChEd');
	macroCheckSubView(applSel, "applSel");
	applSel->SpecifyFileTypes('APPL', '????');
	applSel->InitializeFromPreferences('EDid');

	TFolderSelectView *folderSel = (TFolderSelectView*)window->FindSubView('ChFo');
	macroCheckSubView(folderSel, "folderSel");
	folderSel->InitializeFromPreferences('FEdi');
	
// signature
	TFileSelectView *sigFile = (TFileSelectView*) window->FindSubView('ChSi');
	macroCheckSubView(sigFile, "sigFile");
	sigFile->SpecifyFileTypes('TEXT', '????');
	sigFile->InitializeFromPreferences('Sigu');

	TCheckBox *editSignatureCB = (TCheckBox*) window->FindSubView('EdSi');
	macroCheckSubView(editSignatureCB, "editSignatureCB");
	editSignatureCB->SetState(gPrefs->GetBooleanPrefs('EdSi'), kRedraw);
	
	TCheckBox *editHeadersCB = (TCheckBox*) window->FindSubView('EdHe');
	macroCheckSubView(editHeadersCB, "editHeadersCB");
	editHeadersCB->SetState(gPrefs->GetBooleanPrefs('EdHe'), kRedraw);

// auto wrap
	TNumberText *wrapNT = (TNumberText*) window->FindSubView('WrLn');
	macroCheckSubView(wrapNT, "Missing auto-wrap TNumberText");
	wrapNT->SetValue(gPrefs->GetLongPrefs('WrLn'), kRedraw);

// ISO-2022 encoding for posting
	TCheckBox *encodeCB = (TCheckBox*) window->FindSubView('2022');
	macroCheckSubView(encodeCB, "encodeCB");
	encodeCB->SetState(gPrefs->GetBooleanPrefs('2022'), kRedraw);

// show the thing
	window->Open();
	IDType dismisser = MyPoseModally(window);
	gPrefs->SetWindowPosPrefs('WEPe', window);
	if (dismisser != 'ok  ')
	{
		window->CloseByUser();
		return;
	}
	window->Show(false, kDontRedraw);
	
	folderSel->StoreInPreferences('FEdi');
	applSel->StoreInPreferences('EDid');
	if (applSel->GotFile())
		gPrefs->SetSignaturePrefs('EDsi', applSel->GetFileSignature());
	gPrefs->SetBooleanPrefs('DUSi', defaultUseSignatureCB->IsOn());
	gPrefs->SetBooleanPrefs('EdSi', editSignatureCB->IsOn());
	gPrefs->SetBooleanPrefs('EdHe', editHeadersCB->IsOn());
	sigFile->StoreInPreferences('Sigu');
	gPrefs->SetLongPrefs('WrLn', wrapNT->GetValue());
	gPrefs->SetBooleanPrefs('2022', encodeCB->IsOn());
	window->CloseByUser();
}

// =====================================================================
void AskUseMailerLaunchDocDoEvent(void *, TView *view, EventNumber, TEventHandler*, TEvent*)
{
	TWindow *w = view->GetWindow();
	TCheckBox *useDocCB = (TCheckBox*) w->FindSubView('EuOp');
	TFileSelectView *docSel = (TFileSelectView*)w->FindSubView('Eudo');
	Boolean useDoc = useDocCB->IsOn();
	docSel->DimState(!useDoc, kRedraw);
}

void DoMailerPreferencesDialog()
{
	TWindow *window = gViewServer->NewTemplateWindow(kMailerPrefsView, nil);
	if (!window) 
		return;
// set all the control-states

	window->Center(true, true, window->IsModal() );
	gPrefs->GetSilentWindowPosPrefs('WMPe', window);
	CStr255 s;
	
// Eudora application
	TFileSelectView *eudoraAppl = (TFileSelectView*) window->FindSubView('EuAp');
	macroCheckSubView(eudoraAppl, "eudoraAppl");
	eudoraAppl->SpecifyFileTypes('APPL', 'CSOm');
	eudoraAppl->InitializeFromPreferences('EuAp');	

// Eudora document
	TFileSelectView *eudoraFile = (TFileSelectView*) window->FindSubView('Eudo');
	macroCheckSubView(eudoraFile, "eudoraFile");
	eudoraFile->SpecifyFileTypes('PREF', 'CSOm');
	eudoraFile->InitializeFromPreferences('Eudo');	

	TCheckBox *useDocCB = (TCheckBox*) window->FindSubView('EuOp');
	macroCheckSubView(useDocCB, "useDocCB");
	Boolean useDoc = gPrefs->GetBooleanPrefs('EuOp');
	useDocCB->SetState(useDoc, kRedraw);
	eudoraFile->DimState(!useDoc, kRedraw);
	AddActionBehaviour(useDocCB, AskUseMailerLaunchDocDoEvent, nil);	

// show the thing
	window->Open();
	IDType dismisser = MyPoseModally(window);
	gPrefs->SetWindowPosPrefs('WMPe', window);
	if (dismisser != 'ok  ')
	{
		window->CloseByUser();
		return;
	}
	window->Show(false, kDontRedraw);
	
	eudoraAppl->StoreInPreferences('EuAp');
	eudoraFile->StoreInPreferences('Eudo');
	gPrefs->SetBooleanPrefs('EuOp', useDocCB->IsOn());

	window->CloseByUser();
}
// =====================================================================
void DoMiscPreferencesDialog()
{
	TWindow *window = gViewServer->NewTemplateWindow(kMiscPrefsView, nil);
	if (!window) 
		return;
// set all the control-states

	window->Center(true, true, window->IsModal() );
	gPrefs->GetSilentWindowPosPrefs('WIPe', window);
	
// Notes folder
	TFolderSelectView *folderSel = (TFolderSelectView*)window->FindSubView('ChFo');
	macroCheckSubView(folderSel, "folderSel");
	folderSel->InitializeFromPreferences('FNot');

// Auto update
	TCheckBox *autoUpdateCB = (TCheckBox*) window->FindSubView('AuUp');
	macroCheckSubView(autoUpdateCB, "autoUpdateCB");
	autoUpdateCB->SetState(gPrefs->GetBooleanPrefs('AuUp'), kRedraw);
	
// Check for new groups at startup
	TCheckBox *checkNewGroupsCB = (TCheckBox*) window->FindSubView('ChNw');
	macroCheckSubView(checkNewGroupsCB, "checkNewGroupsCB");
	checkNewGroupsCB->SetState(gPrefs->GetBooleanPrefs('ChNw'), kRedraw);

// Save open windows
	TCheckBox *saveWindowsInfoCB = (TCheckBox*) window->FindSubView('WSav');
	macroCheckSubView(saveWindowsInfoCB, "saveWindowsInfoCB");
	saveWindowsInfoCB->SetState(gPrefs->GetBooleanPrefs('WSav'), kRedraw);
	
// Article cache size:
	TNumberText *articleCacheSizeNT = (TNumberText*) window->FindSubView('AMem');
	macroCheckSubView(articleCacheSizeNT, "Missing article cache size TNumberText");
	articleCacheSizeNT->SetValue(gPrefs->GetLongPrefs('AMem') / 1024, kRedraw);

// Max number of articles in group database:
	TNumberText *maxArticlesInGroupNT = (TNumberText*) window->FindSubView('Mart');
	macroCheckSubView(maxArticlesInGroupNT, "Missing max articles in DB TNumberText");
	maxArticlesInGroupNT->SetValue(gPrefs->GetLongPrefs('Mart'), kRedraw);

// show the thing
	window->Open();
	IDType dismisser = MyPoseModally(window);
	gPrefs->SetWindowPosPrefs('WIPe', window);
	if (dismisser != 'ok  ')
	{
		window->CloseByUser();
		return;
	}
	window->Show(false, kDontRedraw);
	
	folderSel->StoreInPreferences('FNot');
	gPrefs->SetBooleanPrefs('AuUp', autoUpdateCB->IsOn());
	gPrefs->SetBooleanPrefs('ChNw', checkNewGroupsCB->IsOn());
	gPrefs->SetBooleanPrefs('WSav', saveWindowsInfoCB->IsOn());
	gPrefs->SetLongPrefs('AMem', articleCacheSizeNT->GetValue() * 1024);
	gPrefs->SetLongPrefs('Mart', maxArticlesInGroupNT->GetValue());

	window->CloseByUser();
}
