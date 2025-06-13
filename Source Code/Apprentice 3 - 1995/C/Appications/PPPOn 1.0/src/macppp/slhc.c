/*
 * Routines to compress and uncompress tcp packets (for transmission
 * over low speed serial lines.
 *
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	Van Jacobson (van@helios.ee.lbl.gov), Dec 31, 1989:
 *	- Initial distribution.
 *
 * modified for KA9Q Internet Software Package by
 * Katie Stevens (dkstevens@ucdavis.edu)
 * University of California, Davis
 * Computing Services
 *	- 01-31-90	initial adaptation (from 1.19)
 *	PPP.05	02-15-90 [ks]
 *	PPP.08	05-02-90 [ks]	use PPP protocol field to signal compression
 *	PPP.15	09-90	 [ks]	improve mbuf handling
 *	PPP.16	11-02	 [karn]	substantially rewritten to use NOS facilities
 *
 *	- Feb 1991	Bill_Simpson@um.cc.umich.edu
 *			variable number of conversation slots
 *			allow zero or one slots
 *			separate routines
 *			status display
 *
 *  1992-93  Modifications for MacPPP.
 *				-Larry Blunk, Merit Network, Inc./ University of Michigan
 */

#include "ppp.h"
#include "slhc.h"

/* Initialize compression data structure
 *	slots must be in range 0 to 255 (zero meaning no compression)
 */
void
slhc_init( LapInfo *lap, short rslots, short tslots )
{
	b_16 i;
	struct cstate *ts;

	if ( rslots > 0  &&  rslots <= MAXSLOTS ) {
		lap->comp.rstate = lap->rcvslots;
		lap->comp.rslot_limit = rslots - 1;
	}

	if ( tslots > 0  &&  tslots <= MAXSLOTS ) {
		lap->comp.tstate = lap->txslots;
		lap->comp.tslot_limit = tslots - 1;
	}

	lap->comp.xmit_oldest = 0;
	lap->comp.xmit_current = lap->comp.recv_current = 255;

	if ( tslots > 0 ) {
		ts = lap->comp.tstate;
		for(i = lap->comp.tslot_limit; i > 0; --i){
			ts[i].this = i;
			ts[i].next = &(ts[i - 1]);
		}
		ts[0].next = &(ts[lap->comp.tslot_limit]);
		ts[0].this = 0;
	}
}

/* Encode a number */
static b_8 *
encode(b_8 *cp, b_16 n)
{
	if(n >= 256 || n == 0){
		*cp++ = 0;
		*cp++ = n >> 8;
	}
	*cp++ = n;
	return cp;
}

/* Decode a number */
static long
decode(struct bufheader *bufptr)
{
	short x;

	x = yankbyte(bufptr);
	if (x == 0) {
		return yank16(bufptr);	/* yank16 returns -1 on error */
	} else {
		return (long)x;		/* -1 if error */
	}
}

short
slhc_compress(LapInfo *lap, struct bufheader *bufptr, short compress_cid)
{
	struct slcompress *comp = &(lap->comp);
	struct cstate *ocs = &(comp->tstate[comp->xmit_oldest]);
	struct cstate *lcs = ocs;
	struct cstate *cs = lcs->next;
	b_16 hiplen, htcplen, hlen;
	b_16 ipoptlen, tcpoptlen;
	struct tcpheader *oth;
	unsigned long deltaS, deltaA;
	b_16 changes = 0;
	b_8 new_seq[16];
	b_8 *cp = new_seq;
	struct tcpheader th;
	struct ipheader iph;

	/* Extract IP header */
	hiplen = getipheader(&iph,bufptr);
	/* Bail if this packet isn't TCP, or is an IP fragment */
	if(iph.protocol != TCP_PROTOCOL || (iph.offset & 0x3fff) != 0 ){
		bufptr->dataptr -= hiplen;
		bufptr->length += hiplen;
		return SL_TYPE_IP;
	}
	/* Extract TCP header */
	htcplen = gettcpheader(&th,bufptr);
	hlen = hiplen + htcplen;

	/*  Bail if the TCP packet isn't `compressible' (i.e., ACK isn't set or
	 *  some other control bit is set).
	 */
	if((th.flags & ( TCP_FIN | TCP_SYN | TCP_ACK | TCP_RST )) ^ TCP_ACK ){
		/* TCP connection stuff; send as regular IP */
		bufptr->dataptr -= hlen;
		bufptr->length += hlen;
		return SL_TYPE_IP;
	}
	/*
	 * Packet is compressible -- we're going to send either a
	 * COMPRESSED_TCP or UNCOMPRESSED_TCP packet.  Either way,
	 * we need to locate (or create) the connection state.
	 *
	 * States are kept in a circularly linked list with
	 * xmit_oldest pointing to the end of the list.  The
	 * list is kept in lru order by moving a state to the
	 * head of the list whenever it is referenced.  Since
	 * the list is short and, empirically, the connection
	 * we want is almost always near the front, we locate
	 * states via linear search.  If we don't find a state
	 * for the datagram, the oldest state is (re-)used.
	 */
	for ( ; ; ) {
		if( iph.source_addr == cs->cs_ip.source_addr
		 && iph.dest_addr == cs->cs_ip.dest_addr
		 && th.source_port == cs->cs_tcp.source_port
		 && th.dest_port == cs->cs_tcp.dest_port)
			goto found;

		/* if current equal oldest, at end of list */
		if ( cs == ocs )
			break;
		lcs = cs;
		cs = cs->next;
	};
	/*
	 * Didn't find it -- re-use oldest cstate.  Send an
	 * uncompressed packet that tells the other side what
	 * connection number we're using for this conversation.
	 *
	 * Note that since the state list is circular, the oldest
	 * state points to the newest and we only need to set
	 * xmit_oldest to update the lru linkage.
	 */
	comp->xmit_oldest = lcs->this;

	goto uncompressed;

found:
	/*
	 * Found it -- move to the front on the connection list.
	 */
	if(lcs == ocs) {
		/* found at most recently used */
	} else if (cs == ocs) {
		/* found at least recently used */
		comp->xmit_oldest = lcs->this;
	} else {
		/* more than 2 elements */
		lcs->next = cs->next;
		cs->next = ocs->next;
		ocs->next = cs;
	}

	/*
	 * Make sure that only what we expect to change changed.
	 * Check the following:
	 * IP protocol version, header length & type of service.
	 * The "Don't fragment" bit.
	 * The time-to-live field.
	 * The TCP header length.
	 * IP options, if any.
	 * TCP options, if any.
	 * If any of these things are different between the previous &
	 * current datagram, we send the current datagram `uncompressed'.
	 */
	oth = &cs->cs_tcp;

	ipoptlen = hiplen - IPHLEN;
	tcpoptlen = htcplen - TCPHLEN;
	if (iph.version != cs->cs_ip.version
	 || iph.tos != cs->cs_ip.tos
	 || (iph.offset ^ cs->cs_ip.offset) & IP_DONT_FRAG
	 || iph.ttl != cs->cs_ip.ttl
	 || (th.offset ^ cs->cs_tcp.offset) & 0xf0
	 || (ipoptlen > 0 && bytecmp(iph.options,cs->cs_ip.options,
	 					ipoptlen) != 0)
	 || (tcpoptlen > 0 && bytecmp(th.options,cs->cs_tcp.options,
	 			tcpoptlen) != 0)){
		goto uncompressed;
	}
	/*
	 * Figure out which of the changing fields changed.  The
	 * receiver expects changes in the order: urgent, window,
	 * ack, seq (the order minimizes the number of temporaries
	 * needed in this section of code).
	 */
	if(th.flags & TCP_URG){
		deltaS = th.urgent_pointer;
		cp = encode(cp,deltaS);
		changes |= NEW_U;
	} else if (th.urgent_pointer != oth->urgent_pointer){
		/* argh! URG not set but urp changed -- a sensible
		 * implementation should never do this but RFC793
		 * doesn't prohibit the change so we have to deal
		 * with it. */
		goto uncompressed;
	}
	if((deltaS = th.window - oth->window) != 0){
		cp = encode(cp,deltaS);
		changes |= NEW_W;
	}
	if((deltaA = th.acknowledge - oth->acknowledge) != 0L){
		if(deltaA > 0x0000ffff)
			goto uncompressed;
		cp = encode(cp,deltaA);
		changes |= NEW_A;
	}
	if((deltaS = th.sequence - oth->sequence) != 0L){
		if(deltaS > 0x0000ffff)
			goto uncompressed;
		cp = encode(cp,deltaS);
		changes |= NEW_S;
	}

	switch(changes){
	case 0:	/* Nothing changed. If this packet contains data and the
		 * last one didn't, this is probably a data packet following
		 * an ack (normal on an interactive connection) and we send
		 * it compressed.  Otherwise it's probably a retransmit,
		 * retransmitted ack or window probe.  Send it uncompressed
		 * in case the other side missed the compressed version.
		 */
		if(iph.length != cs->cs_ip.length && cs->cs_ip.length == hlen)
			break;
		goto uncompressed;
	case SPECIAL_I:
	case SPECIAL_D:
		/* actual changes match one of our special case encodings --
		 * send packet uncompressed.
		 */
		goto uncompressed;
	case NEW_S|NEW_A:
		if(deltaS == deltaA &&
		    deltaS == cs->cs_ip.length - hlen){
			/* special case for echoed terminal traffic */
			changes = SPECIAL_I;
			cp = new_seq;
		}
		break;
	case NEW_S:
		if(deltaS == cs->cs_ip.length - hlen){
			/* special case for data xfer */
			changes = SPECIAL_D;
			cp = new_seq;
		}
		break;
	}
	deltaS = iph.id - cs->cs_ip.id;
	if(deltaS != 1){
		cp = encode(cp,deltaS);
		changes |= NEW_I;
	}
	if(th.flags & TCP_PUSH)
		changes |= TCP_PUSH_BIT;
	/* Grab the cksum before we overwrite it below.  Then update our
	 * state with this packet's header.
	 */
	deltaA = th.checksum;

	BlockMove(&iph, &cs->cs_ip, hiplen);
	BlockMove(&th, &cs->cs_tcp, htcplen);

	/* We want to use the original packet as our compressed packet.
	 * (cp - new_seq) is the number of bytes we need for compressed
	 * sequence numbers.  In addition we need one byte for the change
	 * mask, one for the connection id and two for the tcp checksum.
	 * So, (cp - new_seq) + 4 bytes of header are needed.
	 */
	deltaS = cp - new_seq;
	if(compress_cid == 0 || comp->xmit_current != cs->this){
		makeroom(bufptr, deltaS + 4);
		cp = bufptr->dataptr;
		*cp++ = changes | NEW_C;
		*cp++ = cs->this;
		comp->xmit_current = cs->this;
	} else {
		makeroom(bufptr,deltaS + 3);
		cp = bufptr->dataptr;
		*cp++ = changes;
	}
	*cp++ = deltaA >> 8;	/* Write TCP checksum */
	*cp++ = deltaA;
	BlockMove(new_seq,cp,(long)deltaS);	/* Write list of deltas */
	return SL_TYPE_COMPRESSED_TCP;

	/* Update connection state cs & send uncompressed packet (i.e.,
	 * a regular ip/tcp packet but with the 'conversation id' we hope
	 * to use on future compressed packets in the protocol field).
	 */
uncompressed:
	iph.protocol = cs->this;
	BlockMove(&iph, &cs->cs_ip, hiplen);
	BlockMove(&th, &cs->cs_tcp, htcplen);
	comp->xmit_current = cs->this;
	bufptr->dataptr -= hlen;
	bufptr->length += hlen;
	*(bufptr->dataptr + IP_PROTO_OFFSET) = iph.protocol;
	return SL_TYPE_UNCOMPRESSED_TCP;
}

short
slhc_uncompress(LapInfo *lap, struct bufheader *bufptr)
{
	struct slcompress *comp = &(lap->comp);
	short changes;
	long x;
	unsigned long cksum;
	b_16 *ptr;
	unsigned short ipheadlen;
	unsigned short sumword;
	struct tcpheader *thp;
	struct cstate *cs;
	short len;

	/* We've got a compressed packet; read the change byte */
	if (bufptr->length < 3)
		return 0;
		
	changes = yankbyte(bufptr);	/* "Can't fail" */
	if(changes & NEW_C){
		/* Make sure the state index is in range, then grab the state.
		 * If we have a good state index, clear the 'discard' flag.
		 */
		x = yankbyte(bufptr);	/* Read conn index */
		if(x < 0 || x > comp->rslot_limit)
			goto bad;

		comp->flags &=~ SLF_TOSS;
		comp->recv_current = x;
	} else {
		/* this packet has an implicit state index.  If we've
		 * had a line error since the last time we got an
		 * explicit state index, we have to toss the packet. */
		if(comp->flags & SLF_TOSS)
			return 0;
	}
	cs = &comp->rstate[comp->recv_current];
	thp = &cs->cs_tcp;
	
	ipheadlen = (cs->cs_ip.version & 0x0f) << 2 ;

	if((x = yank16(bufptr)) == -1)	/* Read the TCP checksum */
		goto bad;
	thp->checksum = x;

	if (changes & TCP_PUSH_BIT)
		thp->flags |= TCP_PUSH;
	else
		thp->flags &= ~TCP_PUSH;

	switch(changes & SPECIALS_MASK){
	case SPECIAL_I:		/* Echoed terminal traffic */
		{
		b_16 i;
		i = cs->cs_ip.length;
		i -= ( ipheadlen + TCPHLEN);
		thp->acknowledge += i;
		thp->sequence += i;
		}
		break;

	case SPECIAL_D:			/* Unidirectional data */
		thp->sequence += cs->cs_ip.length - (ipheadlen + TCPHLEN);
		break;

	default:
		if(changes & NEW_U){
			thp->flags |= TCP_URG;
			if((x = decode(bufptr)) == -1)
				goto bad;
			thp->urgent_pointer = x;
		} else
			thp->flags &= ~TCP_URG;
		if(changes & NEW_W){
			if((x = decode(bufptr)) == -1)
				goto bad;
			thp->window += x;
		}
		if(changes & NEW_A){
			if((x = decode(bufptr)) == -1)
				goto bad;
			thp->acknowledge += x;
		}
		if(changes & NEW_S){
			if((x = decode(bufptr)) == -1)
				goto bad;
			thp->sequence += x;
		}
		break;
	}
	if(changes & NEW_I){
		if((x = decode(bufptr)) == -1)
			goto bad;
		cs->cs_ip.id += x;
	} else
		cs->cs_ip.id++;

	/*
	 * At this point, the buffer points to the first byte of data in the
	 * packet.  Put the reconstructed TCP and IP headers back on the
	 * packet.  Recalculate IP checksum (but not TCP checksum).
	 */
	len =  bufptr->length + ipheadlen + TCPHLEN;
	cs->cs_ip.length = len;

	puttcpheader(thp,bufptr);
	
	/* need to recalc IP header checksum here */
	cs->cs_ip.checksum = 0;
	ipheadlen >>= 1;
	ptr = (b_16 *) &cs->cs_ip;
	cksum = 0;
	while (ipheadlen-- != 0)
		cksum += *ptr++;
	while ((sumword = cksum >> 16) != 0)
		cksum = sumword + (cksum & 0xffffL);
	cs->cs_ip.checksum = ~cksum & 0xffffL;
	putipheader(&cs->cs_ip,bufptr);
	return len;
bad:
	return slhc_toss( comp );
}

short
slhc_remember(LapInfo *lap, struct bufheader *bufptr)
{
	struct slcompress *comp = &lap->comp;
	struct cstate *cs;
	struct ipheader iph;
	struct tcpheader th;
	register unsigned short headerlen;
	b_8 slot_id;
	
	/*  verify slot ID */
	if( (slot_id = *(bufptr->dataptr + IP_PROTO_OFFSET)) > comp->rslot_limit) {
		PPP_DEBUG_CHECKS("\pillegal slot num");
		return slhc_toss(comp);
	}

	/* Update local state */
	cs = &comp->rstate[comp->recv_current = slot_id];
	comp->flags &=~ SLF_TOSS;
	*(bufptr->dataptr + IP_PROTO_OFFSET) = TCP_PROTOCOL; /* restore proto */

	headerlen = getipheader(&cs->cs_ip,bufptr);
	headerlen += gettcpheader(&cs->cs_tcp,bufptr);

	/* Put headers back on packet */
	bufptr->dataptr -= headerlen;
	bufptr->length += headerlen;

	return (bufptr->length);
}

short
slhc_toss(struct slcompress *comp)
{
	comp->flags |= SLF_TOSS;
	return 0;
}
