/****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1986-1992,									*
*	Board of Trustees of the University of Illinois				*
****************************************************************/

#ifdef MPW
#pragma segment 4
#endif

#include <stdio.h>
#include <string.h>

#include "TelnetHeader.h"
#include "parse.h"				// For our #defines
#include "wind.h"				/* For WindRec structure */
#include "network.proto.h"				/* For net functions */
#include "debug.h"				/* For putln proto */
#include "vgtek.proto.h"
#include "vsdata.h"
#include "vsinterf.proto.h"
#include "maclook.proto.h"
#include "tekrgmac.proto.h"
#include "menuseg.proto.h"

#include "vr.h"
#include "vr.proto.h"
#include "encrypt.proto.h"
#include "kerberos.proto.h"
#include "wdefpatch.proto.h"

//#define	OPTS_DEBUG
#ifdef	OPTS_DEBUG
#define	opts_debug_print(x)	putln(x)	
#include "optsdebug.h"
#else
#define	opts_debug_print(x)
#endif

#define	SEND_NOSUPPORT_ON_SLC_INIT	1

extern short 	scrn;
extern WindRec	*screens;

#include "parse.proto.h"

/*********************************************************************/
/*  parse
*   look at the string which has just come in from outside and
*   check for special sequences that we are interested in.
*
*   Tries to pass through routine strings immediately, waiting for special
*   characters ESC and 255 to change modes.
*/

#ifdef notdef
void parse (struct WindRec *tw, unsigned char *st, short cnt)
{
	short	j, l;
	unsigned char *cp, ebuf[256];

	if (tw && tw->decrypt_input) {
		while (cnt > 0) {
			l = j = ((cnt > sizeof(ebuf))? sizeof(ebuf) : cnt);
			cp = ebuf;
			while (j-- > 0)
				*cp++ = (*tw->decrypt_input)(tw, (int)(*st++));
			parse2(tw, ebuf, l);		
			cnt -= l;
		}		
	} else {
		parse2(tw, st, cnt);
	}
}
#endif

void parse (struct WindRec *tw, unsigned char *st, short cnt)
{
	short i,temptw;
	unsigned char *mark,*orig;
#ifdef OPTS_DEBUG
	char munger[255]; 
#endif
	unsigned char *cp;
    char nullbuf[] = {AUTH_NULL, AUTH_CLIENT_TO_SERVER|AUTH_HOW_ONE_WAY};

	orig = st;				/* remember beginning point */
	mark = st + cnt;		/* set to end of input string */
/*
* raw mode for debugging, passes through escape sequences and other
* special characters as <27> symbols
*/
	if (tw->termstate == DUMBTYPE)
		{
		for (i=0; i < cnt; i++,st++)			/* put on screen raw */
			if (*st == 27 || *st > 126 && !(tw->ftpstate && gFTPServerPrefs->DoISOtranslation))
				{
				sprintf((char *)tw->parsedat,"<%d>",*st);
				VSwrite(tw->vs,(char *)tw->parsedat,strlen((char *)tw->parsedat));	/* send to virtual screen */
				}
			else
				VSwrite(tw->vs,(char *) st,1);		/* BYU LSC */
		}
	else
		{

/*
*  traverse string, looking for any special characters which indicate that
*  we need to change modes.
*/
	while (st < mark) 
		{
		switch (tw->telstate)
			{
			case GS:
			case ESCFOUND:
				if (tw->tektype >= 0) {	/* we'll allow the TEK */
					if ((*st == 12) || (tw->telstate == GS)) {	/* esc-FF */
						if ((*st == 12) && 
							((tw->termstate == VTEKTYPE) || (!tw->tekclear))) {
							if (tw->termstate == VTEKTYPE)
								putln("Entering Tek mode");
							else if (tw->curgraph > -1)
								detachGraphics(tw->curgraph);

							if (tw->curgraph <=  -1) {		// No current TEK window
								temptw = VGnewwin(1,tw->vs);

								if (temptw > -1) {
									Str255	scratchPstring;
									
									tw->curgraph = temptw;
	
									VGgiveinfo(temptw);
									GetWTitle(tw->wind, scratchPstring);
									PtoCstr(scratchPstring);
									RGattach(temptw,tw->vs,(char *)scratchPstring, tw->tektype);
								}
								else
									tw->telstate = STNORM;  // Can't create TEK window
							}

							if (tw->telstate != STNORM)
								tw->termstate = TEKTYPE;
						}

						if (tw->telstate == GS) {
							st++;
							VGwrite(tw->curgraph,"\035",1);
						}
						else if (tw->telstate != STNORM)
							VGwrite(tw->curgraph,"\037\033\014",3);

						tw->telstate = STNORM;
						break;
					} // FF or GS
				} // tw->tektype >= 0
	
				if (*st == '^')	{			/* ESC- ^ */
					tw->termstate = RASTYPE;
					tw->telstate = STNORM;
					VRwrite("\033^",2);		/* Put it through */
					orig = ++st;
					break;
				}
				
				if (tw->termstate == TEKTYPE)
					VGwrite(tw->curgraph,"\033",1);
				else if (tw->termstate  == RASTYPE)
					VRwrite("\033",1);
				else
					VSwrite(tw->vs,"\033",1);	/* send the missing ESC */
				tw->telstate = STNORM;
				break;


/*------------------------------------------------------------------------------*
 * 	IACFOUND:  This is the start of the Telnet option negotiation.  If Telnet	*
 *	gets an IAC character, then negotiation data follows, and is ready to be	*
 * 	parsed. 																	*
 *------------------------------------------------------------------------------*/
			case IACFOUND: 		/* telnet option negotiation- START */
				if (*st == 255) {		/* real data = 255 */
					orig = st;			// MP: translation mod (break will make it miss
										// this assignment later on in the case, Jim!)
					st++;				/* real 255 will get sent */
					tw->telstate = STNORM;
					break;
				}
				if ( 239 < *st ) {
					tw->telstate = *st;		/* by what the option is */
					st++;
					break;
				}
				tw->telstate = STNORM;
				orig=st;
				break;


			case TEL_EOF:		/* BYU LSC */
#ifdef OPTS_DEBUG
				sprintf(munger,"RECV: %s",telstates[tw->telstate - TEL_SE]);
				opts_debug_print(munger); 
#endif
				orig=st;
				tw->telstate=STNORM;
				break;

			case TEL_EL:				/* thanx Quincey!!! */
			case TEL_EC:
			case TEL_AYT:
			case TEL_AO:
			case TEL_IP:
			case TEL_BREAK:
			case TEL_DM:
			case TEL_NOP:
			case TEL_SE:
#ifdef OPTS_DEBUG
				sprintf(munger,"RECV: %s",telstates[tw->telstate - TEL_SE]);
				opts_debug_print(munger); 
#endif
				tw->telstate = STNORM;
				orig=st;


			case TEL_GA:
				orig=st;
				tw->telstate = STNORM;
				break;



/*------------------------------------------------------------------------------*
 * 	Negotiate DOTEL:  sent by the server whenever it wants the client to do 	*
 *	an option																	*
 *------------------------------------------------------------------------------*/
            case TEL_DOTEL:
#ifdef OPTS_DEBUG
				sprintf(munger,"RECV: %s %s",telstates[tw->telstate-TEL_SE],teloptions[*st]);
				opts_debug_print(munger); 
#endif
				switch( *st) 
					{
					case  N_SGA:		/* Sure we'll supress GA */
						if (!tw->Isga) {
							tw->Isga=1;
							sprintf((char *)tw->parsedat,"%c%c%c",255,TEL_WILLTEL,*st);
							netpush(tw->port);
							netwrite(tw->port,tw->parsedat,3);
							}
#ifdef OPTS_DEBUG
						sprintf(munger,"SENT: WILL %s",teloptions[*st]);
						opts_debug_print(munger); 
#endif
						tw->telstate = STNORM;
						orig = ++st;
						break;
					
					case N_TERMTYP:		/* And we'll even tell you about ourselves */
						if (!tw->Ittype) {
							tw->Ittype=1;
							sprintf((char *)tw->parsedat,"%c%c%c",255,TEL_WILLTEL,*st);
							netpush(tw->port);
							netwrite(tw->port,tw->parsedat,3);
							}
#ifdef OPTS_DEBUG
						sprintf(munger,"SENT: WILL %s",teloptions[*st]);
						opts_debug_print(munger); 
#endif
						tw->telstate = STNORM;
						orig = ++st;
						break;

					case  N_NAWS:			/* NCSA: sure, I like changing the window size! */
						tw->naws =1;		/* NCSA: this session is now NAWS */
						sprintf((char *)tw->parsedat,"%c%c%c",255,TEL_WILLTEL,*st);		/* NCSA: reply, we will to NAWS */
						netpush(tw->port);								/* NCSA */
						netwrite(tw->port,tw->parsedat,3);				/* NCSA: send it out */
#ifdef OPTS_DEBUG
						sprintf(munger,"SENT: WILL %s",teloptions[*st]);
						opts_debug_print(munger); 
#endif
						SendNAWSinfo(tw, tw->width, tw->rows);
//						sprintf((char *)tw->parsedat,"%c%c%c%c%c%c%c%c%c",IAC,TEL_SB,	/* BYU 2.4.16 */
//							N_NAWS,(char)0,(unsigned char)tw->width,		/* BYU 2.4.16 */
//							(char)0,(unsigned char)tw->rows,IAC,TEL_SE);	/* BYU 2.4.16 */
//						netpush(tw->port);								/* BYU 2.4.16 */
//						netwrite(tw->port,tw->parsedat,9);					/* BYU 2.4.16 */
#ifdef OPTS_DEBUG
						opts_debug_print("SENT: IAC TEL_SB N_NAWS <data> IAC TEL_SE"); 
#endif

						tw->telstate = STNORM;							/* NCSA */
						orig = ++st;									/* NCSA */
						break;											/* NCSA */

					case N_LINEMODE:  /* Sure I'll do line mode... */
						if (tw->lineAllow)  {
							sprintf((char *)tw->parsedat,"%c%c%c",IAC,TEL_WILLTEL,*st++);
							netwrite(tw->port,tw->parsedat,3);
#ifdef OPTS_DEBUG
							sprintf(munger,"SENT: WILL %s",teloptions[*st]);
							opts_debug_print(munger); 
#endif
							sprintf((char *)tw->parsedat,"%c%c%c%c",IAC,TEL_SB,N_LINEMODE,L_SLC);
							netwrite(tw->port,tw->parsedat,4);
#ifdef OPTS_DEBUG
							opts_debug_print("SENT: IAC SB LINEMODE SLC");
#endif	
							for (i=1; i<= SLC_MAX; i++){
								if (tw->slc[i]==-1) {
#ifdef	SEND_NOSUPPORT_ON_SLC_INIT
									sprintf((char *)tw->parsedat,"%c%c%c",i,SLC_NOSUPPORT,0);
#ifdef OPTS_DEBUG
									sprintf(munger,"     %s NO_SUPPORT 0",LMoptions[i]);
#endif
#endif
									}
								else
									{
									sprintf((char *)tw->parsedat,"%c%c%c",i,SLC_CANTCHANGE,(char)tw->slc[i]);

#ifdef OPTS_DEBUG
									sprintf(munger,"     %s CANTCHANGE %i",LMoptions[i],(int)tw->slc[i]);
#endif
									}
								opts_debug_print(munger);
								netwrite(tw->port,tw->parsedat,3);
								}
	
							opts_debug_print("SENT: IAC SE");
							sprintf((char *)tw->parsedat,"%c%c",IAC,TEL_SE);
							netwrite(tw->port,tw->parsedat,2);
							}
						else {
							sprintf((char *)tw->parsedat,"%c%c%c",IAC,TEL_WONTTEL,*st++);
							opts_debug_print("SENT: WONT Linemode");
							netwrite(tw->port,tw->parsedat,3);
							}
						break;

					case N_AUTHENTICATION:		/* do auth */
						if (!tw->myopts[OPT_AUTHENTICATION-MHOPTS_BASE]) {
							if (tw->authenticate) {
								(tw->myopts)[OPT_AUTHENTICATION-MHOPTS_BASE] = 1;
								sprintf((char *)tw->parsedat,"%c%c%c",IAC,TEL_WILLTEL,*st);
								netwrite(tw->port,tw->parsedat,3);
#ifdef OPTS_DEBUG
								sprintf(munger,"SENT: WILL %s",teloptions[*st]);
								opts_debug_print(munger); 
#endif
							} else {
								sprintf((char *)tw->parsedat,"%c%c%c",IAC,TEL_WONTTEL,*st);
								netwrite(tw->port,tw->parsedat,3);
#ifdef OPTS_DEBUG
								sprintf(munger,"SENT: WONT %s",teloptions[*st]);
								opts_debug_print(munger); 
#endif
							}
						}
						tw->telstate = STNORM;
						orig = ++st;
						break;

					case N_ENCRYPT: 			/* do encrypt */
						if (!tw->myopts[OPT_ENCRYPT-MHOPTS_BASE]) {
							if (tw->encrypt) {
								(tw->myopts)[OPT_ENCRYPT-MHOPTS_BASE] = 1;
								sprintf((char *)tw->parsedat,"%c%c%c",IAC,TEL_WILLTEL,*st);
								netwrite(tw->port,tw->parsedat,3);
#ifdef OPTS_DEBUG
								sprintf(munger,"SENT: WILL %s",teloptions[*st]);
								opts_debug_print(munger); 
#endif
							} else {
								sprintf((char *)tw->parsedat,"%c%c%c",IAC,TEL_WONTTEL,*st);
								netwrite(tw->port,tw->parsedat,3);
#ifdef OPTS_DEBUG
								sprintf(munger,"SENT: WONT %s",teloptions[*st]);
								opts_debug_print(munger); 
#endif
							}
						}
						tw->telstate = STNORM;
						orig = ++st;
						break;

					default:				/* But, we won't do .... */
                		sprintf((char *)tw->parsedat,"%c%c%c",255,TEL_WONTTEL,*st++);
                		netwrite(tw->port,tw->parsedat,3);  
#ifdef OPTS_DEBUG
						sprintf(munger,"SENT: WONT %s",teloptions[*(st-1)]);
						opts_debug_print(munger); 
#endif
						break;
					}
                tw->telstate = STNORM;
                orig = st;
                break;


/*------------------------------------------------------------------------------*
 * 	Negotiate DONTTEL:  sent by the server whenever it wants the client to  	*
 *	definitely NOT do an option.  Sent to prevent client from negotiating an	*
 *	unsupported option															*
 *------------------------------------------------------------------------------*/
            case TEL_DONTTEL:
#ifdef OPTS_DEBUG
				sprintf(munger,"RECV: %s %s",telstates[tw->telstate-TEL_SE],teloptions[*st]);
				opts_debug_print(munger); 
#endif
				tw->telstate = STNORM;
                switch (*st++) {
					case N_ENCRYPT:					/* dont encrypt */
					case N_AUTHENTICATION:			/* dont authenticate */
						tw->myopts[*(st-1)-MHOPTS_BASE] = 0;
                        sprintf((char *)tw->parsedat,"%c%c%c", IAC, TEL_WONTTEL, *(st-1));
						netpush(tw->port);
                        netwrite(tw->port,tw->parsedat,3);  /* refuse it */
#ifdef OPTS_DEBUG
						sprintf(munger,"SENT: WONT %s", teloptions[*(st-1)]);
						opts_debug_print(munger); 
#endif	
                        break;
				}
                orig = st;
            	break;


/*------------------------------------------------------------------------------*
 * 	Negotiate WILLTEL:  sent by the client to try and get the server to use an	*
 *	option.  This tells the server that the client is requesting that an option *
 * 	be used																		*
 *------------------------------------------------------------------------------*/
            case TEL_WILLTEL:
#ifdef OPTS_DEBUG
				sprintf(munger,"RECV: %s %s",telstates[tw->telstate-TEL_SE],teloptions[*st]);
				opts_debug_print(munger); 
#endif
                tw->telstate = STNORM;
                switch(*st++) 
                	{
                    case N_ECHO:             /* Echo on the other end*/
#ifdef	OPTS_DEBUG
						if (!tw->echo)
							opts_debug_print("tw->echo is False.");
#endif
						if (!tw->echo)
							break;
						tw->echo = 0;	/* Ok, in that case they can echo... */
						changeport(scrn,scrn);
                        sprintf((char *)tw->parsedat,"%c%c%c",IAC,TEL_DOTEL,1);
						netpush(tw->port);
                        netwrite(tw->port,tw->parsedat,3);  /* refuse it */
#ifdef OPTS_DEBUG
						sprintf(munger,"SENT: DO %s",teloptions[*(st-1)]);
						opts_debug_print(munger); 
#endif
						break;
					
					case N_SGA:				/* Supress GA */
#ifdef	OPTS_DEBUG
						if (tw->Usga)
							opts_debug_print("tw->Usga is True.");
#endif
						if (tw->Usga)
							break;
						tw->Usga = 1;	/* Go Ahead and supress GA */
                        sprintf((char *)tw->parsedat,"%c%c%c",IAC,TEL_DOTEL,3);
						netpush(tw->port);
                        netwrite(tw->port,tw->parsedat,3);  /* refuse it */
#ifdef OPTS_DEBUG
						sprintf(munger,"SENT: DO %s",teloptions[*(st-1)]);
						opts_debug_print(munger); 
#endif
						break;
					
					case N_TIMING:				/* Timing Mark */
						tw->timing = 0;
						break;
                    
					case N_AUTHENTICATION:		/* will auth */
						if (!tw->hisopts[OPT_AUTHENTICATION-MHOPTS_BASE]) {
							if (tw->authenticate) {
								(tw->hisopts)[OPT_AUTHENTICATION-MHOPTS_BASE] = 1;
								sprintf((char *)tw->parsedat,"%c%c%c",IAC,TEL_DOTEL,OPT_AUTHENTICATION);
								netwrite(tw->port,tw->parsedat,3);
#ifdef OPTS_DEBUG
								sprintf(munger,"SENT: WILL %s",teloptions[OPT_AUTHENTICATION]);
								opts_debug_print(munger); 
#endif
							} else {
								sprintf((char *)tw->parsedat,"%c%c%c",IAC,TEL_DONTTEL,OPT_AUTHENTICATION);
								netwrite(tw->port,tw->parsedat,3);
#ifdef OPTS_DEBUG
								sprintf(munger,"SENT: WONT %s",teloptions[OPT_AUTHENTICATION]);
								opts_debug_print(munger); 
#endif
							}
						}
						break;

					case N_ENCRYPT: 			/* will encrypt */
						if (!tw->hisopts[OPT_ENCRYPT-MHOPTS_BASE]) {
							if (tw->encrypt) {
								(tw->hisopts)[OPT_ENCRYPT-MHOPTS_BASE] = 1;
								sprintf((char *)tw->parsedat,"%c%c%c",IAC,TEL_DOTEL,OPT_ENCRYPT);
								netwrite(tw->port,tw->parsedat,3);
#ifdef OPTS_DEBUG
								sprintf(munger,"SENT: DO %s",teloptions[OPT_ENCRYPT]);
								opts_debug_print(munger); 
#endif
							} else {
								sprintf((char *)tw->parsedat,"%c%c%c",IAC,TEL_DONTTEL,OPT_ENCRYPT);
								netwrite(tw->port,tw->parsedat,3);
#ifdef OPTS_DEBUG
								sprintf(munger,"SENT: DONT %s",teloptions[OPT_ENCRYPT]);
								opts_debug_print(munger); 
#endif
							}
						}

						if (tw->authenticate && tw->encrypt)	/* do this here (and tn3270?) ddd */
							encrypt_send_support(tw->edata);
						break;

                    default:
                        sprintf((char *)tw->parsedat,"%c%c%c",IAC,TEL_DONTTEL,*(st-1));
                        netwrite(tw->port,tw->parsedat,3);  /* refuse it */
#ifdef OPTS_DEBUG
						sprintf(munger,"SENT: DONT %s",teloptions[*(st-1)]);
						opts_debug_print(munger); 
#endif
                	}
                orig = st;
                break;



/*------------------------------------------------------------------------------*
 * 	Negotiate WONTTEL:  The client refuses a Telnet option, and wants the	 	*
 *	to forget about trying to negotiate it.										*
 *------------------------------------------------------------------------------*/
            case TEL_WONTTEL:
#ifdef OPTS_DEBUG
				sprintf(munger,"?SENT: (WONTTEL)%s %s",telstates[tw->telstate-TEL_SE],teloptions[*st]);
				opts_debug_print(munger); 
#endif
                tw->telstate = STNORM;
                switch(*st++) 
                	{         /* which option? */
                    case N_ECHO:             /* echo */
						if (tw->echo)
							break;
                        tw->echo = 1;	/* Ok, I will echo if I have to... */
						changeport(scrn,scrn);
                        sprintf((char *)tw->parsedat,"%c%c%c",IAC,TEL_DONTTEL,N_ECHO);
						netpush(tw->port);
                        netwrite(tw->port,tw->parsedat,3);  /* refuse it */
#ifdef OPTS_DEBUG
						sprintf(munger,"SENT: %s %s",telstates[TEL_WONTTEL-TEL_SE],teloptions[*(st-1)]);
						opts_debug_print(munger); 
#endif	
                        break;
					
					case N_SGA:
						if (!tw->Usga)
							break;
						tw->Usga = 0;
                        sprintf((char *)tw->parsedat,"%c%c%c",IAC,TEL_DONTTEL,N_SGA);
						netpush(tw->port);
                        netwrite(tw->port,tw->parsedat,3);  /* refuse it */
#ifdef OPTS_DEBUG
						sprintf(munger,"SENT: %s %s",telstates[TEL_WONTTEL-TEL_SE],teloptions[*(st-1)]);
						opts_debug_print(munger); 
#endif	
						break;
					
					case N_TIMING:				/* Timing Mark */
						tw->timing = 0;
						break;
                    
					case N_ENCRYPT:					/* wont encrypt */
					case N_AUTHENTICATION:			/* wont authenticate */
						tw->hisopts[*(st-1)-MHOPTS_BASE] = 0;
                        sprintf((char *)tw->parsedat,"%c%c%c",IAC,TEL_DONTTEL,*(st-1));
						netpush(tw->port);
                        netwrite(tw->port,tw->parsedat,3);  /* refuse it */
#ifdef OPTS_DEBUG
						sprintf(munger,"SENT: WONT %s", teloptions[*(st-1)]);
						opts_debug_print(munger); 
#endif	
                        break;

                    default:
                        break;
                	}
                orig = st;
                break;



/*------------------------------------------------------------------------------*
 * 	SUBNEGOTIATION -- If there is an SB token found, then Telnet needs to do 	*
 *	some subnegotiating.  The subnegotiation data follows, and needs to be put	*
 *	somewhere safe.  Make sure and update the state, so that we know that		*
 *	Telnet is doing some subnegotiations and not just horsing around			*
 *------------------------------------------------------------------------------*/
			case TEL_SB:
				tw->telstate = NEGOTIATE;		/* Guess what I'm doing... */
				orig=st;
				substate=0;				/* Defined for each */
				tw->parseIndex =0;		/* No data yet! Clear any possible garbage */
				break;

			case NEGOTIATE:
				if (substate <200) {
					switch( *st) {
						case IAC:				/* is it the famous DOUBLE-IAC??? */
							if (*(st+1) ==IAC) 	/* Yes, treat as data value 255 */
								{
								tw->parsedat[tw->parseIndex++]=*st++;
								/*tw->parsedat[tw->parseIndex++]=* */ st++;
								}
							else 				/* otherwise just save it */
								{
								tw->parsedat[tw->parseIndex]='\0';
								substate=*st++;
								}							
							break;
						default:
							tw->parsedat[tw->parseIndex++]=*st++;
							break;
						}
					} else 
					
					{
					switch( substate) 	/* subnegotiate */
						{
						case IAC:
							substate=*st++;
							if (substate==TEL_SE) {
								switch( tw->parsedat[0]) 
									{
/*------------------------------------------------------------------------------*
 * 	SUBNegotiate Termtype:  send the term type data now							*
 *------------------------------------------------------------------------------*/
									case N_TERMTYP:			
										if (tw->parsedat[1]==1) 
											{
											char s[255], termtmp[40];
											BlockMove(tw->answerback, termtmp, 32);
											PtoCstr((StringPtr)termtmp);
	#ifdef OPTS_DEBUG
											sprintf(munger,"RECV: SB TERMINAL-TYPE SEND\r\nSEND: SB TERMINAL-TYPE IS %s\r\n",termtmp);
											opts_debug_print(munger); 
	#endif
											netpush(tw->port);
											netwrite(tw->port,"\377\372\030\000",4);
											netpush(tw->port);
											sprintf(s,"%s\377\360",termtmp);
	                        				netwrite(tw->port, s, strlen(s));
											}
										break;
										
/*------------------------------------------------------------------------------*
 * 	SUBNegotiate ENCRYPTION:                                     		        *
 *------------------------------------------------------------------------------*/
									case N_ENCRYPT:	{
										unsigned char *subpointer = &tw->parsedat[SB_TYPE];
										long	sublen = &tw->parsedat[tw->parseIndex] - 
											subpointer;
#ifdef OPTS_DEBUG
										{
											short	j;
											char *cp = munger;

											sprintf(cp,"RECV: SB ENCRYPT %02X ",
													tw->parsedat[SB_SUBOPTION]);
											cp += strlen(cp);
											for (j = 0; j < sublen; j++) {
												sprintf(cp, "%02X ", subpointer[j] & 0xff);
												cp += strlen(cp);
											}
											sprintf(cp, "\r\n");
											opts_debug_print(munger); 
										}
#endif
 										if (tw->edata != NULL)
											netportencryptstate(tw->port, (Ptr)tw->edata);
										switch (tw->parsedat[SB_SUBOPTION]) {
										case ENCRYPT_START:
											if (!tw->hisopts[OPT_ENCRYPT-MHOPTS_BASE]) 	/* my_want_state_is_dont(tw->o_encrypt) */
												break;
											encrypt_start(tw->edata, subpointer, sublen);
											/*
											 * If we turned on decryption we must
											 * decrypt the remainder of the buffer
											 */
											if (tw->edata->decrypt_input) {
												unsigned char *cp = st;
												while (cp < mark) {
													*cp = (*tw->edata->decrypt_input)(tw->edata, (int)(*cp));
													cp++;
												}
 												netportencryptstate(tw->port, (Ptr)tw->edata);
											}
											break;

										case ENCRYPT_END:
											if (!tw->hisopts[OPT_ENCRYPT-MHOPTS_BASE]) /* if my_want_state_is_dont(tw->o_encrypt) */
											break;
											encrypt_end(tw->edata);
											break;

										case ENCRYPT_SUPPORT:
											if (!tw->myopts[OPT_ENCRYPT-MHOPTS_BASE]) /* if my_want_state_is_wont(tw->o_encrypt) */
												break;
											encrypt_support(tw->edata, subpointer, sublen);
											break;

										case ENCRYPT_REQSTART:
										if (!tw->myopts[OPT_ENCRYPT-MHOPTS_BASE]) /* if my_want_state_is_wont(tw->o_encrypt) */
											break;
											encrypt_request_start(tw->edata, subpointer, sublen);
											break;

										case ENCRYPT_REQEND:
											if (!tw->myopts[OPT_ENCRYPT-MHOPTS_BASE]) /* if my_want_state_is_wont(tw->o_encrypt) */
												break;
											/*
											 * We can always send an REQEND so that 
											 * we cannot get stuck encrypting.  We 
											 * should only get this if we have been 
											 * able to get in the correct mode
											 * anyhow.
											 */
											encrypt_request_end(tw->edata);
											break;
										case ENCRYPT_IS:
											if (!tw->hisopts[OPT_ENCRYPT-MHOPTS_BASE]) /* if my_want_state_is_dont(tw->o_encrypt) */
												break;
											encrypt_is(tw->edata, subpointer, sublen);
											break;
										case ENCRYPT_REPLY:
											if (!tw->myopts[OPT_ENCRYPT-MHOPTS_BASE]) /* if my_want_state_is_wont(tw->o_encrypt) */
												break;
											encrypt_reply(tw->edata, subpointer, sublen);
											break;
										case ENCRYPT_ENC_KEYID:
											if (!tw->hisopts[OPT_ENCRYPT-MHOPTS_BASE]) /* if my_want_state_is_dont(tw->o_encrypt) */
												break;
											encrypt_enc_keyid(tw->edata, subpointer, sublen);
											break;
										case ENCRYPT_DEC_KEYID:
											if (!tw->myopts[OPT_ENCRYPT-MHOPTS_BASE]) /* if my_want_state_is_wont(tw->o_encrypt) */
												break;
											encrypt_dec_keyid(tw->edata, subpointer, sublen);
											break;
										default:
											break;
										}
									}
									break;

/*------------------------------------------------------------------------------*
 * 	SUBNegotiate Authentication:  send the authentication data now     		    *
 *------------------------------------------------------------------------------*/
									case N_AUTHENTICATION:			
#ifdef OPTS_DEBUG2
										{
											short	j;
											char	*cp = munger;
											char	*subpointer = &tw->parsedat[SB_TYPE];
											long	sublen = &tw->parsedat[tw->parseIndex] - 
												subpointer;

											sprintf(cp,"RECV: SB AUTH %02X ",
													tw->parsedat[SB_SUBOPTION]);
											cp += strlen(cp);
											for (j = 0; j < sublen; j++) {
												sprintf(cp, "%02X ", subpointer[j] & 0xff);
												cp += strlen(cp);
											}
											sprintf(cp, "\r\n");
											opts_debug_print(munger); 
										}
#endif
										switch (tw->parsedat[SB_SUBOPTION]) {
										case TNQ_IS:
											break;

										case TNQ_SEND:
											/*
											 * For telnet clients.
											 * The buffer contains:
											 * AUTHENTICATION SEND type modifier 
											 *        [type modifier] [...]
											 * Scan the type/modifier pairs until we 
											 * find one we can do. Since they are are 
											 * in priority order, the  first one we
											 * find that we can do wins.
											 */
											cp = &tw->parsedat[SB_TYPE];
											for (; cp < &tw->parsedat[tw->parseIndex]; 
												                    cp += 2) {
												switch (*cp) {
												case AUTH_KERBEROS_V4:
													if (krb_telnet(tw, (char *)cp, true)) {
														/*
														 * if krb_telnet fails, send null authentication
														 * to avoid hanging.
														 */
														tn_sendsub(tw, N_AUTHENTICATION, TNQ_IS, 
													   			    nullbuf, sizeof(nullbuf));
													}																		
													goto xx1;
												}
											}
											/*
											 * If we can't do any of the requested 
											 * authorizations, send AUTH_NULL.
											 */
											tn_sendsub(tw, N_AUTHENTICATION, TNQ_IS, 
													   nullbuf, sizeof(nullbuf));
											xx1:
											break;

										case TNQ_REPLY:
											/*
											 * This is a reply to information we sent 
											 * in a previous IS AUTH or CHALLENGE 
											 * command. The buffer contains:
											 * AUTHENTICATION REPLY type modifier 
											 *       auth-data-type [auth data bytes]
											 */
											switch (tw->parsedat[SB_TYPE] & 0xFF) {
											case AUTH_KERBEROS_V4:
												tn_kerberos_reply(tw, (unsigned char *)tw->parsedat);
												break;
											default:
												;
												/* should we do somthing with an 
												   unknown reply type ddd */
											}
											break;

										case TNQ_NAME:
											/*
											 * For the telnet server. This is the 
											 * name that the telnet client
											 * wants us to use.
											 */
											break;
										} /* switch tw->parsedat[SB_SUBOPTION] */
										break;
										
/*------------------------------------------------------------------------------*
 * 	SUBNegotiate Linemode:  set up local characters, modes, and such			*
 *------------------------------------------------------------------------------*/
									case N_LINEMODE:
										switch(tw->parsedat[1])
											{
											char s[80];

											case L_MODE:	/* change mode */
#ifdef OPTS_DEBUG
												strcpy(s, "RECV: SB LINEMODE MODE => ");
												DemangleLineModeShort(s, tw->parsedat[2]);
												opts_debug_print(s);
#endif
												if (tw->lineAllow) {			// First make sure we allowed linemode in the first place.
													// RFC 1184 says client should ignore MODE negotiations with the MODE_ACK bit set, and should not
													//	generate a response if the negotiated MODE matches the current MODE
													if (!((tw->parsedat[2] & L_MODE_ACK) || ((tw->parsedat[2] & L_MODE_MASK) == tw->lmode))) {
													
														// This is a hack.  Accept the state of L_EDIT and L_TRAPSIG that the server wants.  If
														// the server only wants zero, one, two or both of those bits, generate an ack.  Otherwise
														// send the server a mode that masks out all requested bits except EDIT and TRAPSIG.
														
														// (Now, since we only support L_EDIT and/or L_TRAPSIG, if those are the only modes requested,)
														//	(send an ACK of that MODE to the server.)
														
														tw->lmode = (tw->parsedat[2] & (L_EDIT | L_TRAPSIG));	// Accept the mode.
														
														if ((tw->parsedat[2] & (L_EDIT | L_TRAPSIG)) == tw->parsedat[2]) {
															tw->parsedat[2] |= L_MODE_ACK;	// Set the MODE_ACK bit
															}
														// We got a MODE bit we dont support, mask out just what we support and send it back, UNacked.
														else {
															tw->parsedat[2] = tw->parsedat[2] & (L_EDIT | L_TRAPSIG);
															}

														sprintf(s,"%c%c%c%c",IAC,TEL_SB,N_LINEMODE,L_MODE);
														netwrite(tw->port,s,4);
														sprintf(s,"%c%c%c",tw->parsedat[2],IAC,TEL_SE);
														netwrite(tw->port,s,3); 
#ifdef OPTS_DEBUG
														opts_debug_print("SENT: IAC SB");
														strcpy(s, "SENT: LM MODE = ");
														DemangleLineModeShort(s, tw->parsedat[2]);
														opts_debug_print(s);
														opts_debug_print("SENT: IAC SE");
#endif
													}
#ifdef OPTS_DEBUG
													else {
														strcpy(s, "LINEMODE MODE = ");
														DemangleLineModeShort(s, tw->parsedat[2]);
														opts_debug_print(s);
														if (tw->parsedat[2] & L_MODE_ACK) 
															opts_debug_print("\tignored 'cause MODE_ACK was set.");
														else
															opts_debug_print("\tIMPLICITLY ACCEPTED.");
														strcpy(s, "Curr Linemode = ");
														DemangleLineModeShort(s, tw->lmode);
														opts_debug_print(s);
													}
#endif
													
												}
												break;
											
											case TEL_DOTEL:	
#ifdef OPTS_DEBUG
												sprintf(munger,"RECV: SB LINEMODE DO %c", tw->parsedat[2]);
												opts_debug_print(munger);
#endif
												netwrite(tw->port,"\377\372\042",3);
												sprintf(s,"%c\002\377\360",TEL_WONTTEL);
												opts_debug_print("SENT: IAC SB LINEMODE WONT FORWARDMASK IAC SE");
												netwrite(tw->port,s,strlen(s));
												break;

											case TEL_WILLTEL:
#ifdef OPTS_DEBUG
												sprintf(munger,"RECV: SB LINEMODE WILL %c", tw->parsedat[2]);
												opts_debug_print(munger);
#endif
												netwrite(tw->port,"\377\372\042",3);
												sprintf(s,"%c\002\377\360",TEL_DONTTEL);
												opts_debug_print("SENT: IAC SB LINEMODE DONT FORWARDMASK IAC SE");
												netwrite(tw->port,s,strlen(s));
												break;


											case L_SLC:		/* set local chars */
												{
												short	lmslcflag = 0;
#ifdef OPTS_DEBUG
												sprintf(munger,"RECV: SB LINEMODE SLC");
												opts_debug_print(munger);
												for(i=2;(tw->parsedat[i]!='\0') && (tw->parsedat[i]!=IAC); i+=3) {
													if(tw->parsedat[i+1] & SLC_AWK)
														sprintf(munger,"     %s %s|AWK %d",LMoptions[tw->parsedat[i]],LMflags[tw->parsedat[i+1] & SLC_LEVELBITS],tw->parsedat[i+2]);
													else
														sprintf(munger,"     %s %s %d",LMoptions[tw->parsedat[i]],LMflags[tw->parsedat[i+1] & SLC_LEVELBITS],tw->parsedat[i+2]);
													opts_debug_print(munger);
										// 2.6b16.1			if ((unsigned char)(tw->parsedat[i+2])==IAC) i++;
													}	/* end for */
#endif

												for (i=2, lmslcflag=0; (tw->parsedat[i]!='\0') && (tw->parsedat[i]!=IAC); i+=3) {

													// If func = 0, skip it 'cause we are the client.
//													if (tw->parsedat[i] == 0)
//														continue;
													
													// If it's a function we don't know about, say we don't support it.  If the server is telling
													// us he doesn't support it, just ignore him.
													if (tw->parsedat[i] > SLC_MAX) {
														if ((tw->parsedat[i+1] & SLC_LEVELBITS) != SLC_NOSUPPORT) {
															if (!lmslcflag) {
																lmslcflag = 1;		// Do this only once
																sprintf(s,"%c%c%c%c",IAC,TEL_SB,N_LINEMODE,L_SLC);
																netwrite(tw->port,s,4);
																opts_debug_print("SENT: IAC SB LINEMODE SLC");
																}
															sprintf(s,"%c%c%c",tw->parsedat[i],SLC_NOSUPPORT,0);
#ifdef OPTS_DEBUG
															sprintf(munger,"     %d SLC_NOSUPPORT 0",tw->parsedat[i]);
															opts_debug_print(munger);
#endif
															netwrite(tw->port,s,3);
														}
														else
															continue;
													}
													
													// RFC 1184: If we get a response that is the same as the current setting, ignore it.  Technically, we
													//	should check the value for a CANTCHANGE response to satisfy part 2 of Sec 5.5.  However, since we
													//	advertised that key as CANTCHANGE in the first place, the server should not be changing it.  This
													//	should be fixed later, however I don't see it causing big problems right now. - JMB 12/93
													
													if (! ((((tw->parsedat[i+1] & SLC_LEVELBITS) == SLC_NOSUPPORT) && (tw->slc[tw->parsedat[i]] == -1))
														|| (((tw->parsedat[i+1] & SLC_LEVELBITS) == SLC_CANTCHANGE) && (tw->slc[tw->parsedat[i]] != -1)))) {
														// If we get to this point, the value sent to us doesn't agree with what we want.
														
															
														if (!(tw->parsedat[i+1] & SLC_AWK)) {
															if (!lmslcflag) {
																lmslcflag = 1;		// Do this only once
																sprintf(s,"%c%c%c%c",IAC,TEL_SB,N_LINEMODE,L_SLC);
																netwrite(tw->port,s,4);
																opts_debug_print("SENT: IAC SB LINEMODE SLC");
																}
														
															if (tw->slc[tw->parsedat[i]]==-1)
																{
																sprintf(s,"%c%c%c",tw->parsedat[i],SLC_NOSUPPORT,0);
#ifdef OPTS_DEBUG
																sprintf(munger,"     %s SLC_NOSUPPORT 0",LMoptions[tw->parsedat[i]]);
#endif
																}
															else
																{
																sprintf(s,"%c%c%c",tw->parsedat[i],SLC_CANTCHANGE,(char)tw->slc[tw->parsedat[i]]);
#ifdef OPTS_DEBUG
																sprintf(munger,"     %s SLC_CANTCHANGE %d",LMoptions[tw->parsedat[i]],
																							(char)tw->slc[tw->parsedat[i]]);
#endif
																}																
#ifdef OPTS_DEBUG
															opts_debug_print(munger);
#endif
															netwrite(tw->port,s,3);
										// 2.6b16.1					if (tw->parsedat[i+2]==IAC) i++;
															}
														}
													}
													
												if (lmslcflag) {
													sprintf(s,"%c%c",IAC,TEL_SE);
													netwrite(tw->port,s,2);
													}
													

													/* otherwise just exit */
													}
													break;
												
												default:
#ifdef OPTS_DEBUG
													sprintf(munger, "RECV: SB LINEMODE ?? (?? = %c)", tw->parsedat[1]);
													opts_debug_print(munger);
#endif
													break;

												}
										break;

/* end of all the new line-mode stuff */								
								
								
								default:
									break;
								}
							} // substate == TEL_SE
							orig=st;
							tw->telstate=STNORM;
							break;
						default:
							orig=st;
							tw->telstate=STNORM;
							break;
						} // switch(substate)
					}

				break;
            default:
                tw->telstate = STNORM;
                break;
		} // switch(tw->telstate)

/*
* quick scan of the remaining string, skip chars while they are
* uninteresting
*/
		if (tw->telstate == STNORM) {
/*
*  skip along as fast as possible until an interesting character is found
*/

			if (!tw->eightbit) {											/* BYU 2.4.10 */
				while (st < mark) {											/* BYU 2.4.10 */
					if (*st == IAC) 										/* BYU 2.4.10 */
						break;												/* BYU 2.4.10 */
					else {													/* BYU 2.4.10 */
						*st &= 0x7f; 										/* BYU 2.4.10 */
						if (*st == ESC  || *st == GS) 						/* BYU 2.4.10 */
							break;											/* BYU 2.4.10 */
						st++;												/* BYU 2.4.10 */
				}	}														/* BYU 2.4.10 */
			} else 															/* BYU 2.4.10 */
				while (st < mark && *st != ESC  && *st < 255 && *st != GS) 	// MP: translation mod
					st++; 													/* BYU 2.4.10 */
/*
*  send the string where it belongs
*/
			if (!tw->timing) {
				if (tw->termstate == TEKTYPE) {
					short i;
					
					i = VGwrite( tw->curgraph,(char *) orig,  st-orig);	/* BYU LSC */
					if (i < (st - orig)) {
						detachGraphics(tw->curgraph);
						st = orig + i;
						}
					}
				else if (tw->termstate == RASTYPE) {
					short i;
					
					i= VRwrite((char *) orig, st-orig);		/* BYU LSC */
					if (i <(st-orig)) {
						tw->termstate = VTEKTYPE;
						st = orig +i;				/* Next char to parse */
						}
					}
				else 
					VSwrite( tw->vs,(char *) orig,st-orig);	/* BYU LSC - send to virtual VT102 */
				}

			orig = st;				/* forget what we have sent already */

			if (st < mark)
				switch (*st) {
					case 255:			/* telnet IAC */
						tw->telstate = IACFOUND;
						st++;
						break;
					case GS:
						if (tw->telstate != GS) {
							tw->telstate = GS;
							}
						else
							tw->telstate = STNORM;
						st++;
						break;
						
					case ESC:			/* ESCape code */
						if (st == mark-1 || *(st+1) == 12 || *(st+1) == '^' ) {
							tw->telstate = ESCFOUND;
						}
						st++;			/* strip or accept ESC char */
						break;
	
					default:
						if (*st++ > 127) {
							if (st==mark)							/*new addition */
								VSwrite(tw->vs,(char *) orig,1);	/* BYU LSC */
	         				}
						break;
				}	// switch(*st)
			} // tw->telstate == STNORM
	} // while (st < mark)
	} // tw->termstate != DUMBTYPE
} /* parse */

void	SendNAWSinfo(WindRec *s, short horiz, short vert)
{
	char			blah[20];
	unsigned char	height, width;

	height = vert & 0xff;
	width = horiz & 0xff;
	
    /* 931112, ragge, NADA, KTH, ugly patch to not send IAC as window size  */
	if(height == 0xFF) height = 0xFE;
	if(width == 0xFF) width = 0xFE;

	netpush (s->port);

/* NCSA: syntax for command is:  IAC SB NAWS widthHI widthLO heightHI heightLO IAC SE */

	netwrite(s->port,"\377\372\037\000",4);
	sprintf(blah,"%c\000", width);
	netwrite(s->port,blah,2);
	sprintf(blah,"%c\377\360", height);
	netwrite(s->port,blah,3);
	opts_debug_print("SENT: IAC SB NAWS <data> IAC SE");		
}

/*
 * Implementation specific Kerberos routines
 */


/*
 * net_write
 */
void net_write (struct WindRec *tw, char *buf, short length)
{
	netpush(tw->port);
	netwrite(tw->port, buf, length);
}


/*
 * send_auth_opt
 */
void send_auth_opt (struct WindRec *tw)
{
	unsigned char *cp, buf[30];
	
	cp = buf;
	if (tw->authenticate) {
		sprintf((char *)cp, "%c%c%c", IAC, TEL_WILLTEL, OPT_AUTHENTICATION);	/* will authenticate */
		cp += 3;
		(tw->myopts)[OPT_AUTHENTICATION-MHOPTS_BASE] = 1;

		if (tw->encrypt) {
			sprintf((char *)cp, "%c%c%c", IAC, TEL_WILLTEL, OPT_ENCRYPT);		/* will encrypt */
			cp += 3;
			(tw->myopts)[OPT_ENCRYPT-MHOPTS_BASE] = 1;
		}
		netpush(tw->port);
		netwrite(tw->port, buf, cp - buf);
	}
}


void tn_sendsub (struct WindRec *tw, short code, short request, char *cp, short length)
{
	long len;
	unsigned char *src, *lp, *limit;
	static char start[] = {IAC, TEL_SB, 0, 0};
	static char end[] = {IAC, TEL_SE};

	src = (unsigned char *)cp;
	limit = src + length;
	start[2] = code;
	start[3] = request;
	netpush(tw->port);
	netwrite(tw->port, start, 4);

	/*
	 * Transmit the buffer. IACs must be doubled
	 */
	if (*src == IAC) {						/* check initial iac in buffer */
		netpush(tw->port);
		netwrite(tw->port, start, 1);		/* IAC */
	}
	while (src < limit) {
		lp = src+1;							/* dont check first char */
		while (lp < limit) {				/* scan for IAC */
			if (*lp == IAC)
				break;
			lp++;		
		}
		len = lp - src;
		if (lp < limit)						/* if stopped on IAC */
			len++;							/* include IAC in xmit */

		netpush(tw->port);
		netwrite(tw->port, src, len);

		src = lp;							/* resume scanning */
    }

	netpush(tw->port);
	netwrite(tw->port, end, 2);
}


/*
 * getcname
 * Return a pointer to the cannonical host name
 */
char *getcname (WindRec *tw)
{
	char *cp;
	static char *b, buf[100];

	cp = 0;
	if (tw->cannon[0])
		cp = tw->cannon;

//	Doing the following is bad because we disposed of our init params!
//	else if ((*(*(ConnInitParams **)(tw->myInitParams))->session)->hostname)
//		cp = (char *)(*(*(ConnInitParams **)(tw->myInitParams))->session)->hostname;

	/* make a local copy to avoid locking handles */
	if (cp) {
		b = buf;		
		while (*cp)
			*b++ = *cp++;			
		*b++ = '\0';
		return buf;
	}

	return cp;
}


/*
 * encryptStatechange
 * Called by encrypt.c when the encryption state changes
 */
#define kOurHit	32
void encryptStatechange (struct WindRec *tw)
{
	MyWDEFPatch(zoomDocProc , tw->wind, wDraw, kOurHit);
}

void DemangleLineMode(char *s, short mode)
{
#ifdef OPTS_DEBUG
	short 	i;
	
	for (i=0; i<5; i++) {
		if (mode & (1 << i))
			strcat(s, LMmodes[i]);
	}
#endif
}

void DemangleLineModeShort(char *s, short mode)
{
#ifdef OPTS_DEBUG
	short 	i;
	
	for (i=0; i<5; i++) {
		if (mode & (1 << i))
			strcat(s, LMmodeBits[i]);
		else
			strcat(s," ");
	}
#endif
}

#if 0	// Temp storage space - JMB 2.6b16.1
												/* First check to see if we need to reply */
												for (i=2;(tw->parsedat[i]!='\0') && (tw->parsedat[i]!=IAC); i+=3) {
													if (!(tw->parsedat[i+1] & SLC_AWK)) break;
													if ((unsigned char)(tw->parsedat[i+2])==IAC) i++;
													}

												/* if we do then send a reply */
												if ((tw->parsedat[i]!=IAC) && (tw->parsedat[i]!='\0')) {
												//	sprintf(s,"%c%c%c%c",IAC,TEL_SB,N_LINEMODE,L_SLC);
												//	netwrite(tw->port,s,4);
												//	opts_debug_print("SENT: IAC SB LINEMODE SLC");
	

#ifdef NEWSLC
													for (i=2; (tw->parsedat[i]!='\0') && (tw->parsedat[i]!=IAC); i+=3)
														{
														if (!(tw->parsedat[i+1] & SLC_AWK))		/* no response of ACK set */
															{
															if (tw->parsedat[i+1] != SLC_NOSUPPORT)
																{
																sprintf(s,"%c%c%c",tw->parsedat[i],SLC_NOSUPPORT,0);
#ifdef OPTS_DEBUG
																sprintf(munger,"     %s NOSUPPORT 0",LMoptions[tw->parsedat[i]]);
#endif
																}
															else		/* Acknowledge the No_Support case */
																{
																sprintf(s,"%c%c%c",tw->parsedat[i],SLC_AWK|SLC_NOSUPPORT,(char)0);
#ifdef OPTS_DEBUG
																sprintf(munger,"     %s NOSUPPORT|AWK 0",LMoptions[tw->parsedat[i]]);
#endif
																}																
#ifdef OPTS_DEBUG
															opts_debug_print(munger);
#endif
															netwrite(tw->port,s,3);
															}
															if ((unsigned char)(tw->parsedat[i+2])==IAC) i++;
														}

#else
													for (i=2, lmslcflag=0; (tw->parsedat[i]!='\0') && (tw->parsedat[i]!=IAC); i+=3) {
														// RFC 1184: If we get a response that is the same as the current setting, ignore it.  Technically, we
														//	should check the value for a CANTCHANGE response to satisfy part 2 of Sec 5.5.  However, since we
														//	advertised that key as CANTCHANGE in the first place, the server should not be changing it.  This
														//	should be fixed later, however I don't see it causing big problems right now. - JMB 12/93
														if (! ((((tw->parsedat[i+2] & SLC_LEVELBITS) == SLC_NOSUPPORT) && (tw->slc[tw->parsedat[i+1]] == -1))
															|| (((tw->parsedat[i+2] & SLC_LEVELBITS) == SLC_CANTCHANGE) && (tw->slc[tw->parsedat[i+1]] != -1)))) {
															// If we get to this point, the value sent to us doesn't agree with what we want.
															
																
															if (!(tw->parsedat[i+1] & SLC_AWK)) {
																if (!lmslcflag) {
																	lmslcflag = 1;		// Do this only once
																	sprintf(s,"%c%c%c%c",IAC,TEL_SB,N_LINEMODE,L_SLC);
																	netwrite(tw->port,s,4);
																	opts_debug_print("SENT: IAC SB LINEMODE SLC");
																	}
															
																if (tw->slc[tw->parsedat[i]]==-1)
																	{
																	sprintf(s,"%c%c%c",tw->parsedat[i],SLC_NOSUPPORT,0);
#ifdef OPTS_DEBUG
																	sprintf(munger,"     %s SLC_NOSUPPORT 0",LMoptions[tw->parsedat[i]]);
#endif
																	}
																else
																	{
																	sprintf(s,"%c%c%c",tw->parsedat[i],SLC_CANTCHANGE,(char)tw->slc[tw->parsedat[i]]);
#ifdef OPTS_DEBUG
																	sprintf(munger,"     %s SLC_CANTCHANGE %d",LMoptions[tw->parsedat[i]],
																								(char)tw->slc[tw->parsedat[i]]);
#endif
																	}																
#ifdef OPTS_DEBUG
																opts_debug_print(munger);
#endif
																netwrite(tw->port,s,3);
																if (tw->parsedat[i+2]==IAC) i++;
																}
															}
														}
														
#endif
													if (lmslcflag) {
														sprintf(s,"%c%c",IAC,TEL_SE);
														netwrite(tw->port,s,2);
														}
													}

													/* otherwise just exit */
													}
													break;
#endif