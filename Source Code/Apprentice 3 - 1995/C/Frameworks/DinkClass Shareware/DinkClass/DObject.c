/*
	File:		DObject.cp

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	© 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <3>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <2>	 9/20/92	MTG		Bringin the C++ version up to date with the THINK C version

	To Do:
*/


// This is the definition file for the DObject root class
// for the DinkClass CLass library.  it is responcible for
// implementing Global wornings and some Debuging tools.

#include "DObject.h"

#ifdef THINK_C
	#include <pascal.h>
#else
	//nothing!
#endif

#include <stdio.h>

void	DObject::ErrorAlert(short stringsID, short theError)
{
	short	result;
	Str255	theStr;
	
	GetIndString(theStr, stringsID, theError);
	
	ParamText( theStr , "\p", "\p", "\p");
	result = CautionAlert(rErrorAlert, NULL);

}// end of function ErrorAlert

 

void	DObject::Beep(int times)
{
	while ( times--)
		SysBeep(20);
}

void 	DObject::EnterMB(void)
{
	Debugger( );
}

/*
void	DObject::EnterMBStr(char *theString)
{
	
	DebugStr( CtoPstr(theString) );
	// MPW version ... Debugger( C2Pstr(theString) );

}
*/

void	DObject::EnterMBStr(char *theString)
{
	
//	DebugStr( CtoPstr(theString) );

// Non-DeBug Version.....
	short	result;
		
	ParamText( CtoPstr(theString) , "\p", "\p", "\p");
	result = CautionAlert(rErrorAlert, NULL);
}


void DObject::MakeMBAlias(char* alias, void* address)
{
	char buffer[80];
	
	sprintf(buffer, ";MC %s %lx", alias, address);
				// Just be carefull that the string in alias is a vaild
				// MB macro name!!!	
	EnterMBStr(buffer);
}		
	

pascal void DObject::SetZero(void)
{
	*(long *)(NULL) = 0x50FFC001;
}

Boolean DObject::HasColorQD(void)
{
	SysEnvRec	env_rec;
	
	if (SysEnvirons(1, &env_rec) == envNotPresent || !env_rec.hasColorQD)
		return(FALSE);
	return(TRUE);
}


// The following are the hooks into the ThinkC oopsDebug lib
// Gee, why arn't these guys docuemented in the manuals?
// I just kept getting link errors untill I knew enough 
// to look at the interface file oops.h to find the specs
// for these two "/*  debugging hooks  */"

void __noObject(void)
{
		SysBeep(20);
		DebugStr("\p noObject catch, from oopsDebug stubs!!!");
}

void __noMethod(void)
{
		SysBeep(20);
		SysBeep(20);
		DebugStr("\p noMethod catch, from oopsDebug stubs");
}


