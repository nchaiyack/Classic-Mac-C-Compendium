/* CRC32.h */

#ifndef Included_CRC32_h
#define Included_CRC32_h

/* CRC32 module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */

/* calculate a CRC-32 on a block of data */
unsigned long							CalculateCRC32(char* Buffer, long NumBytes);

#endif
