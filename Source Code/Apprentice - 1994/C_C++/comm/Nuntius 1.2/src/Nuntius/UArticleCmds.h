// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
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
	public:
		pascal void DoIt();

		TShowDiscJunkCommand();
		pascal void Initialize();
		void IShowDiscJunkCommand(TArticleListView *articleListView, long index);
		pascal void Free();
	private:
		TArticleListView *fArticleListView; 
		long fIndex;
};
//---------------------------------------------------------------------


class TCopyArticleTextCommand : public TCommand
{
	public:
		pascal void DoIt();
		
		TCopyArticleTextCommand();
		pascal void ICopyArticleTextCommand(TArticleListView *articleListView, Boolean asQuote, Boolean addHeader);
	private:
		TArticleListView *fArticleListView;
		Boolean fAsQuote;
		Boolean fAddHeader;
};
//---------------------------------------------------------------------


class TUpdateArticleViewCommand : public TCommand
{
	public:
		pascal void DoIt();
		
		TUpdateArticleViewCommand();
		pascal void Initialize();
		void IUpdateArticleViewCommand(TArticleListView *alv, ArticleShowType whatToShow);
		pascal void Free();
	private:
		TArticleListView *fArticleListView;
		ArticleShowType fWhatToShow;
};

//---------------------------------------------------------------------

class TStoreNotesCommand : public TCommand
{
	public:
		pascal void DoIt();
		
		TStoreNotesCommand();
		pascal void Initialize();
		void IStoreNotesCommand(TArticleListView *alv);
		pascal void Free();

	private:
		TArticleListView *fArticleListView;
		TFile *fFile;
		ParamBlockRec *fPBP;
		
		void OpenFile();
		void StoreQuote();
};
