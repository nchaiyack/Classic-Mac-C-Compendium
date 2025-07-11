/*
 * Athough getenv() is nominally implemented in Think-C,
 * it simply returns NULL.
 * This proposed Think-C replacement for getenv()
 * searches a specified file in the Preference Folder --
 * as found by the in-built function FindFolder() --
 * and reads through it for the definition of an "environment variable".
 *
 * More precisely, the replacement function getpref(char *env_name)
 * looks in the Preference folder
 * for the file with the externally-defined PreferenceName.
 * If there is a line in this file of the form
 * ENVNAME = "env_value";
 * then getpref("ENVNAME") returns "env_value".
 *
 * The intention is that getenv() be over-ridden by getpref()
 * through a judiciously placed
 *
 * #define getenv(env_name) getpref(env_name)
 *
 * This source is largely copied from a file Preference.c,
 * the name of whose author I have unfortunately mislaid.
 * 
 * Timothy Murphy <tim@maths.tcd.ie>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unix.h>
#include <ctype.h>
#include <Traps.h>
#include <GestaltEqu.h>
#include <Folders.h>
extern int errno;

extern char *PreferenceName;
char PreferencePath[FILENAME_MAX] = "";

#define ENVIRONMENT_SIZE 4096
#define MAX_ENV 32

static char *Environment;
static char *Env_name[MAX_ENV];

/* Defines for GestaltAvailable */
#define GESTALT_TRAP				0xA1AD

/* Defines for FindPrefFolder */
#define BTstQ(arg, bitnbr)			(arg & (1 << bitnbr))

/* Prototypes */
char*		getpref(char*);
FILE*		OpenPreferenceFile(char*);
int			GetPathName(char*, short, long);
int			GestaltAvailable(void);
OSErr		FindPrefFolder(short *, long *);
Boolean		TrapAvailable(int);
TrapType	GetTrapType (int);
int			NumToolboxTraps(void);

char *getpref(char *env_name)
{
	short VRefNum;
	long DirID;
	FILE *fp;
	char line[256];
	char *Environment, *Env_end, *cp;
	int i = 0;
	static int read_already = 0;
	
	if (read_already++ == 0) {
		if ((Environment = malloc(ENVIRONMENT_SIZE)) == NULL)
			return NULL;
		Env_end = Environment + ENVIRONMENT_SIZE;
		if (FindPrefFolder(&VRefNum, &DirID) == noErr)
			GetPathName(PreferencePath, VRefNum, DirID);
		else
			return NULL;
		strcat(PreferencePath, ":");
		strcat(PreferencePath, PreferenceName);
		fp = fopen(PreferencePath, "r");
		if (fp == NULL) {
			fprintf(stderr, "\nWarning! \"%s\" not found\n", PreferencePath);
			return NULL;
		}
		
		while (fgets(line, 255, fp)) {
			if (!isalnum(*line) || (cp = strchr(line, '=')) == NULL)
				continue;	/* line not of required form "ABC... = " */
			(void)strtok(line, "= \t");
			while (*++cp && isspace(*cp));
			if (*cp == '"')
				(void)strtok(++cp, "\"");
			else if (*cp)
				(void)strtok(cp, " \t\n");
			if (Environment + strlen(line) + strlen(cp) + 4 >= Env_end) {
				fprintf(stderr, "Preference file too long!\n");
				break;
			}
			Env_name[i++] = Environment;
			strcpy(Environment, line);
			Environment += strlen(line) + 1;
			strcpy(Environment, cp);
			Environment += strlen(cp) + 1;
		}
		fclose(fp);
	}

	for(i = 0; Env_name[i]; i++)
		if (strcmp(env_name, Env_name[i]) == 0)
			return (Env_name[i] + strlen(env_name) + 1);
	return NULL;
}

/*
FindPrefFolder returns the (real) vRefNum, and the DirID of
the current preference folder on System 7 and beyond. Returns
info about the system folder in previous releases. It uses the
Folder Manager if present, otherwise it falls back to
SysEnvirons. It returns zero on success, otherwise a standard
system error.
*/

OSErr    FindPrefFolder(foundVRefNum, foundDirID)
short			*foundVRefNum;
long			*foundDirID;
{

long			gesResponse;
SysEnvRec		envRec;
WDPBRec			myWDPB;
unsigned char	volName[34];
OSErr			err;
int				done;
    
    
*foundVRefNum = 0;
*foundDirID = 0;

done=FALSE;
if(GestaltAvailable())
	/* Does Folder Manager exist? */
    if (!Gestalt (gestaltFindFolderAttr, &gesResponse) &&
        BTstQ (gesResponse, gestaltFindFolderPresent))
		{
		err = FindFolder (kOnSystemDisk, kPreferencesFolderType,kDontCreateFolder,
			foundVRefNum, foundDirID);
		done=TRUE;
		}

if(!done)	/* Gestalt can't give us the answer, so we resort to SysEnvirons */
	{
    if (!(err = SysEnvirons (curSysEnvVers, &envRec)))
		{
		myWDPB.ioVRefNum = envRec.sysVRefNum;
		volName[0] = '\000';                    /* Zero volume name */
		myWDPB.ioNamePtr = volName;
		myWDPB.ioWDIndex = 0;
		myWDPB.ioWDProcID = 0;
		if (!(err = PBGetWDInfo (&myWDPB, 0)))
			{
			*foundVRefNum = myWDPB.ioWDVRefNum;
			*foundDirID = myWDPB.ioWDDirID;
            }
        }
    }
	
return (err);
}


int GestaltAvailable()
{
/* Gestalt is available from System Software V.6.0.4 */
return(TrapAvailable(GESTALT_TRAP));
}


Boolean TrapAvailable (theTrap)
int	theTrap;
{
TrapType	tType;

tType = GetTrapType(theTrap);
if(tType == ToolTrap)
	{
	theTrap = theTrap & 0x07FF;
    if(theTrap >= NumToolboxTraps())
         theTrap = _Unimplemented;
	}
return(NGetTrapAddress(theTrap, tType) != NGetTrapAddress(_Unimplemented, ToolTrap));
}

int NumToolboxTraps()
{
if(NGetTrapAddress(_InitGraf, ToolTrap) == NGetTrapAddress(0xAA6E, ToolTrap))
      return(0x200);
   else
      return(0x400);
}

TrapType GetTrapType (theTrap)
int theTrap;
{
if((theTrap&0x0800) > 0)
      return(ToolTrap);
   else
      return(OSTrap);
}

int GetPathName(char *pathname, short vRefNum, long DirID)
{
	CInfoPBRec cipbr;
	HFileInfo *fpb = (HFileInfo*)&cipbr;
	DirInfo *dpb = (DirInfo*)&cipbr;
	char dirname[FILENAME_MAX];
	OSErr err;
	int depth = 0;
	int dirlen, pathlen = 0;
	
	*pathname = 0;
	for (dpb->ioDrDirID = DirID; dpb->ioDrDirID >= 2; dpb->ioDrDirID = dpb->ioDrParID) {
		dirname[0] = 0;
		dpb->ioNamePtr = (unsigned char*)dirname;
		dpb->ioVRefNum = vRefNum;
		dpb->ioFDirIndex = -1;
		if (PBGetCatInfo(&cipbr, FALSE)) {
			errno = ENODEV;
			return -1;
		}
		p2cstr(dirname);
		if (depth++)
			strcat(dirname, ":");
		dirlen = strlen(dirname);
		if (dirlen + pathlen + 1 >= FILENAME_MAX) {
			errno = ERANGE;
			return -1;
		}
		memmove(pathname + dirlen, pathname, pathlen + 1);
		memcpy(pathname, dirname, dirlen);
		pathlen += dirlen;
	}
}

