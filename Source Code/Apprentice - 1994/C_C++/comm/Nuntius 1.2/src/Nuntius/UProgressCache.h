// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UProgressCache.h

#define __UPROGRESSCACHE__

#ifndef __UOBJECTCACHE__
#include "UObjectCache.h"
#endif

class TProgress;

class TProgressCache : public TObjectCache
{
	public:
		TProgress *GetProgress();
		void ReturnProgress(TProgress *progress); // cannot fail
		void DiscardProgress(TProgress *progress); // cannot fail
		// it's allowed to return/discard nil objects

		TProgressCache();
		pascal void Initialize();
		void IProgressCache();
		pascal void Free();
	protected:
		TObject *CreateNewObject();
		const char *GetObjectName();
};

void InitUProgressCache();
void CloseDownUProgressCache();
extern TProgressCache *gProgressCache;

