/* DataMunging.h */

#ifndef Included_DataMunging_h
#define Included_DataMunging_h

/* DataMunging module depends on: */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */

#include "Definitions.h"

/* Make a copy of the supplied Ptr.  If there is not enough memory to copy */
/* the Ptr, NIL is returned. */
char*					CopyPtr(char* ThePtr);

/* return the length of a null-terminated string */
long					StrLen(char* String);

/* make a new copy of the Ptr containing a null-terminated string.  The */
/* returned Ptr will NOT contain a null at the end.  Returns NIL if the */
/* routine failed */
char*					StringToBlockCopy(char* String);

/* Make a copy of the specified block Ptr, appending a null to the end */
/* to make it into a null-terminated string.  Returns NIL if it failed */
char*					BlockToStringCopy(char* Block);

/* allocate a block and copy the raw data to it.  Returns NIL if it failed. */
char*					BlockFromRaw(char* Data, long Length);

/* allocate a block and copy the null-terminated string (including the null) */
/* to it.  Returns NIL if the routine failed. */
char*					StringFromRaw(char* Data);

/* allocate a copy of the specified Ptr, appending the character at the end */
/* returns NIL if the routine failed */
char*					AppendCharToBlockCopy(char* ThePtr, char TheChar);

/* compare memory ranges and return True if they are equal or False if not. */
MyBoolean			MemEqu(char* Left, char* Right, long NumBytes);

/* compare memory ranges, but treat uppercase and lowercase letters as equal */
MyBoolean			MemEquNoCase(char* First, char* Second, long NumBytes);

/* return true if the null terminated strings are equal, or false if not */
MyBoolean			StrEqu(char* Left, char* Right);

/* insert the specified block of raw data into the Ptr and return the new copy */
/* returns NIL if the routine failed */
char*					InsertBlockIntoBlockCopy(char* Block, char* NewData,
								long Where, long Length);

/* remove the specified area from the Ptr and return the copy or NIL if it failed */
char*					RemoveBlockFromBlockCopy(char* Block, long Where, long Length);

/* identical to InsertBlockIntoBlockCopy but allows element size to be specified */
char*					InsertEntryIntoArrayCopy(char* Array, char* NewEntry,
								long Where, long ElementSize);

/* Identical to RemoveBlockFromBlockCopy but allows element size to be specified */
char*					RemoveEntryFromArrayCopy(char* Array, long EntryIndex, long ElementSize);

/* search for the first occurrence of Key in Block and replace the data with */
/* Replacement, returning a copy of the data or NIL if it failed */
char*					ReplaceBlockCopy(char* Block, char* Key, char* Replacement);

/* concatenate two blocks, returning the new block or NIL if it failed */
char*					ConcatBlockCopy(char* Left, char* Right);

/* return a middle section of the block or NIL if it failed */
char*					MidBlockCopy(char* Block, long Start, long NumChars);

/* return a block from which a section has been stripped, or NIL if it failed. */
char*					ReduceBlockCopy(char* Block, long Start, long NumChars);

#endif
