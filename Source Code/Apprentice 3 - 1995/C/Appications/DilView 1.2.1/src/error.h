/* error.h */

void myError( char *errMsg, Boolean fatal );

// myError is really a wrapper for ErrorDLOG, but I don't wan't to trap myself!
void ErrorDLOG( char *errMsg, Boolean fatal );
