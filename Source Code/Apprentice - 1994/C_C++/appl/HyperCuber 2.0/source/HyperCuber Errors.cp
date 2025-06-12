//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| HyperCuber Errors.cp
//|
//| This file contains error-handling code
//|___________________________________________________________________________

#include "HyperCuber Errors.h"


//============================ Prototypes ============================\\

void terminal_error(short error);
void general_error(short error);



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure: terminal_error
//|
//| Purpose: Display error dialog and exit.
//|
//| Parameters: error: the error code
//|__________________________________________________

void terminal_error(short error)
{

	Str255 error_string;
	GetIndString(error_string, TERMINAL_ERRORS_STRN_ID, error);	//  Set up the error text

	ParamText(error_string, NULL, NULL, NULL);					//  Display error alert

	short item = Alert(TERMINAL_ERROR_ALERT_ID, NULL);

	ExitToShell();												//  Exit

}	//==== terminal_error() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure: general_error
//|
//| Purpose: Display error dialog.
//|
//| Parameters: error: the error code
//|__________________________________________________

void general_error(short error)
{

	Str255 error_string;
	GetIndString(error_string, GENERAL_ERRORS_STRN_ID, error);	//  Set up the error text

	ParamText(error_string, NULL, NULL, NULL);					//  Display error alert

	short item = Alert(GENERAL_ERROR_ALERT_ID, NULL);

}	//==== general_error() ====\\



