/******************************************************************************
 CAuthentication.cp

							The PowerTalk Authentication Class
								
	SUPERCLASS = CObject
	
	Copyright � 1993 Marty Wachter. All rights reserved.
	
	Portions Copyright � 1993 Apple Computer, Inc.  All rights reserved.
	Portions Copyright � 1989 Symantec Corporation. All rights reserved.

	Original Author: 	Martin R. Wachter		email:	mrw@welchgate.welch.jhu.edu
	Created:			8/4/93					by:		mrw			TCL Version:	1.1.3
	Modified:			4/6/94					by:		mrw			TCL Version:	1.1.3


	Version change history:
	
	1.0		Initial release.


Sample Usage.  Place in your CApplication subclass.  Don't forget to include 
			   the STR# resources in your project!
    
// PowerTalk User Prompts
#define 	kGuest		"\pLog in as a Guest..."
#define 	kSpecific	"\pPlease enter your User Name and Password for access"
#define		kLocal		"\pPlease enter your PowerTalk Access Code"
    
    
    ***sample usage***
    
	itsAuth = new (CAuthentication);
	
	itsAuth->IAuthentication(kGuest,kSpecific,kLocal,TRUE,TRUE,TRUE);
	
	if (itsAuth->hasOCE)
		itsAuth->GetUserID();
	
	if (itsAuth->status)
		DoCommand(cmdQuit);
		
	******************

******************************************************************************/

#include "CAuthentication.h"

#include "TBUtilities.h"
#include <GestaltEqu.h>

extern 	CError		*gError;						//Global TCL Error handler

/******************************************************************************
 IAuthentication

		Initialize an AOCE object
 ******************************************************************************/

void	CAuthentication::IAuthentication(ConstStr255Param 		aGuestPrompt,
										 ConstStr255Param 		aSpecificPrompt,
										 ConstStr255Param 		aLocalPrompt,
										 Boolean				aAllowGuest,
										 Boolean				aAllowSpecific,
										 Boolean				aAllowLocal)
{

long					gestaltResponse;

	hasOCE = FALSE;// assume AOCE is not present yet
	status = noErr;// assume noErr yet
	
	// Initialize the authentication prompts with TCL string routines
	CopyPString(aGuestPrompt, guestPrompt);
	CopyPString(aSpecificPrompt, specificPrompt);
	CopyPString(aLocalPrompt, localPrompt);
		
	allowGuest = 	aAllowGuest;
	allowSpecific = aAllowSpecific;
	allowLocal = 	aAllowLocal;
	
	//  Make sure this Macintosh supports AOCE by means of the Gestalt Mgr.
	status = Gestalt(gestaltOCEToolboxAttr, &gestaltResponse);
		
	switch (status) {
		case noErr:
			hasOCE = TRUE;
			break;
		case kOCEToolboxNotOpen:
			hasOCE = TRUE;
			break;
		case gestaltUndefSelectorErr:
			gError->PostAlert(kAuthenticationStrs, kAOCENotInstalled);
			hasOCE = FALSE;
			break;
		default:
			gError->PostAlert(kAuthenticationStrs, kAOCENotInstalled);
			hasOCE = FALSE;
			break;
	}
	
}


/******************************************************************************
 GetUserID

		Get the user authentication ID from the PowerTalk Manager.
		
 ******************************************************************************/

void	CAuthentication::GetUserID(void)
{
AuthParamBlock			authParamBlock;
SDPIdentityKind			allowedKind = 0;


		//
		// OCE Setup: get the user's local authentication identity. If it's
		// already set, we can do this silently. If it's not set (the system
		// has just been started), we'll prompt for the user's identity
		// and password.
		//
		// The only expected error is "Cancel" from SDPPromptForIdentity.
		//
		 
		status = AuthGetLocalIdentity(
				&authParamBlock,			// Parameter block
				FALSE						// Synchronous
			);
		
		if (status == noErr) {
			
		// The user identity has already been specified.
			 
			userIdentity = authParamBlock.getLocalIdentityPB.theLocalIdentity;
		}

		else if (status == kOCELocalAuthenticationFail) {
		
		// No user identity has been specified. Ask for the user to specify
		// the local identity.
		 
			if (allowGuest) 
				allowedKind = ( allowedKind | kSDPGuestMask);
			if (allowSpecific)
				allowedKind = ( allowedKind | kSDPSpecificIdentityMask);
			if (allowLocal)
				allowedKind = ( allowedKind | kSDPLocalIdentityMask);
			
			status = SDPPromptForID(
					&userIdentity,			// AuthIdentity id
					guestPrompt,			// Default guest prompt
					specificPrompt,			// Default specific prompt
					localPrompt,			// Default local prompt
					NULL,					// RString			*recordType
					allowedKind,			// SDPIdentityKind	permittedKinds
					&selectedKind,			// SDPIdentityKind	*selectedKind
					NULL,					// RecordID			*loginFilter
					0						// SDPLoginFilterKind filterKind						
				);
			if (status != noErr){
				gError->PostAlert(kAuthenticationStrs, kNotAuthorizedUser);
			}
		}
}