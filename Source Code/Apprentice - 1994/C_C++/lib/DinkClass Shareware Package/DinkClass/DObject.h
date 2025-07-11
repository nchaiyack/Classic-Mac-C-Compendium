/*
	File:		DObject.h

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	� 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <4>	12/31/92	MTG		fixing typeo
		 <3>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <2>	 9/20/92	MTG		Bringing the C++ version up to date with the THINK C version

	To Do:
*/

#ifndef __DOBJECT__
#define __DOBJECT__
#ifdef THINK_C
	#include <oops.h>
#else
	//for MPW
	#include <Types.h>
	#include <Events.h>
	#include <Quickdraw.h>
	#include <Menus.h>
	#include <Memory.h>
	#include <Desk.h>
	#include <Scrap.h>
	#include <AppleEvents.h>
	#include <DiskInit.h>
	#include <Errors.h>
	#include <OSEvents.h>
	#include <Quickdraw.h>
	#include <Fonts.h>
	#include <ToolUtils.h>
	#include <Dialogs.h>
	#include <Strings.h>
	#include <Sysequ.h>
	
	#define CtoPstr c2pstr
	
	#define FALSE false
	#define TRUE true
#endif

// This is the class declaration for the root Class of my class
// Library,  It has global warning and debug functions in it.
// I provides on convieniant place to but this kind of stuff.

// More usefull member fucntions will be added as the use of the
// class library provides more experiance, especialy with respect 
// to th MacsBug (MB) realated functions.

#ifdef THINK_C
	class DObject 
#else
	class DObject  : public PascalObject
#endif	
{ 
public :
	
	void	ErrorAlert(short stringsID, short theError);
		// brings up a dialog box loaded from the Resource file
		// and dissmisses with out doing anything else.
	
	void	Beep(int times);
		// Just beep so many times, calls sysbeep.

	void 	EnterMB(void);
			// simply calls the Debugger trap
	
	void	EnterMBStr(char *theString);
			// converts theString (in PLACE) into a Pstring and calls
			// the DebugStr trap.

	void MakeMBAlias(char* alias, void* address);
			// uses DebugStr to define an alias macro for the
			// addres passed to the ruteen.  Us this to look at
			// local variables or other runtime memory items.
			// I like to use it to SS an array bounds or something 
			// like that.
			
	pascal void SetZero(void);
			// A debuging trick taken from MacsBugReference and Debugging
			// guid, Call this funciton from the main event loop to
			// catch the derefrencing of an NULL Handle

	Boolean HasColorQD(void);
			// checks to see if color quick draw is running.
			
};// more MB realted member functions to follow!!

#define	rErrorAlert	255
#define rErrorStrings	255
#define	sNoMem	1
#define	sFileOpen	2
#define	sUnknownErr	3


#endif __DOBJECT__
