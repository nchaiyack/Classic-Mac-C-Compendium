/*****************************************************************************\
* Module: macmovie.c                                                          *
*                                                                             *
* Purpose:                                         							  *
*	This is the source for a simple utility to convert PICT files             *
*	into QuickTime movies.  It was based upon the source in the QuickTime     *
*   Developers (CD Version 1.0) sample-C-code directory: PICStoMovie.		  *
*																			  *
*   It shows how to use the Standard Compression library to request           *
*   compression setting from the user, how to use the Image Compression		  *
*	Manager to compress sequences of images, and how to use the Movie 		  *
*   Toolbox to create simple QuickTime movies.								  *
* Usage:																	  *
*	You will need to produce a sequence of PICT files of the format:          *
*	"pictfilename.#" where "#" begins with 1 and continues until the movie    *
*   ends, we reach the end of long integers = 2^32-1, or you run out of       *
*   disk space.																  *
*   This program will ask you for the location of the first frame in the      *
*   sequence (pictfilename.1) and will also give you the option of saving     *
*   your movie in any of the currently installed codecs (compressions) that   *
*   are available.															  *
*                                                                             *
* Created by: Reid Judd (ILLUMINATI@AppleLink.apple.com)                      *
* Created on: September 23, 1992                                              *
* Modified:                                                                   *
*    reid		Sept 29, 92		Call abortive_error() if (result != 0) after  *
*								  cleanup().  Also restore inReply->fName     *
\*****************************************************************************/


#include <stdio.h>

#include <memory.h>
#include <resources.h>
#include <menus.h>
#include <osevents.h>
#include <events.h>
#include <fonts.h>
#include <errors.h>

#include "movies.h"
#include "moviesformat.h"
#include "macerrors.h"

#include "StdCompression.h"
 

/*	These macros for error checking all force us to go thru cleanup() to
 *  try to sweep up any messes we've made before we call the abortive_error()
 *  routine in macerrors.c.  This routine will display a message and then
 *  attempt to put us back in the main loop again. 
 */

#define	BailOnError(result)	if (result) cleanup();
#define	BailOnNil(p)		if (!p) { result = -1; cleanup();}


/* Globals, needed for cleanup() so it can deallocate resources.
 */

short				dstMovieRefNum = 0;
Movie				dstMovie = nil;
 
ImageSequence		srcSeqID = 0;
ImageSequence		dstSeqID = 0;
	
short				srcResRefNum = 0;
Handle				compressedData = nil;
ImageDescription	**idh = nil;
WindowPtr			progressWindow = nil;
GWorldPtr			pictGWorld = nil;
 
OSErr				result = noErr;
short				quitFlag = FALSE;
 
ComponentInstance	ci; 	   /* Initialized in macstuff.c, init_mac.  Must be inited
								*   before calling EnterMovies() to init QuickTime.
								*/

 
/* ---------------------------------------------------------------
 * loadPict()
 */
PicHandle LoadPict( SFReply *Rep )
{
    GDHandle saveDevice;
    CGrafPtr savePort;
 	Str255 volName;
	short curVolRefNum, refNum;
	char buf[512];
	long len;
	PicHandle qdpic;
    GWorldPtr mygw;
 
	result = FSOpen( Rep->fName, Rep->vRefNum, &refNum );
    /* Error for file not found might mean that we're at the end of the sequence. */
    if (result == fnfErr)
      return NULL;  
      
    /* Error loading PICT file, can't open file (for reason other than fnfErr). */  
	BailOnError(result);
	
	result = GetEOF( refNum, &len );
	/* Error loading PICT file, can't get EOF */
	BailOnError(result);
	
	if (len <= 512)
	{
		/* Pict file is empty */
		result = FSClose( refNum );
		return NULL;
	}
	result = SetFPos( refNum, fsFromStart, 512L );
	/* Error loading PICT file, can't skip header */
	BailOnError(result);	
	
	qdpic = (PicHandle) NewHandle( len - 512L );
	/* Error loading PICT file, not enough memory */
	BailOnNil( qdpic );
 
	HLock( qdpic );
	len -= 512L;
	result = FSRead( refNum, &len, (Ptr) (*qdpic) );
	/* Error loading PICT file, can't read data */
	BailOnError(result);		
	
	result = FSClose( refNum );
	/* Error loading PICT file, can't close file */
	BailOnError(result);
	
	/* unlock the handle to the pict. */	
    HUnlock( qdpic );	
 
    return( qdpic );	
}
 
/* --------------------------------------------------------------- */

void convert_picts_to_movie( SFReply *inReply )
 
{
 	SCParams			p;
	Point				where;
 	SFReply				outReply;
	SFTypeList			typeList;
	Rect				r;
	short				hstate;
	short				i;
	
	int 				framenum;
	PicHandle			thePict;
	Rect				pictRect;
  	long				compressedFrameSize;
 	
 	Media				dstMedia = nil;
	Track				dstTrack = nil;
 
 	GDHandle			progressGDevice;

    GDHandle 			saveDevice;
    CGrafPtr 			savePort;	

	FSSpec				theFSSpec;
	TimeScale			dstTimeScale;
	long				frames_per_second = 12;
	char 				filename[255];

    /***************************************
	 *
	 *	Get destination movie file, 
	 *   setup filename, get first pict in sequence.
	 *
	 ***************************************/

	/*
	 *	Default destination movie file name is 
	 *  "pictfile.qt.movie".  Save the 'filename',
	 *  we'll use it again to read in the rest of the frames.
	 */
 
	BlockMove(inReply->fName,outReply.fName,64);
  	outReply.good = outReply.copy = outReply.fType = outReply.version = 0;
	outReply.vRefNum = inReply->vRefNum;
	p2cstr(outReply.fName);
	strcpy(filename, (char *) outReply.fName );
	sprintf( (char *) outReply.fName,"%s.movie", filename );
	c2pstr(outReply.fName);
		
 	
	/* Load the first pict of the sequence.  
	 *   It should have the name: pictfile.qt.1  
	 *   Default movie name will be based upon this name.
	 */		 
	framenum = 1;
	p2cstr( inReply->fName );
    sprintf( (char *) inReply->fName, "%s.%d", filename, framenum );
    c2pstr( inReply->fName );

	thePict = LoadPict( inReply );

	BailOnNil(thePict);
	pictRect = (*thePict)->picFrame;


    /***************************************
	 *
	 *	Setup QuickTime.
	 *
	 ***************************************/ 
 		
	/*	Open the standard compression dialog component. 
	 *    (this has already been done in macstuff.c, init_mac() ).
	 * 	
	   ci = OpenStdCompression();
	   BailOnNil(ci);
	
	   result = EnterMovies();
	   BailOnError(result);
 	 ****/
 		
	/*
	 *	Fill in default settings for compression dialog.  These will be the
	 *	settings displayed when the compression dialog is first displayed.
	 *	Note that these settings are only set up once so that if the dialog
	 *	is used more than once, the user's settings from the previous time
	 *	will be the defaults for the next time.
	 */

	p.flags = scShowMotionSettings;
	p.theCodecType = 'rpza';
	p.theCodec = anyCodec;
	p.spatialQuality = codecNormalQuality;
	p.temporalQuality = codecNormalQuality;
	p.depth = 8;
	p.frameRate = (long)10 << 16;
	p.keyFrameRate = 100;

 
	/***************************************
	 *
	 *	Ask user for compression parameters.
	 *
	 ***************************************/

ReaskOptions:
	
	/*
	 *	Save the purgeable state of the first PICT resource and then
	 *	make it unpurgeable.  We don't want the resource getting purged
	 *	while we're using it for the test image in standard compression.
	 */

	hstate = HGetState((Handle)thePict);
	HNoPurge((Handle)thePict);
	
	/*	Set the PICT as the test image for standard compression. */
	
	SCSetTestImagePictHandle(ci,thePict,nil,0);
	
	/*
	 *	Get compression settings from user.  Center dialog on best screen
	 *	by setting where to (-2,-2)
	 */
	
	where.h = where.v = -2;
	result = SCGetCompression(ci,&p,where);
	
	/*	Return picture to its previous purge state. */
	
	HSetState((Handle)thePict,hstate);
	
	/*	If the user selected Cancel, go ask for a new source PICS file. */
	
	if (result == 1)
		cleanup();
	BailOnError(result);

 
	/***************************************
	 *
	 *	Open a progress window.
	 *
	 ***************************************/
	
	/*	Use standard compression routines to center window on best device. */
	where.h = where.v = -2;
 
	r = pictRect;
	SCPositionRect(ci,&r,&where);
	progressWindow = NewCWindow(0,&r,outReply.fName,true,0,(WindowPtr)-1,false,0);
	BailOnNil(progressWindow);
	SetPort(progressWindow);
	
	/*	Set coordinate system of window to match that of picture. */
	
	SetOrigin(pictRect.left,pictRect.top);
	progressGDevice = GetGDevice();


	/***************************************
	 *
	 *	Prepare GWorld to draw pictures into.
	 *
	 ***************************************/

	result = NewGWorld(&pictGWorld,p.depth,&pictRect,nil,nil,0);
	BailOnError(result);
	LockPixels(pictGWorld->portPixMap);
	
	GetGWorld( &savePort, &saveDevice );

	SetGWorld(pictGWorld,nil);
	EraseRect(&pictRect);
	UnlockPixels(pictGWorld->portPixMap);


	/***************************************
	 *
	 *	Set up movie file.
	 *
	 ***************************************/
	 
	frames_per_second = (p.frameRate + 0x00008000) >> 16;
	
	/*	Create an FSSpec for the destination file. */
		
	result = FSMakeFSSpec(outReply.vRefNum,0,outReply.fName,&theFSSpec);
	if (result && result != fnfErr)
		cleanup();
		
	/*	Create the movie file, deleting the old one if it exists. */
		
	result = CreateMovieFile(&theFSSpec,'TVOD',0,createMovieFileDeleteCurFile,
							&dstMovieRefNum,&dstMovie);
	BailOnError(result);
		
	/*
	 *	Create a new track with the picture's dimensions.
	 *	Note that the dimensions are fixed point numbers.
	 */		
	dstTrack = NewMovieTrack(dstMovie,(long)(pictRect.right - pictRect.left) << 16,
									(long)(pictRect.bottom - pictRect.top) << 16,0);
		
	/*
	 *	Create a new video media with a time scale that is large
	 *	enough to accurately handle a fractional frame rate.
	 */		
	dstTimeScale = 60;
	while (frames_per_second > dstTimeScale)
		dstTimeScale *= 10;
	dstMedia = NewTrackMedia(dstTrack,VIDEO_TYPE,dstTimeScale,0,0);
	result = BeginMediaEdits(dstMedia);
	BailOnError(result);


	/***************************************
	 *
	 *	Prepare compression sequence.
	 *
	 ***************************************/

	/*	Create an uninitialized image desciption. */

	idh = (ImageDescription**)NewHandle(sizeof(ImageDescription));
	
	/*
	 *	Find out how large the largest possible compressed frame
	 *	will be, allocated memory for it, and lock it down.
	 */
	
	result = GetMaxCompressionSize(pictGWorld->portPixMap,&pictRect,p.depth,p.spatialQuality,
				p.theCodecType,p.theCodec,&compressedFrameSize);
	BailOnError(result);
	compressedData = NewHandle(compressedFrameSize);
	BailOnNil(compressedData);
	HLock(compressedData);
	
	/*
	 *	Begin the compression sequence.  Note the the image desciption will still not be
	 *	initialized after the CompressSequenceBegin call, so now is not the time to be
	 *	looking at its contents.  This may change in the future.
	 */
	
	result = CompressSequenceBegin(&srcSeqID,pictGWorld->portPixMap,nil,&pictRect,nil,p.depth,
				p.theCodecType,p.theCodec,p.spatialQuality,p.temporalQuality,
				p.keyFrameRate,nil,codecFlagUpdatePrevious,idh);
	BailOnError(result);


	/***************************************
	 *
	 *	Create and compress frames. Start at frame 1 and continue 
	 *  until a there are no more frames with the format:
	 *		pictfile.#
	 *
	 ***************************************/

	 framenum = 1;
     while (quitFlag == FALSE) {
 
		unsigned char	similarity;
		short			syncFlag;
		TimeValue		duration;
		long			flags;
		
		/*	Let the user abort by pressing the button. */
		
		if (Button())
			break;

		/*	Draw the next image from the PICS file into the pictGWorld. */

		SetGWorld(pictGWorld,nil);
		{			
 			Rect	r;
 			
 		    r = (*thePict)->picFrame;
			DrawPicture(thePict,&r);

			ReleaseResource((Handle)thePict); 
  		 
		}
		SetGWorld((CGrafPtr)progressWindow,progressGDevice);

		/*
		 *	Compress a frame.  The flags are set to codecFlagUpdatePrevious + PreviousComp
		 *	so that frame differencing is cleanup with the previous compressed image instead
		 *	of the previous source image.  This gives more accurate frame differencing.
		 *
		 *	Note that *compressedData is StripAddress-ed.  Data pointers passed into
		 *	image compression manager routines must be 32-bit clean.
		 */

		flags = codecFlagUpdatePrevious + codecFlagUpdatePreviousComp;
		result = CompressSequenceFrame(srcSeqID,pictGWorld->portPixMap,&pictRect,flags,
					StripAddress(*compressedData),&compressedFrameSize,&similarity,nil);
		BailOnError(result);

		/*
		 *	If this is the first frame, begin the decompression sequence for displaying
		 *	in the progress window.  We have to wait until after the the first
		 *	CompressSequenceFrame before we can use the image description handle idh because
		 *	it doesn't get initialized until after a CompressSequenceFrame.
		 */

		if (framenum == 1) {
			result = DecompressSequenceBegin(&dstSeqID,idh,nil,nil,&pictRect,nil,ditherCopy,
						nil,0,codecNormalQuality,anyCodec);
			BailOnError(result);
		}

		/*
		 *	Add the newly compressed frame to the dstMedia.  We set the syncFlag based on
		 *	the similarity returned by CompressSequenceFrame.  If the similarity is non-zero,
		 *	the frame has some frame differencing in it and therefore is not a sync sample.
		 *	If the similarity is zero, there is no frame differencing and the frame is a
		 *	sync/key frame.
		 */
		 
		/* default duration is set to 12 frames/sec: 60/12 = 5 */
		duration = dstTimeScale/frames_per_second;  
 
		syncFlag = (similarity ? mediaSampleNotSync : 0);
		result = AddMediaSample(dstMedia,compressedData,0,compressedFrameSize,
					duration,
					(SampleDescriptionHandle)idh,1,syncFlag,nil);
		BailOnError(result);
		
		/*
		 *	Decompress the current frame to the progress window.  Again notice that
		 *	the dereferenced compressedData handle was StripAddressed.
		 */
		
		if (progressWindow) {
			CodecFlags	outFlags;
			result = DecompressSequenceFrame(dstSeqID,StripAddress(*compressedData),0,&outFlags,nil);
			BailOnError(result);
		}
		
		
		/* Get the next frame in the sequence.   
		 */
		framenum++;
	
        p2cstr( inReply->fName );
 		sprintf( (char *) inReply->fName, "%s.%d", filename, framenum );
	    c2pstr( inReply->fName );

		thePict = LoadPict( inReply );
 		if (thePict == NULL)
		  quitFlag = TRUE;     	        
	}
	HUnlock((Handle)compressedData);


	/***************************************
	 *
	 *	Finish creating destination movie file.
	 *
	 ***************************************/

	{
		short resID = 0;
		result = EndMediaEdits(dstMedia);
		BailOnError(result);
		InsertMediaIntoTrack(dstTrack,0,0,GetMediaDuration(dstMedia),1L<<16);
		BailOnError(result = GetMoviesError());
 	
		result = AddMovieResource(dstMovie,dstMovieRefNum,
							&resID, outReply.fName );

		BailOnError(result);

		dstMedia = nil;
	}

	/***************************************
	 *
	 *	Return the input filename to what it was originally
	 *
	 ***************************************/

 	 strcpy( (char *) inReply->fName, filename );
	 c2pstr( inReply->fName );
 
	/***************************************
	 *
	 *	Deallocate everything that was created.
	 *
	 ***************************************/
	 
	 cleanup();
	 
     if (ci)
	    CloseComponent(ci);		 
	    
	 return;
 
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *
 * cleanup:
 *		Deallocate everything that was created.	 If no error, 
 *		continue, otherwise we're bailing out.
 */
cleanup()
{

	if (dstMovieRefNum) {
		CloseMovieFile(dstMovieRefNum);
		dstMovieRefNum = 0;
	}

	if (dstMovie) {
		DisposeMovie(dstMovie);
		dstMovie = nil;
	}

	if (srcSeqID) {
		CDSequenceEnd(srcSeqID);
		srcSeqID = 0;
	}
	
	if (dstSeqID) {
		CDSequenceEnd(dstSeqID);
		dstSeqID = 0;
	}	

	if (srcResRefNum) {
		CloseResFile(srcResRefNum);
		srcResRefNum = 0;
	}
	
	if (compressedData) {
		DisposHandle(compressedData);
		compressedData = nil;
	}
	
	if (idh) {
		DisposHandle((Handle)idh);
		idh = nil;
	}
	
	if (progressWindow) {
		CloseWindow(progressWindow);
		progressWindow = nil;
	}
	
	if (pictGWorld) {
		DisposeGWorld(pictGWorld);
		pictGWorld = nil;
	}

    if (result != 0)
      abortive_error( result );

 
  
} 	
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  