// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UPostArticleView.h

#define __UPOSTARTICLEVIEW__

#ifndef __UDIALOG__
#include <UDialog.h>
#endif

class PPostArticleInfo;

class TEditArticleToPostDialogView : public TDialogView
{
	public:
		virtual void SetInfo(PPostArticleInfo *info);

		virtual pascal void DoEvent(EventNumber eventNumber, 
							TEventHandler *source, TEvent *event);

		TEditArticleToPostDialogView();
		virtual pascal void Initialize();
		virtual pascal void ReadFields(TStream *aStream);
		virtual pascal void DoPostCreate(TDocument *itsDocument);
		virtual void pascal Free();
	private:
		TWindow *fWindow;
		TEditText *fSubjectET, *fGroupsET, *fDistributionET;
		TCheckBox *fAddSignatureCB;
		PPostArticleInfo *fPostArticleInfo;

		void DoEditKey();
		void CheckHeader(TEditText *fET, const CStr255 &text, long emptyMessage);
};


class TPostArticleDialogView : public TDialogView
{
	public:
		virtual void SetInfo(PPostArticleInfo *infoH);

		virtual pascal void DoEvent(EventNumber eventNumber, 
							TEventHandler *source, TEvent *event);

		TPostArticleDialogView();
		virtual pascal void Initialize();
		virtual pascal void ReadFields(TStream *aStream);
		virtual void pascal Free();
	private:
		TEditText *fSubjectET;
		TStaticText *fSubjectST;
		TCheckBox *fAddSignatureCB;
		TCheckBox *fMoveToTrashCB;
		PPostArticleInfo *fPostArticleInfo;
};
