#ifdef TN3270
#define WDATA struct cnr_
#endif

#ifdef NCSA_ENC
#define WDATA struct WindRec
#endif

#include "encrypt.h"
#include "KrbDriver.h"

#define MAXKEYLEN 64					/* for encryption */

#define P(x) x

/*
 * Kerberos, encryption
 */
#define OPT_AUTHENTICATION 37
#define OPT_ENCRYPT 38

#define KRB_REJECT		1		/* Rejected (reason might follow) */
#define KRB_AUTH		0		/* Authentication data follows */
#define KRB_ACCEPT		2		/* Accepted */
#define KRB_CHALLENGE	3		/* Challenge for mutual auth */
#define KRB_RESPONSE	4		/* Response for mutual auth */

#define TNQ_IS			0		/* Option is ... */
#define TNQ_SEND		1		/* send option */
#define TNQ_REPLY		2		/* suboption reply */
#define TNQ_NAME		3		/* suboption name */

/*
* AUTHENTICATION option types
*/
#define AUTH_NULL        0      /* no authentication */
#define AUTH_KERBEROS_V4 1      /* Kerberos version 4 */
#define AUTH_KERBEROS_V5 2      /* Kerberos version 5 */

/*
* AUTHENTICATION option modifiers
*/
#define AUTH_WHO_MASK         1
#define AUTH_CLIENT_TO_SERVER 0
#define AUTH_SERVER_TO_CLIENT 1
#define AUTH_HOW_MASK         2
#define AUTH_HOW_ONE_WAY      0
#define AUTH_HOW_MUTUAL       2

/*
 * suboption buffer offsets 
 */
#define SB_OPTION    0			/* option byte */
#define SB_SUBOPTION 1          /* is, send, reply, name */
#define SB_TYPE      2          /* authentication type */
#define SB_MODIFIER  3          /* type modifier */
#define SB_DATATYPE  4          /* type of data */
#define SB_DATA      5          /* offset to first data byte */

/*
 * ENCRYPTION suboptions
 */
#define	ENCRYPT_IS			0	/* I pick encryption type ... */
#define	ENCRYPT_SUPPORT		1	/* I support encryption types ... */
#define	ENCRYPT_REPLY		2	/* Initial setup response */
#define	ENCRYPT_START		3	/* Am starting to send encrypted */
#define	ENCRYPT_END			4	/* Am ending encrypted */
#define	ENCRYPT_REQSTART	5	/* Request you start encrypting */
#define	ENCRYPT_REQEND		6	/* Request you send encrypting */
#define	ENCRYPT_ENC_KEYID	7
#define	ENCRYPT_DEC_KEYID	8
#define	ENCRYPT_CNT			9

#define	ENCTYPE_ANY			0
#define	ENCTYPE_DES_CFB64	1
#define	ENCTYPE_DES_OFB64	2
#define	ENCTYPE_CNT			3

#define	IAC	255
#define	SB	250
#define	SE	240

struct key_info {						/* for encryption */
	unsigned char keyid[MAXKEYLEN];
	long keylen;
	long dir;
	long *modep;
#ifdef MPW
	Encryptions *(*getcrypt)();
#else
	Encryptions *(*getcrypt)(struct edata_ *, long);
#endif
};

struct fb {								/* for encryption */
	des_cblock krbdes_key;
	des_key_schedule krbdes_sched;
	Block temp_feed;
	unsigned char fb_feed[64];
	long need_start;
	long state[2];
	long keyid[2];
	long once;
	struct stinfo {
		des_cblock			str_output;
		des_cblock			str_feed;
		des_cblock			str_iv;
		des_cblock			str_ikey;
		des_key_schedule	str_sched;
		long			str_index;
		long			str_flagshift;
	} streams[2];
};

 
 #ifdef notdef
 	short will_wont_resp_encrypt;
 	short do_dont_resp_encrypt;
 	short o_encrypt;
 #endif
 
 
 #define BOGUS 0xf0d01111
 
 /* 
  * Encryption data 
  */
 typedef struct edata_ {
 	void (*encrypt_output)(void *, unsigned char *, long);
 	long (*decrypt_input)(void *, long);
 
 	long encrypt_debug_mode;
 	long decrypt_mode;
 	long encrypt_mode;
 	long encrypt_verbose;
 	long havesessionkey;
 	long autoencrypt;
 	long autodecrypt;
 
 	long Server;
 	char *Name;
  
 	long i_support_encrypt;
 	long i_support_decrypt;
 	long i_wont_support_encrypt;
 	long i_wont_support_decrypt;
  
 	long remote_supports_encrypt;
 	long remote_supports_decrypt;
  
 	unsigned char str_send[64];
 	unsigned char str_suplen;
 	unsigned char str_start[72];
 	unsigned char str_end[6];
  
 	struct key_info ki[2];
 	unsigned char str_keyid[(MAXKEYLEN*2)+5];
  
 	struct fb fb[2];
 	
 	WDATA *wp;
 } CDATA;
 
 /* 
  * Authorization / encryption data 
  */
 #define ADATA \
  	char auth_response[8];		/* kerberos expected authorization response */ \
 	char auth_challenge[8];  	/* kerberos authorization challenge */ \
 	CDATA *edata;
