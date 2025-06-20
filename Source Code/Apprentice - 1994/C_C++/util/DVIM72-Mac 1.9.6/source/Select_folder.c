#include <Script.h>
#include "mac-specific.h"
Boolean TrapAvailable( short trapnum );
void Param_string( StringPtr out, StringPtr format,
	StringPtr p0, StringPtr p1, StringPtr p2 );

#define SFGETFOLDER_DLOG_ID		4045
#define GESTALT			0xA1AD

#define DEBUG 0

#if DEBUG
#define CKPT(x)		DebugStr("\p" x);
#define ASSERT(x,y)		if (!(x)) {DebugStr("\p" y);}
#else
#define CKPT(x)
#define ASSERT(x,y)
#endif

void Select_folder( char *folder_path );

static pascal short dialog_hook( short item, DialogPtr dialog_ptr );
static void Get_folder_pathname ( StringPtr PathNamePtr);

Boolean g_folder_selected;
long	g_folder_dirID;
short	g_folder_vRefNum;

ControlHandle	g_folder_button;
SFReply		g_reply;	/* global so the dialog hook can see it */

/* ------------------------- Select_folder ---------------------------- */
void Select_folder( char *folder_path )
{
	Point		where;
	CursHandle	watch_cursor;
	
	CKPT( "Select_folder start" );
	g_folder_selected = false;
	g_folder_dirID = NIL;
	g_folder_button = NIL;
	g_folder_vRefNum = -SFSaveDisk;
	where = Get_SF_place( SFGETFOLDER_DLOG_ID );
	SFPGetFile(where, "\pfile:", nil, -1, nil,
		(ProcPtr)dialog_hook, &g_reply, SFGETFOLDER_DLOG_ID, nil );
	if (g_folder_selected)
	{
		CKPT( "Select_folder selected" );
		watch_cursor = GetCursor( watchCursor );
		SetCursor( *watch_cursor );
		Get_folder_pathname( (StringPtr) folder_path );
		InitCursor();
		if (folder_path[0] == 0)
			Show_error( "Sorry, full pathname too long." );
		else
			PtoCstr( (StringPtr) folder_path );
	}
	else
		folder_path[0] = 0;
	CKPT( "Select_folder end" );
}




/* --------------------- Get_folder_pathname ----------------- */
/* 
	Use the low-memory globals set by Standard File,
	SFSaveDisk = negative of last volume reference number,
	CurDirStore = last directory ID
	to find the full path name of the selected folder.
*/
static void Get_folder_pathname ( StringPtr PathNamePtr)
{
Str255	ParVolName;
int err, i;
char *bufPtr, *name_ptr;
char buf[500];
Boolean		HaveAUX;
CInfoPBRec	block;

	HaveAUX = (HWCfgFlags & (1 << 9)); /* a low-mem global */

/* We'll fill in buf from right to left. */
	buf[499] = '\0';
	bufPtr = buf + 499;
	
	block.dirInfo.ioNamePtr = (StringPtr) ParVolName;
	block.dirInfo.ioDrParID = g_folder_dirID;
	block.dirInfo.ioVRefNum = g_folder_vRefNum;
	block.dirInfo.ioFDirIndex = -1;

	do {
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;

		err = PBGetCatInfo(&block, FALSE);
		if (HaveAUX) {
			if (ParVolName[1] != '/')
				/* If this isn't root (i.e. '/'), append a slash ('/') */
				*(--bufPtr) = '/';
		} else 
			/* Append a Macintosh style colon (':') */
			*(--bufPtr) = ':';
		i = (unsigned char) ParVolName[0]; /* length of name */
		name_ptr = (char *)ParVolName + i + 1;
		for (; i; i--)
			*(--bufPtr) = *(--name_ptr);
	} while (block.dirInfo.ioDrDirID != 2);

	if ( (buf + 499) - bufPtr > 255 )
	{
		PathNamePtr[0] = 0;	/* string too long; return null string */
	}
	else /* copy all, including final colon */
	{
		PathNamePtr[0] = (buf + 499) - bufPtr;
		BlockMove( bufPtr, &PathNamePtr[1], (buf + 499) - bufPtr );
	}
} /* Get_folder_pathname */

#define FOLDER_BUTTON	11

/* -------------------------- dialog_hook ---------------------------- */
static pascal short dialog_hook( short item, DialogPtr dialog_ptr )
{
	short	retval;
	short	itype;
	Rect	box;
	long	this_dirID;
	short	this_vRefNum;
	Str255	button_title, folder_name;
	CInfoPBRec	block;
	OSErr	err;
	static Boolean trunc_folder;
	static short	title_room;
	static Str255	folder_name_format;
	long	response;
	
	retval = item;
	if ( (g_reply.fName[0] == 0) && (g_reply.fType != NIL) )
		this_dirID = g_reply.fType;
	else
		this_dirID = CurDirStore;
	this_vRefNum = -SFSaveDisk;
	switch (item)
	{
		case sfHookFirstCall:
			GetDItem( dialog_ptr, FOLDER_BUTTON, &itype,
				(Handle *) &g_folder_button, &box );
			GetCTitle( g_folder_button, folder_name_format );
			title_room = (box.right - box.left)
				- StringWidth(folder_name_format);
			trunc_folder = TrapAvailable(GESTALT) &&
				(Gestalt(gestaltScriptMgrVersion,&response) == noErr) &&
				(response >= 0x0700L);
			/*
				Note: I purposely let this case fall through to the
				null event case, so the buttons title will be set sooner.
			*/
		case sfHookNullEvent:
			if ( (this_dirID != g_folder_dirID) /* update button title */
				|| (this_vRefNum != g_folder_vRefNum) )
			{
				g_folder_dirID = this_dirID;
				g_folder_vRefNum = this_vRefNum;
				block.dirInfo.ioCompletion = NIL;
				block.dirInfo.ioNamePtr = folder_name;
				block.dirInfo.ioFDirIndex = -1;
				block.dirInfo.ioVRefNum = g_folder_vRefNum;
				block.dirInfo.ioDrDirID = g_folder_dirID;
				err = PBGetCatInfo( &block, false );
				if (err == noErr)
				{
					if (trunc_folder)
							(void) TruncString( title_room, folder_name,
							smTruncMiddle );
					Param_string( button_title, folder_name_format,
						folder_name, NIL, NIL );
					SetCTitle( g_folder_button, button_title );
				}
			}
			break;
		case FOLDER_BUTTON:
			g_folder_selected = TRUE;
			retval = getCancel; /* return fake Cancel to dismiss the dialog */
			break;
		case getOpen:
			SysBeep(1);
			retval = sfHookNullEvent;
			break;
	}

	return retval;
}

