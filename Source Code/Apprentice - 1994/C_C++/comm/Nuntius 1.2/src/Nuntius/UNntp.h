// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UNntp.h

#define __UNNTP__

#ifndef __UPERIODIC__
#include "UPeriodic.h"
#endif

#ifndef __UPTROBJECT__
#include "UPtrObject.h"
#endif

class PNntp : public PPtrObject
{
	public:
		// NOTE: All returned handles are owned by the caller
		// The handles are null terminated

		virtual Handle GetListOfAllGroups() = 0;
		virtual Handle GetListOfNewGroups(unsigned long fromDate) = 0;
		
		virtual Handle GetListOfGroupDesciptions() = 0;
		
		virtual void SetGroup(const CStr255 &dotName, Boolean forceUpdate = false);

		virtual void GetGroupInfo(long &firstArticleID, long &lastArticleID);
		
		virtual Handle GetHeaderList(const char *headerName, long firstArticleID, long lastArticleID) = 0;
		
		virtual Handle GetArticle(long articleID) = 0;
		
		virtual void PostArticle(Handle h, short ackStringID) = 0;
		
		virtual Boolean IsPostingAllowed() = 0; // general, not for the specific group
		
		virtual void ExamineNewsServer() = 0;

		PNntp();
		virtual void INntp();
		virtual ~PNntp();
	protected:
		CStr255 fCurrentDotName;
		long fFirstArticleID, fLastArticleID;

		virtual void DoSetGroup(const CStr255 &name) = 0;
};


class TNntpCache : public TPeriodicAction
{
	public:
		PNntp *GetNntp();
		void ReturnNntp(PNntp *nntp); // cannot fail
		void DiscardNntp(PNntp *nntp); // cannot fail
		// it's allowed to return/discard nil objects
		
		long GetNoIdleConnections();
		long GetNoActiveConnections();

		void FlushCache();
		void UpdateTiming();

		TNntpCache();
		pascal void Initialize();
		void INntpCache();
		pascal void Free();
	
	protected:
		void DoPeriodic();
		const char *GetDebugDescription();
		Boolean NeedTime();
		Boolean SkipFirstCheckAfterWakeUp();
		
	private:
		TLongintList *fObjectsInUseList; // realy need TPtrList
		TLongintList *fIdleObjectsList;
		TLongintList *fTimeWhenCachedList;

		long CreateNewObject();
		long FreeObject(long obj);

		const char *GetObjectName();
};

void DoUseFakeNntp(); // sets global flag
Boolean UsesFakeNntp();

void InitUNntp();
void CloseDownUNntp();
extern TNntpCache *gNntpCache;
