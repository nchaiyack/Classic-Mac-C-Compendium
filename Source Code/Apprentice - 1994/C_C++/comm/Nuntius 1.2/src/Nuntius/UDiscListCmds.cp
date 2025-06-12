// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UDiscListCmds.cp

#include "UDiscListCmds.h"
#include "UArticleListView.h"
#include "UDiscListView.h"
#include "UGroupDoc.h"
#include "UPrefsDatabase.h"

#include <RsrcGlobals.h>

#include <UPrinting.h>

#pragma segment MyGroup


//-------------------------------------------------------------------
TOpenArticleCommand::TOpenArticleCommand()
{
}

void TOpenArticleCommand::IOpenArticleCommand(TGroupDoc *doc, 
														TLongintList *discIndexList, 
														Boolean removeSelection)
{
	inherited::ICommand(cOpenArticle, nil, false, false, nil);
	fDoc = doc;
	fDiscIndexList = discIndexList;
	fRemoveSelection = removeSelection;
}

pascal void TOpenArticleCommand::Free()
{
	inherited::Free();
}

pascal void TOpenArticleCommand::DoIt()
{
	TWindow *window;
	window = gViewServer->NewTemplateWindow(kArticleView, fDoc);
	CRect maxResize(window->fResizeLimits);
	maxResize.right = 650; //@@ hack
	window->SetResizeLimits(maxResize[topLeft], maxResize[botRight]);
	gPrefs->GetSilentWindowPosPrefs('WArt', window);
	TArticleListView *alView;
	alView = (TArticleListView*) window->FindSubView(kArticleListViewID);
	if (!alView) {
#if qDebug
		ProgramBreak("UPS: did not find TArticleListView!");
#endif
		FailOSErr(errMissingClass);
	}

	alView->SetArticleList(fDiscIndexList);

	TStdPrintHandler *aHandler = new TStdPrintHandler;
	aHandler->IStdPrintHandler(fDoc, alView, !kSquareDots, kFixedSize, !kFixedSize);

//	if (fRemoveSelection)
//		fDoc->GetDiscListView()->SetEmptySelection(kHighlight);
}

//-------------------------------------------------------------------
