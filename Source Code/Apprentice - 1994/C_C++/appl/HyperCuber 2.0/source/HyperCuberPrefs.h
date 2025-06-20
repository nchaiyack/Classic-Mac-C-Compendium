#pragma once

typedef struct
	{
	short		prefs_version;
	RGBColor	background_color;
	RGBColor	object_color;
	RGBColor	axes_3D_color;
	RGBColor	axes_4D_color;
	RGBColor	left_eye_color;
	RGBColor	right_eye_color;
	RGBColor	positive_x_color;
	RGBColor	negative_x_color;
	RGBColor	intermediate_x_color;
	Boolean		perspective_3D;
	Boolean		perspective_4D;
	Boolean		axes_3D;
	Boolean		axes_4D;
	Boolean		stereo;
	Boolean		two_image;
	Rect		graphics_window_position;
	Rect		controls_window_position;
	Boolean		controls_window_visible;
	} PrefsStruct;

#define CURRENT_PREFS_VERSION	5
