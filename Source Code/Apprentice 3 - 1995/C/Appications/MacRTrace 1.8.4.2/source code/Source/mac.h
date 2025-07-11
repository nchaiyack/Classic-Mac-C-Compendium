/*
 * Copyright (c) 1992, 1993 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Reid Judd           - overall, portability
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Antonio Costa, at INESC-Norte. The name of the author and
 * INESC-Norte may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/**********************************************************************
 *    RAY TRACING - Configuration - Version 8.4.1                     *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, September 1992         *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, September 1994         *
 **********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <console.h>
#ifdef TIMES
#undef TIMES
#endif
#ifndef TIME
#define TIME
#endif
//#define ECHO


/* Externals */
extern short	temp_folder_wd_id;	/* Working Directory refnum of Temporary Items folder */
extern short	scene_file_wd_id;	/* Working Directory refnum of .sff file folder */


/* Prototypes */
void mac_write(FILE *file, char *format, ...);
void mac_halt(void);
void step_progress_bar(void);
void set_progress_bar_value(long value);
void set_progress_bar_max(long max);
void set_status_text(char *string);
void update_memory_bar_value(void);
void process_mac_event(void);
void init_progress_bar_step_mode(void);
Boolean is_temp_file(char *filename);

/* on the mac, we need to check when we're opening a temp file,
	and make sure we create it in the RTrace directory */

#define OPEN(file, name, mode)\
do {\
	if (is_temp_file(name))\
		SetVol( (StringPtr) NULL, temp_folder_wd_id);\
	else\
		SetVol( (StringPtr) NULL, scene_file_wd_id);\
	(file) = fopen(name, mode);\
	IO_status = (file) ? IO_OK : IO_OPEN;\
} while (0)


/* When running on the mac, we need to send the output lines to the log,
   so we define WRITE to call mac_write, which outputs the string to
   the log.  */
#define WRITE (void) mac_write


extern Boolean get_another_event;
#define CHECK_EVENTS 	if (get_another_event) process_mac_event()

#define INPUT_PARAMETERS\
do {\
  console_options.nrows = 35;\
  console_options.ncols = 55;\
  cshow(stdout);\
  parameters = ccommand(&parameter);\
} while (0)



#define EXIT ExitToShell()
#define HALT mac_halt()

/* These macros keep the user continually informed
   of the situation as the file is processed */
#define UPDATE_STATUS_TEXT(s)			set_status_text(s)
#define UPDATE_STATUS_FREE_MEMORY		update_memory_bar_value()
#define UPDATE_STATUS_INIT_STEP_MODE	init_progress_bar_step_mode()
#define UPDATE_STATUS_STEP_ON_TRUE(p)	if (p) step_progress_bar();
#define UPDATE_STATUS_VALUE(v)			set_progress_bar_value(v);
#define UPDATE_STATUS_VALUE_ON_TRUE(p,v) if (p) set_progress_bar_value(v);
#define UPDATE_STATUS_MAX_VALUE(m)		set_progress_bar_max(m);

#define PROTOTYPES
