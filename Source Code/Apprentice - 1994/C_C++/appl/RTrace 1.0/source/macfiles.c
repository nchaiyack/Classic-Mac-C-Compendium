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


/* types */
typedef
struct
	{
	
	short	prefs_version;
	Rect	options_window_position;
	Rect	image_window_position;
	Rect	log_window_position;
	Rect	status_window_position;
	Rect	animation_window_position;
	Boolean show_image_rendering;
	Boolean allow_background_tasks;
	Boolean keep_image_in_memory;
	Boolean show_status_window_flag;
	Boolean show_about_window_flag;
	Boolean hide_options_window_flag;
	long	time_between_events;
	
	} preferences_struct;

#define CURRENT_PREFS_VERSION	2


/* externals */
extern Boolean		keep_image_in_memory;	/* TRUE if we keep a pixmap in memory */
extern Boolean		show_image_rendering;	/* TRUE if we show pixmap while it renders */
extern short		num_new_log_lines;		/* the number of stdout lines not yet in the log */
extern CGrafPtr		image_port;				/* offscreen port which contains the image */
extern WindowPtr	image_window;			/* the image window data structures */
extern short		SCREEN_SIZE_X_MAX;
extern short		SCREEN_SIZE_Y_MAX;
extern char			sff_filename[200];		/* pathname of file to render */
extern short		sff_file_vrefnum;		/* The volume refnum of the sff file directory */
extern DialogPtr	options_dialog;			/* the options dialog */
extern DialogPtr	preferences_dialog;		/* the preferences dialog */
extern DialogPtr	status_dialog;			/* the status dialog */
extern DialogPtr	animation_dialog;		/* the animation dialog */
extern WindowPtr	log_window;				/* the log window */
extern DialogPtr	about_dialog;			/* the about dialog */
extern DialogPtr	abort_render_dialog;	/* the Abort Render dialog */
extern DialogPtr	cancel_dialog;			/* the Cancel dialog */
extern DialogPtr	error_dialog;			/* the Error dialog */
extern DialogPtr	offer_to_abort_dialog;	/* the Offer to Abort dialog */
extern DialogPtr	saving_frame_dialog;	/* the Saving Frame dialog */
extern Boolean		allow_background_tasks;	/* TRUE if we handle events and give time to
												background processed while rendering */
extern Boolean		show_image_rendering;	/* TRUE if we show the image in a window */
extern Boolean		keep_image_in_memory;	/* TRUE if we keep a pixmap of the image in memory */
extern Boolean		show_status_window_flag;/* TRUE if the status dialog is shown each render */
extern Boolean		show_about_window_flag;	/* TRUE if the about window is shown at startup */
extern Boolean		hide_options_window_flag;/* TRUE if the options dialog is hidden each render */
extern long			time_between_events;	/* number of ticks between calls to WaitNextEvent */
extern long			frames;					/* The number of frames in the animation sequence */
extern Boolean		find_folder_available;	/* TRUE if FindFolder is available */
extern Boolean		starting_up;			/* TRUE if we're still starting up */
extern Boolean		f8bit_QD_available;		/* TRUE if 8-bit QuickDraw is available */
extern Boolean		quicktime_available;	/* TRUE if QuickTime is available */


/* globals */
fpos_t			current_stdout_read_pos;	/* position we're reading in stdout */
fpos_t			current_stderr_read_pos;	/* position we're reading in stderr */
FILE			*stdout_file;				/* file to which stdout is redirected */
FILE			*stderr_file;				/* file to which stderr is redirected */
short			blessed_folder_wd_id;		/* Working Directory refnum of blessed folder */
short			temp_folder_wd_id;			/* Working Directory refnum of Temporary Items folder */
short			rtrace_wd_id;				/* Working Directory refnum of RTrace folder */
short			sff_file_wd_id;				/* Working Directory refnum of .sff file folder */

int				log_window_available=FALSE; /* Changed to TRUE after setup_log_window used to
											 *  tell DB macro defined in macerrors.h that we can 
											 *  write to the log window instead of the console.
											 */

char			rtrace_prefs_filename[] = "\pMacRTrace Preferences";

short				preferences_file_refnum;
preferences_struct	**prefs_handle;




/* Prototypes */
Boolean open_sff_file(char *pathname);
pascal Boolean filter_sff (fileParam *file_params);
long get_num_objects_sff(char *filename);
void write_rect(short refnum, Rect *rect);
void write_long(short refnum, long number);
void write_short(short refnum, short number);
void write_char(short refnum, char number);
void do_stdout_check(void);
void save_pict_file(SFReply	*my_reply);
void save_ppm_file(SFReply *my_reply);
void save_quicktime_movie(SFReply *my_reply);
void save_pict_files(SFReply *my_reply);
void delete_pict_files(SFReply *my_reply);
void stdout_to_log(void);
void add_line_to_log_window (char *line);
void preprocess_sff_file(long *num_lights, long *num_surfaces, long *num_objects);
void count_polygon(FILE *sff_file, char *this_line, long *num_objects);
void count_text3d(FILE *sff_file, char *this_line, long *num_objects);
void place_window (WindowPtr window, Rect *bounds);
void write_preferences(void);
void get_preferences(void);
void update_preferences_handle(void);
void remember_window_position(WindowPtr window, Rect *dest_rect);
void convert_ppm_to_pict(char *ppm_filename, Str255 pict_filename, short pict_vrefnum);
void handle_io_error(short error);
void setup_temp_folder(void);
void delete_temp_files(Boolean movie_too);
short make_working_directory(short vrefnum, long dir_id);
void create_about_movie(void);



/*****************************************************************************\
* procedure open_sff_file                                                     *
*                                                                             *
* Purpose: This procedure allows the user to select a .sff file using the     *
*          standard Mac file dialog.                                          *
*                                                                             *
* Parameters: pathname contains pathname of selected file, if any.            *
*             returns 0 of user cancels, 1 of not                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

Boolean open_sff_file(char *pathname)
{

	SFTypeList	types;				/* permissible file types to open */
	SFReply 	sff_file_reply;		/* data structure for file info */
	Point		where = {40, 40};	/* SF dialog location */

	types[0] = 'TEXT';

	/* Let the user choose an .sff input file.  */
	SFGetFile (where, 0L, filter_sff, 1, types, 0L, &sff_file_reply);

	/* If the user cancelled, return 1 */
	if (!sff_file_reply.good)
		return 0;

	/* Set the default directory to the .sff directory */
	sff_file_wd_id = sff_file_reply.vRefNum;
	SetVol ((StringPtr) NULL, sff_file_wd_id);

	/* Get the filename of this .sff file */
	strcpy (pathname, PtoCstr(sff_file_reply.fName));

	/* no problems */
	return 1;
	
}	/* open_sff_file() */



/*****************************************************************************\
* procedure filter_sff                                                        *
*                                                                             *
* Purpose: This procedure filters files in the standard file dialog so only   *
*          those ending in .sff appear.                                       *
*                                                                             *
* Parameters: file_params contains a description of one file                  *
*             returns FALSE if the file should be displayed.                  *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 21, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

pascal Boolean filter_sff (fileParam *file_params)
{

	/* Get the filename address and length */
	char *name = (char *) file_params->ioNamePtr;
	int length = name[0];
	
	/* check that the last four characters are ".sff" */
	if ((name[length - 3] == '.') &&
		(name[length - 2] == 's') &&
		(name[length - 1] == 'f') &&
		(name[length] == 'f'))
		return FALSE;
	else return TRUE;

}	/* filter_sff() */



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

	/* Convert ~rttemp.ppm into a PICT file */
	convert_ppm_to_pict("~~rttemp.ppm", my_reply->fName, my_reply->vRefNum);

}	/* save_pict_file() */



/*****************************************************************************\
* procedure convert_ppm_to_pict                                               *
*                                                                             *
* Purpose: This procedure converts a .ppm file to a PICT file.                *
*			We are not going thru offscreen GWorlds but are trying to save    *
*			memory by reading and compacting the .ppm file on the fly.        *
* Note:    this code will work only for .ppm created by RTrace (see comment   *
*			in code reading in .ppm header).                                  *
*                                                                             *
* Parameters: ppm_filename:  the filename of the ppm file to convert          *
*             pict_filename: the filename to of the PICT file to create       *
*             pict_vrefnum:  the vrefnum of the PICT file directory           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 9, 1992                                               *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
*   Reid Judd  9/18/92            numerous changes for V2 packed pict files   *
\*****************************************************************************/

void convert_ppm_to_pict(char *ppm_filename, Str255 pict_filename, short pict_vrefnum)
{

	EventRecord		event;
	short			pict_file_refnum;			/* refnum of output file */
	short			error;
	char			*zeros_buffer;				/* pointer to buffer of zeros */
	long			scan_line_buffer_size;
	long			num_bytes;
	short 			row_bytes;					/* number of bytes per row */
	long			pict_file_length;
	Rect			pict_frame;
	FILE			*source;
	short			lines_found;
	char			this_char;
	Rect			huge_rect = {-32767, -32767, 32767, 32767};
	long			component_buffer_size;
	unsigned char	**unpacked_buffer_handle;
	unsigned char	**packed_buffer_handle;
	unsigned char	**red_buffer_handle;
	unsigned char	**green_buffer_handle;
	unsigned char	**blue_buffer_handle;
	unsigned char	*unpacked_buffer;
	unsigned char	*packed_buffer;
	unsigned char	*source_buffer;
	unsigned char	*dest_buffer;
	unsigned char	*red_buffer;
	unsigned char	*green_buffer;
	unsigned char	*blue_buffer;
	long			i, j;
	long			read_component_bytes;

	/* Create and open the ouput file for writing */
	FSDelete (pict_filename, pict_vrefnum);
	error = Create (pict_filename, pict_vrefnum, 'ttxt', 'PICT');
	if (error) abortive_error(error);

	error = FSOpen (pict_filename, pict_vrefnum, &pict_file_refnum);
	if (error) abortive_error(error);

	/* Find the image size for later use */
	pict_frame.left = pict_frame.top = 0;
	pict_frame.bottom = SCREEN_SIZE_Y_MAX - 1;
	pict_frame.right = SCREEN_SIZE_X_MAX - 1;

	/* Find number of bytes per row */
    row_bytes = 4*(SCREEN_SIZE_X_MAX - 1);

	/* Create a 512 byte buffer of zeros for write_zeros */
	zeros_buffer = NewPtrClear (512);
	if (error = MemError()) abortive_error(error);
	num_bytes = 512;
	error = FSWrite (pict_file_refnum, &num_bytes, zeros_buffer);
	if (error) abortive_error(error);
	DisposPtr (zeros_buffer);

	/* Write all the header data.  This is taken straight from
		IM VI, 17-24, 2nd edition june'91. */
	write_short(pict_file_refnum, 0);			/* Write zero picSize (fill in later) */
	write_rect (pict_file_refnum, &pict_frame);	/* Write the picture Frame (8 bytes) */
	write_long(pict_file_refnum, 0x001102FF);	/* Write the version information */
	write_short(pict_file_refnum, 0x0C00);		/* Write a Header opcode */

	write_short(pict_file_refnum, 0xFFFE);	    /* Write -2 to indicate v2 picture */
	write_short(pict_file_refnum, 0x0000);	    /* Write 0 to reserved value */
	write_long(pict_file_refnum, (0x00480000) );  /* Native hRes (72dpi) */
	write_long(pict_file_refnum, (0x00480000) );  /* Native vRes (72dpi) */
	write_rect (pict_file_refnum, &pict_frame);	/* Write srcRect (8 bytes) */
	write_long(pict_file_refnum, 0);			/* Write a 0 to the reserved area */
	write_short(pict_file_refnum, 0x001E);		/* Write DefHilite opcode */
	write_short(pict_file_refnum, 0x0001);		/* Write Clip opcode */
	write_short(pict_file_refnum, 0x000A);
	write_rect (pict_file_refnum, &huge_rect);	/* define clipping to be all QDSpace */
	write_short(pict_file_refnum, 0x009A);		/* Write a DirectBitsRect opcode */
	write_long (pict_file_refnum, 0x000000FF);	/* Write to baseAddr field */
	write_short(pict_file_refnum, row_bytes | 0x8000);  /* Write bytes per row */

	write_rect (pict_file_refnum, &pict_frame);	/* Write the source rectangle */
	write_short(pict_file_refnum, 0);			/* Write pixel map version */
	write_short(pict_file_refnum, 4);			/* Write packing type (RLE) */
	write_long(pict_file_refnum, 0);			/* Write packed data size (0 for RLE) */
	write_long(pict_file_refnum, 0x00480000);	/* Write a horizontal resolution (72 dpi) */
	write_long(pict_file_refnum, 0x00480000);	/* Write a vertical resolution (72 dpi) */
	write_short(pict_file_refnum, RGBDirect);	/* Write direct pixel mode */
	write_short(pict_file_refnum, 32);			/* Write bits per pixel */
	write_short(pict_file_refnum, 3);			/* Write color components per pixel */
	write_short(pict_file_refnum, 8);			/* Write color component size */
	write_long(pict_file_refnum, 0);			/* Write 0 to plane longword */
	write_long(pict_file_refnum, 0x645970);		/* Write 0x645970 because it works! */
	write_short(pict_file_refnum, 0);			/* Write 0 to color table flags word */
	write_short(pict_file_refnum, 0);			/* Write 0 to color table size word */
	write_rect (pict_file_refnum, &pict_frame);	/* Write the source rectangle */
	write_rect (pict_file_refnum, &pict_frame);	/* Write the source rectangle */
	write_short(pict_file_refnum, srcCopy);		/* Write transfer mode */

	/* Open the temporary .ppm file (in the Temporary Items directory),
		and transfer the picture data to the PICT file. */
	error = SetVol( (StringPtr) NULL, temp_folder_wd_id);
	if (error) abortive_error(error);
	source = fopen (ppm_filename, "rb");	/* open to read binary */
	
	/* Ignore the header.  We know the header is over when we find the 4th newline */
	lines_found = 0;
	while (lines_found < 4)
		{
		fread (&this_char, 1, 1, source);
		if (this_char == '\n') lines_found++;
		}
	
	/* Create buffers for the ppm scan line, for each color component, and for
		the RLE-compressed component */
	scan_line_buffer_size = 3*(SCREEN_SIZE_X_MAX-1);
	component_buffer_size = SCREEN_SIZE_X_MAX-1;

	/* Allocate ppm scan line buffer */
	unpacked_buffer_handle = (unsigned char **) NewHandle(scan_line_buffer_size);
	if (error = MemError()) abortive_error(error);
	HLock(unpacked_buffer_handle);
	unpacked_buffer = *unpacked_buffer_handle;

	/* Allocate RLE-compressed component buffer */
	packed_buffer_handle = (unsigned char **) NewHandle(scan_line_buffer_size);
	if (error = MemError()) abortive_error(error);
	HLock(packed_buffer_handle);
	packed_buffer = *packed_buffer_handle;
	
	/* Allocate red component buffer */
	red_buffer_handle = (unsigned char **) NewHandle(component_buffer_size);
	if (error = MemError()) abortive_error(error);
	HLock(red_buffer_handle);
	red_buffer = *red_buffer_handle;

	/* Allocate green component buffer */
	green_buffer_handle = (unsigned char **) NewHandle(component_buffer_size);
	if (error = MemError()) abortive_error(error);
	HLock(green_buffer_handle);
	green_buffer = *green_buffer_handle;

	/* Allocate blue component buffer */
	blue_buffer_handle = (unsigned char **) NewHandle(component_buffer_size);
	if (error = MemError()) abortive_error(error);
	HLock(blue_buffer_handle);
	blue_buffer = *blue_buffer_handle;

 	/* copy the file one line at a time, compressing as we go */
	while (1)
		{
		
		/* Give background processes a little time, but don't accept any events */
		WaitNextEvent (0, &event, 0, (RgnHandle) 0L);
	
		/* Read a line */
		num_bytes = fread (unpacked_buffer, 1, scan_line_buffer_size, source);
		
		/* If there's no more, we're done */
		if (feof(source))
			break;
		
		/* Extract the components */
		for (i = 0, j = 0; i < component_buffer_size; i++, j += 3)
			{
			red_buffer[i] = unpacked_buffer[j];
			green_buffer[i] = unpacked_buffer[j+1];
			blue_buffer[i] = unpacked_buffer[j+2];
			}
		
		/* compress the red component */
		source_buffer = red_buffer;
		dest_buffer = packed_buffer;
		PackBits(&source_buffer, &dest_buffer, component_buffer_size);
		
		/* compress the green component */
		source_buffer = green_buffer;
		PackBits(&source_buffer, &dest_buffer, component_buffer_size);
		
		/* compress the blue component */
		source_buffer = blue_buffer;
		PackBits(&source_buffer, &dest_buffer, component_buffer_size);
		
		/* Write the compressed line size (a byte if the number of bytes in
			scan line is < 250, or a word otherwise */
		read_component_bytes = dest_buffer - packed_buffer;

		if (row_bytes > 250) 
			write_short(pict_file_refnum, read_component_bytes);
		else
			write_char(pict_file_refnum, read_component_bytes);
  		
		/* write the compressed scan line */
		error = FSWrite (pict_file_refnum, &read_component_bytes, packed_buffer);
		if (error) abortive_error(error);
		}

	/* Free up the buffers */
	DisposHandle(unpacked_buffer_handle);
	DisposHandle(packed_buffer_handle);
	DisposHandle(red_buffer_handle);
	DisposHandle(green_buffer_handle);
	DisposHandle(blue_buffer_handle);

	/* Write a EndOfPicture opcode, word-aligned */
	error = GetFPos (pict_file_refnum, &pict_file_length);
	if (error) abortive_error(error);
	if (pict_file_length & 1)
		{
		write_char(pict_file_refnum, 0);	/* pad with zero byte */		
		pict_file_length++;
		}		
	write_short(pict_file_refnum, 0x00FF);
	
	/* Write the PICT length, back at the beginning */
	error = SetFPos (pict_file_refnum, fsFromStart, 512);
	if (error) abortive_error(error);
	write_short(pict_file_refnum, (short) (pict_file_length - 512 + 2) );
	
	/* Close the .ppm file */
	fclose(source);
	
	/* Close the output file */
	error = FSClose (pict_file_refnum);
	if (error) abortive_error(error);

}	/* save_pict_file() */



void write_short(short refnum, short number)
{
	long	num_bytes = 2;
	short	error;
	
	error = FSWrite (refnum, &num_bytes, &number);
	if (error) abortive_error(error);
}


void write_char(short refnum, char number)
{
	long num_bytes = 1;
	short	error;
	
	error = FSWrite (refnum, &num_bytes, &number);
	if (error) abortive_error(error);

}


void write_long(short refnum, long number)
{
	long num_bytes = 4;
	short	error;
	
	error = FSWrite (refnum, &num_bytes, &number);
	if (error) abortive_error(error);
}


void write_rect(short refnum, Rect *rect)
{
	long num_bytes = 8;
	short	error;
	
	error = FSWrite (refnum, &num_bytes, rect);
	if (error) abortive_error(error);
}



/*****************************************************************************\
* procedure save_ppm_file                                                     *
*                                                                             *
* Purpose: This procedure creates a ppm file.  It does this by copying the    *
*          temporary ppm file.                                                *
*                                                                             *
* Parameters: my_reply: the result of the SPutFile call                       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 26, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void save_ppm_file(SFReply *my_reply)
{

	FILE	*source;
	char	*buffer;			/* buffer for file data */
	long	copy_buffer_size;
	short	ppm_file_refnum;
	short	error;
	long	count;

	/* Open the input file for reading (in the Temporary Items directory) */
	SetVol( (StringPtr) NULL, temp_folder_wd_id);
	source = fopen ("~~rttemp.ppm", "rb");	/* open to read binary */
	
	/* Create and open the ouput file for writing */
	error = FSDelete (my_reply->fName, my_reply->vRefNum);
	error = Create (my_reply->fName, my_reply->vRefNum, 'RTRC', '.ppm');
	if (error) abortive_error(error);
	error = FSOpen (my_reply->fName, my_reply->vRefNum, &ppm_file_refnum);
	if (error) abortive_error(error);
	
	/* Create the largest possible file buffer */
	copy_buffer_size = MaxBlock ();
	buffer = NewPtr(copy_buffer_size);
	
	/* copy data from source to dest in blocks of copy_buffer_size */
	while (!feof(source))
		{
		count = fread (buffer, 1, copy_buffer_size, source);
		FSWrite (ppm_file_refnum, &count, buffer);
		if (error) abortive_error(error);
		}
		
	/* Free up the file buffer */
	DisposPtr(buffer);

	/* Close the input file */
	fclose(source);
	
	/* Close the output file */
	FSClose (ppm_file_refnum);
	if (error) abortive_error(error);
	
}	/* save_ppm_file() */



/*****************************************************************************\
* procedure save_pict_files                                                   *
*                                                                             *
* Purpose: This procedure save the animation sequence as a series of PICT     *
*          files.  It does this by copying the PPM files.                     *
*                                                                             *
* Parameters: my_reply: the result of the SPutFile call                       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 9, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void save_pict_files(SFReply *my_reply)
{

	long	i;
	Str255	pict_filename;
	char	ppm_filename[255];
	
	/* Convert the chosen filename for the PICT files to a C string */
	PtoCstr(my_reply->fName);

	/* Show the saving frame dialog */
	ShowWindow(saving_frame_dialog);
	SelectWindow(saving_frame_dialog);

	/* Loop through all the "~~rttempn.ppm" files, and convert each one to
		a PICT file */
	for	(i = 1; i <= frames; i++)
		{

		/* Update the text in the saving frame dialog */
		NumToString(i, pict_filename);
		ParamText(pict_filename, 0, 0, 0);
		DrawDialog(saving_frame_dialog);

		/* Find the filename of this ppm file */
		sprintf(ppm_filename, "~~rttemp%ld.ppm", i);
		
		/* Find the filename of this PICT file */
		sprintf((char *) pict_filename, "%s.%ld", my_reply->fName, i);
		CtoPstr(pict_filename);
	
		/* Convert the ppm into a PICT file */
		convert_ppm_to_pict(ppm_filename, pict_filename, my_reply->vRefNum);
				
		}

	/* Convert the filename back to a P string */
	CtoPstr(my_reply->fName);

	/* Hide the saving frame dialog */
	HideWindow(saving_frame_dialog);

}	/* save_pict_files() */

/*****************************************************************************\
* procedure delete_pict_files                                                 *
*                                                                             *
* Purpose: This procedure removes the sequence of PICT files that were used   *
*          to form the QuickTime movie and are therefore, no longer useful.   *
*																			  *
*		   However, we may replace this soon with a new procedure that will   *
*		   create each individual pict file, append it to a movie, and then   *
*          delete it on the fly.                                              *
*                                                                             *
* Parameters: my_reply: the result of the SPutFile call                       *
*                                                                             *
* Created by: Reid Judd                                                       *
* Created on: September 28, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

void delete_pict_files(SFReply *my_reply)
{

	long	i;
	Str255	pict_filename;
	char	ppm_filename[255];
	short   error;
	
	/* Convert the chosen filename for the PICT files to a C string */
	PtoCstr(my_reply->fName);
 
	/* Loop through all the pict files, and delete each one */
 	for	(i = 1; i <= frames; i++)
		{
		/* Find the filename of this PICT file */
		sprintf((char *) pict_filename, "%s.%ld", my_reply->fName, i);
		CtoPstr(pict_filename);
		error = FSDelete ( pict_filename, my_reply->vRefNum); 
			
		if (error) abortive_error(error);

		}

	/* Convert the filename back to a P string */
	CtoPstr(my_reply->fName);

}	/* delete_pict_files() */



/*****************************************************************************\
* procedure save_quicktime_movie                                              *
*                                                                             *
* Purpose: This procedure saves the animation sequence as a quicktime movie.  *
*                                                                             *
* Parameters: my_reply: the result of the SPutFile call                       *
*                                                                             *
* Created by: Reid Judd                                                       *
* Created on: September 21, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

void save_quicktime_movie(SFReply *my_reply)
{
  save_pict_files( my_reply );

  convert_picts_to_movie( my_reply );
  
  /* Now delete the pict files that were created. */
  delete_pict_files( my_reply );
	
}	/* save_quicktime_movie() */



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
	strcpy ((char *) fsspec.name, (char *) "~~rtrace_movie");
	CtoPstr ((char *) fsspec.name);

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
*   WHO          WHEN             WHAT                                        *
\*****************************************************************************/

void delete_temp_files(Boolean movie_too)
{

	short		index = 1;
	short		error;
	HFileInfo	CPB;
	char		filename[256];
	long		dirid;
	long		proc_id;

	/* Set up filename buffer */
	CPB.ioNamePtr = (unsigned char *) filename;

	/* Find the volume and directory id of the Temporary directory */
	error = GetWDInfo (temp_folder_wd_id, &(CPB.ioVRefNum), &dirid, &proc_id);
	if (error) terminal_startup_error(error);

	/* Scan through all files in the directory */
	do
		{
		
		/* Set index to point to current file */
		CPB.ioFDirIndex = index;	

		/* Set directory to search */
		CPB.ioDirID = dirid;
		
		/* Get the info on this file */
		error = PBGetCatInfo(&CPB, FALSE);
		if (error) continue;
	
		/* Convert filename to a C string */
		PtoCstr(filename);

		/* Is this the movie file, and if so, are we supposed to delete it? */
		if ( (movie_too && !strcmp(filename, "rtrace_movie"))
		
		/* Or is it a temporary image file? */
			|| ((filename[0] == '~') && (filename[1] == '~')) )
			
			/* It's our file-- delete it */
			{
			CtoPstr(filename);
			error = FSDelete (CPB.ioNamePtr, temp_folder_wd_id);
			if (error) terminal_startup_error(error);
			}
		
		/* Go to next file */
		else
			index++;
		
		}
	while (!error);

}	/* delete_temp_files() */



/*****************************************************************************\
* procedure preprocess_sff_file                                               *
*                                                                             *
* Purpose: This procedure quickly scans through a .sff file, grabbing the     *
*          most important information, and ignoring the actual scene          *
*          description.                                                       *
*                                                                             *
* Parameters: num_lights: receives the number of lights.                      *
*             num_surfaces: receives the number of surfaces.                  *
*             num_objects:  receives the number of objects.                   *
*             also sets globals look, eye, up, view_angle_x, view_angle_y     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 3, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void preprocess_sff_file(long *num_lights, long *num_surfaces, long *num_objects)
{

	short	i;
	char	this_line[200];
	FILE	*scene;
	Boolean	is_polygon;
	double	value;
	
	/* Initialize the number of lights, surfaces, and objects to zero */
	*num_lights = *num_surfaces = *num_objects = 0;

	/* Open the sff file for reading */
	scene = fopen(sff_filename, "r");

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
	gaze_distance = LENGTH(gaze);

	/* If look point is on top of eye, point, gaze is null-- error */
	if (gaze_distance < ROUNDOFF)
		runtime_abort("EYE POINT equal to LOOK POINT");

	/* No problem */
	NORMALIZE(gaze);

	/* Get up vector */
	get_valid(scene, &value, X_MIN, X_MAX, "UP VECTOR X");
	up.x = value;
	get_valid(scene, &value, Y_MIN, Y_MAX, "UP VECTOR Y");
	up.y = value;
	get_valid(scene, &value, Z_MIN, Z_MAX, "UP VECTOR Z");
	up.z = value;

	/* if up vector is too null, error */
	if (LENGTH(up) < ROUNDOFF)
		runtime_abort("no UP VECTOR");

	/* No problem */
	NORMALIZE(up);

	/* Check for bad up vector */
	if (ABS(DOT_PRODUCT(gaze, up)) > COS(ANGLE_MIN))
		runtime_abort("bad UP VECTOR");

	/* Get view angles */
	ADVANCE(scene);
	get_valid(scene, &value, 0.5, 89.5, "HORIZONTAL VIEW Angle");   /* Degrees */
	view_angle_x = DEGREE_TO_RADIAN(value);
	get_valid(scene, &value, 0.5, 89.5, "VERTICAL VIEW Angle");     /* Degrees */
	view_angle_y = DEGREE_TO_RADIAN(value);

	/* Ignore the colors and the lights comment */
	for (i = 1; i <= 6; i++)
		fgets(this_line, 10000, scene);
		
	/* Count the number of lights */
	while (this_line [0] != '\n')
		{
		(*num_lights)++;
		fgets(this_line, 10000, scene);
		}

	/* Read the Surfaces comment, and the first surface description */
	fgets(this_line, 10000, scene);
	fgets(this_line, 10000, scene);

	/* Count the number of surfaces */
	while (this_line [0] != '\n')
		{
		(*num_surfaces)++;
		fgets(this_line, 10000, scene);
		}

	/* Read the Objects comment, and the first object description line */
	fgets(this_line, 10000, scene);
	fgets(this_line, 10000, scene);

	/* Count the number of objects */
	while (this_line [0] != '\n')
		{
		/* If this is an object which can have a descriptor file,
			and if the file is imbedded here, skip it */
		if (this_line[1] == ' ')

			switch (this_line[0])
				{
				case '5':	/* polygon */
				case '3':	/* patch */
				case '6':	/* triangle */
					
					/* count the number of objects in this polygon, patch, or triangle */
					count_polygon(scene, this_line, num_objects);
				
					break;
				
				case '7':	/* 3D text */
					
					/* count the number of objects in this 3D text object */
					count_text3d(scene, this_line, num_objects);
					
					break;

				default:
					
					/* It's a simple object */
					(*num_objects)++;
		
				
				}

		/* Get next object description */
		fgets(this_line, 10000, scene);

		}

}	/* preprocess_sff_file() */



/*****************************************************************************\
* procedure count_polygon                                                     *
*                                                                             *
* Purpose: This procedure counts the number of objects in a polygon,          *
*          triangle, or patch file.                                           *
*                                                                             *
* Parameters: sff_file: the sff file we're preprocessing.                     *
*             this_line: the polygon/triangle/patch descriptor line           *
*             num_objects: the number of objects counted so far.              *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 7, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void count_polygon(FILE *sff_file, char *this_line, long *num_objects)
{

	FILE	*sub_file;
	char	*subfilename;

	/* Decide whether to read from this file or from another one */
	if (strstr(this_line, "-"))
	
		/* Use this file as the file */
		sub_file = sff_file;
	
	else	/* description is in another file */
		{
		
		/* get the filename to use */
		subfilename = (strrchr(this_line, ' ') + 1);
		
		/* Open the file */
		sub_file = fopen (subfilename, "r");

		}
	
	/* Ignore the object descriptor line */
	fgets(this_line, 10000, sub_file);
	
	/* Count the number of objects here */
	while (this_line [0] != '\n')
		{
		(*num_objects)++;
		fgets(this_line, 10000, sub_file);
		}
	
	/* If this was a separate file, we're done */
	if (sff_file != sub_file)
		fclose (sub_file);

	else	/* skip the rest */
		{
		
		/* Read the blank line */
		fgets(this_line, 10000, sub_file);
	
		/* Skip the vertex info (read until the first blank line) */
		while (!feof(sub_file) && (this_line [0] != '\n'))
			fgets(this_line, 10000, sub_file);
		
		}

}	/* count_polygon() */



/*****************************************************************************\
* procedure count_text3d                                                      *
*                                                                             *
* Purpose: This procedure counts the number of objects in a 3D text object.   *
*                                                                             *
* Parameters: sff_file: the sff file we're preprocessing.                     *
*             this_line: the text object descriptor line                      *
*             num_objects: the number of objects counted so far.              *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 7, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void count_text3d(FILE *sff_file, char *this_line, long *num_objects)
{

	FILE	*sub_file;
	char	*subfilename;
	char	*text_string;
	short	i;

	/* Decide whether to read from this file or from another one */
	if (strstr(this_line, "-"))
	
		/* Use this file as the file */
		sub_file = sff_file;
	
	else	/* description is in another file */
		{
		
		/* get the filename to use */
		subfilename = (strrchr(this_line, ' ') + 1);
		
		/* chop off the '\n' */
		subfilename[strlen(subfilename) - 1] = 0;
		
		/* Open the file */
		sub_file = fopen (subfilename, "r");

		}
	
	/* Run through all lines */
	while (this_line [0] != '\n')
		{
		
		/* Check whether this line has a " on it */
		if (text_string = strstr (this_line, "\""))
			{
			
			/* move pointer to right after the " */
			text_string++;

			/* there are as many objects as non-space characters */
			i = 0;
			while (text_string[i] != '"')
				if (text_string[i++] != ' ')
					(*num_objects)++;
			
			}
		
		/* Go to next line */
		fgets(this_line, 10000, sub_file);
		
		}
	
}	/* count_text3d() */



/*****************************************************************************\
* procedure get_preferences                                                   *
*                                                                             *
* Purpose: This procedure gets the preferences from the Preferences folder in *
*          the System Folder of the startup disk.  If we are dealing with a   *
*          pre-System 7 system which does not have such a folder, we just get *
*          the preferences from the blessed folder.                           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 7, 1992                                               *
* Modified:                                                                   *
*   WHO          WHEN             WHAT                                        *
*   Greg Ferrar  9/10/92          Added support for pre-System 7 systems.     *
\*****************************************************************************/

void get_preferences(void)
{


	long	prefs_hard_dirid;
	long	num_bytes;
	short	prefs_folder_wd_id;
	short	prefs_vrefnum;
	short	error;
	
	preferences_struct *prefs_ptr;
	
	/* allocate the preferences structure */
	prefs_handle = (preferences_struct **) NewHandle(sizeof(preferences_struct));

	/* If we have FindFolder, use it.  Otherwise, use the blessed folder */
	if (find_folder_available)
		{
	
		/* Find the Preferences folder */
		error = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
							&prefs_vrefnum, &prefs_hard_dirid);
		if (error) terminal_startup_error(error);
	
		/* Make it a working directory */
		prefs_folder_wd_id = make_working_directory(prefs_vrefnum, prefs_hard_dirid);
	
		}
	else

		/* use the blessed folder */
		prefs_folder_wd_id = blessed_folder_wd_id;


	/* Open "MacRTrace Preferences" file */
	error = FSOpen (rtrace_prefs_filename, prefs_folder_wd_id, &preferences_file_refnum);
	
	/* If the file does not exist, create it */
	if (error == fnfErr)

		{
			
		/* Create and open the file */
		error = FSDelete (rtrace_prefs_filename, prefs_folder_wd_id);
		error = Create (rtrace_prefs_filename, prefs_folder_wd_id, 'RTRC', 'PREF');
		if (error) terminal_startup_error(error);
		error = FSOpen (rtrace_prefs_filename, prefs_folder_wd_id, &preferences_file_refnum);
		if (error) terminal_startup_error(error);
	
		/* Set the preferences structure according to the current preferences */
		write_preferences();
		
		}
	else if (error) terminal_startup_error(error);

		
	/* the file exists-- read the resources into memory */
		
	/* Read the preferences structure */
	SetFPos(preferences_file_refnum, 1, 0);
	num_bytes = sizeof(preferences_struct);
	FSRead(preferences_file_refnum, &num_bytes, *prefs_handle);
	
	/* lock the handle and dereference it */
	HLock(prefs_handle);
	prefs_ptr = *prefs_handle;
	
	/* Check the preferences version */
	if (prefs_ptr->prefs_version != CURRENT_PREFS_VERSION)
		{
		
		/* This an older preferences version-- delete the file and build
			a new one from defaults */
		error = FSClose (preferences_file_refnum);
		if (error) terminal_startup_error(error);
		error = FSDelete (rtrace_prefs_filename, prefs_folder_wd_id);
		if (error) terminal_startup_error(error);
		
		/* Get rid of handle, since we'll make a new one when we call ourselves */
		DisposHandle(prefs_handle);
		
		/* Call ourselves-- this will create a new preferences file */
		get_preferences();
		return;
		
		}
	
	/* Set the program defaults according to the preferences structure */
	place_window (log_window, &(prefs_ptr->log_window_position));
	place_window (options_dialog, &(prefs_ptr->options_window_position));
	place_window (status_dialog, &(prefs_ptr->status_window_position));
	if (f8bit_QD_available)
		place_window (image_window, &(prefs_ptr->image_window_position));
	place_window (animation_dialog, &(prefs_ptr->animation_window_position));
	show_image_rendering = prefs_ptr->show_image_rendering;
	allow_background_tasks = prefs_ptr->allow_background_tasks;
	keep_image_in_memory = prefs_ptr->keep_image_in_memory;
	show_status_window_flag = prefs_ptr->show_status_window_flag;
	show_about_window_flag = prefs_ptr->show_about_window_flag;
	hide_options_window_flag = prefs_ptr->hide_options_window_flag;
	time_between_events = prefs_ptr->time_between_events;
		
	/* get rid of the handle */
	HUnlock(prefs_handle);

}	/* get_preferences() */



/*****************************************************************************\
* procedure place_window                                                      *
*                                                                             *
* Purpose: This procedure moves and resizes a window to fit the passed Rect.  *
*          It ensure that the window has a dragable portion of its title bar  *
*          visible-- if not, it moves it to to main monitor.                  *
*                                                                             *
* Parameters: window: the window to move and resize                           *
*             bounds: the rectangle to move the window into.                  *
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
	if (error) terminal_startup_error(error);
	
	/* Find the preferences working directory id */
	return (pb.ioVRefNum);
	
}	/* make_working_directory() */


/*****************************************************************************\
* procedure place_window                                                      *
*                                                                             *
* Purpose: This procedure moves and resizes a window to fit the passed Rect.  *
*          It ensure that the window has a dragable portion of its title bar  *
*          visible-- if not, it moves it to to main monitor.                  *
*                                                                             *
* Parameters: window: the window to move and resize                           *
*             bounds: the rectangle to move the window into.                  *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 7, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void place_window (WindowPtr window, Rect *bounds)
{

	Rect		title_bar;		/* the rectangle enclosing the title bar of the window */
	Rect		intersection;	/* used to test rectangle intersections */
	Rect		first_bounds;	/* the boundary of the main graphics device */
	short		window_height;	/* the desired height of the window */
	short		window_width;	/* the desired width of the window */
	GDHandle	first_device;	/* the first graphics device on the the list */
	GDHandle	graphics_device;/* a graphics device on the the list */
	Boolean		enough_intersection = FALSE;
	Rect		rect_storage;
	WStateData	**wstate_handle;
	
	/* Find the first graphics device, and its bounding rectangle */
	first_device = graphics_device = GetDeviceList ();
	BlockMove(&((*first_device)->gdRect), &first_bounds, 8);

	/* If the rectangle is null, or the pointer is null it means the window
		was never shown before its position was saved, so we need to compute
		a new position. */
	if (((!bounds->right) && (!bounds->left) && (!bounds->top) && (!bounds->bottom)) ||
			!bounds)
		{

		/* Find the bounds of the window */
		bounds = &rect_storage;
		BlockMove (&(window->portRect), bounds, 8);

		/* Find the window's height and width */
		window_height = bounds->bottom - bounds->top;
		window_width = bounds->right - bounds->left;


		if (window == log_window)
						
			/* Place the window in the upper left corner */
			OffsetRect(bounds, first_bounds.left + 4, first_bounds.top + 40);

		else if (window == options_dialog)			
			
			/* Place the window at the left, right below log window */
			OffsetRect(bounds, first_bounds.left + 4, log_window->portRect.bottom + 63);
		
		else if (window == image_window)
			
			/* Place the window below the status dialog and to the right of
				the options dialog */
			OffsetRect(bounds, options_dialog->portRect.right + 10,
						status_dialog->portRect.bottom + 63);

		else if (window == status_dialog)
			
			/* Place the window at the top, to the right of the log window */
			OffsetRect(bounds, log_window->portRect.right + 10, first_bounds.top + 40);

		else if (window == animation_dialog)
			
			/* Center the window on the main screen */
			OffsetRect(bounds, (first_bounds.right + first_bounds.left)/2 - (window_width/2),
						(first_bounds.bottom + first_bounds.top)/2 - (window_height/2));

		else 	/* any other window is centered on the screen */
					
			/* Place the window in the "alert" position, in the upper center of the
				main screen */
			OffsetRect(bounds, (first_bounds.right + first_bounds.left)/2 - (window_width/2),
						(first_bounds.bottom + first_bounds.top)/3 - (window_height/2));
		
		/* Now use this new rectangle as a first approximation to place the window */
		place_window(window, bounds);	
		return;
		
		} 

	/* Find the window's height and width */
	window_height = bounds->bottom - bounds->top;
	window_width = bounds->right - bounds->left;
		
	/* Find the window's title bar */
	BlockMove(bounds, &title_bar, 8);
	title_bar.bottom = title_bar.top + 16;
	
	/* Scan through all the devices.  If there is any one device which has more than
		a 4x4 block of the title bar in it, we're okay. */
	do
		{
		SectRect (&title_bar, &((*graphics_device)->gdRect), &intersection);
		enough_intersection = (((intersection.bottom - intersection.top) >= 4) &&
								((intersection.right - intersection.left) >= 4));

		graphics_device = (GDHandle) (*graphics_device)->gdNextGD;
		}
	while (graphics_device && (!enough_intersection));

	/* If no device has enough of an intersection, move this to the main window.
		We try to place it as close as possible to its original place, but we
		ensure that it's all on the main window (if possible). */
	if (!enough_intersection)
		{
		
		/* If the window is above the screen, pin it to the top */
		if ((bounds->bottom - 4) < first_bounds.top)
			{
			bounds->top = first_bounds.top + 41;
			bounds->bottom = bounds->top + window_height;
			}
		
		/* window was below screen-- pin it to the bottom */
		else
			{
			bounds->bottom = first_bounds.bottom - 5;
			bounds->top = bounds->bottom - window_height;
			}

		/* If the window is to the left of the screen, pin it to the left */
		if ((bounds->right - 4) < first_bounds.left)
			{
			bounds->left = first_bounds.left + 5;
			bounds->right = bounds->left + window_width;
			}
		
		/* window is to the right of the screen, pin it to the right */
		else
			{
			bounds->right = first_bounds.right - 8;
			bounds->left = bounds->right - window_width;
			}
		}

	/* Save the new location in the window record */
	wstate_handle = (WStateData **) ( (WindowPeek) window )->dataHandle;
	BlockMove(bounds, &(*wstate_handle)->userState, 8);

	/* Move the window to the new location */
	MoveWindow (window, bounds->left, bounds->top, TRUE);
	
	/* Resize the window */
	SizeWindow (window, window_width, window_height, TRUE);
	
}	/* place_window() */



/*****************************************************************************\
* procedure write_preferences                                                 *
*                                                                             *
* Purpose: This procedure writes the current preferences to the Preferences   *
*          Folder in the System Folder of the startup disk.                   *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 7, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void write_preferences(void)
{

	long	num_bytes;
	short	error;

	/* Set the preferences structure according to the current preferences */
	update_preferences_handle();

	/* Write the preferences to the resource file */
	num_bytes = sizeof(preferences_struct);
	error = SetFPos(preferences_file_refnum, 1, 0);
	if (error) terminal_startup_error(error);
	error = FSWrite(preferences_file_refnum, &num_bytes, *prefs_handle);
	if (error) terminal_startup_error(error);

}	/* write_preferences() */



/*****************************************************************************\
* procedure update_preferences_handle                                         *
*                                                                             *
* Purpose: This procedure sets the preferences structure according to the     *
*          values of the current preferences.                                 *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 7, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_preferences_handle(void)
{

	preferences_struct *prefs_ptr;

	/* lock the handle and dereference it */
	HLock(prefs_handle);
	prefs_ptr = *prefs_handle;
	
	/* Create the preferences structure, using the default values */
	prefs_ptr->prefs_version = CURRENT_PREFS_VERSION;
	remember_window_position(options_dialog, &(prefs_ptr->options_window_position));
	remember_window_position(log_window, &(prefs_ptr->log_window_position));
	remember_window_position(status_dialog, &(prefs_ptr->status_window_position));
	remember_window_position(image_window, &(prefs_ptr->image_window_position));
	remember_window_position(animation_dialog, &(prefs_ptr->animation_window_position));
	prefs_ptr->show_image_rendering = show_image_rendering;
	prefs_ptr->allow_background_tasks = allow_background_tasks;
	prefs_ptr->keep_image_in_memory = keep_image_in_memory;
	prefs_ptr->show_status_window_flag = show_status_window_flag;
	prefs_ptr->show_about_window_flag = show_about_window_flag;
	prefs_ptr->hide_options_window_flag = hide_options_window_flag;
	prefs_ptr->time_between_events = time_between_events;

	/* Unlock the handle */
	HUnlock(prefs_handle);

}



/*****************************************************************************\
* procedure remember_window_position                                          *
*                                                                             *
* Purpose: This procedure copies the current position of a window into a      *
*          rectangle structure, to save for later restoration.                *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 7, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void remember_window_position(WindowPtr window, Rect *dest_rect)
{

	WStateData **wstate_handle;

	/* If this is the image window, and if we're running in 1-bit QD, don't
		even try to save it-- it doesn't exist */
	if ((window == image_window) && (!f8bit_QD_available))
		
		dest_rect->left = dest_rect->right = dest_rect->top = dest_rect->bottom = 0;

	else
		{

		/* Get the Window State data */
		wstate_handle = (WStateData **) ( (WindowPeek) window )->dataHandle;

		/* Copy it to the rectangle */
		BlockMove(&(*wstate_handle)->userState, dest_rect, 8);

		}

}
