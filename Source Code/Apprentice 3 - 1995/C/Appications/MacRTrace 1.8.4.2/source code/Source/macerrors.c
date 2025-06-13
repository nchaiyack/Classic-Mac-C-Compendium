/*****************************************************************************\
* macerrors.c                                                                 *
*                                                                             *
* This file contains code which is specific to the Macintosh.  It error       *
* checking, error messages, and error dialogs.                                *
\*****************************************************************************/

#include "macresources.h"
#include "macerrors.h"
#include <setjmp.h>
#include <string.h>


/* Bounds on classes of error messages */
#define	OS_ERROR_CODES_MIN				-233
#define	OS_ERROR_CODES_MAX				-1
#define	SYSENVIRONS_ERROR_CODES_MIN		-5502
#define	SYSENVIRONS_ERROR_CODES_MAX		-5500
#define	SYSTEM_ERROR_CODES_MIN			1
#define	SYSTEM_ERROR_CODES_MAX			105
#define	FILE_SYSTEM_ERROR_CODES_MIN		-1307
#define	FILE_SYSTEM_ERROR_CODES_MAX		-1300
#define PICTURE_ERROR_CODES_MIN			-11005
#define	PICTURE_ERROR_CODES_MAX			-11000
#define	STARTUP_MANAGER_ERROR_CODES_MIN	-351
#define	STARTUP_MANAGER_ERROR_CODES_MAX	-290
#define	MEMORYDISPATCH_ERROR_CODES_MIN	-625
#define	MEMORYDISPATCH_ERROR_CODES_MAX	-620
#define	APPLETALK_ERROR_CODES_MIN		-1029
#define	APPLETALK_ERROR_CODES_MAX		-1024
#define	ASP_ERROR_CODES_MIN				-1075
#define	ASP_ERROR_CODES_MAX				-1066
#define	ATP1_ERROR_CODES_MIN			-1105
#define	ATP1_ERROR_CODES_MAX			-1096
#define	ATP2_ERROR_CODES_MIN			-3109
#define	ATP2_ERROR_CODES_MAX			-3101
#define	PRINTER_ERROR_CODES_MIN			-4101
#define	PRINTER_ERROR_CODES_MAX			-4096
#define	GESTALT_ERROR_CODES_MIN			-5553
#define	GESTALT_ERROR_CODES_MAX			-5550


/* STR# resources containing error messages */
#define	OS_ERROR_MESSAGES_STRN				128
#define	SYSENVIRONS_ERROR_MESSAGES_STRN		129
#define	SYSTEM_ERROR_MESSAGES_STRN			130
#define	FILE_SYSTEM_ERROR_MESSAGES_STRN		131
#define PICTURE_ERROR_MESSAGES_STRN			132
#define	STARTUP_MANAGER_ERROR_MESSAGES_STRN	133
#define	MEMORYDISPATCH_ERROR_MESSAGES_STRN	134
#define	APPLETALK_ERROR_MESSAGES_STRN		135
#define	ASP_ERROR_MESSAGES_STRN				136
#define	ATP1_ERROR_MESSAGES_STRN			137
#define	ATP2_ERROR_MESSAGES_STRN			138
#define	PRINTER_ERROR_MESSAGES_STRN			139
#define	GESTALT_ERROR_MESSAGES_STRN			140



/* External globals */
extern Boolean		image_complete;			/* TRUE if the image is completely rendered */
extern jmp_buf		environment;			/* Used to store environment to restore on error */
extern Boolean		starting_up;			/* TRUE if we're still starting up */
extern DialogPtr	error_dialog;			/* the Error dialog */
extern DialogPtr	offer_to_abort_dialog;	/* the Offer to Abort dialog */


/* External prototypes */
extern void cleanup(Boolean natural);

/* Internal prototypes */
Boolean is_os_error(short error_code, short *message_offset);
Boolean is_sysenvirons_error(short error_code, short *message_offset);
Boolean is_system_error(short error_code, short *message_offset);
Boolean is_file_system_error(short error_code, short *message_offset);
Boolean is_picture_error(short error_code, short *message_offset);
Boolean is_startup_manager_error(short error_code, short *message_offset);
Boolean is_memorydispatch_error(short error_code, short *message_offset);
Boolean is_printer_error(short error_code, short *message_offset);
Boolean is_appletalk_error(short error_code, short *message_offset);
Boolean is_atp1_error(short error_code, short *message_offset);
Boolean is_atp2_error(short error_code, short *message_offset);
Boolean is_asp_error(short error_code, short *message_offset);
Boolean is_printer_error(short error_code, short *message_offset);
Boolean is_gestalt_error(short error_code, short *message_offset);



/*****************************************************************************\
* procedure abortive_error                                                    *
*                                                                             *
* Purpose: This procedure is called when an error is encountered during       *
*          rendering.  It aborts the render, puts up an error dialog, and     *
*          returns to the event loop.                                         *
*                                                                             *                                                                             *
* Parameters: error_code: the error code of the message to display.           *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 28, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void abortive_error(short error_code)
{

	short			item;
	unsigned char	*error_string = get_mac_error_message(error_code);
	Str63			error_number_string;
	
	/* Convert the error code to a string */
	NumToString((long) error_code, error_number_string);

	/* We never finished rendering the image */
	image_complete = FALSE;

	/* Display the error dialog */
	ParamText (error_string, "\p (", error_number_string, "\p).");
	ShowWindow(error_dialog);
	SelectWindow(error_dialog);
	ModalDialog ((ModalFilterUPP) NULL, &item);
	HideWindow(error_dialog);
	
	/* try to clean up after this failed render */
	cleanup(FALSE);
	
	/* Go back to the event loop (restore startup environment) */
	longjmp(environment, 1);

}	/* abortive_error() */



/*****************************************************************************\
* procedure abortive_string_error                                             *
*                                                                             *
* Purpose: This procedure is called when an error is encountered during       *
*          rendering.  It aborts the render, puts up an error dialog, and     *
*          returns to the event loop.                                         *
*                                                                             *                                                                             *
* Parameters: error_string: the error message to display.                     *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 28, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void abortive_string_error(char *error_string)
{

	short	item;

	/* We never finished rendering the image */
	image_complete = FALSE;

	/* Display the error dialog */
	ParamText (c2pstr(error_string), "\p (RTrace)", NULL, NULL);
	ShowWindow(error_dialog);
	SelectWindow(error_dialog);
	ModalDialog ((ModalFilterUPP) NULL, &item);
	HideWindow(error_dialog);
	p2cstr ((unsigned char *) error_string);
	
	/* try to clean up after this failed render */
	cleanup(FALSE);
	
	/* Go back to the event loop (restore startup environment) */
	longjmp(environment, 1);

}	/* abortive_string_error() */



/*****************************************************************************\
* procedure terminal_error                                                    *
*                                                                             *
* Purpose: This procedure is called when rtrace encounters an unrecoverable   *
*          error.  It displays an error alert, and quits.                     *
*                                                                             *                                                                             *
* Parameters: error_code: the error code of the message to display.           *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 10, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

void terminal_error(short error_code)
{

	short 			item;
	unsigned char	*error_string = get_mac_error_message(error_code);
	Str63			error_number_string;
	
	/* Convert the error code to a string */
	NumToString((long) error_code, error_number_string);

	/* Display error */
	ParamText (error_string, "\p (", error_number_string, "\p)");
	ShowWindow(error_dialog);
	SelectWindow(error_dialog);
	ModalDialog ((ModalFilterUPP) NULL, &item);

	/* Quit */
	ExitToShell();
	
}	/* terminal_error() */



/*****************************************************************************\
* procedure terminal_string_error                                             *
*                                                                             *
* Purpose: This procedure is called when rtrace encounters an unrecoverable   *
*          error.  It displays an error alert, and quits.                     *
*                                                                             *                                                                             *
* Parameters: error_code: the error code of the message to display.           *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 10, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

void terminal_string_error(char *error_string)
{

	short 	item;

	/* Display error */
	ParamText (c2pstr(error_string), "\p. Click Okay to quit MacRTrace.", 0, 0);
	ShowWindow(error_dialog);
	SelectWindow(error_dialog);
	ModalDialog ((ModalFilterUPP) NULL, &item);

	/* Quit */
	ExitToShell();
	
}	/* terminal_string_error() */



/*****************************************************************************\
* procedure terminal_startup_error                                            *
*                                                                             *
* Purpose: This procedure is called when rtrace encounters an error which is  *
*          terminal during startup but only abortive any other time.          *
*                                                                             *                                                                             *
* Parameters: error_code: the error code of the message to display.           *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 10, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

void terminal_startup_error(short error_code)
{

	short 			item;

	if (starting_up)
		terminal_error(error_code);
	else
		abortive_error(error_code);
	
}	/* terminal_startup_error() */



/*****************************************************************************\
* procedure possibly_terminal_string_error                                    *
*                                                                             *
* Purpose: This procedure is called when rtrace encounters an error which may *
*          be terminal, if the user chooses.  It displays an error alert, and *
*          prompts the user whether to quit or not.                           *
*                                                                             *                                                                             *
* Parameters: error_string: the error message to display.                     *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 10, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

void possibly_terminal_string_error(char *error_string)
{

	short 	item;

	/* Display message, abort if appropriate */
	ParamText (c2pstr(error_string), 0, 0, 0);
	ShowWindow(offer_to_abort_dialog);
	SelectWindow(offer_to_abort_dialog);
	ModalDialog ((ModalFilterUPP) NULL, &item);
	HideWindow(offer_to_abort_dialog);

	/* Abort if user wants to */
	if (item == OFFER_ABORT_BUTTON)
		ExitToShell();

	p2cstr((unsigned char *) error_string);

}	/* possibly_terminal_string_error() */



/*****************************************************************************\
* procedure get_mac_error_message                                             *
*                                                                             *
* Purpose: This procedure takes an error code and returns the appropriate     *
*          error message.                                                     *
*                                                                             *                                                                             *
* Parameters: error_code: the error code of the message to display.           *
*             return the message string (a Pascal string)                     *                                                                             *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 10, 1992                                              *
* Modified: December 10, 1994 so prototype indicates that string is Pascal.   *
\*****************************************************************************/

unsigned char *get_mac_error_message(short error_code)
{

	Str255			error_body;
	static Str255	error_message;
	short			message_offset;

	/* Is it an OS error? */
	if (is_os_error(error_code, &message_offset))
		
		/* Get the error code string */
		GetIndString (error_message, OS_ERROR_MESSAGES_STRN, message_offset);

	/* Is it a SysEnvirons error? */
	else if (is_sysenvirons_error(error_code, &message_offset))
		
		/* Get the error code string */
		GetIndString (error_message, SYSENVIRONS_ERROR_MESSAGES_STRN, message_offset);
	
	/* Is it a System error? */
	else if (is_system_error(error_code, &message_offset))
		
		/* Get the error code string */
		GetIndString (error_message, SYSTEM_ERROR_MESSAGES_STRN, message_offset);
	
	/* Is it a File System error? */
	else if (is_file_system_error(error_code, &message_offset))
		
		/* Get the error code string */
		GetIndString (error_message, FILE_SYSTEM_ERROR_MESSAGES_STRN, message_offset);
	
	/* Is it a Picture error? */
	else if (is_picture_error(error_code, &message_offset))
		
		/* Get the error code string */
		GetIndString (error_message, PICTURE_ERROR_MESSAGES_STRN, message_offset);
	
	/* Is it a Startup Manager error? */
	else if (is_startup_manager_error(error_code, &message_offset))
		
		/* Get the error code string */
		GetIndString (error_message, STARTUP_MANAGER_ERROR_MESSAGES_STRN, message_offset);
	
	/* Is it a MemoryDispatch error? */
	else if (is_memorydispatch_error(error_code, &message_offset))
		
		/* Get the error code string */
		GetIndString (error_message, MEMORYDISPATCH_ERROR_MESSAGES_STRN, message_offset);
	
	/* Is it an AppleTalk error? */
	else if (is_appletalk_error(error_code, &message_offset))
		
		/* Get the error code string */
		GetIndString (error_message, APPLETALK_ERROR_MESSAGES_STRN, message_offset);
	
	/* Is it an ASP error? */
	else if (is_asp_error(error_code, &message_offset))
		
		/* Get the error code string */
		GetIndString (error_message, ASP_ERROR_MESSAGES_STRN, message_offset);
	
	/* Is it a Apple Talk ATP/group 1 error? */
	else if (is_atp1_error(error_code, &message_offset))
		
		/* Get the error code string */
		GetIndString (error_message, ATP1_ERROR_MESSAGES_STRN, message_offset);
	
	/* Is it a Apple Talk ATP/group 2 error? */
	else if (is_atp2_error(error_code, &message_offset))
		
		/* Get the error code string */
		GetIndString (error_message, ATP2_ERROR_MESSAGES_STRN, message_offset);
	
	/* Is it a Printer error? */
	else if (is_printer_error(error_code, &message_offset))
		
		/* Get the error code string */
		GetIndString (error_message, PRINTER_ERROR_MESSAGES_STRN, message_offset);
	
	/* Is it a Gestalt error? */
	else if (is_gestalt_error(error_code, &message_offset))
		
		/* Get the error code string */
		GetIndString (error_message, GESTALT_ERROR_MESSAGES_STRN, message_offset);
	
	else
	
		/* We don't know about this error; make it blank so it will be tagged
			as an unknown error */
		error_message[0] = 0;
	
	/* If this error is blank, call it an unknown error */
	if (error_message[0] == 0)
		{
		strcpy ((char *) error_message, "Unknown type of error");
		c2pstr((char *) error_message);
		}
	
	return error_message;
	
}	/* get_mac_error_message() */


/*****************************************************************************\
* procedures is_xxxx_error                                                    *
*                                                                             *
* Purpose: These procedures take an error code TRUE if it is of the a certain *
*          type (specified by the xxxx in the name).  Furthermore, if it IS   *
*          of the specified type, message_offset will be set to the STR#      *
*          offset of the error message in the appropriate STR# resource.      *
*                                                                             *                                                                             *
* Parameters: error_code: the error code.                                     *
*             message_offest: receives the STR# offset                        *
*             return TRUE if the error is of that type.                       *                                                                             *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 24, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

Boolean is_os_error(short error_code, short *message_offset)
{

	if ((error_code >= OS_ERROR_CODES_MIN) && (error_code <= OS_ERROR_CODES_MAX))
		{
		*message_offset = OS_ERROR_CODES_MAX - error_code + 1;
		return TRUE;
		}
	else
		return FALSE;

}	/* is_os_error() */

/**********************************************/

Boolean is_sysenvirons_error(short error_code, short *message_offset)
{

	if ((error_code >= SYSENVIRONS_ERROR_CODES_MIN) && (error_code <= SYSENVIRONS_ERROR_CODES_MAX))
		{
		*message_offset = SYSENVIRONS_ERROR_CODES_MAX - error_code + 1;
		return TRUE;
		}
	else
		return FALSE;

}	/* is_sysenvirons_error() */

/**********************************************/

Boolean is_system_error(short error_code, short *message_offset)
{

	if ((error_code >= SYSTEM_ERROR_CODES_MIN) && (error_code <= SYSTEM_ERROR_CODES_MAX))
		{
		*message_offset = error_code;
		return TRUE;
		}
	else
		return FALSE;
		
}	/* is_system_error() */

/**********************************************/

Boolean is_file_system_error(short error_code, short *message_offset)
{

	if ((error_code >= FILE_SYSTEM_ERROR_CODES_MIN) && (error_code <= FILE_SYSTEM_ERROR_CODES_MAX))
		{
		*message_offset = FILE_SYSTEM_ERROR_CODES_MAX - error_code + 1;
		return TRUE;
		}
	else
		return FALSE;

}	/* is_file_system_error() */

/**********************************************/

Boolean is_picture_error(short error_code, short *message_offset)
{

	if ((error_code >= PICTURE_ERROR_CODES_MIN) && (error_code <= PICTURE_ERROR_CODES_MAX))
		{
		*message_offset = PICTURE_ERROR_CODES_MAX - error_code + 1;
		return TRUE;
		}
	else
		return FALSE;

}	/* is_picture_error() */

/**********************************************/

Boolean is_startup_manager_error(short error_code, short *message_offset)
{

	if ((error_code >= STARTUP_MANAGER_ERROR_CODES_MIN) && (error_code <= STARTUP_MANAGER_ERROR_CODES_MAX))
		{
		*message_offset = STARTUP_MANAGER_ERROR_CODES_MAX - error_code + 1;
		return TRUE;
		}
	else
		return FALSE;

}	/* is_startup_manager_error() */

/**********************************************/

Boolean is_memorydispatch_error(short error_code, short *message_offset)
{

	if ((error_code >= MEMORYDISPATCH_ERROR_CODES_MIN) && (error_code <= MEMORYDISPATCH_ERROR_CODES_MAX))
		{
		*message_offset = MEMORYDISPATCH_ERROR_CODES_MAX - error_code + 1;
		return TRUE;
		}
	else
		return FALSE;

}	/* is_memorydispatch_error() */

/**********************************************/

Boolean is_printer_error(short error_code, short *message_offset)
{

	if ((error_code >= PRINTER_ERROR_CODES_MIN) && (error_code <= PRINTER_ERROR_CODES_MAX))
		{
		*message_offset = PRINTER_ERROR_CODES_MAX - error_code + 1;
		return TRUE;
		}
	else
		return FALSE;

}	/* is_printer_error() */

/**********************************************/

Boolean is_appletalk_error(short error_code, short *message_offset)
{

	if ((error_code >= APPLETALK_ERROR_CODES_MIN) && (error_code <= APPLETALK_ERROR_CODES_MAX))
		{
		*message_offset = APPLETALK_ERROR_CODES_MAX - error_code + 1;
		return TRUE;
		}
	else
		return FALSE;

}	/* is_appletalk_error() */

/**********************************************/

Boolean is_atp1_error(short error_code, short *message_offset)
{

	if ((error_code >= ATP1_ERROR_CODES_MIN) && (error_code <= ATP1_ERROR_CODES_MAX))
		{
		*message_offset = ATP1_ERROR_CODES_MAX - error_code + 1;
		return TRUE;
		}
	else
		return FALSE;

}	/* is_atp1_error() */

/**********************************************/

Boolean is_atp2_error(short error_code, short *message_offset)
{

	if ((error_code >= ATP2_ERROR_CODES_MIN) && (error_code <= ATP2_ERROR_CODES_MAX))
		{
		*message_offset = ATP2_ERROR_CODES_MAX - error_code + 1;
		return TRUE;
		}
	else
		return FALSE;

}	/* is_atp2_error() */

/**********************************************/

Boolean is_asp_error(short error_code, short *message_offset)
{

	if ((error_code >= ASP_ERROR_CODES_MIN) && (error_code <= ASP_ERROR_CODES_MAX))
		{
		*message_offset = ASP_ERROR_CODES_MAX - error_code + 1;
		return TRUE;
		}
	else
		return FALSE;

}	/* is_asp_error() */

/**********************************************/

Boolean is_gestalt_error(short error_code, short *message_offset)
{

	if ((error_code >= GESTALT_ERROR_CODES_MIN) && (error_code <= GESTALT_ERROR_CODES_MAX))
		{
		*message_offset = GESTALT_ERROR_CODES_MAX - error_code + 1;
		return TRUE;
		}
	else
		return FALSE;

}	/* is_gestalt_error() */
