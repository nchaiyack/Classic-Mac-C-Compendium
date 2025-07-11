/*****************************************************************************\
* macfiles.c                                                                  *
*                                                                             *
* This file contains code which is specific to the Macintosh.  It implements  *
* the opening and saving of files for RTrace.                                 *
\*****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <Folders.h>
#include <Script.h>
#include "defs.h"
#include "extern.h"
#include "macerrors.h"
#include "macdefaults.h"
#include "mactypes.h"



/* externals */
extern Boolean		keep_image_in_memory;	/* TRUE if we keep a bitmap in memory */
extern Boolean		show_image_rendering;	/* TRUE if we keep a bitmap in memory */
extern short		num_new_log_lines;		/* the number of stdout lines not yet in the log */
extern CGrafPtr		image_port;				/* offscreen port which contains the image */
extern WindowPtr	image_window;			/* the image window data structures */
extern int          screen_size_x;          /* Horizontal size of the image */
extern int          screen_size_y;          /* Vertical size of the image */
extern char			scene_filename[];		/* Filename of the scene file */
extern short		scene_file_wd_id;		/* Working Directory refnum of scene file folder */
extern DialogPtr	options_dialog;			/* the options dialog */
extern DialogPtr	status_dialog;			/* the status dialog */
extern DialogPtr	animation_dialog;		/* the animation dialog */
extern WindowPtr	log_window;				/* the log window */
extern DialogPtr	about_dialog;			/* the about dialog */
extern DialogPtr	abort_render_dialog;	/* the Abort Render dialog */
extern DialogPtr	cancel_dialog;			/* the Cancel dialog */
extern DialogPtr	error_dialog;			/* the Error dialog */
extern DialogPtr	offer_to_abort_dialog;	/* the Offer to Abort dialog */
extern DialogPtr	saving_frame_dialog;	/* the Saving Frame dialog */
extern long			frames;					/* The number of frames in the animation sequence */
extern Boolean		find_folder_available;	/* TRUE if FindFolder is available */
extern Boolean		starting_up;			/* TRUE if we're still starting up */
extern Boolean		f8bit_QD_available;		/* TRUE if 8-bit QuickDraw is available */
extern Boolean		quicktime_available;	/* TRUE if QuickTime is available */
extern PicHandle	bitmap_pict;			/* The Picture which contains the image bitmap */


/* globals */
fpos_t			current_stdout_read_pos;	/* position we're reading in stdout */
fpos_t			current_stderr_read_pos;	/* position we're reading in stderr */
FILE			*stdout_file;				/* file to which stdout is redirected */
FILE			*stderr_file;				/* file to which stderr is redirected */
short			blessed_folder_wd_id;		/* Working Directory refnum of blessed folder */
short			temp_folder_wd_id;			/* Working Directory refnum of Temporary Items folder */
short			rtrace_wd_id;				/* Working Directory refnum of RTrace folder */



/************************* Local Prototypes **************************/
Boolean choose_scene_file(void);
pascal Boolean filter_scene(ParmBlkPtr file_params);
long get_num_objects_sff(char *filename);
void write_rect(short refnum, Rect *rect);
void write_long(short refnum, long number);
void write_short(short refnum, short number);
void write_char(short refnum, char number);
void do_stdout_check(void);
void save_pict_file(SFReply	*my_reply);
void stdout_to_log(void);
void add_line_to_log_window (char *line);
void read_view_info(void);
void convert_ppm_to_pict(char *ppm_filename, Str255 pict_filename, short pict_vrefnum);
void handle_io_error(short error);
void setup_temp_folder(void);
void delete_temp_files(Boolean movie_too);
short make_working_directory(short vrefnum, long dir_id);
void create_about_movie(void);
Boolean is_temp_file(char *filename);



/*****************************************************************************\
* procedure choose_scene_file                                                 *
*                                                                             *
* Purpose: This procedure allows the user to select a scene file using the    *
*          standard Mac file dialog.  This sets scene_filename and            *
*          scene_wd_id if the user chooses a file.                            *
*                                                                             *
* Parameters: returns 0 if user cancels, 1 of not                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

Boolean choose_scene_file(void)
{

	SFTypeList	types;				/* permissible file types to open */
	SFReply 	scene_file_reply;	/* data structure for file info */
	Point		where = {40, 40};	/* SF dialog location */
	FileFilterUPP filter_scene_upp;

	types[0] = 'TEXT';
	
	filter_scene_upp = NewFileFilterProc(filter_scene);

	/* Let the user choose a scene input file.  */
	SFGetFile (where, 0L, filter_scene_upp, 1, types, 0L, &scene_file_reply);
	
	DisposeRoutineDescriptor(filter_scene_upp);

	/* If the user cancelled, return 1 */
	if (!scene_file_reply.good)
		return 0;

	/* Make a working directory for the scene file directory */
	scene_file_wd_id = scene_file_reply.vRefNum;

	/* Save the filename of this scene file (a global) */
	strcpy (scene_filename, p2cstr(scene_file_reply.fName));

	/* no problems */
	return 1;
	
}	/* choose_scene_file() */



/*****************************************************************************\
* procedure filter_scene                                                      *
*                                                                             *
* Purpose: This procedure filters files in the standard file dialog so only   *
*          those ending in .sff or .scn appear.                               *
*                                                                             *
* Parameters: file_params contains a description of one file                  *
*             returns FALSE if the file should be displayed.                  *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
*   January 26, 1995 by Greg Ferrar                                           *
*     added support for .scn files.                                           *
\*****************************************************************************/

pascal Boolean filter_scene(ParmBlkPtr file_params)
{

	/* Get the filename address and length */
	char *name = (char *) ((FileParam *) file_params)->ioNamePtr;
	int length = name[0];
	
	/* check that the last four characters are ".sff" or ".scn" */
	if (((name[length - 3] == '.') &&
		(name[length - 2] == 's') &&
		(name[length - 1] == 'f') &&
		(name[length] == 'f')) ||
	   ((name[length - 3] == '.') &&
		(name[length - 2] == 's') &&
		(name[length - 1] == 'c') &&
		(name[length] == 'n')))
		return FALSE;
	else return TRUE;

}	/* filter_scene() */



/*****************************************************************************\
* procedure save_pict_file                                                    *
*                                                                             *
* Purpose: This procedure creates a pict file.  The PICT file is generated    *
*          from the temporary .ppm file.                                      *
*                                                                             *
* Parameters: my_reply: the result of the SPutFile call                       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 26, 1992                                                 *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
*   Greg Ferrar  9/9/92           moved guts to convert_ppm_to_pict           *
\*****************************************************************************/

void save_pict_file(SFReply	*my_reply)
{

	short	pict_file_refnum;			/* refnum of output file */
	long	zero_longword = 0;
	short	error;
	short	i;
	long	num_bytes;

	/* Create and open the ouput file for writing */
	FSDelete (my_reply->fName, my_reply->vRefNum);
	error = Create (my_reply->fName, my_reply->vRefNum, 'ttxt', 'PICT');
	if (error) abortive_error(error);

	error = FSOpen (my_reply->fName, my_reply->vRefNum, &pict_file_refnum);
	if (error) abortive_error(error);

	/* Write 512 zeros (the PICT header) */
	for (i = 512; i;)
		{
		
		/* Write a zero long word */
		num_bytes = sizeof(long);
		error = FSWrite (pict_file_refnum, &num_bytes, &zero_longword);
		if (error) abortive_error(error);
		
		/* Decrement the number of zeros left to write */
		i -= sizeof(long);
		
		}
		
	/* Write the PICT */
	num_bytes = GetHandleSize((Handle) bitmap_pict);
	error = FSWrite (pict_file_refnum, &num_bytes, *bitmap_pict);		
	if (error) abortive_error(error);

	/* Close the output file */
	error = FSClose (pict_file_refnum);
	if (error) abortive_error(error);

}	/* save_pict_file() */



/*****************************************************************************\
* procedure stdout_to_log                                                     *
*                                                                             *
* Purpose: This procedure transfers a line from stdout to the log file.       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 26, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void stdout_to_log(void)
{

	char	this_line[100];
	
	/* Go to the last place we read from */
	fsetpos(stdout_file, &current_stdout_read_pos);

	while (1)
		{
		
		/* Try to read a string */
		fgets(this_line, 10000, stdout_file);
	
		/* If we found the end of the file, that's it */
		if (feof(stdout_file)) break;

		/* make the newline a return, */
		this_line[strlen(this_line) - 1] = '\r';
		
		/* and add it to the log window */
		add_line_to_log_window (this_line);
		
		/* We have one less log line to worry about */
		num_new_log_lines--;
		
		/* Remember where to read from next time */
		fgetpos(stdout_file, &current_stdout_read_pos);

		}

}



/*****************************************************************************\
* procedure setup_temp_folder                                                 *
*                                                                             *
* Purpose: This procedure finds the vrefnum of the Temporary Items folder so  *
*          we can store our temporary files there.                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 12, 1992                                              *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void setup_temp_folder(void)
{

	short 	error;
	short 	temp_folder_vrefnum;
	long	temp_folder_dirid;

	/* If FindFolder doesn't exist, we'll just use the RTrace directory */
	if (!find_folder_available)
		temp_folder_wd_id = rtrace_wd_id;

	else
		{
		error = FindFolder(kOnSystemDisk, kTemporaryFolderType, kCreateFolder,
							&temp_folder_vrefnum, &temp_folder_dirid);
		if (error) terminal_startup_error(error);
		
		/* Make it into a working directory */
		temp_folder_wd_id = make_working_directory(temp_folder_vrefnum, temp_folder_dirid);
		}

	/* Clear out any of our temporary files in the Temporary Items Folder, if somehow
		they were left there last time the program ran */
	delete_temp_files(TRUE);

	/* Create the About... movie, if we have QuickTime */
	if (quicktime_available)
		create_about_movie();
	
}	/* create_about_movie() */



/*****************************************************************************\
* procedure create_about_movie                                                *
*                                                                             *
* Purpose: This procedure creates the movie which will rotate the RTrace in   *
*          the About... dialog.                                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 22, 1992                                              *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void create_about_movie(void)
{

	long	proc_id;
	FSSpec	fsspec;
	short	error;
	long	size;
	Handle	handle;
	short	frefnum;
	
	/* Create the FSSpec for the movie file */
	error = GetWDInfo (temp_folder_wd_id, &(fsspec.vRefNum), &(fsspec.parID), &proc_id);
	if (error) terminal_error(error);
	strcpy ((char *) fsspec.name, (char *) "rtrace_movie");
	c2pstr ((char *) fsspec.name);

	/* Create the movie file */
	error = FSpCreate(&fsspec, 'TVOD', 'MooV', smSystemScript);
	if (error) terminal_error(error);
	
	/* Write the movie file resource fork */
	error = FSpOpenRF(&fsspec, fsCurPerm, &frefnum);
	if (error) terminal_error(error);
	handle = GetResource('movf', 128);
	size = GetHandleSize(handle);
	HLock(handle);
	error = FSWrite (frefnum, &size, *handle);
	if (error) terminal_error(error);
	HUnlock(handle);
	error = FSClose(frefnum);
	if (error) terminal_error(error);
	
	/* Write the movie file data fork */
	error = FSpOpenDF(&fsspec, fsCurPerm, &frefnum);
	if (error) terminal_error(error);
	handle = GetResource('movf', 129);
	size = GetHandleSize(handle);
	HLock(handle);
	error = FSWrite (frefnum, &size, *handle);
	if (error) terminal_error(error);
	HUnlock(handle);
	error = FSClose(frefnum);
	if (error) terminal_error(error);

}	/* create_about_movie() */



/*****************************************************************************\
* procedure is_temp_file                                                      *
*                                                                             *
* Purpose: This procedure determines from a filename whether the associated   *
*          file is one of our temporary files.                                *
*                                                                             *
* Parameters: filename: the name of the file.                                 *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 9, 1995                                                 *
\*****************************************************************************/

Boolean is_temp_file(char *filename)
{

	/* Our temp files start with "rtrace" */
	return ((filename[0] == 'r') &&
			(filename[1] == 't') &&
			(filename[2] == 'r') &&
			(filename[3] == 'a') &&
			(filename[4] == 'c') &&
			(filename[5] == 'e'));

}	/* is_temp_file() */



/*****************************************************************************\
* procedure delete_temp_files                                                 *
*                                                                             *
* Purpose: This procedure deletes all temporary files from the Temporary      *
*          Items directory.                                                   *
*                                                                             *
* Parameters: movie_too: TRUE if the About... movie should also be deleted.   *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 12, 1992                                              *
* Modified:                                                                   *
*   December 29, 1994 by Greg Ferrar                                          *
*     fixed a serious crash bug due to incorrect declaration of CPB.          *
\*****************************************************************************/

void delete_temp_files(Boolean movie_too)
{

	short		index = 1;
	short		error;
	CInfoPBRec	CPBRec;
	HFileInfo	*CPB = (HFileInfo *) &CPBRec;
	Str255		filename;
	long		dirid;
	long		proc_id;

	/* Set up filename buffer */
	CPB->ioNamePtr = (unsigned char *) filename;

	/* Find the volume and directory id of the Temporary directory */
	error = GetWDInfo (temp_folder_wd_id, &(CPB->ioVRefNum), &dirid, &proc_id);
	if (error) terminal_startup_error(error);

	/* Scan through all files in the directory */
	do
		{
		
		/* Set index to point to current file */
		CPB->ioFDirIndex = index;	

		/* Set directory to search */
		CPB->ioDirID = dirid;
		
		/* Get the info on this file */
		error = PBGetCatInfo(&CPBRec, FALSE);
		if (error) continue;
	
		/* Convert filename to a C string */
		p2cstr(filename);

		/* If this is the movie file, and we're suppose to delete it, delete it. */
		if (!strcmp((char *) filename, "rtrace_movie"))
			{
			if (movie_too)
				{
				/* Delete the movie file */
				c2pstr((char *) filename);
				error = FSDelete (CPB->ioNamePtr, temp_folder_wd_id);
				if (error) terminal_startup_error(error);
				}
				
			/* Go on to the next file */
			else
				index++;
			}
		
		else if (is_temp_file((char *) filename))
				
			/* It's our file-- delete it */
			{
			c2pstr((char *) filename);
			error = FSDelete (CPB->ioNamePtr, temp_folder_wd_id);
			if (error) terminal_startup_error(error);
			}
		
		/* Go to next file */
		else
			index++;
		
		}
	while (!error);

}	/* delete_temp_files() */



/*****************************************************************************\
* procedure read_view_info                                                    *
*                                                                             *
* Purpose: This procedure scans the beginning of an .sff file, grabbing the   *
*          view information.                                                  *
*                                                                             *
* Parameters: sets globals look, eye, up, view_angle_x, view_angle_y          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void read_view_info(void)
{

	FILE	*scene;
	double	value;
	short	error;
		
	/* Open the sff file for reading */
	error = SetVol((StringPtr) NULL, scene_file_wd_id);
	if (error) abortive_error(error);
	scene = fopen(scene_filename, "r");

	/* read the eye point */
	ADVANCE(scene);
	get_valid(scene, &value, X_MIN, X_MAX, "EYE POINT X");
	eye.x = value;
	get_valid(scene, &value, Y_MIN, Y_MAX, "EYE POINT Y");
	eye.y = value;
	get_valid(scene, &value, Z_MIN, Z_MAX, "EYE POINT Z");
	eye.z = value;
	
	/* read the look point */
	ADVANCE(scene);
	get_valid(scene, &value, X_MIN, X_MAX, "LOOK POINT X");
	look.x = value;
	get_valid(scene, &value, Y_MIN, Y_MAX, "LOOK POINT Y");
	look.y = value;
	get_valid(scene, &value, Z_MIN, Z_MAX, "LOOK POINT Z");
	look.z = value;

	/* Compute gaze */
	ADVANCE(scene);
	gaze.x = look.x - eye.x;
	gaze.y = look.y - eye.y;
	gaze.z = look.z - eye.z;

	/* Get up vector */
	get_valid(scene, &value, X_MIN, X_MAX, "UP VECTOR X");
	up.x = value;
	get_valid(scene, &value, Y_MIN, Y_MAX, "UP VECTOR Y");
	up.y = value;
	get_valid(scene, &value, Z_MIN, Z_MAX, "UP VECTOR Z");
	up.z = value;

	/* Get view angles */
	ADVANCE(scene);
	get_valid(scene, &value, 0.5, 89.5, "HORIZONTAL VIEW Angle");   /* Degrees */
	view_angle_x = DEGREE_TO_RADIAN(value);
	get_valid(scene, &value, 0.5, 89.5, "VERTICAL VIEW Angle");     /* Degrees */
	view_angle_y = DEGREE_TO_RADIAN(value);
	
	/* Close the file */
	fclose(scene);

}	/* read_view_info() */



/*****************************************************************************\
* procedure make_working_directory                                            *
*                                                                             *
* Purpose: This procedure makes a working directory, given the volume refnum  *
*          and the directory id.                                              *
*                                                                             *
* Parameters: vrefnum: the volume refnum                                      *
*             dir_id:  hard directory id.                                     *
*             returns working directory id                                    *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 7, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

short	make_working_directory(short vrefnum, long dir_id)
{

	WDPBRec	pb;
	short	error;

	pb.ioNamePtr = "\p";
	pb.ioWDProcID = 'RTRC';
	pb.ioWDDirID = dir_id;
	pb.ioVRefNum = vrefnum;
	error = PBOpenWD (&pb, FALSE);
	if (error)
		terminal_startup_error(error);
	
	/* Find the preferences working directory id */
	return (pb.ioVRefNum);
	
}	/* make_working_directory() */



