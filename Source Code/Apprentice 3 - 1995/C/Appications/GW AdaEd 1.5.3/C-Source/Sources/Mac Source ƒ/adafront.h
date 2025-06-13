
#ifdef MAC_GWADA

#ifndef _ANSIFabLib_h
#include	"ANSIFabLib.h"
#endif

#ifndef _ifile_h
#include	"ifile.h"
#endif

extern fabIFILE	*LIBFILE;
extern fabFILE	*gSrcFileP;
#else
extern IFILE	*LIBFILE;
extern FILE		*gAdafile;
#endif

extern IFILE *TREFILE, *AISFILE, *STUBFILE;
extern FILE	*errfile;
