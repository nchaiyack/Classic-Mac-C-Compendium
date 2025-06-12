// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UPreferencesView.h

#define __UPREFERENCESVIEW__

#ifndef __UDIALOG__
#include <UDialog.h>
#endif

void DoMiscPreferencesDialog();
void DoYourNamePreferencesDialog();
void DoNewsServerPreferencesDialog(); // creates it, and all that stuff...
void DoBinariesPreferencesDialog();
void DoEditorPreferencesDialog();
void DoMailerPreferencesDialog();

class TPrefRammeView : public TView
{
	public:
		virtual pascal void Draw(const VRect &area);

		TPrefRammeView();
		virtual pascal void Initialize();
		virtual pascal void ReadFields(TStream *aStream);
		virtual void pascal Free();
	protected:
		CStr255 fText;
		TextStyle fTextStyle;
		short fRammeVertOffset;
		short fTextHorzOffset;
		short fTextVertOffset;
};

class TMailerPrefDialogView : public TDialogView
{
	public:
		virtual pascal void DoEvent(EventNumber eventNumber, TEventHandler *source,
								TEvent *event);

		TMailerPrefDialogView();
};