//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberPrefs.cp
//|
//| This implements the HyperCuber preferences.
//|_________________________________________________________

#include "CHyperCuberPrefs.h"
#include "CControlsDirector.h"

#include <string.h>


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberPrefs::IHyperCuberPrefs
//|
//| Purpose: Initialize the preferences and set the to their default values
//|
//| Parameters: none
//|_________________________________________________________________________

void CHyperCuberPrefs::IHyperCuberPrefs(void)
{

	CPrefs::IPrefs("\pHyperCuber Prefs",			//  Initialize the superclass
					sizeof(PrefsStruct),
					CURRENT_PREFS_VERSION);

	SetDefaults();									//  Set all preferences to default values
	
}	//==== CHyperCuberPrefs::IHyperCuberPrefs() ====\\
	


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberPrefs::SetDefaults
//|
//| Purpose: This sets all preferences to their default values.
//|_________________________________________________________________________

void CHyperCuberPrefs::SetDefaults(void)
{
	
	prefs.prefs_version = CURRENT_PREFS_VERSION;	//  Set the version of these preferences

	prefs.background_color.red = 0x0000;			//  Initialize the background color
	prefs.background_color.green = 0x0000;
	prefs.background_color.blue = 0x0000;
	
	prefs.left_eye_color.red = 0x0000;				//  Initialize the left eye color
	prefs.left_eye_color.green = 0x7FF0;
	prefs.left_eye_color.blue = 0x0000;
	
	prefs.right_eye_color.red = 0xFFFF;				//  Initialize the right eye color
	prefs.right_eye_color.green = 0x0000;
	prefs.right_eye_color.blue = 0x0000;

	prefs.stereo = FALSE;							//  Don't view in stereo
	prefs.two_image = TRUE;							//  If view in stereo, use two-image stereo
													//    (otherwise, use two-color stereo)
	prefs.antialias = FALSE;						//  Don't antialias line segments
	
	Rect rect = {42, 4, 242, 204};					//  Set default graphics window
	prefs.graphics_window_position = rect;

	long controls_window_top = 34;					//  The first controls window is at the top
	
	long i;
	for (i = 3; i <= MAX_DIMENSION; i++)
		{
		
		Rect window_rect;
		window_rect.top = controls_window_top;		//  Place the controls window
		window_rect.left = 210;
		window_rect.bottom = controls_window_top + i*20 + 5;
		window_rect.right = 4+300;

		prefs.controls_window_position[i] =
									window_rect;	//  Save this in the prefs
		
		controls_window_top += 20*i + 20;			//  Point to next window top
		
		prefs.controls_window_visible[i] = TRUE;	//  Controls are visible by default
		
		}
	
	prefs.num_parameters = 0;
	
	Parameter param;
	strcpy(param.name, "alpha");					//  Set default parameter alpha: [0, 3.1415] step 0.6283
	param.start = 0;
	param.end = 3.1415;
	param.step = 0.6283;
	prefs.parameters[prefs.num_parameters++] = param;
	
	strcpy(param.name, "beta");						//  Set default parameter beta: [-1.5708, 1.5708] step 0.6283
	param.start = -1.5708;
	param.end = 1.5708;
	param.step = 0.6283;
	prefs.parameters[prefs.num_parameters++] = param;
	
	strcpy(param.name, "gamma");					//  Set default parameter gamma: [0, 1.5708] step 0.31415
	param.start = 0;
	param.end = 1.5708;
	param.step = 0.31415;
	prefs.parameters[prefs.num_parameters++] = param;
	
	prefs.equation_dimension = 4;					//  Plot four-dimensional equation
	
	strcpy(prefs.functions[0], "sin(alpha)*cos(beta)*sin(gamma)");		//  Set up functions
	strcpy(prefs.functions[1], "cos(alpha)*cos(beta)*cos(gamma)");
	strcpy(prefs.functions[2], "cos(alpha)*sin(beta)");
	strcpy(prefs.functions[3], "sin(alpha)");
	
	SetDefaultKeyCommands();						//  Set up the default key commands
	SetDefaultMouseCommands();						//  Set up the default mouse commands

}	//==== CHyperCuberPrefs::SetDefaults() ====\\
	


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberPrefs::SetDefaultKeyCommands
//|
//| Purpose: This sets all key commands to their default values.
//|_________________________________________________________________________

void CHyperCuberPrefs::SetDefaultKeyCommands(void)
{

	long i = 0;
	KeyControl key;

	key.dimension = 3;					//  Set right arrow to increase 3D angle 1 (longitude)
	key.angle = 1;
	key.increment = 1;
	key.key_code = KeyRightCursor;
	key.modifiers = 0;
	prefs.key_controls[i++] = key;

	key.dimension = 3;					//  Set shift-right arrow to increase 3D angle 1 (longitude) by 10
	key.angle = 1;
	key.increment = 10;
	key.key_code = KeyRightCursor;
	key.modifiers = shiftKey;
	prefs.key_controls[i++] = key;

	key.dimension = 3;					//  Set left arrow to decrease 3D angle 1 (longitude)
	key.angle = 1;
	key.increment = -1;
	key.key_code = KeyLeftCursor;
	key.modifiers = 0;
	prefs.key_controls[i++] = key;

	key.dimension = 3;					//  Set shift-left arrow to decrease 3D angle 1 (longitude) by 10
	key.angle = 1;
	key.increment = -10;
	key.key_code = KeyLeftCursor;
	key.modifiers = shiftKey;
	prefs.key_controls[i++] = key;

	key.dimension = 3;					//  Set up arrow to increase 3D angle 2 (lattitude)
	key.angle = 2;
	key.increment = 1;
	key.key_code = KeyUpCursor;
	key.modifiers = 0;
	prefs.key_controls[i++] = key;

	key.dimension = 3;					//  Set shift-up arrow to increase 3D angle 2 (lattitude) by 10
	key.angle = 2;
	key.increment = 10;
	key.key_code = KeyUpCursor;
	key.modifiers = shiftKey;
	prefs.key_controls[i++] = key;

	key.dimension = 3;					//  Set down arrow to decrease 3D angle 2 (lattitude)
	key.angle = 2;
	key.increment = -1;
	key.key_code = KeyDownCursor;
	key.modifiers = 0;
	prefs.key_controls[i++] = key;

	key.dimension = 3;					//  Set shift-down arrow to decrease 3D angle 2 (lattitude) by 10
	key.angle = 2;
	key.increment = -10;
	key.key_code = KeyDownCursor;
	key.modifiers = shiftKey;
	prefs.key_controls[i++] = key;

	key.dimension = 3;					//  Set / to increase 3D perspective
	key.angle = 0;
	key.increment = 1;
	key.key_code = 0x2C;
	key.modifiers = 0;
	prefs.key_controls[i++] = key;

	key.dimension = 3;					//  Set . to decrease 3D perspective
	key.angle = 0;
	key.increment = -1;
	key.key_code = 0x2F;
	key.modifiers = 0;
	prefs.key_controls[i++] = key;

	key.dimension = 4;					//  Set keypad 9 to increase 4D angle 1
	key.angle = 1;
	key.increment = 1;
	key.key_code = KeyPad9;
	key.modifiers = 0;
	prefs.key_controls[i++] = key;

	key.dimension = 4;					//  Set shift-keypad 9 to increase 4D angle 1 by 10
	key.angle = 1;
	key.increment = 10;
	key.key_code = KeyPad9;
	key.modifiers = shiftKey;
	prefs.key_controls[i++] = key;

	key.dimension = 4;					//  Set keypad 7 to decrease 4D angle 1
	key.angle = 1;
	key.increment = -1;
	key.key_code = KeyPad7;
	key.modifiers = 0;
	prefs.key_controls[i++] = key;

	key.dimension = 4;					//  Set shift-keypad 7 to decrease 4D angle 1 by 10
	key.angle = 1;
	key.increment = -10;
	key.key_code = KeyPad7;
	key.modifiers = shiftKey;
	prefs.key_controls[i++] = key;

	key.dimension = 4;					//  Set keypad 6 to increase 4D angle 2
	key.angle = 2;
	key.increment = 1;
	key.key_code = KeyPad6;
	key.modifiers = 0;
	prefs.key_controls[i++] = key;

	key.dimension = 4;					//  Set shift-keypad 6 to increase 4D angle 2 by 10
	key.angle = 2;
	key.increment = 10;
	key.key_code = KeyPad6;
	key.modifiers = shiftKey;
	prefs.key_controls[i++] = key;

	key.dimension = 4;					//  Set keypad 4 to decrease 4D angle 2
	key.angle = 2;
	key.increment = -1;
	key.key_code = KeyPad4;
	key.modifiers = 0;
	prefs.key_controls[i++] = key;

	key.dimension = 4;					//  Set shift-keypad 4 to decrease 4D angle 2 by 10
	key.angle = 2;
	key.increment = -10;
	key.key_code = KeyPad4;
	key.modifiers = shiftKey;
	prefs.key_controls[i++] = key;

	key.dimension = 4;					//  Set keypad 3 to increase 4D angle 3
	key.angle = 3;
	key.increment = 1;
	key.key_code = KeyPad3;
	key.modifiers = 0;
	prefs.key_controls[i++] = key;

	key.dimension = 4;					//  Set shift-keypad 3 to increase 4D angle 3 by 10
	key.angle = 3;
	key.increment = 10;
	key.key_code = KeyPad3;
	key.modifiers = shiftKey;
	prefs.key_controls[i++] = key;

	key.dimension = 4;					//  Set keypad 1 to decrease 4D angle 3
	key.angle = 3;
	key.increment = -1;
	key.key_code = KeyPad1;
	key.modifiers = 0;
	prefs.key_controls[i++] = key;

	key.dimension = 4;					//  Set shift-keypad 1 to decrease 4D angle 3 by 10
	key.angle = 3;
	key.increment = -10;
	key.key_code = KeyPad1;
	key.modifiers = shiftKey;
	prefs.key_controls[i++] = key;

	key.dimension = 4;					//  Set keypad + to increase 4D perspective
	key.angle = 0;
	key.increment = 1;
	key.key_code = KeyPadPlus;
	key.modifiers = 0;
	prefs.key_controls[i++] = key;

	key.dimension = 4;					//  Set keypad - to decrease 4D perspective
	key.angle = 0;
	key.increment = -1;
	key.key_code = KeyPadMinus;
	key.modifiers = 0;
	prefs.key_controls[i++] = key;

	prefs.num_key_controls = i;

}	//==== CHyperCuberPrefs::SetDefaultKeyCommands() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberPrefs::SetDefaultMouseCommands
//|
//| Purpose: This sets all mouse commands to their default values.
//|_________________________________________________________________________

void CHyperCuberPrefs::SetDefaultMouseCommands(void)
{

	long i = 0;
	MouseControl mouse_control;

	mouse_control.dimension = 3;				
	mouse_control.angle = 1;
	mouse_control.horiz = TRUE;
	mouse_control.multiplier = -1;
	mouse_control.modifiers = 0;
	prefs.mouse_controls[i++] = mouse_control;

	mouse_control.dimension = 3;				
	mouse_control.angle = 2;
	mouse_control.horiz = FALSE;
	mouse_control.multiplier = -1;
	mouse_control.modifiers = 0;
	prefs.mouse_controls[i++] = mouse_control;

	mouse_control.dimension = 4;				
	mouse_control.angle = 1;
	mouse_control.horiz = TRUE;
	mouse_control.multiplier = 1;
	mouse_control.modifiers = shiftKey;
	prefs.mouse_controls[i++] = mouse_control;

	mouse_control.dimension = 4;				
	mouse_control.angle = 2;
	mouse_control.horiz = FALSE;
	mouse_control.multiplier = 1;
	mouse_control.modifiers = shiftKey;
	prefs.mouse_controls[i++] = mouse_control;

	mouse_control.dimension = 4;				
	mouse_control.angle = 2;
	mouse_control.horiz = TRUE;
	mouse_control.multiplier = 1;
	mouse_control.modifiers = optionKey;
	prefs.mouse_controls[i++] = mouse_control;

	mouse_control.dimension = 4;				
	mouse_control.angle = 3;
	mouse_control.horiz = FALSE;
	mouse_control.multiplier = 1;
	mouse_control.modifiers = optionKey;
	prefs.mouse_controls[i++] = mouse_control;

	prefs.num_mouse_controls = i;

}	//==== CHyperCuberPrefs::SetDefaultMouseCommands() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberPrefs::GetPrefsPointer
//|
//| Purpose: This returns a pointer to the preferences structure.
//|
//| Parameters: returns pointer to prefs
//|_________________________________________________________________________

short *CHyperCuberPrefs::GetPrefsPointer(void)
{

	return ((short *) &prefs);			//  Return pointer to prefs structure

}	//==== CHyperCuberPrefs::GetPrefsPointer() ====\\
	


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberPrefs::Update
//|
//| Purpose: Update the preferences to match the settings being used.
//|
//| Parameters: none
//|_____________________________________________________________________________

void CHyperCuberPrefs::Update(void)
{

	Rect		window_position;
	Rect		zoomed_window_position;

	//  The prefs structure keeps the current values of the preferences
	//  for the various switches, so no updating is necessary.

//	long i;
//	for (i = 3; i <= dimension; i++)		//  Find bounds of controls windows
//		{
//		get_window_rect(
//				((CControlsDirector *) controls_directors->NthItem(i))->itsWindow,	
//					&prefs.controls_window_position[i]);
//		}

}	//==== CHyperCuberPrefs::Update() ====\\



