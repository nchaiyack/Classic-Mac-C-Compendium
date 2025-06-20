// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UPostArticleView.cp

#include "UPostArticleView.h"
#include "UPostArticleCmds.h"
#include "UPrefsDatabase.h"
#include "NetAsciiTools.h"
#include "UThread.h"

#include <ErrorGlobals.h>
#include <RsrcGlobals.h>

#pragma segment MyArticle

//==================================================================================================
TEditArticleToPostDialogView::TEditArticleToPostDialogView()
{
}


pascal void TEditArticleToPostDialogView::Initialize()
{
	inherited::Initialize();
	fWindow = nil;
	fSubjectET = fGroupsET = fDistributionET = nil;
	fAddSignatureCB = nil;
	fPostArticleInfo = nil;
}

pascal void TEditArticleToPostDialogView::ReadFields(TStream *aStream)
{
	inherited::ReadFields(aStream);
}

pascal void TEditArticleToPostDialogView::Free()
{
	fWindow = nil;
	fPostArticleInfo = nil;
	inherited::Free();
}

pascal void TEditArticleToPostDialogView::DoPostCreate(TDocument *itsDocument)
{
	inherited::DoPostCreate(itsDocument);
	fWindow = GetWindow();
	fSubjectET = (TEditText*) FindSubView('Subj');
	fGroupsET = (TEditText*) FindSubView('Grup');
	fDistributionET = (TEditText*) FindSubView('Dist');
	fAddSignatureCB = (TCheckBox*) FindSubView('AdSi');
#if qDebug
	if (!IsObject(fSubjectET))			ProgramBreak("Did not find subject ET");
	if (!IsObject(fGroupsET))				ProgramBreak("Did not find groups ET");
	if (!IsObject(fDistributionET))	ProgramBreak("Did not find distribution ET");
	if (!IsObject(fAddSignatureCB))	ProgramBreak("Did not find addSignature CB");
#endif
}

void TEditArticleToPostDialogView::SetInfo(PPostArticleInfo *info)
{
	fPostArticleInfo = info;
	fSubjectET->SetText(CStr255(fPostArticleInfo->fSubject), kRedraw);
	fSubjectET->SetSelection(32000, 32000, kRedraw);
	if (fPostArticleInfo->fNewsGroups == "net.general")
		fGroupsET->SetText("net.followup", kRedraw);// RFC 850, pg 8
	else
		fGroupsET->SetText(fPostArticleInfo->fNewsGroups, kRedraw);
	fDistributionET->SetText("world", kRedraw); //@@
	fAddSignatureCB->SetState(fPostArticleInfo->fAddSignatureChoice, kRedraw);
}

pascal void TEditArticleToPostDialogView::DoEvent(EventNumber eventNumber, 
								TEventHandler *source, TEvent *event)
{
	if (eventNumber == mCancelHit || eventNumber == mCancelKey)
	{
		fPostArticleInfo->Cancel();
		return;
	}
	if (source && source->fIdentifier == 'Edit')
	{
			DoEditKey();
			return;
	}
	inherited::DoEvent(eventNumber, source, event);
}

void TEditArticleToPostDialogView::DoEditKey()
{
	CStr255 s;
	fSubjectET->GetText(s);
	StripSpaces(s);
	CheckHeader(fSubjectET, s, messageEmptySubject);
	fPostArticleInfo->fSubject = s;

	fGroupsET->GetText(s);
	StripSpaces(s);
	CheckHeader(fGroupsET, s, messageEmptyGroups);
	fPostArticleInfo->fNewsGroups = s;

	fDistributionET->GetText(s);
	StripSpaces(s);
	CheckHeader(fDistributionET, s, messageEmptyDistribution);
	fPostArticleInfo->fDistribution = s;

	fPostArticleInfo->fAddSignatureChoice = fAddSignatureCB->IsOn();

	fPostArticleInfo->ShowPostArticleWindow();
	fPostArticleInfo->MakeFile();
	fPostArticleInfo->LaunchEditor();
}

void TEditArticleToPostDialogView::CheckHeader(
		TEditText *fET, const CStr255 &text, long emptyMessage)
{
	if (text == "")
		Failure(errBadHeader, emptyMessage);
	for (short i = 1; i <= text.Length(); i++)
	{
		char ch = text[i];
		if (ch < 32 || ch > 126)
		{
			fET->SetSelection(i - 1, i, kRedraw);
			Failure(errBadHeader, messageInvalidHeaderChar);
		}
	}
}

//------------------------------------------------------------------------------
TPostArticleDialogView::TPostArticleDialogView()
{
}


pascal void TPostArticleDialogView::Initialize()
{
	inherited::Initialize();
	fSubjectET = nil;
	fSubjectST = nil;
	fAddSignatureCB = nil;
	fMoveToTrashCB = nil;
	fPostArticleInfo = nil;
}

pascal void TPostArticleDialogView::ReadFields(TStream *aStream)
{
	inherited::ReadFields(aStream);
}

pascal void TPostArticleDialogView::Free()
{
	fPostArticleInfo = nil;
	inherited::Free();
}

void TPostArticleDialogView::SetInfo(PPostArticleInfo *info)
{
	fPostArticleInfo = info;

	fSubjectET = (TEditText*) FindSubView('Subj');
	fSubjectST = (TStaticText*) FindSubView('SUBS');

	Boolean editHeaders = fPostArticleInfo->fEditHeaders;
	fSubjectST->SetEnable(editHeaders);
	fSubjectET->Show(!editHeaders, kRedraw);
	fSubjectET->SetEnable(!editHeaders);
	fSubjectST->Show(editHeaders, kRedraw);
	if (editHeaders)
		fSubjectET->Locate(VPoint(1000, 1000), kRedraw);
	else
		fSubjectST->Locate(VPoint(1000, 1000), kRedraw);
	CStr255 subject(fPostArticleInfo->fSubject);
	if (editHeaders)
		fSubjectST->SetText(subject, kRedraw);
	else
		fSubjectET->SetText(subject, kRedraw);
	
	fAddSignatureCB = (TCheckBox*) FindSubView('AdSi');
	fAddSignatureCB->Show(!fPostArticleInfo->fEditSignature, kRedraw);	
	fAddSignatureCB->SetEnable(!fPostArticleInfo->fEditSignature);	
	fAddSignatureCB->SetState(fPostArticleInfo->fAddSignatureChoice, kRedraw);	

	fMoveToTrashCB = (TCheckBox*) FindSubView('PoTr');
	fMoveToTrashCB->SetState(fPostArticleInfo->fTrashDiskCopyAfterPost, kRedraw);	
}

pascal void TPostArticleDialogView::DoEvent(EventNumber eventNumber, 
								TEventHandler *source, TEvent *event)
{
	CStr255 s;
	if (eventNumber == mCancelHit || eventNumber == mCancelKey)
	{
		fPostArticleInfo->Cancel();
		return;
	}
	if (source && source->fIdentifier == 'Edit')
	{
			fPostArticleInfo->LaunchEditor();
			return;
	}
	if (source && source->fIdentifier == 'Post')
	{
		if (fPostArticleInfo->fEditHeaders)
			fSubjectST->GetText(s);
		else
			fSubjectET->GetText(s);
		if (!fPostArticleInfo->fEditSignature)
			fPostArticleInfo->fAddSignatureChoice = fAddSignatureCB->IsOn();
		fPostArticleInfo->fSubject = s;
		fPostArticleInfo->fTrashDiskCopyAfterPost = fMoveToTrashCB->IsOn();
		fPostArticleInfo->DoPostArticle();
		return;
	}
	inherited::DoEvent(eventNumber, source, event);
}
