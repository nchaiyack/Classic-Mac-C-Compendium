/******************************************************************************
 CMovieController.cp
 
		
	SUPERCLASS = CPane
	
	Copyright © 1994 Johns Hopkins University. All rights reserved.
	
	CMovieController is a class which implements the QuickTime Movie Controller
	Component for QuickTime movie playback ONLY.  You should subclass 
	CMovieController to add more functionality by handling other components
	like Sequence Grabber, Clock, and others.
	
	The updating of movies is handled by the MovieController routine MCIdle()
	(which calls MovieTask()) instead of calling MovieTask() directly.  
	This prevents us from having to patch or subclass CSwitchboard in order to 
	handle QT events.
	
	Support for creating CMovieController panes directly from DLOG resources
	by overloading DITL items is also included.  Make sure you copy and use 
	the 'MoCt' TMPL from this projects resource file.
	
	NOTE:  I have added two error strings in STR# 131 for reporting QT errors.
	
	Please let me know if you enhance or expand this class.
	
	Original Author: 	Martin R. Wachter		email:	mrw@welchgate.welch.jhu.edu
	
	Modified:			4/20/94					by:		mrw			TCL Version:	1.1.3
	Created:			4/19/94					by:		mrw			TCL Version:	1.1.3

	Version change history:
	
	1.0		Initial release.
	
 ******************************************************************************/

#include "CMovieController.h"
#include <CPaneBorder.h>
#include <TBUtilities.h>

extern long			gSleepTime;			/* Max time between events			*/

/******************************************************************************
 IMovieController

		Initialize a Movie object
 ******************************************************************************/

void	CMovieController::IMovieController(
	CView			*anEnclosure,
	CBureaucrat		*aSupervisor,
	short			aWidth,
	short			aHeight,
	short			aHEncl,
	short			aVEncl,
	SizingOption	aHSizing,
	SizingOption	aVSizing)
{
OSErr	err = noErr;

	TRY{
		// fail if QT is not installed
		err = Gestalt(gestaltQuickTime, 0L);
		if (err != noErr)
			Failure(err,SpecifyMsg(0,5));
		
		// init QT
		FailOSErr(EnterMovies());
	}
	CATCH{
	}
	ENDTRY;

	CPane::IPane(anEnclosure, aSupervisor,
					aWidth, aHeight, aHEncl, aVEncl, aHSizing, aVSizing);

	IMovieControllerX();

}

/******************************************************************************
 IViewTemp
 
 	Initialize from a template resource.
******************************************************************************/

void CMovieController::IViewTemp(CView *anEnclosure, CBureaucrat *aSupervisor,
								Ptr viewData)
{
OSErr	err = noErr;
FSSpec	spec;
short	numFound;


	TRY{
		// fail if QT is not installed
		err = Gestalt(gestaltQuickTime, 0L);
		if (err != noErr)
			Failure(err,SpecifyMsg(0,5));
		
		// init QT
		FailOSErr(EnterMovies());
	}
	CATCH{
	}
	ENDTRY;

	tMovieControllerTempP MoCtpane = (tMovieControllerTempP) viewData;
	
	CPane::IViewTemp( anEnclosure, aSupervisor, viewData);

	IMovieControllerX();

	CopyPString(MoCtpane->movieTitle, itsMovieTitle);

	//  we will be using the origin of the DITL item
	// and the frame size will be set from the MovieBox
	width = height = 0;
	SetLongRect(&frame, 0,0,0,0);
	CalcAperture();
	
	TRY{
		numFound = LocateMovie(&spec);

		if (numFound == 1){
			SetMovie(&spec);
		}
		else if (numFound > 1){
			OpenMovie();
		}
		else{ // movie not found
			Failure(-43,SpecifyMsg(0,6));
		}
	}
	CATCH{
		Dispose();
	}
	ENDTRY;

	
}	/* CMovieController::IViewTemp */

/******************************************************************************
 IMovieControllerX
 
 	Perform common initialization.
******************************************************************************/

void CMovieController::IMovieControllerX(void)
{
	// gimmie a border
	itsBorder = new(CPaneBorder);
	itsBorder->IPaneBorder(kBorderFrame);

	// inti data members
	itsMovie = NULL;
	itsMovieRefNum = 0;
	itsController = NULL;

	SetWantsClicks(TRUE);
	SetCanBeGopher(TRUE);
	BecomeGopher(TRUE);
	
}	/* CMovieController::IMovieControllerX */

/******************************************************************************
 OpenMovie

        Initialize a movie from a Macitosh 'Moov' file.
 ******************************************************************************/

void    CMovieController::OpenMovie(void)
{
StandardFileReply	sfr;
short				numTypes = 1, rfRefNum;
SFTypeList			sfFileTypes = {MovieFileType};
	
	StandardGetFilePreview(nil, numTypes, sfFileTypes, &sfr);
	
	if (sfr.sfGood){
		SetMovie(&sfr.sfFile);
	}
}

/******************************************************************************
 Dispose  {OVERRIDE}

        Dispose of a Movie object
 ******************************************************************************/

void    CMovieController::Dispose()
{
	ForgetMembers();	
    CPane::Dispose();
}

/******************************************************************************
 ForgetMembers

        Dispose of a Movie objects
 ******************************************************************************/

void    CMovieController::ForgetMembers()
{
	if (itsController)
		DisposeMovieController(itsController);

	if (itsMovie)
		DisposeMovie(itsMovie);

	if (itsMovieRefNum)
		CloseMovieFile(itsMovieRefNum);
		
	itsMovieRefNum = 0;
}

/******************************************************************************
 SetMovie

        This routine sets itsMovie and creates and inits our Movie Controller
        from an FSSpec.
 ******************************************************************************/
void	CMovieController::SetMovie(FSSpec *spec)
{
Rect	movieRect,MCRect;
Movie	theMovie;
OSErr	err = noErr;

	Prepare();
		
	ForgetMembers();	
	
	FailOSErr(OpenMovieFile(spec, &itsMovieRefNum, fsRdPerm));
	FailOSErr(NewMovieFromFile(&theMovie, itsMovieRefNum, nil, nil, newMovieActive, nil));
	
	itsMovie = theMovie;
	
	GetMovieBox(itsMovie, &movieRect);
	FailOSErr(GetMoviesError());

	GoToBeginningOfMovie(itsMovie);			
	FailOSErr(GetMoviesError());

	itsController = NewMovieController(itsMovie, &movieRect, mcWithFrame+mcTopLeftMovie);
	FailOSErr(GetMoviesError());
	
	// resize our frame and aperture to the size of the movie and the controller
	MCGetControllerBoundsRect(itsController, &MCRect);
	SetRect(&MCRect, 0, 0, MCRect.right-1, MCRect.bottom-2);
	SetMovieBox(itsMovie, &movieRect);			
	FailOSErr(GetMoviesError());

	ResizeFrame(&MCRect);
	CalcAperture();
		
	Refresh();
}


/******************************************************************************
 NameFileSearch

	by Jim Luther, Apple Developer Technical Support

	From the File:		MoreFilesExtras.c/ Dev.CD Mar 94	
	Copyright © 1992-1994 Apple Computer, Inc.All rights reserved.
		
******************************************************************************/

OSErr CMovieController::NameFileSearch(StringPtr volName,
							   short vRefNum,
							   const Str255 fileName,
							   FSSpecPtr matches,
							   long reqMatchCount,
							   long *actMatchCount,
							   Boolean newSearch,
							   Boolean partial)
{
	CInfoPBRec		searchInfo1, searchInfo2;
	HParamBlockRec	pb;
	OSErr			error;
	static CatPositionRec catPosition;
	static short	lastVRefNum = 0;
	short			bufferSize = 1024 * 16;
	Str255			fName;
	
	CopyPString(fileName, fName);

	pb.csParam.ioNamePtr = nil;
	pb.csParam.ioVRefNum = vRefNum;
	pb.csParam.ioMatchPtr = matches;
	pb.csParam.ioReqMatchCount = reqMatchCount;
	pb.csParam.ioSearchBits = (partial) ?	/* tell CatSearch what we're looking for: */
		(fsSBPartialName + fsSBFlAttrib) : 	/* partial name file matches or */
		(fsSBFullName + fsSBFlAttrib);		/* full name file matches */
	pb.csParam.ioSearchInfo1 = &searchInfo1;
	pb.csParam.ioSearchInfo2 = &searchInfo2;
	pb.csParam.ioSearchTime = 0;
	if ((newSearch) ||				/* If caller specified new search */
		(lastVRefNum != vRefNum))	/* or if last search was to another volume, */
		catPosition.initialize = 0;	/* then search from beginning of catalog */
	pb.csParam.ioCatPosition = catPosition;

	while(bufferSize) {				
         pb.csParam.ioOptBuffer = NewPtr(bufferSize);
		if(pb.csParam.ioOptBuffer) break;
		else bufferSize -= 1024;
	}
	
    if (pb.csParam.ioOptBuffer) 
    	pb.csParam.ioOptBufSize = bufferSize;            
    else 
    	pb.csParam.ioOptBufSize = 0;

	/* search for fName */
	searchInfo1.hFileInfo.ioNamePtr = (StringPtr)fName;
	searchInfo2.hFileInfo.ioNamePtr = nil;
	
	/* only match files (not directories) */
	searchInfo1.hFileInfo.ioFlAttrib = 0x00;
	searchInfo2.hFileInfo.ioFlAttrib = 0x10;

	error = PBCatSearchSync((CSParamPtr)&pb);

	if ((error == noErr) ||							/* If no errors or the end of catalog was */
		(error == eofErr))							/* found, then the call was successful so */
		*actMatchCount = pb.csParam.ioActMatchCount;	/* return the match count */
	else
		*actMatchCount = 0;							/* else no matches found */
	
	if ((error == noErr) ||							/* If no errors */
		(error == catChangedErr))					/* or there was a change in the catalog */
	{
		catPosition = pb.csParam.ioCatPosition;
		lastVRefNum = vRefNum;
			/* we can probably start the next search where we stopped this time */
	}
	else
		catPosition.initialize = 0;
			/* start the next search from beginning of catalog */
	
	if (pb.csParam.ioOptBuffer)
		DisposPtr(pb.csParam.ioOptBuffer);
		
	return (error);
}

/******************************************************************************
 LocateMovie {OVERRIDE}

	Tries to locates a Movie from a file name by using NameFileSearch().  If
	the file is found spec returns with a valid FSSpec.
		
 ******************************************************************************/

short	CMovieController::LocateMovie(FSSpec *spec)

{
OSErr				iErr = noErr;
CInfoPBRec 			pb;
long				dirID, totalMatches, maxMatches;
FSSpec				file[2];
short				vRefNum;
Str255				volName;

	dirID = 0;									// Search Entire Drive;
	vRefNum = -1;								// Search boot volume;
	maxMatches = 2;								// Find atleast 2 files


	pb.hFileInfo.ioNamePtr = volName;
	pb.hFileInfo.ioVRefNum = vRefNum;
	pb.hFileInfo.ioDirID = dirID;
	pb.hFileInfo.ioFDirIndex = -1;	/* get information about ioDirID */
	PBGetCatInfoSync(&pb);

	iErr = NameFileSearch(volName, vRefNum, itsMovieTitle, 
				  		 &file[0], maxMatches, &totalMatches, TRUE, FALSE);
	
	if (totalMatches == 1){
		*spec = file[0];
	}
	
	return totalMatches;
	
}

/******************************************************************************
 Activate {OVERRIDE}

		Try to handle an Activate event.  If not, pass it up.
		
 ******************************************************************************/

void	CMovieController::Activate( void)
{
ComponentResult	result;
	
	Prepare();
	result = MCActivate (itsController, macPort, TRUE);
	if (!result)
		CView::Activate();
}	

/******************************************************************************
 Deactivate {OVERRIDE}

		Try to handle an Deactivate event.  If not, pass it up.
		
 ******************************************************************************/

void	CMovieController::Deactivate( void)
{
ComponentResult	result;
	
	Prepare();
	result = MCActivate (itsController, macPort, FALSE);
	if (!result)
		CView::Deactivate();
}	

/******************************************************************************
 DoKeyDown {OVERRIDE}

		Respond to a keystroke
 ******************************************************************************/

void	CMovieController::DoKeyDown(char theChar,Byte keyCode,EventRecord *macEvent)
{
ComponentResult	result;

	Prepare();
	result =  MCKey (itsController, theChar, macEvent->modifiers);	
	if (!result)						
		itsSupervisor->DoKeyDown(theChar, keyCode, macEvent);
}
	
/******************************************************************************
 DoClick {OVERRIDE}

		Respond to a mouse click within the controller.
 ******************************************************************************/

void	CMovieController::DoClick(Point hitPt,short modifierKeys,long when)
{
ComponentResult	result;

	Prepare();
	result = MCClick(itsController, macPort, hitPt, when, modifierKeys);
	if (!result)
		CView::DoClick(hitPt, modifierKeys, when);
}

/******************************************************************************
 Dawdle {OVERRIDE}

		Call MoviesTask here if we have a movie and it is playing. MoviesTask()
		handles updating the movie for us. It will update the movie in the background
		also.
		
 ******************************************************************************/

void	CMovieController::Dawdle(long	*maxSleep)
{
	Prepare();
	
	if (itsMovie && !IsMovieDone(itsMovie)) {
			gSleepTime = 0;						// set sleep to minimum
	}

	if (itsController)
		MCIdle(itsController);					// update the movie
}

/******************************************************************************
 Draw {OVERRIDE}

		Draw a Movie and the controller if need be.
		
 ******************************************************************************/

void	CMovieController::Draw(Rect *area)
{
	Rect			tempRect;
	
	Prepare();
	
	// if there is no movie, fill with gray
	if (!itsMovie) {
		LongToQDRect( &aperture, &tempRect);
		FillRect(&tempRect, gray);
	}
	
	// draw the controller
	if (itsController)
		MCDraw(itsController, macPort);
}
