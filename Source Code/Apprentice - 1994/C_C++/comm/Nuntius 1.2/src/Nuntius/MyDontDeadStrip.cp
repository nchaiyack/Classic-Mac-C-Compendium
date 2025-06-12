// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// MyDontDeadStrip.cp

#include "MyDontDeadStrip.h"

// for macroDontDeadStrip:
#include <UGroupTreeView.h>
#include <UGroupListView.h>
#include <UDiscListView.h>
#include <UArticleListView.h>
#include <UArticleView.h>
#include <UNavigatorView.h>
#include <UTextScroller.h>
#include <UPreferencesView.h>
#include <UFolderSelectView.h>
#include <UPostArticleView.h>
#include <UStructureInspectors.h>
#include <UTriangleControl.h>
#include <UPassword.h>

#pragma segment MANever

void DoTheMyDontDeadStrip()
{
	macroDontDeadStrip(TGroupTreeView);
	macroDontDeadStrip(TGroupListView);
	macroDontDeadStrip(TDiscListView);
	macroDontDeadStrip(TArticleView);
	macroDontDeadStrip(TArticleListView);
	macroDontDeadStrip(TNavigatorView);
	macroDontDeadStrip(TNavigatorArrowView);
	macroDontDeadStrip(TTextScroller);
	macroDontDeadStrip(TFolderSelectView);
	macroDontDeadStrip(TFileSelectView);
	macroDontDeadStrip(TEditArticleToPostDialogView);
	macroDontDeadStrip(TPostArticleDialogView);
	macroDontDeadStrip(TPasswordView);
//
	macroDontDeadStrip(TIdler);
	macroDontDeadStrip(TObjectListView);
	macroDontDeadStrip(TViewHierarchyView);
	macroDontDeadStrip(TTargetChainView);
	macroDontDeadStrip(TTriangleControl);
}

#pragma segment Main

void MyDontDeadStrip()
{
	if (gDeadStripSuppression)
		DoTheMyDontDeadStrip();
}
