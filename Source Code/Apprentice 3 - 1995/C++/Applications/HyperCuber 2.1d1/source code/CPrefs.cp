//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPrefs.cp
//|
//| This implements a Preferences class.
//|_________________________________________________________

#include "CPrefs.h"

#include <TBUtilities.h>
#include <Folders.h>
#include <Packages.h>


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPrefs::IPrefs
//|
//| Purpose: Initialize the preferences and set them to their default
//|          values.  Subclasses should override this to set the defaults.
//|
//| Parameters: the_filename: filename of the preferences file
//|             prefs_size:   size of the Preferences structure
//|             version:      the current preferences version
//|_________________________________________________________________________

void CPrefs::IPrefs(Str255 the_filename, short prefs_size, short version)
{

	CopyPString(the_filename, filename);		//  Save the filename
	size = prefs_size;							//  Save the size of the prefs struct
	current_version = version;					//  Save the current prefernces version

}	//==== CPrefs::IPrefs() ====\\
	


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPrefs::GetPrefsPointer
//|
//| Purpose: This returns a pointer to the preferences structure.  This is
//|          used internally, so the Load and Save methods know where to
//|          find the preferences.
//|
//| Parameters: returns pointer to prefs
//|_________________________________________________________________________

short *CPrefs::GetPrefsPointer(void)
{

	return NULL;		//  This should be overridden by subclasses

}	//==== CPrefs::GetPrefsPointer() ====\\
	


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CPrefs::Load
//
// Purpose: Load the preferences from the preferences file.
//
// Parameters: none
//_________________________________________________________

void CPrefs::Load(void)
{

	short		error;
	FSSpec		prefs_file_spec;
	short		prefs_file_refnum;
	short		*temp_prefs;
	
	temp_prefs = (short *) NewPtr(size);						//  Allocate temp prefs space

	error = FindFolder(kOnSystemDisk, kPreferencesFolderType,	//  Find the preferences file FSSpec
						kCreateFolder,
						&(prefs_file_spec.vRefNum),
						&(prefs_file_spec.parID));
	CopyPString(filename, prefs_file_spec.name);

	error = FSpOpenDF(&prefs_file_spec, fsRdPerm,				//  Open the preferences file
						&prefs_file_refnum);
	
	/* If the file does not exist, create it */
	if (error == fnfErr)
		{
		
		Save();													//  Write the default preferences
																//  to the preferences file

		error = FSpOpenDF(&prefs_file_spec, fsRdPerm,			//  Open the preferences file
						&prefs_file_refnum);
		
		}

	SetFPos(prefs_file_refnum, 1, 0);							//  Read the prefs from the file
	long num_bytes = size;
	FSRead(prefs_file_refnum, &num_bytes, temp_prefs);
	
	if (current_version != *temp_prefs)							//  Check the prefs version
		{

			//  This an older preferences version-- delete the file and build
			//  a new one from defaults.
		
		error = FSClose (prefs_file_refnum);					//  Get rid of the old prefs file
		error = FSpDelete(&prefs_file_spec);
		
			//  Call LoadPrefs recursively-- this will create a new default prefs
			//  file, since the old one has been deleted.
		
		Load();
		return;
		
		}
	
	BlockMove(temp_prefs, GetPrefsPointer(), size);				//  Use the prefs from the file
	
	DisposPtr((char *) temp_prefs);								//  Free temp prefs memory
	
}	//==== CPrefs::Load() ====\\



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CPrefs::Save
//
// Purpose: Write the current preferences to the preferences file.
//
// Parameters: none
//_________________________________________________________________

void CPrefs::Save(void)
{

	FSSpec	prefs_file_spec;
	short	prefs_file_refnum;
	short	error;

	error = FindFolder(kOnSystemDisk,						//  Find the preferences file FSSpec
						kPreferencesFolderType,	
						kCreateFolder,
						&(prefs_file_spec.vRefNum),
						&(prefs_file_spec.parID));
	CopyPString(filename, prefs_file_spec.name);

	error = FSpDelete(&prefs_file_spec);					//  Create a new prefs file
	error = FSpCreate(&prefs_file_spec, 'HCub',
						'Pref', iuSystemScript);
	error = FSpOpenDF(&prefs_file_spec, fsWrPerm,			//  Open new file for writing
						&prefs_file_refnum);

	long num_bytes = size;									//  Write the current prefs to file
	error = FSWrite(prefs_file_refnum, &num_bytes,
						GetPrefsPointer());
	
	error = FSClose(prefs_file_refnum);						//  Close the prefs file
	
}	//==== CPrefs::Save() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPrefs::Update
//|
//| Purpose: Update the preferences to match the settings being used.
//|
//| Parameters: none
//|_____________________________________________________________________________

void CPrefs::Update(void)
{

		//  This should be overridden by subclasses

}	//==== CPrefs::Update() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPrefs::SetDefaults
//|
//| Purpose: Set the preferences to their default values.
//|
//| Parameters: none
//|_____________________________________________________________________________

void CPrefs::SetDefaults(void)
{

		//  This should be overridden by subclasses

}	//==== CPrefs::SetDefaults() ====\\



