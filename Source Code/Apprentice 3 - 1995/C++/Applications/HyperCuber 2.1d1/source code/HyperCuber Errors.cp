//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| HyperCuber Errors.cp
//|
//| This file contains error-handling code
//|___________________________________________________________________________

#include "HyperCuber Errors.h"


#include <string.h>


//============================ Prototypes ============================\\

void TerminalError(short error);
void GeneralError(short error);
void GeneralErrorByString(char *error_string);



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure: TerminalError
//|
//| Purpose: Display error dialog and exit.
//|
//| Parameters: error: the error code
//|__________________________________________________

void TerminalError(short error)
{

	Str255 error_string;
	GetIndString(error_string, TERMINAL_ERRORS_STRN_ID, error);	//  Set up the error text

	ParamText(error_string, NULL, NULL, NULL);					//  Display error alert

	short item = Alert(TERMINAL_ERROR_ALERT_ID, NULL);

	ExitToShell();												//  Exit

}	//==== TerminalError() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure: GeneralError
//|
//| Purpose: Display error dialog.
//|
//| Parameters: error: the error code
//|__________________________________________________

void GeneralError(short error)
{

	Str255 error_string;
	GetIndString(error_string, GENERAL_ERRORS_STRN_ID, error);	//  Set up the error text

	ParamText(error_string, NULL, NULL, NULL);					//  Display error alert

	short item = Alert(GENERAL_ERROR_ALERT_ID, NULL);

}	//==== GeneralError() ====\\




//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure: GeneralErrorByString
//|
//| Purpose: Display error dialog containing the error string specified.
//|
//| Parameters: error: the error code
//|______________________________________________________________________

void GeneralErrorByString(char *error_string)
{

	Str255 pascal_string;
	strcpy((char *) pascal_string, error_string);				//  Convert error string to pascal
	CtoPstr((char *) pascal_string);

	ParamText(pascal_string, NULL, NULL, NULL);					//  Display error alert

	short item = Alert(GENERAL_ERROR_ALERT_ID, NULL);

}	//==== GeneralErrorByString() ====\\