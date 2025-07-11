// Copyright � 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UPtrObject.h

#define __UPTROBJECT__

class PPtrObject
{
	public:
		void *operator new(size_t size);
		void operator delete(void *p);
		
		PPtrObject();
		virtual ~PPtrObject();
};


Boolean IsPtrObject(PPtrObject *obj);
void FreeIfPtrObject(PPtrObject *obj);
void FailNonPtrObject(PPtrObject *obj);

void InitUPtrObject();
