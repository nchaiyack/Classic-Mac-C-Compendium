// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UMailCmds.h

#define __UMAILCMDS__

class TArticle;
class TArticleListView;
class TGroupDoc;

class TMailToAuthorCommand : public TCommand
{
	public:
		pascal void DoIt();
		
		TMailToAuthorCommand();
		pascal void Initialize();
		void IMailToAuthorCommand(TGroupDoc *doc, TArticleListView *articleListView, 
				Boolean asQuote, Boolean addHeader);
		pascal void Free();
	private:
		TArticleListView *fArticleListView;
		TArticle *fArticle;
		Handle fQuoteH;
		Handle fTypeH;
		TGroupDoc *fDoc;
		Boolean fAsQuote;
		Boolean fAddHeader;
		TLongintList *fArticleIDList;
		
		void LaunchEudora();
		Boolean ApplicationIsInfront(ProcessSerialNumber &appPSN);
		void PutQuoteIntoScrap();
		void CreateTypeText();
		void ExecuteHack();
};
