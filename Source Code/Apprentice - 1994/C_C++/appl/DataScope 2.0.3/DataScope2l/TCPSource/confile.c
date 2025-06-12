/*
*   Confile.c
*   Split from util.c 5/88
*   Reads and stores the appropriate information for the config file
*
*/
#include "stdio.h"
#include "memory.h"
#include "whatami.h"
#include "hostform.h"

#ifdef PC
#include "string.h"
#else
#define strchr(A,B) index(A,B)
#endif

#define NUMSPECS 131

char 							/* special function types */
			*neterrstring();
int32 time();					/* don't forget this sucker! */

static struct machinfo *Smachlist,*Smptr;
struct machinfo *Sns=NULL;

static unsigned char *Smachfile = {"config.tel"},
			Sflags[NUMSPECS-95],	/* which parms we have or have not */
			*Sspace;

struct config Scon = {
		0,0,0,0,
		0,
		3,
		127,0,0,1,
		"",
		7,1,0x70,
		"atalk",
		"ega",
		0,
		1,
		1,
		1,
		0,
		"DEC-VT100",
		"*",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		1,
		4,
		3,
		-1,
		120,
		0x0d000,
		0x0300
};

int
Sxxnf[3] = NFDEF,
Sxxnb[3] = NBDEF,
Sxxbf[3] = BFDEF,
Sxxbb[3] = BBDEF,
Sxxuf[3] = UFDEF,
Sxxub[3] = UBDEF;

static int
		mno=0,					/* how many machines in host file */
		lineno,					/* line number in hosts file */
		position,				/* position for scanning string */
		constate,				/* state for config file parser */
		inquote;				/* flag, inside quotes now */

/*
*   States for config file reading state machine.
*   One for each type of keyword and some for controlling.
*/

#define	CONNAME	101
#define CONHOST	102
#define CONIP	103
#define CONGATE	104
#define CONCOLOR 105
#define CONBKSP	106
#define CONBKSC	107
#define CONRETR	108
#define CONWIND	109
#define CONSEG	110
#define CONMTU	111
#define CONNS	112
#define CONTO	113
#define CONCRMAP 114
#define CONDUP  115
#define CONWRAP 116
#define CONWIDE 117
#define CONFONT 118
#define CONFSIZE 119
#define CONNF 120
#define CONNB 121
#define CONBF 122
#define CONBB 123
#define CONUF 124
#define CONUB 125
#define CONRF 126
#define CONRB 127
#define CONCLMODE 128
#define CONPORT 129
#define CONLINES 130
#define CONLKEYS 131
/*
*  above this line are per machine entries, below are configuration entries
*/
#define CONMYIP	132
#define CONHPF  133
#define CONPSF  134
#define CONTEKF 135
#define CONJTIME 136
#define CONME	137
#define CONCCOL	138
#define CONHW	139
#define CONADDR	140
#define CONIOA	141
#define CONDEF  142
#define CONCKEYS 143
#define CONINT	144
#define CONBIOS	145
#define CONTEK	146
#define CONVIDEO	147
#define CONFTP	148
#define CONRCP	149
#define CONPASS	150
#define CONCAP	151
#define CONTTYPE	152
#define CONNSTYPE 	153
#define CONFROM	154
#define CONARPTO 155
#define CONZONE 156
#define CONNDOM  157
#define CONDOMTO 158
#define CONBLOCK 159
#define	CONMASK	160


char *Skeyw[] = {
		"",	
		"name",							/* name of session */
		"host",							/* name of host */
		"hostip",						/* IP number */
		"gateway",						/* gateway level */
		"color",						/* color code  ==5== */
		"erase",						/* value to use for backspace */
		"scrollback",					/* how many lines to backscroll */
		"retrans",						/* initial retrans time */
		"rwin",							/* window to allow for this host */
		"maxseg",						/* maximum transfer size (in) ==10== */
		"mtu",							/* transfer unit (out) */
		"nameserver",					/* name server level */
		"contime",						/* timeout for opening connection */
		"crmap",						/* map for Berkeley 4.3 compatibility */
		"duplex",						/* half duplex for IBM machines */
		"vtwrap",						/* should VT wrap? */
		"vtwidth",						/* width of VT100 screen */
		"font",							/* font to use, when given a choice */
		"fsize",						/* font size, in points */
		"nfcolor",						/* normal foreground color */
		"nbcolor",						/* normal background color */
		"bfcolor",						/* blink foreground color */
		"bbcolor",						/* blink background color */
		"ufcolor",						/* underline foreground color */
		"ubcolor",						/* underline background color */
		"rfcolor",						/* reverse foreground color */
		"rbcolor",						/* reverse background color */
		"clearsave",					/* clear screen saves lines */
		"port",							/* TCP port to go for */
		"vtlines",						/* number of lines for the VT100 window */
		"localkeys",					/* keys to use for int,susp,resume */

/*
*  following are one-time entries, above are part of the data structure
*/
		"myip",							/* local machine's IP # */
		"hpfile",						/* HPGL output file */
		"psfile",						/* postscript output file */
		"tekfile",						/* tektronix output file */
		"timeslice",					/* timer slice for multi-tasking */
		"myname",						/* identifying info ==15==*/
		"concolor",						/* console colors */
		"hardware",						/* network hardware */
		"address",						/* Address of hardware */
		"ioaddr",						/* ioaddress of hardware */
		"domain",						/* default domain for lookup */
		"commandkeys",					/* use command keys on mac */
		"interrupt",					/* interrupt request 3 or 5 */
		"bios",							/* use BIOS screen */
		"tek",							/* tektronix graphics ==20==*/
		"video",						/* type of video hardware */
		"ftp",							/* enable ftp? */
		"rcp",							/* enable rcp? */
		"passfile",						/* password file name */
		"capfile",						/* capture file name */
		"termtype",						/* terminal type */
		"nameservertype",				/* nameserver type */
		"copyfrom",						/* copy from another machine */
		"arptime",						/* time-out for ARPs */
		"zone",							/* NBP zone for Macs */
		"domainretry",					/* # of retries */
		"domaintime",					/* time-out for DOMAIN */
		"block",						/* blocking for network update */
		"netmask",						/* subnetting mask */
		""
	};



/************************************************************************/
/*  Sgetconfig
*   copy the configuration information into the user's data structure
*   directly.  The user can do with it what he feels like.
*/
Sgetconfig(cp)
	struct config *cp;
	{

	movenbytes(cp,&Scon,sizeof(struct config));
	return(0);

}

/************************************************************************/
/*  Sreadhosts
*   read in the hosts file into our in-memory data structure.
*   Handle everything by keyword, see docs for specifications about file.
*/
Sreadhosts()
	{
	FILE *fp;
	int c,retval;

	Smachlist = Smptr = NULL;
	mno = 0;

	Sspace = (char *)NewPtr(256);				/* get room for gathering stuff */
	if (Sspace == NULL) {
		Serrline(901);
		return(1);
	}
	position = constate = inquote = lineno = 0;   /* state vars */	

	if (NULL == (fp = fopen(Smachfile,"r"))) {
		Serrline(900);
		return(1);
	}

	retval = 0;
	while (!retval) {
		c = fgetc(fp);
		if (c == '#' && !inquote) {
			while (c != EOF && c != '\n' && c != '\r')		/* skip to EOL */
				c = fgetc(fp);
		}
		if (c == '\n' || c == '\r')
			lineno++;
		retval = Scontoken(c);
	}

	fclose(fp);
	DisposPtr ((Ptr)Sspace);

	Smadd("default");				/* make sure name is in list */

	if (retval == EOF)				/* EOF is normal end */
		return(0);
	else
		return(retval);

}


/************************************************************************/
/*  ncstrcmp
*   No case string compare.
*   Only returns 0=match, 1=no match, does not compare greater or less
*   There is a tiny bit of overlap with the | 32 trick, but shouldn't be
*   a problem.  It causes some different symbols to match.
*/
ncstrcmp(sa,sb)
	char *sa,*sb;
	{

	while (*sa && *sa < 33)		/* don't compare leading spaces */
		sa++;
	while (*sb && *sb < 33)
		sb++;

	while (*sa && *sb) {
		if ((*sa != *sb) && ((*sa | 32) != (*sb | 32)))
			return(1);
		sa++;sb++;
	}
	if (!*sa && !*sb)		/* if both at end of string */
		return(0);
	else
		return(1);
}

/************************************************************************/
/*  Serrline
*   prints the line number of the host file error and posts the event
*   for the line number error and posts the hosts file error.
*/

Serrline(n)
	int n;
	{
	char *p;

	p = neterrstring(-1);
	sprintf(p,"Config file: error in line %4d",lineno+1);
	netposterr(-1);

	netposterr(n);
}

/************************************************************************/
/* Scontoken
*  tokenize the strings which get passed to Sconfile.
*  Handles quotes and uses separators:  <33, ;:=
*/
Scontoken(c)
	int c;
	{
	int retval;

	if (c == EOF) {
		Sspace[position++] = '\0';
		Sconfile(Sspace);
		if (!Sflags[0]) {			/* make sure last entry gets copied */
			if (ncstrcmp("default",Smptr->sname))
				Scopyfrom("default");
			else
				Scopyfrom("==");
		}
		return(-1);
	}

	if (!position && Sissep(c))		/* skip over junk before token */
		return(0);

	if (inquote || !Sissep(c)) {

		if (position > 200) {
			Serrline(903);
			return(1);
		}
/*
*  check for quotes, a little mixed up here, could be reorganized
*/
		if (c == '"' ) {
			if (!inquote) {			/* beginning of quotes */
				inquote = 1;
				return(0);
			}
			else
				inquote = 0;		/* turn off flag and drop through */

		}
		else {						
			if (c == '\n') {			/* check for EOL inside quotes */
				Serrline(904);
				return(1);
			}
			Sspace[position++] = c;    /* include in current string */
			return(0);
		}
				
	}

	Sspace[position++] = '\0';

	retval = Sconfile(Sspace);			/* pass the token along */

	position = 0;
	inquote = 0;
	Sspace[0] = '\0';

	return(retval);
}

/************************************************************************/
/*  Sconfile
*   take the characters read from the file and parse them for keywords
*   which require configuration action.
*/
Sconfile(s)
	char *s;
	{
	int i,a,b,c,d;

	switch (constate) {
		case 0:								/* lookup keyword */
			if (!(*s))						/* empty token */
				return(0);


			for (i=1; *Skeyw[i] && ncstrcmp(Skeyw[i],s); i++)
					;
			if (!(*Skeyw[i])) {			/* not in list */
				Serrline(902);
				return(0);				/* don't die - helps backward compatibility */
			}
			constate = 100+i;	/* change to state for keyword */
/*
*  check if this is a machine specific parm without a machine to
*  give it to.  "name" being the only machine specific parm allowed, of course
*/
			if (Smptr == NULL && constate > 101 && constate <= NUMSPECS) {
				Serrline(905);
				return(1);
			}
			break;

		case CONNAME:
/*
*  allocate space for upcoming parameters
*/
			if (Smachlist == NULL) {
				Smachlist = (struct machinfo *)NewPtr(sizeof(struct machinfo));
				Smptr = Smachlist;
				Smptr->sname = NULL;
				Smptr->hname = NULL;
			}
			else {
				if (!Sflags[0]) {
					if (ncstrcmp("default",Smptr->sname))
						Scopyfrom("default");
					else
						Scopyfrom("==");	/* to make sure 'default' gets set */
				}
				Smptr->next = 
					(struct machinfo *)NewPtr(sizeof(struct machinfo));
				Smptr = Smptr->next;
			}
			Smptr->next = NULL;
			Smptr->hname = NULL;				/* guarantee to be null */
			Smptr->sname = (char *)NewPtr(position);	/* size of name string */
			strcpy(Smptr->sname,s);				/* keep name field */
			constate = 0;						/* back to new keyword */
			for (i=0; i<NUMSPECS-99; i++)
				Sflags[i] = 0;					/* we have no parms */
			Smptr->mno = ++mno;					/* new machine number */
			break;

		case CONHOST:							/* also a name */
			Smptr->hname = (char *)NewPtr(position);
			strcpy(Smptr->hname,s);
			constate = 0;
			Sflags[CONHOST-100] = 1;
			break;

		case CONIP:								/* IP number for host */
			if ( 4 != sscanf(s,"%d.%d.%d.%d",&a,&b,&c,&d)) {
				Serrline(906);
				return(3);
			}
			Smptr->hostip[0]=a; Smptr->hostip[1] =b; 	/* keep number */
			Smptr->hostip[2]=c; Smptr->hostip[3] =d;
			Smptr->mstat = HFILE;
			constate = 0;
			Sflags[CONIP-100] = 1;
			break;

		case CONGATE:
			Smptr->gateway = atoi(s);			/* gateway level */
			constate = 0;
			Sflags[CONGATE-100] = 1;
			break;

		case CONCOLOR:					/* support old format */
			Smptr->nfcolor[0] = s[1]-48;
			Smptr->nbcolor[0] = s[0]-48;
			Smptr->bfcolor[0] = s[5]-48;
			Smptr->bbcolor[0] = s[4]-48;
			Smptr->ufcolor[0] = s[3]-48;
			Smptr->ubcolor[0] = s[2]-48;
			constate = 0;
			Sflags[CONNF-100] = 1;		/* sets them all at one shot */
			Sflags[CONNB-100] = 1;
			Sflags[CONBF-100] = 1;
			Sflags[CONBB-100] = 1;
			Sflags[CONUF-100] = 1;
			Sflags[CONUB-100] = 1;
			break;

		case CONNF:						/* foreground normal color */
			if (Scolorset(Smptr->nfcolor,s))
				Sflags[CONNF-100] = 1;
			constate = 0;
			break;
		case CONNB:						/* background normal color */
			if (Scolorset(Smptr->nbcolor,s))
				Sflags[CONNB-100] = 1;
			constate = 0;
			break;
		case CONRF:
		case CONBF:						/* blink foreg color */
			if (Scolorset(Smptr->bfcolor,s))
				Sflags[CONBF-100] = 1;	/* in copyfrom, r's are really b's */
			constate = 0;
			break;
		case CONRB:
		case CONBB:						/* blink bg color */
			if (Scolorset(Smptr->bbcolor,s))
				Sflags[CONBB-100] = 1;
			constate = 0;
			break;
		case CONUF:						/* foreground underline color */
			if (Scolorset(Smptr->ufcolor,s))
				Sflags[CONUF-100] = 1;
			constate = 0;
			break;
		case CONUB:						/* bg underline color */
			if (Scolorset(Smptr->ubcolor,s))
				Sflags[CONUB-100] = 1;
			constate = 0;
			break;

		case CONLKEYS:					/* local key bindings */
			{ int a,b,c;
				if (3!=sscanf(s,"{%d,%d,%d}", &a, &b, &c))
					Smptr->ckey = Smptr->skey = Smptr->qkey = 0;	/* default = off */
				else {
					Smptr->ckey = a;
					Smptr->skey = b;
					Smptr->qkey = c;
				}
			}
			Sflags[CONLKEYS-100] = 1;
			constate = 0;
			break;

		case CONBKSP:
			if (!ncstrcmp(s,"backspace"))
				Smptr->bksp = 8;
			else
				Smptr->bksp = 127;
			constate = 0;
			Sflags[CONBKSP-100] = 1;
			break;

		case CONBKSC:
			Smptr->bkscroll = atoi(s);
			constate = 0;
			Sflags[CONBKSC-100] = 1;
			break;

		case CONRETR:
			Smptr->retrans = atoi(s);
			constate = 0;
			Sflags[CONRETR-100] = 1;
			break;

		case CONWIND:
			Smptr->window = atoi(s);
			constate = 0;
			Sflags[CONWIND-100] = 1;
			break;

		case CONSEG:
			Smptr->maxseg = atoi(s);
			constate = 0;
			Sflags[CONSEG-100] = 1;
			break;

		case CONMTU:
			Smptr->mtu = atoi(s);
			constate = 0;
			Sflags[CONMTU-100] = 1;
			break;

		case CONNS:
			Smptr->nameserv = atoi(s);
			if (!Sns || (Sns->nameserv > Smptr->nameserv))	/* keep NS */
				Sns = Smptr;
			constate = 0;
			Sflags[CONNS-100] = 1;
			break;

		case CONTO:
			i = atoi(s);
			if (i > 2) {
				Smptr->conto = i;
				Sflags[CONTO-100] = 1;
			}
			constate = 0;
			break;

		case CONCRMAP:
			if (!ncstrcmp(s,"4.3BSDCRNUL")) 
				Smptr->crmap = 0;
			else
				Smptr->crmap = 10;
			Sflags[CONCRMAP-100] = 1;
			constate = 0;
			break;

		case CONDUP:
			if (!ncstrcmp(s,"half")) {
				Smptr->halfdup = 1;
				Sflags[CONDUP-100] = 1;
			}
			constate = 0;
			break;

		case CONWRAP:
			if ('Y' == toupper(s[0])) 
				Smptr->vtwrap = 1;
			else
				Smptr->vtwrap = 0;
			Sflags[CONWRAP-100] = 1;
			constate = 0;
			break;

		case CONCLMODE:
			if ('N' == toupper(s[0])) 
				Smptr->clearsave = 0;
			else
				Smptr->clearsave = 1;
			Sflags[CONCLMODE-100] = 1;
			constate = 0;
			break;

		case CONFONT:
			Smptr->font = (char *)NewPtr(position);
			strcpy(Smptr->font,s);
			Sflags[CONFONT-100] = 1;
			constate = 0;
			break;

		case CONFSIZE:
			Smptr->fsize = atoi(s);
			Sflags[CONFSIZE-100] = 1;
			constate = 0;
			break;

		case CONWIDE:
			if (132 == atoi(s)) 
				Smptr->vtwidth = 132;
			else
				Smptr->vtwidth = 80;

			Sflags[CONWIDE-100] = 1;
			constate = 0;
			break;

		case CONLINES:
			i = atoi(s);
			if (i > 6 && i < 100)
				Smptr->nlines = i;
			else
				Smptr->nlines = 24;			/* default is 24 lines */
			Sflags[CONLINES-100] = 1;
			constate = 0;
			break;

		case CONPORT:						/* File name for Tek dump */
			i = atoi(s);
			if (i < 1)
				i = 23;
			Smptr->port = i;
			Sflags[CONPORT-100] = 1;
			constate = 0;
			break;

/*
*  now the one-time entries
*  Generally this information goes into the "Scon" structure for later
*  retrieval by other routines.
*
*/
#ifdef PC
		case CONMASK:
			if ( 4 != sscanf(s,"%d.%d.%d.%d",&a,&b,&c,&d)) {
				Serrline(907);
				return(3);
			}
			Scon.netmask[0]=a; Scon.netmask[1] =b; 	
			Scon.netmask[2]=c; Scon.netmask[3] =d;
			Scon.havemask=1;
			constate = 0;
			break;

		case CONMYIP:
			constate = 0;
			if (!ncstrcmp(s,"rarp")) {
				movebytes(Scon.myipnum,s,4);
				netsetip("RARP");
				break;
			}
			if ( 4 != sscanf(s,"%d.%d.%d.%d",&a,&b,&c,&d)) {
				Serrline(908);
				return(3);
			}
			Scon.myipnum[0]=a; Scon.myipnum[1] =b; 	/* put number back in s */
			Scon.myipnum[2]=c; Scon.myipnum[3] =d;
			netsetip(Scon.myipnum);		/* make permanent set */
			break;
#endif
		case CONME:				/* what my name is  */
			strncpy(Scon.me,s,30);
			Scon.me[30] = '\0';
			constate = 0;
			break;

		case CONHW:				/* what hardware are we using? */
			i = strlen(s);
			if (i > 9) i = 9;
			s[i] = '\0';
			i--;
			while (i--)
				s[i] = tolower(s[i]);
			strcpy(Scon.hw,s);
			constate = 0;
			break;

#ifdef PC
		case CONINT:			/* what IRQ to use */
			sscanf(s,"%x",&i);
			Scon.irqnum = i;
			constate = 0;
			break;

		case CONBIOS:
			if (toupper(*s) == 'Y') {
				Scwritemode(0);
				Scon.bios = 1;
			}
			constate = 0;
			break;

		case CONADDR:				/* segment address for board */
			sscanf(s,"%x",&i);
			Scon.address = i;
			constate = 0;
			break;

		case CONIOA:				/* io address for board */
			sscanf(s,"%x",&i);
			Scon.ioaddr = i;
			constate = 0;
			break;
#endif
#ifdef MAC
		case CONCKEYS:
			if (toupper(*s) == 'Y') {
				Scon.comkeys = 1;
			}
			constate = 0;
			break;

		case CONZONE:
			Scon.zone = (char *)NewPtr(position);	/* space for name */
			strcpy(Scon.zone,s);			/* copy it in */
			constate = 0;
			break;

		case CONJTIME:				/* Time slice */
			i = atoi(s);
			if (i > 1)
				Scon.timesl = i;
			constate = 0;
			break;
#endif		

		case CONTEK:
			if (toupper(*s) == 'N') {
				Stekmode(0);
				Scon.tek = 0;
			}
			constate = 0;
			break;

		case CONVIDEO:
			i = strlen(s);
			if (i > 9) i = 9;
			s[i] = '\0';
			strcpy(Scon.video,s);
			i--;
			while (i--)
				s[i] = tolower(s[i]);
			constate = 0;
			break;

		case CONTTYPE:
			Scon.termtype = (char *)NewPtr(position);
			strcpy(Scon.termtype,s);
			constate = 0;
			break;

		case CONCCOL:
			for (i=0; i<3; i++)
				Scon.color[i] = ((s[i*2]-48)<<4) + (s[i*2+1]-48);
			constate = 0;
			break;

		case CONFTP:
			if (toupper(*s) == 'N') 
				Scon.ftp = 0;	
			constate = 0;
			break;

		case CONRCP:
			if (toupper(*s) == 'N')
				Scon.rcp = 0;
			constate = 0;
			break;

		case CONPASS:
			Scon.pass = (char *)NewPtr(position);	/* space for name */
			strcpy(Scon.pass,s);			/* copy it in */
			constate = 0;
			break;

		case CONDEF:						/* default domain */
			Scon.defdom = (char *)NewPtr(position);	/* space for name */
			strcpy(Scon.defdom,s);			/* copy it in */
			constate = 0;
			break;

		case CONCAP:						/* capture file name */
			Snewcap(s);
			constate = 0;
			break;

		case CONFROM:						/* copy the rest from another */
											/* entry in the table */
			if (Scopyfrom(s))
				return(1);
			Sflags[0] = 1;					/* indicate did copy from */
			constate = 0;
			break;

		case CONARPTO:						/* need to lengthen arp time-out (secs) */
			i = atoi(s);
			if (i > 0)
				netarptime(i);
			constate = 0;					/* don't forget me! */
			break;

		case CONBLOCK:						/* blocking factor for text */
			i = atoi(s);
			if (i > 4096) i = 4096;
			if (i > 0)
				Scon.textblock = i;
			constate = 0;					/* don't forget me! */
			break;

		case CONDOMTO:						/* DOMAIN timeout value */
			i = atoi(s);
			if (i > 1)
				Scon.domto = i;
			constate = 0;
			break;

		case CONNDOM:						/* DOMAIN number of retries */
			i = atoi(s);
			if (i > 1)
				Scon.ndom = i;
			constate = 0;
			break;

		case CONHPF:				/* File name for HP dump */
			Snewhpfile(s);
			constate = 0;
			break;

		case CONPSF:				/* File name for PS dump */
			Snewpsfile(s);
			constate = 0;
			break;

		case CONTEKF:				/* File name for Tek dump */
			Snewtekfile(s);
			constate = 0;
			break;

		default:
			constate = 0;
			break;
	}

	return(0);
}

/************************************************************************/
/*  Scopyfrom
*   Look at the Sflags array to determine which elements to copy from
*   a previous machine's entries.  If a machine name as been given as
*   "default", the state machine will fill in the fields from that
*   machine's entries.
*
*   If the machine name to copyfrom is not present in the list, set the
*   program default values for each field.
*/
Scopyfrom(s)
	char *s;
	{
	struct machinfo *m;
	int i;

	m = Shostlook(s);			/* search list */

	for (i=3; i <= NUMSPECS-100; i++) 		/* through list of parms */
		if (!Sflags[i]) {
			if (m) 							/* copy old value */
			switch (100+i) {
				case CONHOST:
					Smptr->hname = m->hname;
					break;
				case CONIP:
					movebytes(Smptr->hostip,m->hostip,4);
					Smptr->mstat = m->mstat;
					break;
				case CONGATE:			/* gateways cannot be copied from */
					Smptr->gateway = 0;
					break;
				casecCONNS:					/* can't copy nameservers either */
					Smptr->nameserv = 0;
					break;

				case CONBKSP:
					Smptr->bksp = m->bksp;
					break;
				case CONBKSC:
					Smptr->bkscroll = m->bkscroll;
					break;
				case CONCLMODE:
					Smptr->clearsave = m->clearsave;
					break;
				case CONRETR:
					Smptr->retrans = m->retrans;
					break;
				case CONWIND:
					Smptr->window = m->window;
					break;
				case CONSEG:
					Smptr->maxseg = m->maxseg;
					break;
				case CONMTU:
					Smptr->mtu = m->mtu;
					break;

				case CONTO:
					Smptr->conto = m->conto;
					break;
				case CONCRMAP:
					Smptr->crmap = m->crmap;
					break;
				case CONDUP:
					Smptr->halfdup = m->halfdup;
					break;
				case CONWRAP:
					Smptr->vtwrap = m->vtwrap;
					break;
				case CONWIDE:
					Smptr->vtwidth = m->vtwidth;
					break;
				case CONLINES:
					Smptr->nlines = m->nlines;
					break;

				case CONNF:
					movebytes(Smptr->nfcolor,m->nfcolor,3*sizeof(int));
					break;
				case CONNB:
					movebytes(Smptr->nbcolor, m->nbcolor,3*sizeof(int));
					break;
				case CONBF:
					movebytes(Smptr->bfcolor,m->bfcolor,3*sizeof(int));
					break;
				case CONBB:
					movebytes(Smptr->bbcolor,m->bbcolor,3*sizeof(int));
					break;
				case CONUF:
					movebytes(Smptr->ufcolor,m->ufcolor,3*sizeof(int));
					break;
				case CONUB:
					movebytes(Smptr->ubcolor,m->ubcolor,3*sizeof(int));
					break;

				case CONLKEYS:
					Smptr->ckey = m->ckey;
					Smptr->skey = m->skey;
					Smptr->qkey = m->qkey;
					break;

				case CONFONT:
					Smptr->font = m->font;
					break;
				case CONFSIZE:
					Smptr->fsize = m->fsize;
					break;
				case CONPORT:
					Smptr->port = m->port;
					break;

				default:
					break;
			}
			else
			switch (100+i) {		/* m=NULL, install default values */
				case CONHOST:
					Smptr->hname = NULL;
					break;
				case CONIP:
					Smptr->mstat = NOIP;
					break;
				case CONGATE:			/* gateways cannot be copied from */
					Smptr->gateway = 0;
					break;
				case CONBKSP:
					Smptr->bksp = 127;
					break;
				case CONBKSC:
					Smptr->bkscroll = 0;
					break;
				case CONCLMODE:
					Smptr->clearsave = 1;
					break;
				case CONRETR:
					Smptr->retrans = SMINRTO;
					break;
				case CONWIND:
#ifdef MAC
					Smptr->window = 512;
#else
					Smptr->window = DEFWINDOW;
#endif
					break;
				case CONSEG:
#ifdef MAC
					Smptr->maxseg = 512;
#else
					Smptr->maxseg = DEFSEG;
#endif
					break;
				case CONMTU:
#ifdef MAC
					Smptr->mtu = 512;
#else
					Smptr->mtu = TSENDSIZE;
#endif
					break;

				case CONNS:					/* can't copy nameservers either */
					Smptr->nameserv = 0;
					break;
		
				case CONTO:
					Smptr->conto = CONNWAITTIME;
					break;

				case CONCRMAP:
					Smptr->crmap = 10;
					break;

				case CONDUP:
					Smptr->halfdup = 0;
					break;
				case CONWRAP:
					Smptr->vtwrap = 0;
					break;
				case CONWIDE:
					Smptr->vtwidth = 80;
					break;
				case CONLINES:
					Smptr->nlines = 24;		/* overall default to 24 lines */
					break;

				case CONNF:
					movebytes(Smptr->nfcolor,Sxxnf,3*sizeof(int));
					break;
				case CONNB:
					movebytes(Smptr->nbcolor,Sxxnb,3*sizeof(int));
					break;
				case CONBF:
					movebytes(Smptr->bfcolor,Sxxbf,3*sizeof(int));
					break;
				case CONBB:
					movebytes(Smptr->bbcolor,Sxxbb,3*sizeof(int));
					break;
				case CONUF:
					movebytes(Smptr->ufcolor,Sxxuf,3*sizeof(int));
					break;
				case CONUB:
					movebytes(Smptr->ubcolor,Sxxub,3*sizeof(int));
					break;

				case CONLKEYS:
					Smptr->ckey = 3;			/* Ctrl-C */
					Smptr->skey = 19;			/* Ctrl-S */
					Smptr->qkey = 17;			/* Ctrl-Q */
					break;

				case CONFONT:
					Smptr->font = "Monaco";
					break;
				case CONFSIZE:
					Smptr->fsize = 9;
					break;
				case CONPORT:
					Smptr->port = 23;			/* the telnet port */
					break;

				default:
					break;
			}
		}

	Sflags[0] = 1;					/* set that this machine was copied */
	return(0);
}

/************************************************************************/
/*  Smadd
*   Add a machine to the list. Increments machine number of machine.
*   Puts in parameters copied from the "default" entry.
*
*/
struct machinfo 
*Smadd(mname)
	char *mname;
	{
	int i;
	struct machinfo *m;
/*
*  First, do we have the name already?
*/
	m = Shostlook(mname);
	if (m)
		return(m);
/*
*   Don't have name, add another record
*/
	Smptr = (struct machinfo *)NewPtr(sizeof(struct machinfo));
	if (Smptr == NULL)
		return(NULL);

	for (i=0; i < NUMSPECS-99; i++)
		Sflags[i] = 0;					/* we have no parms */
	Scopyfrom("default");

	Smptr->sname = NULL;
	Smptr->hname = (char *)NewPtr(strlen(mname)+1);
	if (Smptr->hname)
		strcpy(Smptr->hname,mname);		/* copy in name of machine */
	Smptr->mno = ++mno;
	Smptr->mstat = NOIP;

	Smptr->next = Smachlist;			/* add to front of machlist */
	Smachlist = Smptr;

	return(Smptr);

}


/************************************************************************/
/* Shostfile
*   if the user wants to change the host file name from 'config.tel' to
*   something else.
*/
Shostfile(ptr)
	char *ptr;
	{
	Smachfile = ptr;	
/*
*  note that the area with the file name must stay allocated for
*  later reference, typically it is in some argv[] parm.
*/

}

/************************************************************************/
/*  get host by name
*   Given the name of a host machine, search our database to see if we
*   have that host ip number.  Search first the name field, and then the
*   hostname field.
*   Returns the pointer to the correct record, or pointer to the default
*   record if the number is not available
*/
struct machinfo 
*Sgethost(machine)
	char *machine;
	{
	int i,j,k,l;
	unsigned char ipto[4],myipnum[4],xmask[4];
	unsigned long hnum;
	struct machinfo *m;

	m = NULL;
/*
*  First, check for the pound sign character which means we should use
*  the current netmask to build an IP number for the local network.
*  Take the host number, install it in the ipto[] array.  Then mask
*  in my IP number's network portion to build the final IP address.
*/

	if ('#' == machine[0]) {		/* on my local network */
		netgetip(myipnum);
		netgetmask(xmask);			/* mask of network portion of IP # */

		sscanf(&machine[1],"%ld",&hnum);/* host number for local network */
		for (i=3; i >= 0; i--) {
			ipto[i] = hnum & 255L;	/* take off a byte */
			hnum >>= 8;				/* shift it over */
		}

		for (i=0; i < 4; i++) 
			ipto[i] |= (myipnum[i] & xmask[i]);		/* mask new one in */

	}
/*
*  next, is it an IP number?  We take it if the number is in four
*  parts, separated by periods.
*/
	else 
	if (4 == sscanf(machine,"%d.%d.%d.%d",&i,&j,&k,&l)) {	/* given ip num */
		ipto[0] = i;
		ipto[1] = j;
		ipto[2] = k;
		ipto[3] = l;
	}
/*
*  lastly, it must be a name, first check the local host table
*  A first number of 127 means that it doesn't have an IP number, but
*  is in the table (strange occurrence)
*/
	else {									/* look it up */

		m = Shostlook(machine);
		if (m == NULL) {
			netposterr(805);				/* informative */
			return(NULL);
		} 
		if (m->mstat < HAVEIP) {
			netposterr(806);				/* informative */
			return(NULL);
		}
	}

	if (!m) {
		m = Shostlook("default");
		movebytes(m->hostip,ipto,4);		/* copy in newest host # */
		m->mstat = HAVEIP;					/* we have the IP # */
	}

	return(m);
}

struct machinfo 
*Shostlook(hname)
	char *hname;
	{
	struct machinfo *m;
	m = Smachlist;
	while (m != NULL) {

		if (m->sname && !ncstrcmp(hname,m->sname)) 
			return(m);

		m = m->next;
	}

	m = Smachlist;
	while (m != NULL) {
		if (m->hname && !ncstrcmp(hname,m->hname))
			return(m);

		m = m->next;
	}

	return(NULL);

}

/************************************************************************/
/*  Slooknum
*   get the host record by machine number, used primarily in DOMAIN name
*   lookup.
*/
struct machinfo 
*Slooknum(num)
	int num;
	{
	struct machinfo *m;

	m = Smachlist;
	while (m) {
		if (m->mno == num)
			return(m);
		m = m->next;
	}

	return(NULL);

}

/**************************************************************************/
/*  Slookip
*   For FTP to look up the transfer options to use when running
*
*/
struct machinfo 
*Slookip(ipnum)
	unsigned char *ipnum;
	{
	struct machinfo *m;

	m = Smachlist;
	while (m) {
		if (comparen(m->hostip,ipnum,4))
			return(m);
		m = m->next;
	}

	return(NULL);

}

/**************************************************************************/
/*  Sissep
*   is the character a valid separator for the hosts file?
*   separators are white space, special chars and :;=
*
*/
Sissep(c)
	int c;
	{
	if (c < 33)
		return(1);
	if (c == ':' || c == ';' || c == '=')
		return(1);
	return(0);
}

/*********************************************************************/
/*  Snewns()
*   Rotate to the next nameserver
*   Chooses the next highest number from the nameserv field
*/
Snewns()
	{
	struct machinfo *m,*low;
	int i;

	if (!Sns)					/* safety, should never happen */
		Sns = Smachlist;

	low = Sns;
	i = Sns->nameserv;			/* what is value now? */

	m = Smachlist;
	while (m) {
		if (m->nameserv == i+1) {
			Sns = m;
			return(0);
		}
		if ((m->nameserv > 0) && (m->nameserv < low->nameserv))
			low = m;
		m = m->next;
	}

	if (Sns == low)
		return(1);				/* no alternate */
	else
		Sns = low;

	return(0);
}


Ssetns(ipn)
	unsigned char ipn[4];
	{
	struct machinfo *m;
	int i;

	i = 0;
	if (NULL == (m = Slookip(ipn))) {		/* have already? */
		m = Smadd("=nameserv=");
		movebytes(m->hostip,ipn,4);
		m->mstat = FROMKIP;
		i = 1;
	}

	m->nameserv = 1;
	Sns = m;

	return(i);
}

/************************************************************************/
/*  setgates
*   set up the gateway machines and the subnet mask after netinit()
*   and start up ftp and rcp for them.
*/
Ssetgates()
	{
	struct machinfo *m;
	int level,again;

	if (Scon.havemask)					/* leave default unless specified */
		netsetmask(Scon.netmask);
/*
*  Search the list of machines for gateway flags.
*  Invoke netsetgate in increasing order of gateway level #s.
*  Terminates when it gets through list without finding next higher number.
*/
	level = 0;
	do {
		level++;
		again = 0;
		m = Smachlist;
		while (m != NULL) {
			if (m->gateway == level && m->mstat >= HAVEIP) 
				netsetgate(m->hostip);
			if (m->gateway == level+1)
				again=1;
			m = m->next;
		}
	} while (again);

	Sftpmode(Scon.ftp);
#ifdef PC
	Srcpmode(Scon.rcp);
#endif
	return(0);
}
