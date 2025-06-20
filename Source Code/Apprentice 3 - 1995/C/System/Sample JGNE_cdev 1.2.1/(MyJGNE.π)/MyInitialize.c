//
// MyInitialize.c
//
// Written by Ken Worley, 06/03/94, using Symantec Think C 7.0
// Copyright 1994.
// AOL KNEworley
// internet KNEworley@aol.com
//
//	Feel free to use this code in a project of your own, but give me proper
//	credit in your documentation or about box.  Feel free to distribute this
//	code in its entirety to anyone, but never do so without the copyright
//	notice above nor without its accompanying files.  This code is NOT in
//	the public domain.  Use of this code in a commercial product requires my
//	permission.
//
// This file takes care of allocating memory, loading resources, and initializing
// variables specific to the function of the filter.  We use this file mainly to
// set function specific code off from the rest of the code that doesn't change
// much from project to project.
//
// This routine is allowed to change the values of installTask, and showIcon.
// These variables have already been initialized by the calling routine, but their
// values can be changed here if need be.  installTask determines whether or
// not the filter code is actually installed, and showIcon determines whether or
// not the icon is shown at startup time.  (showIcon is called showStartupIcon
// in the calling routine)
//
//	We use what I call a 'preferences' resource to save some settings between
//	restarts.  For the most part, this same information is also held in memory
//	(in the shared data structure) while the computer is running so that the
//	task code and cdev can access the settings.  The resource is normally accessed
//	once here, then accessed and possibly changed later by the
//	control panel code.
//
//	This preferences resource contains values that indicate whether or not the
//	task should be installed (control panel on or off) and whether or not the
//	icon should be shown at startup time.
//

Boolean MyInitialize( myDataPtr myData, Boolean *installTask, Boolean *showIcon )
{
	Boolean				stillOK = true;	/* return value */
	CPprefsHandle		prefsHandle;	/* Handle to the cdev prefs rsrc */

	// Initialize the preferences resource handle in myData to NULL
	
		myData->CPprefsRsrc = NULL;
			
	//	Load the control panel's preferences resource to see if we should show
	//	the icon at startup and the task should be 'on.'  After we've read it,
	//	release it.  The control panel will reload it.  If we cannot read the
	//	resource from the file (probably because it doesn't yet exist), just
	//	assume true for on/off and show icon and use the default modifier keys.
	//  The control panel will create a new preferences resource if need be.
 
	 	myData->CPon = true;	/* preset to 'on' unless we find otherwise */
	 						/* installTask was also preset to true above */
	 	
	 	/* This sample captures mouse clicks when these modifiers are down */
	 	
	 	myData->CPmodifiers = controlKey + optionKey + shiftKey;
	 									/* preset modifiers to default */
	 	
	 	prefsHandle = NULL;
	 	
	 	prefsHandle = (CPprefsHandle)Get1Resource( kCPprefsRsrcType,
	 													kCPprefsRsrcID );
	 	if ( prefsHandle )
	 	{
	 		if ( !(*prefsHandle)->On )	// control panel set to off?
	 		{
	 			*installTask = false;
	 			myData->CPon = false;
	 		}
	 		
	 		if ( !(*prefsHandle)->ShowIcon )	// show icon?
	 			*showIcon = false;
	 		
	 		myData->CPmodifiers = (*prefsHandle)->modifiers;
	 			
	 		ReleaseResource( (Handle)prefsHandle );
	 	}

	/* If any keys are down, do not load (this means ANY key) */
	
		{
			KeyMap	theKeys;
			
			GetKeys( theKeys );
			if ( theKeys[0] || theKeys[1] || theKeys[2] || theKeys[3] )
				stillOK = false;
		}
	
	 return stillOK;
}
