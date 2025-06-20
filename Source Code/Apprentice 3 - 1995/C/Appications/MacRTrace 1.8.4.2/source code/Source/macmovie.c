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
*								movie_cleanup().  Also restore inReply->fName *
*    January 3, 1995 by Greg Ferrar                                           *
*      changed name of cleanup to movie_cleanup to fix name collision.        *
*    January 4, 1995 by Greg Ferrar                                           *
*      added prototypes.                                                      *
\*****************************************************************************/


#include <stdio.h>
#include <string.h>

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

//#if !__powerc

/*	Think C likes all caps for routines in converted .o files. */

//#ifdef THINK_C
#define	OpenStdCompression	OPENSTDCOMPRESSION
//#endif

//#include "StdCompression.h"

//#else
#include <QuickTimeComponents.h>
//#endif


/************************** Prototypes *************************/
PicHandle LoadPict(SFReply *Rep);
void movie_cleanup(void);
void init_quicktime_save( SFReply *inReply );
void compress_one_movie_frame(long frame);
void finish_movie(void);

/*************************** Externals *******************************/
extern PicHandle	bitmap_pict;			/* The Picture which contains the image bitmap */
extern CGrafPtr  	image_port;				/* The port of the image bitmap */


/*	These macros for error checking all force us to go thru movie_cleanup() to
 *  try to sweep up any messes we've made before we call the abortive_error()
 *  routine in macerrors.c.  This routine will display a message and then
 *  attempt to put us back in the main loop again. 
 */

#define	BailOnError(result)	if (result) movie_cleanup();
#define	BailOnNil(p)		if (!p) { result = -1; movie_cleanup();}


/* Globals, needed for movie_cleanup() so it can deallocate resources.
 */

short				dstMovieRefNum = 0;
Movie				dstMovie = nil;
ComponentInstance	ci;

ImageSequence		srcSeqID = 0;
ImageSequence		dstSeqID = 0;
	
short				srcResRefNum = 0;
Handle				compressedData = nil;
ImageDescription	**idh = nil;
//GWorldPtr			pictGWorld = nil;
 
OSErr				result = noErr;
short				quitFlag = FALSE;
 
TimeScale			dstTimeScale;
Media				dstMedia = nil;
Track				dstTrack = nil;
long				frames_per_second = 12;
Str63				dstMovieFilename;


/*****************************************************************************\
* procedure init_quicktime_save                                               *
*                                                                             *
* Purpose: This procedure prepares for an animation render which is going to  *
*          be saved as a QuickTime movie.                                     *
*                                                                             *                                                                             *
* Parameters: outReply: the file to write the movie to.                       *
*             returns FALSE iff user cancels                                  *
*                                                                             *                                                                             *
* Created by: Reid Judd                                                       *
* Modified:                                                                   *
*   January 11, 1994 by Greg Ferrar                                           *
*     Split movie-generation routine into multiple routines to allow support  *
*     for on-the-fly movie generation.                                        *
\*****************************************************************************/

void init_quicktime_save(SFReply *outReply)
 
{
 	SCParams			p;
	Point				where;
	SFTypeList			typeList;
	Rect				r;
	short				i;
	
	int 				framenum;
	Rect				pict_rect;
  	long				compressedFrameSize;
 	
	ComponentInstance	ci;
	
    GDHandle 			saveDevice;
    CGrafPtr 			savePort;	

	FSSpec				theFSSpec;
	char 				filename[255];

	/* Get the frame of the picture */
	pict_rect = (*bitmap_pict)->picFrame;

    /***************************************
	 *
	 *	Setup QuickTime.
	 *
	 ***************************************/ 
 		
	/*	Open the standard compression dialog component. */

//#if !__powerc
//	ci = OpenStdCompression();
//#else
	ci = OpenDefaultComponent(StandardCompressionType, StandardCompressionSubType);
//#endif
	BailOnNil(ci);

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
	
	/*	Set the PICT as the test image for standard compression. */
	
	SCSetTestImagePictHandle(ci,bitmap_pict,nil,0);
	
	/*
	 *	Get compression settings from user.  Center dialog on best screen
	 *	by setting where to (-2,-2)
	 */
	
	where.h = where.v = -2;
	result = SCGetCompression((long) ci, &p, where);
	
	/*	If the user selected Cancel, go ask for a new source PICS file. */
	
	if (result == 1)
		movie_cleanup();
	BailOnError(result);

	/***************************************
	 *
	 *	Set up movie file.
	 *
	 ***************************************/
	 
	frames_per_second = (p.frameRate + 0x00008000) >> 16;
	
	/*	Create an FSSpec for the destination file. */
		
	result = FSMakeFSSpec(outReply->vRefNum,0,outReply->fName,&theFSSpec);
	if (result && result != fnfErr)
		movie_cleanup();
		
	/*	Create the movie file, deleting the old one if it exists. */
		
	result = CreateMovieFile(&theFSSpec,'TVOD',0,createMovieFileDeleteCurFile,
							&dstMovieRefNum,&dstMovie);
	BailOnError(result);
		
	/*
	 *	Create a new track with the picture's dimensions.
	 *	Note that the dimensions are fixed point numbers.
	 */		
	dstTrack = NewMovieTrack(dstMovie,(long)(pict_rect.right - pict_rect.left) << 16,
									(long)(pict_rect.bottom - pict_rect.top) << 16,0);
		
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
	
	result = GetMaxCompressionSize(image_port->portPixMap, &pict_rect, p.depth, p.spatialQuality,
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
	
	result = CompressSequenceBegin(&srcSeqID,image_port->portPixMap,nil,&pict_rect,nil,p.depth,
				p.theCodecType,p.theCodec,p.spatialQuality,p.temporalQuality,
				p.keyFrameRate,nil,codecFlagUpdatePrevious,idh);
	BailOnError(result);

}	/* init_quicktime_save() */



/*****************************************************************************\
* procedure compress_one_movie_frame                                          *
*                                                                             *
* Purpose: This procedure compresses one frame into the movie we're building. *
*                                                                             *                                                                             *
* Parameters: none.                                                           *                                                                             *
*                                                                             *                                                                             *
* Created by: Reid Judd                                                       *
* Modified:                                                                   *
*   January 11, 1994 by Greg Ferrar                                           *
*     Split movie-generation routine into multiple routines to allow support  *
*     for on-the-fly movie generation.                                        *
\*****************************************************************************/

void compress_one_movie_frame(long frame)
{

	Rect			pict_rect = (*bitmap_pict)->picFrame;
	CGrafPtr		temp_port;
	GDHandle 		temp_gdh;
	unsigned char	similarity;
	short			syncFlag;
	TimeValue		duration;
	long			flags;
	long			compressedFrameSize;
	
	/*
	 *	Compress a frame.  The flags are set to codecFlagUpdatePrevious + PreviousComp
	 *	so that frame differencing is cleanup with the previous compressed image instead
	 *	of the previous source image.  This gives more accurate frame differencing.
	 *
	 *	Note that *compressedData is StripAddress-ed.  Data pointers passed into
	 *	image compression manager routines must be 32-bit clean.
	 */

	flags = codecFlagUpdatePrevious + codecFlagUpdatePreviousComp;
	result = CompressSequenceFrame(srcSeqID,image_port->portPixMap,&pict_rect,flags,
				StripAddress(*compressedData),&compressedFrameSize,&similarity,nil);
	BailOnError(result);

	/*
	 *	If this is the first frame, begin the decompression sequence for displaying
	 *	in the progress window.  We have to wait until after the the first
	 *	CompressSequenceFrame before we can use the image description handle idh because
	 *	it doesn't get initialized until after a CompressSequenceFrame.
	 */

	if (frame == 1)
		{
		result = DecompressSequenceBegin(&dstSeqID,idh,nil,nil,&pict_rect,nil,ditherCopy,
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
	
	HUnlock((Handle)compressedData);

}	/* compress_one_movie_frame() */



/*****************************************************************************\
* procedure finish_movie                                                      *
*                                                                             *
* Purpose: This procedure finished building the movie.                        *
*                                                                             *                                                                             *
* Parameters: none.                                                           *                                                                             *
*                                                                             *                                                                             *
* Created by: Reid Judd                                                       *
* Modified:                                                                   *
*   January 11, 1994 by Greg Ferrar                                           *
*     Split movie-generation routine into multiple routines to allow support  *
*     for on-the-fly movie generation.                                        *
\*****************************************************************************/

void finish_movie(void)
{

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
							&resID, dstMovieFilename);

		BailOnError(result);

		dstMedia = nil;
	}

	/***************************************
	 *
	 *	Deallocate everything that was created.
	 *
	 ***************************************/
	 
	 movie_cleanup();
	 
     if (ci)
	    CloseComponent(ci);		 
	    
	 return;
 
}	/* finish_movie() */


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *
 * movie_cleanup:
 *		Deallocate everything that was created.	 If no error, 
 *		continue, otherwise we're bailing out.
 */
void movie_cleanup(void)
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
	
    if (result != 0)
      abortive_error( result );
  
} 	
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  