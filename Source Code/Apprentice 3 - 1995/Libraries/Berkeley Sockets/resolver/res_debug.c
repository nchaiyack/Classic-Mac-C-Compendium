/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)res_debug.c	5.22 (Berkeley) 3/7/88";
#endif /* LIBC_SCCS and not lint */

#if defined(lint) && !defined(DEBUG)
#define DEBUG
#endif

#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/nameser.h>
#ifdef XFER
#include <syslog.h>
#include <netdb.h>
extern char *malloc();
#endif /* XFER */

extern char *p_cdname(), *p_rr(), *p_type(), *p_class(), *p_time();
extern char *inet_ntoa();

char *_res_opcodes[] = {
	"QUERY",
	"IQUERY",
	"CQUERYM",
	"CQUERYU",
	"4",
	"5",
	"6",
	"7",
	"8",
	"UPDATEA",
	"UPDATED",
	"UPDATEDA",
	"UPDATEM",
	"UPDATEMA",
	"ZONEINIT",
	"ZONEREF",
};

char *_res_resultcodes[] = {
	"NOERROR",
	"FORMERR",
	"SERVFAIL",
	"NXDOMAIN",
	"NOTIMP",
	"REFUSED",
	"6",
	"7",
	"8",
	"9",
	"10",
	"11",
	"12",
	"13",
	"14",
	"NOCHANGE",
};

p_query(msg,msglen)
	char *msg;
	int msglen;
{
#ifdef DEBUG
	fp_query(msg,msglen,stdout);
#endif
}

/*
 * Print the contents of a query.
 * This is intended to be primarily a debugging routine.
 */
fp_query(msg,msglen,file)
	char *msg;
	int msglen;
	FILE *file;
{
#ifdef DEBUG
	register char *cp;
	register HEADER *hp;
	register int n;

	/*
	 * Print header fields.
	 */
	hp = (HEADER *)msg;
	cp = msg + sizeof(HEADER);
	fprintf(file,"HEADER:\n");
	fprintf(file,"\topcode = %s", _res_opcodes[hp->opcode]);
	fprintf(file,", id = %d", ntohs(hp->id));
	fprintf(file,", rcode = %s\n", _res_resultcodes[hp->rcode]);
	fprintf(file,"\theader flags: ");
	if (hp->qr)
		fprintf(file," qr");
	if (hp->aa)
		fprintf(file," aa");
	if (hp->tc)
		fprintf(file," tc");
	if (hp->rd)
		fprintf(file," rd");
	if (hp->ra)
		fprintf(file," ra");
	if (hp->pr)
		fprintf(file," pr");
	fprintf(file,"\n\tqdcount = %d", ntohs(hp->qdcount));
	fprintf(file,", ancount = %d", ntohs(hp->ancount));
	fprintf(file,", nscount = %d", ntohs(hp->nscount));
	fprintf(file,", arcount = %d\n\n", ntohs(hp->arcount));
	/*
	 * Print question records.
	 */
	if (n = ntohs(hp->qdcount)) {
		fprintf(file,"QUESTIONS:\n");
		while (--n >= 0) {
			fprintf(file,"\t");
			cp = p_cdname(cp, msg, file);
			if (cp == NULL)
				return;
			fprintf(file,", type = %s", p_type(_getshort(cp)));
			cp += sizeof(u_short);
			fprintf(file,", class = %s\n\n", p_class(_getshort(cp)));
			cp += sizeof(u_short);
		}
	}
	/*
	 * Print authoritative answer records
	 */
	if (n = ntohs(hp->ancount)) {
		fprintf(file,"ANSWERS:\n");
		while (--n >= 0) {
			fprintf(file,"\t");
			cp = p_rr(cp, msg, file);
			if (cp > msg + msglen) {
				fprintf(file, "garbled answer!\n");
				return;
			}
			if (cp == NULL)
				return;
		}
	}
	/*
	 * print name server records
	 */
	if (n = ntohs(hp->nscount)) {
		fprintf(file,"NAME SERVERS:\n");
		while (--n >= 0) {
			fprintf(file,"\t");
			cp = p_rr(cp, msg, file);
			if (cp == NULL)
				return;
		}
	}
	/*
	 * print additional records
	 */
	if (n = ntohs(hp->arcount)) {
		fprintf(file,"ADDITIONAL RECORDS:\n");
		while (--n >= 0) {
			fprintf(file,"\t");
			cp = p_rr(cp, msg, file);
			if (cp == NULL)
				return;
		}
	}
#endif
}

char *
p_cdname(cp, msg, file)
	char *cp, *msg;
	FILE *file;
{
#ifdef DEBUG
	char name[MAXDNAME];
	int n;

	if ((n = dn_expand(msg, msg + 512, cp, name, sizeof(name))) < 0)
		return (NULL);
	if (name[0] == '\0') {
		name[0] = '.';
		name[1] = '\0';
	}
	fputs(name, file);
	return (cp + n);
#endif
}

/*
 * Print resource record fields in human readable form.
 */
char *
p_rr(cp, msg, file)
	char *cp, *msg;
	FILE *file;
{
#ifdef DEBUG
	int type, class, dlen, n, c;
	struct in_addr inaddr;
	char *cp1;

	if ((cp = p_cdname(cp, msg, file)) == NULL)
		return (NULL);			/* compression error */
	fprintf(file,"\n\ttype = %s", p_type(type = _getshort(cp)));
	cp += sizeof(u_short);
	fprintf(file,", class = %s", p_class(class = _getshort(cp)));
	cp += sizeof(u_short);
	fprintf(file,", ttl = %s", p_time(_getlong(cp)));
	cp += sizeof(u_long);
	fprintf(file,", dlen = %d\n", dlen = _getshort(cp));
	cp += sizeof(u_short);
	cp1 = cp;
	/*
	 * Print type specific data, if appropriate
	 */
	switch (type) {
	case T_A:
		switch (class) {
		case C_IN:
			bcopy(cp, (char *)&inaddr, sizeof(inaddr));
			if (dlen == 4) {
				fprintf(file,"\tinternet address = %s\n",
					inet_ntoa(inaddr));
				cp += dlen;
			} else if (dlen == 7) {
				fprintf(file,"\tinternet address = %s",
					inet_ntoa(inaddr));
				fprintf(file,", protocol = %d", cp[4]);
				fprintf(file,", port = %d\n",
					(cp[5] << 8) + cp[6]);
				cp += dlen;
			}
			break;
		default:
			cp += dlen;
		}
		break;
	case T_CNAME:
	case T_MB:
#ifdef OLDRR
	case T_MD:
	case T_MF:
#endif /* OLDRR */
	case T_MG:
	case T_MR:
	case T_NS:
	case T_PTR:
		fprintf(file,"\tdomain name = ");
		cp = p_cdname(cp, msg, file);
		fprintf(file,"\n");
		break;

#ifdef	T_UNAME
	case T_UNAME:
		fprintf(file,"\tuucp name = ");
		cp = p_cdname(cp, msg, file);
		fprintf(file,"\n");
		break;
#endif	/* T_UNAME */
#ifdef	T_MP
	case T_MP:
		if (n = *cp++) {
			fprintf(file,"\tmail path = %.*s\n", n, cp);
			cp += n;
		}
		break;

#endif	/* T_MP */
	case T_HINFO:
		if (n = *cp++) {
			fprintf(file,"\tCPU=%.*s\n", n, cp);
			cp += n;
		}
		if (n = *cp++) {
			fprintf(file,"\tOS=%.*s\n", n, cp);
			cp += n;
		}
		break;

	case T_SOA:
		fprintf(file,"\torigin = ");
		cp = p_cdname(cp, msg, file);
		fprintf(file,"\n\tmail addr = ");
		cp = p_cdname(cp, msg, file);
		fprintf(file,"\n\tserial=%ld", _getlong(cp));
		cp += sizeof(u_long);
		fprintf(file,", refresh=%s", p_time(cp));
		cp += sizeof(u_long);
		fprintf(file,", retry=%s", p_time(cp));
		cp += sizeof(u_long);
		fprintf(file,", expire=%s", p_time(cp));
		cp += sizeof(u_long);
		fprintf(file,", min=%s\n", p_time(cp));
		cp += sizeof(u_long);
		break;

	case T_MX:
		fprintf(file,"\tpreference = %ld,",_getshort(cp));
		cp += sizeof(u_short);
		fprintf(file," name = ");
		cp = p_cdname(cp, msg, file);
		break;

	case T_MINFO:
		fprintf(file,"\trequests = ");
		cp = p_cdname(cp, msg, file);
		fprintf(file,"\n\terrors = ");
		cp = p_cdname(cp, msg, file);
		break;

	case T_UINFO:
		fprintf(file,"\t%s\n", cp);
		cp += dlen;
		break;

	case T_UID:
	case T_GID:
		if (dlen == 4) {
			fprintf(file,"\t%ld\n", _getlong(cp));
			cp += sizeof(int);
		}
		break;

	case T_WKS:
		if (dlen < sizeof(u_long) + 1)
			break;
		bcopy(cp, (char *)&inaddr, sizeof(inaddr));
		cp += sizeof(u_long);
		fprintf(file,"\tinternet address = %s, protocol = %d\n\t",
			inet_ntoa(inaddr), *cp++);
		n = 0;
		while (cp < cp1 + dlen) {
			c = *cp++;
			do {
 				if (c & 0200)
					fprintf(file," %d", n);
 				c <<= 1;
			} while (++n & 07);
		}
		putc('\n',file);
		break;

#ifdef ALLOW_T_UNSPEC
	case T_UNSPEC:
		{
			int NumBytes = 8;
			char *DataPtr;
			int i;

			if (dlen < NumBytes) NumBytes = dlen;
			fprintf(file, "\tFirst %d bytes of hex data:",
				NumBytes);
			for (i = 0, DataPtr = cp; i < NumBytes; i++, DataPtr++)
				fprintf(file, " %x", *DataPtr);
			fputs("\n", file);
			cp += dlen;
		}
		break;
#endif /* ALLOW_T_UNSPEC */

	default:
		fprintf(file,"\t???\n");
		cp += dlen;
	}
	if (cp != cp1 + dlen)
		fprintf(file,"packet size error (%#x != %#x)\n", cp, cp1+dlen);
	fprintf(file,"\n");
	return (cp);
#endif
}

static	char nbuf[80];

/*
 * Return a string for the type
 */
char *
p_type(type)
	int type;
{
	switch (type) {
	case T_A:
		return("A");
	case T_NS:		/* authoritative server */
		return("NS");
#ifdef OLDRR
	case T_MD:		/* mail destination */
		return("MD");
	case T_MF:		/* mail forwarder */
		return("MF");
#endif /* OLDRR */
	case T_CNAME:		/* connonical name */
		return("CNAME");
	case T_SOA:		/* start of authority zone */
		return("SOA");
	case T_MB:		/* mailbox domain name */
		return("MB");
	case T_MG:		/* mail group member */
		return("MG");
	case T_MX:		/* mail routing info */
		return("MX");
	case T_MR:		/* mail rename name */
		return("MR");
	case T_NULL:		/* null resource record */
		return("NULL");
	case T_WKS:		/* well known service */
		return("WKS");
	case T_PTR:		/* domain name pointer */
		return("PTR");
	case T_HINFO:		/* host information */
		return("HINFO");
	case T_MINFO:		/* mailbox information */
		return("MINFO");
	case T_AXFR:		/* zone transfer */
		return("AXFR");
	case T_MAILB:		/* mail box */
		return("MAILB");
	case T_MAILA:		/* mail address */
		return("MAILA");
	case T_ANY:		/* matches any type */
		return("ANY");
	case T_UINFO:
		return("UINFO");
	case T_UID:
		return("UID");
	case T_GID:
		return("GID");
#ifdef ALLOW_T_UNSPEC
	case T_UNSPEC:
		return("UNSPEC");
#endif /* ALLOW_T_UNSPEC */
#ifdef	T_UNAME
	case T_UNAME:
		return("UNAME");
#endif	/* T_UNAME */
#ifdef	T_MP
	case T_MP:
		return("MP");
#endif	/* T_MP */
	default:
		(void)sprintf(nbuf, "%d", type);
		return(nbuf);
	}
}

#ifdef XFER
/*
 * Make a copy of a string and return a pointer to it.
 */
static char *
savestr(str)
	char *str;
{
	char *cp;
 
	cp = malloc((unsigned)strlen(str) + 1);
	if (cp == NULL) {
		syslog(LOG_ERR, "savestr: %m");
		exit(2);	/* djw 7/5/88: 1->2 */
	}
	(void) strcpy(cp, str);
	return (cp);
}

char *
p_protocol(num)
    	int num;
{
	static	char number[8];
	struct protoent *pp;
	extern struct protoent *cgetprotobynumber();
 
	pp = cgetprotobynumber(num);
	if(pp == 0)  {
		(void) sprintf(number, "%d", num);
		return(number);
	}
	return(pp->p_name);
}
 
char *
p_service(port, proto)
	u_short port;
	char *proto;
{
	static	char number[8];
	struct servent *ss;
	extern struct servent *cgetservbyport();
 
	ss = cgetservbyport((int)htons(port), proto);
	if(ss == 0)  {
		(void) sprintf(number, "%d", port);
		return(number);
	}
	return(ss->s_name);
}


struct valuelist {
	struct valuelist *next, *prev;
	char	*name;
	char	*proto;
	short	port;
} *servicelist, *protolist;

buildservicelist()
{
	struct servent *sp;
	struct valuelist *slp;

	setservent(1);
	while (sp = getservent()) {
		slp = (struct valuelist *)malloc(sizeof(struct valuelist));
		if (!slp) {	/* djw 7/5/88 */
		    syslog(LOG_ERR, "buildservicelist: %m");		    
		    exit(2);	/* "some other error" */
		}
		slp->name = savestr(sp->s_name);
		slp->proto = savestr(sp->s_proto);
		slp->port = ntohs(sp->s_port);
		slp->next = servicelist;
		slp->prev = NULL;
		if (servicelist) {
			servicelist->prev = slp;
		}
		servicelist = slp;
	}
	endservent();
}

buildprotolist()
{
	struct protoent *pp;
	struct valuelist *slp;

	setprotoent(1);
	while (pp = getprotoent()) {
		slp = (struct valuelist *)malloc(sizeof(struct valuelist));
		if (!slp) {	/* djw 7/5/88 */
		    syslog(LOG_ERR, "buildprotolist: %m");
		    exit(2);	/* "some other error" */
		}
		slp->name = savestr(pp->p_name);
		slp->port = pp->p_proto;
		slp->next = protolist;
		slp->prev = NULL;
		if (protolist) {
			protolist->prev = slp;
		}
		protolist = slp;
	}
	endprotoent();
}

struct servent *
cgetservbyport(port, proto)
	u_short port;
	char *proto;
{
	register struct valuelist **list = &servicelist;
	register struct valuelist *lp = *list;
	static struct servent serv;

	port = htons(port);
	for (; lp != NULL; lp = lp->next) {
		if (port != lp->port)
			continue;
		if (strcasecmp(lp->proto, proto) == 0) {
			if (lp != *list) {
				lp->prev->next = lp->next;
				if (lp->next)
					lp->next->prev = lp->prev;
				(*list)->prev = lp;
				lp->next = *list;
				*list = lp;
			}
			serv.s_name = lp->name;
			serv.s_port = htons(lp->port);
			serv.s_proto = lp->proto;
			return(&serv);
		}
	}
	return(0);
}

struct protoent *
cgetprotobynumber(proto)
    	register int proto;
{

	register struct valuelist **list = &protolist;
	register struct valuelist *lp = *list;
	static struct protoent prot;

	for (; lp != NULL; lp = lp->next)
		if (lp->port == proto) {
			if (lp != *list) {
				lp->prev->next = lp->next;
				if (lp->next)
					lp->next->prev = lp->prev;
				(*list)->prev = lp;
				lp->next = *list;
				*list = lp;
			}
			prot.p_name = lp->name;
			prot.p_proto = lp->port;
			return(&prot);
		}
	return(0);
}

#endif /* XFER */

/*
 * Return a mnemonic for class
 */
char *
p_class(class)
	int class;
{

	switch (class) {
	case C_IN:		/* internet class */
		return("IN");
	case C_ANY:		/* matches any class */
		return("ANY");
	default:
		(void)sprintf(nbuf, "%d", class);
		return(nbuf);
	}
}

/*
 * Return a mnemonic for a time to live
 */
char
*p_time(value)
	u_long value;
{
	int secs, mins, hours;
	register char *p;

	p = nbuf;
	(void)sprintf(p, "%u (", value);

	secs = value % 60;
	value /= 60;
	mins = value % 60;
	value /= 60;
	hours = value % 24;
	value /= 24;

#define	PLURALIZE(x)	x, (x == 1) ? "" : "s"
	while (*++p);
	if (value) {
		(void)sprintf(p, "%d day%s", PLURALIZE(value));
		while (*++p);
	}
	if (hours) {
		if (value && p != nbuf)
			*p++ = ' ';
		(void)sprintf(p, "%d hour%s", PLURALIZE(hours));
		while (*++p);
	}
	if (mins) {
		if ((hours || value) && p != nbuf)
			*p++ = ' ';
		(void)sprintf(p, "%d min%s", PLURALIZE(mins));
		while (*++p);
	}
	if (secs) {
		if ((mins || hours || value) && p != nbuf)
			*p++ = ' ';
		(void)sprintf(p, "%d sec%s", PLURALIZE(secs));
		while (*++p);
	}
	*p++ = ')';
	*p = '\0';
	return(nbuf);
}
