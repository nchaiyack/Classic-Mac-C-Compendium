/* Alerts */

extern void	Calert( char* theMsg );
extern int  Cask( char* theMsg );
extern void	PError( char *s );


extern void  theAlert( int ErrorInd,int ErrorType, OSErr theOSErr, Boolean exiting);


#define strerr (char*)_strerr

extern char  _strerr[];