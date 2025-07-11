/* catinfo.c - implements catinfo.h (catalog info from path) */

/*
modification history
--------------------
01b,20may95,ejo  a few extra brain damages for CW.
01a,06nov94,ejo  written.
*/

#include <string.h>
#include <errno.h>

#include "catinfo.h"

#ifndef ENOENT
#define ENOENT		 2 /* No such file or directory */
#endif
#ifndef EFAULT
#define EFAULT		14 /* Bad address */
#endif
#ifndef ENODEV
#define ENODEV		19 /* No such device */
#endif
#ifndef ENOTDIR
#define ENOTDIR 	20 /* Not a directory */
#endif

/*******************************************************************************
*
* pstrncpy - strncpy that produce a pascal string from a C string.
*
* RETURNS: as strncpy.
*/
static unsigned char *pstrncpy
	(
	unsigned char *pstr,
	const char *cstr,
	size_t n
	)
	
	{
	return
		*pstr = n, (unsigned char *) strncpy ((char *) pstr + 1, cstr, n) - 1;
	}
	
/*******************************************************************************
*
* GetNextPathElem - get next element in a path of the form [vol:]dir:..:file[:].
*
* A trailing colon is ignored. Consequtive colons is ok and will be reported
* as elements of length zero. Use this to move upwards in the path.
*
* RETURNS: pointer to next element of LEN chars, or NULL if end of path.
*/
static const char *GetNextPathElem (const char **path, int *len)
	{
	const char *p = *path;
	
	if (p == NULL)
		return NULL;

	*len = (*path = strchr (p, ':')) != NULL ? (*path)++ - p : strlen (p);
	
	return *len > 0 || *path != NULL ? p : NULL;
	}

/*******************************************************************************
*
* GetCatInfo - PBGetCatInfo from full or partial path.
*
* DESCRIPTION: See header file.
*
* RETURNS: 0 on success, otherwise and errno code describing the problem.
*/
int GetCatInfo (const char *path, CInfoPBRec *info, long *dirID)
	{
	static unsigned char pas_name[MAX_PAS_LEN + 1];
	int elemlen;
	const char *element;
	long dirIDstore;
	
	/* verify arguments */
	if (path == NULL || info == NULL)
		return EFAULT;
		
	/* setup store for communicating pascal strings to the OS */
	info->dirInfo.ioNamePtr = pas_name;
	
	/* partial path? i.e. begin with colon or contain no colons */
	if (path[0] == ':' || strchr (path, ':') == NULL)
		{
		/* partial path; start path parse at default volume and directory */
		
		WDPBRec vinfo;
		
		vinfo.ioNamePtr = NULL;
		
		/* get default volume and directory */
		if (PBHGetVolSync (&vinfo) != noErr)
			return ENODEV;
			
		info->dirInfo.ioVRefNum = vinfo.ioWDVRefNum;
		info->dirInfo.ioDrDirID = vinfo.ioWDDirID;
		
		/* absorb a leading colon, we consider it part of the volume name */
		if (path[0] == ':')
			++path;
		}
	else if ((element = GetNextPathElem (&path, &elemlen)) != NULL)
		{
		/* full path; identify volume from first element */

		HParamBlockRec vinfo;
		
		/* valid volume names shall have no more than MAX_VOL_LEN characters */
		if (elemlen > MAX_VOL_LEN)
			return ENODEV;
		
		/* include colon in volume name for complete specification */
		vinfo.volumeParam.ioNamePtr = pstrncpy (pas_name, element, ++elemlen);
		vinfo.volumeParam.ioVolIndex = -1;
		if (PBHGetVInfoSync (&vinfo) != noErr)
			return ENODEV;
			
		/* start path parse at the root directory on the volume */
		info->dirInfo.ioVRefNum = vinfo.volumeParam.ioVRefNum;
		info->dirInfo.ioDrDirID = fsRtDirID;
		}
	else
		/* this should never happen, go home with a fault */
		return EFAULT;
		
	/* collect info about the starting directory  */
	info->dirInfo.ioFDirIndex = -1;
	if (PBGetCatInfoSync (info) != noErr)
		return ENOTDIR;

	/* setup storage for dirID if caller didn't */
	if (dirID == NULL)
		dirID = &dirIDstore;
		
	*dirID = info->dirInfo.ioDrParID;
		
	/* finally walk down the path */
	while ((element = GetNextPathElem (&path, &elemlen)) != NULL)
		{
		/* another offspring; verify that the last offspring was a directory */
		if ((info->dirInfo.ioFlAttrib & ioDirMask) == 0)
			return ENOTDIR;
		
		/* file and folder names shall have no more than MAX_DIR_LEN chars */
		if (elemlen > MAX_DIR_LEN)
			return ENOTDIR;	/* ENAMETOOLONG would be better */
			
		/* move up if element is empty (i.e we saw a ::) */
		if (elemlen == 0)
			{
			info->dirInfo.ioDrDirID = info->dirInfo.ioDrParID;
			info->dirInfo.ioFDirIndex = -1;
			}
		else
			{
			pstrncpy (pas_name, element, elemlen);
			info->dirInfo.ioFDirIndex = 0;
			}
			
		*dirID = info->dirInfo.ioDrDirID;
		
		if (PBGetCatInfoSync (info) != noErr)
			if (elemlen == 0 || GetNextPathElem (&path, &elemlen) != NULL)
				return ENOTDIR;
			else
				return ENOENT;
		}
		
	return 0;
	}
