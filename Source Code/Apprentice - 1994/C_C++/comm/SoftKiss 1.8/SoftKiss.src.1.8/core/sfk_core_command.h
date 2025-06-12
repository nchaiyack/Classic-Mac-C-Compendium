/*
 * SoftKiss driver commands
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 * list of commands and help text
 * include this list with the macro SFK_CMD defined in various
 * ways in order to do various things
 */

#define SFK_NUM_TEXT (4)	/*number of text variables*/

/*
 * if SKF_CMD isn't defined to do something else, default to
 * defineing a enum list of commands
 */
#ifndef SFK_CMD
#define SFK_CMD(xx_cmd_name,xx_cmd_arg_type,xx_cmd_help,xx_default) \
	sfk_cval_##xx_cmd_name,

#define SFK_CMD_UNDEFINE_SFK_CMD
enum sfk_cmd_t {
#endif

  SFK_CMD(just_me,SFK_CF_BOOL,
    "ignore packets not addressed to mycall or digipeat callsigns NYI","0")
  SFK_CMD(mycall,SFK_CF_TEXT,
  	"callsign to listen for connects from NYI",0)
  SFK_CMD(digipeat_id,SFK_CF_TEXT,
  	"if recieved packet wants to digipeat thru this id send it out this port NYI","*-10")

  SFK_CMD(xmit_slottime,SFK_CF_INT,
  	"if xmit_persist says wait wait this many usecs","90000")
  SFK_CMD(os_xmit_slottime,SFK_CF_INT|SFK_CF_RO,
 	"xmit_slottime in -usecs or +ms depending on what time mgr supports","1")
  SFK_CMD(xmit_persist,SFK_CF_INT,
  	"P*1000, for 75% chance to transmit use 750 ","750")
  SFK_CMD(xmit_dwait,SFK_CF_INT,
  	"delay in usec before transmiting non digipeated packets","0")
  SFK_CMD(os_xmit_dwait,SFK_CF_INT|SFK_CF_RO,
 	"xmit_dwait in -usecs or +ms depending on what time mgr supports","1")

  SFK_CMD(debug_level,SFK_CF_INT|SFK_CF_IMMEDIATE,
  	"zero for normal, larger numbers for more debugging info","0")
  SFK_CMD(display,SFK_CF_TEXT|SFK_CF_IMMEDIATE,
  	"display a counter",0)
  SFK_CMD(help,SFK_CF_TEXT|SFK_CF_IMMEDIATE,
  	"help -- for command list or help <command> for help on command",0)
  SFK_CMD(shutdown,SFK_CF_TEXT|SFK_CF_IMMEDIATE,
  	"shutdown -- put all interfaces offline, release all devices/buffers",0)
  SFK_CMD(max_packet_size,SFK_CF_INT,
  	"largest packet to recieve/xmit including FCS bytes","520")
  SFK_CMD(online,SFK_CF_BOOL|SFK_CF_IMMEDIATE,
    "set to true to start operating with current settings","0")
  SFK_CMD(soft_tnc,SFK_CF_BOOL|SFK_CF_IMMEDIATE,
    "set to true to replace serial port driver with Softkiss driver","0")
  SFK_CMD(ignore_internal_modem,SFK_CF_BOOL,
    "set to true to ignore internal modem and use serial port","1")
  SFK_CMD(line_powered_modem,SFK_CF_BOOL,
    "set to true to leave transmit driver power on during recieve","1")
  SFK_CMD(port,SFK_CF_TEXT|SFK_CF_IMMEDIATE,"select port to talk to 0,1,... or modem,printer,loopback",0)
  SFK_CMD(header_queue_size,SFK_CF_INT,
  	"packet header queue size in number of packets, shared between recieve and transmit","120")
  SFK_CMD(data_queue_size,SFK_CF_INT,
  	"packet data queue size in number of packets, shared between recieve and transmit","40")
  SFK_CMD(recv_speed,SFK_CF_INT,"buadrate to use when recieving","1200")
  SFK_CMD(xmit_speed,SFK_CF_INT,"buadrate to use when recieving","1200")
  SFK_CMD(data_clock_rate,SFK_CF_INT,
 	"data clock used to compute baud rate dividers","3672000")

  /*
   * counters used internaly, read only
   */
  SFK_CMD(xmit_rate_divider,SFK_CF_INT|SFK_CF_RO,
 	"transmit baudrate divider",0)
  SFK_CMD(recv_rate_divider,SFK_CF_INT|SFK_CF_RO,
 	"receive baudrate divider",0)
  SFK_CMD(xmit_interupt,SFK_CF_INT|SFK_CF_RO,
 	"number of transmit buffer empty interupts",0)
  SFK_CMD(data_interupt,SFK_CF_INT|SFK_CF_RO,
 	"number of read character interupts",0)
  SFK_CMD(special_interupt,SFK_CF_INT|SFK_CF_RO,
 	"number of scc special interupts",0)
  SFK_CMD(external_interupt,SFK_CF_INT|SFK_CF_RO,
 	"number of scc external interupts",0)
  SFK_CMD(scc_ctl_reads,SFK_CF_INT|SFK_CF_RO,
 	"number of reads of scc control register",0)
  SFK_CMD(scc_ctl_writes,SFK_CF_INT|SFK_CF_RO,
 	"number of writes of scc control register",0)
  SFK_CMD(scc_data_reads,SFK_CF_INT|SFK_CF_RO,
 	"number of reads of scc data register",0)
  SFK_CMD(scc_data_writes,SFK_CF_INT|SFK_CF_RO,
 	"number of writes of scc data register",0)
  SFK_CMD(our_vectors,SFK_CF_BOOL|SFK_CF_RO,
 	"softkiss interupt vectors are installed",0)
  SFK_CMD(recv_overrun,SFK_CF_INT|SFK_CF_RO,
 	"number of times someone had interupts off too long and we lost a packet",0)
  SFK_CMD(queue_mem,SFK_CF_INT|SFK_CF_RO|SFK_CF_HEX,
 	"address of memory block allocated for packet queue",0)
  SFK_CMD(queue_no_swap,SFK_CF_BOOL|SFK_CF_RO,
 	"if true virtual memory is on and the queue is locked in memory",0)
  SFK_CMD(queue_mem_phys_size,SFK_CF_INT|SFK_CF_RO,
 	"size of queue in bytes",0)
  SFK_CMD(packet_phys_size,SFK_CF_INT|SFK_CF_RO,
 	"size of a packet including header and padding",0)
  SFK_CMD(recv_largest_seen,SFK_CF_INT|SFK_CF_RO,
 	"if max_packet_size where this big no packets whould have gotten chopped",0)

  SFK_CMD(tx_delay,SFK_CF_INT,
 	"delay in usec for transmiter to keyup","400000")
  SFK_CMD(os_tx_delay,SFK_CF_INT|SFK_CF_RO,
 	"tx_delay in -usecs or +ms depending on what time mgr supports","1")
  SFK_CMD(force_port,SFK_CF_BOOL,
  	"true to use the port even if it looks busy","0")
  SFK_CMD(tail_delay,SFK_CF_INT,
 	"delay in usec after last data byte to send crc and flags","50000")
  SFK_CMD(os_tail_delay,SFK_CF_INT|SFK_CF_RO,
 	"tail_delay in -usecs or +ms depending on what time mgr supports","1")
  SFK_CMD(state_var,SFK_CF_INT|SFK_CF_RO,
 	"internal state 0-off 1-rx 2-rx now 3-rx chuck 4 slow wait 5-keyup 6-xmit 7-tail","0")
  SFK_CMD(kiss,SFK_CF_BOOL|SFK_CF_IMMEDIATE,
    "set to true to enable kiss mode on current port","0")
  SFK_CMD(xmit_overrun,SFK_CF_INT|SFK_CF_RO,
 	"times interupts off too long during packet exmit","0")

/*
 * if we defined SFK_CMD to make the enum list then cleanup
 */
#ifdef SFK_CMD_UNDEFINE_SFK_CMD
#undef SFK_CMD_UNDEFINE_SFK_CMD
#undef SFK_CMD
  sfk_max_cmd_num
}; /*end enum*/
#endif
