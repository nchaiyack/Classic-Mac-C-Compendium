/* -*-C-*- special.h */
/*-->special*/
/**********************************************************************/
/****************************** special *******************************/
/**********************************************************************/

/*
	In DVIM72-Mac, we implement \special to include PICT files.
	I use as an outline the procedure in Inside Macintosh, V-88.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "m72.h"
#include "egblvars.h"
#include "mac-specific.h"
#include "Scale_rect.h"
#include "Parse_special.h"



/* --------- Globals ----------- */
short		g_PICT_refnum;
Str255	g_PICT_filename;

/* ---------- Prototypes -------- */
void Handle_io_error( OSErr err, Str255 fn );
pascal void Get_PICT_data( Ptr data_ptr, int byte_count );

#define HEADER_SIZE		512
#define HEADER_TOO_SMALL	5000
#define OUT_OF_MEMORY		5001
#define CANT_READ_PICT		5002
#define SCREEN_DPI			72

/* ---------------------- special ---------------------------------- */
void
special(s)			/* process TeX \special{} string in s[] */
register char *s;
{
	GrafPtr save_port;
	Rect	dest_rect, print_dest, preview_dest;
	int		i;
	OSErr	err;
	FInfo	finder_info;
	long	pict_size;
	char	pict_header[HEADER_SIZE];
	PicHandle	the_pict;
	float	hscale, vscale;
	short	keytype;

	for (i = 0; i < strlen(s); i++)
		tolower( s[i] );
	
	s = Get_keyword( s, &keytype );
	if (keytype != sp_pict)
		return;					/* Not our type of \special */
	s = Get_filename( s, g_PICT_filename );
	err = GetFInfo( g_PICT_filename, 0, &finder_info );
	if (err != noErr)
	{
		Handle_io_error( err, g_PICT_filename );
		return;
	}
	else if (finder_info.fdType != 'PICT')
		return;					/* File is not a PICT */
	err = HOpen( 0, NIL, g_PICT_filename, fsRdPerm, &g_PICT_refnum );
	if (err != noErr)
	{
		Handle_io_error( err, g_PICT_filename );
		return;
	}

	/* Skip the header */
	err = SetFPos( g_PICT_refnum, fsFromStart, HEADER_SIZE );
	if (err != noErr)
	{
		Handle_io_error( err, g_PICT_filename );
		(void) FSClose( g_PICT_refnum );
		return;
	}
	
	/* Now We're just going to read enough to get the picture frame. */
	the_pict = (PicHandle) NewHandle( sizeof(Picture) );
	if (the_pict == nil)
	{
		Handle_io_error( OUT_OF_MEMORY, g_PICT_filename );
		(void) FSClose( g_PICT_refnum );
		return;
	}
	HLock( (Handle)the_pict );
	pict_size = sizeof(Picture);
	err = FSRead( g_PICT_refnum, &pict_size, *the_pict );
	if (err != noErr)
	{
		Handle_io_error( err, g_PICT_filename );
		(void) FSClose( g_PICT_refnum );
		HUnlock( (Handle)the_pict );
		DisposHandle( (Handle)the_pict );
		return;
	}
	
	/* Scale and place the picture. */
	dest_rect = (**the_pict).picFrame;
	/*
		Make the bottom left corner be the origin. Then the values
		of right and top will be the width and height, in points.
	*/
	OffsetRect( &dest_rect, -dest_rect.left, -dest_rect.bottom );

	Get_scales( s, &hscale, &vscale, -dest_rect.top, dest_rect.right );
	
	dest_rect.right = (short)((float)dest_rect.right * (float)g_dpi
		* hscale / SCREEN_DPI + 0.5);
	dest_rect.top = (short)((float)dest_rect.top * (float)g_dpi
		* vscale / SCREEN_DPI + 0.5);
	OffsetRect( &dest_rect, hh, vv );

	GetPort( &save_port );
	if (g_draw_offscreen)
	{
		SetPort( &g_offscreen_GrafPort );
		DrawPicture( the_pict, &dest_rect );
	}
	else
	{
		Scale_rect( &dest_rect, &print_dest, &preview_dest );
		/*
			Now we're ready to draw a QuickDraw \special
		*/
		SetPort( (GrafPtr)g_print_port_p );
		DrawPicture( the_pict, &print_dest );
		
		if (g_preview)
		{
			(void) SetFPos( g_PICT_refnum, fsFromStart, HEADER_SIZE + pict_size );
			SetPort( g_page_window );
			DrawPicture( the_pict, &preview_dest );
		}
	}
	/*
		Put things back to normal.
	*/
	SetPort( save_port );
	HUnlock( (Handle)the_pict );
	DisposHandle( (Handle)the_pict );
	
	(void) FSClose( g_PICT_refnum );
}

/* -------------------------- Get_PICT_data -------------------------- */
pascal void Get_PICT_data( Ptr data_ptr, int byte_count )
{
	long	long_count;
	OSErr	err;
	
	long_count = byte_count;
	err = FSRead( g_PICT_refnum, &long_count, data_ptr );
	if (err != noErr)
		Handle_io_error( err, g_PICT_filename );
}

/* ------------------------ Handle_io_error -------------------------- */
void Handle_io_error( OSErr err, Str255 fn )
{
	char	message[256];
	
	switch (err)
	{
		case bdNamErr:
			(void)sprintf(message, "Bad file name error on '%#s'.", fn );
			break;
		case extFSErr:
			(void)sprintf(message, "External file system error on '%#s'.", fn );
			break;
		case fnfErr:
			(void)sprintf(message, "File not found error on '%#s'.", fn );
			break;
		case ioErr:
			(void)sprintf(message, "I/O error on '%#s'.", fn );
			break;
		case nsvErr:
			(void)sprintf(message, "No such volume error on '%#s'.", fn );
			break;
		case paramErr:
			(void)sprintf(message, "Negative count error on '%#s'.", fn );
			break;
		case eofErr:
			(void)sprintf(message, "End of file error on '%#s'.", fn );
			break;
		case fnOpnErr:
			(void)sprintf(message, "File not open error on '%#s'.", fn );
			break;
		case rfNumErr:
			(void)sprintf(message, "Bad reference number error on '%#s'.", fn );
			break;
		case opWrErr:
			(void)sprintf(message, "File '%#s' already open.", fn );
			break;
		case tmfoErr:
			(void)sprintf(message, "Too many files open error on '%#s'.", fn );
			break;
		case OUT_OF_MEMORY:
			(void)sprintf(message, "Out of memory reading '%#s'.", fn );
			break;
		case HEADER_TOO_SMALL:
			(void)sprintf(message, "Header size error on '%#s'.", fn );
			break;
		default:
			(void)sprintf(message, "Miscellaneous error on '%#s'.", fn );
			break;
	}
	Show_error( message );
}
