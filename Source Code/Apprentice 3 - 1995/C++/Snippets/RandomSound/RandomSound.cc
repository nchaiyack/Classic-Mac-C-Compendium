/*
 ***********************************************************************
 *
 * This is a whimsy set of functions that lets one play the sound
 * picked at random from the sound folder.
 * The sound folder name is supposed to be specified as a "Sound Folder"
 * STR resource. Even if you bastard forgot to set this resource,
 * the folder "Sounds" in the system folder would be assumed.
 *
 ***********************************************************************
 */

/* MacHeaders Included */

#include <Folders.h>
#include <Sound.h>

/*
 *-----------------------------------------------------------------------------------
 * 								Some standard I/O classes
 */
class ParamBlockHdr
{
	char internal[12];					// Managed by the File/Queue Managers exclusively
	int (*completion_routine)(void * param_block);	// Completion routine
	OSErr result;						// Result code, =0 means OK, <0 means error
public:
	StringPtr name;						// Pointer to some name
	short VRefNum;						// Vol ref number (if <0), drive ref number (if >0)
										// or working dir reference number
										
	ParamBlockHdr(void);
	~ParamBlockHdr(void) {}
	OSErr io_error(void) const		{ return result; }
};

ParamBlockHdr::ParamBlockHdr(void)
{
	completion_routine = nil;
	result = 1;							// Work is in progress
}

class DIRInfo: public ParamBlockHdr
{
public:
	short ioFRefNum;					// File ref number
	signed char ioFVersNum;				// Version number
	signed char	filler1;				// Not used
	short ioFDirIndex;					// Index for the directory
	signed char ioFlAttrib;				// File Attribute
	signed char	filler2;				// Not used
	DInfo ioDrUsrWds;					// Finder Info
	long ioDrDirId;						// Directory ID
	short ioDrNmFls;					// No. files and directories inside this dir
	short filler3[9];					// Not used
	long ioDrCrDat;						// Creation date
	long ioDrMdDat;						// Modification date
	long ioDrBkDat;						// Backup date
	DXInfo ioDrFndrInfo;				// Additional info for the finder
	long ioDRParID;						// Dir ID of the parent dir
	
	DIRInfo(const short vol_refno, const long parent_dirid, StringPtr dir_name);
	DIRInfo(StringPtr dir_path_name);
	~DIRInfo(void) {}
	
};

									// Getting a directory info
									// Dir is specified by the volume refno, 
									// parent ID and its name
DIRInfo::DIRInfo(const short vol_refno, const long parent_dirid, StringPtr dir_name)
{
	VRefNum = vol_refno;
	name = dir_name;
	ioFDirIndex = 0;				// not used here
	ioDrDirId = parent_dirid;
	do_well( PBGetCatInfo((CInfoPBRec *)this,FALSE) );
	assert( io_error() == noErr );
}


									// Getting a directory info
									// Directory is now specified as a path name
DIRInfo::DIRInfo(StringPtr dir_path_name)
{
	FSSpec dir_record;
	do_well( FSMakeFSSpec(0,0,dir_path_name,&dir_record) );
	
	VRefNum = dir_record.vRefNum;
	name = dir_record.name;
	ioFDirIndex = 0;				// not used here
	ioDrDirId = dir_record.parID;
	do_well( PBGetCatInfo((CInfoPBRec *)this,FALSE) );
	assert( io_error() == noErr );
}


class FLInfo: public ParamBlockHdr
{
public:
	short ioFRefNum;					// File ref number
	signed char ioFVersNum;				// Version number
	signed char	filler1;				// Not used
	short ioFDirIndex;					// Index for the directory
	signed char ioFlAttrib;				// File Attribute
	signed char	ioFlVersNum;			// Version number
	FInfo ioFlFndrInfo;					// Finder Info
	long ioDirId;						// Directory ID
	short ioFlStBlk;					// First alloc block of data fork
	long ioFlLgLen;						// Logical end-of-file of data fork
	long ioFlPyLen;						// Physical end-of-file of data fork
	short ioFlRStBlk;					// First alloc block of resource fork
	long ioFlRLgLen;					// Logical end-of-file of resource fork
	long ioFlRPyLen;					// Physical end-of-file of resource fork
	long ioFlCrDat;						// Creation date
	long ioFlMdDat;						// Modification date
	
	Str63 file_name;
	FLInfo(const short vol_refno, const long parent_dirid, const int index);
	~FLInfo(void) {}
	
};

									// Getting a file info 
									// File is specified by the volume refno, 
									// dir ID and the index
									// Check io_error() for possible fnfErr
FLInfo::FLInfo(const short vol_refno, const long parent_dirid, const int index)
{
	VRefNum = vol_refno;
	name = file_name;
	ioFDirIndex = index;
	ioDirId = parent_dirid;

	OSErr err = PBHGetFInfo((HParamBlockRec *)this,FALSE);
	if( err != noErr && err != fnfErr )
	  _error("Failed to get a file info for a file indexed %d; error code %d",index,
	  		 err);
}

/*
 *-----------------------------------------------------------------------------------
 * 				Sound Directory class that contains all the info about
 *						the directory with sound files
 */

class SoundDirectory
{
	short volume_refno;
	long  dir_id;				// Directory ID of the Sound directory
	long  no_files;				// No. of files and folders in the directory
	void locate_sound_directory(void);
	void locate_system_sound_directory(void);	// i.e. folder "Sounds" in the System Folder

								// Locate the file with given index and tries
								// to open it as a resource. Returns the resource
								// refnumber if succeeded, or -1
	short locate_file_open_resource(const int file_no);
	
											// Pick a 'snd ' resource at random
											// (if there are more than one)
	Handle get_random_sound_rsc(void);

public:
	SoundDirectory(void)	{ volume_refno = 0; dir_id = -1; }
	~SoundDirectory(void) {}
	void play_at_random(void);
};

/*
 *-----------------------------------------------------------------------------------
 * 				Selecting a folder that contains sound files
 */


										// Try to read the resource "Sound Folder" in the
										// STR resource group, and locate the named
										// folder. If no such resource is specified,
										// locate_system_sound_directory() is called
void SoundDirectory::locate_sound_directory(void)
{
	Handle sound_dir_strp = GetNamedResource('STR ',"\pSound Folder");
	if( sound_dir_strp == nil && ResError() == resNotFound )
	{
	  locate_system_sound_directory();		// No resource was specified, use that
	  return;								// in the system folder
	}
	
	do_well( ResError() );
	assert( sound_dir_strp != nil );
	
									// Then get the directory ID of the "Sounds"
									// folder within it
	HLock(sound_dir_strp);
	DIRInfo sound_dir((StringPtr)*sound_dir_strp);
	HUnlock(sound_dir_strp);
	ReleaseResource(sound_dir_strp);
	
	assure( sound_dir.ioFlAttrib & 0x10, "The named thing is not a directory!");
	volume_refno = sound_dir.VRefNum;
	dir_id = sound_dir.ioDrDirId;
	no_files = sound_dir.ioDrNmFls;
//	message("Dir Id of the sound dir is %d, no. files %d",dir_id,no_files);
}


										// The present version locates the
										// directory named Sounds inside the
										// system folder
void SoundDirectory::locate_system_sound_directory(void)
{
									// First locate the system directory
	long system_dir_id;
	do_well( ::FindFolder(kOnSystemDisk,kSystemFolderType,kDontCreateFolder,
						 &volume_refno,&system_dir_id) );
	
									// Then get the directory ID of the "Sounds"
									// folder within it
	DIRInfo sound_dir(volume_refno, system_dir_id, "\pSounds");
	assure( sound_dir.ioFlAttrib & 0x10, "The named thing is not a directory!");
	dir_id = sound_dir.ioDrDirId;
	no_files = sound_dir.ioDrNmFls;
//	message("Dir Id of the sound dir is %d, no. files %d",dir_id,no_files);
}


/*
 *-----------------------------------------------------------------------------------
 * 					Picking a file at random and playing it
 */

									// Locating a file in the sound dir with a
									// specified index. The program then tries
									// to open it as a resource. Returns the resource
									// refnumber if succeeded, or -1
short SoundDirectory::locate_file_open_resource(const int file_index)
{
	assert( file_index > 0 && file_index <= no_files );
	FLInfo file_info(volume_refno,dir_id,file_index);
	if( file_info.io_error() != noErr )
	  return -1;
//	message("File name is %#s",file_info.file_name);
	short res_refno = HOpenResFile(volume_refno,dir_id,file_info.file_name,fsRdPerm);
	if( res_refno == -1 )
	  message("Failed to open the resource fork because of error %d",ResError());
	return res_refno;
}


											// Pick a 'snd ' resource at random
											// (if there are more than one)
Handle SoundDirectory::get_random_sound_rsc(void)
{
	const ResType snd_restype = 'snd ';
	const int no_snd_resources = Count1Resources(snd_restype);
    if( ResError() != noErr || no_snd_resources < 1 )
	{
	  message("Failed to count sound resources because of error %d",ResError());
	  return nil;
	}
	
	// message("No of resources %d",no_snd_resources);
	int index;
	if( no_snd_resources == 1 )
	  index = 1;
	else
	{											// Pick up an index at random
		index = Random() % no_snd_resources;	// within [1,no_snd_resources]
		index = ( index < 0 ? index + no_snd_resources : index ) + 1;
	}
													// Pick the sound resource
 	Handle sound_handle = Get1IndResource(snd_restype,index);
	if( ResError() != noErr )
	  message("Failed to load a sound resource because of error %d",ResError());
	return sound_handle;
}

											// Plays and closes the resource file
void SoundDirectory::play_at_random(void)
{
	if( dir_id < 0 )
	  locate_sound_directory();
	GetDateTime((unsigned long *)&randSeed);
	Random(); Random(); Random(); Random();					// Just randomizing
	
	const int max_no_tries = 5;
	register int try;
	for(try=1; try <= max_no_tries; try++)
	{										// Pick up an index at random [1,no_files]
	  int file_index;
	  if( no_files == 1 )
	    file_index = 1;
	  else
	  {
	    file_index = Random() % no_files;
	    file_index = (file_index < 0 ? file_index + no_files : file_index ) + 1;
	  }
	  short res_refno = locate_file_open_resource(file_index);
	  if( res_refno == -1 )
	    continue;
	    
	  Handle sound_handle = get_random_sound_rsc();
	  if( sound_handle != nil )
  	  {
  	    LoadResource(sound_handle);	  
	    do_well( SndPlay(nil,sound_handle,FALSE) );		// Play sound in sync mode
	    DisposeHandle(sound_handle);
        CloseResFile(res_refno);
        break;
	  }
	  else
	    CloseResFile(res_refno);						// and try another sound
     }
}

/*
 *-----------------------------------------------------------------------------------
 * 								Routing module
 */
 
static SoundDirectory sound_bundle;

void play_random_sound(void)
{
	sound_bundle.play_at_random();
}
