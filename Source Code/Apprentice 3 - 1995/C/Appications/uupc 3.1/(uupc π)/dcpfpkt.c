/* $Header: fio.c,v 1.20 85/04/30 12:57:32 rick Exp $ */
/*	%M%	%I%	%E%	(Mathematisch Centrum)	*/

/*
 * flow control protocol.
 *
 * This protocol relies on flow control of the data stream.
 * It is meant for working over links that can (almost) be
 * guaranteed to be errorfree, specifically X.25/PAD links.
 * A sumcheck is carried out over a whole file only. If a
 * transport fails the receiver can request retransmission(s).
 * This protocol uses a 7-bit datapath only, so it can be
 * used on links that are not 8-bit transparent.
 *
 * When using this protocol with an X.25 PAD:
 * Although this protocol uses no control chars except CR,
 * control chars NULL and ^P are used before this protocol
 * is started; since ^P is the default char for accessing
 * PAD X.28 command mode, be sure to disable that access
 * (PAD par 1). Also make sure both flow control pars
 * (5 and 12) are set. The CR used in this proto is meant
 * to trigger packet transmission, hence par 3 should be 
 * set to 2; a good value for the Idle Timer (par 4) is 10.
 * All other pars should be set to 0.
 * Normally a calling site will take care of setting the
 * local PAD pars via an X.28 command and those of the remote
 * PAD via an X.29 command, unless the remote site has a
 * special channel assigned for this protocol with the proper
 * par settings.
 *
 * Author: Piet Beertema, CWI, Amsterdam, Sep 1984
 * Adapted to uupc 3.0 and THINK C 4.0 by Dave Platt, Jul 1991
 */

/*
			Portions Copyright © David Platt, 1992, 1991.  All Rights Reserved
			Worldwide.
*/

#include "dcp.h"

#define PKTSIZE   128

#ifndef MAXMSGLEN
#define MAXMSGLEN	BUFSIZ
#endif MAXMSGLEN

#include "dcpfpkt.proto.h"

static int chksum;

int fopenpk(int master)
{
	flowcontrol(TRUE, useHardwareFlowControl);
    pktsize = PKTSIZE;
    msgtime = MSGTIME;
	zzz(2); /* Give peer time to perform corresponding port setup */
	return OK;
}

fclosepk(void)
{
	flowcontrol(FALSE, FALSE);
	return OK;
}

fwrmsg(char *str, boolean nowait)
{
	register char *s;
	char bufr[MAXMSGLEN];

	s = bufr;
	while (*str)
		*s++ = *str++;
	if (*(s-1) == '\n')
		s--;
	*s++ = '\r';
	(void) swrite(bufr, s - bufr);
	return OK;
}

frdmsg(register char *str, int *bytes)
{
	register char *smax;
	char *base = str;

	smax = str + MAXPACK - 1;
	for (;;) {
		if (sread(str, 1, msgtime) <= 0)
			goto msgerr;
		if (*str == '\r')
			break;
		if (*str < ' ')
			continue;
		if (str++ >= smax)
			goto msgerr;
	}
	*str = '\0';
	*bytes = strlen(base) + 1;
	return OK;
msgerr:
	return FAILED;
}


fgetpkt(char *packet, int *bytes)
{
	register char *op, c, *ip;
	register int sum, len, nl, left;
	char buf[5], tbuf[MAXPACK];
	int i, tleft, sready, slept;
	static char special = 0;
	
	left = pktsize;
	op = packet;
	sum = chksum;
	tleft = 0;

	do {
		if (tleft == 0) {
			ip = tbuf;
			sready = savail();
			if (sready > MAXPACK) {
				sready = MAXPACK;
			}
			if (sready > left) {
				sready = left;
			} else if (sready == 0) {
				sready = 1;
			}
			len = sread(ip, sready, msgtime);
			if (len == 0) {
				printmsg(0, "Timeout occurred while reading data");
				return FAILED; /* fail if timed out */
			}
			tleft = len;
		}
		if ((*ip &= 0177) >= '\172') {
			if (special) {
				special = 0;
				if (*ip++ != '\176') {
					printmsg(0, "End-of-file checksum sequence garbled");
					goto dcorr;
				}
				tleft--;
				if (tleft > 0) {
					if (tleft > 5) tleft = 5;
					memcpy(buf, ip, tleft);
					len = tleft;
				} else {
					len = 0;
				}
				while (len < 5) {
					i = sread(&buf[len], 5 - len, msgtime);
					if (i == 0) {
						len = FAILED;
						printmsg(0, "Timeout occurred while reading checksum");
						goto dcorr;
					}
					len += i;
				}
				if (buf[4] != '\r')
					goto dcorr;
				sscanf(buf, "%4x", &chksum);
				if (chksum == sum) {
					*bytes = packet - op; /* return negative count to signal EOF */
					remote_stats.breceived -= *bytes;
					return OK;
				} else {
					printmsg(0, "Checksum mismatch, told %04x, calc %04x", chksum, sum);
					*bytes = op - packet;
					remote_stats.breceived -= *bytes;
					fsendresp(RETRY);
					return 'R';
				}
			}
			special = *ip++;
		} else {
			if (*ip < '\040') {
				/* error: shouldn't get control chars */
				printmsg(0, "Received a control character (shouldn't have!)");
				goto dcorr;
			}
			switch (special) {
			case 0:
				c = *ip++;
				break;
			case '\172':
				c = *ip++ - 0100;
				break;
			case '\173':
				c = *ip++ + 0100;
				break;
			case '\174':
				c = *ip++ + 0100;
				break;
			case '\175':
				c = *ip++ + 0200;
				break;
			case '\176':
				c = *ip++ + 0300;
				break;
			}
			*op++ = c;
			left --;
			if (sum & 0x8000) {
				sum <<= 1;
				sum++;
			} else
				sum <<= 1;
			sum += c & 0377;
			sum &= 0xffff;
			special = 0;
		}
		tleft--;
	} while (left > 0);
	*bytes = pktsize;
	chksum = sum;
	remote_stats.breceived += pktsize;
	return OK;
dcorr:
	printmsg (0, "Data corrupted, skipping to EOF");
	while (sread(tbuf, 1, msgtime) > 0) {
		/* spin really fast */
	}
	fsendresp(RETRY);
	return 'R';
}

fsendpkt(char *ip, int len, int flag)
{
	register char *op;
	register int sum, nl;
	int ret;
	char obuf[MAXPACK * 2];
	op = obuf;
	nl = 0;
	sum = chksum;
    if (flag) {
    	len = strlen(ip)+1;
    } else {
    	if (len == 0) return;
    }
	remote_stats.bsent += len;
	do {
		if (sum & 0x8000) {
			sum <<= 1;
			sum++;
		} else
			sum <<= 1;
		sum += *ip & 0377;
		sum &= 0xffff;
		if (*ip & 0200) {
			*ip &= 0177;
			if (*ip < 040) {
				*op++ = '\174';
				*op++ = *ip++ + 0100;
			} else
			if (*ip <= 0171) {
				*op++ = '\175';
				*op++ = *ip++;
			}
			else {
				*op++ = '\176';
				*op++ = *ip++ - 0100;
			}
			nl += 2;
		} else {
			if (*ip < 040) {
				*op++ = '\172';
				*op++ = *ip++ + 0100;
				nl += 2;
			} else
			if (*ip <= 0171) {
				*op++ = *ip++;
				nl++;
			} else {
				*op++ = '\173';
				*op++ = *ip++ - 0100;
				nl += 2;
			}
		}
	} while (--len > 0);
	chksum = sum;
	ret = swrite(obuf, nl);
	return ret == nl ? nl : ret < 0 ? 0 : -ret;
}

int ffilepkt(void)
{
	chksum = 0xffff;
	return 0;
}

feofpkt(void)
{
	char ibuf[MAXMSGLEN], ack;
	int len;
	sprintf(ibuf, "\176\176%04x", chksum);
	fwrmsg(ibuf, FALSE);
	ack = 'A';
	if (frdmsg(ibuf, &len) != FAILED) {
		ack = ibuf[0];
	}
	return ack;
}

int fsendresp(int state)
{
	char spacket[64];
	switch (state) {
	 case OK:
		return (fwrmsg("G", FALSE));
	 case RETRY:
		return (fwrmsg("R", FALSE));
	 default:
		return (fwrmsg("Q", FALSE));
	}
}
