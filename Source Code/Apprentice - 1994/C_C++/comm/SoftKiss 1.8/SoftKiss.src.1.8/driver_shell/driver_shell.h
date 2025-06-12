/*
 * driver_shell - code for dynamicly installing a driver
 * at runtime
 * SoftKiss
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#define ds_NAME_LEN (48)	/*name length field including length byte*/

struct driver_shell_header_R {
	short	drvrFlags;
	short	drvrDelay;
	short	drvrEMask;		/*just used for DAs*/
	short	drvrMenu;		/*just used for DAs*/
/*offsets to routines*/
	short	drvrOpen;
	short	drvrPrime;
	short	drvrCtl;
	short	drvrStatus;
	short	drvrClose;

	unsigned char drvrName[ds_NAME_LEN]; /*pstring name of driver*/
	long call_me;		/*routine to call*/
	long iba4;			/*a4 reg value when in use*/
	long iba5;			/*a5 reg value*/
	long sh_more;		/*extra argument*/
	long sh_reserve[2];	/*reserved for future expansion*/
};
typedef struct driver_shell_header_R driver_shell_header,*driver_shell_header_pt;

#define sh_OPEN 	0
#define sh_PRIME	1
#define sh_CONTROL	2
#define sh_STATUS	3
#define sh_CLOSE	4

/*
 * prepare a driver template to be a certain driver
 * returns true if successfull
 */
int sh_prepare(
	driver_shell_header_pt ads,
	unsigned char *aname,
	short aFlags,
	short aDelay,
	long acall_me,
	long aiba4,
	long aiba5,
	long ash_more);

/*
 * return a5 for driver_shell to set when it calls us back
 * a5 is used when this module is linked to a test application
 * normaly we are part of a driver and a4 is used
 */
long sh_geta5(void);

/*
 * return a4 for driver_shell to set when it calls us back
 */
long sh_geta4(void);

/*
 * install the passed driver
 * apple never got around to the glue for this call
 * if they ever do this routine should be removed
 */
short DrvrInstall(Handle drvrHandle, short dRef);

/*
 * remove the passed device driver
 * apple never got around to the glue for this call
 * if they ever do this routine should be removed
 */
short DrvrRemove(short refNum);

/*
 * Drvr install doesn't do anything except fill in dCtlRefNum
 * so follow up and copy over the drivers info.
 * Handle passed in should be locked
 */
short Real_DrvrInstall(Handle DrvrHandle, short dRef);

/*
 * load the driver template into memory
 * returns nil if it can't be loaded
 */
Handle sh_read_in_driver_shell(void);

/*
 * return a duplicate of the passed handle
 * the copy will be in the system heap
 * and moved high and locked
 */
Handle sh_copy_template(Handle sh_template);

/*
 * a resource into the system heap by name
 * returns nil if it can't be loaded
 */
Handle sh_read_in_sys_res(ResType id,unsigned char *name);

/*
 * strip address that strips even before 32bit addressing is on
 * so the address will still be valid later
 */
void *sh_strip_address(void *addr);
