// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UNntp.cp

#include "UNntp.h"
#include "URealNntp.h"
#include "UFakeNntp.h"
#include "UNewsAppl.h"
#include "UPrefsDatabase.h"
#include "UProgress.h"
#include "UFatalError.h"

#include <RsrcGlobals.h>

#include <ErrorGlobals.h>

#ifndef __STDIO__
#include <stdio.h>
#endif

#pragma segment MyComm

#define qDebugNntpCache qDebug & 0
#define qDebugConstruct qDebug & 0

TNntpCache *gNntpCache = nil;

//-----------------------------------------------------------------
Boolean gUsesFakeNntp = false;

void DoUseFakeNntp()
{
	gUsesFakeNntp = true;
}

Boolean UsesFakeNntp()
{
	return gUsesFakeNntp;
}

//==========================================================

PNntp::PNntp()
{
#if qDebugConstruct
	fprintf(stderr, "PNntp::PNntp() at $%lx called\n", long(this));
#endif
	fCurrentDotName = "";
	fFirstArticleID = 1;
	fLastArticleID = 0;
}

PNntp::~PNntp()
{
#if qDebugConstruct
	fprintf(stderr, "PNntp::~PNntp() at $%lx called\n", long(this));
#endif
}

void PNntp::INntp()
{
#if qDebugConstruct
	fprintf(stderr, "void PNntp::INntp() at $%lx called\n", long(this));
#endif
}

void PNntp::SetGroup(const CStr255 &dotName, Boolean forceUpdate)
{
	if (forceUpdate || fCurrentDotName != dotName)
	{
		fCurrentDotName = dotName;
		DoSetGroup(dotName);
	}
}
		
void PNntp::GetGroupInfo(long &firstArticleID, long &lastArticleID)
{
	firstArticleID = fFirstArticleID;
	lastArticleID = fLastArticleID;
}

//-----------------------------------------------------------------
TNntpCache::TNntpCache()
{
}


pascal void TNntpCache::Initialize()
{
	inherited::Initialize();
	fObjectsInUseList = nil;
	fIdleObjectsList = nil;
	fTimeWhenCachedList = nil;
}

void TNntpCache::INntpCache()
{
	inherited::IPeriodicAction(true);
	FailInfo fi;
	if (fi.Try())
	{
		TLongintList *lList = new TLongintList();
		lList->ILongintList();
		fObjectsInUseList = lList;

		lList = new TLongintList();
		lList->ILongintList();
		fIdleObjectsList = lList;

		lList = new TLongintList();
		lList->ILongintList();
		fTimeWhenCachedList = lList;

		UpdateTiming();
		fi.Success();
	}
	else // fail
	{
		Free();
		fi.ReSignal();
	}
}

pascal void TNntpCache::Free()
{
#if qDebugNntpCache
	if (fIdleObjectsList)
		fprintf(stderr, "TObjectCache::Free(), %ld idle %s's was free'd\n", fIdleObjectsList->GetSize(), GetObjectName());
#endif
	if (fObjectsInUseList && fIdleObjectsList && fTimeWhenCachedList)
		FlushCache(); // free all cached objects
	if (fObjectsInUseList && fObjectsInUseList->GetSize())
	{
		char msg[200];
		sprintf(msg, "TObjectCache::Free(), Has %ld objects in list of in-use %s", fObjectsInUseList->GetSize(), GetObjectName());
		CStr255 s(msg);
		PanicExitToShell(s);
	}
	FreeIfObject(fObjectsInUseList); fObjectsInUseList = nil;
	FreeIfObject(fIdleObjectsList); fIdleObjectsList = nil;
	FreeIfObject(fTimeWhenCachedList); fTimeWhenCachedList = nil;
	inherited::Free();
}

long TNntpCache::CreateNewObject()
{
	gCurProgress->SetText(kNntpOpensConnection);
	if (gUsesFakeNntp)
	{
		PFakeNntp *nntp = new PFakeNntp();
		nntp->IFakeNntp();
		return long(nntp);
	}
	else
	{
		PRealNntp *nntp = new PRealNntp();
		nntp->IRealNntp(gPrefs->GetLongPrefs('SvIP'));
		return long(nntp);
	}
}

long TNntpCache::FreeObject(long obj)
{
	if (obj)
	{
		if (gUsesFakeNntp)
			delete (PFakeNntp *) obj;
		else
			delete (PRealNntp *) obj;
	}
	return nil;
}

const char *TNntpCache::GetObjectName()
{
	if (gUsesFakeNntp)
		return "PFakeNntp";
	else
		return "PRealNntp";
}

long TNntpCache::GetNoIdleConnections()
{
	return fIdleObjectsList->GetSize();
}

long TNntpCache::GetNoActiveConnections()
{
	return fObjectsInUseList->GetSize();
}

void TNntpCache::UpdateTiming()
{
	SetSleep(gPrefs->GetLongPrefs('Tntp') / 4);
}

//----------------------------------------------------

void TNntpCache::FlushCache()
{
#if qDebugNntpCache
	fprintf(stderr, "TNntpCache::FlushCache(), %ld idle %s's was free'd\n", fIdleObjectsList->GetSize(), GetObjectName());
#endif
#if qDebug
	if (fObjectsInUseList->GetSize())
	{
		fprintf(stderr, "TNntpCache::FlushCache(), There is %ld in list of USED %s!\n", fObjectsInUseList->GetSize(), GetObjectName());
	}
#endif
	while (fIdleObjectsList->GetSize())
	{
		fTimeWhenCachedList->Pop();
		FreeObject(fIdleObjectsList->Pop());
	}
}

PNntp *TNntpCache::GetNntp()
{
	long obj = nil;
	VOLATILE(obj);
	FailInfo fi;
	if (fi.Try())
	{
		if (fIdleObjectsList->GetSize())
		{
			obj = fIdleObjectsList->Pop();
			long idleTick = fTimeWhenCachedList->Pop();
#if qDebugNntpCache
			fprintf(stderr, "TNntpCache::GetObject(), popped %s at $%lx", GetObjectName(), long(obj));
			fprintf(stderr, " from free list, was idle for %ld seconds\n", (TickCount() - idleTick) / 60);
#endif
#if qDebug
			if (!IsPtrObject((PNntp*)obj))
			{
				fprintf(stderr, "Object $%lx popped from free list is not object", long(obj));
				ProgramBreak(gEmptyString);
			}
#endif
		}
		else
		{
			obj = CreateNewObject();
#if qDebugNntpCache
			fprintf(stderr, "TNntpCache::GetObject(), created new %s at $%lx\n", GetObjectName(), long(obj));
#endif
#if qDebug
			if (!IsPtrObject((PNntp*)obj))
			{
				fprintf(stderr, "Created object $%lx is not object", long(obj));
				ProgramBreak(gEmptyString);
			}
#endif
		}
		fObjectsInUseList->InsertLast(obj);
		fi.Success();
		return (PNntp*)obj;
	}
	else // fail
	{
		obj = FreeObject(obj);
		fi.ReSignal();
	}
}

void TNntpCache::ReturnNntp(PNntp *nntp)
{
	long obj = long(nntp);
	if (!obj)
		return;
	ArrayIndex index = fObjectsInUseList->GetIdentityItemNo(obj);
#if qDebug
	if (!IsPtrObject(nntp))
	{
		ProgramBreak("Returned obj is not object");
		fObjectsInUseList->Delete(obj);
		return;
	}
#endif
	if (index == kEmptyIndex)
	{
#if qDebug
		fprintf(stderr, "TNntpCache::ReturnObject($%lx), this obj is not in list of used obj's\n", long(obj));
		ProgramBreak(gEmptyString);
#endif
		return;
	}
	fObjectsInUseList->DeleteElementsAt(index, 1);
#if qDebug
	ArrayIndex iii = fIdleObjectsList->GetIdentityItemNo(obj);
	if (iii != kEmptyIndex)
	{
		fprintf(stderr, "TNntpCache::ReturnObject($%lx), this obj is in list of unused obj's with index = %ld\n", long(obj), iii);
		ProgramBreak(gEmptyString);
		return; // don't insert it, when it's already in list
	}
#endif
#if qDebugNntpCache
	fprintf(stderr, "TNntpCache, got returned %s at $%lx\n",GetObjectName(), long(obj));
#endif
	if (fIdleObjectsList->GetSize() >= gPrefs->GetLongPrefs('Mntp'))
	{
		obj = FreeObject(obj);
		return;
	}
	FailInfo fi;
	if (fi.Try())
	{
		fIdleObjectsList->InsertLast(obj);
		fTimeWhenCachedList->InsertLast(TickCount());
		fi.Success();
	}
	else // fail
	{
#if qDebug
		fprintf(stderr, "Note: could not add obj $%lx to free list (OK, just continue)", long(obj));
		ProgramBreak(gEmptyString);
#endif
		fIdleObjectsList->Delete(obj);
		obj = FreeObject(obj);
		// we do NOT resignal here, as this can be called from a failure handler
	}
}

void TNntpCache::DiscardNntp(PNntp *nntp)
{
	long obj = long(nntp);
	if (!obj)
		return;
#if qDebugNntpCache
	fprintf(stderr, "TNntpCache, discarded %s at $%lx\n", GetObjectName(), long(obj));
#endif
	ArrayIndex index = fObjectsInUseList->GetIdentityItemNo(obj);
#if qDebug
	if (!IsPtrObject(nntp))
	{
		ProgramBreak("Discarded obj is not object");
		fObjectsInUseList->Delete(obj);
		return;
	}
	if (index == kEmptyIndex)
	{
		fprintf(stderr, "TNntpCache::ReturnObject($%lx), this obj is not in list of used obj's\n", long(obj));
		ProgramBreak(gEmptyString);
		return;
	}
#endif
	fObjectsInUseList->DeleteElementsAt(index, 1);
	FailInfo fi;
	if (fi.Try())
	{
		obj = FreeObject(obj);
		fi.Success();
	}
	else // fail
	{
#if qDebug
		fprintf(stderr, "Got signal when free'ing discarded %s\n", GetObjectName());
		ProgramBreak(gEmptyString);
#endif
	}
}

void TNntpCache::DoPeriodic()
{
	long tc = TickCount();
	long oldTick = tc - gPrefs->GetLongPrefs('Tntp') * 4 / 3;
	long maxCached = gPrefs->GetLongPrefs('Mntp');
	long numReleased = 0;
	for (ArrayIndex index = 1; index <= fTimeWhenCachedList->GetSize(); index++)
	{
		long cacheTime = fTimeWhenCachedList->At(index);
		if (cacheTime < oldTick || index > maxCached)
		{
			long obj = fIdleObjectsList->At(index);
#if qDebugNntpCache
			fprintf(stderr, "TNntpCache, Free's expired %s at $%lx, idle time = %ld seconds\n",
				GetObjectName(), long(obj), (tc - cacheTime) / 60);
			if (!IsPtrObject((PNntp*)obj))
				ProgramBreak("The obj is not object");
#endif
			fIdleObjectsList->DeleteElementsAt(index, 1);
			fTimeWhenCachedList->DeleteElementsAt(index, 1);
			FailInfo fi;
			if (fi.Try())
			{
				obj = FreeObject(obj);
				fi.Success();
			}
			else // fail
			{
#if qDebug
				ProgramBreak("Failed in free of expired cached object");
#endif
			}
			numReleased++;
		}
	} // for
#if qDebug
	fprintf(stderr, "Released %ld NNTP connections\n", numReleased);
#endif
}

Boolean TNntpCache::NeedTime()
{
	if (fIdleObjectsList->fSize > gPrefs->GetLongPrefs('Mntp'))
		return true;
	long tc = TickCount();
	long oldTick = tc - gPrefs->GetLongPrefs('Tntp') * 4 / 3;
	for (ArrayIndex index = 1; index <= fTimeWhenCachedList->fSize; index++)
	{
		long cacheTime = fTimeWhenCachedList->At(index);
		if (cacheTime < oldTick)
			return true;
	}
	return false;	
}

Boolean TNntpCache::SkipFirstCheckAfterWakeUp()
{
	return true;
}

const char *TNntpCache::GetDebugDescription()
{
	return "TNntpCache";
}


//.................................................................

void InitUNntp()
{
#if qDebug
	if (gNntpCache)
		ProgramBreak("InitUNntp called twice");
#endif
	TNntpCache *nc = new TNntpCache();
	nc->INntpCache();
	gNntpCache = nc;
}

void CloseDownUNntp()
{
#if qDebug
	if (!gNntpCache)
		fprintf(stderr, "gNntpCache is nil in CloseDownUNntp()\n");
#endif
	if (gNntpCache)
	{
		gNntpCache->Close();
		FreeIfObject(gNntpCache); gNntpCache = nil;
	}
}
