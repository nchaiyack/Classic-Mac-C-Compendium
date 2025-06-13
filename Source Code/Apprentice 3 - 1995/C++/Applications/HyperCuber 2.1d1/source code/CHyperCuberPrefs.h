//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberPrefs.h
//|
//| This is the interface to the HyperCuber preferences.
//|_________________________________________________________

#pragma once
#include "CPrefs.h"
#include "Keys.h"
#include "Mouse.h"
#include "Parameter.h"

#define MAX_DIMENSION		20
#define MAX_MOUSE_CONTROLS	20
#define MAX_KEY_CONTROLS	100
#define MAX_PARAMETERS		100
#define MAX_FUNCTION_LENGTH	200

typedef struct								//  The preferences structure
	{
	short		prefs_version;
	RGBColor	background_color;
	RGBColor	left_eye_color;
	RGBColor	right_eye_color;
	Boolean		stereo;
	Boolean		two_image;
	Boolean		antialias;
	Rect		graphics_window_position;
	
	Rect		controls_window_position[MAX_DIMENSION+1];
	Boolean		controls_window_visible[MAX_DIMENSION+1];
	
	short					num_mouse_controls;
	MouseControl			mouse_controls[MAX_MOUSE_CONTROLS];
	
	short					num_key_controls;
	KeyControl				key_controls[MAX_KEY_CONTROLS];
		
	short					num_parameters;
	Parameter				parameters[MAX_PARAMETERS];
	
	long					equation_dimension;
	char					functions[MAX_DIMENSION][MAX_FUNCTION_LENGTH];
	
	} PrefsStruct;

#define CURRENT_PREFS_VERSION	13

class CWindow;

class CHyperCuberPrefs : public CPrefs
	{

  protected:

	short	*GetPrefsPointer(void);

  public:

	PrefsStruct	prefs;

	void	IHyperCuberPrefs(void);
	void	SetDefaults(void);
	void	SetDefaultKeyCommands(void);
	void	SetDefaultMouseCommands(void);

	void	Update(void);

	};
