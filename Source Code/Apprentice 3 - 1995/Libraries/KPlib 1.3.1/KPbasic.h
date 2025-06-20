// A module of KPlib v1.3.1.
// Written by Keith Pomakis during the summer of 1994.
// Released to the public domain on October 10, 1994.

#ifndef KP_BASIC_DEFINED
#define KP_BASIC_DEFINED

#include <string.h>
#include <iostream.h>
#include <stdlib.h>

/****************************************************************************/

#ifndef NULL
#define NULL 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

/****************************************************************************/

// Since the bool type has only recently been added to the C++ standard,
// many compilers may not implement it yet.  Uncomment the following lines
// to simulate the bool type.

/*
typedef int bool;
const int false = 0;
const int true = 1;
*/

/****************************************************************************/

#ifndef min         // Hopefully this isn't already defined as a macro

template <class T>
inline T
min(T arg1, T arg2)
{
    return (arg1 < arg2)? arg1 : arg2;
}

inline char *
min(char *arg1, char *arg2)
{
    return (strcmp(arg1, arg2) < 0)? arg1 : arg2;
}

#endif

/****************************************************************************/

#ifndef max         // Hopefully this isn't already defined as a macro

template <class T>
inline T
max(T arg1, T arg2)
{
    return (arg2 < arg1)? arg1 : arg2;
}

inline char *
max(char *arg1, char *arg2)
{
    return (strcmp(arg2, arg1) < 0)? arg1 : arg2;
}

#endif

/****************************************************************************/

template <class T>
inline void
swap(T &arg1, T &arg2)
{
    T tmp = arg1;
    arg1 = arg2;
    arg2 = tmp;
}

inline void swap(int &arg1, int &arg2)
{ arg1 ^= arg2 ^= arg1 ^= arg2; }

inline void swap(unsigned int &arg1, unsigned int &arg2)
{ arg1 ^= arg2 ^= arg1 ^= arg2; }

inline void swap(short &arg1, short &arg2)
{ arg1 ^= arg2 ^= arg1 ^= arg2; }

inline void swap(unsigned short &arg1, unsigned short &arg2)
{ arg1 ^= arg2 ^= arg1 ^= arg2; }

inline void swap(long &arg1, long &arg2)
{ arg1 ^= arg2 ^= arg1 ^= arg2; }

inline void swap(unsigned long &arg1, unsigned long &arg2)
{ arg1 ^= arg2 ^= arg1 ^= arg2; }

inline void swap(char &arg1, char &arg2)
{ arg1 ^= arg2 ^= arg1 ^= arg2; }

inline void swap(unsigned char &arg1, unsigned char &arg2)
{ arg1 ^= arg2 ^= arg1 ^= arg2; }

inline void swap(bool &arg1, bool &arg2)
{ arg1 ^= arg2 ^= arg1 ^= arg2; }

/****************************************************************************/

inline void
check_mem(void *ptr)
{
    // Declare string as static so that it isn't defined per instantiation.
    static const char *const mem_err = "Error allocating memory.\n";
    if (ptr == NULL) {
        cerr << mem_err;
        exit(EXIT_FAILURE);
    }
}

/****************************************************************************/

#endif /* KP_BASIC_DEFINED */
