//	Copyright 1993 Ralph Gonzalez

/*
*   FILE:    class.h
*   AUTHOR:  R.G.
*   CREATED: Mar 31, 1992
*   
*   Header file defines Generic_Class
*/

# ifdef	THINK_C
# include	"Types.h"	// TC4: # include	"MacTypes.h"
# endif

# ifndef class_h       // prevents header "re-inclusion"
# define class_h

typedef  int   boolean;  // simulate boolean data type
# define TRUE  1
# define FALSE 0

/******************************************************************
*   Root class for all classes.  In Think C assume the "classes are
*   indirect by default" compiler option is on.
******************************************************************/
class  Generic_Class
{
protected:
    boolean         initialized;

public:
    Generic_Class(void);
    boolean is_initialized(void);
};

# endif
