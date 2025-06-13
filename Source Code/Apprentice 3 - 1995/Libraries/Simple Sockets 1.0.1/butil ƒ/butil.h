/*
 * butil.h 
 *
 * Ansi headers for the butil.c file
 *
 * Mike Trent 8/94
 *
 */
 
void bcopy (char *b1, char *b2, long length);
int bcmp (char *b1, char *b2, long length);
void bzero (char *data, long size);
long ffs (long i);
