// ===========================================================================
//	3DobClasses.cp	  ©1995 J. Rodden, DD/MF & Associates. All rights reserved
// ===========================================================================
//
//	The RegisterAll3DClasses() function calls URegistrar::RegisterClass()
//	for every 3D Additions class that can create objects from a PPob resource.
//
//	If you call this function, you will have to include all 3D Additions
//	classes in your project. Furthermore, every class will be "used"
//	as far as the Linker is concerned, meaning that the code for all those
//	classes will be included in the final program even if you never create
//	an instance of a particular class.
//
//	Therefore, you may want to only register the classes that you do use.
//	Copy the necessary calls from below and paste them into the contructor
//	for your application class.

#include "3DobClasses.h"

#include "3DPanes.h"
#include "3DTextFields.h"
#include "LStdCDEFControl.h"

#include <URegistrar.h>

// ---------------------------------------------------------------------------
//		¥ RegisterAll3DClasses
// ---------------------------------------------------------------------------
//	Register all 3D Additions classes that can create objects from Stream data
//	Call this routine AFTER RegisterAllPPClasses in order to override PP controls.

void
RegisterAll3DClasses()
{
#define RegisterClass(C)	URegistrar::RegisterClass(C::class_ID, C::CreateFromStream);

	// Use application wide color settings
	RegisterClass(C3DPanel);
	RegisterClass(C3DFrame);
	RegisterClass(C3DCaption);
	RegisterClass(C3DEditField);

	RegisterClass(LStdCDEFButton);
	RegisterClass(LStdCDEFCheckBox);
	RegisterClass(LStdCDEFRadioButton);

	// Individual color settings
	RegisterClass(CThreeDPanel);
	RegisterClass(CThreeDFrame);

#undef RegisterClass

	// To minimize user hassle
	URegistrar::RegisterClass('pbut', LStdCDEFButton::CreateFromStream);
	URegistrar::RegisterClass('cbox', LStdCDEFCheckBox::CreateFromStream);
	URegistrar::RegisterClass('rbut', LStdCDEFRadioButton::CreateFromStream);
}

