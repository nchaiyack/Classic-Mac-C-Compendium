/************************************************************************
*                                                                       *
* sscanf.h                                                              *
*                                                                       *
* Header file for a freely distributable sscanf() function.             *
*                                                                       *
* ********************************************************************* *
*                                                                       *
* Copyright (C) 1992   Brent Burton                                     *
*                                                                       *
************************************************************************/


/* #define COMPARING  /* uncomment this to compare this to the original */


#ifdef COMPARING    /* if comparing, rename sscanf() compatible function */
  int mysscanf(const char *str, const char *fmt, ...);
#else
  int   sscanf(const char *str, const char *fmt, ...);
#endif
