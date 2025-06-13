//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Macros3D.h
//|
//| This file contains various macros
//|
//| This code was created by Greg Ferrar, based on original code
//| created by Jeff Palmer.  There are no restrictions on the use
//| or distribution of this library, except that you may not
//| restrict its use or distribution.  Any project which is
//| created from this code, or any significant modification of
//| this code, may be distributed in any way you choose; you may
//| even sell it or its source code.  However, any limitations
//| placed on the distribution or use of such a project or
//| modification may not in any way limit the distribution or use
//| of this original code.
//| MAART: Added MAX, MIN
//|________________________________________________________________


#ifndef Macros3D_h
#define Macros3D_h
				 

#define ABSVAL(a) (((a)<0) ? -(a) : (a))
#define SIGN(a)   (((a)<0) ? -1 : 1)
#define ABS(X)    (((X) > 0) ? (X) : -(X))
#ifndef MAX
#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))   
#endif
#ifndef MIN
#define MIN(X, Y) (((Y) > (X)) ? (X) : (Y))   
#endif

#endif

