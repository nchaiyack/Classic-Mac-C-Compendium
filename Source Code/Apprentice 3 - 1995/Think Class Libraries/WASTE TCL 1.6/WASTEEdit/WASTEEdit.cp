/******************************************************************************
	WASTEEdit.c
	
	Main program for a tiny editor.
	
	Copyright © 1989 Symantec Corporation. All rights reserved.

 ******************************************************************************/

#include "CEditApp.h"


void main()

{
	CEditApp	*editApp;
		
	editApp = new CEditApp;
	editApp->IEditApp();
	editApp->Run();
	editApp->Exit();

}
