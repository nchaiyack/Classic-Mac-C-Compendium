/* File Access Functions
 *
 * A collection of routines that should make it a bit easier to access
 * Macintosh files AFTER they're open.
 *
 * This code is Copyright 1994 by Ken Worley. All Rights Reserved.
 *
 * You may use this code for any purpose you like and you do not
 * need to credit me in any way. However, if you redistribute
 * this source code, I must be identified as the author.
 * If you modify the routines and redistribute the code, it
 * must be identified as having been modified from my original.
 *
 * In general, the routines in this file make it easier to access
 * byte, short int, and long int sized data (as well as larger data
 * blocks) by simply specifying a record number. The data can be
 * read or written just as easily in simple sequential order using
 * the same routines. The routines are more efficient than the
 * Toolbox high level routines because they use static parameter
 * blocks and low level routines for file access, but they retain
 * a very simple interface.
 *
 * By the way, I'm Ken Worley of Apt Ideas Software. I can be reached
 * on America Online or eWorld at KNEworley or via the internet at
 * KNEworley@eworld.com.
 *
 * The routines in this file view files as collections of numbered
 * records that are all the same size with the first record being
 * record number zero just like in a C array. Access is accomplished
 * by specifying a file reference number (of an open file), a record
 * number, and a pointer to the data to be written or storage for
 * data to be read. Most of the functions are Boolean indicating
 * whether or not they were successful. If you're interested in the
 * specific error when a function fails, you can call LastFileError
 * to retrieve it.
 *
 * Now, to the specifics of the routines provided in this file:
 *
 * Here's an important define used in the routines:
 *
 *		#define kSequential	-1		Used to indicate that a read or
 *									write operation should be done
 *									at the current file position
 *									when passed as the record #.
 *
 * Here are the routines contained in this file along with explanations
 * as to their functions:
 *
 *		Boolean ReadByte( short fileRef, long recordNo, char* buffer );
 *							Reads a byte sized record
 *
 *		Boolean WriteByte( short fileRef, long recordNo, char buffer );
 *							Writes a byte sized record
 *
 *		Boolean ReadShort( short fileRef, long recordNo, short* buffer );
 *							Reads a short int sized record (2 bytes)
 *
 *		Boolean WriteShort( short fileRef, long recordNo, short buffer );
 *							Writes a short int sized record (2 bytes)
 *
 *		Boolean ReadLong( short fileRef, long recordNo, long* buffer );
 *							Reads a long int sized record (4 bytes)
 *
 *		Boolean WriteLong( short fileRef, long recordNo, long buffer );
 *							Writes a long int sized record (4 bytes)
 *
 *
 *		Boolean ReadRecord( short fileRef, long recordNo, void* buffer );
 *							Reads a record that is whatever size
 *							the storage allocated for the data is.
 *
 *		Boolean WriteRecord( short fileRef, long recordNo, void* buffer );
 *							Writes a record whatever size it is.
 *
 *
 *		Boolean ReadBlock( short fileRef, long filePos,
 *								long bytesRequested, void* buffer );
 *							Called by the other routines - the low
 *							level interface to the toolbox routines
 *							for reading from the file.
 *
 *		Boolean WriteBlock( short fileRef, long filePos,
 *								long bytesRequested, void* buffer );
 *							Called by the other routines - the low
 *							level interface to the toolbox routines
 *							for writing to the file.
 *
 *		long	RemainingSpace( short vRefNum );
 *							Returns the number of bytes free on
 *							the volume specified.
 *
 *
 *		long	RecordsInFile( short refNum, short recordSize );
 *							Returns the current number of records
 *							in the file based on the file size and
 *							record size.
 *
 *
 *		long	GetCurrentRecord( short refNum, short recordSize );
 *							Returns the record number that would be
 *							read or written if kSequential were
 *							sent as the record number in the next
 *							call to one of the file access routines.
 *
 *		Boolean	SetCurrentRecord( short refNum, long recordNo,
 *												short recordSize );
 *							Sets the file mark so that the specified
 *							record would be read or written
 *							if kSequential were sent as the record
 *							number in the next call to one of the
 *							file access routines.
 *
 *
 *		short	LastFileError( void );
 *							Get the error number of the last
 *							function call.
 */

#include "FileRoutines.h"

static ParamBlockRec	PB;		/* This is the parameter block */
								/* used to pass info back and */
								/* forth to/from the low level */
								/* toolbox routines. */
	
static short 			gFileError;	/* This global holds the error */
									/* number of the last call. */

Boolean ReadByte( short fileRef, long recordNo, char* buffer )
{
	return ReadBlock( fileRef, recordNo, 1, buffer );
}


Boolean WriteByte( short fileRef, long recordNo, char buffer )
{
	char value;
	
	value = buffer;
	return WriteBlock( fileRef, recordNo, 1, &value );
}



Boolean ReadShort( short fileRef, long recordNo, short* buffer )
{
	long	filePos;
	
	if ( recordNo == kSequential )
		filePos = kSequential;
	else
		filePos = ( 2 * recordNo );
	
	return ReadBlock( fileRef, filePos, 2, buffer );
}


Boolean WriteShort( short fileRef, long recordNo, short buffer )
{
	long	filePos;
	short	value;
	
	value = buffer;
	
	if ( recordNo == kSequential )
		filePos = kSequential;
	else
		filePos = ( 2 * recordNo );
	
	return WriteBlock( fileRef, filePos, 2, &value );
}



Boolean ReadLong( short fileRef, long recordNo, long* buffer )
{
	long	filePos;
	
	if ( recordNo == kSequential )
		filePos = kSequential;
	else
		filePos = ( 4 * recordNo );
	
	return ReadBlock( fileRef, filePos, 4, buffer );
}


Boolean WriteLong( short fileRef, long recordNo, long buffer )
{
	long	filePos;
	long	value;
	
	value = buffer;
	
	if ( recordNo == kSequential )
		filePos = kSequential;
	else
		filePos = ( 4 * recordNo );
	
	return WriteBlock( fileRef, filePos, 4, &value );
}



Boolean ReadRecord( short fileRef, long recordNo, void* buffer )
{
	long	filePos;
	long	bytes;
	
	bytes = GetPtrSize( buffer );
	
	if ( recordNo == kSequential )
		filePos = kSequential;
	else
		filePos = ( bytes * recordNo );

	return ReadBlock( fileRef, filePos, bytes, buffer );
}


Boolean WriteRecord( short fileRef, long recordNo, void* buffer )
{
	long	filePos;
	long	bytes;
	
	bytes = GetPtrSize( buffer );
	
	if ( recordNo == kSequential )
		filePos = kSequential;
	else
		filePos = ( bytes * recordNo );

	return WriteBlock( fileRef, filePos, bytes, buffer );
}



Boolean ReadBlock( short fileRef, long filePos, long bytesRequested,
														void* buffer )
{
/* This routine sets up an i/o parameter block and calls PBRead      */
/* to read in the specified amount of data at the specified position */
/* in the file. If filePos is kSequential, then the data is read     */
/* at the file's current mark position. After the read operation,    */
/* PBRead leaves the file's mark at the end of the data read.        */

	/* Prepare the parameter block for the read function call */
	
	PB.ioParam.ioCompletion = NULL;		/* no completion routine */
	PB.ioParam.ioRefNum = fileRef;		/* file reference number */
	PB.ioParam.ioBuffer = (Ptr)buffer;	/* data storage */
	PB.ioParam.ioReqCount = bytesRequested;
										/* bytes we reqested read */
	if ( filePos == kSequential )
	{
		PB.ioParam.ioPosMode = fsAtMark;	/* read at current mark */
		PB.ioParam.ioPosOffset = 0;			/* not used in this case */
	}
	else
	{
		PB.ioParam.ioPosMode = fsFromStart;	/* read at offset from bof */
		PB.ioParam.ioPosOffset = filePos;	/* where in file to read */
	}
	
	/* Now call PBRead to read the data in */
	
	PBRead( &PB, false );			/* false is to force */
									/* synchronous operation */
	
	gFileError = PB.ioParam.ioResult;	/* store error code in global */
	
	if ( PB.ioParam.ioResult == noErr )
		return true;
	else
		return false;
}


Boolean WriteBlock( short fileRef, long filePos, long bytesRequested,
														void* buffer )
{
/* This routine sets up an i/o parameter block and calls PBWrite    */
/* to write the specified amount of data to the specified position  */
/* in the file. If filePos is kSequential, then the data is written */
/* at the file's current mark position. After the write operation,  */
/* PBWrite leaves the file's mark at the end of the data written.   */

	/* Prepare the parameter block for the write function call */
	
	PB.ioParam.ioCompletion = NULL;		/* no completion routine */
	PB.ioParam.ioRefNum = fileRef;		/* file reference number */
	PB.ioParam.ioBuffer = (Ptr)buffer;	/* data storage */
	PB.ioParam.ioReqCount = bytesRequested;
										/* bytes we reqested written */
	if ( filePos == kSequential )
	{
		PB.ioParam.ioPosMode = fsAtMark;	/* write at current mark */
		PB.ioParam.ioPosOffset = 0;			/* not used in this case */
	}
	else
	{
		PB.ioParam.ioPosMode = fsFromStart;	/* write at offset from bof */
		PB.ioParam.ioPosOffset = filePos;	/* where in file to write */
	}
	
	/* Now call PBWrite to write the data out to the file */
	
	PBWrite( &PB, false );			/* false is to force */
									/* synchronous operation */
	
	gFileError = PB.ioParam.ioResult;	/* store error code in global */
	
	if ( PB.ioParam.ioResult == noErr )
		return true;
	else
		return false;
}


long	RemainingSpace( short vRefNum )
{
/* Returns the amount of space remaining on the volume specified */

	HParamBlockRec	pb;
	
	pb.volumeParam.ioCompletion = NULL;	/* no completion routine */
	pb.volumeParam.ioNamePtr = NULL;	/* not using this now */
	pb.volumeParam.ioVRefNum = vRefNum;	/* volume reference # */
	pb.volumeParam.ioVolIndex = 0;		/* use ioVRefNum */
	
	PBHGetVInfo( &pb, false );	/* false forces synch operation */
	
	gFileError = pb.volumeParam.ioResult;
	
	if ( gFileError == noErr )
		return (pb.volumeParam.ioVAlBlkSiz *
								pb.volumeParam.ioVFrBlk);
	else
		return -1L;
}


long	RecordsInFile( short refNum, short recordSize )
{
	long	logicalEOF;
	
	PB.ioParam.ioCompletion = NULL;		/* no completion routine */
	PB.ioParam.ioRefNum = refNum;		/* file reference # */
	
	PBGetEOF( &PB, false );		/* false forces synchronous call */
	
	gFileError = PB.ioParam.ioResult;
	
	if ( gFileError == noErr )
	{
		logicalEOF = (long)PB.ioParam.ioMisc;
		return (logicalEOF / recordSize);
	}
	else
		return -1L;
}


long	GetCurrentRecord( short refNum, short recordSize )
{
	PB.ioParam.ioCompletion = NULL;		/* no completion routine */
	PB.ioParam.ioRefNum = refNum;		/* file reference # */
	
	PBGetFPos( &PB, false );	/* false forces synchronous call */
	
	gFileError = PB.ioParam.ioResult;
	
	if ( gFileError == noErr )
		return (PB.ioParam.ioPosOffset / recordSize);
	else
		return -1L;
}


Boolean	SetCurrentRecord( short refNum, long recordNo,
											short recordSize )
{
	/* NOTE - If you specify a record beyond the end of the file, */
	/* this routine will return false and the file mark will be   */
	/* set to the logical end of file. If you specify the record  */
	/* that is just past the end of file, no error is returned.   */
	/* At that point, writing a record sequentially would extend  */
	/* the file, but attempting to read a record would return an  */
	/* error. */
	
	PB.ioParam.ioCompletion = NULL;		/* no completion routine */
	PB.ioParam.ioRefNum = refNum;		/* file reference # */
	PB.ioParam.ioPosMode = fsFromStart;	/* offset from beg of file */
	PB.ioParam.ioPosOffset =
		( recordSize * recordNo );		/* where to put the file mark */
	
	PBSetFPos( &PB, false );	/* false forces synchronous call */
	
	gFileError = PB.ioParam.ioResult;
	
	if ( gFileError == noErr )
		return true;
	else
		return false;
}


short	LastFileError( void )
{
	return gFileError;
}

