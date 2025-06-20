// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UPassword.cp

#include "UPassword.h"
#include "UPrefsDatabase.h"
#include "Tools.h"
#include "UNntp.h"
#include "ViewTools.h"

#include <RsrcGlobals.h>

#include <Packages.h>
#include <UDialog.h>

#pragma segment MyDialogs

#define qDebugPassword qDebug & 1

CStr255 gCurrentPassword;

void CryptPassword(CStr255 &password)
{
	CStr255 s("01"); // version number
	for (short i = 1; i <= password.Length(); i++)
	{
		s += char(97 + (Random() & 15));
		s += char(password[i] ^ 9);
		s += char(107 + (Random() & 15));
	}
#if qDebugPassword
	fprintf(stderr, "Password '%s'", (char*)password);
	fprintf(stderr, " crypted to '%s'\n", (char*)s);
#endif
	password = s;
}

Boolean DeCryptPassword(CStr255 &password)
{
	CStr255 s(password);
	password = "";
	if (s.Length() < 2)
		return false; // missing version number
	if (s[1] != '0' || s[2] != '1')
		return false; // unknown version
	for (short i = 4; i <= s.Length(); i += 3)
	{
		password += char(s[i] ^ 9);
	}
#if qDebugPassword
	fprintf(stderr, "Password '%s'", (char*)password);
	fprintf(stderr, " decrypted from '%s'\n", (char*)s);
#endif
	BlockSet(Ptr(&s), sizeof(s), 0);
	return true;
}

void GetUserNameAndPassword(CStr255 &username, CStr255 &password)
{
	password = "";
	gPrefs->GetStringPrefs('UNam', username);
	if (gCurrentPassword.Length())
		password = gCurrentPassword;

	if (gPrefs->PrefExists('Gate'))
		gPrefs->GetStringPrefs('Gate', password);

	if (!username.Length())
	{
		gPrefs->GetStringPrefs('@adr', username);
		short pos = username.Pos("@", 1);
		if (pos)
			username.Delete(pos, username.Length() - pos + 1);
	}
	if (DeCryptPassword(password))
		if (username.Length() && password.Length())
			return;

	TWindow *window = gViewServer->NewTemplateWindow(kAskPasswordView, nil);
	FailNIL(window);
// set all the control-states

	window->Center(true, true, window->IsModal() );
	gPrefs->GetSilentWindowPosPrefs('WPas', window);
	CStr255 s;
	
// Names
	TEditText *usernameET = (TEditText*)window->FindSubView(kUsernameView);
	TPasswordView *passwordView = (TPasswordView*)window->FindSubView(kPasswordView);

	usernameET->SetText(username, kRedraw);
	DeCryptPassword(password);
	passwordView->SetPassword(password);
	BlockSet(Ptr(&password), sizeof(password), 0);
#if qDebug
	if (!IsObject(passwordView))
		ProgramBreak("passwordView is not object");
	if (!IsObject(usernameET))
		ProgramBreak("usernameET is not object");
#endif

// show the thing
	if (!username.Length())
		window->SetWindowTarget(usernameET);
	window->Open();
	IDType dismisser = MyPoseModally(window);
	gPrefs->SetWindowPosPrefs('WPas', window);
	if (dismisser != 'ok  ')
	{
		window->CloseByUser();
		BlockSet(Ptr(&username), sizeof(username), 0);
		BlockSet(Ptr(&password), sizeof(password), 0);
		Failure(0, 0);
	}
	
	usernameET->GetText(username);
	gPrefs->SetStringPrefs('UNam', username);
	passwordView->GetPassword(password);
	gCurrentPassword = password;
	CryptPassword(gCurrentPassword);
	TCheckBox *rememberCB = (TCheckBox*) window->FindSubView('Husk');
	if (rememberCB->IsOn())
		gPrefs->SetStringPrefs('Gate', gCurrentPassword);
	else if (gPrefs->PrefExists('Gate'))
	{
		CStr255 s(password);
		BlockSet(Ptr(&s[1]), 255, 0);
		gPrefs->SetStringPrefs('Gate', s); // clear memory
		gPrefs->DeletePrefs('Gate');
	}
	window->CloseByUser();
}

void InvalidateCurrentPassword()
{
	long numActive = gNntpCache->GetNoActiveConnections();
	if (numActive)
	{
		CStr255 s;
		NumToString(numActive, s);
		ParamText(gEmptyString, gEmptyString, gEmptyString, s);
		StdAlert(phUnableToForgetAllPasswords);
		return;
	}
	gNntpCache->FlushCache();
	ForgetCurrentPassword();
}

void ForgetCurrentPassword()
{
	BlockSet(Ptr(&gCurrentPassword[1]), 255, 0);
	gPrefs->SetStringPrefs('Gate', gCurrentPassword); // clear memory
	gPrefs->DeletePrefs('Gate');
	BlockSet(Ptr(&gCurrentPassword), sizeof(gCurrentPassword), 0);
	gCurrentPassword.Length() = 0;
}

Boolean HasPassword()
{
	return gCurrentPassword.Length() > 0 || gPrefs->PrefExists('Gate');
}

void InitUPassword()
{
	gCurrentPassword = "";
}

//----------------------------------------------------------------

pascal void TPasswordView::Initialize()
{
	inherited::Initialize();
	BlockSet(Ptr(&fPassword), sizeof(fPassword), 0);
}

pascal void TPasswordView::DoPostCreate(TDocument* itsDocument)
{
	inherited::DoPostCreate(itsDocument);
	TEFeatureFlag(teFOutlineHilite, TEBitClear, fHTE);
}

pascal void TPasswordView::Free()
{
	BlockSet(Ptr(&fPassword), sizeof(fPassword), 0);
	inherited::Free();
}

void TPasswordView::SetPassword(const CStr255 &password)
{
	CStr255 dot;
	MyGetIndString(dot, kPasswordDot);
	dot += '�'; // at least one char
	dot.Length() = 1;
	CStr255 s(gEmptyString);
	for (short i = 1; i <= password.Length(); i++)
		s += dot;
	fPassword = password;
	SetText(s);
}

void TPasswordView::GetPassword(CStr255 &password)
{
	password = fPassword;
}

pascal void TPasswordView::DoKeyEvent(TToolboxEvent* event)
{
	unsigned char ch = event->fCharacter;
	if (ch == chBackspace)
	{
		if ((**fHTE).selStart == (**fHTE).selEnd)
		{
			fPassword.Length()--;
			inherited::DoKeyEvent(event);
		}
		else
		{
			fPassword.Length() = 0;
			SetText(gEmptyString);
		}
	}
	else if (ch >= chSpace)
	{
		CStr255 dot;
		MyGetIndString(dot, kPasswordDot);
		dot += '�'; // at least one char
		fPassword += event->fCharacter;
		event->fCharacter = dot[1];
		inherited::DoKeyEvent(event);
	}
	else if (ch == chTab || ch == chEscape || ch == chReturn || ch == chEnter)
		TEventHandler::DoKeyEvent(event); // bypass TTEView
	else
		return; // ignore arrows etc.
}

pascal void TPasswordView::DoMouseCommand(VPoint& /* theMouse */, TToolboxEvent* /* event */, CPoint /* hysteresis */)
{
	SetSelection(0, 1000, kRedraw);
}

pascal void TPasswordView::DoSetupMenus()
{
	inherited::DoSetupMenus();
	Enable(cCopy, false);
	Enable(cPaste, false);
	Enable(cCut, false);
}
