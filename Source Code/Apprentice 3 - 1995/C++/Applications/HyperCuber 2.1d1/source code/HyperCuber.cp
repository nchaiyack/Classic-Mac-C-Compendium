/******************************************************************************

  HyperCuber.c
 
 	A starter main file for writing programs with the
 	THINK Class Library
 
  Copyright © 1990 Symantec Corporation.  All rights reserved.
  
******************************************************************************/

 
#include "CHyperCuberApp.h"
#include <string.h>

//#define __TCL_DEBUG__

void main()
{

	CHyperCuberApp	*HyperCuberApp;					

	HyperCuberApp = new CHyperCuberApp;
	HyperCuberApp->IHyperCuberApp();

	HyperCuberApp->Run();
	HyperCuberApp->Exit();
}
