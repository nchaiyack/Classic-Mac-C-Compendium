/*
 * SoftKiss
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#pragma once

#ifndef sfk_ABSOULTE_SIZE_INTS_DEFINED
#include "sfk_types.h"
#endif

/*
 * driver major and minor version numbers
 * send it these number from when your program was compiled
 * if it is too new or too old to emulate the version you
 * ask for it will fail to let you attach to a port
 */
#define sfk_ST_maj_ver (17)
#define sfk_ST_min_ver (23)


/*
 * data for reading/writing a packet
 * on write cnt bytes are written
 * on read cnt is the size of the buffer available it is updated
 * to be the count of actual bytes available
 */
struct sfk_packet_R {
	struct sfk_packet_R *next; /*used internaly, must be first*/
#define SFK_ufirst 		(1)	/*this read call was the first to return this packet*/
#define sfk_db_SHORT	(2)	/*packet is too short to be leagle*/
#define sfk_db_LONG		(4)	/*packet is too long to fit in queue or recieve*/
#define sfk_db_CRC		(8)	/*packet had bad CRC*/
#define sfk_db_BAD_AX25	(16) /*ax25 header is bad*/
#define sfk_db_BAD		(32) /*something bad*/
#define sfk_db_OVERRUN	(64) /*someone had interupts off too long*/
#define sfk_db_DIBS		(128) /*a different attachment claimed this packet*/
	uint32 data_flags;	/*[O]per packet flags*/
	uint32 sfk_time_in;	/*[O]TickCount() when packet came in*/
	uint8  portno;		/*port data comes in/out*/
	uint8  sfk_xx_pad0;	/*reserved for expansion*/
	uint16 cnt;			/*size of data to read/write*/
	uint16 att_read;	/*attachments that handled this packet*/
	uint8 *data;		/*[I]pointer to data to read/write*/
};
typedef struct sfk_packet_R sfk_packet,*sfk_packet_pt;

/*
 * well known packet ports
 */
#define sfk_PRINTER_PORT 	(0)
#define sfk_MODEM_PORT 		(1)
#define sfk_NUM_MAGIC_PORTS (2)

/*
 * ports after last well known are dynamicly assigned
 */
#define sfk_NUM_PORTS		(2)

/*
 * maximum length of short string variables (callsigns) including nulls
 */
#define	sfk_STRING_SIZE	(16)

/*
 * possible commands to driver
 */
#define sfk_CMD_goodbye	  (-1)	/*goodbye kiss, the heap is about to be initilized*/
 					/*   0	  unused*/
#define	sfk_CMD_killio	(1)		/*killio does this control entry to stop io*/
#define sfk_CMD_run		(65)	/*run periodic functions*/

#define sfk_CMD_base	(100)	/*first command with extended error info*/
#define sfk_CMD_attach	(100)	/*start useing ports*/
#define sfk_CMD_detach	(101)	/*stop using ports*/
#define sfk_CMD_msg 	(102) 	/*send driver a command message*/
#define sfk_CMD_write	(103)	/*send a packet*/
#define sfk_CMD_read	(104)	/*read a packet*/

/*
 * common header record for all softkiss driver calls
 * I - input only not modified
 * O - output only, contents on call in don't matter
 * IO - driver both reads then updates this field
 */
struct sfk_CMD_header_R {
	uint32 att_num;		/*[IO]attachment port number*/
	int16 sys_err_code;	/*[O]sytem error code, or zero on success*/
	int16 sfk_err_code;	/*[O]sfk error code or zero on success*/
};
typedef struct sfk_CMD_header_R sfk_CMD_header,*sfk_CMD_header_pt;

/*
 * string passed to driver for the driver to write to
 */
struct sfk_string_R {
	int32 sfk_in_cnt;		/*size of input string*/
	int32 sfk_out_cnt;		/*size used in output*/
	char *sfk_str;			/*pointer to data*/
};
typedef struct sfk_string_R sfk_string,*sfk_string_pt;

/*
 * text commands to send to SoftKiss and it's reply
 */
struct sfk_ARG_msg_R {
	int portno;			/*[IO]default port*/
	sfk_string imsg;	/*input message to sfk*/
	sfk_string omsg;	/*output message from sfk*/
};
typedef struct sfk_ARG_msg_R sfk_ARG_msg,*sfk_ARG_msg_pt;

/*
 * notify proc is called as follows
 */
#define sfk_NT_online 	(1)	/*port just came online*/
#define sfk_NT_offline 	(2) /*port just went offline*/
#define sfk_NT_read	  	(3)	/*here is a packet to read*/
#define sfk_NT_ok_write (4) /*output write queue nearly empty, queue more writes now*/

typedef (*sfk_notify_proc)(int16 sfk_NT_cmd,int16 portno,sfk_packet_pt pkt);

/*
 * input data to attach to a port
 */
struct sfk_ARG_attach_R {
	sfk_notify_proc *notify_proc; 	/*[I]routine to call for async notification*/
	char *my_name;					/*[I]description of what this program is/does*/
	void *attachment_private_vars;	/*[I]pass this to notify proc*/
};
typedef struct sfk_ARG_attach_R sfk_ARG_attach,*sfk_ARG_attach_pt;

/*
 * there is no input data to detach from a port, just the standard header
 */

typedef int sfk_port_state;
struct sfk_command_record_R {
	sfk_CMD_header 	sfk_hdr;	/*header common to all commands*/
	union {
		sfk_ARG_attach  sfk_attach; /*attach arguments*/
		sfk_packet   	sfk_rw;		/*data for read/write*/
		sfk_ARG_msg		sfk_msg;	/*command message*/
	}cknd;							/*depends on command kind*/
};
typedef struct sfk_command_record_R sfk_command_record,*sfk_command_record_pt;

/*
 * io manager control parameter block to pass to Control
 */
struct sfk_io_record_R {
	void		*qLink;
	short		qType;
	short		ioTrap;
	void		*ioCmdAddr;
	void		*ioCompletion;
	short		ioResult;
	void		*ioNamePtr;
	short		ioVRefNum;
	short		ioRefNum;
	short		csCode;
	void 		*private_state;	/*returned with pointer to drvr private vars*/	
	sfk_command_record	do_this;	/*state to set, current state returned*/
};
typedef struct sfk_io_record_R sfk_io_record,*sfk_io_record_pt;

/*
 * shortest legal packet not including CRC
 */
#define SFK_MIN_PACK_SIZE (15)

/*
 * sfk error codes
 */
#define sfk_ERR_ISERR		0x020

#define sfk_ERR_NOERRR		(0)	/*no error*/
#define sfk_ERR_TOO_OLD		(1)	/*driver to old to talk to server*/
#define sfk_ERR_BAD_BAUD	(2)	/*zero or perposterious baud rate*/
#define sfk_ERR_BAD_PARSE	(3)	/*parse of message to driver failed*/
#define sfk_NOTHING_TO_READ	(4)	/*no packets available to read*/
 /*driver understands the request but the code to do it isnt written
  *yet.
  */
#define sfk_NOT_YET_IMPLEMENTED (5) 
#define sfk_NO_SUCH_PORT	(5)	/*no such port online*/

/*
 * initialize a parameter block for a control coll
 */
#define sfk_INIT_CPB(xx_pb,xx_ref,xx_cscode) \
  do {(xx_pb).ioCompletion=0; \
   (xx_pb).ioVRefNum=0; \
   (xx_pb).ioRefNum=(xx_ref); \
   (xx_pb).csCode=(xx_cscode); \
  } while(0)

/*
 * setup an io record to send the driver a message
 */
void sfk_parse_from(sfk_io_record_pt cmd,
	char *in_str,
	int in_len,
	char *out_str,
	int out_len,
	int def_port);

/*
 * when we want to transmit we wait till the channel is clear
 * then generate a random number between 0 and PERSISTANCE_LIMIT-1
 * if the number we generate is less than or equal to the xmit_persist
 * value we transmit if not we try again
 */
#define PERSISTANCE_LIMIT (1000L)