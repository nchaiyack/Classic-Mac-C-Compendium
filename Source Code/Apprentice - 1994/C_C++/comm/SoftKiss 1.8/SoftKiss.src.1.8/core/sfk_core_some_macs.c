/*
 * SoftKiss code that runs on some macs and not others
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#include "sfk_core_some_macs.h"
#include <OSChecks.h>
#include <Traps.h>
#include <Power.h>
#include <GestaltEqu.h>
/*
 * traps that are too new to be in Traps.h
 */
#define _SerialPowerX 	(0xA685)
#define _IdleStateX 	(0xA485)
#define _CacheFlushTrap	(0xA0BD)

/*
 * is the scc hardware available
 */
int have_scc(void)
{
  long response;
  int iErr=Gestalt(gestaltHardwareAttr,&response);
  if(iErr!=0)
  	return TRUE;	/*can't tell assume yes*/
  return ((response&(1<<gestaltHasSCC))!=0);
}

/*
 * tell if the power manager is implemented on this machine
 */
int serial_power_implemented(void)
{
  return TrapAvailable(_SerialPowerX);
}

/*
 * tell if the sleep/speed manager is implemented on this machine
 */
int sleep_state_implemented(void)
{
  return TrapAvailable(_IdleStateX);
}

/*
 * tell if the virtual memory manager is implemented on this machine
 */
int is_vm_manager_present(void)
{
  return TrapAvailable(_MemoryDispatch);
}

/*
 * tell if the code/data cache  manager is implemented on this machine
 */
int is_cache_manager_present(void)
{
  return TrapAvailable(_CacheFlushTrap);
}

/*
 * power up scc and select internal or external modem
 */
void power_up_port(int pnum,int ignore_modem)
{
	if(sleep_state_implemented())
  		DisableIdle();
  	if(serial_power_implemented()) {
		if(pnum==0)
			BOn();
		else if (ignore_modem)
	  		AOnIgnoreModem();
	  	else
	  		AOn();
	}
}

/*
 * power down scc and modem
 */
void power_down_port(int pnum)
{
  if(serial_power_implemented()) {
    if(pnum==0)
	  AOff();
    else
  	  BOff();
  }
  if(sleep_state_implemented())
 	 EnableIdle();
}

/*
 * Lock something in memory if virtual memory is running
 * return TRUE iff it was locked
 */
int sfk_lock_if_vm_implimented(void *something_to_lock,long size)
{
// get vm working some day ??
// if(!is_vm_manager_present())
  	return FALSE;
  LockMemory(something_to_lock,size);
  return TRUE;
}

/*
 * unlocked the passed block to make it swapable
 */
void sfk_unlock_if_vm_implimented(void *something_to_unlock,long size)
{
//  if(is_vm_manager_present())
//	UnlockMemory(something_to_unlock,size);
}

/*
 * some macs have a data and code cache that needs to be flushed
 * after patching code
 */
void sfk_some_flush(void)
{
  FlushCache();
}

/*
 * return the time manager version
 */
int time_manager_version(void)
{
  long response;
  int iErr=Gestalt(gestaltTimeMgrVersion,&response);
  if(iErr!=0)
  	return 1;	/*can't tell assume one version 1*/
  return response;
}


