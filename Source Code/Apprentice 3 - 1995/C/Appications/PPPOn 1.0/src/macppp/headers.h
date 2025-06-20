/* headers.h --  TCP/IP header structures
 *
 * Copyright 1992-1993 Merit Network, Inc. and The Regents of the
 *  University of Michigan.  Usage of this source code is restricted
 *  to non-profit, non-commercial purposes.  The source is provided
 *  "as-is", without warranty.
 */

#define TCP_PROTOCOL 6
/* ip flags */
#define IP_MORE_FRAG 0x2000
#define IP_DONT_FRAG 0x4000
/* tcp flags */
#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PUSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20
/*  header lengths */
#define IPHLEN 20
#define TCPHLEN 20
#define MAXIPOPTLEN 40
#define MAXTCPOPTLEN 40
/* field offsets for looking into packet */
#define IP_PROTO_OFFSET 9
#define IP_OFFSET_OFFSET 6
#define IP_SOURCE_OFFSET 12
#define IP_DEST_OFFSET 16
#define TCP_WINDOW_OFFSET 14
#define TCP_CHECKSUM_OFFSET 16

struct ipheader {
	b_8		version;	/* version and header length */
	b_8		tos;
	b_16	length;
	b_16	id;
	b_16	offset;
	b_8		ttl;
	b_8		protocol;
	b_16	checksum;
	b_32	source_addr;
	b_32	dest_addr;
	b_8		options[MAXIPOPTLEN];		/* options baggage */
};

struct tcpheader {
	b_16	source_port;
	b_16	dest_port;
	b_32	sequence;
	b_32	acknowledge;
	b_8		offset;
	b_8		flags;
	b_16	window;
	b_16	checksum;
	b_16	urgent_pointer;
	b_8		options[MAXTCPOPTLEN];
};

/* tcp flags */
#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RESET 0x04
#define TCP_PUSH 0x08
#define TCP_ACK 0x10
#define TCP_URGENT 0x20