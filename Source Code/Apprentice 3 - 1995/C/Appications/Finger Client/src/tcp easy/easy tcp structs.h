// easy tcp structs.h
// Darrell Anderson

/*

structures used to carry data between routines and modules

note: if you're planning on defaulting char * items, copy them 
in instead of redirecting the pointer.  This prevents longer user-supplied
strings from exceeding the array bounds.

*/

#ifndef __easy_tcp_structs__
#define __easy_tcp_structs__

typedef struct HostNPort {
	char hostname[255];
	short port;
} HostNPort;

typedef struct UserNPass {
	char username[255];
	char password[255];
} UserNPass;

#endif