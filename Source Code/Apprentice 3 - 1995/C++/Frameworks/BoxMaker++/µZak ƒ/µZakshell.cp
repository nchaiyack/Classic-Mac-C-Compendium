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
#include <Movies.h>
#include <Folders.h>

#include "standardgetfile.h"
#include "boxmakergetfile.h"

#include "boxmaker constants.h"
#include "boxmaker.h"
#include "preferences.cp"
#include "queue.cp"

#include "�Zak error messages.h"
#include "�Zakshell.h"

#pragma template_access public

void main();

void main()
{
	muZaksettings defaultSettings =
	{
		iShowPreferences,
		false,				// do not requeue
		false,				// do not shuffle
		VolumeNormal		// normal volume
	};
	StringHandle prefsFileHandle = GetString( 128);
	Handle configHandle = Get1Resource( 'cnfg', 128);
	const long queueLength = (configHandle == 0) ? 50 : **(long **)configHandle;
	muZakshell it( *(Str255 *)*prefsFileHandle, defaultSettings, queueLength);
	it.run();
	//
	// For now, do not free the memory allocated properly
	// (quitting is faster this way).
	//	ReleaseResource( (Handle)prefsFileHandle);
	//	ReleaseResource( configHandle);
	//
}

muZakshell::muZakshell( Str255 prefsFileName,
	muZaksettings &defaultsettings, unsigned long queueLength)
	: boxmaker( 3000)
	, muZakPrefs( prefsFileName, defaultsettings)
	, alias_queue( queueLength)
{
	//
	// Check for presence of QuickTime:
	//
	long response;
	OSErr result = Gestalt( gestaltQuickTime, &response);
	
	if( (result != noErr) || (response == 0))
	{
		//
		// QuickTime not present => display alert saying so, and exit
		//
		ErrorAlertQuit( (result != noErr) ? result : response, kNeedQuickTimeError);
	}
	result = EnterMovies();
	if( result != noErr)
	{
		ErrorAlertQuit( result, kEnterMoviesFailedError);
	}
	theMovie = 0;
	theAlias = 0;
	SetButtons();
	SetRequeueButton();
	SetShuffleButton();
	SetVolumePopup();

	switch( wie_van_de_drie)
	{
		case iShowPreferences:
			ShowPreferences();
			//
			// For nice feedback when scores of files are dropped
			// on the shell we draw the dialog immediately.
			//
			DrawDialog( gMainDialog);
			SetPort( gMainDialog);
			ValidRect( &thePort->portRect);
			break;
			
		case  iHideOnLaunch:
			//
			// First item in the Applications menu is 'Hide <current app>'
			//
			SystemMenu( 0xBF970001);
			break;

		case iNeither:
			;
	}
	qd.randSeed = TickCount();	// for true shuffle
}

muZakshell::~muZakshell()
{
	if( theMovie != 0)
	{
		StopCurrentMovie();
	}
	//
	// Should dispose of any aliases in the queue or in 'theAlias', here.
	// (see the handling of 'iCancelQueue' how to do this)
	//
	// This is not done since we will ExitToShell real soon now, and
	// this will free the memory, anyway.
	//
}

void muZakshell::OpenDoc( Boolean opening)
{
	if( opening)
	{
		//
		// Add the movie to the list of movies to be played.
		// We do not check for errors. If the queue is full,
		// the item is discarded.
		//
		if( !isFull())
		{
			AliasHandle newAlias;
			if( NewAliasMinimal( &theFSSpec, &newAlias) == noErr)
			{
				Add( newAlias);
				updateNumber();
			}
		}
	}
}

void muZakshell::HandleDialogEvent( short itemHit, DialogPtr theDialog)
{
	switch( itemHit)
	{
		case iShowPreferences:
		case iHideOnLaunch:
		case iNeither:
			wie_van_de_drie = itemHit;
			SetButtons();
			break;
		
		case iRequeue:
			requeue_after_playing = !requeue_after_playing;
			SetRequeueButton();
			break;
	
		case iShuffle:
			shuffle = !shuffle;
			SetShuffleButton();
			break;

		case iVolume:
			{
				short	iType;
				Handle	iHandle;
				Rect	iRect;
				GetDialogItem( gMainDialog, iVolume, &iType, &iHandle, &iRect);
				volume = (soundVolume)GetControlValue( (ControlHandle)iHandle);
				
				if( theMovie != 0)
				{
					SetMovieVolume( theMovie, theVolumes[ volume]);
				}
			}
			break;

		case iCancelCurrent:
			if( theMovie != 0)
			{
				StopCurrentMovie();
			}
			break;

		case iCancelQueue:
			while( !isEmpty())
			{
				AliasHandle anAlias = Remove();
				DisposeHandle( (Handle)anAlias);
			}
			updateNumber();
			break;
			
		case iNameOfFile:
		case iQueueLength:
		default:
			;
	}
}

Boolean muZakshell::mayEnterFolder( Boolean opening)
{
	//
	// it doesn't make sense to enter a folder when the alias queue is
	// full, since then any movie file found will be discarded by the
	// OpenDoc handler
	//
	return !isFull();
}

Boolean muZakshell::EventloopHook()
{
	Boolean result = false;
	if( theMovie == 0)
	{
		//
		// Is there an item to play?
		//
		if( isEmpty() || (the_status == shell_is_quitting))
		{
			result = true;
		} else {
			//
			// brute force shuffle (will be fairly slow when
			// lots of files are present)
			//
			const int maxNumToSkip = getLength() - 3;
			if( shuffle && (maxNumToSkip > 0))
			{
				const int numToSkip = Random() % maxNumToSkip;
				for( int i = 0; i < numToSkip; i++)
				{
					Add( Remove());
				}
			}
			theAlias = Remove();
			updateNumber();
			StartAMovie();
		}
	} else {
		if( IsMovieDone( theMovie) || (the_status == shell_is_quitting))
		{
			StopCurrentMovie( false);
			if( requeue_after_playing && !isFull()
					&& (the_status != shell_is_quitting))
			{
				Add( theAlias);
				updateNumber();
			} else {
				DisposeHandle( (Handle)theAlias);
			}
		} else {
			MoviesTask( theMovie, 0L);
		}
	}
	return result;
}

void muZakshell::StartAMovie()
{
	//
	// First resolve the Alias:
	//
	FSSpec theFSSpec;
	Boolean wasChanged;
	const OSErr result = ResolveAlias( 0L, theAlias, &theFSSpec, &wasChanged);
	
	if( result == noErr)
	{
		//
		// Note: we do not have to check whether we are in the background
		// when setting the cursor. The system seems to do that for us.
		//
		CursHandle prisma = GetCursor( watchCursor);
		SetCursor( *prisma);
		ReleaseResource( (Handle)prisma);
		OSErr err = OpenMovieFile( &theFSSpec, &theMovieFile, fsRdPerm);
		long  controllerFlags;
		
		if( (err != noErr) || (GetMoviesError() != noErr))
		{
			ErrorAlert( err, kMovieToolboxError);
		}	
		short theMovieResID = 0;	// want first movie in the file
	
		err = NewMovieFromFile( &theMovie, theMovieFile, &theMovieResID,
					(StringPtr) 0, newMovieActive, (Boolean *) 0);
	
		if( (err != noErr) || (GetMoviesError() != noErr))
		{
			ErrorAlert( err, kMovieToolboxError);
		}	
		//
		// Disable all non-sound tracks found:
		//
		const long numTracks = GetMovieTrackCount( theMovie);
		unsigned long numSoundTracks = 0;
		//
		// 941130: Thanks to Paul C. Ho of 'All Midi' fame
		// I learnt to count from track 1 (instead of from zero)
		//
		for( int trackNo = 1; trackNo <= numTracks; trackNo++)
		{
			Track theTrack = GetMovieIndTrack( theMovie, trackNo);
			Media theMedia = GetTrackMedia( theTrack);
			OSType mediaType;
			GetMediaHandlerDescription( theMedia, &mediaType, nil, nil);
			//
			// Note: some older sound media have mediaType == 0, but so do some
			// video media => some sound media will not be disabled.
			// Also, my headers aren't up to date, so MusicMediaType isn't defined.
			//
			const Boolean isaSoundMedia =
				(mediaType == SoundMediaType) || (mediaType == MusicMediaType);
	
			if( isaSoundMedia)
			{
				numSoundTracks += 1;
			} else {
				SetTrackEnabled( theTrack, false);
			}
		}
		if( numSoundTracks == 0)
		{
			StopCurrentMovie();
		} else {
			//
			// Change the cover procedures (no longer needed now that we call 'SetMovieBox')
			//			
			const Rect nullRect = {0, 0, 0, 0};
			SetMovieBox( theMovie, &nullRect);
			//
			// Now we can preroll and play the movie
			//
			err = PrerollMovie( theMovie, GetMovieDuration( theMovie), GetMovieRate( theMovie));
			
			if( err != noErr)
			{
				ErrorAlert( err, kMovieToolboxError);
			}
			//
			// 950516: change volume
			//
			SetMovieVolume( theMovie, theVolumes[ volume]);
			
			StartMovie( theMovie);
			updateName( theFSSpec.name);
		}
		InitCursor();
	}
}

void muZakshell::StopCurrentMovie( Boolean dispose_of_alias)
{
	if( theMovie != 0)	// better be safe than sorry
	{
		CloseMovieFile( theMovieFile);
		DisposeMovie( theMovie);
		theMovie = 0;
		updateName( "\p");
		if( dispose_of_alias)
		{
			DisposeHandle( (Handle)theAlias);
			theAlias = 0;	// just for tidyness; a _perfect_ compiler will optimize it away
		}
	}
}

void muZakshell::SetButtons()
{
	for( int itemNo = iShowPreferences; itemNo <= iNeither; itemNo++)
	{
		short	iType;
		Handle	iHandle;
		Rect	iRect;
		GetDialogItem( gMainDialog, itemNo, &iType, &iHandle, &iRect);
		SetControlValue( (ControlHandle)iHandle, (itemNo == wie_van_de_drie));	
	}
}

void muZakshell::SetRequeueButton()
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	GetDialogItem( gMainDialog, iRequeue, &iType, &iHandle, &iRect);
	SetControlValue( (ControlHandle)iHandle, requeue_after_playing);	
}

void muZakshell::SetShuffleButton()
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	GetDialogItem( gMainDialog, iShuffle, &iType, &iHandle, &iRect);
	SetControlValue( (ControlHandle)iHandle, shuffle);	
}

void muZakshell::SetVolumePopup()
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	GetDialogItem( gMainDialog, iVolume, &iType, &iHandle, &iRect);
	SetControlValue( (ControlHandle)iHandle, volume);
}

void muZakshell::updateName( Str63 theName)
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	GetDialogItem( gMainDialog, iNameOfFile, &iType, &iHandle, &iRect);
	SetDialogItemText( iHandle, theName);
}

void muZakshell::updateNumber()
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	Str15 string;
	NumToString( getLength(), string);
	GetDialogItem( gMainDialog, iQueueLength, &iType, &iHandle, &iRect);
	SetDialogItemText( iHandle, string);
}
