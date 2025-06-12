// Copyright © 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UPtrObject.cp

#include "UPtrObject.h"
#include "UFatalError.h"

#pragma segment MyTools

Boolean IsPtrObject(PPtrObject *obj)
{
#if qDebug
	if (!obj)
		fprintf(stderr, "obj is nil\n");
	else if (long(obj) & 1)
		fprintf(stderr, "obj is odd: $%lx\n", long(obj));
	else if (PtrZone(Ptr(obj)) != ApplicationZone() || MemError())
		fprintf(stderr, "PtrZone(Ptr(obj)) = $%lx != ApplicationZone() = %lx,   obj = $%lx\n", PtrZone(Ptr(obj)), ApplicationZone(), long(obj));
	else if (*(long*)obj == 0xF1F1F1F1)
		fprintf(stderr, "obj appears to be uninitialized: at $%lx\n", long(obj));
	else
		return true;
	ProgramBreak("Probably don't continue...");
	return false;
#else
	return obj != nil;
#endif
}

void FreeIfPtrObject(PPtrObject *obj)
{
	if (obj && IsPtrObject(obj))
		delete obj;
}

void FailNonPtrObject(PPtrObject *obj)
{
	if (!IsPtrObject(obj))
		FailOSErr(minErr);
}

//----------------------------------------------------------------------

void InitUPtrObject()
{
}

void *PPtrObject::operator new(size_t size)
{
	Ptr p = NewPtr(size);
	if (!p)
		PanicExitToShell("Got nil NewPtr in 'void *PPtrObject::operator new'");
		//@@ Failure ????
#if qDebug
	BlockSet(p, size, 0xF1);
#endif
	return p;
}

void PPtrObject::operator delete(void *p)
{
	if (!p)
		return;
#if qDebug
	IsPtrObject((PPtrObject*)p);
	long size = GetPtrSize(Ptr(p));
	BlockSet(Ptr(p), size, 0xF1);
#endif
	DisposPtr(Ptr(p));
	if (MemError())
		ProgramBreak("Bad DisposPtr()");
}

PPtrObject::PPtrObject()
{
}

PPtrObject::~PPtrObject()
{
}

