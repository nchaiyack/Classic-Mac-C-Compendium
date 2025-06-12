// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UProgress.cp

#include "UProgressCache.h"
#include "UProgress.h"

#pragma segment MyThread

#define qDebugProgressCache qDebug & 0

TProgressCache *gProgressCache = nil;

//-----------------------------------------------------------------
TProgressCache::TProgressCache()
{
}

pascal void TProgressCache::Initialize()
{
	inherited::Initialize();
}

void TProgressCache::IProgressCache()
{
	inherited::IObjectCache();
	fObjectsInUseList->SetEltType("TProgress");
	fFreeObjectsList->SetEltType("TProgress");
	SetObjectCacheParams(5, kMaxIdleTime);
}

pascal void TProgressCache::Free()
{
	inherited::Free();
}

TProgress *TProgressCache::GetProgress()
{
	TProgress *pgrs = (TProgress*) inherited::GetObject();
#if qDebugProgressCache
	fprintf(stderr, "GetProgress: at $%lx\n", pgrs);
#endif
	return pgrs;
}

void TProgressCache::ReturnProgress(TProgress *progress)
{
#if qDebugProgressCache
	fprintf(stderr, "ReturnProgress: at $%lx\n", progress);
#endif
	if (progress)
		progress->SetThread(nil);
	inherited::ReturnObject(progress);
}

void TProgressCache::DiscardProgress(TProgress *progress)
{
#if qDebugProgressCache
	fprintf(stderr, "DiscardProgress: at $%lx\n", progress);
#endif
	if (progress)
		progress->SetThread(nil);
	inherited::DiscardObject(progress);
}

TObject *TProgressCache::CreateNewObject()
{
	TProgress *progress = new TProgress();
	progress->IProgress();
#if qDebugProgressCache
	fprintf(stderr, "TProgressCache::CreateNewObject: at $%lx\n", progress);
#endif
	return progress;
}

const char *TProgressCache::GetObjectName()
{
	return "TProgress";
}

//.................................................................

void InitUProgressCache()
{
#if qDebug
	if (gProgressCache)
		ProgramBreak("InitUProgress is called twice");
#endif
	TProgressCache *pc = new TProgressCache();
	pc->IProgressCache();
	gProgressCache = pc;
}

void CloseDownUProgressCache()
{
	if (!gProgressCache)
	{
#if qDebug
		fprintf(stderr, "gProgressCache is nil in CloseDownUProgress\n");
#endif
		return;
	}
#if qDebug
	//@@ AVOID ERROR SOMEWHERE IN CODE:
	// At quit: 1 in list of USED TProgress
	FreeIfObject(gProgressCache); gProgressCache = nil;
#endif
}
