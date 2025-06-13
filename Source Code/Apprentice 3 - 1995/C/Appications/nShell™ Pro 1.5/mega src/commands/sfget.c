/* ========== the commmand file: ==========

	sfget.c
	
	Copyright (c) 1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#ifdef __MWERKS__			// Get the appropriate A4 stuff
#include <A4Stuff.h>
#else
#include <SetUpA4.h>
#endif

#include <GestaltEqu.h>
#include <Folders.h>

#include "nshc.h"
#include "str_utl.proto.h"
#include "nshc_utl.proto.h"

/* ======================================== */

// constants

#define SFGetDialog		16000
#define	SFGetButton		10

/* ======================================== */

// typedefs

typedef struct {
	StandardFileReply	*replyPtr;
	FSSpec				oldSelection;
	} SFData, *SFDataPtr;

/* ======================================== */

// globals

Boolean	gHasFindFolder;
short	gFileRef;
FSSpec	gDeskFolderSpec;
Str255	gPath;
Str32	gVarName;

/* ======================================== */

// prototypes - for local use only

int     sfget( void );
void    sfget_button(StringPtr selName,DialogPtr theDlg);
void    sfget_close_rsrc(void);
OSErr   sfget_ensure_name(FSSpec *fSpec);
int     sfget_gestalt( OSType selector, int bit );
OSErr   sfget_get_desktop_folder(short vRefNum);
int     sfget_init(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
int     sfget_open_rsrc(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls);
int     sfget_path( StandardFileReply *sfReply );
int     sfget_pre7( void );
Boolean sfget_same_file(FSSpec *file1,FSSpec *file2);
pascal  short sfget_hook(short item,DialogPtr theDlg,Ptr userData);

/* ======================================== */

// utility


int	sfget_pre7( void )
{
	long	response;
	
	if ( Gestalt( 'sysv', &response ) )
		return( 1 );
		
	if ( response < 0x700 )
		return( 1 );
	else
		return( 0 );
}

int sfget_path( StandardFileReply *sfReply )
{
	CInfoPBRec	block;
	int			error;
	Str255		temp;

	error = 0;
	gPath[0] = 0;
	temp[0] = 0;
	block.dirInfo.ioNamePtr = temp;
	block.dirInfo.ioDrParID = sfReply->sfFile.parID;
	
	if ( sfReply->sfFile.parID != 1)
		do {
		
			block.dirInfo.ioVRefNum = sfReply->sfFile.vRefNum;
			block.dirInfo.ioFDirIndex = -1;
			block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;
	
			error = PBGetCatInfo(&block,false);
			
			if ( temp[0] + gPath[0] + 1 > 255 )
				error = 1;
			
			if (!error) {
				temp[ ++temp[0] ] = ':';
				pStrAppend(temp,gPath);
				pStrCopy(gPath,temp);
				}
	
		} while (!error && (block.dirInfo.ioDrDirID != fsRtDirID));

	if ( error )
		return( error );

	if ( gPath[0] + sfReply->sfFile.name[0] > 255 )
		error = 1;
	else
		pStrAppend( gPath, sfReply->sfFile.name );

	if ( sfReply->sfFile.parID == 1 )
		if ( gPath[0] < 255 )
			gPath[ ++gPath[0] ] = ':';
		else
			error = 1;

	return( error );
}

/* ======================================== */

int	sfget_gestalt( OSType selector, int bit )
{
	long	response;
	OSErr	error;
	
	if ( error = Gestalt( selector, &response ) )
		return(false);
		
	if ( response & ( 1L << bit ) )
		return(true);
	else
		return(false);
}

/* ======================================== */

OSErr sfget_ensure_name(FSSpec *fss)
{
	DirInfo infoPB;
	OSErr err;

	if (fss->name[0] != '\0')
		return( 0 );
		
	infoPB.ioNamePtr = fss->name;
	infoPB.ioVRefNum = fss->vRefNum;
	infoPB.ioDrDirID = fss->parID;
	infoPB.ioFDirIndex = -1;
	err = PBGetCatInfo(&infoPB,false);
	fss->parID = infoPB.ioDrParID;
	
	return( err );
}

/* ======================================== */

Boolean sfget_same_file(FSSpec *file1,FSSpec *file2)
{
	if ( file1->vRefNum != file2->vRefNum )
		return( 0 );
		
	if ( file1->parID != file2->parID )
		return( 0 );
		
	if ( !EqualString(file1->name,file2->name,false,true) )
		return( 0 );
	
	return( 1 );
}

/* ======================================== */

OSErr sfget_get_desktop_folder(short vRefNum)
{
	DirInfo infoPB;
	OSErr	error;
	
	if (!gHasFindFolder) {
		gDeskFolderSpec.vRefNum = -9999;
		return( -1 );
		}
	
	gDeskFolderSpec.name[0] = '\0';

	error = FindFolder(vRefNum,kDesktopFolderType,kDontCreateFolder,&gDeskFolderSpec.vRefNum,&gDeskFolderSpec.parID);

	if ( error )
		return( error );
	
	return( sfget_ensure_name( &gDeskFolderSpec ) );
}

/* ======================================== */

// sfget

int sfget( void )
{
	int					my_err;
	Point				where = {-1,-1};
	SFData				sfData;
	StandardFileReply	sfReply;
	
	/* initialize user data area */
	
	sfData.replyPtr = &sfReply;
	sfData.oldSelection.vRefNum = -9999;	/* init to ridiculous value */
	
	CustomGetFile(nil,-1,nil,&sfReply,SFGetDialog,where,sfget_hook,nil,nil,nil,&sfData);
	
	if (sfReply.sfGood)
		my_err = sfget_path( &sfReply );
	else
		my_err = 1;
		
	return( my_err );
}

/* ======================================== */

pascal short sfget_hook(short item,DialogPtr theDlg,Ptr userData)
{
	Boolean hiliteButton;
	FSSpec curSpec;
	OSType refCon;
	SFDataPtr sfData;
	
	refCon = GetWRefCon(theDlg);
	if (refCon!=sfMainDialogRefCon)
		return item;
		
	sfData = (SFDataPtr) userData;
	
	if (item==sfHookFirstCall || item==sfHookLastCall)
		return item;
	
	if (item==sfItemVolumeUser) {
		sfData->replyPtr->sfFile.name[0] = '\0';
		sfData->replyPtr->sfFile.parID = 2;
		sfData->replyPtr->sfIsFolder = false;
		sfData->replyPtr->sfIsVolume = false;
		sfData->replyPtr->sfFlags = 0;
		item = sfHookChangeSelection;
		}
		
	if (!sfget_same_file(&sfData->replyPtr->sfFile,&sfData->oldSelection)) {
	
		BlockMove(&sfData->replyPtr->sfFile,&curSpec,sizeof(FSSpec));
		sfget_ensure_name(&curSpec);
		
		if (curSpec.vRefNum!=sfData->oldSelection.vRefNum)
			sfget_get_desktop_folder(curSpec.vRefNum);
			
		sfget_button(curSpec.name,theDlg);
		
		BlockMove(&sfData->replyPtr->sfFile,&sfData->oldSelection,sizeof(FSSpec));
		}
	
	if (item==SFGetButton)
		item = sfItemOpenButton;
		
	return item;
}

/* ======================================== */

void sfget_button( StringPtr selName ,DialogPtr theDlg )
{
	Handle iHndl;
	Rect iRect;
	short iType;
	Str255 temp;
		
	pStrCopy( temp, "\pSelect" );
	pStrAppend( temp, "\p \"");
	pStrAppend( temp, selName );
	temp[ ++temp[0] ] = '"';

	GetDItem(theDlg,SFGetButton,&iType,&iHndl,&iRect);
	SetCTitle(iHndl,temp);
	SetDItem(theDlg,SFGetButton,iType,iHndl,&iRect);
}

/* ======================================== */

int sfget_init(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	char	*p;
	char	*q;
	char	c;
	int		argc;
	int		error;
	int		i;
	int		prompt;
	int		usage;
	int		var;
	Str255	prompt_str;
	
	// return if bad include file version
		
	if (nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION ))
		return( 0 );
	
	// return if pre system 7 os
		
	if ( sfget_pre7() ) {
		nshc_calls->NSH_putStr_err("\psfget: This command requires System 7.\r");
		nshc_parms->result = NSHC_ERR_GENERAL;
		nshc_parms->action = nsh_idle;
		return( 0 );
		}

	// return if bad usage
		
	usage = 0;
	argc = nshc_parms->argc;
	prompt = nshc_got_option(nshc_parms, 'p');

	if ((argc != 2) && (argc != 4))
		usage = 1;
		
	if ( (argc == 4) && ( prompt != 1 ) && ( prompt != 2 ) )
		usage = 1;
	
	if ( usage ) {
		nshc_calls->NSH_putStr_err("\pUsage: sfget variable_name [-p \"prompt string\"].\r");
		nshc_parms->result = NSHC_ERR_PARMS;
		nshc_parms->action = nsh_idle;
		return( 0 );
		}
		
	// return if bad prompt string
	
	if (prompt) {
	
		p = &nshc_parms->arg_buf[nshc_parms->argv[prompt + 1]];
			
		if ( cStrLen( p ) > 255 ) {
			nshc_calls->NSH_putStr_err("\psfget: Prompt string is too long.");
			nshc_parms->result = NSHC_ERR_PARMS;
			nshc_parms->action = nsh_idle;
			return( 0 );
			}
		else
			pStrFromC( prompt_str, p );
			
		ParamText( prompt_str, "\p",  "\p",  "\p" );
		
		}
	else
		ParamText( "\pSelect a folder or file:", "\p",  "\p",  "\p" );
	
	// return if bad variable name
	
	if ( prompt == 1 )
		var = 3;
	else
		var = 1;
		
	p = q = &nshc_parms->arg_buf[ nshc_parms->argv[ var ] ];
	i = 0;
	while (c = *p++) {
		error = 1;
		if ( c == '_' ) error = 0; else
		if ( ( c >= 'a' ) && ( c <= 'z' ) ) error = 0; else
		if ( ( c >= 'A' ) && ( c <= 'Z' ) ) error = 0; else
		if ( ( c >= '0' ) && ( c <= '9' ) ) error = 0;
		if (error) {
			nshc_calls->NSH_putStr_err( "\psfget: Invalid variable name = " );
			nshc_calls->NSH_puts_err( q );
			nshc_calls->NSH_putchar_err( '\r' );
			nshc_parms->result = NSHC_ERR_PARMS;
			nshc_parms->action = nsh_idle;
			return( 0 );
			}
		gVarName[++i] = c;
		}
		
	if ( i > 31 ) {
		nshc_calls->NSH_putStr_err( "\psfget: Variable name too long = " );
		nshc_calls->NSH_puts_err( q );
		nshc_calls->NSH_putchar_err( '\r' );
		nshc_parms->result = NSHC_ERR_PARMS;
		nshc_parms->action = nsh_idle;
		return( 0 );
		}
	else
		gVarName[0] = i;

	gHasFindFolder = sfget_gestalt( gestaltFindFolderAttr, gestaltFindFolderPresent );

	return( 1 );
}

/* ======================================== */

int sfget_open_rsrc(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
	Str255	fileName;

	pStrFromC( fileName, &nshc_parms->arg_buf[nshc_parms->argv[0]] );
	
	gFileRef = -1;
	
	if ( !nshc_calls->NSH_path_which( fileName ) )
		gFileRef = OpenResFile( fileName );
		
	if ( gFileRef < 0 ) {
		nshc_calls->NSH_putStr_err("\psfget: Could not open resource file.\r");
		nshc_parms->result = NSHC_ERR_PARMS;
		nshc_parms->action = nsh_idle;
		return( 1 );
		}
	else
		return( 0 );
}

/* ======================================== */

void sfget_close_rsrc(void)
{
	if (gFileRef)
		CloseResFile(gFileRef);
}

/* ======================================== */

void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
	int		error;
	
#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#else
	RememberA0();
	SetUpA4();
#endif
	
	if ( sfget_init( nshc_parms, nshc_calls ) ) {
	
		error = sfget_open_rsrc( nshc_parms, nshc_calls );
	
		if ( !error )
			error = sfget();
			
		sfget_close_rsrc();
		
		if ( !error )
			nshc_calls->NSH_var_set( gVarName, gPath );

 		nshc_parms->action = nsh_idle;
		nshc_parms->result = error;
			
		}

#ifdef __MWERKS__
	SetA4(oldA4);
#else
	RestoreA4();
#endif
}
