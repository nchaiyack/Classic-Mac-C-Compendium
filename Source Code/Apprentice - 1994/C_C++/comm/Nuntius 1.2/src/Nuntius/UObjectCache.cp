// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UObjectCache.cp

#include "UObjectCache.h"
#include "UFatalError.h"

#ifndef __STDIO__
#include <stdio.h>
#endif

#pragma segment MyTools

#define qDebugObjectCache qDebug & 0

TObjectCache::TObjectCache()
{
}

pascal void TObjectCache::Initialize()
{
	inherited::Initialize();
	fObjectsInUseList = nil;
	fFreeObjectsList = nil;
	fTimeWhenCachedList = nil;
	fCoHandlerInstalled = false;
}

void TObjectCache::IObjectCache()
{
	inherited::IEventHandler(nil);
	FailInfo fi;
	if (fi.Try())
	{
		fMaxNoCachedObjects = 1 << 20;
		fMaxIdleTime = kMaxIdleTime;

		fObjectsInUseList = NewList();
		fFreeObjectsList = NewList();

		TLongintList *lList = new TLongintList();
		lList->ILongintList();
		fTimeWhenCachedList = lList;

		fi.Success();
	}
	else // fail
	{
		Free();
		fi.ReSignal();
	}
}

pascal void TObjectCache::Free()
{
	if (fCoHandlerInstalled)
	{
		gApplication->InstallCohandler(this, false);
		fCoHandlerInstalled = false;
	}
#if qDebugObjectCache
	if (fFreeObjectsList)
		fprintf(stderr, "TObjectCache::Free(), %ld idle %s's was free'd\n", fFreeObjectsList->GetSize(), GetObjectName());
#endif
	if (fObjectsInUseList && fObjectsInUseList->GetSize())
	{
		char msg[200];
		sprintf(msg, "TObjectCache::Free(), Has %ld object in list of USED %s", fObjectsInUseList->GetSize(), GetObjectName());
		CStr255 s(msg);
		PanicExitToShell(s);
	}
	if (fObjectsInUseList)
	{
		fObjectsInUseList->FreeAll();
		fObjectsInUseList->Free();
		fObjectsInUseList = nil;
	}
	if (fFreeObjectsList)
	{
		fFreeObjectsList->FreeAll();
		fFreeObjectsList->Free();
		fFreeObjectsList = nil;
	}
	FreeIfObject(fTimeWhenCachedList); fTimeWhenCachedList = nil;
	inherited::Free();
}


void TObjectCache::SetObjectCacheParams(long maxNoCachedObjects, long maxIdleTime)
{
	if (maxIdleTime != kMaxIdleTime && fCoHandlerInstalled)
	{
		gApplication->InstallCohandler(this, false);
		fCoHandlerInstalled = false;
	}
	fMaxNoCachedObjects = maxNoCachedObjects;
	fMaxIdleTime = maxIdleTime;
	if (maxIdleTime != kMaxIdleTime)
	{
		fMaxIdleTime = maxIdleTime * 3 / 4;
		if (!fCoHandlerInstalled)
		{
			gApplication->InstallCohandler(this, true);
			fCoHandlerInstalled = true;
		}
		SetIdleFreq(maxIdleTime / 4);
	}
	while (fFreeObjectsList->GetSize() > maxNoCachedObjects)
		DiscardObject(GetObject());
	CheckIdleTime();
}

void TObjectCache::FlushCache()
{
#if qDebugObjectCache
	fprintf(stderr, "TObjectCache::FlushCache(), %ld idle %s's was free'd\n", fFreeObjectsList->GetSize(), GetObjectName());
#endif
#if qDebug
	if (fObjectsInUseList->GetSize())
	{
		// we could have a flag for
		// Failure, Panic or FreeThem when any in list of used objects
		char msg[200];
		sprintf(msg, "TLowLevelObjectCache::FlushCache(), have %ld items in list of USED %s", fObjectsInUseList->GetSize(), GetObjectName());
		PanicExitToShell(msg);
	}
#endif
	fFreeObjectsList->FreeAll();
}

TObject *TObjectCache::GetObject()
{
	TObject *obj = nil;
	VOLATILE(obj);
	FailInfo fi;
	if (fi.Try())
	{
		if (fFreeObjectsList->GetSize())
		{
			obj = fFreeObjectsList->Pop();
			long idleTick = fTimeWhenCachedList->Pop();
#if qDebugObjectCache
			fprintf(stderr, "TObjectCache::GetObject(), popped %s at $%lx", GetObjectName(), long(obj));
			fprintf(stderr, " from free list, was idle for %ld seconds\n", (TickCount() - idleTick) / 60);
#endif
#if qDebug
			if (!IsObject(obj))
			{
				fprintf(stderr, "Object $%lx popped from free list is not object", long(obj));
				ProgramBreak(gEmptyString);
			}
#endif
		}
		else
		{
			obj = CreateNewObject();
#if qDebugObjectCache
			fprintf(stderr, "TObjectCache::GetObject(), created new %s at $%lx\n", GetObjectName(), long(obj));
#endif
#if qDebug
			if (!IsObject(obj))
			{
				fprintf(stderr, "Created object $%lx is not object", long(obj));
				ProgramBreak(gEmptyString);
			}
#endif
		}
		fObjectsInUseList->InsertLast(obj);
		fi.Success();
		return obj;
	}
	else // fail
	{
		obj = FreeIfObject(obj);
		fi.ReSignal();
	}
}

void TObjectCache::ReturnObject(TObject *obj)
{
	if (!obj)
		return;
	ArrayIndex index = fObjectsInUseList->GetIdentityItemNo(obj);
#if qDebug
	if (!IsObject(obj))
	{
		ProgramBreak("Returned obj is not object");
		fObjectsInUseList->Delete(obj);
		return;
	}
#endif
	if (index == kEmptyIndex)
	{
#if qDebug
		fprintf(stderr, "TObjectCache::ReturnObject($%lx), this obj is not in list of used obj's\n", long(obj));
		ProgramBreak(gEmptyString);
#endif
		return;
	}
	fObjectsInUseList->DeleteElementsAt(index, 1);
#if qDebug
	ArrayIndex iii = fFreeObjectsList->GetIdentityItemNo(obj);
	if (iii != kEmptyIndex)
	{
		fprintf(stderr, "TObjectCache::ReturnObject($%lx), this obj is in list of unused obj's with index = %ld\n", long(obj), iii);
		ProgramBreak(gEmptyString);
		return; // don't insert it, when it's already in list
	}
#endif
#if qDebugObjectCache
	fprintf(stderr, "TObjectCache, got returned %s at $%lx\n",GetObjectName(), long(obj));
#endif
	FailInfo fi;
	if (fi.Try())
	{
		fFreeObjectsList->InsertLast(obj);
		fTimeWhenCachedList->InsertLast(TickCount());
		fi.Success();
	}
	else // fail
	{
#if qDebug
		fprintf(stderr, "Note: could not add obj $%lx to free list (OK, just continue)", long(obj));
		ProgramBreak(gEmptyString);
#endif
		fFreeObjectsList->Delete(obj);
		obj = FreeIfObject(obj);
		// we do NOT resignal here, as this can be called from a failure handler
	}
}

void TObjectCache::DiscardObject(TObject *obj)
{
	if (!obj)
		return;
#if qDebugObjectCache
	fprintf(stderr, "TObjectCache, discarded %s at $%lx\n", GetObjectName(), long(obj));
#endif
	ArrayIndex index = fObjectsInUseList->GetIdentityItemNo(obj);
#if qDebug
	if (!IsObject(obj))
	{
		ProgramBreak("Discarded obj is not object");
		fObjectsInUseList->Delete(obj);
		return;
	}
	if (index == kEmptyIndex)
	{
		fprintf(stderr, "TObjectCache::ReturnObject($%lx), this obj is not in list of used obj's\n", long(obj));
		ProgramBreak(gEmptyString);
		return;
	}
#endif
	fObjectsInUseList->DeleteElementsAt(index, 1);
	FailInfo fi;
	if (fi.Try())
	{
		obj = FreeIfObject(obj);
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

pascal Boolean TObjectCache::DoIdle(IdlePhase phase)
{
	CheckIdleTime();
	return inherited::DoIdle(phase);
}

void TObjectCache::CheckIdleTime()
{
	long tc = TickCount();
	long oldTick = tc - fMaxIdleTime;
	for (ArrayIndex index = fTimeWhenCachedList->GetSize(); index; index--)
	{
		long cacheTime = fTimeWhenCachedList->At(index);
		if (cacheTime < oldTick)
		{
			TObject *obj = fFreeObjectsList->At(index);
#if qDebugObjectCache
			fprintf(stderr, "TObjectCache, Free's expired %s at $%lx, idle time = %ld seconds\n",
				GetObjectName(), long(obj), (tc - cacheTime) / 60);
			if (!IsObject(obj))
				ProgramBreak("The obj is not object");
#endif
			FailInfo fi;
			if (fi.Try())
			{
				obj = FreeIfObject(obj);
				fi.Success();
			}
			else // fail
			{
#if qDebug
				ProgramBreak("Failed in free of expired cached object");
#endif
			}
			fFreeObjectsList->DeleteElementsAt(index, 1);
			fTimeWhenCachedList->DeleteElementsAt(index, 1);
		}
	} // for
}

TObject *TObjectCache::CreateNewObject()
{
	SubClassResponsibility();
	return nil;
}

const char *TObjectCache::GetObjectName()
{
	SubClassResponsibility();
	return "TObject";
}

long TObjectCache::GetNoCachedObjects()
{
	return fFreeObjectsList->GetSize();
}

long TObjectCache::GetNoActiveObjects()
{
	return fObjectsInUseList->GetSize();
}

