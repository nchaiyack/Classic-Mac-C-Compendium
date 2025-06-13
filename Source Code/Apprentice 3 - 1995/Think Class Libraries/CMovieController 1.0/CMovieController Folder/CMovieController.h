/******************************************************************************
 CMovieController.h

		
	SUPERCLASS = CPane
	
	Copyright © 1994 Johns Hopkins University. All rights reserved.
	
 ******************************************************************************/
 
#pragma once

#include <Movies.h>
#include <CPane.h>					/* Interface for its superclass		*/

class CMovieController : public CPane
{
public:
								/** Data Members **/
	Movie				itsMovie;
	MovieController		itsController;
	short				itsMovieRefNum;
	Str255				itsMovieTitle;

								/** Methods **/
	void				IMovieController(CView *anEnclosure, CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing);
    
	virtual void		IViewTemp(CView *anEnclosure, CBureaucrat *aSupervisor, Ptr viewData);

	virtual void		OpenMovie(void);
    virtual void        Dispose(void);
    virtual void        ForgetMembers(void);

	virtual void		SetMovie(FSSpec *spec);
	
	virtual void		Activate(void);
	virtual void		Deactivate(void);
	virtual void		DoKeyDown(char theChar, Byte keyCode, EventRecord *macEvent);
	virtual void		DoClick(Point hitPt,short modifierKeys,long when);
	virtual void		Dawdle(long	*maxSleep);
	virtual void		Draw(Rect *area);

protected:

	void				IMovieControllerX(void);
	short				LocateMovie(FSSpec *spec);
	OSErr 				NameFileSearch(StringPtr volName,
							   short vRefNum,
							   const Str255 fileName,
							   FSSpecPtr matches,
							   long reqMatchCount,
							   long *actMatchCount,
							   Boolean newSearch,
							   Boolean partial);
};

typedef struct tMovieControllerTemp
{
	PaneTemp	paneTemp;
	Str255		movieTitle;
	
} tMovieControllerTemp, *tMovieControllerTempP;
