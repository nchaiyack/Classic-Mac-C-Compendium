/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:	 ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)encrypt.c	5.1 (Berkeley) 2/28/91";
#endif /* not lint */

/*
 * Copyright (C) 1990 by the Massachusetts Institute of Technology
 *
 * Export of this software from the United States of America is assumed
 * to require a specific license from the United States Government.
 * It is the responsibility of any person or organization contemplating
 * export to obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.	M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
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
#endif /* tn3270 */

#ifdef NCSA_ENC
#include "TelnetHeader.h"
#include "wind.h"
#include "parse.proto.h"
#endif

#include <stdarg.h>

#define TELCMDS 1
#define TELOPTS 1

#define ENCRYPT_NAMES

#include "encrypt.h"

#include "enc_des.proto.h"
#include "encrypt.proto.h"
#include "kerberos.proto.h"
/* #include "wdefpatch.proto.h" */

#ifdef	__STDC__
#include <stdlib.h>
#endif
#ifdef	NO_STRING_H
#include <strings.h>
#else
#include <string.h>
#endif

#define typemask(x)		((x) > 0 ? 1 << ((x)-1) : 0)
#define kOurHit	32

static Encryptions encryptions[] = {
	{ "DES_CFB64",		ENCTYPE_DES_CFB64,
						(void(*)()) cfb64_encrypt,	
						(long(*)()) cfb64_decrypt,
						(void(*)()) cfb64_init,
						(long(*)()) cfb64_start,
						(long(*)()) cfb64_is,
						(long(*)()) cfb64_reply,
						(void(*)()) cfb64_session,
						(long(*)()) cfb64_keyid,
						(void(*)()) cfb64_printsub },
	{ "DES_OFB64",		ENCTYPE_DES_OFB64,
						(void(*)()) ofb64_encrypt,	
						(long(*)()) ofb64_decrypt,
						(void(*)()) ofb64_init,
						(long(*)()) ofb64_start,
						(long(*)()) ofb64_is,
						(long(*)()) ofb64_reply,
						(void(*)()) ofb64_session,
						(long(*)()) ofb64_keyid,
						(void(*)()) ofb64_printsub },
	{ 0, },
};

#define I_SUPPORT_ENCRYPT		(tw->i_support_encrypt & ~tw->i_wont_support_encrypt)
#define I_SUPPORT_DECRYPT		(tw->i_support_decrypt & ~tw->i_wont_support_decrypt)

static unsigned char str_send_init[] = { IAC, SB, OPT_ENCRYPT,
										 ENCRYPT_SUPPORT };
static unsigned char str_start_init[] = { IAC, SB, OPT_ENCRYPT };
static unsigned char str_end_init[] = { IAC, SB, OPT_ENCRYPT, 0, IAC, SE };

static struct key_info ki_init[2] = {
    { { 0 }, 0, DIR_ENCRYPT, 0, findencryption },
	{ { 0 }, 0, DIR_DECRYPT, 0, finddecryption },
};

#ifdef	ENCRYPT_NAMES
char *encrypt_names[] = {
	"IS", "SUPPORT", "REPLY", "START", "END",
	"REQUEST-START", "REQUEST-END", "ENC-KEYID", "DEC-KEYID",
	0,
};
char *enctype_names[] = {
	"ANY", "DES_CFB64",  "DES_OFB64",  0,
};
#else
extern char *encrypt_names[];
extern char *enctype_names[];
#endif


#define	ENCRYPT_NAME_OK(x)	((x) >= 0 && (x) < ENCRYPT_CNT)
#define	ENCRYPT_NAME(x)		encrypt_names[x]

#define	ENCTYPE_NAME_OK(x)	((x) >= 0 && (x) < ENCTYPE_CNT)
#define	ENCTYPE_NAME(x)		enctype_names[x]

Encryptions *findencryption (CDATA *tw, long type)
{
	Encryptions *ep = encryptions;

	if (!(I_SUPPORT_ENCRYPT & tw->remote_supports_decrypt & typemask(type)))
		return(0);
	while (ep->type && ep->type != type)
		++ep;
	return(ep->type ? ep : 0);
}

Encryptions *finddecryption (CDATA *tw, long type)
{
	Encryptions *ep = encryptions;

	if (!(I_SUPPORT_DECRYPT & tw->remote_supports_encrypt & typemask(type)))
		return(0);
	while (ep->type && ep->type != type)
		++ep;
	return(ep->type ? ep : 0);
}

void encrypt_init (CDATA *tw, char *name, long server)
{
	Encryptions *ep = encryptions;

//	tw->will_wont_resp_encrypt = 0;
//	tw->do_dont_resp_encrypt = 0;
//	tw->o_encrypt = 0;
	tw->encrypt_debug_mode = 0; /* TelInfo->debug; ddd */
	tw->Name = name;
	tw->Server = server;
	tw->i_support_encrypt = tw->i_support_decrypt = 0;
	tw->remote_supports_encrypt = tw->remote_supports_decrypt = 0;
	tw->encrypt_mode = 0;
	tw->decrypt_mode = 0;
	tw->encrypt_output = 0;
	tw->decrypt_input = 0;
	tw->encrypt_verbose = 0; /* TelInfo->debug; ddd */
	tw->autoencrypt = 1;				/* ... was 0 */
	tw->autodecrypt = 1;				/* ... was 0 */
	tw->havesessionkey = 0;
	tw->i_wont_support_encrypt = 0;
	tw->i_wont_support_decrypt = 0;
	xbcopy(str_send_init, tw->str_send, sizeof(str_send_init));
	xbcopy(str_start_init, tw->str_start, sizeof(str_start_init));
	xbcopy(str_end_init, tw->str_end, sizeof(str_end_init));
	xbcopy(ki_init, tw->ki, sizeof(ki_init));
	tw->ki[0].modep = &tw->encrypt_mode;
	tw->ki[1].modep = &tw->decrypt_mode;
	
	tw->str_suplen = 4;

	while (ep->type) {
		if (tw->encrypt_debug_mode)
			xprintf(tw, ">>>%s: I will support %s\r\n",
				   tw->Name, ENCTYPE_NAME(ep->type));
		tw->i_support_encrypt |= typemask(ep->type);
		tw->i_support_decrypt |= typemask(ep->type);
		if ((tw->i_wont_support_decrypt & typemask(ep->type)) == 0)
			if ((tw->str_send[tw->str_suplen++] = ep->type) == IAC)
				tw->str_send[tw->str_suplen++] = IAC;
		if (ep->init)
			(*ep->init)(tw, tw->Server);
		++ep;
	}
	tw->str_send[tw->str_suplen++] = IAC;
	tw->str_send[tw->str_suplen++] = SE;
}

#ifdef notdef
void encrypt_list_types ()
{
	Encryptions *ep = encryptions;

	xprintf(tw, "Valid encryption types:\n");
	while (ep->type) {
   		xprintf(tw, "\t%s (%d)\r\n", ENCTYPE_NAME(ep->type), ep->type);
		++ep;
	}
}


long EncryptEnable (CDATA *tw, char *type, char *mode)
{
	if (isprefix(type, "help") || isprefix(type, "?")) {
		xprintf(tw, "Usage: encrypt enable <type> [input|output]\n");
		encrypt_list_types();
		return(0);
	}
	if (EncryptType(tw, type, mode))
		return(EncryptStart(tw, mode));
	return(0);
}

long EncryptDisable (CDATA *tw, char *type, char *mode)
{
	register Encryptions *ep;
	long ret = 0;

	if (isprefix(type, "help") || isprefix(type, "?")) {
		xprintf(tw, "Usage: encrypt disable <type> [input|output]\n");
		encrypt_list_types();
	} else if ((ep = (Encryptions *)genget(type, encryptions,
										   sizeof(Encryptions))) == 0) {
		xprintf(tw, "%s: invalid encryption type\n", type);
	} else if (Ambiguous(ep)) {
		xprintf(tw, "Ambiguous type '%s'\n", type);
	} else {
		if ((mode == 0) || (isprefix(mode, "input") ? 1 : 0)) {
			if (tw->decrypt_mode == ep->type)
				EncryptStopInput(tw);
			tw->i_wont_support_decrypt |= typemask(ep->type);
			ret = 1;
		}
		if ((mode == 0) || (isprefix(mode, "output"))) {
			if (tw->encrypt_mode == ep->type)
				EncryptStopOutput(tw);
			tw->i_wont_support_encrypt |= typemask(ep->type);
			ret = 1;
		}
		if (ret == 0)
			xprintf(tw, "%s: invalid encryption mode\n", mode);
	}
	return(ret);
}

long EncryptType (CDATA *tw, char *type, char *mode)
{
	register Encryptions *ep;
	long ret = 0;

	if (isprefix(type, "help") || isprefix(type, "?")) {
		xprintf(tw, "Usage: encrypt type <type> [input|output]\n");
		encrypt_list_types();
	} else if ((ep = (Encryptions *)genget(type, encryptions,
										   sizeof(Encryptions))) == 0) {
		xprintf(tw, "%s: invalid encryption type\n", type);
	} else if (Ambiguous(ep)) {
		xprintf(tw, "Ambiguous type '%s'\n", type);
	} else {
		if ((mode == 0) || isprefix(mode, "input")) {
			tw->decrypt_mode = ep->type;
			tw->i_wont_support_decrypt &= ~typemask(ep->type);
			ret = 1;
		}
		if ((mode == 0) || isprefix(mode, "output")) {
			tw->encrypt_mode = ep->type;
			tw->i_wont_support_encrypt &= ~typemask(ep->type);
			ret = 1;
		}
		if (ret == 0)
			xprintf(tw, "%s: invalid encryption mode\n", mode);
	}
	return(ret);
}

long EncryptStart (CDATA *tw, char *mode)
{
	register long ret = 0;

	if (mode) {
		if (isprefix(mode, "input"))
			return(EncryptStartInput(tw));
		if (isprefix(mode, "output"))
			return(EncryptStartOutput(tw));
		if (isprefix(mode, "help") || isprefix(mode, "?")) {
			xprintf(tw, "Usage: encrypt start [input|output]\n");
			return(0);
		}
		xprintf(tw, "%s: invalid encryption mode 'encrypt start ?' for help\n", mode);
		return(0);
	}
	ret += EncryptStartInput(tw);
	ret += EncryptStartOutput(tw);
	return(ret);
}

long EncryptStartInput (CDATA *tw)
{
	if (tw->decrypt_mode) {
		encrypt_send_request_start(tw);
		return(1);
	}
	xprintf(tw, "No previous decryption mode, decryption not enabled\r\n");
	return(0);
}

long EncryptStartOutput (CDATA *tw)
{
	if (tw->encrypt_mode) {
		encrypt_start_output(tw, tw->encrypt_mode);
		return(1);
	}
	xprintf(tw, "No previous encryption mode, encryption not enabled\r\n");
	return(0);
}

long EncryptStop (CDATA *tw, char *mode)
{
	long ret = 0;
	if (mode) {
		if (isprefix(mode, "input"))
			return(EncryptStopInput(tw));
		if (isprefix(mode, "output"))
			return(EncryptStopOutput(tw));
		if (isprefix(mode, "help") || isprefix(mode, "?")) {
			xprintf(tw, "Usage: encrypt stop [input|output]\n");
			return(0);
		}
		xprintf(tw, "%s: invalid encryption mode 'encrypt stop ?' for help\n", mode);
		return(0);
	}
	ret += EncryptStopInput(tw);
	ret += EncryptStopOutput(tw);
	return(ret);
}

long EncryptStopInput (CDATA *tw)
{
	encrypt_send_request_end(tw);
	return(1);
}

long EncryptStopOutput (CDATA *tw)
{
	encrypt_send_end(tw);
	return(1);
}

void encrypt_display (CDATA *tw)
{
	if (tw->encrypt_output)
		xprintf(tw, "Currently encrypting output with %s\r\n",
			   ENCTYPE_NAME(tw->encrypt_mode));
	if (tw->decrypt_input)
		xprintf(tw, "Currently decrypting input with %s\r\n",
			   ENCTYPE_NAME(tw->decrypt_mode));
}

long EncryptStatus (CDATA *tw)
{
	if (tw->encrypt_output)
		xprintf(tw, "Currently encrypting output with %s\r\n",
			   ENCTYPE_NAME(tw->encrypt_mode));
	else if (tw->encrypt_mode) {
		xprintf(tw, "Currently output is clear text.\r\n");
		xprintf(tw, "Last encryption mode was %s\r\n",
			   ENCTYPE_NAME(tw->encrypt_mode));
	}
	if (tw->decrypt_input) {
		xprintf(tw, "Currently decrypting input with %s\r\n",
			   ENCTYPE_NAME(tw->decrypt_mode));
	} else if (tw->decrypt_mode) {
		xprintf(tw, "Currently input is clear text.\r\n");
		xprintf(tw, "Last decryption mode was %s\r\n",
			   ENCTYPE_NAME(tw->decrypt_mode));
	}
	return 1;
}
#endif

void encrypt_send_support (CDATA *tw)
{
	if (tw->str_suplen) {
		/*
		 * If the user has requested that decryption start
		 * immediatly, then send a "REQUEST START" before
		 * we negotiate the type.
		 */
		if (!tw->Server && tw->autodecrypt)
			encrypt_send_request_start(tw);
		net_write(tw->wp, (char *)tw->str_send, tw->str_suplen);
/* ddd	printsub(">", &tw->str_send[2], tw->str_suplen - 2, tw); */
		tw->str_suplen = 0;
	}
}

long EncryptDebug (CDATA *tw, long on)
{
	if (on < 0)
		tw->encrypt_debug_mode ^= 1;
	else
		tw->encrypt_debug_mode = on;
	xprintf(tw, "Encryption debugging %s\r\n",
		   tw->encrypt_debug_mode ? "enabled" : "disabled");
	return(1);
}

long EncryptVerbose (CDATA *tw, long on)
{
	if (on < 0)
		tw->encrypt_verbose ^= 1;
	else
		tw->encrypt_verbose = on;
	xprintf(tw, "Encryption %s verbose\r\n",
		   tw->encrypt_verbose ? "is" : "is not");
	return(1);
}

long EncryptAutoEnc (CDATA *tw, long on)
{
	encrypt_auto(tw, on);
	xprintf(tw, "Automatic encryption of output is %s\r\n",
		   tw->autoencrypt ? "enabled" : "disabled");
	return(1);
}

long EncryptAutoDec (CDATA *tw, long on)
{
	decrypt_auto(tw, on);
	xprintf(tw, "Automatic decryption of input is %s\r\n",
		   tw->autodecrypt ? "enabled" : "disabled");
	return(1);
}


/*
 * Called when ENCRYPT SUPPORT is received.
 */
void encrypt_support (CDATA *tw, unsigned char *typelist, long cnt)
{
	register long type, use_type = 0;
	Encryptions *ep;

	/*
	 * Forget anything the other side has previously told us.
	 */
	tw->remote_supports_decrypt = 0;

	while (cnt-- > 0) {
		type = *typelist++;
		if (tw->encrypt_debug_mode)
			xprintf(tw, ">>>%s: He is supporting %s (%d)\r\n",
				   tw->Name,
				   ENCTYPE_NAME(type), type);
		if ((type < ENCTYPE_CNT) &&
			(I_SUPPORT_ENCRYPT & typemask(type))) {
			tw->remote_supports_decrypt |= typemask(type);
			if (use_type == 0)
				use_type = type;
		}
	}
	if (use_type) {
		ep = findencryption(tw, use_type);
		if (!ep)
			return;
		type = ep->start ? (*ep->start)(tw, DIR_ENCRYPT, tw->Server) : 0;
		if (tw->encrypt_debug_mode)
			xprintf(tw, ">>>%s: (*ep->start)() returned %d\r\n",
				   tw->Name, type);
		if (type < 0)
			return;
		tw->encrypt_mode = use_type;
		if (type == 0)
			encrypt_start_output(tw, use_type);
	}
}

void encrypt_is (CDATA *tw, unsigned char *data, long cnt)
{
	Encryptions *ep;
	register long type, ret;

	if (--cnt < 0)
		return;
	type = *data++;
	if (type < ENCTYPE_CNT)
		tw->remote_supports_encrypt |= typemask(type);
	if (!(ep = finddecryption(tw, type))) {
		if (tw->encrypt_debug_mode)
			xprintf(tw, ">>>%s: Can't find type %s (%d) for initial negotiation\r\n",
				   tw->Name,
				   ENCTYPE_NAME_OK(type)
				   ? ENCTYPE_NAME(type) : "(unknown)",
				   type);
		return;
	}
	if (!ep->is) {
		if (tw->encrypt_debug_mode)
			xprintf(tw, ">>>%s: No initial negotiation needed for type %s (%d)\r\n",
				   tw->Name,
				   ENCTYPE_NAME_OK(type)
				   ? ENCTYPE_NAME(type) : "(unknown)",
				   type);
		ret = 0;
	} else {
		ret = (*ep->is)(tw, data, cnt);
		if (tw->encrypt_debug_mode)
			xprintf(tw, "(*ep->is)(%x, %d) returned %s(%d)\n", data, cnt,
				   (ret < 0) ? "FAIL " :
				   (ret == 0) ? "SUCCESS " : "MORE_TO_DO ", ret);
	}
	if (ret < 0) {
		tw->autodecrypt = 0;
	} else {
		tw->decrypt_mode = type;
		if (ret == 0 && tw->autodecrypt)
			encrypt_send_request_start(tw);
	}
}

void encrypt_reply (CDATA *tw, unsigned char *data, long cnt)
{
	Encryptions *ep;
	register long ret, type;

	if (--cnt < 0)
		return;
	type = *data++;
	if (!(ep = findencryption(tw, type))) {
		if (tw->encrypt_debug_mode)
			xprintf(tw, ">>>%s: Can't find type %s (%d) for initial negotiation\r\n",
				   tw->Name,
				   ENCTYPE_NAME_OK(type)
				   ? ENCTYPE_NAME(type) : "(unknown)",
				   type);
		return;
	}
	if (!ep->reply) {
		if (tw->encrypt_debug_mode)
			xprintf(tw, ">>>%s: No initial negotiation needed for type %s (%d)\r\n",
				   tw->Name,
				   ENCTYPE_NAME_OK(type)
				   ? ENCTYPE_NAME(type) : "(unknown)",
				   type);
		ret = 0;
	} else {
		ret = (*ep->reply)(tw, data, cnt);
		if (tw->encrypt_debug_mode)
			xprintf(tw, "(*ep->reply)(%x, %d) returned %s(%d)\n",
				   data, cnt,
				   (ret < 0) ? "FAIL " :
				   (ret == 0) ? "SUCCESS " : "MORE_TO_DO ", ret);
	}
	if (tw->encrypt_debug_mode)
		xprintf(tw, ">>>%s: encrypt_reply returned %d\n", tw->Name, ret);
	if (ret < 0) {
		tw->autoencrypt = 0;
	} else {
		tw->encrypt_mode = type;
		if (ret == 0 && tw->autoencrypt)
			encrypt_start_output(tw, type);
	}
}

/*
 * Called when a ENCRYPT START command is received.
 */
void encrypt_start (CDATA *tw, unsigned char *data, long cnt)
{
	#pragma unused (data, cnt)
	Encryptions *ep;

	if (!tw->decrypt_mode) {
		/*
		 * Something is wrong.	We should not get a START
		 * command without having already picked our
		 * decryption scheme.  Send a REQUEST-END to
		 * attempt to clear the channel...
		 */
		xprintf(tw, "%s: Warning, Cannot decrypt input stream!!!\r\n", tw->Name);
		encrypt_send_request_end(tw);
		return;
	}

	if (ep = finddecryption(tw, tw->decrypt_mode)) {
		tw->decrypt_input = ep->input;
		encryptStatechange(tw->wp);
#ifdef notdef /* ddd */
		(void) MyWDEFPatch(zoomDocProc , tw->wind, wDraw, kOurHit);
#endif
		if (tw->encrypt_verbose)
			xprintf(tw, "[ Input is now decrypted with type %s ]\r\n",
				   ENCTYPE_NAME(tw->decrypt_mode));
		if (tw->encrypt_debug_mode)
			xprintf(tw, ">>>%s: Start to decrypt input with type %s\r\n",
				   tw->Name, ENCTYPE_NAME(tw->decrypt_mode));
	} else {
		xprintf(tw, "%s: Warning, Cannot decrypt type %s (%d)!!!\r\n",
			   tw->Name,
			   ENCTYPE_NAME_OK(tw->decrypt_mode)
			   ? ENCTYPE_NAME(tw->decrypt_mode)
			   : "(unknown)",
			   tw->decrypt_mode);
		encrypt_send_request_end(tw);
	}
}

void encrypt_session_key (CDATA *tw, Session_Key *key, long server)
{
	Encryptions *ep = encryptions;

	tw->havesessionkey = 1;

	while (ep->type) {
		if (ep->session)
			(*ep->session)(tw, key, server);
#ifdef notdef
		if (!tw->encrypt_output && tw->autoencrypt && !server)
			encrypt_start_output(tw, ep->type);
		if (!tw->decrypt_input && tw->autodecrypt && !server)
			encrypt_send_request_start(tw);
#endif
		++ep;
	}
}

/*
 * Called when ENCRYPT END is received.
 */
void encrypt_end (CDATA *tw)
{
	tw->decrypt_input = 0;
	encryptStatechange(tw->wp);
#ifdef notdef /* ddd */
	(void) MyWDEFPatch(zoomDocProc , tw->wind, wDraw, kOurHit);
#endif
	if (tw->encrypt_debug_mode)
		xprintf(tw, ">>>%s: Input is back to clear text\r\n", tw->Name);
	if (tw->encrypt_verbose)
		xprintf(tw, "[ Input is now clear text ]\r\n");
}

/*
 * Called when ENCRYPT REQUEST-END is received.
 */
void encrypt_request_end (CDATA *tw)
{
	encrypt_send_end(tw);
}

/*
 * Called when ENCRYPT REQUEST-START is received.  If we receive
 * this before a type is picked, then that indicates that the
 * other side wants us to start encrypting data as soon as we
 * can. 
 */
void encrypt_request_start (CDATA *tw, unsigned char *data, long cnt)
{
	#pragma unused (data, cnt)
	if (tw->encrypt_mode == 0)	{
		if (tw->Server)
			tw->autoencrypt = 1;
		return;
	}
	encrypt_start_output(tw, tw->encrypt_mode);
}

void encrypt_enc_keyid (CDATA *tw, unsigned char *keyid, long len)
{
	encrypt_keyid(tw, &tw->ki[1], keyid, len);
}

void encrypt_dec_keyid (CDATA *tw, unsigned char *keyid, long len)
{
	encrypt_keyid(tw, &tw->ki[0], keyid, len);
}

void encrypt_keyid (CDATA *tw, struct key_info *kp, unsigned char *keyid, long len)
{
	Encryptions *ep;
#ifdef notdef
	unsigned char *strp, *cp;
#endif
	long dir = kp->dir;
	register long ret = 0;

	if (!(ep = (*kp->getcrypt)(tw, *kp->modep))) {
		if (len == 0)
			return;
		kp->keylen = 0;
	} else if (len == 0) {
		/*
		 * Empty option, indicates a failure.
		 */
		if (kp->keylen == 0)
			return;
		kp->keylen = 0;
		if (ep->keyid)
			(void)(*ep->keyid)(tw, dir, kp->keyid, &kp->keylen);

	} else if ((len != kp->keylen) || (xbcmp(keyid, kp->keyid, len) != 0)) {
		/*
		 * Length or contents are different
		 */
		kp->keylen = len;
		xbcopy(keyid, kp->keyid, len);
		if (ep->keyid)
			(void)(*ep->keyid)(tw, dir, kp->keyid, &kp->keylen);

		if (tw->encrypt_debug_mode)
			xprintf(tw, "encrypt_keyid: different dir=%d len=%d %d\n", dir, len, kp->keylen);
	} else {
		if (ep->keyid)
			ret = (*ep->keyid)(tw, dir, kp->keyid, &kp->keylen);

		if (tw->encrypt_debug_mode)
			xprintf(tw, "encrypt_keyid: ret=%d dir=%d autoe=%d\n",
					ret, dir, tw->autoencrypt);

		if ((ret == 0) && (dir == DIR_ENCRYPT) && tw->autoencrypt)
			encrypt_start_output(tw, *kp->modep);
		return;
	}

	encrypt_send_keyid(tw, dir, kp->keyid, kp->keylen, 0);
}


void encrypt_send_keyid (CDATA *tw, long dir, unsigned char *keyid, long keylen, long saveit)
{
	unsigned char *strp;
	
	tw->str_keyid[0] = IAC;
	tw->str_keyid[1] = SB;
	tw->str_keyid[2] = OPT_ENCRYPT;
	tw->str_keyid[3] = (dir == DIR_ENCRYPT)
		? ENCRYPT_ENC_KEYID : ENCRYPT_DEC_KEYID;

	if (saveit) {
		struct key_info *kp = &tw->ki[(dir == DIR_ENCRYPT) ? 0 : 1];

		if (tw->encrypt_debug_mode)
			xprintf(tw, "encrypt_send_keyid: saveit dir=%d keylen=%d id=%d\n", dir, keylen, *keyid);

		xbcopy(keyid, kp->keyid, keylen);
		kp->keylen = keylen;
	}

	for (strp = &tw->str_keyid[4]; keylen > 0; --keylen) {
		if ((*strp++ = *keyid++) == IAC)
			*strp++ = IAC;
	}
	*strp++ = IAC;
	*strp++ = SE;
	net_write(tw->wp, (char *)tw->str_keyid, strp - tw->str_keyid);
/*	printsub(">", &tw->str_keyid[2], strp - tw->str_keyid - 2, tw); ddd */
}

void encrypt_auto (CDATA *tw, long on)
{
	if (on < 0)
		tw->autoencrypt ^= 1;
	else
		tw->autoencrypt = on ? 1 : 0;
}

void decrypt_auto (CDATA *tw, long on)
{
	if (on < 0)
		tw->autodecrypt ^= 1;
	else
		tw->autodecrypt = on ? 1 : 0;
}

void encrypt_start_output (CDATA *tw, long type)
{
	Encryptions *ep;
	register unsigned char *p;
	register long i;

	if (!(ep = findencryption(tw, type))) {
		if (tw->encrypt_debug_mode) {
			xprintf(tw, ">>>%s: Can't encrypt with type %s (%d)\r\n",
				   tw->Name,
				   ENCTYPE_NAME_OK(type)
				   ? ENCTYPE_NAME(type) : "(unknown)",
				   type);
		}
		return;
	}
	if (ep->start) {
		i = (*ep->start)(tw, DIR_ENCRYPT, tw->Server);
		if (tw->encrypt_debug_mode) {
			xprintf(tw, ">>>%s: Encrypt start: %s (%d) %s\r\n",
				   tw->Name, 
				   (i < 0) ? "failed" :
				   "initial negotiation in progress",
				   i, ENCTYPE_NAME(type));
		}
		if (i)
			return;
	}
	p = tw->str_start + 3;
	*p++ = ENCRYPT_START;
	for (i = 0; i < tw->ki[0].keylen; ++i) {
		if ((*p++ = tw->ki[0].keyid[i]) == IAC)
			*p++ = IAC;
	}
	*p++ = IAC;
	*p++ = SE;
	
	net_write(tw->wp, (char *)tw->str_start, p - tw->str_start);
	net_encrypt();
/*	printsub(">", &tw->str_start[2], p - &tw->str_start[2], tw); ddd */
	/*
	 * If we are already encrypting in some mode, then
	 * encrypt the ring (which includes our request) in
	 * the old mode, mark it all as "clear text" and then
	 * switch to the new mode.
	 */
	tw->encrypt_output = ep->output;
	encryptStatechange(tw->wp);
#ifdef notdef /* ddd */	
	(void) MyWDEFPatch(zoomDocProc , tw->wind, wDraw, kOurHit);
#endif
	tw->encrypt_mode = type;
	if (tw->encrypt_debug_mode)
		xprintf(tw, ">>>%s: Started to encrypt output with type %s\r\n",
			   tw->Name, ENCTYPE_NAME(type));
	if (tw->encrypt_verbose)
		xprintf(tw, "[ Output is now encrypted with type %s ]\r\n",
			   ENCTYPE_NAME(type));
}

void encrypt_send_end (CDATA *tw)
{
	if (!tw->encrypt_output)
		return;

	tw->str_end[3] = ENCRYPT_END;
	net_write(tw->wp, (char *)tw->str_end, sizeof(tw->str_end));
	net_encrypt();
/*	printsub(">", &tw->str_end[2], sizeof(tw->str_end) - 2, tw); ddd */
	/*
	 * Encrypt the output buffer now because it will not be done by
	 * netflush...
	 */
	tw->encrypt_output = 0;
	encryptStatechange(tw->wp);
#ifdef notdef /* ddd */
	(void) MyWDEFPatch(zoomDocProc , tw->wind, wDraw, kOurHit);
#endif
	if (tw->encrypt_debug_mode)
		xprintf(tw, ">>>%s: Output is back to clear text\r\n", tw->Name);
	if (tw->encrypt_verbose)
		xprintf(tw, "[ Output is now clear text ]\r\n");
}

void encrypt_send_request_start (CDATA *tw)
{
	register unsigned char *p;
	register long i;

	p = &tw->str_start[3];
	*p++ = ENCRYPT_REQSTART;
	for (i = 0; i < tw->ki[1].keylen; ++i) {
		if ((*p++ = tw->ki[1].keyid[i]) == IAC)
			*p++ = IAC;
	}
	*p++ = IAC;
	*p++ = SE;
	net_write(tw->wp, (char *)tw->str_start, p - tw->str_start);
/*	printsub(">", &tw->str_start[2], p - &tw->str_start[2], tw); ddd */
	if (tw->encrypt_debug_mode)
		xprintf(tw, ">>>%s: Request input to be encrypted\r\n", tw->Name);
}

void encrypt_send_request_end (CDATA *tw)
{
	tw->str_end[3] = ENCRYPT_REQEND;
	net_write(tw->wp, (char *)tw->str_end, sizeof(tw->str_end));
/*	printsub(">", &tw->str_end[2], sizeof(tw->str_end) - 2, tw); ddd */

	if (tw->encrypt_debug_mode)
		xprintf(tw, ">>>%s: Request input to be clear text\r\n", tw->Name);
}

#ifdef notdef
void encrypt_wait (CDATA *tw)
{
#ifdef notdef
	register long encrypt, decrypt;
#endif

	if (tw->encrypt_debug_mode)
		xprintf(tw, ">>>%s: in encrypt_wait\r\n", tw->Name);
	if (!tw->havesessionkey || !(I_SUPPORT_ENCRYPT & tw->remote_supports_decrypt))
		return;
	while (tw->autoencrypt && !tw->encrypt_output)
		if (telnet_spin())
			return;
}
#endif

void encrypt_debug (CDATA *tw, long mode)
{
	tw->encrypt_debug_mode = mode;
}

#ifdef notdef
void encrypt_gen_printsub (unsigned char *data, long cnt, unsigned char *buf, long buflen)
{
	char tbuf[16], *cp;

	cnt -= 2;
	data += 2;
	buf[buflen-1] = '\0';
	buf[buflen-2] = '*';
	buflen -= 2;;
	for (; cnt > 0; cnt--, data++) {
		sprintf(tbuf, " %d", *data);
		for (cp = tbuf; *cp && buflen > 0; --buflen)
			*buf++ = *cp++;
		if (buflen <= 0)
			return;
	}
	*buf = '\0';
}

void encrypt_printsub (unsigned char *data, long cnt, unsigned char *buf, long buflen)
{
	Encryptions *ep;
	register long type = data[1];

	for (ep = encryptions; ep->type && ep->type != type; ep++)
		;

	if (ep->printsub)
		(*ep->printsub)(data, cnt, buf, buflen);
	else
		encrypt_gen_printsub(data, cnt, buf, buflen);
}
#endif

#ifdef notdef
void xxxprintsub (CDATA *tw, char direction, unsigned char *pointer, long length)
{
	char buf[256];

	if (tw->encrypt_debug_mode) {
		sprintf(buf, "printsub: %c", direction);
		hexout(pointer, length, buf);
	}
}
#endif

void net_encrypt ()
{
}


/*
 * xprintf
 */
void xprintf (CDATA *cp, char *format, ...)
{
#pragma unused (cp, format)
#ifdef notdef
	char string[256];
	va_list args;

	va_start(args, format);
	vsprintf(string, format, args);
	sess_putln(dbgmsg, 0, cp);
	va_end(args);
#endif
}

/*
 * Junk so Emacs will set local variables to be compatible with Mac/MPW.
 * Should be at end of file.
 * 
 * Local Variables:
 * tab-width: 4
 * End:
 */

