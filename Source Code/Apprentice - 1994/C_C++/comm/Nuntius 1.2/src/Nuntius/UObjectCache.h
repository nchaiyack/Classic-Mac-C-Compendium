// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UObjectCache.h

#define __UOBJECTCACHE__

// methods are not made pure virtual as CFront is too verbose about it (pascalobject)
class TObjectCache : public TEventHandler
{
	public:
		virtual void FlushCache();
		
		virtual pascal Boolean DoIdle(IdlePhase phase);
		
		virtual void SetObjectCacheParams(long maxNoCachedObjects, long maxIdleTime);
		// if maxIdleTime is kMaxIdleTime, idle free is disabled
		
		TObjectCache();
		virtual pascal void Initialize();
		void IObjectCache();
		// no idle if kMaxIdleTime
		virtual pascal void Free();
	protected:
		TList *fObjectsInUseList;
		TList *fFreeObjectsList;
		TLongintList *fTimeWhenCachedList;
		long fMaxIdleTime;
		long fMaxNoCachedObjects;
		Boolean fCoHandlerInstalled;
		
		virtual TObject *CreateNewObject(); // = 0
		virtual const char *GetObjectName(); // only for debug

		virtual TObject *GetObject();
		virtual void ReturnObject(TObject *obj); // cannot fail
		virtual void DiscardObject(TObject *obj); // when obj cannot be reused
		// it's allowed to return/discard nil objects

		virtual long GetNoCachedObjects();
		virtual long GetNoActiveObjects();
		
		virtual void CheckIdleTime();
};
