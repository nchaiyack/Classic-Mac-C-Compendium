/*****************************************************************************\
* maclog.c                                                                    *
*                                                                             *
* This file contains code which is specific to the Macintosh.  It contains    *
* procedures which handle the log files and the log window.                   *
\*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "macresources.h"


/* externals */
extern fpos_t	current_stdout_read_pos;/* position we're reading in stdout */
extern fpos_t	current_stderr_read_pos;/* position we're reading in stderr */
extern FILE		*stdout_file;			/* file to which stdout is redirected */
extern FILE		*stderr_file;			/* file to which stderr is redirected */
extern Rect		drag_window_rect;		/* the area in which windows may be dragged */
extern MenuHandle windows_menu;			/* the Windows menu */


/* Globals */
Rect			log_scroll_bar_rect;	/* Rectangle containing the scroll bar */
Rect			log_text_rect;			/* Rectangle containing the log text */
WindowPtr		log_window;				/* the log window */
DialogRecord	log_window_rec;			/* storage for the log window */
ControlHandle	vert_scroll_bar;		/* the vertical scroll bar in the log window */
ControlHandle	horiz_scroll_bar;		/* the horizontal scroll bar in the log window */
TEHandle		log_text;				/* a handle to the log TERec */
Boolean			num_new_log_lines = 0;	/* number of lines to transfer to log window */
short			window_height;			/* height of text in log window, in pixels */
short			text_height;			/* height of log text, in pixels */
char			last_log_line[200];		/* The last line in the log text */
char			last_error_message[200];/* The most recent error message */
ControlActionUPP scroll_action_proc_upp;/* UPP for our TrackControl scroll action proc  */



/* height of log window text line */
#define LINE_HEIGHT			11

/* Maximum lines in the log window */
#define	MAX_NUM_LOG_LINES	1000

/* Size of unzoomed log window */
#define LOG_WINDOW_WIDTH	350
#define LOG_WINDOW_HEIGHT	290

/* Prototypes */
void add_line_to_log_window (char *line);
void setup_log_text (void);
void setup_log_window(void);
void update_log_window (void);
void update_scroll_bar(void);
void handle_log_click(Point where);
pascal void scroll_action_proc(ControlHandle control, short part);
void handle_log_window_zoom(Point where, short part_code);
void log_window_resized(void);
void grow_log_window(Point where);
void update_scroll_bar_with_active(Boolean active);
void show_log_window(Boolean bring_to_front);
void hide_log_window(void);
void mac_write (FILE *file, ...);
void process_mac_event(void);



/*****************************************************************************\
* procedure setup_log_window                                                  *
*                                                                             *
* Purpose: This procedure sets up the log window.                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 29, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void setup_log_window(void)
{

	Rect		temp_rect = {0, 0, 0, 0};
	WStateData	*wstate_data;
	
	/* Redirect stdout and stderr */
	stdout_file = freopen ("RTrace log file", "w+", stdout);
	stderr_file = freopen ("RTrace error file", "w+", stderr);

	/* Remember our place in stdout_file and stderr_file */
	fgetpos(stdout_file, &current_stdout_read_pos);
	fgetpos(stderr_file, &current_stderr_read_pos);

	/* Create the log window */
	log_window = GetNewWindow(LOG_WINDOW, &log_window_rec, (WindowPtr) -1L);

	/* Clear out the WStateData so we will know later that this window has
		not been placed */
	BlockMove(&temp_rect, *(( (WindowPeek) log_window)->dataHandle), 8);

	/* Create a new TEHandle for the log text-- don't worry about the
		rectangles yet */
	SetPort (log_window);
	log_text = TENew(&temp_rect, &temp_rect);
	
	/* Set the text characteristics */
	(*log_text)->txFont = monaco;		/* Monaco font */
	(*log_text)->txSize = 9;			/* 9 point */
	(*log_text)->lineHeight = 11;		/* 11 points line height */
	(*log_text)->fontAscent = 9;		/* 9 points font ascent */
	
	/* Enable automatic scrolling */
	TEAutoView (TRUE, log_text);

	/* Create the scroll bars; don't worry about the rect yet */
	vert_scroll_bar = NewControl(log_window, &temp_rect, "\p",
									TRUE, 0, 0, 32767, scrollBarProc, 0);
	horiz_scroll_bar = NewControl(log_window, &temp_rect, "\p",
									TRUE, 0, 0, 0, scrollBarProc, 0);

	/* Disable the horizontal scroll bar (permanently) */
	HiliteControl (horiz_scroll_bar, 255);

	/* Set destRect to be as wide as the window, but zero height */
	SetRect (&((*log_text)->destRect), 2, 2, LOG_WINDOW_WIDTH - 20, 2);

	/* Set the zoomed size */
	wstate_data = (WStateData *) *( ((WindowPeek) log_window)->dataHandle);
	SetRect (&(wstate_data->stdState), 3, 40, LOG_WINDOW_WIDTH,
				drag_window_rect.bottom - drag_window_rect.top - 36);
	
	/* Initialize the UPP for our scroll action proc */
	scroll_action_proc_upp = NewControlActionProc(scroll_action_proc);
	
}	/* setup_log_window() */



/*****************************************************************************\
* procedure show_log_window                                                   *
*                                                                             *
* Purpose: This procedure displays the log window.                            *
*                                                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void show_log_window(Boolean bring_to_front)
{

	/* Show the window */
	ShowWindow (log_window);
	
	/* Bring it to the front if we're supposed to */
	SelectWindow (log_window);
	
	/* Set the text to "Hide Log Window" */
	SetItem (windows_menu, SHOW_LOG_ITEM, "\pHide Log Window");
	
}	/* show_log_window() */



/*****************************************************************************\
* procedure hide_log_window                                                   *
*                                                                             *
* Purpose: This procedure hides the log window.                               *
*                                                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void hide_log_window(void)
{

	/* Hide the window */
	HideWindow (log_window);
	
	/* Set the text of the Windows menu to "Show Log Window" */
	SetItem (windows_menu, SHOW_LOG_ITEM, "\pShow Log Window");
	
}	/* hide_log_window() */



/*****************************************************************************\
* procedure update_log_window                                                 *
*                                                                             *
* Purpose: This procedure updates the log window.                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 28, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void update_log_window (void)
{

	/* Begin the update */
	BeginUpdate(log_window);

	/* Update the log text */
	TEUpdate(&log_text_rect, log_text);

	/* Draw the scroll bar */
	DrawControls (log_window);
	
	/* Draw the grow icon */
	DrawGrowIcon (log_window);

	/* terminate the update */
	EndUpdate(log_window);

}	/* update_log_window() */



/*****************************************************************************\
* procedure mac_write                                                         *
*                                                                             *
* Purpose: This procedure is called every time rtrace wants to write to a     *
*          file.  If it is headed for the log file, this intercepts it and    *
*          sends it to the log window too.                                    *
*                                                                             *
* Parameters: line: line to insert                                            *
*                                                                             *
* Created by: Reid Judd                                                       *
* Created on: September 18, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

void mac_write (FILE *file, ...)
{
	va_list arglist;
	char	*format;
	char	string[200];

	/* Give background processes some time */
	process_mac_event();

	/* Get the argument list */
	va_start(arglist, file);

	/* Get the format string */
	format = va_arg(arglist, char *);

	/* Generate a string from the arguments */
	vsprintf(string, format, arglist);
	
	/* If this is being output to the log file, put it in the log window */
	if ((file == stderr_file) || (file == stdout_file))
		{
			
		/* If the last character is a newline, convert it to a into a return */
		if (string[strlen(string) - 1] == '\n')
			string[strlen(string) - 1] = '\r';
		
		/* Add the string to the log window */
		add_line_to_log_window(string);
		
		/* If the last character is a now a return, chop it off */
		if (string[strlen(string) - 1] == '\r')
			string[strlen(string) - 1] = '\0';
		
		/* Remember this so we can display it in the window when the program halts. */
		strcpy(last_error_message, string);
		}

	/* Write the string to the file */
	fputs(string, file);
	
	/* Done processing argument lists */
	va_end(arglist);

}	/* mac_write() */



/*****************************************************************************\
* procedure add_line_to_log_window                                            *
*                                                                             *
* Purpose: This procedure adds a line of text to the log window, and updates  *
*          it appropriately.                                                  *
*                                                                             *
* Parameters: line: line to insert                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 28, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void add_line_to_log_window (char *line)
{

	/* If there are already the maximum number of lines, delete the first */
	if ((*log_text)->nLines == MAX_NUM_LOG_LINES)
		{
		
		/* Select the first line */
		TESetSelect(0, (*log_text)->lineStarts[1], log_text);
		
		/* Delete the first line */
		TEDelete (log_text);

		}

	/* Move the insertion point to the end */
	TESetSelect(32767, 32767, log_text);
	
	/* Insert the text */
	TEInsert(line, strlen(line), log_text);

	/* If this line ends with a return we have a new line in out file.
		Get the new total height of the text in pixels */
	if (line[strlen(line) - 1] == '\r')
		{
		text_height += LINE_HEIGHT;
		(*log_text)->destRect.bottom += LINE_HEIGHT;

		/* Set the new maximum control height */
		SetCtlMax(vert_scroll_bar, text_height - window_height);

		/* update the scroll bar */
		update_scroll_bar ();
		}
	
	/* Make this line globally available, in case it's an error message */
	strcpy (last_log_line, line);

}	/* add_line_to_log_window() */



/*****************************************************************************\
* procedure update_scroll_bar                                                 *
*                                                                             *
* Purpose: This procedure calls update_scroll_bar_with_active, using the      *
*          current log window active state.                                   *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 28, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void update_scroll_bar(void)
{

	update_scroll_bar_with_active(((WindowPeek) log_window)->hilited);

}	/* update_scroll_bar() */



/*****************************************************************************\
* procedure update_scroll_bar_with_active                                     *
*                                                                             *
* Purpose: This procedure updates the vertical scroll bar, according to the   *
*          log TERec and the setting of active.                               *
*                                                                             *
* Parameters: active: FALSE if the scroll bar should be deactivated           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 28, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void update_scroll_bar_with_active(Boolean active)
{

	/* Set the thumb to the correct place */
	SetCtlValue(vert_scroll_bar, (*log_text)->viewRect.top - (*log_text)->destRect.top);

	/* Deactivate the scroll bar if there's no way to scroll, or if the
		window isn't active */
	if ( ( ((*log_text)->destRect.top >= (*log_text)->viewRect.top) &&
			((*log_text)->destRect.bottom <= (*log_text)->viewRect.bottom) ) ||
				!active )
		
		HiliteControl(vert_scroll_bar, 255);
	
	/* Otherwise, activate it */
	else

		HiliteControl (vert_scroll_bar, 0);

}	/* update_scroll_bar_with_active() */



/*****************************************************************************\
* procedure handle_log_click                                                  *
*                                                                             *
* Purpose: This procedure handles a click in the content region of the log    *
*          window.                                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 29, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void handle_log_click(Point where)
{

	short 			part;			/* part of the control which was hit */
	short 			old_value;		/* the old value of the scroll bar */
	short 			new_value;		/* the new value of the scroll bar */
	short 			scroll_distance;/* the distance to scroll */
	ControlHandle	control;		/* the scroll bar */

	/* Find if it was in a scroll bar */
	SetPort (log_window);
	GlobalToLocal (&where);
	part = FindControl(where, log_window, &control);
	
	/* If it was the vertical scroll bar, handle it */
	if (control == vert_scroll_bar)
		{
		switch (part)
			{
			case inUpButton:
			case inDownButton:
			case inPageUp:
			case inPageDown:
				TrackControl(control, where, scroll_action_proc_upp);
				
				break;
			case inThumb:
				
				/* Remember what the original value is */
				old_value = GetCtlValue(control);
				
				/* Let user drag the thumb */
				TrackControl(control, where, NULL);
				
				/* Get the new value of the control */
				new_value = GetCtlValue(control);
				
				/* find distance to scroll */
				scroll_distance = (old_value - new_value) -
									(old_value - new_value) % LINE_HEIGHT;
				
				/* Scroll the text appropriately */
				TEPinScroll (0, scroll_distance, log_text);
				
				/* Update the scroll bar */
				update_scroll_bar();

			}
		}

}	/* handle_log_click() */


pascal void scroll_action_proc(ControlHandle control, short part)
{
	
	short vscroll;		/* distance to scroll vertically */

	switch (part)
		{
		case inUpButton:
			vscroll = LINE_HEIGHT;
			break;
		case inDownButton:
			vscroll = -LINE_HEIGHT;
			break;
		case inPageUp:
			vscroll = ((*log_text)->viewRect.bottom - (*log_text)->viewRect.top);
			break;
		case inPageDown:
			vscroll = -((*log_text)->viewRect.bottom - (*log_text)->viewRect.top);
			break;
		}

	TEPinScroll (0, vscroll, log_text);

	if ((*log_text)->viewRect.bottom > (*log_text)->destRect.bottom)

		/* If the bottom of the text is above the bottom of the window,
			set the scroll bar at its maximum */
		SetCtlValue(control, GetCtlMax(control));

	else
			
		/* Otherwise, set it according to the current position in the text */	
		SetCtlValue(control, (*log_text)->viewRect.top - (*log_text)->destRect.top);

}	/* scroll_action_proc() */



/*****************************************************************************\
* procedure grow_log_window                                                   *
*                                                                             *
* Purpose: This procedure handles a click in the grow region of the log       *
*          window.                                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 29, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void grow_log_window(Point where)
{

	Rect		size_rect = {48, LOG_WINDOW_WIDTH+1, 32767, LOG_WINDOW_WIDTH+1};
									/* bounds on size of window */
	long		new_size;			/* requested size */
	short		width, height;		/* requested size as shorts */
	short		real_height;		/* height after pinning */

	/* Let user resize the window */
	new_size = GrowWindow(log_window, where, &size_rect);

	if (new_size)
		{
		
		/* Get new width and height-- adjust height to an even number of
			lines of text fit */
		width = LoWord(new_size);
		height = HiWord(new_size);
		real_height = height - ((height - 9) % LINE_HEIGHT);
		
		/* Resize the window as requested */
		SizeWindow(log_window, width, real_height, TRUE);
		
		/* Move the resize the text and scroll bars to accomodate */
		log_window_resized();
		
		/* Force the window to completely update */
		SetPort (log_window);
		EraseRect (&log_window->portRect);
		InvalRect (&log_window->portRect);
		
		}
}



/*****************************************************************************\
* procedure handle_log_window_zoom                                            *
*                                                                             *
* Purpose: This procedure handles a click in the zoom region of the log       *
*          window.                                                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 29, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void handle_log_window_zoom(Point where, short part_code)
{

	/* Track user until (s)he releases the mouse */
	if (TrackBox (log_window, where, part_code))
		{

		/* Prepare for zoom */
		SetPort (log_window);
		EraseRect (&log_window->portRect);
		
		/* Zoom */
		ZoomWindow(log_window, part_code, TRUE);
	
		/* change the text and scroll bars to accommodate the new size */
		log_window_resized();
		}	

}	/* handle_log_window_zoom() */



/*****************************************************************************\
* procedure log_window_resized                                                *
*                                                                             *
* Purpose: This procedure sets the size of the log text box and the scroll    *
*          bars according the the size of the log window.                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 29, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void	log_window_resized(void)
{

	/* Copy the window rectangle into the text rectangle */
	BlockMove (&(log_window->portRect), &log_text_rect, 8);
	
	/* Move it to the origin */
	OffsetRect (&log_text_rect, -log_text_rect.left, -log_text_rect.top);
	
	/* Shrink the text rectangle to make room for the scroll bars */
	log_text_rect.right -= 16;
	log_text_rect.bottom -= 16;
	
	/* Inset the text rect to make a 2-pixel margin */
	InsetRect (&log_text_rect, 2, 2);
	
	/* Resize the text to fit its rectangle */
	BlockMove (&log_text_rect, &((*log_text)->viewRect), 8);

	/* Move the vertical scroll bar to the right of the text rectangle */
	MoveControl (vert_scroll_bar, log_text_rect.right + 3, -1);
	
	/* Resize the vertical scroll bar to fill the window vertically */
	SizeControl (vert_scroll_bar, 16, log_text_rect.bottom + 5);

	/* Move the horizontal scroll bar to right below text rectangle */
	MoveControl (horiz_scroll_bar, -1, log_text_rect.bottom + 3);
	
	/* Resize the horizontal scroll bar to fill the window horizontally */
	SizeControl (horiz_scroll_bar, log_text_rect.right + 5, 16);

	/* Recompute window height */
	window_height = log_text_rect.bottom - log_text_rect.top;

	/* Redraw the scroll bar */
	update_scroll_bar();

	/* Set the new maximum control height */
	SetCtlMax(vert_scroll_bar, text_height - window_height);

}	/* log_window_resized() */

