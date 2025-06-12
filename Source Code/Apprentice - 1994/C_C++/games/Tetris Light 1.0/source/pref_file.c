/**********************************************************************\

File:		pref_file.c

Purpose:	Contains routines to open/create preference files in the
			standard loction for preference files.
			

``Tetris Light'' - a simple implementation of a Tetris game.
Copyright (C) 1993 Hoylen Sue

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the
Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

\**********************************************************************/

#include "local.h"

#include <Folders.h>
#include <GestaltEqu.h>

#include "env.h"
#include "pref_file.h"

/*--------------------------------------------------------------------*/

static OSErr pref_open_FindFolder(INTEGER *ref, unsigned char *name,
								  OSType creator, OSType type, Boolean create)
/* Implementation of `pref_open' using the ` FindFolder' call to find the
   location for the preference file. */
{
	register OSErr erc;
	INTEGER pref_vref;
	LONGINT pref_dir_id;
	
	/* Find the preferences directory */
	
	erc = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
			   		 &pref_vref, &pref_dir_id);
	if (erc != noErr)
		return erc;
	
	/* Try to open an already existing preferences file */
	
	*ref = HOpenResFile(pref_vref, pref_dir_id, name, fsRdWrPerm);
	erc = ResError();
	if (erc == noErr)
		return noErr; /* Success */
	
	/* Failed to open */
	
	if ((erc == fnfErr) && create) {	
		/* Try to create it if it did not exist */
		
		/* Create data fork, so we get its signature is correct. It may
		   already exist (and only the resource fork is missign), so we
		   ignore any errors from this call. */
		
		(void) HCreate(pref_vref, pref_dir_id, name, creator, type);
		
		/* Create the resource fork */
		
		HCreateResFile(pref_vref, pref_dir_id, name);
		erc = ResError();
		if (erc != noErr)
			return erc;
		
		*ref = HOpenResFile(pref_vref, pref_dir_id, name, fsRdWrPerm);
		erc = ResError();
	}
	
	return erc;
}

/*--------------------------------------------------------------------*/

static OSErr pref_open_SysEnvirons(INTEGER *ref, unsigned char *name,
								   OSType creator, OSType type, Boolean create)
/* Implementation of `pref_open' using the ` SysEnvirons' call to find the
   location for the System Folder, and uses that as the location of the
   preference file. */
{
	register OSErr erc;
	SysEnvRec env_rec;
	Str255 volume_name;
	INTEGER volume_ref;

	/* Save the current volume/working directory */
		
	erc = GetVol(volume_name, &volume_ref);
	if (erc != noErr)
		return erc;

	/* Get System Folder's reference number */
	
	erc = SysEnvirons(1, &env_rec);	/* Hope there's compiler glue for 
									   machines without this call ??? */
	switch (erc) {
	case noErr:
		break;
	case envNotPresent:
		/* SysEnvirons trap not present */
		env_rec.sysVRefNum = volume_ref; /* Put it on current volume */
		break;
	default:
		return erc;
		break;
	}
	
	/* Change volumes/working directory to where the preference file goes */
	
	erc = SetVol(NIL, env_rec.sysVRefNum);
	if (erc != noErr)
		return erc;
	
	/* Try to open it */
	
	*ref = OpenResFile(name);
	erc = ResError();
	if (erc == noErr) {
		(void) SetVol(volume_name, volume_ref);
		return noErr; /* Success */
	}
	
	if ((erc == fnfErr) && create) {
		/* Try to create it */
		
		/* Create data fork, so we get its signature is correct. It may
		   already exist (and only the resource fork is missign), so we
		   ignore any errors from this call. */
		
		(void) Create(name, env_rec.sysVRefNum, creator, type);
		
		/* Create the resource fork */
		
		CreateResFile(name);
		erc = ResError();
		if (erc != noErr) {
			(void) SetVol(volume_name, volume_ref);
			return erc;
		}
		
		*ref = OpenResFile(name);
		erc = ResError();
	}
	
	(void) SetVol(volume_name, volume_ref);
	return erc;
}

/*--------------------------------------------------------------------*/

OSErr pref_open(INTEGER *ref, INTEGER pref_name_str_resid,
				OSType creator, OSType type, Boolean create)
/* Tries to open the perference file with the name given in the resource
   string with ID `pref_name_str_resid'.  If it cannot be found, and
   `create' is TRUE, it tries to create it.  The preference
   file is created with the given `creator' signature.  Thpreference
   file is opened as a resource file.  The opened reference is returned
   in `ref'.  If an error occured, an error code is returned and `ref'
   is undefined.  The name of the resource file comes from a resource
   string to simplify localization. */
{
	register OSErr erc;
	StringHandle name_handle;
	
	name_handle = GetString(pref_name_str_resid);
	if (!name_handle)
		return resNotFound;

	HLock(name_handle);
	
	if (env_FindFolder_available()) 
		erc = pref_open_FindFolder(ref, *name_handle, creator, type, create);
	 else 
		erc = pref_open_SysEnvirons(ref, *name_handle, creator, type, create);
	
	HUnlock(name_handle);
	return erc;
}

/*--------------------------------------------------------------------*/
