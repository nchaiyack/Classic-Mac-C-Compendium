/*
	In PPC applications, constructors and destructors for global and static objects are called automatically
	by __cplusstart, which is actually the main entry point to your application (and not main()).  This
	routine calls your constructors, initializes your QuickDraw globals, calls your main() function, then
	calls your destructors.
	
	In a shared library, constructors for global and static objects are called by __cplusinit, which the linker
	assigns as an initialization routine.  The Code Fragment Manager will automatically call this initialization
	routine when your shared library is loaded.  __cplusinit will also allow hook your shared library's
	QuickDraw globals to the QuickDraw globals of the calling application.  Destructors for global and static
	objects are called by __cplusterm, which the linker assigns as a termination routine.  The Code Fragment
	Manager will automatically call this termination routine when your shared library is unloaded.
	
	In code resources, there is no way to call these constructors or destructors automatically, or hook
	QuickDraw globals, and still preserve the arguments passed to your main entry point (main()).  Instead,
	you will need to call initialization and termination routines to do this for you.
	
	In C, you should use __rsrcinit() and __rsrcterm(), defined in stdlib.h.
	
	In C++, you should use __cplusrsrcinit() and __cplusrsrcterm(), defined in new.h.
	
	The C++ initialization and termination routines will automatically call the C initialization and termination
	routines.  You should only use one set or the other.
	
	You must not use any objects that rely on their constructor, or any QuickDraw globals, before calling
	__rsrcinit() or __cplusrsrcinit().
	
	
	Singlely-entrant code resources :
	
	If your code resource is called only once, you should call __rsrcinit() or __cplusrsrcinit() at the start of
	main(), and __rsrcterm() or __cplusrsrcterm() at the end.
	
	Reentrant code resources :
	
	If your code resource is reentrant, you will need to call __rsrcinit() or __cplusrsrcinit() only the first
	time through, and __rsrcterm() or __cplusrsrcterm() the last time through.
	
	
	Colen Garoutte-Carson
	Symantec Corp.
*/

static Boolean gDidInit=false,gDidTerm=false;

void main(void){
	
	if (!gDidInit){
		gDidInit=true;
		__rsrcinit();
	}
	
	if (gDoTerm){
		gDoTerm=false;
		__rsrcterm();
	}
}
