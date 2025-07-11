/* BufferedFileInput.h */

#ifndef Included_BufferedFileInput_h
#define Included_BufferedFileInput_h

/* BufferedFileInput module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Files */
/* Memory */

struct BufferedInputRec;
typedef struct BufferedInputRec BufferedInputRec;

/* forwards */
struct FileType;

/* create a new buffered input object around a file.  the file is not allowed */
/* to be tampered with after it has been registered with this since I am too */
/* lazy to implement proper buffering in the Level 0 library like I should. */
BufferedInputRec*		NewBufferedInput(struct FileType* TheFileDescriptor);

/* clean up a buffered input object from around a file.  the file may be */
/* used normally after this has been called. */
void								EndBufferedInput(BufferedInputRec* BufferedThang);

/* read a raw block of data from the file.  returns True if all went well or */
/* False if some data could not be written to the file. */
MyBoolean						ReadBufferedInput(BufferedInputRec* BufferedThang,
											long RequestedBytes, char* PlaceToPut);


/* read in a signed (2's complement) character.  returns True if successful. */
MyBoolean						ReadBufferedSignedChar(BufferedInputRec* BufferedThang,
											signed char* SignedCharOut);

/* read in an unsigned character.  returns True if successful. */
MyBoolean						ReadBufferedUnsignedChar(BufferedInputRec* BufferedThang,
											unsigned char* UnsignedCharOut);


/* read in a signed (2's complement) 16-bit short little endian.  returns True if sucessful. */
MyBoolean						ReadBufferedSignedShortLittleEndian(BufferedInputRec* BufferedThang,
											signed short* SignedShortOut);
/* read in a signed (2's complement) 16-bit short big endian.  returns True if sucessful. */
MyBoolean						ReadBufferedSignedShortBigEndian(BufferedInputRec* BufferedThang,
											signed short* SignedShortOut);

/* read in an unsigned 16-bit short little endian.  returns True if successful. */
MyBoolean						ReadBufferedUnsignedShortLittleEndian(BufferedInputRec* BufferedThang,
											unsigned short* UnsignedShortOut);
/* read in an unsigned 16-bit short big endian.  returns True if successful. */
MyBoolean						ReadBufferedUnsignedShortBigEndian(BufferedInputRec* BufferedThang,
											unsigned short* UnsignedShortOut);

/* read in a signed (2's complement) 32-bit long little endian.  returns True if successful. */
MyBoolean						ReadBufferedSignedLongLittleEndian(BufferedInputRec* BufferedThang,
											signed long* SignedLongOut);
/* read in a signed (2's complement) 32-bit long big endian.  returns True if successful. */
MyBoolean						ReadBufferedSignedLongBigEndian(BufferedInputRec* BufferedThang,
											signed long* SignedLongOut);

/* read in an unsigned 32-bit long little endian.  returns True if successful. */
MyBoolean						ReadBufferedUnsignedLongLittleEndian(BufferedInputRec* BufferedThang,
											unsigned long* UnsignedLongOut);
/* read in an unsigned 32-bit long big endian.  returns True if successful. */
MyBoolean						ReadBufferedUnsignedLongBigEndian(BufferedInputRec* BufferedThang,
											unsigned long* UnsignedLongOut);

#endif
