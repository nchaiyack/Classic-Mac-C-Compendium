#if defined(powerc) || defined (__powerc)
#define	FabSystem7orlater	1
#else
/*
comment out the #define below to obtain a System 6
compatible version of some libraries.
If you #define FabSystem7orlater, the libraries get
smaller (the code needed under System 6 only is
stripped out). */
//
#define	FabSystem7orlater	1
//#define	FabNoSegmentDirectives	1

#endif

#include	"UtilsCommon.h"
