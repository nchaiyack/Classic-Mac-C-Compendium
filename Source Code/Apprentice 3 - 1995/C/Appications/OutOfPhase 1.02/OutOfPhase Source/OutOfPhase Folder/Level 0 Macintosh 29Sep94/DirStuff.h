/* DirStuff.h */

#ifndef Included_DirStuff_h
#define Included_DirStuff_h

/* DirStuff module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Files */

/* Implementation Note:  This library depends on FileSpec == FSSpec.  If Files */
/* decides to change this, then it must be changed here as well. */
struct FileSpec;

/* directory data structure abstraction */
struct DirectoryRec;
typedef struct DirectoryRec DirectoryRec;

/* types of items that can be in a directory */
typedef enum {eFile EXECUTE(= -2415), eSymbolicLink, eDirectory, eOther} DirFileTypes;

/* read the first level list of items in the specified directory.  NIL specifies */
/* a root directory.  If NIL is returned, then the operation could not be completed */
DirectoryRec*			ReadDirectory(struct FileSpec* Directory);

/* get rid of the directory structure when we are done with it */
void							DisposeDirectory(DirectoryRec* Dir);

/* find out how many entries there are in the directory structure */
long							GetDirectorySize(DirectoryRec* Dir);

/* return the item type of an indexed directory entry (indices start from 0) */
/* Indices start from 0 up to GetDirectorySize() - 1 */
DirFileTypes			GetDirectoryEntryType(DirectoryRec* Dir, long Index);

/* return a pointer containing the name of the specified directory entry or NIL */
/* if allocation failed.  Name is not null terminated. */
char*							GetDirectoryEntryName(DirectoryRec* Dir, long Index);

/* get a file spec describing a directory entry */
/* this entry is a standard FileSpec, the same type as used in the Files module */
/* and should be manipulated and disposed using routines from Files */
struct FileSpec*	GetDirectoryEntryFileSpec(DirectoryRec* Dir, long Index);

/* resort the directory alphabetically.  Returns True if it succeeded or */
/* False if it failed. */
MyBoolean					ResortDirectory(DirectoryRec* Dir);

/* this compares to file specifications and returns True if they refer to the */
/* same file */
MyBoolean					CompareFileSpecs(struct FileSpec* First, struct FileSpec* Second);

/* dereference a symbolic link one level only */
struct FileSpec*	DereferenceSymbolicLink(struct FileSpec* Source);

/* get root file specification.  On UNIX, this would return "/"; on Macintosh, */
/* it returns a bogus file descriptor */
struct FileSpec*	GetRootFileSpec(void);

/* time record */
typedef struct
	{
		unsigned long					Year : 12;  /* 0..4095 */
		unsigned long					Month : 4;  /* 0..11 */
		unsigned long					Day : 5;  /* 0..30 */
		unsigned long					Hour : 5;  /* 0..23 */
		unsigned long					Minute : 6;  /* 0..59 */
		unsigned long					Second : 6;  /* 0..59 */
		unsigned long					DayOfTheWeek : 3;  /* 0..6 */
	} TimeRec;

/* statistics record for a file */
typedef struct
	{
		unsigned long					CreatorCode;
		unsigned long					FileTypeCode;
		TimeRec								CreationDate;
		TimeRec								LastModificationDate;
	} FileInfoRec;

/* get statistics for a file */
MyBoolean					GetFileStatistics(struct FileSpec* File, FileInfoRec* InfoOut);

/* create a new directory with the specified file specification. */
MyBoolean					CreateNewDirectory(struct FileSpec* DirLocation);

/* find out of the specified file specification is a directory */
MyBoolean					IsTheFileSpecADirectory(struct FileSpec* Spec);

/* find out if the specified file is a symbolic link */
MyBoolean					IsTheFileSpecASymbolicLink(struct FileSpec* Spec);

/* obtain a file spec for a file inside of the specified directory.  the file */
/* name must be a non-null-terminated heap block */
struct FileSpec*	FileSpecForFileInDirectory(struct FileSpec* DirLocation,
										char* Filename);

#endif
