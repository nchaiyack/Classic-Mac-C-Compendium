/*
 * tcp/ip io glue
 */

#pragma once

#include "MacTCPCommonTypes.h"
#include "TCPPB.h"
#include <Processes.h>

#define TCP_FINGER_PORT (79)
	
/*
 * an asyncronous io request
 */
struct side_R {
    TCPiopb 	ipb;    			/*io parameters for current io */
	struct tcp_conv_R *sconv;		/*conversation block for this io*/
    wdsEntry   	awds[2];
};
typedef struct side_R tcp_side, *side_pt;

#define MACTCP_RECV_BUFFER_LEN 	(4500)
#define MACTCP_SEND_BUFFER_LEN	(6500)
#define	SEND_TIMEOUT			(90)	//seconds to time out a send
#define MAX_NUM_CONVS 			(3)

#define ST_FREE		(0)
#define ST_LISTEN	(1)
#define ST_SEND		(2)
#define ST_CLOSE	(3)

/*
 * set to false to quit server
 */
extern int sip_keep_running;

#define TCP_WAKE_MAGIC_NUM ('AdG3')

/*
 * info used at interupt time to wake a process
 */
struct process_wakeup_control_R {
	int 			need_wake_up;
	int 			can_sleep;
    ProcessSerialNumber	server;
    Ptr				a5;
    long			tcp_wake_magic_num;
};
typedef struct process_wakeup_control_R process_wakeup_control,*process_wakeup_control_pt;

/*
 * one connection
 */
struct tcp_conv_R {
	int			state;
	int			service_me;
	process_wakeup_control_pt server_proc;
	char		mactcp_recv_buffer[MACTCP_RECV_BUFFER_LEN];
	char		mactcp_send_buffer[MACTCP_SEND_BUFFER_LEN];
	int			closing;
	int			terminating;
	int			data_arrived;
    StreamPtr	tcpStream;
    tcp_side	write_side;
    int			last_event;
};
typedef struct tcp_conv_R tcp_conv, *tcp_conv_pt;

/*
 * handle action on each tcp connection
 */
void tcp_service_event(void);

/*
 * clean up all the streams
 */
void tcp_release_streams(void);

/*
 * setup all the streams
 */
int tcp_init_streams(void);

/*
 * release a tcp stream, aborting any io in progress
 */
void tcp_release_connection(tcp_conv_pt sconv);

/*
 * open the mactcp device driver
 * returns true on an error
 */
int tcp_open_driver(void);

/*
 * did any new events happens since we last awoke?
 */
int tcp_ok_to_sleep_now(void);

/*
 * create a tcp stream
 */
int tcp_create_stream(tcp_conv_pt uptr);

/*
 * close a tcp stream
 */
int tcp_close(tcp_conv_pt uptr);

/*
 * just woke up and will service any requests
 */
void tcp_just_awoke(void);

/*
 * transfer outgoing packets into MacTCP output buffer
 */
int tcp_send_packet(tcp_conv_pt sconv);

/*
 * recieve data on this connection
 * the data is left in an internal mactcp buffer which must be returned to mactcp
 * with the returnTCPbuffer proceedure
 */
int tcp_recv_packet(tcp_conv_pt sconv);

/*
 * passive connection to listen for incomming tcp connections
 */
int tcp_passive_connect(tcp_conv_pt sconv,tcp_port port_number);

/*
 * allocate io buffers
 * return true on error
 */
int tcp_allocate_memory(void);

/*
 * setup one stream
 */
int tcp_init_stream(tcp_conv_pt sconv);


