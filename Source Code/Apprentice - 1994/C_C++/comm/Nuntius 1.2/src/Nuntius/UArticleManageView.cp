// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticleManageView.cp

#include "UArticleManageView.h"
#include "UArticleView.h"
#include "UArticleCache.h"
#include "UTriangleControl.h"
#include "UGroupDoc.h"
#include "UArticleStatus.h"
#include "UPrefsDatabase.h"
#include "NetAsciiTools.h"
#include "Tools.h"

#include <RsrcGlobals.h>
#include <ErrorGlobals.h>

#pragma segment MyArticle

#define qDebugFrame qDebug & 0

const VPoint kArticleViewLocation(17, 0);
const VCoordinate kManageViewWidth = 640;
const VCoordinate kMinHeight = 16;

pascal void TArticleManageView::Initialize()
{
	inherited::Initialize();
	fDoc = nil;
	fArticleView = nil;
	fTriControl = nil;
}

void TArticleManageView::IArticleManageView(TGroupDoc *doc, TView *superView, VCoordinate top,
							long articleID, TArticle *article, Boolean showExpanded)
{
	VPoint tl(0, top);
	inherited::IView(doc, superView, tl, VPoint(kManageViewWidth, 0), 
		sizeVariable, sizeVariable);
	FailInfo fi;
	if (fi.Try())
	{
		fDoc = doc;
		fArticleID = articleID;
		
		StandardGridViewTextStyle gvts;
		gPrefs->GetTextStylePrefs('TSar', gvts.fTextStyle);
		CalcStandardGridViewFont(gvts);
		fGridViewTextStyle = gvts;

		TTriangleControl *tri = new TTriangleControl();
		tri->ITriangleControl(this, VPoint(0, 0), showExpanded);
		fTriControl = tri;
		
#if qDebugFrame
		VRect oldFrame;
		GetFrame(oldFrame);
#endif
		BuildDisplay(showExpanded, article);
#if qDebugFrame
		VRect newFrame;
		GetFrame(newFrame);
		fprintf(stderr, "TAMV: Frame %s -> ", (char*)oldFrame);
		fprintf(stderr, "%s\n", (char*)newFrame);
#endif
		Focus();
		ForceRedraw();
		fi.Success();
	}
	else // fail
	{
		Free();
		fi.ReSignal();
	}
}

pascal void TArticleManageView::Free()
{
	inherited::Free();
}

pascal void TArticleManageView::DoEvent(EventNumber eventNumber, 
															TEventHandler* source, TEvent* event)
{
	if (eventNumber == mToggle)
		BuildDisplay(!fIsExpanded);
	inherited::DoEvent(eventNumber, source, event);
}

void TArticleManageView::BuildDisplay(Boolean expand, TArticle *article)
{
	VOLATILE(expand);
	FailInfo fi;
	if (fi.Try())
	{
		fTriControl->SetExpand(expand);
		fIsExpanded = expand;
		if (expand)
			DoShowArticleView(article);
		else
			DoShowHeaderView();
		fi.Success();
	}
	else // fail
	{
		fTriControl->SetExpand(false);
		fIsExpanded = false;
		DoShowHeaderView();
		FailNewMessage(fi.error, fi.message, messageExpandArticle);
	}
}

void TArticleManageView::DoShowArticleView(TArticle *article)
{
	if (fArticleView)
	{
#if qDebug
		if (article)
			fprintf(stderr, "Problem: in TAMV::DoSAV: got article view and new article\n");
#endif
		fArticleView->UpdateDisplay();
	}
	else
	{
		if (!article)
		{
			CStr255 groupDotName;
			fDoc->GetGroupDotName(groupDotName);
			FailInfo fi;
			if (fi.Try())
			{
				article = gArticleCache->GetArticle(groupDotName, fArticleID);
				fi.Success();
			}
			else // fail
			{
				fDoc->GetArticleStatus()->SetStatus(fArticleID, kArticleRead);
				fDoc->GetOldArticleStatus()->SetMinStatus(fArticleID, kArticleRead);
				fi.ReSignal();
			}
		}
		TArticleView *av = new TArticleView();
		av->IArticleView(fDoc, this, kArticleViewLocation, fArticleID, article);
		fArticleView = av;
		// SubViewChangedFrame takes care of resizing this view
	}
}

void TArticleManageView::DoShowHeaderView()
{
	if (fArticleView)
	{
		fArticleView->Close();
		fArticleView->Free();
		fArticleView = nil;
	}
	VPoint viewSize(kManageViewWidth, Max(16, fGridViewTextStyle.fRowHeight));
	VRect myNewFrame(fLocation, fLocation + viewSize);
	// don't invalidate released space, as TViewListView takes care of updating
	// so invalidate only changed contents
	VRect vr(kArticleViewLocation, kArticleViewLocation + viewSize);
	vr.right = fSize.h; // to get all updated
	Focus();
	InvalidateVRect(vr);
#if qDebugFrame
fprintf(stderr, "MyNewFrame = %s\n", (char*)myNewFrame);
#endif
	SetFrame(myNewFrame, !kRedraw);
}


pascal void TArticleManageView::Draw(const VRect& area)
{
	FailInfo fi;
	if (fi.Try())
	{
		inherited::Draw(area);
		fi.Success();
	}
	else // fail
	{
		if (fi.error == errNoSuchArticle)
		{
			fTriControl->SetExpand(false);
			fIsExpanded = false;
			DoShowHeaderView();
		}
		fi.ReSignal();
	}
	if (!fIsExpanded)
	{
		TextStyle itsTextStyle = fGridViewTextStyle.fTextStyle;
		SetPortTextStyle(itsTextStyle);
		MoveTo(20, fGridViewTextStyle.fVertOffset + short(fSize.v - fGridViewTextStyle.fRowHeight)/2);
		HandleOffsetLength hol;
		CStr255 text, from, email, realname, subject;
		if (!fDoc->GetSubject(fArticleID, hol))
		{
			MyGetIndString(text, kUnknownArticleText);
			DrawString(text);
			return;
		}
		CopyHolToCStr255(hol, subject);
		if (!fDoc->GetFrom(fArticleID, hol))
		{
			MyGetIndString(text, kUnknownArticleText);
			DrawString(text);
			return;
		}
		CopyHolToCStr255(hol, text);
		if (!GetPrintableAuthorName(text, realname, email))
			realname = email;
		MyGetIndString(text, kArticleCompactTemplate);
		SubstituteStringItems(text, "ÇrealnameÈ", realname);
		SubstituteStringItems(text, "ÇemailÈ", email);
		SubstituteStringItems(text, "ÇsubjectÈ", subject);
		DrawString(text);
	}
}

TArticleView *TArticleManageView::GetArticleView()
{
	return fArticleView;
}

pascal void TArticleManageView::SubViewChangedFrame(TView* theSubView,
							const VRect& oldFrame, const VRect& newFrame, Boolean invalidate)
{
	inherited::SubViewChangedFrame(theSubView, oldFrame, newFrame, invalidate);
	VRect myFrame(0, 0, kManageViewWidth, Max(kMinHeight, fGridViewTextStyle.fRowHeight));
	myFrame = myFrame | newFrame;
	myFrame.right = kManageViewWidth;
	myFrame += fLocation;
	SetFrame(myFrame, !kRedraw);
	// don't invalidate, as TViewListView takes care of updating
#if qDebugFrame
	fprintf(stderr, "AMV: SBCF -> new frame = %s\n", (char*)myFrame);
#endif
}

void TArticleManageView::SetNewFont(const TextStyle &textStyle)
{
	StandardGridViewTextStyle gvts;
	gvts.fTextStyle = textStyle;
	CalcStandardGridViewFont(gvts);
	fGridViewTextStyle = gvts;
	if (fArticleView)
		fArticleView->SetNewFont(textStyle);
	else
		BuildDisplay(fIsExpanded);
}
