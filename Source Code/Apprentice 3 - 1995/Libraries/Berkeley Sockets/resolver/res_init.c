#include <types.h>
#include <Memory.h>
#include <Resources.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/nameser.h>
#include <resolv.h>

/*
 * Resolver configuration file. In this implementation the information is
 * read from a resource in the MacTCP driver.
 */

#define CONF_RES_TYPE	'dnsl'	/* resource type for name server list */

typedef struct nsentry
{
	long ipaddr;
	Boolean isDefault;
	char name[1];
} nsentry;

typedef struct nslist
{
	short count;
	nsentry ns;
} nslist;

/*
 * Resolver state default settings
 */

struct state _res = 
{
    RES_TIMEOUT,               	/* retransmition time interval */
    4,                         	/* number of times to retransmit */
    RES_DEFAULT,				/* options flags */
    1,                         	/* number of name servers */
};

/*
 * Set up default settings.  If the configuration file exists, the values
 * there will have precedence.  Otherwise, the server address is set to
 * INADDR_ANY and the default domain name comes from the gethostname().
 *
 * The configuration file should only be used if you want to redefine your
 * domain or run without a server on your machine.
 *
 * THE CONFIGURATION FILE IS REQUIRED ON A MAC. THERE IS NO LOCAL NAMESERVER.
 *
 * THE 'CONFIGURATION FILE' IS THE NAMESERVER LIST IN THE APPLE TCP DRIVER.
 *
 * Return 0 if completes successfully, -1 on error
 */
 res_init()
{
	int resRef;
	Handle resH;
	Size size;
	nslist *nsl;
	nsentry *ns;
    register char *cp, **pp;
    extern char *strchr();
    extern char *strcpy(), *strncpy();
    int n;

    bzero((char *)&_res, sizeof _res);
    _res.retrans = RES_TIMEOUT;
    _res.retry = 4;
    _res.options = RES_DEFAULT;
    _res.nsaddr.sin_addr.s_addr = INADDR_ANY;
    _res.nsaddr.sin_family = AF_INET;
    _res.nsaddr.sin_port = htons(NAMESERVER_PORT);
    _res.nscount = 1;
    _res.defdname[0] = '\0';

	resRef = OpenOurRF();
	resH = GetIndResource(CONF_RES_TYPE,1);
	if (resH != NULL && GetHandleSize(resH) >= sizeof(nslist))
	{
		HLock(resH);
		nsl = (nslist *)*resH;
		if (nsl->count > 0)
		{
			_res.nscount = nsl->count;
			if (_res.nscount > MAXNS) 
				_res.nscount = MAXNS;
			ns = &nsl->ns;
			for (n=0; n<_res.nscount; n++) 
			{
				if (ns->isDefault) 
				{
					(void) strncpy(_res.defdname, ns->name, sizeof(_res.defdname));
					_res.defdname[sizeof(_res.defdname) - 1] = '\0';
				} 
				_res.nsaddr_list[n].sin_addr.s_addr = ns->ipaddr;
				_res.nsaddr_list[n].sin_family = AF_INET;
				_res.nsaddr_list[n].sin_port = htons(NAMESERVER_PORT);
				ns = (nsentry *)((long)ns + sizeof(nsentry) + strlen(ns->name));
			}
		}
		HUnlock(resH);
	}
	if (resRef != -1)
		CloseResFile(resRef);
   
    if (_res.defdname[0] == 0) 
	{
        if (gethostname(_res.defdname, sizeof(_res.defdname)) == 0)
		{
			cp = strchr(_res.defdname, '.');
			if (cp != NULL)
				(void) strcpy(_res.defdname, cp+1);
			else
    			_res.defdname[0] = 0;
		}
    }

    /* find components of local domain that might be searched */
    pp = _res.dnsrch;
    *pp++ = _res.defdname;
    for (cp = _res.defdname, n = 0; *cp; cp++)
		if (*cp == '.')
			n++;
    cp = _res.defdname;
    for (; n >= LOCALDOMAINPARTS && pp < _res.dnsrch + MAXDNSRCH; n--) 
	{
		cp = strchr(cp, '.');
		*pp++ = ++cp;
    }
    if (pp < _res.dnsrch + MAXDNSRCH)
		*pp = NULL;
    _res.options |= RES_INIT;
    return(0);
}
