/* Defines */
#define DEBUG_PRINT 0

/* Prototypes */
char *get_mac_error_message(short error_code);
void abortive_error(short error_code);
void abortive_string_error(char *error_string);
void terminal_error(short error_code);
void terminal_string_error(char *error_string);
void terminal_startup_error(short error_code);
void possibly_terminal_string_error(char *error_string);
 
extern int  log_window_available;

/*****************************************************************************\
* macro DB                                                                    *
*                                                                             *
* Purpose: This macro is used for debug if the Think_C debugger can't be      *
*          used. If the debug_print flag is ON, then this message will print, *
*          otherwise, it only costs a simple �if� test (very cheap).          *
* Notes: If this procedure is called after the log window has been inited,the *
*       message will appear in the log window, otherwise, it will appear in   *
*       a console window.  The string should have a one character pad at the  *
*       end to convert to a '\r' if the message will appear in the log window.*
*                                                                             *
* Parameters: msg: line to output                                             *
*                                                                             *
* Created by: Reid Judd                                                       *
* Created on: Oct 18, 1992                                                    *
* Modified:                                                                   *
\*****************************************************************************/

#define DB( msg )                                 \
{                                                 \
  if (DEBUG_PRINT)                                \
  {                                               \
     if (log_window_available)                    \
     {                                            \
 		msg[strlen(msg) - 1] = '\r';              \
		add_line_to_log_window(msg);              \
	 } else {                                     \
	     printf("debug:%s\n", msg );              \
	 }                                            \
  }                                               \
}                 