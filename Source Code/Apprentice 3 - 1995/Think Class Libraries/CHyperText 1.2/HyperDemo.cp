/******************************************************************************
 HyperDemo.cp

		
	main file
	
	Copyright © 1994 Johns Hopkins University. All rights reserved.
	
	Original Author: 	Martin R. Wachter		email:	mrw@welchgate.welch.jhu.edu
	Created:			4/4/94					by:		mrw			TCL Version:	1.1.3
	Modified:			4/5/94					by:		mrw			TCL Version:	1.1.3

 ******************************************************************************/

 
#include "CHyperDemoApp.h"


void main()

{
	CHyperDemoApp	*HyperDemoApp;					

	HyperDemoApp = new CHyperDemoApp;
	
	HyperDemoApp->IHyperDemoApp();
	HyperDemoApp->Run();
	HyperDemoApp->Exit();
}
