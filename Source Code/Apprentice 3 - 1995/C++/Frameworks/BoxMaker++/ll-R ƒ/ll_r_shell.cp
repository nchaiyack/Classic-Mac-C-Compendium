#include <Types.h>
#include <Memory.h>
#include <QuickDraw.h>
#include <OSUtils.h>
#include <ToolUtils.h>
#include <Menus.h>
#include <Packages.h>
#include <Traps.h>
#include <Files.h>
#include <Aliases.h>
#include <AppleEvents.h>
#include <GestaltEqu.h>
#include <Processes.h>
#include <Fonts.h>
#include <OSEvents.h>
#include <Resources.h>
#include <Desk.h>

#include "standardgetfile.h"
#include "boxmakergetfile.h"

#include "boxmaker constants.h"
#include "boxmaker.h"
#include "preferences.cp"
#include "ll-R constants.h"
#include "ll_r_shell.h"
#include "outfile.h"

#pragma template_access public

void main()
{
	StringHandle prefsFileHandle = GetString( 128);

	ll_r_settings defaultSettings =
	{
		false,		// item 0 is not used in dialogs
		true,		// kType
		true,		// kCreator
		true,		// kSize
		false,		// kBytes
		true,		// kKiloBytes
		false,		// kCreatDate
		true,		// kModDate
		false,		// kBakDate
		false,		// kTimes

		true,		// kTabs
		false,		// kSpaces
		false,		// kAskFileName
		true		// kIndentFolders
	};
	ll_r_shell it( *(Str255 *)*prefsFileHandle, defaultSettings);
	it.run();
}

ll_r_shell::ll_r_shell( Str255 prefsFilename, ll_r_settings defaultsettings)
	: boxmaker( 3000)
	, ll_r_prefs( prefsFilename, defaultsettings)
{
	indentationLevel = 0;
	outFileNo = 0;
	SetupDialog( gMainDialog);
}

void ll_r_shell::OpenDoc( Boolean opening)
{
	StartNewLine();
	if( !itsADirectory())
	{
		if( myPrefs[ kCreator])
		{
			append( (char *)&theCInfoPBRec.hFileInfo.ioFlFndrInfo.fdCreator, 4);
		}
		if( myPrefs[ kType])
		{
			append( (char *)&theCInfoPBRec.hFileInfo.ioFlFndrInfo.fdType, 4);
		}
		if( myPrefs[ kSize])
		{
			Str255 sizeString;
			long totalsize = theCInfoPBRec.hFileInfo.ioFlLgLen + theCInfoPBRec.hFileInfo.ioFlRLgLen;
			
			if( myPrefs[ kKiloBytes])
			{
				totalsize = KiloBytes( totalsize);
			}
			NumToString( totalsize, sizeString);
			if( myPrefs[ kSpaces])
			{
				const int spacesBefore = 5 - sizeString[ 0];
				for( int i = 0; i < spacesBefore; i++)
				{
					append( ' ');
				}
			}
			append( sizeString);

			if( myPrefs[ kKiloBytes])
			{
				RemoveLastChar();		// the delimiter added by 'append( sizeString)'
				append( 'K');
				append( delimiter);
			}
		}
		if( myPrefs[ kCreatDate])
		{
			appendDate( theCInfoPBRec.hFileInfo.ioFlCrDat);
		}
		if( myPrefs[ kModDate])
		{
			appendDate( theCInfoPBRec.hFileInfo.ioFlMdDat);
		}
		if( myPrefs[ kBakDate])
		{
			appendDate( theCInfoPBRec.hFileInfo.ioFlBkDat);
		}
	}
	//
	// append filename:
	//
	append( theCInfoPBRec.hFileInfo.ioNamePtr);
	ChangeLastToNewLine();

	long numtowrite = currentEnd - plentySpace;

	(void)FSWrite( outFileNo, &numtowrite, (Ptr)plentySpace);
}

void ll_r_shell::CantEnterFolder( Boolean opening)
{
	StartNewLine();
	//
	// 'OpenDoc' wrote a line with the name of the folder on it and ended
	// it with a newline. We want to add an error message to that line.
	// Therefore we overwrite that newline by the first character of the
	// error message (which starts with a tab).
	//
	StringHandle message = GetString( 131);

	append( *message);

	ChangeLastToNewLine();

	long numtowrite = currentEnd - plentySpace;
	(void)SetFPos( outFileNo, fsFromLEOF, -1);
	(void)FSWrite( outFileNo, &numtowrite, (Ptr)plentySpace);
}

ll_r_shell::~ll_r_shell()
{
	if( outFileNo != 0)
	{
		(void)FSClose( outFileNo);
		(void)FlushVol( 0, outVRefNum);
	}
}

void ll_r_shell::StartABunch( long numTopLevelItems, Boolean opening)
{
	boxmaker::StartABunch( numTopLevelItems, opening);
	if( outFileNo == 0)
	{
		StringHandle outName = GetString( 129);
		if( myPrefs[ kAskFileName])
		{
			StandardFileReply stdReply;
			StringHandle prompt = GetString( 130);
			StandardPutFile( *prompt, *outName, &stdReply);
			ReleaseResource( (Handle)prompt);
			if( stdReply.sfGood)
			{
				OSErr result = noErr;
				if( stdReply.sfReplacing)
				{
					result = FSpDelete( &stdReply.sfFile);
				}
				if( result == noErr)
				{
					result = FSpCreate( &stdReply.sfFile,
								'R*ch', 'TEXT', stdReply.sfScript);
					if( result == noErr)
					{
						result = FSpOpenDF( &stdReply.sfFile, fsWrPerm, &outFileNo);
						outVRefNum = stdReply.sfFile.vRefNum;
					}
				}
				if( result != noErr)
				{
					ErrorAlertQuit( result, kFileCreationFailed);
				}
			} else {
				ErrorAlertQuit( noErr, kNeedAFile);
			}
		} else {
			Handle folderTypeHandle = Get1Resource( 'Wher', 128);
			const OSType folderType = **((OSType **)folderTypeHandle);
			ReleaseResource( folderTypeHandle);
			outfile desktopFile( *(Str255 *)*outName, folderType);
			outFileNo  = desktopFile();
			outVRefNum = desktopFile.vRefNum();
		}
		ReleaseResource( (Handle)outName);
	}
}

void ll_r_shell::SetupDialog( DialogPtr theDialog)
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;

	for( int i = 1; i < kNumButtons; i++)
	{
		GetDialogItem( theDialog, i, &iType, &iHandle, &iRect);
		SetControlValue( (ControlHandle)iHandle, myPrefs[ i]);
	}
	delimiter = "\t "[ myPrefs[ kSpaces]];

	const int enable_SizeCheckBoxes = myPrefs[ kSize];

	GetDialogItem( theDialog, kBytes, &iType, &iHandle, &iRect);
	HiliteControl( (ControlHandle) iHandle, enable_SizeCheckBoxes ? 0 : 255);
	
	GetDialogItem( theDialog, kKiloBytes, &iType, &iHandle, &iRect);
	HiliteControl( (ControlHandle) iHandle, enable_SizeCheckBoxes ? 0 : 255);

	const int enable_kTimes = myPrefs[ kCreatDate]
						|| myPrefs[ kModDate] || myPrefs[ kBakDate];

	GetDialogItem( theDialog, kTimes, &iType, &iHandle, &iRect);
	HiliteControl( (ControlHandle) iHandle, enable_kTimes ? 0 : 255);
}

void ll_r_shell::HandleDialogEvent( short itemHit, DialogPtr theDialog)
{
	switch( itemHit)
	{
		case kType:
		case kCreator:
		case kTimes:
		case kAskFileName:
		case kIndentFolders:
			FlipButton( theDialog, itemHit);
			break;

		case kSize:
			{
				short	iType;
				Handle	iHandle;
				Rect	iRect;
				FlipButton( theDialog, itemHit);
				const int enable_SizeCheckBoxes = myPrefs[ kSize];
			
				GetDialogItem( theDialog, kBytes, &iType, &iHandle, &iRect);
				HiliteControl( (ControlHandle) iHandle, enable_SizeCheckBoxes ? 0 : 255);
				
				GetDialogItem( theDialog, kKiloBytes, &iType, &iHandle, &iRect);
				HiliteControl( (ControlHandle) iHandle, enable_SizeCheckBoxes ? 0 : 255);
			}
			break;

		case kBytes:
		case kKiloBytes:
			if( !myPrefs[ itemHit])
			{
				short	iType;
				Handle	iHandle;
				Rect	iRect;
	
				FlipButton( theDialog, kBytes);
				FlipButton( theDialog, kKiloBytes);
			}
			break;			

		case kCreatDate:
		case kModDate:
		case kBakDate:
			{
				short	iType;
				Handle	iHandle;
				Rect	iRect;
	
				FlipButton( theDialog, itemHit);
	
				const int enable_kTimes = myPrefs[ kCreatDate]
						|| myPrefs[ kModDate] || myPrefs[ kBakDate];
	
				GetDialogItem( theDialog, kTimes, &iType, &iHandle, &iRect);
				HiliteControl( (ControlHandle) iHandle, enable_kTimes ? 0 : 255);
			}
			break;			

		case kTabs:
		case kSpaces:
			if( !myPrefs[ itemHit])
			{
				FlipButton( theDialog, kTabs);
				FlipButton( theDialog, kSpaces);
				
				delimiter = "\t "[ itemHit == kSpaces];
			}
			break;
	}
}

void ll_r_shell::appendDate( unsigned long seconds)
{
	DateTimeRec theDate;
	SecondsToDate( seconds, &theDate);
	
	append2Digits( theDate.year);
	append2Digits( theDate.month);
	append2Digits( theDate.day);
	append( delimiter);

	if( myPrefs[ kTimes])
	{
		append2Digits( theDate.hour);
		append( ':');
		append2Digits( theDate.minute);
		append( ':');
		append2Digits( theDate.second);
		append( delimiter);
	}
}

void ll_r_shell::FlipButton( DialogPtr theDialog, short theItem)
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;

	myPrefs[ theItem] = !myPrefs[ theItem];

	GetDialogItem( theDialog, theItem, &iType, &iHandle, &iRect);
	SetControlValue( (ControlHandle)iHandle, myPrefs[ theItem]);
}

void ll_r_shell::append( char *item, int len)
{
	for( int i = 0; i < len; i++)
	{
		*currentEnd++ = *item++;
	}
	append( delimiter);
}
