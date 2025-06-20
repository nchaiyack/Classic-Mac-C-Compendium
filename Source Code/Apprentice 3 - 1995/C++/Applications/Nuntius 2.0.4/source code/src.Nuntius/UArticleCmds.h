// Copyright � 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticleCmds.h

#define __UARTICLECMDS__

#ifndef __UARTICLESTATUS__
#include "UArticleStatus.h"
#endif

class TArticleView;
class TArticleListView;
//---------------------------------------------------------------------

class TShowDiscJunkCommand : public TCommand
{
	DeclareClass(TShowDiscJunkCommand);
	public:
		void DoIt();

		TShowDiscJunkCommand();
		void IShowDiscJunkCommand(TArticleListView *articleListView, long index);
		void Free();
	private:
		TArticleListView *fArticleListView; 
		long fIndex;
};
//---------------------------------------------------------------------

class TUpdateArticleViewCommand : public TCommand
{
	DeclareClass(TUpdateArticleViewCommand);
	public:
		void DoIt();
		
		TUpdateArticleViewCommand();
		void IUpdateArticleViewCommand(TArticleListView *alv, ArticleShowType whatToShow);
		void Free();
	private:
		TArticleListView *fArticleListView;
		ArticleShowType fWhatToShow;
};

//---------------------------------------------------------------------

class TStoreNotesCommand : public TCommand
{
	DeclareClass(TStoreNotesCommand);
	public:
		void DoIt();
		
		TStoreNotesCommand();
		void IStoreNotesCommand(TArticleListView *alv);
		void Free();

	private:
		TArticleListView *fArticleListView;
		TFile *fFile;
		ParamBlockRec *fPBP;
		
		void OpenFile();
		void StoreQuote();
};
