/* 22may95,ejo: added this for OSErr */
#include <Types.h>

/*****************************************************/
/* STR resources */

#define FILE_MANAGER	128
#define GETFINFOERROR	1
#define CREATEERROR		2
#define FSDELETEERROR	3
#define RENAMEERROR		4
#define PBGETFINFO		5

#define FILE_IO			129
#define W_ERROR			1
#define R_ERROR			2

#define GENERIC			130
#define NO_CURSOR		1
#define EMPTY_FILE		2
#define NOT_TEXT		3
#define RES_FORK		4
#define NO_SAVE_PREF	5
#define EMPTY_SUFFIX	6
#define OLD_SYSTEM		7
#define BAD_DISK		8

/*****************************************************/
/* OpenFile Results */

#define Cancelled		10
#define	GzipNo			-1
#define	GzipOK			0
#define	GzipError		1
#define	GzipWarning		2


/*****************************************************/
/* stdio */

extern void	Calert( char* theMsg );
extern int  Cask( char* theMsg );
extern void	PError( char *s );


extern void	theAlert( int ErrorInd,int ErrorType, OSErr theOSErr, Boolean exiting);


#define strerr (char*)_strerr
extern char  _strerr[];