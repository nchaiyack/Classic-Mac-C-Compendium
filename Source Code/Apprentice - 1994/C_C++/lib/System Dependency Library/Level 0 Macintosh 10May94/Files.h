/* Files.h */

#ifndef Included_Files_h
#define Included_Files_h

/* Files module depends on: */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Array */

struct FileSpec;
typedef struct FileSpec FileSpec;

/* refnum for files that have been opened */
struct FileType;
typedef struct FileType FileType;

/* modes that an existing file can be opened as */
typedef enum {eReadOnly EXECUTE(= -7541), eReadAndWrite} FileModesType;

/* initialize the file subsystem.  should only be called from Screen */
MyBoolean						Eep_InitializeFiles(void);

/* shutdown the file subsystem. */
void								Eep_ShutdownFiles(void);

/* this is an internal routine -- do not use */
#if DEBUG
	MyBoolean					Eep_RegisterFileSpec(FileSpec* Spec);
#else
	#define Eep_RegisterFileSpec(stupid) (True)
#endif

/* make a copy of a file specification */
/* this routine does not perform operations on file descriptors. */
FileSpec*						DuplicateFileSpec(FileSpec* Original);

/* dispose of a file specification thing */
void								DisposeFileSpec(FileSpec* Spec);

/* create a new temporary file with a known unique name, and return a specification */
/* leading to it.  the creator and filetype codes may or may not be used by */
/* the implementation; the Macintosh does use them.  The file is actually created. */
/* this routine does not perform operations on file descriptors. */
FileSpec*						NewTempFileSpec(unsigned long Creator, unsigned long FileType);

/* create a file spec leading to a named preference file.  The location of preference */
/* files are implementation defined:  UNIX may put them in the user's home directory */
/* with a leading period; Macintosh puts them in a "Preferences" folder.  The file */
/* is actually created.  PrefsFileName is a null terminated string. */
/* this routine does not perform operations on file descriptors. */
FileSpec*						NewPrefsFileSpec(char* PrefsFileName, unsigned long Creator,
											unsigned long FileType);

/* present a dialog box allowing the user to select where to create a new file. */
/* If the file will overwrite an existing file, verify this with the user and then */
/* delete the existing file before returning. DefaultFileName is a null terminated */
/* string. */
/* this routine does not perform operations on file descriptors. */
FileSpec*						PutFile(char* DefaultFileName);

/* let the user find a file with the specified list of types.  Whether the types */
/* are actually used is implementation defined.  The Macintosh uses up to 4 types. */
/* returns NIL if the operation was cancelled. */
/* this routine does not perform operations on file descriptors. */
FileSpec*						GetFileStandard(long NumFileTypes, unsigned long* ArrayOfFileTypes);

/* get any file.  like GetFileStandard except it shows all possible files */
/* this routine does not perform operations on file descriptors. */
FileSpec*						GetFileAny(void);

/* return a pointer containing a non-null-terminated which is the filename */
/* this routine does not perform operations on file descriptors. */
char*								ExtractFileName(FileSpec* Spec);

/* create a file.  Returns True if the creation succeeded.  Whether or not */
/* the Creator and FileType codes are used is implementation defined.  The Macintosh */
/* does use them */
/* this routine does not perform operations on file descriptors. */
MyBoolean						CreateFile(FileSpec* FileLocation, unsigned long Creator,
											unsigned long FileType);

/* delte a file.  The file must not be in use.  Returns True if successful */
/* this routine does not perform operations on file descriptors. */
MyBoolean						DeleteFile(FileSpec* FileLocation);

/* Open a file for the specified access.  Returns True if successful. */
/* this routine DOES perform operations on file descriptors. */
MyBoolean						OpenFile(FileSpec* FileLocation, FileType** FileRefOut,
											FileModesType FileAccessMode);

/* close a file.  The file must have been open.  Implicitly calls FlushLocalBuffers. */
/* this routine DOES perform operations on file descriptors. */
void								CloseFile(FileType* FileRef);

/* make sure all data associated with a file gets written out */
/* this routine DOES perform operations on file descriptors. */
void								FlushLocalBuffers(FileType* FileRef);

/* write a block of data to the file.  Returns the number of bytes which could */
/* not be written or 0 if all were written. */
/* This is the only function that calls "FSWrite" */
/* this routine DOES perform operations on file descriptors. */
long								WriteToFile(FileType* FileRef, char* Buffer, long NumBytes);

/* read a block of data from the file.  Returns the number of bytes which */
/* could not be read, or 0 if all were read. */
/* This is the only function that calls "FSRead" */
/* this routine DOES perform operations on file descriptors. */
long								ReadFromFile(FileType* FileRef, char* Buffer, long NumBytesDesired);

/* get the current index into the specified file */
/* this routine DOES perform operations on file descriptors. */
long								GetFilePosition(FileType* FileRef);

/* move to a new location within the specified file */
/* if the new location is past the EOF, the EOF will be extended to the new location */
/* this routine DOES perform operations on file descriptors. */
MyBoolean						SetFilePosition(FileType* FileRef, long NewLocation);

/* get the length of the specified file */
/* this routine DOES perform operations on file descriptors. */
long								GetFileLength(FileType* FileRef);

/* set the length of the specified file.  call TestFileErrorFlag to see if */
/* an error occurred (probably wasn't enough disk space to extend the file) */
/* returns True if everything went well or False if there was an error */
/* If the file could not be extended due to lack of disk space, the EOF */
/* remains unchanged */
/* this routine DOES perform operations on file descriptors. */
MyBoolean						SetFileLength(FileType* FileRef, long NewFileLength);

/* write a signed character to the file, returning True if successful. */
/* this routine does not perform operations on file descriptors. */
MyBoolean						WriteSChar(FileType* FileRef, char TheCharacter);

/* write an unsigned character to the file.  Value is in 0..255 */
/* this routine does not perform operations on file descriptors. */
MyBoolean						WriteUChar(FileType* FileRef, unsigned char TheCharacter);

/* write a 16-bit little endian 2's complement value to the file, returning */
/* True if successful. */
/* this routine does not perform operations on file descriptors. */
MyBoolean						WriteSShort(FileType* FileRef, short TheShort);

/* write a 16-bit unsigned integer to the file, returning True if successful */
/* this routine does not perform operations on file descriptors. */
MyBoolean						WriteUShort(FileType* FileRef, unsigned short TheShort);

/* write a 32-bit little endian 2's complement value to the file, returning */
/* True if successful. */
/* this routine does not perform operations on file descriptors. */
MyBoolean						WriteSLong(FileType* FileRef, long TheLong);

/* write an unsigned 32-bit little endian value to the file, returning */
/* True if successful. */
/* this routine does not perform operations on file descriptors. */
MyBoolean						WriteULong(FileType* FileRef, unsigned long TheLong);

/* read a character from the file. */
/* this routine does not perform operations on file descriptors. */
MyBoolean						ReadSChar(FileType* FileRef, char* DataOut);

/* read an unsigned character from the file. */
/* this routine does not perform operations on file descriptors. */
MyBoolean						ReadUChar(FileType* FileRef, unsigned char* DataOut);

/* read a 16-bit little endian 2's complement value from the file, convert it to */
/* the machines internal representation, and return. */
/* this routine does not perform operations on file descriptors. */
MyBoolean						ReadSShort(FileType* FileRef, short* DataOut);

/* read a 16-bit unsigned value from the file. */
/* this routine does not perform operations on file descriptors. */
MyBoolean						ReadUShort(FileType* FileRef, unsigned short* DataOut);

/* read a 32-bit little endian 2's complement value from the file, convert it to */
/* the machines internal representation, and return. */
/* this routine does not perform operations on file descriptors. */
MyBoolean						ReadSLong(FileType* FileRef, long* DataOut);

/* read a 32-bit unsigned value from the file. */
/* this routine does not perform operations on file descriptors. */
MyBoolean						ReadULong(FileType* FileRef, unsigned long* DataOut);

/* copy the entire contents of one file to another.  Data overwrites destination */
/* this routine does not perform operations on file descriptors. */
MyBoolean						CopyFile(FileType* Original, FileType* Destination);

/* create a temporary file in the directory.  the file is actually created so it */
/* exists (so nobody can grab it out from under you). */
/* this routine DOES perform operations on file descriptors. */
FileSpec*						NewTempFileInTheSameDirectory(FileSpec* SameDirectoryAsThis);

/* this operation swaps the data forks for two files.  this is used for safe */
/* saving (i.e. write data before clobbering so that the old file still exists */
/* if the write fails).  You create a new file and write the data to it.  Then you */
/* pass the location of the new file in NewFile and the location of the old */
/* file in OldFile.  The routine swaps the data in the files and the last modified */
/* dates.  If successful, the NewFile (temporary file) will be deleted and the */
/* OldFileRef reference will be updated (it will NOT be the same) and the NewFileRef */
/* will be closed. */
/* NOTE:  the files should be created in the same directory. */
/* this routine DOES perform operations on file descriptors. */
MyBoolean						SwapFileDataForks(FileSpec* NewAndTempFile, FileSpec* OldFile,
											FileType* NewFileRef, FileType** OldFileRef);

#endif
