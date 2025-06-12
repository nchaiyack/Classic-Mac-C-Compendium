/*
** utils.h
**
** header file for utilities
*/

#ifndef UTILS_H
#define UTILS_H

Boolean TrapAvailable(int theTrap);


/** My homebuilt string functions - to avoid the standard libraries **/
long mystrlen( char *s);
char *mystrcpy( char *s1, char *s2);
char *mystrcat( char *s1, char *s2);
char *mystrchr( char *s1, char c);

#endif

