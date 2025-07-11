/******************************************************************************
 CAuthentication.h

		Interface for the AOCE Authentication Class
		
		Copyright � 1993 Marty Wachter. All rights reserved.
		

******************************************************************************/

#pragma once

#define		kAuthenticationStrs		3000
#define		kAOCENotInstalled		1
#define		kNotAuthorizedUser		2

#include "CObject.h"

#include <OCEAuthDir.h>
#include <OCE.h>
#include <OCEErrors.h>
#include <OCEStandardDirectory.h>

class CAuthentication : public CObject {

public:
	//Instance Variables
	
	AuthIdentity 			userIdentity;
	Str255			 		guestPrompt;
	Str255 					specificPrompt;
	Str255			 		localPrompt;
	Boolean					allowGuest;
	Boolean					allowSpecific;
	Boolean					allowLocal;
	Boolean					hasOCE;
	OSErr					status;
	SDPIdentityKind			selectedKind;
	
	//Instance Methods
	void				IAuthentication(ConstStr255Param 		aGuestPrompt,
										ConstStr255Param 		aSpecificIDPrompt,
										ConstStr255Param 		aLocalIDPrompt,
										Boolean					aAllowGuest,
										Boolean					aAllowSpecific,
										Boolean					aAllowLocal);
										
	virtual void		GetUserID(void);

};