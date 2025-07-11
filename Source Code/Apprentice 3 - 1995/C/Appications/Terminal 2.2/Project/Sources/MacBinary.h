/*
	Terminal 2.2
	"MacBinary.h"
*/

#define BinHeaderLength 128

short BinCheckHeader (Byte *, Byte *, long *, long *);
short BinCloseRead (void);
short BinCloseWrite (void);
short BinGetEOF (long *);
short BinOpenRead (short, long, Byte *);
short BinOpenWrite (short, long, Byte *, Byte *);
short BinRead (long * , Byte *);
short BinWrite (long *, Byte *);
