/* FileRoutines.h
 *
 * defines and prototypes for functions in FileRoutines.c
 *
 */

#ifndef FileRoutines_h
#define FileRoutines_h

#define kSequential	-1

Boolean ReadByte( short fileRef, long recordNo, char* buffer );
Boolean WriteByte( short fileRef, long recordNo, char buffer );

Boolean ReadShort( short fileRef, long recordNo, short* buffer );
Boolean WriteShort( short fileRef, long recordNo, short buffer );

Boolean ReadLong( short fileRef, long recordNo, long* buffer );
Boolean WriteLong( short fileRef, long recordNo, long buffer );

Boolean ReadRecord( short fileRef, long recordNo, void* buffer );
Boolean WriteRecord( short fileRef, long recordNo, void* buffer );

Boolean ReadBlock( short fileRef, long filePos,
							long bytesRequested, void* buffer );
Boolean WriteBlock( short fileRef, long filePos,
							long bytesRequested, void* buffer );

long	RemainingSpace( short vRefNum );

long	RecordsInFile( short refNum, short recordSize );

long	GetCurrentRecord( short refNum, short recordSize );
Boolean	SetCurrentRecord( short refNum, long recordNo,
											short recordSize );

short	LastFileError( void );

#endif
