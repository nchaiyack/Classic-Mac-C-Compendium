// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UArticleStatus.h

#define __UARTICLESTATUS__

// value of longs stored in this list:
enum ArticleStatus {kArticleNew = 0, kArticleSeen, kArticleRead};

// value of long in preferences
enum ArticleShowType {kShowAllArticles, kShowUnreadArticles, kShowNewArticles, kShowFirstArticleOnly, kShowNoArticles};

enum DiscussionShowType {kShowAllDiscs, kShowTodaysDiscs, 
		kShowDiscsWithUnreadArticles, kShowDiscsWithNewArticles};

class TArticleStatus : public TLongintList
{
	public:
		ArticleStatus GetStatus(long id);
		void SetStatus(long id, ArticleStatus status);
		void SetMinStatus(long id, ArticleStatus status);
		
		void SetNewRange(long newFirstID, long newLastID);

		void DoRead(TStream *aStream);
		void DoIronAgeFormatRead(TStream *aStream);
		void DoWrite(TStream *aStream);
		void DoNeedDiskSpace(long &dataForkBytes);

		void DebugDump();
		Boolean SanityCheck();
		
		void SpecifyWithArticleStatus(TArticleStatus *articleStatus);

		TArticleStatus();
		pascal void Initialize();
		void IArticleStatus(TDocument *doc);
		pascal void Free();
	private:
		friend TArticleStatus::SpecifyWithArray;
		long fFirstID;
		Boolean fChanged;
		TDocument *fDoc;
		
		void IsChanged();
};
