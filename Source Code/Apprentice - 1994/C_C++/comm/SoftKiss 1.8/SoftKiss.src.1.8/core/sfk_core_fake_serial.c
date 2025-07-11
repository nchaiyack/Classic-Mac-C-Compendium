/*
 * SoftKiss - kiss mode replacement serial driver
 * Used for kiss mode emulation and soft tnc emulation
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1992
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#include "sfk_core.h"
#include "sfk_core_private.h"
#include "driver_shell.h"
#include <Serial.h>

/*
 * routines defined in sfk_core_kiss_asm.c
 */
void rp_jiodone(kiss_port_pt p);

/*
 * mark the waiting iopb as finished
 */
void sfk_mark_read_done(sfk_prt_pt p,int er_code)
{
	if(p->K_IN.kp_pb==0)
		return;
	p->K_IN.kp_pb->ioResult=er_code;
	rp_jiodone(&p->K_IN);
}

/*
 * save the dces from the normal serial drivers
 */
static void drivers_save(sfk_prt_pt p)
{
	long *ubase= (long *)UTableBase;
	int i;
	kiss_port_pt k=p->kports;
	for(i=0;i<NUM_KISS_PORTS;i++,k++) {
		if(k->kp_old!=0)
			continue;
		k->kp_old= *(ubase+(-1-k->kp_refnum));
		*(ubase+(-1-k->kp_refnum))=0;
	}
}

/*
 * restore the dces from the normal serial drivers
 */
static void drivers_restore(sfk_prt_pt p)
{
	long *ubase= (long *)UTableBase;
	int i;
	kiss_port_pt k=p->kports;
	for(i=0;i<NUM_KISS_PORTS;i++,k++) {
		if(k->kp_old!=0)
			 *(ubase+(-1-k->kp_refnum))=k->kp_old;
		k->kp_old=0;;
	}
}

/*
 * remove existing drivers from operating system
 */
static void fake_serial_remove_drivers(sfk_prt_pt p)
{
	//??? check remove errors
	DrvrRemove(p->K_IN.kp_refnum);
	DrvrRemove(p->K_OUT.kp_refnum);
}
/*
 * return a duplicate of the 
 */
static Handle copy_template(sfk_iio_pt cmd)
{
	Handle result;
	if((result=sh_copy_template(sfk_gl.driver_template))==0)
		sfk_parse_fail(cmd,TEXT_NUM(22),0);
	return result;
}

/*
 * install one driver, kiss_in or kiss_out or soft_tnc_in or soft_tnc_out
 */
static void fake_serial_install1(
	sfk_iio_pt cmd,
	sfk_prt_pt p,
	kiss_port_pt k,
	long entry_point)
{
	DCtlHandle dCtl;
	Handle drvr_handle=copy_template(cmd);
	driver_shell_header_pt ash= ((driver_shell_header_pt)(*drvr_handle));
	k->kp_state=0;
	k->kp_offset=0;
	if(!sh_prepare(ash,k->kp_name,
			dNeedLock|dStatEnable|dCtlEnable|dRAMBased|
				(k->kp_is_in?(dReadEnable|dNeedTime):dWritEnable),
			2,
			entry_point,
			sh_geta4(),
			sh_geta5(),
			(long)p))
		sfk_parse_fail(cmd,TEXT_NUM(23),0);
	drivers_save(p);
	if(Real_DrvrInstall(drvr_handle,k->kp_refnum)) 
		sfk_parse_fail(cmd,TEXT_NUM(25),0);
	if((dCtl=GetDCtlEntry(k->kp_refnum))==0)
		sfk_parse_fail(cmd,TEXT_NUM(24),0);
	k->kp_de= dCtl;
}

/*
 * initialize input or output static constants
 */
static void fake_serial_init1(
	kiss_port_pt k,
	unsigned char *kp_name,
	int kp_refnum,
	int kp_is_in)
{
	k->kp_refnum=kp_refnum;
	k->kp_is_in=kp_is_in;
	k->kp_name=kp_name;
}

/*
 * one time init at open time
 */
static void fake_serial_init(void)
{
	fake_serial_init1(&sfk_PA->K_IN,"\p.AIn",-6,TRUE);
	fake_serial_init1(&sfk_PA->K_OUT,"\p.AOut",-7,FALSE);
	fake_serial_init1(&sfk_PB->K_IN,"\p.BIn",-8,TRUE);
	fake_serial_init1(&sfk_PB->K_OUT,"\p.BOut",-9,FALSE);
}

/*
 * is the passed driver refnum in use?
 */
static int is_a_driver_open(int ref_num)
{
  DCtlHandle 	dCtl;
  if((dCtl = GetDCtlEntry(ref_num))==0) /*get handle to DCE*/
  	return FALSE;
  return (((*dCtl)->dCtlFlags&dOpened)!=0);
}

/*
 * see if eithor serial port is in use
 */
int is_serial_busy(sfk_prt_pt p)
{
	return (is_a_driver_open(p->K_IN.kp_refnum)||
			is_a_driver_open(p->K_OUT.kp_refnum));
}

/*
 * make shure the serial ports are closed
 */
static void close_serial_ports(sfk_prt_pt p)
{
	FSClose(p->K_IN.kp_refnum);
	FSClose(p->K_OUT.kp_refnum);
}

/*
 * fake serial port input
 */
static short fake_serial_in(CntrlParam *pb,DCtlPtr de,short op,void *extra)
{
	sfk_prt_pt p=extra;
	return (*p->K_IN.kp_io)(pb,de,op,extra);
}

/*
 * fake serial port output
 */
static short fake_serial_out(CntrlParam *pb,DCtlPtr de,short op,void *extra)
{
	sfk_prt_pt p=extra;
	return (*p->K_OUT.kp_io)(pb,de,op,extra);
}

/*
 * enable the fake serial drivers
 */
void fake_serial_on(sfk_iio_pt cmd,sfk_prt_pt p)
{
	fake_serial_init();
	if(p->K_IN.kp_old!=0) return;	/*already on*/
	close_serial_ports(p);
	fake_serial_install1(cmd,p,&p->K_IN,(long)&fake_serial_in);
	fake_serial_install1(cmd,p,&p->K_OUT,(long)&fake_serial_out);
}

/*
 * turn off kiss mode on the passed port
 */
void fake_serial_off(sfk_prt_pt p)
{
	fake_serial_init();
	close_serial_ports(p);
	if(p->K_IN.kp_old!=0) {
		fake_serial_remove_drivers(p);
		drivers_restore(p);
	}
}

/*
 * install one driver, kiss_in or kiss_out
 */
void sfk_fake_serial_install(
	sfk_iio_pt cmd,
	sfk_prt_pt p,
	kiss_port_pt k_in,
	long entry_point_in,
	kiss_port_pt k_out,
	long entry_point_out)
{
	SFK_UNUSED_ARG(cmd);
	SFK_UNUSED_ARG(p);
	k_in->kp_io=(void*)entry_point_in;
	k_out->kp_io=(void*)entry_point_out;
}

/*
 * mark this driver as being open
 * fail if it is already open
 */
int sfk_open_me(DCtlPtr de)
{
	if(de->dCtlStorage!=0)
		return TRUE;
	//we don't really need storage, we use it to tell if we are open
	de->dCtlStorage=NewHandleSysClear(1);
	return FALSE;
}

/*
 * mark this driver as being closed
 */
void sfk_close_me(DCtlPtr de)
{
	if(de->dCtlStorage!=0)
		DisposHandle(de->dCtlStorage);
	de->dCtlStorage=0;
}
