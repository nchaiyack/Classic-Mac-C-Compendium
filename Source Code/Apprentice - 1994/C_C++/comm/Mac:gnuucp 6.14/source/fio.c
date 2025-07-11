From: samsung!apple.com!blob
To: fpr.com!jim
Date: Mon, 29 Oct 90 08:04:09 -0800
Subject: Re: 7 bit GNUUCP transfers 
From mitech!blob@apple.com Mon, 29 Oct 90 19:40:19 EDT remote from fpr
Received: by fpr (Mac/gnuucp v3.5) Mon, 29 Oct 90 19:40:19 EDT
Received: from samsung by mitech.com (DECUS UUCP w/Smail);
          Mon, 29 Oct 90 18:22:26 EDT
Received: by schizo from APPLE.COM via SMTP (vers 5.61+)
          for fpr.com!jim (from samsung!apple.com!blob)
          id <AA18898@schizo>; Mon, 29 Oct 90 11:03:47 -0500
Received: by apple.com with SMTP (5.61/25-eef)
          id AA01296; Mon, 29 Oct 90 08:04:11 -0800
          for jim@fpr.com
Message-Id: <9010291604.AA01296@apple.com>
Cc: apple.com!blob
In-Reply-To: Your message of 10 Saturday 90 00:00:00 -0800.
          <9010291055.AA09339@schizo> 


I got this from Tim Pozar (pozar@kumr.uucp)
]>Do you have a description of the uucp F protocol?
]   This is what I found so far.  Drop me a line if you need more
]info and I can rummage around some other places...

---
READMEfio
---
F protocol (X.25 PAD)
Installation hints

"F" protocol is a UUCP protocol used to communicate over X.25 networks.
The PAD dialer is a "real" dialer that can be used to set up connections
to remote X.25 destinations using MICOM or Motorola PADS - and will probably
work with most other PADS.  fio.c was originally distributed with various 
"alpha-test" versions of the UUCP in BSD4.3 and has been posted several
times.  I wrote the PAD dialer.  This readme gives some pointers on how 
to install this into your UUCP.

Applicability: if you already have "f" protocol (eg: you already have BSD4.3 
UUCP), fio.c is likely to be a little older than the one you have, so don't 
bother using it.  However, Rick Adams has told me that the PAD dialer 
didn't make it into real BSD4.3, so the dialer should be really useful to you
without any changes.  If you have an older UUCP (eg: BSD4.2 or previous, 
System V, XENIX) you may be able to install both of these files.

Note: I have never tried to install either of these into non-4.3 UUCP,
so your mileage may vary - greatly.  The pad dialer in particular makes use
of "generic" facilities (ABORT sequences, chat scripts etc.) that are not
likely to be accessible in older UUCPs.  So you may have to do some carving
just to get your new UUCP to compile and link.  I'm afraid you're on your
own in that case.

Installation:
1) Make copies of cntrl.c and condevs.c
2) Insert into cntrl.c, near the beginning:
    #ifdef PAD
    extern int fturnon(), fturnoff();
    extern int frdmsg(), frddata();
    extern int fwrmsg(), fwrdata();
    #endif PAD
3) in cntrl.c, inside the "struct Proto Ptbl[] =" initialization,
   insert the following just before the "g" proto definition:
    #ifdef PAD
'f', fturnon, frdmsg, fwrmsg, frddata, fwrdata, fturnoff,
    #endif PAD
4) In cntrl.c, inside the fptcl(str) function,
   you will see something like:
    for (p = Ptbl; p->P_id != '\0'; p++) {
    ...
if (index(str, p->P_id) != NULL) {
return p->P_id;
}
    }
   Insert the following code just before the "index" line:
    #ifdef PAD
/* only use 'f' protocol on PAD */
if (p->P_id == 'f' && strcmp("PAD", Flds[F_LINE]))
continue;
    #endif PAD

5) in condevs.c, insert the following into the 
   "struct condev condevs[]=" initialization:
    #ifdef PAD
{ "PAD", "PAD", padopn, nulldev, padcls },
    #endif
6) insert "#define PAD 1" into your uucp.h
7) copy pad.c into the directory containing the rest of the dialers
   and modify the makefile to compile it.
8) copy fio.c into the main UUCP directory and modify the makefile
   to compile it.
9) run your UUCP make.

You may encounter a number of undefined things.  If "dochat" is undefined,
remove it from pad.c.  Ditto the stuff about ABORT sequences.

Typical L.sys entry:

<system> Any PAD <speed> <seq> 

Where <speed> is the baud rate between your computer and your pad.  <seq>
is the "dial number" to dial the destination.  Eg: if you have to type
"c 12345555" to reach that destination, place "12345555" here.  You may
encounter length restrictions when talking to the PAD, so you may have to
set up "2-character aliases" in the PAD configuration.

Typical L-devices entry:

PAD <tty> ignored <baud> <chat sequence>

The <baud> must be the same as in the L.sys entry.  <tty> is the ttyname
that the PAD is connected to.  Eg: "tty44".  Chat sequence (alternatively
protocol specification in some versions of SV UUCP) is probably not
of any use unless you have the rest of 4.3BSD UUCP.  Duplicate this line
with "DIR" instead of "PAD" so that you can "cu" to the line.

Other setup: on dialer out-going line: disable getty, and connect the line
to the PAD.  Initialize the PAD line parameters so that it is a "outgoing"
port, and that you can get at X.28 command mode.  Eg: you should be able
to "cu" to the line and dial out using X.28 commands.  Do not worry about
any other parameters because the dialer will initialize all of the parameters
on the line every time it starts up, and when the link is broken, the parameters
revert.  Thus, the best thing to do is initialize the line for outgoing
cu.
---
fio.c
---
/* $Header: fio.c,v 1.20 85/04/30 12:57:32 rick Exp $ */
/*%M%%I%%E%(Mathematisch Centrum)*/

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
 */

#include "uucp.h"
#include <signal.h>
#ifdef USG
#include <termio.h>
#else !USG
#include <sgtty.h>
#endif !USG
#include <setjmp.h>

#define FBUFSIZ256

#ifndef MAXMSGLEN
#define MAXMSGLENBUFSIZ
#endif MAXMSGLEN

static int chksum;
static jmp_buf Ffailbuf;

static
falarm()
{
signal(SIGALRM, falarm);
longjmp(Ffailbuf, 1);
}

static int (*fsig)();

#ifndef USG
#define TCGETATIOCGETP
#define TCSETAFTIOCSETP
#define termiosgttyb
#else
#endif USG

fturnon()
{
int ret;
struct termio ttbuf;

ioctl(Ifn, TCGETA, &ttbuf);
#ifdef USG
ttbuf.c_iflag = IXOFF|IXON|ISTRIP;
ttbuf.c_cc[VMIN] = FBUFSIZ > 64 ? 64 : FBUFSIZ;
ttbuf.c_cc[VTIME] = 5;
#else
ttbuf.sg_flags = ANYP|CBREAK|TANDEM;
#endif USG
ret = ioctl(Ifn, TCSETAF, &ttbuf);
ASSERT(ret >= 0, "STTY FAILED", "", ret);
#ifndefUSG
{
    intlocalmodeword;
    ioctl(Ifn, TIOCLGET, &localmodeword);
    DEBUG(8, "local mode word: %o\n", localmodeword);
    localmodeword &= ~LNOHANG;
    ioctl(Ifn, TIOCLSET, &localmodeword);
}
#endif
fsig = signal(SIGALRM, falarm);
/* give the other side time to perform its ioctl;
 * otherwise it may flush out the first data this
 * side is about to send.
 */
sleep(2);
return SUCCESS;
}

fturnoff()
{
(void) signal(SIGALRM, fsig);
return SUCCESS;
}

fwrmsg(type, str, fn)
register char *str;
int fn;
char type;
{
register char *s;
char bufr[MAXMSGLEN];

s = bufr;
*s++ = type;
while (*str)
*s++ = *str++;
if (*(s-1) == '\n')
s--;
*s++ = '\r';
(void) write(fn, bufr, s - bufr);
return SUCCESS;
}

frdmsg(str, fn)
register char *str;
register int fn;
{
register char *smax;

if (setjmp(Ffailbuf))
return FAIL;
smax = str + MAXMSGLEN - 1;
(void) alarm(2*MAXMSGTIME);
for (;;) {
if (read(fn, str, 1) <= 0)
goto msgerr;
if (*str == '\r')
break;
if (*str < ' ')
continue;
if (str++ >= smax)
goto msgerr;
}
*str = '\0';
(void) alarm(0);
return SUCCESS;
msgerr:
(void) alarm(0);
return FAIL;
}

fwrdata(fp1, fn)
FILE *fp1;
int fn;
{
register int flen, alen, ret;
register char *obp;
char ibuf[FBUFSIZ];
char ack;
long abytes, fbytes;
struct timeb t1, t2;
int mil, retries = 0;

ret = FAIL;
retry:
chksum = 0xffff;
abytes = fbytes = 0L;
ack = '\0';
#ifdef USG
time(&t1.time);
t1.millitm = 0;
#else !USG
ftime(&t1);
#endif !USG
while ((flen = fread(ibuf, sizeof (char), FBUFSIZ, fp1)) > 0) {
alen = fwrblk(fn, ibuf, flen);
abytes += alen >= 0 ? alen : -alen;
if (alen <= 0)
goto acct;
fbytes += flen;
}
sprintf(ibuf, "\176\176%04x\r", chksum);
abytes += alen = strlen(ibuf);
if (write(fn, ibuf, alen) == alen) {
DEBUG(8, "wrdata: checksum length: %d\n", alen);
DEBUG(8, "checksum: %04x\n", chksum);
if (frdmsg(ibuf, fn) != FAIL) {
if ((ack = ibuf[0]) == 'G')
ret = 0;
DEBUG(4, "ack - '%c'\n", ack);
}
}
acct:
if (ack == 'R') {
DEBUG(4, "RETRY:\n", 0);
fseek(fp1, 0L, 0);
retries++;
goto retry;
}
#ifdef USG
time(&t2.time);
t2.millitm = 0;
#else !USG
ftime(&t2);
#endif !USG
Now = t2;
t2.time -= t1.time;
mil = t2.millitm - t1.millitm;
if (mil < 0) {
--t2.time;
mil += 1000;
}
sprintf(ibuf, "sent data %ld bytes %ld.%02d secs",
fbytes, (long)t2.time, mil/10);
sysacct(abytes, t2.time - t1.time);
if (retries > 0) 
sprintf((char *)ibuf+strlen(ibuf)," %d retries", retries);
DEBUG(1, "%s\n", ibuf);
syslog(ibuf);
#ifdef SYSACCT
if (ret)
sysaccf(NULL);/* force accounting */
#endif SYSACCT
return ret;
}

/* max. attempts to retransmit a file: */
#define MAXRETRIES(fbytes < 10000L ? 2 : 1)

frddata(fn, fp2)
register int fn;
register FILE *fp2;
{
register int flen;
register char eof;
char ibuf[FBUFSIZ];
int ret, retries = 0;
long alen, abytes, fbytes;
struct timeb t1, t2;
int mil;

ret = FAIL;
retry:
chksum = 0xffff;
abytes = fbytes = 0L;
#ifdef USG
time(&t1.time);
t1.millitm = 0;
#else !USG
ftime(&t1);
#endif !USG
do {
flen = frdblk(ibuf, fn, &alen);
abytes += alen;
if (flen < 0)
goto acct;
if (eof = flen > FBUFSIZ)
flen -= FBUFSIZ + 1;
fbytes += flen;
if (fwrite(ibuf, sizeof (char), flen, fp2) != flen)
goto acct;
} while (!eof);
ret = 0;
acct:
if (ret) {
if (retries++ < MAXRETRIES) {
DEBUG(8, "send ack: 'R'\n", 0);
fwrmsg('R', "", fn);
fseek(fp2, 0L, 0);
DEBUG(4, "RETRY:\n", 0);
goto retry;
}
DEBUG(8, "send ack: 'Q'\n", 0);
fwrmsg('Q', "", fn);
#ifdef SYSACCT
sysaccf(NULL);/* force accounting */
#endif SYSACCT
} else {
DEBUG(8, "send ack: 'G'\n", 0);
fwrmsg('G', "", fn);
}
#ifdef USG
time(&t2.time);
t2.millitm = 0;
#else !USG
ftime(&t2);
#endif !USG
Now = t2;
t2.time -= t1.time;
mil = t2.millitm - t1.millitm;
if (mil < 0) {
--t2.time;
mil += 1000;
}
sprintf(ibuf, "received data %ld bytes %ld.%02d secs",
fbytes, (long)t2.time, mil/10);
sysacct(abytes, t2.time - t1.time);
if (retries > 0) 
sprintf((char *)ibuf+strlen(ibuf)," %d retries", retries);
DEBUG(1, "%s\n", ibuf);
syslog(ibuf);
return ret;
}

static
frdbuf(blk, len, fn)
register char *blk;
register int len;
register int fn;
{
static int ret = FBUFSIZ / 2;
#ifndef Not080
extern int linebaudrate;
#endif Not080

if (setjmp(Ffailbuf))
return FAIL;
#ifndef Not080
if (len == FBUFSIZ && ret < FBUFSIZ / 2 &&
    linebaudrate > 0 && linebaudrate < 4800)
sleep(1);
#endif Not080
(void) alarm(MAXMSGTIME);
ret = read(fn, blk, len);
alarm(0);
return ret <= 0 ? FAIL : ret;
}

/* call ultouch every TC calls to either frdblk or fwrblk  */

#defineTC20
staticint tc = TC;

/* Byte conversion:
 *
 *   from        pre       to
 * 000-037       172     100-137
 * 040-171               040-171
 * 172-177       173     072-077
 * 200-237       174     100-137
 * 240-371       175     040-171
 * 372-377       176     072-077
 */

static
fwrblk(fn, ip, len)
int fn;
register char *ip;
register int len;
{
register char *op;
register int sum, nl;
int ret;
char obuf[FBUFSIZ * 2];

/* call ultouch occasionally */
if (--tc < 0) {
tc = TC;
ultouch();
}
DEBUG(8, "fwrblk: %d/", len);
op = obuf;
nl = 0;
sum = chksum;
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
} while (--len);
chksum = sum;
DEBUG(8, "%d\n", nl);
ret = write(fn, obuf, nl);
return ret == nl ? nl : ret < 0 ? 0 : -ret;
}

static
frdblk(ip, fn, rlen)
register char *ip;
int fn;
long *rlen;
{
register char *op, c;
register int sum, len, nl;
char buf[5], *erbp = ip;
int i;
static char special = 0;

/* call ultouch occasionally */
if (--tc < 0) {
tc = TC;
ultouch();
}

if ((len = frdbuf(ip, FBUFSIZ, fn)) == FAIL) {
*rlen = 0;
goto dcorr;
}
*rlen = len;
DEBUG(8, "%d/", len);
op = ip;
nl = 0;
sum = chksum;
do {
if ((*ip &= 0177) >= '\172') {
if (special) {
DEBUG(8, "%d", nl);
special = 0;
op = buf;
if (*ip++ != '\176' || (i = --len) > 5)
goto dcorr;
while (i--)
*op++ = *ip++;
while (len < 5) {
i = frdbuf(&buf[len], 5 - len, fn);
if (i == FAIL) {
len = FAIL;
goto dcorr;
}
DEBUG(8, ",%d", i);
len += i;
*rlen += i;
}
if (buf[4] != '\r')
goto dcorr;
sscanf(buf, "%4x", &chksum);
DEBUG(8, "\nchecksum: %04x\n", sum);
if (chksum == sum)
return FBUFSIZ + 1 + nl;
else {
DEBUG(8, "\n", 0);
DEBUG(4, "Bad checksum\n", 0);
return FAIL;
}
}
special = *ip++;
} else {
if (*ip < '\040') {
/* error: shouldn't get control chars */
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
if (sum & 0x8000) {
sum <<= 1;
sum++;
} else
sum <<= 1;
sum += c & 0377;
sum &= 0xffff;
special = 0;
nl++;
}
} while (--len);
chksum = sum;
DEBUG(8, "%d,", nl);
return nl;
dcorr:
DEBUG(8, "\n", 0);
DEBUG(4, "Data corrupted\n", 0);
while (len != FAIL) {
if ((len = frdbuf(erbp, FBUFSIZ, fn)) != FAIL)
*rlen += len;
}
return FAIL;
}
---
pad.c
---

#ifndef lint
static char*RcsId = "$Header: pad.c,v 1.1 85/01/08 19:58:45 rick Exp $";
#endif !lint

#include "../condevs.h"
#ifdef PAD

/*
 *padopn: establish connection through a PAD.
 *Returns descriptor open to tty for reading and writing.
 *Negative values (-1...-7) denote errors in connmsg.
 *Be sure to disconnect tty when done, via HUPCL or stty 0.
 */

char *padparms[] = {
"set1:0,2:0,3:2,4:1,5:1,6:5,7:4,9:0,10:0,13:0",
"set14:0,15:0,16:0,17:0,18:0,19:1,20:255",
"set102:0,103:17,104:19,105:0,106:0,107:0,108:0",
"set109:0,110:0,111:0,112:0,113:0,114:0,115:0,116:0",
NULL
};

externchar*AbortOn;
intpadcls();
padopn(flds)
register char *flds[];
{
char phone[MAXPH+1];
register char **parmptr;
extern errno;
char *rindex(), *fdig(), dcname[20];
int dh, ok = 0, speed;
register struct condev *cd;
register FILE *dfp;
struct Devices dev;

dfp = fopen(DEVFILE, "r");
ASSERT(dfp != NULL, "Can't open", DEVFILE, 0);

signal(SIGALRM, alarmtr);
dh = -1;
for(cd = condevs; ((cd->CU_meth != NULL)&&(dh < 0)); cd++) {
if (snccmp(flds[F_LINE], cd->CU_meth) == SAME) {
fseek(dfp, (off_t)0, 0);
while(rddev(dfp, &dev) != FAIL) {
if (strcmp(flds[F_CLASS], dev.D_class) != SAME)
continue;
if (snccmp(flds[F_LINE], dev.D_type) != SAME)
continue;
DEBUG(4, "Trying line %s\n", dev.D_line);
if (mlock(dev.D_line) == FAIL)
continue;

sprintf(dcname, "/dev/%s", dev.D_line);
getnextfd();
alarm(10);
if (setjmp(Sjbuf)) {
delock(dev.D_line);
logent(dev.D_line,"pad open TIMEOUT");
dh = -1;
break;
}
dh = open(dcname, 2);
alarm(0);
next_fd = -1;
if (dh > 0) {
break;
}
DEBUG(4, "Can't open line %s\n", dev.D_line);
devSel[0] = '\0';
delock(dev.D_line);
}
}
}
fclose(dfp);
if (dh < 0)
return CF_NODEV;
DEBUG(4, "Using line %s\n", dev.D_line);

speed = atoi(fdig(flds[F_CLASS]));
fixline(dh, speed);
/*Do we need this?  I don't know*/
sleep(1);

/* Synchronize with PAD prompt */
write(dh, "\r", 1);
DEBUG(10, "Pad Sync: wanted %s\n", ">");
ok = expect(">", dh);
DEBUG(10, "got %s\n", ok ? "?" : "that");

if (ok) {
logent(dev.D_line, "PAD SYNC FAILED");
close(dh);
return CF_DIAL;
}

/*Initialization of PAD*/
AbortOn = "err";
for (parmptr = padparms; ok == 0 && *parmptr; parmptr++) {
DEBUG(10, "PAD setup: %s\n", *parmptr);
write(dh, *parmptr, strlen(*parmptr));
write(dh, "\r", 1);
ok = expect(">", dh);
DEBUG(4, "setup %s\n", ok? "failed": "worked");
}
if (Debug > 10) {
DEBUG(10, "PAD %s:\n", "configuration");
write(dh, "par?\r", 6);
ok = expect(">", dh);
}
AbortOn = NULL;/* dochat(login) does this anyways */
if (ok) {
logent(dev.D_line, "PAD SETUP/CONFIG DEBUG FAILED");
close(dh);
return CF_DIAL;
}

/*Do chat from L-devices */
if (dochat(&dev, flds, dh)) {
logent(dev.D_line, "CHAT FAILED");
close(dh);
return CF_DIAL;
}

if (ok == 0) {
exphone(flds[F_PHONE], phone);
DEBUG(4, "PAD: calling %s\n", phone);
write(dh, "c ", 2);
write(dh, phone, strlen(phone));
write(dh, "\r", 1);
DEBUG(4, "wanted %s ", "com");
AbortOn = "clr";
ok = expect("com", dh);
DEBUG(4, "got %s\n", ok ? "?" : "that");
AbortOn = NULL;
}

if (ok != 0) {
if (dh > 2)
close(dh);
DEBUG(4, "pad failed\n", "");
delock(dev.D_line);
return(CF_DIAL);
} 
else
DEBUG(4, "pad ok\n", "");

CU_end = padcls;
strcat(devSel, dev.D_line);/* for later unlock */
return dh;
}

padcls(fd)
register int fd;
{

if (fd > 0) {
DEBUG(4, "Closing %s\n", "PAD");
close(fd);
delock(devSel);
}
}
#endif MICOM
---
eof
---


======================================================================

