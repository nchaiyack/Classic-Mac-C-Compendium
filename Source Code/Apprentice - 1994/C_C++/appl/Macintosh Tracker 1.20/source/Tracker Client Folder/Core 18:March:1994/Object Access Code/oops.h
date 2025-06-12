
/*
 *  oops.h
 *
 *  Copyright (c) 1991 Symantec Corporation.  All rights reserved.
 *
 */

#pragma once


/* ---------- public interface ---------- */


	/*  member  */

#define member(o, c)	__member(__class c, __class(o))
#if __option(far_code) || __option(far_data)
char __member(void *, void *);
#else
char __member(void *, short);
#endif


	/*  instantiation by name  */

void *new_by_name(char *);
#define class_name(o)	__class_name(__class(o))
char *__class_name(...);


	/*  "__cn" - macro implementation of "class_name" (for use in debugger)  */

#if !__option(a4_globals)

#if __option(far_code) || __option(far_data)
	#define __caddr(o)			__class(o)
	#define __cshift			3
	#define __cofs				36
#else
	#define __caddr(o)			(__class(o) + * (long *) 0x904)
	#define __cshift			2
	#define __cofs				22
#endif

#define __cn(o)		&((char *) __caddr(o))[((* (short *) __caddr(o)) << __cshift) + __cofs]

#endif


	/*  debugging hooks  */

void __noObject(void);
void __noMethod(void);


/* ---------- private interface ---------- */


#ifdef OOPS_PRIVATE


	/*  configuration  */

#if __option(far_code) || __option(far_data)
	// no base reg
#elif __option(a4_globals)
	#define BASE_REG	A4
#else
	#define BASE_REG	A5
#endif


	/*  size of references  */

#ifdef BASE_REG
	typedef short Ref;
	#define _			W
	#define DSHIFT		2
#else
	typedef void *Ref;
	#define _			L
	#define DSHIFT		3
#endif


	/*  dispatch table format  */

typedef struct {
	Ref				method;
	Ref				selector;
} DispatchEntry;

typedef struct {
	short			countM1;			/*  # entries - 1  */
	DispatchEntry	entry[];			
} DispatchTable;


	/*  class info format  */

typedef struct {
	Ref				superclass;			/*  0 for root class (MUST BE FIRST)  */
	Ref				allocator;			/*  "operator new" (0 if none)  */
	Ref				deallocator;		/*  "operator delete" (0 if none)  */
	Ref				constructor;		/*  "X::X()" (0 if none)  */
	Ref				destructor;			/*  "X::~X()" (0 if none)  */
	long			size;				/*  # bytes in object of this class  */
	short			dispatch;			/*  offset back to dispatch table (+1 for indirect)  */
	char			name[];				/*  class name (0-terminated)  */
} ClassInfo;

#define ClassInfo_(ofs)		((int) &((ClassInfo *) 0)->ofs)


#endif /* OOPS_PRIVATE */
