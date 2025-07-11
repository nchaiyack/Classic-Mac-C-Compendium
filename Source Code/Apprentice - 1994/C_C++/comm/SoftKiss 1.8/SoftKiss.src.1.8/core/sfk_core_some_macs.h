/*
 * interface to features present on some macs and not others
 */

/*
 * tell if the power manager is implemented on this machine
 */
int serial_power_implemented(void);

/*
 * tell if the sleep/speed manager is implemented on this machine
 */
int sleep_state_implemented(void);

/*
 * tell if the virtual memory manager is implemented on this machine
 */
int is_vm_manager_present(void);

/*
 * tell if the code/data cache  manager is implemented on this machine
 */
int is_cache_manager_present(void);

/*
 * some macs have a data and code cache that needs to be flushed
 * after patching code
 */
void sfk_some_flush(void);

/*
 * Lock something in memory if virtual memory is running
 * return TRUE iff it was locked
 */
int sfk_lock_if_vm_implimented(void *something_to_lock,long size);

/*
 * unlocked the passed block to make it swapable
 */
void sfk_unlock_if_vm_implimented(void *something_to_unlock,long size);

/*
 * power up scc and select internal or external modem
 */
void power_up_port(int pnum,int ignore_modem);

/*
 * power down scc and modem
 */
void power_down_port(int pnum);

/*
 * is the scc hardware available
 */
int have_scc(void);

/*
 * return the time manager version
 */
int time_manager_version(void);
