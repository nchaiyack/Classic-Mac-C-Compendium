/*
 * find the driver number of the softkiss driver
 * by Aaron Wohl (aw0g+@andrew.cmu.edu) jul 1990
 * Carnegie-Mellon University
 * Special Projects
 * Pittsburgh, PA 15213-3890
 * (412)-268-5032
 *
 * special thanks to Luni and Rich Brown @ Dartmouth
 *
 * Luni wrote the MacTCP emulator for macmach from which this file
 * borrows liberaly.
 *
 * Rich Brown provided the source to Dartmouth's BlitzNotify installer
 * which was a big help.
 *
 * Unlike other softkiss source files which are for THINK C compilation
 * only, this module compiles with MPW c also.
 * So be carefull about sizeof(int) in THINK == 2
 * and sizeof(int) in MPW == 4 if you make any changes.
 *
 */

#include "sfk_codecheck.h"
#include "sfk_drvr_find.h"

#define drvrName 18		/*length byte and name of driver [string]*/

/*
 * returns the refnum of the passed driverName
 * or zero if it isn't installed.  If no driver
 * is found free_ref_num returns
 * a refnum that is free both in the
 * unit table and in the system file and any open
 * resource forks.  if there are no free driver slots
 * free_ref_num returns zero
 * if a driver is found the value returned in free_ref_num
 * is indeterminant
 * The ref num free_ref_num returns is currently the largest free
 * Only ref nums less than -12 are considered
 */
short sfk_drvr_find(unsigned char *driverName,short *free_ref_num)
{
  short negCount;
  short dRef;
  DCtlHandle 	dCtl;
  char			*drivePtr;
  short 		result=0;
  char 			**handle;

  if(free_ref_num!=0)
    *free_ref_num=0;		/*assume no free slots*/
  negCount = -UnitNtryCnt; /*get -(table size)*/
        
  /*Check to see that driver is installed, obtain refNum.*/
  /*Assumes that an Open was done previously -- probably by an INIT.*/
  /*Driver doesn't have to be open now, though.*/
        
  SetResLoad(FALSE);
  /*we'll start with driver refnum == -12, right after .ATP entry*/
  /*Look through unit table until we find the driver or reach the end.*/
  for(dRef = -12;(dRef != negCount);dRef--) {

    dCtl = GetDCtlEntry(dRef); /*get handle to DCE*/
 
    if (dCtl == 0L) {
      if(free_ref_num!=0)
        if((handle=GetResource('DRVR',(-dRef)-1))==0)
          *free_ref_num=dRef;
        else
          ReleaseResource(handle);
    } else if (( (**dCtl).dCtlDriver != 0L) ) {
      if ((**dCtl).dCtlFlags & dRAMBased)
	    drivePtr = *(Handle) (**dCtl).dCtlDriver;
	  else
	    drivePtr = (**dCtl).dCtlDriver;
            
	  if ((drivePtr != 0L) &&  
        (EqualString((void *)(drivePtr + drvrName),(void *)driverName,0,0))) {
         result=dRef;
         break;
      }
      
   }
  }
        
  SetResLoad(TRUE);
  return result;
}
