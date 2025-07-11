/*
 * kerberos.c
 */

#ifdef MPW
#pragma segment 22
#endif

#define NCSA_ENC

#ifdef TN3270
#pragma segment 3270tcp
#if !defined(USEDUMP)
#include "maclib.h"
#include "termdef.h"
#include "tn3270funcs.h"
#include "globals.h"
#else
#pragma load "tn3270DumpFile"
#endif
#include "telnet.h"
#include "hostform.h"
#endif /* TN3270 */

#ifdef NCSA_ENC
#include <String.h>
#include "TelnetHeader.h"
#include "wind.h"
#include "parse.proto.h"
#endif

#define TELCMDS 1
#define TELOPTS 1

#include "encrypt.proto.h"
#include "kerberos.proto.h"
#include "desproto.h"

static short kdriver = 0;		/* .Kerberos driver ref */

/*
 * krb_telnet
 * Send Kerberos credentials as a telnet suboption.
 * Return error number if error;
 *
 * Called with doit==false to verify that we have
 * enough credentials.
 */
long krb_telnet (WDATA *cr, char *tp, long doit)
{
	#pragma unused (doit)
	long i, len;
	short s;
	unsigned char *cp, *cn, cannon[256];
	ParamBlockRec pbr, *pb = &pbr;
	krbHiParmBlock khpb, *khipb = &khpb;
	krbParmBlock klpb, *klopb = &klpb;
	unsigned char buf[1258+20];
	Session_Key skey;
	Str255 scratch1, scratch2, kname;
	unsigned char *kp;
	des_cblock sessionKey;

	/*
	 * Open the .Kerberos driver if not already open
	 */
	if (!kdriver) {
		if (s = OpenDriver("\p.Kerberos", &kdriver)) {
			return -1;
		}
	}
	
	/*
	 * Figure out the cannonical hosthame.
	 * If we used domain lookup, it should be in mp->cannon.
	 * Otherwise, we use mp->hname.
	 */
	cp = 0;
	cp = (unsigned char *)getcname(cr);
	if (!cp)
		return -1;

	/*
	 * Get the realm of the host
	 */
	strcpy((char *)scratch2, (char *)cp);
	if (scratch2[strlen((char *)scratch2)-1] == '.')
		scratch2[strlen((char *)scratch2)-1] = '\0';
	xbzero(klopb, sizeof(*klopb));
	klopb->host = (char *)scratch2;
	klopb->uRealm = (char *)scratch1;
	if (s = lowcall(cKrbGetRealm, klopb, kdriver))
		return -1;

	/* 
	 * build rcmd.hostname@realm
	 */
	strcpy((char *)cannon, "rcmd.");
	cn = &cannon[5];
	strncpy((char *)cn, (char *)cp, sizeof(cannon)-5);
	cannon[sizeof(cannon)-1] = '\0';
	if (cp = (unsigned char *)strchr((char *)cn, '.'))
		*cp = '\0';
	strcat((char *)cannon, "@");
	strcat((char *)cannon, (char *)scratch1);
	
	/* 
	 * use cKrbGetAuthForService to get auth data for telnet option 
	 *
	 * Similiar to cKrbGetTicketForService except it builds a kerberos "SendAuth" style
	   request (with SendAuth and application version numbers preceeding the actual ticket)
		  -> service				Formal Kerberos name of service
		  -> buf					Buffer to receive ticket
		  -> checksum				checksum for this service
	 	 <-> buflen					length of ticket buffer (must be at least 1258 bytes)
		 <-  sessionKey				for internal use
		 <-  schedule				for internal use
	 */

	xbzero(khipb, sizeof(krbHiParmBlock));
	khipb->service = (char *)cannon;
	khipb->buf = (char *)buf;				/* where to build it */
	khipb->checksum = 0;
	khipb->buflen = sizeof(buf);
	if (s = hicall(cKrbGetTicketForService, khipb, kdriver))
		return s;
	xbcopy(khipb->sessionKey, sessionKey, sizeof(sessionKey));	/* save the session key */
	/*
	 * cKrbGetTicketForService put a longword buffer length into the buffer
	 * which we don't want, so we ignore it.
     * Make room for first 3 bytes which preceed the auth data.
	 */
	cp = &buf[4-3];						/* skip long, make room for 3 bytes */
	cp[0] = tp[0];						/* copy type and modifier */
	cp[1] = tp[1];
	cp[2] = KRB_AUTH;					/* suboption command */
	len = khipb->buflen - sizeof(long) + 3; /* data - 4 + 3 */

	xbzero(khipb, sizeof(krbHiParmBlock));	/* get kerberos user name */
	khipb->user = (char *)kname;
	if (s = hicall(cKrbGetUserName, khipb, kdriver))
		return s;
	if (kp = (unsigned char *)strchr((char *)kname, '@'))
		*kp = '\0';
		
	tn_sendsub(cr, OPT_AUTHENTICATION, TNQ_NAME, (char *)kname, strlen((char *)kname));
	tn_sendsub(cr, OPT_AUTHENTICATION, TNQ_IS, (char *)cp, len);

	/*
	 * If we are going to do mutual authentication, go ahead and 
	 * generate the challenge and response here so we won't have to figure
	 * out the credentials again later. 
	 */
	if (tp[1] & AUTH_HOW_MUTUAL) {
		unsigned char challenge[8];
		des_key_schedule sched;

		des_init_random_number_generator(sessionKey);

		des_key_sched(sessionKey, sched);
		des_set_random_generator_seed((des_cblock *)sessionKey);
		des_new_random_key(challenge);
		des_ecb_encrypt((unsigned long *)challenge, (unsigned long *)cr->auth_challenge, sched, 1);
		skey.type = SK_DES;						/* save key for ENCRYPT */
		skey.length = 8;
		skey.data = (unsigned char *)cr->auth_challenge;
		encrypt_session_key(cr->edata, &skey, 0);
		/*
		 * Increment the challenge by 1, and encrypt it for
		 * later comparison.
		 */
		for (i = 7; i >= 0; --i) {
			register long x;

			x = (unsigned long)challenge[i] + 1;
			challenge[i] = x;			/* ignore overflow */
			if (x < 256)				/* if no overflow, all done */
				break;
		}
		des_ecb_encrypt((unsigned long *)challenge, (unsigned long *)cr->auth_response, sched, 1);
		skey.type = SK_DES;						/* save key for ENCRYPT */
		skey.length = 8;
		skey.data = (unsigned char *)cr->auth_response;
		encrypt_session_key(cr->edata, &skey, 1);
	}

	return 0;
}


/*
 * tn_kerberos_reply
 * Process a AUTHENTICATION REPLY of type KRBEROS_V4.
 */
void tn_kerberos_reply (WDATA *tw, unsigned char *subbuffer)
{
    switch (subbuffer[SB_DATATYPE] & 0xff) {
    case KRB_AUTH:
        break;
    case KRB_REJECT:
        break;
    case KRB_ACCEPT:
        /*
         * If mutual authentication, it is time to send the challenge.
         */
        if (subbuffer[SB_MODIFIER] & AUTH_HOW_MUTUAL)
            tn_send_challenge(tw, subbuffer);
        break;
    case KRB_CHALLENGE:
        break;
    case KRB_RESPONSE:
        /*
         * We got a response to an authorization challenge. Make
         * sure it is the correct response.
         */
#ifdef notdef /* ... ??? */
		if (xbcmp(&subbuffer[SB_DATA], tw->auth_response, 8))
            /*
             * ??? What do we do here? -- presumably clear the connection.
             */
            printf("\n[Challenge failed]\n");
        else
            printf("\n[Challenge succeeded]\n");
#endif
		break;
    }
}


/*
 * tn_send_challenge
 * Send a challenge
 */
void tn_send_challenge (WDATA *tw, unsigned char *subbuffer)
{
    char buf[2+1+8];

    buf[0] = subbuffer[SB_TYPE];
    buf[1] = subbuffer[SB_MODIFIER];
    buf[2] = KRB_CHALLENGE;
    xbcopy(tw->auth_challenge, &buf[3], 8);

    tn_sendsub(tw, OPT_AUTHENTICATION, TNQ_IS, buf, sizeof(buf));
}


short lowcall (long cscode, krbParmBlock *klopb, short kdriver)
{
	short s;
	ParamBlockRec pb;
	
	xbzero(&pb, sizeof(ParamBlockRec));
	*(long *)pb.cntrlParam.csParam = (long)klopb;
	pb.cntrlParam.ioCompletion = nil;
	pb.cntrlParam.ioCRefNum = kdriver;

	pb.cntrlParam.csCode = cscode;
	if (s = PBControl(&pb, false))
		return s;
	if (s = pb.cntrlParam.ioResult)
		return s;
	return 0;
}


short hicall (long cscode, krbHiParmBlock *khipb, short kdriver)
{
	short s;
	ParamBlockRec pb;
	
	xbzero(&pb, sizeof(ParamBlockRec));
	*(long *)pb.cntrlParam.csParam = (long)khipb;
	pb.cntrlParam.ioCompletion = nil;
	pb.cntrlParam.ioCRefNum = kdriver;

	pb.cntrlParam.csCode = cscode;
	if (s = PBControl(&pb, false))
		return s;
	if (s = pb.cntrlParam.ioResult)
		return s;
	return 0;
}


/*
 * bcopy
 * Block copy
 */
void xbcopy (void *src, void *dst, long n)
{
	long i;
	register unsigned char *s = src;
	register unsigned char *d = dst;

	for (i = 0; i < n; i++)
		*d++ = *s++;
}


/*
 * bcmp
 * return 0 if blocks compare ok
 */
long xbcmp (void *src, void *dst, long n)
{
	register char *s = src;
	register char *d = dst;
	
	while (n-- > 0)
		if (*s++ != *d++)
			return 1;
			
	return 0;
}


/*
 * bzero
 * Block zero
 */
void xbzero (void *dst, long n)
{
	register char *d = dst;

	while (n--)
		*d++ = 0;
}
