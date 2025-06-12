// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UPassword.h

#define __UPASSWORD__

#ifndef __UTEVIEW__
#include <UTEView.h>
#endif

class TPasswordView : public TTEView
{
	public:
		pascal void DoKeyEvent(TToolboxEvent* event);
		pascal void DoMouseCommand(VPoint& theMouse,
									   TToolboxEvent* event, CPoint hysteresis);
		pascal void DoSetupMenus();

		void SetPassword(const CStr255 &password);
		void GetPassword(CStr255 &password);

		
		pascal void Initialize();
		pascal void DoPostCreate(TDocument* itsDocument);
		pascal void Free();
	private:
		CStr255 fPassword;
};

void GetUserNameAndPassword(CStr255 &username, CStr255 &password);
	// asks if missing

void InvalidateCurrentPassword(); // no-op if any open connection
void ForgetCurrentPassword(); // clears stored password
Boolean HasPassword();
void InitUPassword();
