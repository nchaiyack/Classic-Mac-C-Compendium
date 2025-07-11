/* Scanner.c
 *
 * This file contains two routines which, used together, will scan
 * through all files/folders on all mounted volumes on your Mac and
 * take some sort of action for each file/folder found. If you
 * only want to scan one volume, call ScanFolders with a volume
 * reference number and a directory ID of zero.
 *
 * If you wanted to search a volume for a small number of files
 * based on their characteristics, this is the WRONG way to do it.
 * Use PBCatSearch for that purpose.
 *
 * If, on the other hand, you need to find ALL of the files and/or
 * folders on a volume, or need to examine each (as in a virus
 * scanning program), these routines can handle that just fine.
 * Also, if you definitely want to scan files in alphabetical
 * order (as they appear in the finder), that's the way these
 * routines scan.
 *
 * All Macintosh files are indexed (have an index number) within
 * their directory/folder. If there are five files in a folder,
 * they will always have index numbers one through five. Folders
 * have indexes just as files do. If the third file in the folder
 * was a subfolder rather than a file, it would still have index
 * number three. The routines in this file work by starting at
 * the root folder of a volume and simply counting the index up
 * from one getting file information based on each index number.
 * Whenever a subfolder is found, the routine recurses and
 * counts through the files in that subfolder.
 *
 * NOTE that the routines here do NOT take into account the fact
 * that the indexes of files can change within a directory between
 * calls to PBGetCatInfo if another process adds or deletes a file
 * during that time. There are ways to get around that which
 * usually involve rescanning the same directory, but the way
 * you handle it depends on your purpose for scanning. For
 * example, it wouldn't matter a whole lot if you scanned a few
 * files twice with a virus scanner.
 *
 * By the way, you can tell if indexes have shifted within a
 * directory by calling PBGetCatInfo a second time with the same
 * index number just before moving to the next index number.
 * Compare the file name returned with the file name you got
 * last time you called PBGetCatInfo with that index to make
 * sure they're the same. If they are, then immediately call
 * PBGetCatInfo for the next index number. If they're not,
 * that's when you need to rescan or whatever.
 */
 
#include "Scanner.h"

#include "ActionRoutines.h"		/* This file contains the routines   */
								/* FolderAction and FileAction       */
								/* which are simply example routines */
								/* that write to a file opened in    */
								/* the main routine. */

/* ScanVolumes loops and finds all mounted volumes (up to 256) and
 * calls ScanFolders to scan all files and folders on each volume
 * found.
 */
void	ScanVolumes( void )
{
	HParamBlockRec	pb;		/* an HFS parameter block */
	short	x;
	Str31	volumeName;
	
	pb.volumeParam.ioCompletion = NULL;		/* no completion routine */
	pb.volumeParam.ioNamePtr = volumeName;	/* assign storage for name */
	pb.volumeParam.ioVRefNum = 0;
	
	for ( x=1; x<=256; x++ )		/* loop through volume indexes */
	{
		pb.volumeParam.ioVolIndex = x;
		PBHGetVInfo( &pb, false );
		if ( pb.volumeParam.ioResult != noErr )
		{
			/* Error means no more mounted volumes */
			
			break;	/* break out of the for loop */
		}
		else
		{
			/* Found a valid volume. Call FolderAction with the */
			/* volume name to record it in the file, then call  */
			/* ScanFolders to scan all files and folders on the */
			/* volume. */
			
			FolderAction( volumeName, 0 );
			
			ScanFolders( pb.volumeParam.ioVRefNum, 0, 1 );
		}
	}
}

/* ScanFolders scans through all the files and/or folders in the
 * folder specified. If dirID is zero, it scans the root
 * folder of the volume specified. When scanning the folder,
 * if a sub-folder is found, ScanFolders recurses (calls itself)
 * to scan that folder. This behavior causes ALL the files and
 * folders on the volume to be scanned. When ScanFolders is
 * called to scan the root folder of a volume, the value 1
 * should be sent for levels.
 */
void	ScanFolders( short vRefNum, long dirID, short levels )
{
	CInfoPBRec		pb;			/* a Catalog Info parameter block */
	short			index;
	Str31			dirName;
	
	pb.hFileInfo.ioCompletion = NULL;	/* no completion routine */
	pb.hFileInfo.ioNamePtr = dirName;	/* storage for filename */
	pb.hFileInfo.ioVRefNum = vRefNum;	/* the volume reference # */
	
	index = 1;		/* first file in the directory */
	
	while( true )
	{
		pb.hFileInfo.ioFDirIndex = index;	/* file index w/in the dir */
		pb.hFileInfo.ioDirID = dirID;		/* directory ID # */
		PBGetCatInfo( &pb, false );		/* get catalog info for file */
		
		/* If there was an error, quit scanning this folder by */
		/* breaking out of the while loop. */
		
			if ( pb.hFileInfo.ioResult != noErr )
				break;
		
		/* If the file is actually a folder, recurse to scan it */
		/* Otherwise, just call FileAction */
		
			if ( pb.hFileInfo.ioFlAttrib & 0x10 )	/* subfolder? */
			{
				FolderAction( dirName, levels );
				
				ScanFolders( vRefNum, pb.dirInfo.ioDrDirID, levels+1 );
			}
			else
				FileAction();
		
		/* Increase the index by one to move to the next file */
		
			index++;
	}
}

