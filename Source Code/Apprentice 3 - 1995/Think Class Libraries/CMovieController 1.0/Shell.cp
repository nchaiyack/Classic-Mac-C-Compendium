/******************************************************************************

  Shell.c
 
 	A Shell main file for writing programs with the
 	THINK Class Library
 
  Copyright © 1990 Symantec Corporation.  All rights reserved.
  
******************************************************************************/

 
#include "CShellApp.h"


void main()

{
	CShellApp	*ShellApp;					

	ShellApp = new CShellApp;
	
	ShellApp->IShellApp();
	ShellApp->Run();
	ShellApp->Exit();
}
