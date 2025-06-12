// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UPostArticleCmds.h

#define __UPOSTARTICLECMDS__

#ifndef __UPTROBJECT__
#include "UPtrObject.h"
#endif

#ifndef __UPOSTARTICLEVIEW__
#include "UPostArticleView.h"
#endif

class TGroupDoc;
class TArticle;
class PNntp;
class TThreadList;

class PPostArticleInfo : public PPtrObject
{
	public:
		CStr255 fSubject;
		CStr255 fNewsGroups;
		CStr255 fReferences;
		CStr255 fDistribution;
		Boolean fEditHeaders;
		Boolean fEditSignature;
		Boolean fAddSignatureChoice;
		Boolean fTrashDiskCopyAfterPost;
		TFile *fFile;
		Handle fDefaultTextH;
		PNntp *fNntp;
		
		void CreateNntp(); // no-op if existing
		void DiscardNntp();

		void MakeHeader(TStream *aStream, short templateID);
		void MakeSignature(TStream *aStream);

		void CreateWindows();
		void ShowPostArticleWindow();
		void DoPostArticle();
		void Cancel(); // postponed for windows!

		void MakeFile();
		void LaunchEditor();
		
		PPostArticleInfo();
		void IPostArticleInfo();
		~PPostArticleInfo();
	private:
		short fRefCount;
		TThreadList *fThreadList;
		
		TWindow *fCreateArticleW;
		TEditArticleToPostDialogView *fCreateArticleView;
		
		TWindow *fPostArticleW;
		TPostArticleDialogView *fPostArticleView;
};


class TCreateNewDiscussionCommand : public TCommand
{
	public:
		virtual pascal void DoIt();

		TCreateNewDiscussionCommand();
		virtual pascal void Initialize();
		void ICreateNewDiscussionCommand(TGroupDoc *doc);
		virtual pascal void Free();
	protected:
		PPostArticleInfo *fPostArticleInfo;
		TGroupDoc *fDoc;
		
		virtual void PrepareInfo();
};

class TCreateFollowupCommand : public TCreateNewDiscussionCommand
{
	// a follow-up is a "new" thread with references to an existing thread
	public:
		TCreateFollowupCommand();
		virtual pascal void Initialize();
		void ICreateFollowupCommand(TGroupDoc *doc, ArrayIndex discIndex, 
						Handle defaultText);
		virtual pascal void Free();
	protected:
		Boolean GetMsgID(long articleID, CStr255 &id);
		Handle fDefaultText;
		TArticle *fArticle;
		long fDiscIndex;
		
		virtual void PrepareInfo();
		virtual void DoPrepareArticleInfo(long id);
};

class TPostArticleCommand : public TCommand
{
	public:
		virtual pascal void DoIt();
		
		TPostArticleCommand();
		virtual pascal void Initialize();
		void IPostArticleCommand(PPostArticleInfo *info);
		virtual pascal void Free();
	protected:
		PPostArticleInfo *fPostArticleInfo;
		Handle fText;
		
		virtual void ReadArticle();
		virtual void Export2022Body(long bodySize);
		virtual void PostArticle();
		virtual void TrashDiskCopy();
};

class TCancelArticleCommand : public TPostArticleCommand
{
	public:
		pascal void DoIt();

		TCancelArticleCommand();
		pascal void Initialize();
		void ICancelArticleCommand(const CStr255 &messageID);
		pascal void Free();
	protected:
		void ReadArticle();
		void PostArticle();
};
