/*****************************************************************************\
* macequations.c                                                              *
*                                                                             *
* This file contains code which is specific to the Macintosh.  It contains    *
* code which interfaces with the equation interpreter.                        *
\*****************************************************************************/

#include "defs.h"
#include "extern.h"
#include "rtresources.h"
#include "interp_equation.h"

/* The equations */

char	x_up_eq[255];
char	y_up_eq[255];
char	z_up_eq[255];
char	x_look_eq[255];
char	y_look_eq[255];
char	z_look_eq[255];
char	x_eye_eq[255];
char	y_eye_eq[255];
char	z_eye_eq[255];
char	x_angle_eq[255];
char	y_angle_eq[255];


/* Externals */
extern DialogPtr	animation_dialog;		/* the animation dialog */
extern real			t;						/* the current value of t */

/* Protypes */
Boolean find_variable_value (char *var_name, real *value);
void setup_frame(void);
void get_animation_info(real *tstart, real *tend, long *frames);



/*****************************************************************************\
* procedure get_animation_info                                                *
*                                                                             *
* Purpose: This procedure gets the animation parameters from the animation    *
*          dialog.                                                            *
*                                                                             *
* Parameters: tstart: receives the starting t value                           *
*             tend:   receives the ending t value                             *
*             frames: receives the number of frames	                          *
*             the global equation variables are also set.                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 9, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void get_animation_info(real *tstart, real *tend, long *frames)
{

	short	type;					/* the item type of a DITL item */
	Handle	handle;					/* the handle to a DITL item */
	Rect	box;					/* the bounding box of a DITL item */
	Str255	text;					/* the text of a DITL item */
	short	error;					/* error code of detected error */
	static char	error_message[200];	/* error message of detected error */

	/* Get the starting t value */
	GetDItem (animation_dialog, T_RANGES_FROM_EQ, &type, &handle, &box);
	GetIText (handle, text);
	if (error = evaluate_equation(PtoCstr(text), tstart, find_variable_value))
		{
		strcpy(error_message, "Error in starting t value: ");
		strcat(error_message, get_error_message(error));
		abortive_string_error(error_message);
		}

	/* Get the ending t value */
	GetDItem (animation_dialog, T_RANGES_TO_EQ, &type, &handle, &box);
	GetIText (handle, text);
	if (error = evaluate_equation(PtoCstr(text), tend, find_variable_value))
		{
		strcpy(error_message, "Error in starting t value: ");
		strcat(error_message, get_error_message(error));
		abortive_string_error(error_message);
		}

	/* Get the number of frames */
	GetDItem (animation_dialog, NUM_FRAMES_NUM, &type, &handle, &box);
	GetIText (handle, text);
	StringToNum (text, frames);

	/* Get the x eye equation */
	GetDItem (animation_dialog, X_EYE_EQ, &type, &handle, &box);
	GetIText (handle, x_eye_eq);
	PtoCstr(x_eye_eq);

	/* Get the y eye equation */
	GetDItem (animation_dialog, Y_EYE_EQ, &type, &handle, &box);
	GetIText (handle, y_eye_eq);
	PtoCstr(y_eye_eq);

	/* Get the z eye equation */
	GetDItem (animation_dialog, Z_EYE_EQ, &type, &handle, &box);
	GetIText (handle, z_eye_eq);
	PtoCstr(z_eye_eq);

	/* Get the x look equation */
	GetDItem (animation_dialog, X_LOOK_EQ, &type, &handle, &box);
	GetIText (handle, x_look_eq);
	PtoCstr(x_look_eq);

	/* Get the y look equation */
	GetDItem (animation_dialog, Y_LOOK_EQ, &type, &handle, &box);
	GetIText (handle, y_look_eq);
	PtoCstr(y_look_eq);

	/* Get the z look equation */
	GetDItem (animation_dialog, Z_LOOK_EQ, &type, &handle, &box);
	GetIText (handle, z_look_eq);
	PtoCstr(z_look_eq);

	/* Get the x up equation */
	GetDItem (animation_dialog, X_UP_EQ, &type, &handle, &box);
	GetIText (handle, x_up_eq);
	PtoCstr(x_up_eq);

	/* Get the y up equation */
	GetDItem (animation_dialog, Y_UP_EQ, &type, &handle, &box);
	GetIText (handle, y_up_eq);
	PtoCstr(y_up_eq);

	/* Get the z up equation */
	GetDItem (animation_dialog, Z_UP_EQ, &type, &handle, &box);
	GetIText (handle, z_up_eq);
	PtoCstr(z_up_eq);

	/* Get the x angle equation */
	GetDItem (animation_dialog, ANGLE_X_EQ, &type, &handle, &box);
	GetIText (handle, x_angle_eq);
	PtoCstr(x_angle_eq);

	/* Get the y angle equation */
	GetDItem (animation_dialog, ANGLE_Y_EQ, &type, &handle, &box);
	GetIText (handle, y_angle_eq);
	PtoCstr(y_angle_eq);

}	/* get_animation_info() */



/*****************************************************************************\
* procedure setup_frame                                                       *
*                                                                             *
* Purpose: This procedure sets up the eye and view points, and the up vector, *
*          using the current value of t.                                      *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 11, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

void setup_frame(void)
{

	char	error_message[100];
	short	error;

	/* Find the x eye value, check for errors */
	if (error = evaluate_equation(x_eye_eq, &eye.x, find_variable_value))
		{
		strcpy(error_message, "Error evaluating eye x equation: ");
		strcat(error_message, get_error_message(error));
		abortive_string_error(error_message);
		}
	
	/* Find the y eye value, check for errors */
	if (error = evaluate_equation(y_eye_eq, &eye.y, find_variable_value))
		{
		strcpy(error_message, "Error evaluating eye y equation: ");
		strcat(error_message, get_error_message(error));
		abortive_string_error(error_message);
		}
		
	/* Find the z eye value, check for errors */
	if (error = evaluate_equation(z_eye_eq, &eye.z, find_variable_value))
		{
		strcpy(error_message, "Error evaluating eye z equation: ");
		strcat(error_message, get_error_message(error));
		abortive_string_error(error_message);
		}

	/* Find the x look value, check for errors */
	if (error = evaluate_equation(x_look_eq, &look.x, find_variable_value))
		{
		strcpy(error_message, "Error evaluating look x equation: ");
		strcat(error_message, get_error_message(error));
		abortive_string_error(error_message);
		}
		
	/* Find the y look value, check for errors */
	if (error = evaluate_equation(y_look_eq, &look.y, find_variable_value))
		{
		strcpy(error_message, "Error evaluating look y equation: ");
		strcat(error_message, get_error_message(error));
		abortive_string_error(error_message);
		}
		
	/* Find the z look value, check for errors */
	if (error = evaluate_equation(z_look_eq, &look.z, find_variable_value))
		{
		strcpy(error_message, "Error evaluating look z equation: ");
		strcat(error_message, get_error_message(error));
		abortive_string_error(error_message);
		}

	/* Find the x up value, check for errors */
	if (error = evaluate_equation(x_up_eq, &up.x, find_variable_value))
		{
		strcpy(error_message, "Error evaluating up x equation: ");
		strcat(error_message, get_error_message(error));
		abortive_string_error(error_message);
		}
		
	/* Find the y up value, check for errors */
	if (error = evaluate_equation(y_up_eq, &up.y, find_variable_value))
		{
		strcpy(error_message, "Error evaluating up y equation: ");
		strcat(error_message, get_error_message(error));
		abortive_string_error(error_message);
		}
		
	/* Find the z up value, check for errors */
	if (error = evaluate_equation(z_up_eq, &up.z, find_variable_value))
		{
		strcpy(error_message, "Error evaluating up z equation: ");
		strcat(error_message, get_error_message(error));
		abortive_string_error(error_message);
		}

	/* Find the x angle, check for errors */
	if (error = evaluate_equation(x_angle_eq, &view_angle_x, find_variable_value))
		{
		strcpy(error_message, "Error evaluating x angle equation: ");
		strcat(error_message, get_error_message(error));
		abortive_string_error(error_message);
		}
	view_angle_x = DEGREE_TO_RADIAN(view_angle_x);
		
	/* Find the y angle, check for errors */
	if (error = evaluate_equation(y_angle_eq, &view_angle_y, find_variable_value))
		{
		strcpy(error_message, "Error evaluating y angle equation: ");
		strcat(error_message, get_error_message(error));
		abortive_string_error(error_message);
		}
	view_angle_y = DEGREE_TO_RADIAN(view_angle_y);

	/* Recomputer the gaze for this new eye point */
	gaze.x = look.x - eye.x;
	gaze.y = look.y - eye.y;
	gaze.z = look.z - eye.z;
	gaze_distance = LENGTH(gaze);
	
	/* If we're right on top of the look point, it's an error */
	if (gaze_distance < ROUNDOFF)
		{
		sprintf(error_message, "Eye point equal to look point when t = %lg", t);
		abortive_string_error(error_message);
		}
		
	/* Otherwise, we can normalize the gaze */
	NORMALIZE(gaze);

	/* If the up vector is null, it's an error */
	if (LENGTH(up) < ROUNDOFF)
		{
		sprintf(error_message, "Up vector is zero when t = %lg", t);
		abortive_string_error(error_message);
		}
	
	/* Otherwise, we can normalize the up vector */
	NORMALIZE(up);
		
	/* check for bad up vector */
	if (ABS(DOT_PRODUCT(gaze, up)) > COS(ANGLE_MIN))
		{
		sprintf(error_message, "Bad up vector when t = %lg", t);
		abortive_string_error(error_message);
		}
				
}	/* setup_frame() */



/*****************************************************************************\
* procedure find_variable_value                                               *
*                                                                             *
* Purpose: This procedure is called when the equation interpreter wants to    *
*          know the value of a variable.                                      *
*                                                                             *                                                                             *
* Parameters: var_name: the name of the variable.                             *                                                                             *
*             value:    receives the valud of the variable.                   *                                                                             *
*             returns TRUE if variable is undefined.                          *                                                                             *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 9, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

Boolean find_variable_value (char *var_name, real *value)
{

	/* We only know t.  If it's t, use the current value.  Otherwise, it's
		an error */
	if (!strcmp(var_name, "t"))
		{
		*value = t;
		return FALSE;
		}
	else
		return TRUE;

}