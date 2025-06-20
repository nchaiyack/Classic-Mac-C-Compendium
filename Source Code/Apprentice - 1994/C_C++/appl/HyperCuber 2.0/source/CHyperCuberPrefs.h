//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberPrefs.h
//|
//| This is the interface to the HyperCuber preferences.
//|_________________________________________________________

#pragma once
#include "CPrefs.h"
#include "Keys.h"
#include "Mouse.h"

#define MAX_DIMENSION		20
#define MAX_MOUSE_CONTROLS	20
#define MAX_KEY_CONTROLS	100

typedef struct
	{
	short	modifiers;				//  Modifier keys for this mouse control
	short	dimension_h;			//  Dimension of the angle to change when mouse moves horizontally
	short	angle_h;				//  Number of the angle to change when mouse moves horizontally
	Boolean	reverse_h;				//  True if angle should be DECREASED for positive mouse movement
	short	dimension_v;			//  Dimension of the angle to change when mouse moves vertically
	short	angle_v;				//  Number of the angle to change when mouse moves vertically
	Boolean	reverse_v;				//  True if angle should be DECREASED for positive mouse movement
	} MouseCStruct;
	
typedef struct
	{
	short	modifiers;				//  Modifier keys for this key control
	char	key_code;				//  Key code the the key
	short	dimension;				//  Dimension of the angle this key control changes
	short	angle;					//  Number of the angle this key control changes
	short	increment;				//  Amount to change the angle
	} KeyCStruct;
	
	
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
	mouse_control_struct	mouse_controls[MAX_MOUSE_CONTROLS];
	
	short					num_key_controls;
	key_control_struct		key_controls[MAX_KEY_CONTROLS];
		
	} PrefsStruct;

#define CURRENT_PREFS_VERSION	12

CLASS CWindow;

class CHyperCuberPrefs : public CPrefs
	{
	
	short	*GetPrefsPointer(void);
	void	Update(void);

  public:

	PrefsStruct	prefs;

	void	IHyperCuberPrefs(void);
	void	SetDefaults(void);
	void	SetDefaultKeyCommands(void);
	void	SetDefaultMouseCommands(void);

	};
