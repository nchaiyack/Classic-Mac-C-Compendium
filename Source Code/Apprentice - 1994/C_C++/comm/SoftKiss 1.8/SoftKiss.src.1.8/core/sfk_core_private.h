/*
 * SoftKiss device handling code private interface
 * for use by code inside the device driver.
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#pragma once

#define SFK_SOFTKISS

#ifdef CODECHECK
#include "sfk_codecheck.h"
#endif

#include "sfk_core_command.h"
#include "dbo_stdio.h"

#include <string.h>
#include <setjmp.h>
#include <asm.h>
#include "sfk_dyntext.h"

/*
 * bits enabled with debug command
 */
#define	DBO_interupts	(1)
#define DBO_serial		(2)

#define sfk_TICKS (1L)
#define sfk_SECS  (60*sfk_TICKS)	/*one second is this many ticks*/
#define sfk_MINS (60*sfk_SECS)
#define sfk_HOURS (60*sfk_MINS)

#define sfk_imax(xx_arg1,xx_arg2) (((xx_arg1)>(xx_arg2))?(xx_arg1):(xx_arg2))
#define sfk_imin(xx_arg1,xx_arg2) (((xx_arg1)<(xx_arg2))?(xx_arg1):(xx_arg2))

#define SFK_ASSERT(xx_bool,xx_err_msg) \
	do { if(!(xx_bool)) sfk_assert_fail(xx_err_msg);} while(0)

/*
 * used to keep codecheck happy when the end of a routine is never reached
 * becuase prevous function call throws an error
 */
#ifdef CODECHECK
#define SFK_NEVER_RETURN(xx_arg) return (xx_arg)
#else
#define SFK_NEVER_RETURN(xx_arg)
#endif

/*
 * fatal error
 */
void sfk_assert_fail(char *msg);

/*
 * map a text number to it's resource index
 */
#define TEXT_NUM(xx_arg) (xx_arg)

#define SFK_TEXT(xx_num) sfk_map_text(TEXT_NUM(xx_num))

/*
 * name of resource containing driver
 */
#define sfk_driver_resource "\pRawSoftKissDRVR"

/*
 * mask the parity bit of a character
 */
#define sfk_MASK_PARITY(xx_arg) ((xx_arg)&0x7f)

/*
 * localy define a procedure with a prototype
 */
#define sfk_LDEF(xx_arg) static xx_arg; static xx_arg

#define sfk_INTS_OFF 0x2600

#ifdef CODECHECK
#define sfk_SCC_interupts_off
#define sfk_SCC_interupts_on
#else
#define sfk_SCC_interupts_off \
	asm { move sr,-(a7)	}	/*save current sr*/ \
	asm { or #sfk_INTS_OFF,sr }	{ /*turn off scc interupts*/

#define sfk_SCC_interupts_on asm {move (a7)+,sr} }
#endif

/*
 * value of each variable private per port
 */
struct sfk_value_R {
	union {				/*assembler macro below assumes union is first*/
		uint32 nm_ival;	/*value of integer/boolean vars*/
		char *nm_sval;	/*value of string variables*/
	}x;
	char changed;		/*true if changed since last online*/
};
typedef struct sfk_value_R sfk_value,*sfk_value_pt;

#define sfk_CMD_NUM(xx_vnum) (sfk_cval_##xx_vnum)
#define sfk_IVAR(xx_vnum) var_vals[sfk_CMD_NUM(xx_vnum)].x.nm_ival

/*
 * same IVAR for use in assembler
 */
#define sfk_IVAR_ASM(xx_vnum) (OFFSET(sfk_prt,var_vals)+sfk_CMD_NUM(xx_vnum)*sizeof(sfk_value))

#define sfk_SVAR(xx_vnum) var_vals[sfk_CMD_NUM(xx_vnum)].x.nm_sval
#define sfk_CHANGED(xx_vnum) var_vals[sfk_CMD_NUM(xx_vnum)].changed

/*
 * a queue of packets
 */
struct sfk_queue_R {
	sfk_packet_pt sfk_first;	/*first available packet or nil*/
	sfk_packet_pt sfk_last;		/*last packet on queue or nil*/
	uint32 sfk_size;			/*number of packets on queue*/
};
typedef struct sfk_queue_R sfk_queue,*sfk_queue_pt;

/*
 * initialize the passed queue to be empty
 */
void sfk_init_queue(sfk_queue_pt aq);

/*
 * enqueue the passed packet in the passed queue
 */
void sfk_enqueue(sfk_queue_pt aq,sfk_packet_pt ap);

/*
 * remove a packet from the passed queue and return it
 * returns nil if the queue was empty
 */
void sfk_enqueue_protected(sfk_queue_pt aq,sfk_packet_pt ap);

/*
 * remove a packet from the passed queue and return it
 * returns nil if the queue was empty
 */
sfk_packet_pt sfk_dequeue(sfk_queue_pt aq);

/*
 * remove a packet from the passed queue and return it
 * returns nil if the queue was empty
 */
sfk_packet_pt sfk_dequeue_protected(sfk_queue_pt aq);

/*
 * number of write registers in a scc
 * (more or less, there is this 7 prime register...)
 */
#define NUM_SCC_REGS (16)

/*
 * port states
 */
enum px_states_e {
	PX_OFF,			/*0 port offline*/
	PX_RX,			/*1 RX mode*/
	PX_RX_NOW, 		/*2 RX of packet in progress*/
	PX_RX_CHUCK, 	/*3 RX but throw away data*/
	PX_SLOT_WAIT, 	/*4 waiting for dwait/slottime to expire*/
	PX_KEY_UP,		/*5 keying up, sending flags*/
	PX_XMIT,		/*6 transmiting*/
	PX_TAIL			/*7 sending trailer*/
};
typedef enum px_states_e px_states;

#ifdef RUBBISH_THIS_STUFF_COMES_FROM_8530_H
/*
 * names for scc registers
 */
enum sfk_scc_reg_e {
	R0,R1,R2,R3,R4,R5,R6,R7,R8,R9,R10,R11,R12,R13,R14,R15
};
typedef enum sfk_scc_reg_e sfk_scc_reg;
#endif

#define WANT_XMIT(xx_p) (xx_p->sfk_xmitq.sfk_size>0)

/*
 * sfk_circular_buffer
 */
struct sfk_circular_buffer_R {
	long cu_size;			/*size of buffer in characters*/
	long cu_read;			/*removal pointer*/
	long cu_write;			/*insertion pointer*/
	char *cu_buf;			/*pointer to buffer*/
};
typedef struct sfk_circular_buffer_R sfk_circular_buffer,*sfk_circular_buffer_pt;

/*
 * uninitialize a circular buffer, release any allocated space
 */
void sfk_cu_uninit(sfk_circular_buffer_pt cu);

/*
 * initialize a curcular buffer
 */
void sfk_cu_init(sfk_circular_buffer_pt cu,long max_size);

/*
 * write data
 */
long sfk_cu_write(sfk_circular_buffer_pt cu,char *put_me,long put_size);

/*
 * return max number of bytes that can be written
 */
long sfk_cu_write_size(sfk_circular_buffer_pt cu);

/*
 * return max number of bytes that can be read
 */
long sfk_cu_read_size(sfk_circular_buffer_pt cu);

/*
 * read data
 */
long sfk_cu_read(sfk_circular_buffer_pt cu,char *read_me,long read_size);

/*
 * read data stopping on break (control) characters
 */
long sfk_cu_read_line(sfk_circular_buffer_pt cu,char *read_me,long read_size);

/*
 * data for each kiss port (AIn, AOut, BIn or BOut)
 */
struct kiss_port_R {
	int				kp_state;	//read/write packet state
	int				kp_offset;	//data transfer offset
	int				kp_refnum;	//driver number
	int				kp_is_in;	//true if input device
	int				kp_transpose; //transpose next character
	unsigned char	kp_cmd;		//incomming command byte
	sfk_packet_pt 	kp;			//currently packet being transfered
	long 			kp_old;		//old dce
	unsigned char  *kp_name;	//name of driver
	DCtlPtr 		*kp_de;		//this ports dce dce
	IOParam 		*kp_pb;		//pending prime pb
	//kiss_in/out or soft_tnc_in/out
	short (*kp_io)(CntrlParam *pb,DCtlPtr de,short op,void *extra);
	sfk_circular_buffer io_buf;	//text io buffer
};
typedef struct kiss_port_R kiss_port,*kiss_port_pt;

//output port states
#define KRP_NP			0	//no packet, look for sync
#define KRP_SYNC		1	//in sync
#define KRP_WP			2	//write packet
#define KRP_READ_CMD	3	//recieving command byte

//define number of kiss ports
#define KISS_IN 	(0)
#define KISS_OUT 	(1)
#define NUM_KISS_PORTS (2)	//xin,xout

#define K_IN kports[KISS_IN]
#define K_OUT kports[KISS_OUT]

/*
 * port block can specify port a or b
 */
struct sfk_prt_R {
	int 			pnum;		/*port number of this port*/
	px_states		sfk_state;	/*current port state*/
	char 			*port_name;	/*text name of this port*/
	sfk_queue 		sfk_freehq;	/*queue of free packets headers*/
	sfk_queue 		sfk_freedq;	/*queue of free data packets*/
	sfk_queue 		sfk_xmitq;	/*queue of outgoing packets*/
	sfk_queue 		sfk_recvq;	/*queue of incomming packets*/
	sfk_packet_pt 	sfk_in;		/*currently arriving packet*/
	sfk_packet_pt 	sfk_out;	/*currently outgoing packet*/
	long in_count;				/*input packet size*/
	long out_count;				/*bytes left to send*/
	volatile uint8 	*ctl_rd;	/*pointers to scc registers*/
	volatile uint8 	*ctl_wr;
	volatile uint8 	*data_rd;
	volatile uint8 	*data_wr;
	sfk_value 		var_vals[sfk_max_cmd_num]; /*per port variables*/
	uint8 			scc_reg_cache[NUM_SCC_REGS];
	uint8 			extra_tx_bits_for_line_power; /*leave these on even in rx*/
	uint8			timer_installed;	//true if queue entry is installed
	uint8			timer_ignore;		//if it goes off ignore it
	TMTask			timer_task;		/*allow time for keup*/
	void			(*low_level_timer_routine)(); /*call this when the timer goes off*/
	void			(*timer_routine)(struct sfk_prt_R *p); /*call this when the timer goes off*/
#ifdef DBO_ENABLED
	dbo_FILE		dbo;			/*debugging output*/
	int				dbo_optr;		/*output pointer for interupt line*/
#endif
	void (*pr_recv_int)(struct sfk_prt_R *p);
	void (*pr_spcl_int)(struct sfk_prt_R *p);
	void (*pr_extr_int)(struct sfk_prt_R *p);
	int				cmd_port;
	int				cmd_did_prompt;
	int				cmd_buf_len;	/*length of command line*/
	int				cmd_buf_limit;	/*max size of command buffer*/
	char			*cmd_buf;		/*current input line in tnc mode*/
	/*kiss variables*/
	kiss_port		kports[NUM_KISS_PORTS];
};
typedef struct sfk_prt_R sfk_prt,*sfk_prt_pt;

#define sfk_SIZE_Lvl2 	(8)		/*size of level 2 interupt table*/
#define sfk_SIZE_Ext	(4)		/*size of External interupt table*/

/*
 * private state for debugging
 */
struct sfk_priv_state_R {
	sfk_prt skf_prt_table[sfk_NUM_PORTS];
	long magicno;
	/*while we have private interupt vectors save the system ones here*/
	ProcPtr saved_lvl2[sfk_SIZE_Lvl2];
	ProcPtr saved_ext[sfk_SIZE_Ext];
	Handle driver_template;		/*template to create drivers on the fly*/
	dyn_text dt_text;			/*dt text*/
	char vers[16];				/*version text*/
};
typedef struct sfk_priv_state_R sfk_priv_state,*sfk_priv_state_pt;

#define sfk_PN(xx_n) (&sfk_gl.skf_prt_table[xx_n])
#define sfk_PA sfk_PN(sfk_MODEM_PORT)
#define sfk_PB sfk_PN(sfk_PRINTER_PORT)

void sfk_reset_scc(sfk_prt_pt p);
void sfk_setup_scc(sfk_prt_pt p);

#define sfk_or_scc(xx_p,xx_rnum,xx_val) \
	sfk_write_scc(xx_p,(xx_rnum),((xx_p->scc_reg_cache[(xx_rnum)])|(xx_val)))

#define sfk_cand_scc(xx_p,xx_rnum,xx_val) \
	sfk_write_scc((xx_p),(xx_rnum),(((xx_p)->scc_reg_cache[(xx_rnum)])&(~(xx_val))))

#ifndef SCC_ASM_DEFS
void sfk_write_scc_data(sfk_prt_pt p,uint8 val);
uint8 sfk_read_scc_data(sfk_prt_pt p);
void sfk_write_scc(sfk_prt_pt p,uint8 rnum,uint8 val);
uint8 sfk_read_scc(sfk_prt_pt p,uint8 rnum);
#endif

/*
 * macro to declare that an argument is unused in a proceedure
 */
#ifdef CODECHECK
void reference_used(void *)
#define SFK_UNUSED_ARG(xx_arg) reference_used(&xx_arg)
#else
#define SFK_UNUSED_ARG(xx_arg)
#endif

/*
 * begin transmiting
 * call with interupts off
 */
void sfk_initiate_xmit(sfk_prt_pt p);

/*
 * reset highest interupt under service
 */
#define sfk_reset_scc_ius(xx_p) sfk_write_scc(xx_p,R0,RES_H_IUS)

/*
 * error returns from control routine
 */
#define sfk_MMCE_bad_maj	-1000	/*major version in set packet doesn't match driver*/
#define sfk_MMCE_noopen		-1001	/*driver wasn't properly opened*/

#define sfk_MIN_REMIND sfk_imax(((sfk_gl.cps.server_poll)+1*sfk_MINS+10*sfk_SECS),sfk_ABS_MIN_POLL)

void sfk_install_interupt_vectors(sfk_prt_pt p);
void sfk_remove_interupt_vectors(sfk_prt_pt p);

#ifndef sfk_CORE_EXTERN
#define sfk_CORE_EXTERN extern
#endif
sfk_CORE_EXTERN sfk_priv_state sfk_gl;

/*
 * delay between scc accesses where an increment of a 32bit unsigned
 * long is also done between accesses
 */
#define sfk_SCC_COUNT_DELAY

/*
 * parse state
 */
#define sfk_MAX_TOKEN (260)
struct sfk_parse_rec_R {
	sfk_prt_pt p;
	char ch;		/*current character*/
	int eof;
};
typedef struct sfk_parse_rec_R sfk_parse_rec,*sfk_parse_rec_pt;

/*
 * internal io record
 * contains user io record, error throw
 * and per call globals so driver is reentrant
 */
struct sfk_iio_R {
	sfk_io_record_pt uio;	/*users io record*/
	jmp_buf err_throw;		/*throw errors to here to exit control call*/
	/*per command per control call variables*/
	sfk_parse_rec pqx;		/*parse state*/
};
typedef struct sfk_iio_R sfk_iio,*sfk_iio_pt;

/*
 * a control call to the us has failed
 * unwind the stack and return the error
 */
void sfk_control_fail(sfk_iio_pt iicmd,short io_err,short iio_err);

/*
 * initilize the command table
 */
void sfk_init_commands(sfk_iio_pt cmd);

/*
 * parse and execute the passed command
 */
void sfk_parse_command(sfk_iio_pt cmd);

/*
 * read a packet from recieve queues
 */
void sfk_read(sfk_iio_pt cmd);

/*
 * enqueue a packet in a write queue
 */
void sfk_write(sfk_iio_pt cmd);

/*
 * append a c string onto the end of a sfk_string
 * don't overflow the output string
 */
void sfk_put_string(sfk_string_pt astr,char *add_on);

/*
 * handle control calls to the driver
 */
int sfk_control(sfk_io_record_pt pb);

/*
 * handle open calls to the driver
 */
int sfk_init(void);

/*
 * handle close calls to the driver
 */
int sfk_uninit(sfk_io_record_pt pb);

/*
 * perform periodic actions
 */
void sfk_tick(sfk_iio_pt cmd);

/*
 * append a string onto command output
 */
void sfk_cmd_out(sfk_iio_pt cmd,char *add_on);

/*
 * go online or go offline
 */
void sfk_change_online(sfk_iio_pt cmd,int new_setting);

/*
 * map a failure code to it's text
 */
char *sfk_map_text(int text_code);

/*
 * report a command failure
 */
void sfk_parse_fail(sfk_iio_pt cmd,int fail_code,char *extra);

/*
 * patch each interupt routine to known where our data lives
 */
void sfk_patch_in_global_data(void);

/*
 * some macs have a data and code cache that needs to be flushed
 * after patching code
 */
void sfk_some_flush(void);

/*
 * setup pointers to registers
 */
void sfk_init_register_addresses(sfk_prt_pt p);

/*
 * set divider for proper rate to transmit
 * need to wait one underflow to pick up proper rate
 */
void sfk_scc_set_buad_divider_xmit(sfk_prt_pt p);

/*
 * set divider for proper rate to recieve
 * need to wait one underflow to pick up proper rate
 */
void sfk_scc_set_buad_divider_recv(sfk_prt_pt p);

/*
 * shutdown everything
 */
void sfk_shutdown(sfk_iio_pt cmd);

/*
 * handle a transition to a new state
 */
void sfk_go_state(sfk_prt_pt p,px_states new_state);

/*
 * set interupts and buad rate for recieve mode
 */
void sfk_set_recv_mode(sfk_prt_pt p);

/*
 * transition to actually recieving a packet
 */
void sfk_set_recv_now_mode(sfk_prt_pt p);

/*
 * transition to actually recieve packet but throw away data
 */
void sfk_set_recv_chuck_mode(sfk_prt_pt p);

/*
 * keyup and schedule start of transmit
 */
void sfk_keyup(sfk_prt_pt p,long delay_time);

/*
 * put scc in xmit mode and schedule transmit
 */
void sfk_tell_scc_to_xmit(sfk_prt_pt p);

/*
 * end of keyup time so clear keyup timer
 */
void sfk_clear_timer(sfk_prt_pt p);

/*
 * force keydown
 */
void sfk_keydown(sfk_prt_pt p);

/*
 * set the timer to call to the specified routine after delay
 * delay is in milliseconds if posative or microseconds if negative
 */
void sfk_install_timer(sfk_prt_pt p,void (*rtn)(sfk_prt_pt p),long delay);

/*
 * start transmiting
 */
void sfk_send_first_byte(sfk_prt_pt p);

/*
 * set state to allow crc/sync char to go out
 */
void sfk_tail(sfk_prt_pt p);

/*
 * prepare the sfk_in packet to be ready to start recieving
 */
void sfk_prep_rx(sfk_prt_pt);

/*
 * change kiss mode to on or off
 */
void sfk_change_kiss(sfk_iio_pt cmd,int new_setting);

/*
 * turn off kiss mode on the passed port
 */
void sfk_change_soft_tnc(sfk_iio_pt cmd,int new_setting);

/*
 * mark the waiting iopb as finished
 */
void sfk_mark_read_done(sfk_prt_pt p,int er_code);

/*
 * queue up a packet for write and start transmit if it isn't
 * already running
 */
void sfk_enqueue_write(sfk_prt_pt p,sfk_packet_pt pak);

/*
 * kiss_note_offline - tell kiss that the port went offline
 */
void kiss_note_offline(sfk_prt_pt p);

/*
 * entry point of softkiss packet driver
 * This entry is for softkiss itself.
 * The fake serial entrypoints are in sfk_core_kiss.c
 */
short sfk_driver(CntrlParam *pb,DCtlPtr de,short op,void *extra);

/*
 * find the softkiss driver
 * if it is not present, install it
 */
short sfk_drvr_find_or_install(short *ref_num);

/*
 * see if eithor serial port is in use
 */
int is_serial_busy(sfk_prt_pt p);

/*
 * install one driver, kiss_in or kiss_out
 */
void sfk_fake_serial_install(
	sfk_iio_pt cmd,
	sfk_prt_pt p,
	kiss_port_pt k_in,
	long entry_point_in,
	kiss_port_pt k_out,
	long entry_point_out);


/*
 * enable the fake serial drivers
 */
void fake_serial_on(sfk_iio_pt cmd,sfk_prt_pt p);

/*
 * turn off kiss mode on the passed port
 */
void fake_serial_off(sfk_prt_pt p);

/*
 * install soft_tnc as the current fake serial driver
 */
void sfk_soft_tnc_install(sfk_iio_pt cmd,sfk_prt_pt p);

/*
 * name of driver,ie pass this to OpenDriver
 */
#define sfk_driver_name "\p.ax25_packet"

/*
 * allocate some memory
 */
void *sfk_malloc(unsigned long msize);

/*
 * free some memory
 */
void sfk_free(void *buf);

/*
 * allocate the storage that will become the packet queues
 */
void sfk_allocate_queue_memory(sfk_iio_pt cmd,sfk_prt_pt p);

/*
 * allocate memory for soft_tnc
 */
void sfk_allocate_soft_tnc_memory(sfk_iio_pt cmd,sfk_prt_pt p);

/*
 * allocate a packet header and packet
 */
sfk_packet_pt sfk_allocate_packet(sfk_prt_pt p);

/*
 * allocate a packet header and packet
 * may be called with interupts on
 */
sfk_packet_pt sfk_allocate_protected(sfk_prt_pt p);

/*
 * free a packet header and packet
 */
void sfk_free_packet(sfk_prt_pt p,sfk_packet_pt free_me);

/*
 * free a packet header and packet
 * may be called with interupts on
 */
void sfk_free_packet_protected(sfk_prt_pt p,sfk_packet_pt free_me);

/*
 * copy a packet header and share a packet
 */
sfk_packet_pt sfk_copy_packet(sfk_prt_pt p,sfk_packet_pt copy_me);

/*
 * mark this driver as being open
 * fail if it is already open
 */
int sfk_open_me(DCtlPtr de);

/*
 * mark this driver as being closed
 */
void sfk_close_me(DCtlPtr de);

/*
 * recompute values that depend on other variables
 */
void sfk_recompute_derived_nums(sfk_prt_pt p);

/*
 * set the interface offline
 */
void sfk_go_offline(sfk_prt_pt p);

/*
 * return command table entry to dump to resource fork
 * used in test program
 */
int sfk_cmd_dump(int idx,char **nm_text,char **nm_help,
	char **nm_default,long *nm_flags,short *nm_val_num);

/*
 * is one time startup init done
 */
int sfk_init_done(void);

