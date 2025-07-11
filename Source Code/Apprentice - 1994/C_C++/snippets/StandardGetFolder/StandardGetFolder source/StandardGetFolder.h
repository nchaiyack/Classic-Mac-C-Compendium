/*************************************************************************************************

StandardGetFolder.h -- Copyright Chris Larson, 1993 -- All rights reserved.
                       Based partly upon StandardGetFolder example by Steve Falkenburg (MacDTS)
                       and partly on the code in Inside Macintosh: Files.
                       
                       Dialog box layouts taken from Inside Macintosh: Files.
                       
 Include this file in your #includes to use StandardGetFolder.

	Bug Reports/Comments to cklarson@engr.ucdavis.edu

	Version 1.0

*************************************************************************************************/

#ifndef __STANDARDGETFOLDER__
#define __STANDARDGETFOLDER__

// ----------
// Type Declarations
// ----------

typedef struct
	{
	short		sfGood;
	short		sfVRefNum;
	long		sfDirID;
	} StandardFolderReply, *StandardFolderReplyPtr, **StandardFolderReplyHandle;

// ----------
// Function Prototypes
// ----------

#ifdef __cplusplus
extern "C" {
#endif

OSErr StandardGetFolder (Str255 prompt, StandardFolderReplyPtr result, ProcPtr windowProc);

#ifdef __cplusplus
}
#endif

#endif