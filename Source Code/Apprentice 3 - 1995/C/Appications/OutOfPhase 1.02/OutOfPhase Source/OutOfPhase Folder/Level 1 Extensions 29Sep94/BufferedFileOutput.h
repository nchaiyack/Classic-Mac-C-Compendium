/* BufferedFileOutput.h */

#ifndef Included_BufferedFileOutput_h
#define Included_BufferedFileOutput_h

/* BufferedFileOutput module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Files */
/* Memory */

struct BufferedOutputRec;
typedef struct BufferedOutputRec BufferedOutputRec;

/* forwards */
struct FileType;

/* create a new buffered output object around a file.  the file is not allowed */
/* to be tampered with after it has been registered with this since I am too */
/* lazy to implement proper buffering in the Level 0 library like I should. */
BufferedOutputRec*	NewBufferedOutput(struct FileType* TheFileDescriptor);

/* clean up a buffered output object from around a file.  the file may be */
/* used normally after this has been called.  if it failed to write the */
/* data out to disk, then it returns False. */
MyBoolean						EndBufferedOutput(BufferedOutputRec* BufferedThang);

/* write a raw block of data to the file.  it returns 0 if it wrote all of the */
/* data in or however many bytes were not able to be written. */
MyBoolean						WriteBufferedOutput(BufferedOutputRec* BufferedThang,
											long RequestedBytes, char* PlaceToGetFrom);


/* write a signed character to the file.  returns True if successful. */
MyBoolean						WriteBufferedSignedChar(BufferedOutputRec* BufferedThang,
											signed char SignedChar);

/* write an unsigned character to the file.  returns True if successful. */
MyBoolean						WriteBufferedUnsignedChar(BufferedOutputRec* BufferedThang,
											unsigned char UnsignedChar);


/* write a signed 2's complement 16-bit short little endian.  returns True if successful */
MyBoolean						WriteBufferedSignedShortLittleEndian(BufferedOutputRec* BufferedThang,
											signed short SignedShort);
/* write a signed 2's complement 16-bit short big endian.  returns True if successful */
MyBoolean						WriteBufferedSignedShortBigEndian(BufferedOutputRec* BufferedThang,
											signed short SignedShort);

/* write an unsigned 16-bit short little endian.  returns True if successful. */
MyBoolean						WriteBufferedUnsignedShortLittleEndian(BufferedOutputRec* BufferedThang,
											unsigned short UnsignedShort);
/* write an unsigned 16-bit short big endian.  returns True if successful. */
MyBoolean						WriteBufferedUnsignedShortBigEndian(BufferedOutputRec* BufferedThang,
											unsigned short UnsignedShort);


/* write a signed 2's complement 32-bit long little endian.  returns True if successful */
MyBoolean						WriteBufferedSignedLongLittleEndian(BufferedOutputRec* BufferedThang,
											signed long SignedLong);
/* write a signed 2's complement 32-bit long big endian.  returns True if successful */
MyBoolean						WriteBufferedSignedLongBigEndian(BufferedOutputRec* BufferedThang,
											signed long SignedLong);

/* write an unsigned 32-bit long little endian.  returns True if successful */
MyBoolean						WriteBufferedUnsignedLongLittleEndian(BufferedOutputRec* BufferedThang,
											unsigned long UnsignedLong);
/* write an unsigned 32-bit long big endian.  returns True if successful */
MyBoolean						WriteBufferedUnsignedLongBigEndian(BufferedOutputRec* BufferedThang,
											unsigned long UnsignedLong);

#endif
