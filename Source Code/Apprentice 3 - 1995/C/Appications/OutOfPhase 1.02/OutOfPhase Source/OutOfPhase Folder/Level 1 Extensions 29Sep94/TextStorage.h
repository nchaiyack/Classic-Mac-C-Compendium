/* TextStorage.h */

#ifndef Included_DataText_h
#define Included_DataText_h

/* TextStorage module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* DataMunging */
/* Array */
/* Files */
/* BufferedFileOutput */

struct TextStorageRec;
typedef struct TextStorageRec TextStorageRec;

/* to avoid dragging Files.h into the header */
struct FileType;

/* allocate a new structure for storing text type data */
TextStorageRec*			NewTextStorage(void);

/* dispose the text storage structure and any data it contains */
void								DisposeTextStorage(TextStorageRec* Storage);

/* find out how many lines the text storage object contains */
long								TextStorageGetLineCount(TextStorageRec* Storage);

/* get a copy of the specified line */
char*								TextStorageGetLineCopy(TextStorageRec* Storage, long LineIndex);

/* get a pointer to the actual line as stored in the array.  this line should */
/* not be modified in any way. */
char*								TextStorageGetActualLine(TextStorageRec* Storage, long LineIndex);

/* get the length of the specified line */
long								TextStorageGetLineLength(TextStorageRec* Storage, long LineIndex);

/* put a new line in the text storage object.  the original contents of the */
/* specified line are deleted.  returns False if it couldn't be completed. */
/* (it could fail since a copy of the line is made) */
MyBoolean						TextStorageChangeLine(TextStorageRec* Storage, long LineIndex,
											char* NewLine);

/* insert a new empty line at the specified position.  returns False if it failed */
MyBoolean						TextStorageInsertLine(TextStorageRec* Storage, long LineIndex);

/* delete the line at the specified position */
void								TextStorageDeleteLine(TextStorageRec* Storage, long LineIndex);

/* break the line at the specified character index.  this is used for inserting */
/* carriage returns. */
MyBoolean						TextStorageBreakLine(TextStorageRec* Storage, long LineIndex,
											long CharIndex);

/* replace the line and the line after it with a single line containing the */
/* second line concatenated onto the end of the first line */
MyBoolean						TextStorageFoldLines(TextStorageRec* Storage, long LineIndex);

/* extract part of the stored data in the form of another text storage object */
TextStorageRec*			TextStorageExtractSection(TextStorageRec* Storage,
											long StartLine, long StartChar, long EndLine, long EndChar);

/* delete the specified range of data from the storage.  returns False if it */
/* failed.  if this routine fails, it may have left the task partially finished */
MyBoolean						TextStorageDeleteSection(TextStorageRec* Storage,
											long StartLine, long StartChar, long EndLine, long EndChar);

/* insert a storage block at the specified position into this storage block. */
/* returns False if it failed.  if it fails, then it may have actually inserted */
/* some of the data into the storage record */
MyBoolean						TextStorageInsertSection(TextStorageRec* Storage,
											long WhereLine, long WhereChar, TextStorageRec* Stuff);

/* if the end of line sequence is of the specified length, then calculate how */
/* many characters a packed buffer of text would contain */
long								TextStorageTotalNumChars(TextStorageRec* Storage, long EOLNSize);

/* create a packed buffer of lines separated by the specified end of line sequence. */
/* the end of line sequence is null terminated */
char*								TextStorageMakeRawBuffer(TextStorageRec* Storage, char* EOLNChar);

/* decode the packed buffer of lines and create a text storage record from it. */
TextStorageRec*			TextStorageFromRawBuffer(char* Buffer, char* EOLNChar);

/* find out if the data has been changed since the last call to */
/* TextStorageDataIsUpToDate */
MyBoolean						TextStorageHasDataChanged(TextStorageRec* Storage);

/* indicate that any changes in the data have been recognized */
void								TextStorageDataIsUpToDate(TextStorageRec* Storage);

/* write the entire buffer to a file using the specified end of line sequence. */
/* returns True if successful */
MyBoolean						TextStorageWriteDataToFile(TextStorageRec* Storage,
											struct FileType* FileRefNum, char* EOLN);

#endif
