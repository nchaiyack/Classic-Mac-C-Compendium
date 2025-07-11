#include <stdio.h>
#include <ctype.h>
#include <console.h>

#include "compdict.h"
#include "interface.h"

extern unsigned long edgesused;

Rect	gStringRect;
short	gOutFile;

void main( void)
{
	InitMacintosh();
	
	const short infile = OldFile();
	
	if( infile != 0)
	{
		gOutFile = NewFile();
		Handle the_handle = ReadFile( infile);
		unsigned long numchars = GetHandleSize( the_handle);
		
		ParamText( 0L, 0L, 0L, 0L);
		DialogPtr myDialog = GetNewDialog( 128, 0L, (WindowPtr) -1);
		{
			short type;
			Handle handle;
			
			GetDialogItem( myDialog, 3, &type, &handle, &gStringRect);
		}
		DrawDialog( myDialog);
		SetPort( myDialog);
		TextMode( srcCopy);
		HLock( the_handle);
		
		char * the_input = *the_handle;
		
		for( int i = 0; i < numchars; i++)
		{
			if( iscntrl( the_input[ i]))
			{
				the_input[ i] = 0;
			}
		}
		//
		// 'Reserve' room for the number of nodes + edges in the dawg:
		// This will be overwritten later.
		//
		long count = sizeof( edgesused);
		FSWrite( gOutFile, &count, &edgesused);
		
		do_it( the_input, numchars);
		HUnlock( the_handle);
		//
		// Overwrite the initial
		//
		(void)SetFPos( gOutFile, fsFromStart, 0);
		
		edgesused += 1;
		count = sizeof( edgesused);
		FSWrite( gOutFile, &count, &edgesused);
		
		FSClose( gOutFile);	
		while( !Button())
		{
			SysBeep( 9);
		}
		DisposeDialog( myDialog);
	}
}

void InitMacintosh( void)
{
	//
	// We do not call MaxApplZone() since this program needs a large stack
	// (The best solution would be to call SetApplLimit using the proper
	// value, but I don't know what that is)
	//
	InitGraf( &thePort);
	InitFonts();
	FlushEvents( everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( 0L);
	InitCursor();
}

short OldFile( void)
{
	SFTypeList myTypes;
	SFReply reply;
	short result = 0;
	Point where = {50, 50};
	myTypes[ 0] = 'TEXT';

	SFGetFile( where, "\pConvert which text file?", 0L, 1, myTypes, 0L, &reply );

	if( reply.good)
	{
		FSOpen( reply.fName, reply.vRefNum, &result);
	}
	return result;
}

short NewFile( void)
{
	Point where = {50, 50};
	Str255 filename = "\pCompiled dictionary";
	SFReply reply;
	
	short result = 0;
	
	SFPutFile( where, "\pSave dictionary file as", filename, 0L, &reply);
	if( reply.good)
	{
		OSErr ioresult = Create( reply.fName, reply.vRefNum, 'CRAB', 'DICT');
		
		if( (ioresult == noErr) || (ioresult == dupFNErr))
		{
			ioresult = FSOpen( reply.fName, reply.vRefNum, &result);
		}
	}
	return result;
}

OSErr WriteFile( int refNum, Handle thedata)
{
	OSErr ioresult;
	long numtowrite = GetHandleSize( thedata);
	char *address   = *thedata;
	char state = HGetState( thedata);
	HLock( thedata);
	ioresult = FSWrite( refNum, &numtowrite, address);
	HSetState( thedata, state);
	return ioresult;
}

Handle ReadFile( int refNum)
{
	OSErr ioresult;
	long filesize;
	Handle result;
	
	GetEOF( refNum, &filesize);
	
	result = NewHandle( filesize);
	if( result != 0)
	{
		const char state = HGetState( result);
			HLock( result);
			(void)FSRead( refNum, &filesize, *result);
		HSetState( result, state);
	}
	return result;
}
