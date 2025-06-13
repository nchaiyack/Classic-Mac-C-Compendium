// Copyright © 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// MyDontDeadStrip.cp

#include "MyDontDeadStrip.h"

// for macroDontDeadStrip:
#include "UGroupTreeView.h"
#include "UGroupListView.h"
#include "UDiscListView.h"
#include "UArticleListView.h"
#include "UArticleView.h"
#include "UArticleViewCache.h"
#include "UNavigatorView.h"
#include "UTextScroller.h"
#include "UPreferencesView.h"
#include "UFolderSelectView.h"
#include "UPostArticleView.h"
#include "UStructureInspectors.h"
#include "UTriangleControl.h"
#include "UPassword.h"

void DoTheMyDontDeadStrip();

//#pragma segment Main

void MyDontDeadStrip()
{	
	extern Boolean gDontDeadStrip;
	if (gDontDeadStrip)
		DoTheMyDontDeadStrip();
}
#pragma segment MANever

void DoTheMyDontDeadStrip()
{
	macroDontDeadStrip(TGroupTreeView);
	macroDontDeadStrip(TGroupListView);
	macroDontDeadStrip(TDiscListView);
	macroDontDeadStrip(TArticleView);
	macroDontDeadStrip(TArticleListView);
	macroDontDeadStrip(TArticleWindow);
	macroDontDeadStrip(TNavigatorView);
	macroDontDeadStrip(TNavigatorArrowView);
	macroDontDeadStrip(TTextScroller);
	macroDontDeadStrip(TFolderSelectView);
	macroDontDeadStrip(TFileSelectView);
	macroDontDeadStrip(TEditArticleToPostView);
	macroDontDeadStrip(TPostArticleView);
	macroDontDeadStrip(TPasswordView);
//
	macroDontDeadStrip(TIdler);
	macroDontDeadStrip(TObjectListView);
	macroDontDeadStrip(TViewHierarchyView);
	macroDontDeadStrip(TTargetChainView);
	macroDontDeadStrip(TTriangleControl);
}

