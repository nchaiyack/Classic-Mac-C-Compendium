/*
 * This version of MF allows a number of .mf files to be processed successively,
 * taking advantage of a facility introduced by Knuth --
 * but rarely if ever used nowadays --
 * to run TeX or MF from core-dumps already initialized.
 *
 * Unfortunately, for some reason I have been unable to establish,
 * MF does not always close all its open files.
 * The number of open files builds up on successive runs,
 * until finally the number exceeds the maximum allowable.
 *
 * This little Think C function closes all open files.
 */

#include <stdio.h>
 
void closeallfiles(void);
 
void closeallfiles(void)
{
  int fd;
  
  for (fd = 4; fd <= FOPEN_MAX; fd++)
    if (__file[fd].refnum)
      fclose(&__file[fd]);
}
