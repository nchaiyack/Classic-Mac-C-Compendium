
/* These are the different Telnet option negotiation tags */
#define TEL_EOF			236			/* BYU LSC */
#define	TEL_SE			240			/* end sub-negitiation */				// 360
#define	TEL_NOP			241			/* No Operation */
#define TEL_DM			242			/* data mark */
#define TEL_BREAK		243			
#define TEL_IP			244			/* the ole' Interrupt Process */
#define TEL_AO			245			/* Abort Output */
#define	TEL_AYT			246			/* Are You There???? */
#define	TEL_EC			247			/* Erase Char */
#define TEL_EL			248			/* Erase Line */
#define TEL_GA			249			/* Telnet Go-Ahead */
#define	TEL_SB			250			/* start a Subnegotion */				// 372
#define TEL_WILLTEL 	251			/* Negotiate:  Will do this option */	// 373
#define TEL_WONTTEL 	252			/* Negotiate:  Wont do this option */	// 374
#define TEL_DOTEL		253			/* Negotiate:  Do this option */		// 375
#define TEL_DONTTEL 	254			/* Negotiate:  Dont do this option */	// 376

/* these are the rest of the defines needed for the option negitiations */
#define STNORM				0			/* parsing data normal */
#define	NEGOTIATE			1			/* Subnegotiation data follows */
#define ESCFOUND 			5			/* we have found an ESC sequence */
#define IACFOUND 			6			/* negotiation data follows */
#define	SEXECUTE			200
#define	IAC					255			/* signals negotiation data is coming */
#define GS					29			/* we can drop into TEK from here */


/* these are some of the various options that we can negotiate about */
#define N_BINARY 			0			/* binary data transfer */
#define N_ECHO				1			/* for local echo stuff */
#define N_SGA				3			/* Go ahead */
#define N_STATUS			5		
#define N_TIMING			6			/* timing mark */
#define	N_TERMTYP			24			/* set the terminal type */
#define N_NAWS				31			/* Negotiate About Window Size */
#define	N_TERMSPEED			32			/* how fast can we go? */	
#define N_REMOTEFLOW		33			/* do Remote Flow Control */
#define N_LINEMODE 			34			/* Ah yes, the infamous Line Mode option */
	#define L_MODE 		1
	#define L_FORWARDMASK 2
	#define L_SLC 		3 
#define N_AUTHENTICATION	37			/* Authentication */
#define N_ENCRYPT			38			/* Encryption */


/* Values for LINEMODE MODE */
#define L_EDIT    	1
#define L_TRAPSIG  	2
#define L_MODE_ACK	4
#define	L_SOFT_TAB	8
#define	L_LIT_ECHO	16
#define	L_MODE_MASK	31

/* these are the local keys, defined right before our very eyes!! */
/* used for line-mode negotiations */
#define SLC_NOSUPPORT	0
#define SLC_CANTCHANGE	1
#define SLC_VALUE		2
#define SLC_DEFAULT		3
#define SLC_LEVELBITS	3
#define SLC_AWK			128

#define	SLC_MAX			18

#define SLC_SYNCH		1
#define SLC_BRK			2
#define SLC_IP			3
#define SLC_AO			4
#define SLC_AYT			5
#define SLC_EOR			6
#define SLC_ABORT		7
#define SLC_EOF			8
#define SLC_SUSP		9
#define SLC_EC			10
#define SLC_EL   		11
#define SLC_EW   		12
#define SLC_RP			13
#define SLC_LNEXT		14
#define SLC_XON			15
#define SLC_XOFF		16
#define SLC_FORW1		17
#define SLC_FORW2		18

#define	substate	tw->substat
