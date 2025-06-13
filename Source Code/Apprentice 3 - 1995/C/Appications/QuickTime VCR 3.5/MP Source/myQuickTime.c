/*---------------------------------------------------------------------
	File: myQuickTime.c
	Purpose: Contains stuff to manage quicktime stuff such as openning
	files, setting rates, and such.
	
	Created by: Geoffrey Slinker
	Date: 10:58:03 AM  6/13/92
	Modified: 
---------------------------------------------------------------------*/
#include <GestaltEqu.h>
#include <Movies.h>
#include <MoviesFormat.h>
#include <FixMath.h>

#include <oops.h>

#include "myQuickTime.h"
#include "Alert_Class.h"
#include "Dlog.h"
#include "vcr.h"

extern Alert_Class *myAlert;
extern VCR *vcr;
extern Boolean opened;

/*...................................................................*/
void QT::QT()
{
	movieWindow = NULL;
	moviePosterWindow = NULL;
	myMovie = NULL;
}
/*...................................................................*/
void QT::~QT()
{
	CWindowPtr movieWindow,moviePosterWindow;
	Movie myMovie;
	
	movieWindow = (*this).movieWindow;
	moviePosterWindow = (*this).moviePosterWindow;
	myMovie = (*this).myMovie;
	
	
	if (movieWindow != NULL) {
		DisposeWindow((WindowPtr)movieWindow);
	}
	
	if (moviePosterWindow != NULL) {
		DisposeWindow(moviePosterWindow);
	}
	
	
	if (myMovie != NULL) {
		DisposeMovie(myMovie);
	}
	(*this).movieWindow = movieWindow;
	(*this).moviePosterWindow = moviePosterWindow;
	(*this).myMovie = myMovie;
}
/*...................................................................*/
void QT::MovieCheck()
{
	long	version;
	
	
	if (Gestalt( gestaltQuickTime, &version ) != noErr){
		(*myAlert).AlertStop("\pFatal Error!",
								"\pQuickTime not installed",
								"\pPlease Install QuickTime",
								"\pSorry.");
	}
	
	if(EnterMovies() != noErr)	{
		(*myAlert).AlertStop("\pFatal Error!",
								"\pUnable to Enter Movies",
								"\p",
								"\pSorry.");
	}
}
/*...................................................................*/
void QT::LoadResources()
{
	CWindowPtr movieWindow, moviePosterWindow;
	
	movieWindow = (CWindowPtr)GetNewCWindow(MOVIE_WIND,(Ptr)0L,(CWindowPtr)(-1L));
	if (movieWindow == NULL) {
		SysBeep(7);
		(*myAlert).AlertStop("\pFatal Error",
							"\pUnable to allocate new color window",
							"\pQT::LoadResources",NULL);
	}
	
	moviePosterWindow = (CWindowPtr)GetNewCWindow(MOVIE_POSTER_WIND,(Ptr)0L,(CWindowPtr)(-1L));
	if (moviePosterWindow == NULL) {
		SysBeep(7);
		(*myAlert).AlertStop("\pFatal Error",
							"\pUnable to allocate new color window",
							"\pQT::LoadResources",NULL);
	}
	
	(*this).origBounds = (*movieWindow).portRect;
	(*this).movieWindow = movieWindow;
	(*this).moviePosterWindow = moviePosterWindow;
}
/*...................................................................*/
/*
FUNCTION OpenMovieFile (fileSpec: FSSpec; VAR resRefNum: Integer; perms: SignedByte): OSErr;
fileSpec	Contains a pointer to the file system specification for the movie
			file to be opened.
resRefNum	Contains a pointer to a field that is to receive the file reference 
			number for the opened movie file. Your application must use this value 
			when calling other Movie Toolbox routines that work with movie files.
perms		Specifies the permission level for the file. If your application is 
			only going to play the movie that is stored in the file, you can open 
			the file with read permission. If you plan to add data to the file or 
			change data in the file, you should open the file with write permission. 
			Supply avalid File Manager permission value. See Inside Macintosh, 
			VolumeÊII for valid values
*/
/*...................................................................*/
int QT::OpenMovie()
{	
	PicHandle poster;
	Movie myMovie;
	CWindowPtr movieWindow,moviePosterWindow;
	StandardFileReply		sfr;
	SFTypeList	types =	{'MooV'};
	short	resRefNum;
	short numTypes = 1;
	int w,h;
	
	// Shadow variables because the class are handles
	// and therefore purgeable.
	
	myMovie = (*this).myMovie;
	movieWindow = (*this).movieWindow;
	moviePosterWindow = (*this).moviePosterWindow;
	
	
	StandardGetFilePreview(0, numTypes, types, &sfr);
	if (!sfr.sfGood) return(0);				/* Return if no selection */
	
	if (OpenMovieFile(&(sfr.sfFile), &resRefNum, 0) != noErr) {
		(*myAlert).AlertCaution("\pUnable to open movie","\p","\p","\p");
		return(0);
	}
	
	SetWTitle(movieWindow,sfr.sfFile.name);
	
	if (NewMovieFromFile( &myMovie,resRefNum, nil, nil,0, nil ) != noErr) {
		SysBeep(7);
		(*myAlert).AlertStop("\pFatal Error",
							"\pUnable to get NewMovieFromFile",
							"\pQT::OpenMovie",NULL);
	}
	if (CloseMovieFile( resRefNum ) != noErr) {
		SysBeep(7);
		(*myAlert).AlertStop("\pFatal Error",
							"\pUnable to CloseMovieFile",
							"\pQT::OpenMovie",NULL);
	}	
									
	/* Get movie box and place movie at 0,0 */
	GetMovieBox( myMovie, &dispBounds);
	OffsetRect(&dispBounds,-dispBounds.left,-dispBounds.top);
	SetMovieBox(myMovie, &dispBounds);		

	SetPort(movieWindow);
	SetMovieGWorld(myMovie,nil,nil);
	
	GoToBeginningOfMovie(myMovie);
	PrerollMovie(myMovie,0,0);				
	SetMovieActive(myMovie,true);
	
	w = dispBounds.right - dispBounds.left;
	h = dispBounds.bottom - dispBounds.top;
	
	SizeWindow( (WindowPtr)movieWindow,w,h,true);
	SizeWindow( (WindowPtr)moviePosterWindow,w,h,true);
	
	poster = GetMoviePosterPict( myMovie );

		if (poster != nil)
		{
			GrafPtr oldPort;
			Rect rect;
			
			GetPort(&oldPort);
			SetPort(moviePosterWindow);
			rect = (*moviePosterWindow).portRect;
					
			DrawPicture( poster, &rect );
			KillPicture( poster );
			
			SetPort(oldPort);
		}
	
	(*this).myMovie = myMovie;
	return(1);

}
/*...................................................................*/
void QT::PlayMovie()
{
	Movie myMovie;
	CWindowPtr movieWindow;
	
	myMovie = (*this).myMovie;
	movieWindow = (*this).movieWindow;
	
	
	StartMovie(myMovie);					


}
/*...................................................................*/
void QT::CloseMovie()
{
	Movie myMovie;
	int w,h;
	CWindowPtr movieWindow, moviePosterWindow;
	GrafPtr oldPort;
	Rect origBounds;
	
	GetPort(&oldPort);
	myMovie = (*this).myMovie;
	
	if (myMovie != NULL) {
		DisposeMovie(myMovie);
	}
	myMovie = NULL;  // This has to be here, apparently DisposeMovie didn't set
				  // the myMovie variable to NULL
	(*this).myMovie = myMovie;
	
	movieWindow = (*this).movieWindow;
	moviePosterWindow = (*this).moviePosterWindow;
	origBounds = (*this).origBounds;
	
	
	w = origBounds.right - origBounds.left;
	h = origBounds.bottom - origBounds.top;
	
	SizeWindow( (WindowPtr)movieWindow,w,h,true);
	SizeWindow( (WindowPtr)moviePosterWindow,w,h,true);
	
	SetPort(movieWindow);
	SetWTitle(movieWindow, "\pMovie Window");
	EraseRect(&origBounds);
	
	SetPort(moviePosterWindow);
	EraseRect(&origBounds);
	
	SetPort(oldPort);
	
	(*this).movieWindow = movieWindow;
	(*this).moviePosterWindow = moviePosterWindow;
	
	(*vcr).ClearAllButtons();
	
}
/*...................................................................*/
void QT::MyMoviesTask(void)
{	
	Movie myMovie;
	
	myMovie = (*this).myMovie;
	if ((myMovie != NULL) && (opened))
	{
		
		MoviesTask(myMovie,0);
		if (IsMovieDone(myMovie))	// Have we reached the end yet?		
		{	
			if ( (*vcr).vcr_mode == playing) {
				GoToBeginningOfMovie( myMovie );
			}
			else {
				(*vcr).ClearAllButtons();
				//(*vcr).RedrawButtons();
				//GoToBeginningOfMovie( myMovie );
			}
		}
	}
}
/*...................................................................*/
void QT::StopMyMovie(void)
{
	Movie myMovie;
	
	myMovie = (*this).myMovie;
	
	if(myMovie != NULL) {
		StopMovie( myMovie );
	}
	(*this).myMovie = myMovie;
}
/*...................................................................*/
void QT::FFMyMovie(void)
{
	Movie myMovie;
	
	myMovie = (*this).myMovie;
	if (myMovie != NULL) {
		SetMovieRate( myMovie, Long2Fix( 2L ) );
	}
	
	(*this).myMovie = myMovie;
}
/*...................................................................*/
void QT::RWMyMovie(void)
{	
	Movie myMovie;
	
	myMovie = (*this).myMovie;
	if (myMovie != NULL) {
		SetMovieRate( myMovie, Long2Fix( -2L) );
	}
	
	(*this).myMovie = myMovie;
}
/*...................................................................*/		
void QT::GoStartMyMovie(void)
{
	Movie myMovie;
	
	myMovie = (*this).myMovie;
	if (myMovie != NULL) {
		StopMovie(myMovie);
		GoToBeginningOfMovie(myMovie);
	}
	
	(*this).myMovie = myMovie;
}
/*...................................................................*/
void QT::GoEndMyMovie(void)
{
	Movie myMovie;
	
	myMovie = (*this).myMovie;
	if (myMovie != NULL) {
		StopMovie(myMovie);
		GoToEndOfMovie(myMovie);
	}
	
	(*this).myMovie = myMovie;

}
/*...................................................................*/
void QT::RedrawPoster()
{
	PicHandle poster;
	CWindowPtr moviePosterWindow;
	GrafPtr oldPort;
	Movie myMovie;
	
	GetPort(&oldPort);
	
	moviePosterWindow = (*this).moviePosterWindow;
	myMovie = (*this).myMovie;
	
	if (myMovie != NULL) {
		poster = GetMoviePosterPict( myMovie );

		if (poster != nil)
		{
			GrafPtr oldPort;
			Rect rect;
			
			GetPort(&oldPort);
			SetPort(moviePosterWindow);
			rect = (*moviePosterWindow).portRect;
			
			//SetWindowPic( (WindowPtr)moviePosterWindow,poster);
			//InsetRect( &rect, 3, 3 );
			
			DrawPicture( poster, &rect );
			KillPicture( poster );
			
			SetPort(oldPort);
		}
	}
	SetPort(oldPort);
}
/*...................................................................*/
void QT::HandleVolume(int direction)
{
	long volume;
	Movie myMovie = (*this).myMovie;
	volume = GetMovieVolume(myMovie);
		
	switch (direction) {
		case UP:
			volume += 50;
			break;
		case DOWN:
			volume -= 50;
			break;
			
		default : break;
	
	}
	
	if (volume > 255) volume = 255;
	if (volume < 0) volume = 0;
	SetMovieVolume(myMovie, (int)volume );
	(*this).myMovie = myMovie;

}
/*...................................................................*/
int QT::OpenMovieFSSpec(FSSpec mySpec)
{	
	PicHandle poster;
	Movie myMovie;
	CWindowPtr movieWindow,moviePosterWindow;
	short	resRefNum;
	short numTypes = 1;
	int w,h;
	
	// Shadow variables because the class are handles
	// and therefore purgeable.
	
	myMovie = (*this).myMovie;
	movieWindow = (*this).movieWindow;
	moviePosterWindow = (*this).moviePosterWindow;
	
	
	if (OpenMovieFile(&mySpec, &resRefNum, 0) != noErr) {
		(*myAlert).AlertCaution("\pUnable to open movie","\p","\p","\p");
		return(0);
	}
	
	SetWTitle(movieWindow,mySpec.name);
	
	if (NewMovieFromFile( &myMovie,resRefNum, nil, nil,0, nil ) != noErr) {
		SysBeep(7);
		(*myAlert).AlertStop("\pFatal Error",
							"\pUnable to get NewMovieFromFile",
							"\pQT::OpenMovie",NULL);
	}
	if (CloseMovieFile( resRefNum ) != noErr) {
		SysBeep(7);
		(*myAlert).AlertStop("\pFatal Error",
							"\pUnable to CloseMovieFile",
							"\pQT::OpenMovie",NULL);
	}	
									
	/* Get movie box and place movie at 0,0 */
	GetMovieBox( myMovie, &dispBounds);
	OffsetRect(&dispBounds,-dispBounds.left,-dispBounds.top);
	SetMovieBox(myMovie, &dispBounds);		

	SetPort(movieWindow);
	SetMovieGWorld(myMovie,nil,nil);
	
	GoToBeginningOfMovie(myMovie);
	PrerollMovie(myMovie,0,0);				
	SetMovieActive(myMovie,true);
	
	w = dispBounds.right - dispBounds.left;
	h = dispBounds.bottom - dispBounds.top;
	
	SizeWindow( (WindowPtr)movieWindow,w,h,true);
	SizeWindow( (WindowPtr)moviePosterWindow,w,h,true);
	
	poster = GetMoviePosterPict( myMovie );

		if (poster != nil)
		{
			GrafPtr oldPort;
			Rect rect;
			
			GetPort(&oldPort);
			SetPort(moviePosterWindow);
			rect = (*moviePosterWindow).portRect;
					
			DrawPicture( poster, &rect );
			KillPicture( poster );
			
			SetPort(oldPort);
		}
	
	(*this).myMovie = myMovie;
	return(1);

}
/*...................................................................*/
/*...................................................................*/
/*...................................................................*/
/*...................................................................*/
