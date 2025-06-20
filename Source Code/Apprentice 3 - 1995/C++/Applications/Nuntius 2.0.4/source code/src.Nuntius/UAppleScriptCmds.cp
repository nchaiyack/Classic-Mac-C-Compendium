// Copyright � 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UAppleScriptCmds.cp

#include "UAppleScriptCmds.h"
#include "Tools.h"
#include "UArticleListView.h"
#include "UArticle.h"
#include "UArticleCache.h"
#include "UGroupDoc.h"
#include "ProcessTools.h"
#include "NetAsciiTools.h"
#include "UPrefsDatabase.h"
#include "UPrefsFileMgr.h"
#include "UGroupDocCmds.h"
#include "FileTools.h"
#include "UASyncFile.h"
#include "UNewsAppl.h"

#include "RsrcGlobals.h"
#include "ErrorGlobals.h"

#include <AppleScript.h>
#include <OSEvents.h>
#include <GestaltEqu.h>
#include <Processes.h>
#include <OSA.h>
#include <ASRegistry.h>
#include <Errors.h>
#include <Finder.h>


ComponentInstance gOSAComponent = nil;
AEDesc gNuntiusAddressSpec;

#pragma segment MyArticle
//=============================================================================

OSAActiveUPP gOldActiveProc = nil;

pascal OSErr MyActiveProc(long refcon)
{
	gApplication->UpdateAllWindows();
	if (gOldActiveProc)
		return CallOSAActiveProc(gOldActiveProc, refcon);
	else
		return noErr;
}

OSErr LoadAppleScript()
{
	if (gOSAComponent)
		return noErr;
	long response;
	if (Gestalt(gestaltAppleEventsAttr, &response) != noErr
	|| ((response & gestaltScriptingSupport) == 0))
		return errOSACantOpenComponent;
	if (Gestalt(gestaltAppleScriptVersion, &response) != noErr
	|| (response == 0x01008000))
		return errTooOldAppleScriptVersion;
	gOSAComponent = ::OpenDefaultComponent(kOSAComponentType, kOSAGenericScriptingComponentSubtype);
	if (gOSAComponent == nil) // no such component
		return errOSACantOpenComponent;
	FailOSErr(AECreateDesc(typeProcessSerialNumber, &gApplication->fProcessNumber,
						sizeof(gApplication->fProcessNumber), &gNuntiusAddressSpec));
	long refcon;
	FailOSErr(OSAGetActiveProc(gOSAComponent, &gOldActiveProc, &refcon));
	FailOSErr(OSASetActiveProc(gOSAComponent, NewOSAActiveProc(&MyActiveProc), refcon));
	return noErr;
}

Boolean ScriptFileExist(FSSpec &spec, HParamBlockRec &pb)
{
	OSErr err = MyGetFileInfo(spec, pb);
	if (err == fnfErr)
		return false;
	FailOSErr(err);
	if ((pb.fileParam.ioFlFndrInfo.fdFlags & kIsAlias) == 0)
		return true; // not an alias file
	Boolean targetIsFolder, wasAliased;
	err = ResolveAliasFile(&spec, true, &targetIsFolder, &wasAliased);
	if (err == fnfErr)
		return false;
	FailOSErr(err);
	return true;
}

void FindScriptInStandardFolders(const CStr255 &appleScriptName, 
													FSSpec &spec, HParamBlockRec &pb)
{
	GetPrefsDocLocation(spec);
	CopyCString2String(appleScriptName, spec.name);
	if (ScriptFileExist(spec, pb))
		return;
	if (GoPublicFolder(spec))
	{
		CopyCString2String(appleScriptName, spec.name);
		if (ScriptFileExist(spec, pb))
			return;
	}
	ProcessInfoRec info;
	BlockSet(&info, sizeof(info), 0);
	info.processInfoLength = sizeof(ProcessInfoRec);
	info.processAppSpec = &spec;
	FailOSErr(GetProcessInformation(&gApplication->fProcessNumber, &info));
	CopyCString2String(appleScriptName, spec.name);
	if (ScriptFileExist(spec, pb))
		return;
	CStr255 privateFolderName, publicFolderName;
	MyGetIndString(privateFolderName, kPrivateFilesFolderName);
	MyGetIndString(publicFolderName, kPublicFilesFolderName);
	ParamText(appleScriptName, privateFolderName, publicFolderName, gEmptyString);
	StdAlert(phMissingStandardScript);
	Failure(0, 0);
}
//=============================================================================
#define Inherited TObject
DefineClass(TStandardAppleScript, TObject)

TStandardAppleScript::TStandardAppleScript()
{
	fAppleScriptName = gEmptyString;
	fScriptID = kOSANullScript;
	fScriptFile = nil;
	fScriptData = nil;
	fScriptComesFromTextFile = false;
}

void TStandardAppleScript::IStandardAppleScript(const CStr255 &name)
{
	Inherited::IObject();
	FailInfo fi;
	Try(fi)
	{
		FailOSErr(LoadAppleScript());
		fAppleScriptName = name;
		LoadScriptFromStandardFolders();
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(this);
		fi.ReSignal();
	}
}

void TStandardAppleScript::Free()
{
	if (fScriptID)
	{
		FailInfo fi;
		Try(fi)
		{
			SaveScript();
			fi.Success();
		}
	}
	if (fScriptID != kOSANullScript)
		OSADispose(gOSAComponent, fScriptID);
	if (fScriptFile)
		fScriptFile->CloseFile();
	FreeIfObject(fScriptFile); fScriptFile = nil;
	if (fScriptComesFromTextFile)
		fScriptData = DisposeIfHandle(fScriptData); // resource otherwise
	Inherited::Free();
}

OSAID TStandardAppleScript::GetScriptID()
{
	return fScriptID;
}

void TStandardAppleScript::LoadScriptFromTextFile(const FSSpec &spec)
{
	fScriptComesFromTextFile = true;
	fScriptData = LoadFileIntoHandle(spec);
	AEDesc scriptDesc;
	scriptDesc.descriptorType = typeChar;
	scriptDesc.dataHandle = fScriptData;
	const long modeFlags = kOSAModeAlwaysInteract | kOSAModeCompileIntoContext;
	FailOSErr(OSACompile(gOSAComponent, &scriptDesc, modeFlags, &fScriptID));
}
	
void TStandardAppleScript::LoadScriptFromCompiledFile(const FInfo &fndrInfo, const FSSpec &spec)
{
	FailInfo fi;
	Try(fi)
	{
		fScriptFile = NewAsyncFile(fndrInfo.fdType, fndrInfo.fdCreator, !kUsesDataFork, needResourceFork, !kDataOpen, kRsrcOpen);
		fScriptFile->SetPermissions(fsRdWrPerm, fsRdWrPerm);// Since we intend to write to it 
		fScriptFile->Specify(spec);
		FailOSErr(fScriptFile->OpenFile());
		fScriptData = Get1Resource(kOSAScriptResourceType, 128);
		if (fScriptData == nil)
			fScriptData = Get1IndResource(kOSAScriptResourceType, 1);
		FailNILResource(fScriptData);
		UseResFile(gApplicationRefNum);
		AEDesc scriptDesc;
		scriptDesc.descriptorType = typeOSAGenericStorage;
		scriptDesc.dataHandle = fScriptData;
		FailOSErr(OSALoad(gOSAComponent, &scriptDesc, kOSAModeAlwaysInteract, &fScriptID));
		fi.Success();
	}
	else // fail
	{
		UseResFile(gApplicationRefNum);
		fi.ReSignal();
	}
}

void TStandardAppleScript::LoadScriptFromStandardFolders()
{
	HParamBlockRec pb;
	FSSpec spec;
	FindScriptInStandardFolders(fAppleScriptName, spec, pb);
	if (pb.fileParam.ioFlFndrInfo.fdType == 'TEXT')
		LoadScriptFromTextFile(spec);
	else
		LoadScriptFromCompiledFile(pb.fileParam.ioFlFndrInfo, spec);
}

void TStandardAppleScript::SaveScript()
{
	if (fScriptComesFromTextFile)
		return; // sorry, can't save it
	AEDesc scriptDesc;
	scriptDesc.descriptorType = typeOSAGenericStorage;
	scriptDesc.dataHandle = nil;
	FailInfo fi;
	Try(fi)
	{
		FailNIL(fScriptFile);
		FailOSErr(OSAStore(gOSAComponent, fScriptID, typeOSAGenericStorage, kOSAModeAlwaysInteract, &scriptDesc));
		FailNIL(scriptDesc.dataHandle);
		long size = GetHandleSize(scriptDesc.dataHandle);
		if (size == GetHandleSize(fScriptData)
		&& memcmp(*fScriptData, *scriptDesc.dataHandle, size) == 0)
		{
			fi.Success();
			return; // not changed
		}
		SetPermHandleSize(fScriptData, size);
		BytesMove(*scriptDesc.dataHandle, *fScriptData, size);
		short refnum = fScriptFile->fRsrcRefNum;
		UseResFile(refnum);
		FailResError();
		ChangedResource(fScriptData);
		FailResError();
		WriteResource(fScriptData);
		FailResError();
		UpdateResFile(refnum);
		FailResError();
		UseResFile(gApplicationRefNum);
		scriptDesc.dataHandle = DisposeIfHandle(scriptDesc.dataHandle);
		fi.Success();
	}
	else // fail
	{
		scriptDesc.dataHandle = DisposeIfHandle(scriptDesc.dataHandle);
		fi.ReSignal();
	}
}
		
//=============================================================================
#undef Inherited
#define Inherited TCommand
DefineClass(TSendEMailCommand, TCommand)

const long kEudoraSignature = 'CSOm';

struct KeyInfo
{
	long charCode;
	long modifiers;
};

TSendEMailCommand::TSendEMailCommand()
{
	fSubject = "";
	fTypeH = nil;
	fRecipientListH = nil;
	fBodyH = nil;
	fAppleScript = nil;
	fParamListDesc.dataHandle = nil;
	fParamListDesc.descriptorType = typeNull;
}

void TSendEMailCommand::ISendEMailCommand(const CStr255 &subject, Handle recipientsH, Handle bodyH)
{
	Inherited::ICommand(cMailToAuthors, nil, false, false, nil);
	FailInfo fi;
	Try(fi)
	{
		fSubject = subject;
		if (recipientsH)
			fRecipientListH = CloneHandle(recipientsH);
		if (bodyH)
			fBodyH = CloneHandle(bodyH);
		gBusyCursor->ForceBusy();
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(this);
		fi.ReSignal();
	}
}

void TSendEMailCommand::Free()
{
	FreeIfObject(fAppleScript); fAppleScript = nil;
	AEDisposeDesc(&fParamListDesc);
	fTypeH = DisposeIfHandle(fTypeH);
	fBodyH = DisposeIfHandle(fBodyH);
	fRecipientListH = DisposeIfHandle(fRecipientListH);
	Inherited::Free();
}

void WaitLong()
{
	short i = 0;
	while (i <= 2)
	{
		gApplication->PollToolboxEvent(false);
		EventRecord event;
		if (EventAvail(activMask | updateMask | osMask, &event))
			i = 0;
		else
			i++;
	}
}

void WaitABit()
{
	short i = 0;
	EventRecord event;
	while (OSEventAvail(everyEvent, &event))
	{
		WaitNextEvent(0, &event, 1, nil);
		i++;
		if (i > 15)
			Failure(minErr, 0);
	}
}

void PostKey(char key, Boolean commandKey)
{
	KeyInfo *const ki = (KeyInfo*)0x174;
	ki->charCode = 0;
	ki->modifiers = commandKey ? 0x8000 : 0;
	long msg = key;
	if (key == chEscape)
		msg |= kEscapeVirtualCode << 8;
	PostEvent(3, msg);
	ki->charCode = 0;
	ki->modifiers = 0;
}

void TSendEMailCommand::DoIt()
{
	FailInfo fi;
	Try(fi)
	{
// test for OS
		if (LoadAppleScript() == noErr)
			ExecuteAppleScript();
		else
		{
			if (gConfiguration.hasAUX)
				FailOSErr(errCannotUseAUX);
			if (!gPrefs->PrefExists('EuAp'))
				FailOSErr(errNoMailer);
			PutBodyIntoScrap();
			CreateTypeText();
			LaunchEudora();
			ExecuteHack();
		}
		fi.Success();
	}
	else // fail
	{
		FailNewMessage(fi.error, fi.message, messageReplyWithLetter);
	}
}

void TSendEMailCommand::LaunchEudora()
{
	FailInfo fi;
	Try(fi)
	{
		FSSpec spec;
		ProcessSerialNumber eudoraPSN, myPSN;
		FailOSErr(GetCurrentProcess(&myPSN));
		if (FindProcessBySignature(kEudoraSignature, eudoraPSN, &spec) != noErr)
		{
			FSSpec euSpec;
			gPrefs->GetAliasPrefs('EuAp', euSpec);
			if (gPrefs->GetBooleanPrefs('EuOp'))
			{
				if (!gPrefs->PrefExists('Eudo'))
					FailOSErr(errNoMailerDoc);
				FSSpec spec;
				gPrefs->GetAliasPrefs('Eudo', spec);
				AliasHandle alias;
				FailOSErr(NewAliasMinimal(&spec, &alias));
				OpenApplicationDocument(euSpec, alias, true);
			}
			else
				LaunchApplication(euSpec, true);
		}
	
		unsigned long startTick = TickCount();
		while (true)
		{
			Boolean timeout = TickCount() - startTick > 10 * 60;
			WaitLong();	
			if (FindProcessBySignature(kEudoraSignature, eudoraPSN, &spec) != noErr)
				if (timeout)
					FailOSErr(errLaunchEudoraTimeout);
				else
					continue;
			if (SetFrontProcess(&eudoraPSN) != noErr)
				if (timeout)
					FailOSErr(errBringEudoraToFrontTimeout);
				else
					continue;
			if (!ApplicationIsInfront(eudoraPSN))
				if (timeout)
					FailOSErr(errBringEudoraToFrontTimeout);
				else
					continue;
			break;
		}
		fi.Success();
	}
	else // fail
	{
		FailNewMessage(fi.error, fi.message, msgLaunchEudora);
	}
}

Boolean TSendEMailCommand::ApplicationIsInfront(ProcessSerialNumber &appPSN)
{
	ProcessSerialNumber frontPSN;
	FailOSErr(GetFrontProcess(&frontPSN));
	Boolean isSame;
	FailOSErr(SameProcess(&appPSN, &frontPSN, &isSame));
	return isSame;
}

void TSendEMailCommand::PutBodyIntoScrap()
{
	ZeroScrap();
	if (fBodyH)
	{
		HLock(fBodyH);
		PutScrap(GetHandleSize(fBodyH), 'TEXT', *fBodyH);
		HUnlock(fBodyH);
	}
}

void TSendEMailCommand::CreateTypeText()
{
	fTypeH = NewPermHandle(0);
	if (fRecipientListH)
	{
		HLock(fRecipientListH);
		FailOSErr(HandAndHand(fRecipientListH, fTypeH));
		HUnlock(fRecipientListH);
	}
	AppendStringToHandle("\x9", fTypeH);
	CStr255 s;
	MyGetIndString(s, kMailPreSubject);
	AppendStringToHandle(s, fTypeH);
	AppendStringToHandle(fSubject, fTypeH);
	MyGetIndString(s, kMailPostSubject);
	AppendStringToHandle(s, fTypeH);
	AppendStringToHandle("\x9\x9\x9", fTypeH);
}

void TSendEMailCommand::ExecuteHack()
{
// Abort Ask-Password dialog, if any
	PostKey('.', true);
	WaitLong();

// Make new letter
	PostKey('N', true);
	WaitLong();

// Make headers
	HLock(fTypeH);
	Ptr p = *fTypeH;
	long len = GetHandleSize(fTypeH);
	short numSaved = 0;
	for (long i = 1; i <= len; i++)
	{
		PostKey(*p++, false);
		++numSaved;
		if (numSaved > 10)
		{
			WaitABit();
			numSaved = 0;
		}
	}

// Body
	PostKey('V', true);
}

//========================= AppleScript ===================
void TSendEMailCommand::ExecuteAppleScript()
{
	Boolean prevDisabled = gNewsAppl->SetDisableBusyCursorReset(true);
	VOLATILE(prevDisabled);
	FailInfo fi;
	Try(fi)
	{
		DoExecuteAppleScript();
		gNewsAppl->SetDisableBusyCursorReset(prevDisabled);
		fi.Success();
	}
	else // fail
	{
		gNewsAppl->SetDisableBusyCursorReset(prevDisabled);
		fi.ReSignal();
	}
}

void TSendEMailCommand::DoExecuteAppleScript()
{
	CStr255 appleScriptName;
	MyGetIndString(appleScriptName, kMailScriptName);
	TStandardAppleScript *as = new TStandardAppleScript;
	as->IStandardAppleScript(appleScriptName);
	fAppleScript = as;

	AppleEvent theEvent;
	FailOSErr(AECreateAppleEvent(kASAppleScriptSuite, kASSubroutineEvent, &gNuntiusAddressSpec,
					kAutoGenerateReturnID, kAnyTransactionID, &theEvent));

// The parameter list.
	FailOSErr(AECreateList(nil, 0, false, &fParamListDesc));

	// Param1: toText
	if (fRecipientListH)
	{
		HLock(fRecipientListH);
		FailOSErr(AEPutPtr(&fParamListDesc, 0, typeChar, *fRecipientListH, GetHandleSize(fRecipientListH)));
		HUnlock(fRecipientListH);
	}
	else
		FailOSErr(AEPutPtr(&fParamListDesc, 0, typeChar, "", 0));

	// Param2:  subjectText
	FailOSErr(AEPutPtr(&fParamListDesc, 0, typeChar, &fSubject[1], fSubject.Length()));

	// Param3:  bodyText
	if (fBodyH)
	{
		HLock(fBodyH);
		FailOSErr(AEPutPtr(&fParamListDesc, 0, typeChar, *fBodyH, GetHandleSize(fBodyH)));
		HUnlock(fBodyH);
	}
	else
		FailOSErr(AEPutPtr(&fParamListDesc, 0, typeChar, "", 0));
	//
	FailOSErr(AEPutParamDesc(&theEvent, keyDirectObject, &fParamListDesc));

// The event name
	CStr255 handlerName;
	MyGetIndString(handlerName, kMailHandlerName);
	TranslateCStr255(handlerName, gToLowerChar);
	// the name must be in all-lowercase
	FailOSErr(AEPutParamPtr(&theEvent, keyASSubroutineName, typeChar, &handlerName[1], handlerName.Length()));

// Execute
	OSAID resultingScriptValueID = kOSANullScript;
	gBusyCursor->ForceBusy();
	OSAError oerr = ::OSAExecuteEvent(gOSAComponent, &theEvent, fAppleScript->GetScriptID(), kOSAModeNull, &resultingScriptValueID);
	if (resultingScriptValueID != kOSANullScript)
		OSADispose(gOSAComponent, resultingScriptValueID);
	if (oerr == errAEEventNotHandled)
	{
		MyGetIndString(handlerName, kMailHandlerName); // get proper case
		ParamText(appleScriptName, handlerName, gEmptyString, gEmptyString);
		StdAlert(phMissingHandlerInScript);
		Failure(0, 0);
	}
	if (oerr != noErr)
		FailOSErr(oerr);
}
//=============================================================================
#undef Inherited
#define Inherited TCommand
DefineClass(TMailToAuthorCommand, TCommand)

TMailToAuthorCommand::TMailToAuthorCommand()
{
	fQuoteH = nil;
	fRecipientListH = nil;
	fDoc = nil;
	fArticleListView = nil;
	fArticleIDList = nil;
	fArticle = nil;
}

void TMailToAuthorCommand::IMailToAuthorCommand(TGroupDoc *doc, TArticleListView *articleListView, 
				const CStr255 &subject, Boolean asQuote, Boolean addHeader)
{
	Inherited::ICommand(cMailToAuthors, nil, false, false, nil);
	FailInfo fi;
	Try(fi)
	{
		fDoc = doc;
		fArticleListView = articleListView;
		fAsQuote = asQuote;
		fAddHeader = addHeader;
		fSubject = subject;
		gBusyCursor->ForceBusy();
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(this);
		fi.ReSignal();
	}
}

void TMailToAuthorCommand::Free()
{
	gArticleCache->ReturnArticle(fArticle); fArticle = nil;
	fQuoteH = DisposeIfHandle(fQuoteH);
	FreeIfObject(fArticleIDList); fArticleIDList = nil;
	fRecipientListH = DisposeIfHandle(fRecipientListH);
	Inherited::Free();
}

void TMailToAuthorCommand::DoIt()
{
	TMailToAuthorCommand *mtac = nil;
	VOLATILE(mtac);
	FailInfo fi;
	Try(fi)
	{
		CreateRecipientList();
		fQuoteH = fArticleListView->GetSelectionAsQuotedTextForMail();
		TSendEMailCommand *cmd = new TSendEMailCommand;
		cmd->ISendEMailCommand(fSubject, fRecipientListH, fQuoteH);
		gApplication->PostCommand(cmd);
		fi.Success();
	}
	else // fail
	{
		FailNewMessage(fi.error, fi.message, messageReplyWithLetter);
	}
}

void TMailToAuthorCommand::CreateRecipientList()
{
	fRecipientListH = NewPermHandle(0);
	fArticleIDList = fArticleListView->GetListOfSelectedArticles();
	CLongintIterator iter(fArticleIDList);
	Boolean gotOne = false;
	for (long id = iter.FirstLong(); iter.More(); id = iter.NextLong())
	{
		gArticleCache->ReturnArticle(fArticle); fArticle = nil;
		FailInfo fi;
		Try(fi)
		{
			CStr255 groupDotName = fDoc->GetGroupDotName();
			fArticle = gArticleCache->GetArticle(groupDotName, id);
			fi.Success();
		}
		else // fail
		{
			if (fi.error == errNoSuchArticle)
				continue;
			fi.ReSignal();
		}
		CStr255 name, email, text;
		if (!fArticle->GetHeader("Reply-to", text))
			if (!fArticle->GetHeader("From", text))
				continue;
		GetAuthorName(text, name, email);
		if (gotOne)
			email.Insert(", ", 1);
		if (name.Length())
		{
			email += " (";
			email += name;
			email += ")";
		}
		AppendStringToHandle(email, fRecipientListH);
		gotOne = true;
	}
}


//=============================================================================
#undef Inherited
#define Inherited TCommand
DefineClass(TOpenURLCommand, TCommand)

TOpenURLCommand::TOpenURLCommand()
{
	fAppleScript = nil;
	fURLH = nil;
	fURL = "";
	fScheme = "";
	fParamListDesc.dataHandle = nil;
	fParamListDesc.descriptorType = typeNull;
}

void TOpenURLCommand::IOpenURLCommand(const CStr255 &url)
{
	Inherited::ICommand(cGenericCommand, nil, false, false, nil);
	fURL = url;
	gBusyCursor->ForceBusy();
}

void TOpenURLCommand::IOpenURLCommand(Handle urlH)
{
	Inherited::ICommand(cGenericCommand, nil, false, false, nil);
	gBusyCursor->ForceBusy();
	FailInfo fi;
	Try(fi)
	{
		FailNIL(urlH);
		fURLH = CloneHandle(urlH);
		long size = GetHandleSize(fURLH);
		long len = Min(250, size);
		fURL.Length() = len;
		if (len)
			BytesMove(*urlH, &fURL[1], len);
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(this);
		fi.ReSignal();
	}
}

void TOpenURLCommand::Free()
{
	FreeIfObject(fAppleScript); fAppleScript = nil;
	AEDisposeDesc(&fParamListDesc);
	fURLH = DisposeIfHandle(fURLH);
	Inherited::Free();
}

void TOpenURLCommand::ExtractScheme()
{
	fScheme = fURL;
	TranslateCStr255(fScheme, gToLowerChar);
	fScheme.Length() = Min(10, fScheme.Length());
	if (fScheme == "/info-mac/")
		fScheme = "info-mac";
	else
	{
		short pos = fURL.Pos(":");
		if (pos) 
		{
			fScheme = fURL;
			fScheme.Length() = pos - 1;
			TranslateCStr255(fScheme, gToLowerChar);
		}
		else
			fScheme = "";
	}
}

void TOpenURLCommand::ExecuteScript()
{
	CStr255 appleScriptName;
	MyGetIndString(appleScriptName, kURLScriptName);
	TStandardAppleScript *as = new TStandardAppleScript;
	as->IStandardAppleScript(appleScriptName);
	fAppleScript = as;

	AppleEvent theEvent;
	FailOSErr(AECreateAppleEvent(kASAppleScriptSuite, kASSubroutineEvent, &gNuntiusAddressSpec,
					kAutoGenerateReturnID, kAnyTransactionID, &theEvent));

// The parameter list.
	FailOSErr(AECreateList(nil, 0, false, &fParamListDesc));

	// Param1: the URL
	FailOSErr(AEPutPtr(&fParamListDesc, 0, typeChar, &fScheme[1], fScheme.Length()));
	if (fURLH) 
	{
		HLock(fURLH);
#if qDebug & 0
		long size = GetHandleSize(fURLH);
		for (long i = 0; i < size; i++) {
			char ch = (*fURLH)[i];
			fprintf(stderr, "%2ld: %ld = '%c'\n", i, long(ch), ch);
		}
#endif
		FailOSErr(AEPutPtr(&fParamListDesc, 0, typeChar, *fURLH, GetHandleSize(fURLH)));
	}
	else
	{
		CStr255 s(fURL);
		for (short i = 1; i <= s.Length() - 2; i++)
		{
			if (s[i] == '%' && s[i + 1] == '2' && s[i + 3] == '0')
			{
				s[i] = ' ';
				s.Delete(i +1 , 2);
			}
		}
		FailOSErr(AEPutPtr(&fParamListDesc, 0, typeChar, &fURL[1], fURL.Length()));
	}
	//
	FailOSErr(AEPutParamDesc(&theEvent, keyDirectObject, &fParamListDesc));

// The event name
	CStr255 handlerName;
	MyGetIndString(handlerName, kURLHandlerName);
	TranslateCStr255(handlerName, gToLowerChar);
	// the name must be in all-lowercase
	FailOSErr(AEPutParamPtr(&theEvent, keyASSubroutineName, typeChar, &handlerName[1], handlerName.Length()));

// Execute
	OSAID resultingScriptValueID = kOSANullScript;
	OSAError oerr = ::OSAExecuteEvent(gOSAComponent, &theEvent, fAppleScript->GetScriptID(), kOSAModeNull, &resultingScriptValueID);
	if (resultingScriptValueID != kOSANullScript)
		OSADispose(gOSAComponent, resultingScriptValueID);
	if (oerr == errAEEventNotHandled)
	{
		MyGetIndString(handlerName, kURLHandlerName); // get proper case
		ParamText(appleScriptName, handlerName, gEmptyString, gEmptyString);
		StdAlert(phMissingHandlerInScript);
		Failure(0, 0);
	}
	if (oerr != noErr)
		FailOSErr(oerr);
}

const char *gHexArray = "0123456789ABCDEF";
void TOpenURLCommand::HandleMailURL()
{
	Handle toH = nil;
	VOLATILE(toH);
	FailInfo fi;
	Try(fi)
	{
		CStr255 s(fURL);
		s.Delete(1, fScheme.Length() + 1);
		if (s.Length() > 2 && s[1] == '/' && s[2] == '/')
			s.Delete(1, 2);
		for (short i = 1; i <= s.Length() - 2; i++)
		{
			if (s[i] != '%')
				continue;
			const char *p1 = strchr(gHexArray, gToLowerChar[s[i+1]]);
			const char *p2 = strchr(gHexArray, gToLowerChar[s[i+2]]);
			if (!p1 || !p2)
				continue;
			s.Delete(i+1, 2);
			s[i] = (p1 - gHexArray) * 16 + p2 - gHexArray;
		}
		toH = NewPermHandle(s.Length());
		if (s.Length())
			BytesMove(&s[1], *toH, s.Length());
		TSendEMailCommand *cmd = new TSendEMailCommand;
		cmd->ISendEMailCommand(gEmptyString, toH, nil);
		gApplication->PostCommand(cmd);
		toH = DisposeIfHandle(toH);
		fi.Success();
	}
	else // fail
	{
		toH = DisposeIfHandle(toH);
		fi.ReSignal();
	}
}

void TOpenURLCommand::DoIt()
{
	Boolean prevDisabled = gNewsAppl->SetDisableBusyCursorReset(true);
	VOLATILE(prevDisabled);
	FailInfo fi;
	Try(fi)
	{
		ExtractScheme();
		if (fScheme == "mailto")
			HandleMailURL();
		else
			ExecuteScript();
		gNewsAppl->SetDisableBusyCursorReset(prevDisabled);
		fi.Success();
	}
	else // fail
	{
		gNewsAppl->SetDisableBusyCursorReset(prevDisabled);
		FailNewMessage(fi.error, fi.message, messageOpenURL);
	}
}

void ShowNuntiusHomeSite()
{
	CStr255 s;
	MyGetIndString(s, kNuntiusHomeSiteURL);
	TOpenURLCommand *cmd = new TOpenURLCommand();
	cmd->IOpenURLCommand(s);
	gApplication->PostCommand(cmd);
}

void OpenNuntiusWebPage()
{
	CStr255 s;
	MyGetIndString(s, kNuntiusWebPage);
	TOpenURLCommand *cmd = new TOpenURLCommand();
	cmd->IOpenURLCommand(s);
	gApplication->PostCommand(cmd);
}