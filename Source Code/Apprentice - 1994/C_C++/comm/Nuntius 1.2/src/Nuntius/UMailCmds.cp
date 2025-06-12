// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UMailCmds.cp

#include "UMailCmds.h"
#include "Tools.h"
#include "XTypes.h"
#include "UArticleListView.h"
#include "UArticle.h"
#include "UArticleCache.h"
#include "UGroupDoc.h"
#include "ProcessTools.h"
#include "NetAsciiTools.h"
#include "UPrefsDatabase.h"

#include <RsrcGlobals.h>
#include <ErrorGlobals.h>

#include <OSEvents.h>

#pragma segment MyArticle

//StdAlert(phNoMailYet);
//GetListOfSelectedArticles


const long kEudoraSignature = 'CSOm';

struct KeyInfo
{
	long charCode;
	long modifiers;
};

TMailToAuthorCommand::TMailToAuthorCommand()
{
}

pascal void TMailToAuthorCommand::Initialize()
{
	inherited::Initialize();
	fQuoteH = nil;
	fTypeH = nil;
	fDoc = nil;
	fArticleListView = nil;
	fArticleIDList = nil;
	fArticle = nil;
}

void TMailToAuthorCommand::IMailToAuthorCommand(TGroupDoc *doc, TArticleListView *articleListView, Boolean asQuote, Boolean addHeader)
{
	inherited::ICommand(cMailToAuthors, nil, false, false, nil);
	fDoc = doc;
	fArticleListView = articleListView;
	fAsQuote = asQuote;
	fAddHeader = addHeader;
}

pascal void TMailToAuthorCommand::Free()
{
	gArticleCache->ReturnArticle(fArticle); fArticle = nil;
	fQuoteH = DisposeIfHandle(fQuoteH);
	FreeIfObject(fArticleIDList); fArticleIDList = nil;
	fTypeH = DisposeIfHandle(fTypeH);
	inherited::Free();
}

void WaitLong()
{
	short i = 0;
	while (i <= 2)
	{
		gApplication->PollToolboxEvent(false);
		EventRecord event;
		if (EventAvail(activMask | updateMask | osMask, event))
			i = 0;
		else
			i++;
	}
}

void WaitABit()
{
	short i = 0;
	EventRecord event;
	while (OSEventAvail(everyEvent, event))
	{
		WaitNextEvent(0, event, 1, nil);
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

pascal void TMailToAuthorCommand::DoIt()
{
	FailInfo fi;
	if (fi.Try())
	{
// test for OS
		if (gConfiguration.hasAUX)
			FailOSErr(errCannotUseAUX);
		if (!gPrefs->PrefExists('EuAp'))
			FailOSErr(errNoMailer);
		PutQuoteIntoScrap();
		CreateTypeText();
		LaunchEudora();
		ExecuteHack();
		fi.Success();
	}
	else // fail
	{
		FailNewMessage(fi.error, fi.message, messageReplyWithLetter);
	}
}

void TMailToAuthorCommand::LaunchEudora()
{
	FailInfo fi;
	if (fi.Try())
	{
		FSSpec spec;
		ProcessSerialNumber eudoraPSN, myPSN;
		FailOSErr(GetCurrentProcess(myPSN));
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
				FailOSErr(NewAliasMinimal(spec, alias));
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
			if (SetFrontProcess(eudoraPSN) != noErr)
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

Boolean TMailToAuthorCommand::ApplicationIsInfront(ProcessSerialNumber &appPSN)
{
	ProcessSerialNumber frontPSN;
	FailOSErr(GetFrontProcess(frontPSN));
	Boolean isSame;
	FailOSErr(SameProcess(appPSN, frontPSN, isSame));
	return isSame;
}

void TMailToAuthorCommand::PutQuoteIntoScrap()
{
//	fQuoteH = fArticleListView->GetSelectionAsTextForClipboard(fAsQuote, fAddHeader, true);
	fQuoteH = fArticleListView->GetSelectionAsQuotedTextForMail();
	HLock(fQuoteH);
	ZeroScrap();
	PutScrap(GetHandleSize(fQuoteH), 'TEXT', *fQuoteH);
	fQuoteH = DisposeIfHandle(fQuoteH);
}

void TMailToAuthorCommand::CreateTypeText()
{
	fTypeH = NewPermHandle(0);
	fArticleIDList = fArticleListView->GetListOfSelectedArticles();
	{
		CLongintIterator iter(fArticleIDList);
		Boolean gotOne = false;
		for (long id = iter.FirstLong(); iter.More(); id = iter.NextLong())
		{
			gArticleCache->ReturnArticle(fArticle); fArticle = nil;
			FailInfo fi;
			if (fi.Try())
			{
				CStr255 groupDotName;
				fDoc->GetGroupDotName(groupDotName);
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
			AppendStringToHandle(email, fTypeH);
			gotOne = true;
		}
		gArticleCache->ReturnArticle(fArticle); fArticle = nil;
	}
	AppendStringToHandle("\x9", fTypeH);
	{
		CLongintIterator iter(fArticleIDList);
		for (long id = iter.FirstLong(); iter.More(); id = iter.NextLong())
		{
			if (id < fDoc->GetFirstArticleID() || id > fDoc->GetLastArticleID())
				continue;
			CStr255 subject, pre, post;
			HandleOffsetLength hol;
			if (!fDoc->GetSubject(id, hol))
				continue;
			CopyHolToCStr255(hol, subject);
			MyGetIndString(pre, kMailPreSubject);
			MyGetIndString(post, kMailPostSubject);
			AppendStringToHandle(pre, fTypeH);
			AppendStringToHandle(subject, fTypeH);
			AppendStringToHandle(post, fTypeH);
			break;
		}
	}
	AppendStringToHandle("\x9\x9\x9", fTypeH);
}

void TMailToAuthorCommand::ExecuteHack()
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
