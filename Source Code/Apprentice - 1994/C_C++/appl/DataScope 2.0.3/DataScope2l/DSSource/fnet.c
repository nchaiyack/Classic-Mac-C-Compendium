/*
*
*  This code depends on MacTCP, Apple's TCP/IP driver for the Mac.
*  It uses the NCSA Telnet networking kernel and session services.  As such,
*  it requires a config.tel file in the system folder or the same folder as
*  the application.
*
*/

#include <resources.h>
#include "macfview.h"
#include "DScope.h"

#define CNONE 0
#define CWAIT 1
#define COPEN 2
#define CSIZE 3
#define CREAD 4
#define CSEND 5
#define CDONE 6

struct nets {
int 
	pnum, 					/* port number */
	toread,					/* remaining data to be read */
	constat;				/* status of connection */
	struct fdatawin *td;	/* keeper of window data */
char 
	replace,				/* boolean for replacing dataset */
	image,					/* boolean for generating images */
	interp,
	polar,
	filesave,				/* boolean flag for saving to a file */
	flags[20],				/* flags for what to do with data */
	*whereread;				/* keep track of data reading/writing */
};

static struct nets gin,gout,*gcur;	/* keeping track */
static char buf[128];
char netfailed=0;


#define GETPORT 7799

/**********************************************************************/
/*  donetevents
*   This routine should be called in the main event loop.  It handles
*   background networking tasks like the data server which accepts data
*   from other hosts.  It reads the network event queue to discover what
*   to do this time around.
*/
donetevents()
{
	int code;
	
	if (netfailed)
		return;

	if (gin.constat == CNONE) {
		startlisten();
		return;
	}
	
	code = handlenet();
	
	gcur = &gin;
	if (gin.constat >= COPEN)
		processevents();				/* background data receiver */
}

/**********************************************************************/
/*  processevents
*   Implement the DataScope data server.
*   As soon as the connection opens and data begins to arrive, this
*   procedure is called. Data must fit the required protocol, or the connection
*   is closed automatically.
*
*   Numbers must be in Motorola (network) byte order before they arrive.
*   Floating point numbers must be in IEEE32 bit format before they arrive.
*   ASCII strings (nul terminated) are used for flags and var names.
*
*   Command-period stops any of the network waits used.  Otherwise, there
*   are no internal time-outs.
*
*   The only check for illegal numbers used in the protocol is whether
*   you run out of memory on the array allocate.  Otherwise the data goes
*   where it may.
*/

processevents()
{
	int cnt,i,xd,yd;
	char checktype[5];
	float *flp;
	struct fdatawin *ttd;
	

	if (gcur->constat == COPEN/* && 4 <= netqlen(gcur->pnum)*/) {
		
		while (4 > netqlen (gcur->pnum))
			;
	
		cnt = netread(gcur->pnum,checktype,4);		/* tag type */
		
		if (cnt == 4 && !strncmp(checktype,"DSa1",4)) {/* DS data array */
			gcur->constat = CSIZE;
			readtonul(gcur->pnum,buf,100);			/* read var name */
			readtonul(gcur->pnum,gcur->flags,20);	/* read flags field */
			checkflags(gcur);
		}
	}
	else if (gcur->constat == CSIZE && 16 <= netqlen(gcur->pnum)) {

/*
*  Get particulars about array to be read.
*  Dimensions first.  Must be 2-D arrays.
*/
		cnt = 0;
		cnt += netread(gcur->pnum,&yd,4);		/* array dimensions */
		cnt += netread(gcur->pnum,&xd,4);		/* array dimensions */
		
		if (xd <= 1 || yd <= 1 ||				/* illegal values ? */
			getpointers(gcur,buf,xd,yd)) {		/* decide where to put data */
			gcur->constat = CNONE;
			netclose(gcur->pnum);
			return(-1);
		}
/*
*  Maximum and minimum values of interest are first floats to arrive.
*/
		ttd = gcur->td;
		cnt += netread(gcur->pnum,&gcur->td->valmax,4);
		cnt += netread(gcur->pnum,&gcur->td->valmin,4);
/*
*  read row and column labels all at once.
*/
		if (fullread(gcur->pnum,gcur->td->yvals,4*yd) ||
			fullread(gcur->pnum,gcur->td->xvals,4*xd)) {
			gcur->constat = CNONE;
			netclose(gcur->pnum);
			return(-1);
		}
/*
*  read array in the background, using events to keep us alive.
*  Setting up status bytes for the read here.
*/
		gcur->constat = CREAD;
		gcur->whereread = (char *)gcur->td->vals;
		gcur->toread = xd*yd*4;			/* number of bytes to read from net */
			
				
	}
	
/*
*  Control is returned to us for a background read.
*  Read whatever is in the incoming buffer and return control to event loop.
*/
	else if (gcur->constat == CREAD) {
		cnt = netread(gcur->pnum,gcur->whereread,gcur->toread);
		if (cnt < 0) {
			gcur->constat = CNONE;
			losedat(gcur->td);
			netclose(gcur->pnum);
			return(-1);
		}
		gcur->toread -= cnt;				/* update state counters */
		gcur->whereread += cnt;				/* where data goes */
		
/*
*   If we have all of the data, install it into the window list.
*/
		if (gcur->toread <= 0) {
			gcur->constat = COPEN;
			
			if (gcur->replace && gcur->td->text) {
				setwtitle(gcur->td->text->win,gcur->td->dvar);
				drawfloat(gcur->td->text,0,0);	/* update old window */
			}
			else
				ctextwin(gcur->td);				/* create a new one */

/*
*  If max and min are wrong, re-calculate.
*/
			if (gcur->td->valmax <= gcur->td->valmin) {
				maxmin( gcur->td->vals, gcur->td->xdim, gcur->td->ydim,
						&gcur->td->valmax, &gcur->td->valmin, 0,0, 
						gcur->td->ydim, gcur->td->xdim);
			}
			
/*
*  If image generate is on, generate an image from it.
*/
			if (gcur->image && gcur->td->text)
				makeimage(gcur->td->text);

/*
*  If interp generate is on, generate a smoothed image from it.
*/
			if (gcur->interp && gcur->td->text)
				interpit(gcur->td->text);
/*
*  If polar generate is on, generate a polar image from it.
*/
			if (gcur->polar && gcur->td->text)
				polarit(gcur->td->text);
/*
*  If filesave is on, save a copy to a file.
*/
			if (gcur->filesave && gcur->td->text) {
				short v;
				char s[256];
				getvol(s,&v);
				create(gcur->td->dvar, v, 'NCSf', '_HDF');
				savedf(gcur->td->dvar,gcur->td);
			}
		}
	}
	
	return(0);
		
}

/**********************************************************************/
/*  getpointers
*   If the replace flag is on, see if the variable can be replaced.
*   If not, or if no replace flag, allocate memory for a new window.
*   Installs variable name and fname if allocating new memory.
*/
getpointers(g,name,xd,yd)
	struct nets *g;
	char *name;
	int xd,yd;
{
	struct Mwin *m;
	extern struct Mwin *Mlist;
	int i;
	
	m = NULL;

/*
*  Do filename munging similar to what newdatawin does to put in
*  underscores where needed.
*/
	for (i=strlen(name)-1; i > 0 && name[i] == ' '; i--)	/* trim trailing spaces */
			name[i] = 0;
	name[sizeof(gcur->td->dvar)-1] = 0;		/* trim it to required length */
	
	for (i=0; i< strlen(name); i++)
		if (!isalnum(name[i]))
			name[i] = '_';			/* translate periods and spaces to _ */

/*
*  If replace flag is on and all checks out, don't allocate the memory.
*/
	if (g->replace) {

/*
*  Search for matching variable name and check dimensions.
*/
		m = Mlist;
		
		while (m) {
			if (!replacecmp(m->dat->dvar,name) &&
				m->dat->xdim == xd &&
				m->dat->ydim == yd)	{
				gcur->td = m->dat;					/* set struct pointer */
				strcpy(gcur->td->dvar,name);		/* save new name */
				break;
			}
			m = m->next;
		}
	}

/*
*    don't have it, have to allocate memory
*/
	if (!m) {

		gcur->replace = 0;					/* we cannot do replace */
		
		gcur->td = newdatawin(name,xd,yd);	/* allocates all mem */
		if (NULL == gcur->td) 				/* we failed */
			return(-1);
								
		strcpy(gcur->td->fname,gcur->td->dvar);
	}
	
	return(0);
		
}

/************************************************************************/
/*  replacecmp
*   Ignore upper and lower case, but only check as far as the alphanumerics
*   match.  Allows filenames like xx.001, xx.002, xx.003  etc. to replace
*
*   Only returns 0=match, 1=no match, does not compare greater or less
*   There is a tiny bit of overlap with the | 32 trick, but shouldn't be
*   a problem.  It causes some different symbols to match.
*/
replacecmp(sa,sb)
	char *sa,*sb;
	{

	while (*sa && *sa < 33)		/* don't compare leading spaces */
		sa++;
	while (*sb && *sb < 33)
		sb++;

	while (*sa && *sb && isalnum(*sa) && isalnum(*sb)) {
		if ((*sa != *sb) && ((*sa | 32) != (*sb | 32)))
			return(1);
		sa++;sb++;
	}
	if ((!*sa && !*sb) ||		/* if both at end of string */
		(*sa == *sb))			/* or hit identical non-alnums */
		return(0);
	else
		return(1);
}

/**********************************************************************/
/*  checkflags
*  Taking the string of one-character flags, set booleans in the
*  net structure according to what flags are found.  Set flags to 
*  the default settings otherwise.
*
*/
checkflags(g)
	struct nets *g;
{
	char *p;
	
	p = g->flags;
	
	g->replace = 0;
	g->image = 0;
	g->interp = 0;
	g->filesave = 0;
	g->polar = 0;
	
	while (*p) {				/* search flags string, set booleans */
		if (*p == 'R')
			g->replace = 1;
		if (*p == 'G')			/* most are same as command-keys in DS */
			g->image = 1;
		if (*p == 'S')
			g->filesave = 1;
		if (*p == 'I')
			g->interp = 1;
		if (*p == 'P')
			g->polar = 1;
		p++;
	}
	
}
			

/**********************************************************************/
/*
*  netfunction
*  Used to execute a DataScope notebook function when we don't have the
*  function on the local machine.
*  Send the parameters to a remote machine to be executed.
*  Get the answer back.
*
*  Uses rexecd to submit a password-checked job request.
*/
extern 
	char *exuser,*exmachine,*expass;
extern
	int exconnect;
	
netfunction(name,lft,rgt,answer)
	char			*name;
	scope_array		*lft,*rgt,*answer;
{
	int				itemp,code,i,cnt,ac,ar,startanswer;
	char			checktype[256];
	void			ErrorAlert();
	
	gcur = &gout;
	
	/* get setup info, Mac style */
	
	if (exremote(name) || !exuser || !exmachine) {	/* User canceled dialog */
		answer->kind = DS_ERROR;
		ErrorAlert(GetResource('STR ',1007));
		return(-1);
	}
		
	tryconnect(exmachine,exconnect);

	while (gcur->constat != COPEN) {
		code = handlenet();
		
		/*  if either network rejects it or user quits */
		
		if (gcur->constat == CNONE || calcidle()) {
			answer->kind = DS_ERROR;
			ErrorAlert(GetResource('STR ',1008));
			
			if (gcur->constat != CNONE)
				netclose(gcur->pnum);
			return(-1);
		}
	}
	
	netwrite(gcur->pnum,"0\0",2);					/* write opening */
	netwrite(gcur->pnum,exuser,strlen(exuser)+1);	/* write user name */
	netwrite(gcur->pnum,expass,strlen(expass)+1);	/* and password */
	netwrite(gcur->pnum,"./DS_serve\0",11);			/* the command to run */

/*
*  We now sit and wait for TWO zero characters.
*  The first zero character comes from the rexecd to show that it could
*  launch the shell successfully.
*  The second one comes from the DataScope server which is required to send
*  one zero character to indicate that it is ready to receive a function 
*  and its data.
*  If either the shell or the server returns an error message, this
*  section will determine that there are not two zeros and report the 
*  characters, whatever they may be to the user.  It is assumed to be a
*  human-readable error message.
*/
	cnt = 0;
	memset(checktype, 0, 255);

	do {										/* look for go-ahead signal */
		if (calcidle()) {
			answer->kind = DS_ERROR;
			ErrorAlert(GetResource('STR ',1007));
			netclose(gcur->pnum);
			return(-1);
		}
	
		code = handlenet();
		cnt = netqlen(gcur->pnum);				/* how much data has arrived? */

	} while (gcur->constat == COPEN && cnt >= 0 && cnt < 2);
	
	cnt = netqlen(gcur->pnum);
	netread(gcur->pnum,checktype,cnt);

	if (checktype[0] || checktype[1]) {
		answer->kind = DS_ERROR;
		netclose(gcur->pnum);
		if (!checktype[0])
			checktype[0] = ':';					/* indicate shell was OK */
		ErrorAlert(GetResource('STR ',1004));
		netalert(checktype);					/* tell user */
		return(-1);
	}

/*
*  Start sending over the stuff we need to transfer.
*
*  1. The transfer code 'DSfn'
*  2. The name of the function to be calculated.
*  3. The left parameter.  Could be an array or constant.
*  4. The right parameter.  Could be an array or constant.
*
*  If there is an error at any time, check to see if the other side sent
*  us any type of error message about it.
*/

	if ((4 > netwrite(gcur->pnum,"DSfn",4)) ||				/* transfer code */
		(0 > netwrite(gcur->pnum,name,strlen(name)+1)) || 	/* nul terminated name */	
		netparm(gcur->pnum,lft) ||	/* send one parm */
		netparm(gcur->pnum,rgt)) {	/* send other parm */

		if (0 < (i = netqlen(gcur->pnum)))			/* received any messages? */
			netread(gcur->pnum,checktype,i);
			
		answer->kind = DS_ERROR;
		ErrorAlert(GetResource('STR ',1006));
		netclose(gcur->pnum);
		
		if (i > 0)
			netalert(checktype);					/* tell the user */

		return(-1);
	}

/**************************************************************************/
/*
*  This is the pause while we wait for the other side to do the
*  computation.  It could be awhile.
*/
	startanswer = 0;

	do {							/* look for answer */
		if (calcidle()) {
			answer->kind = DS_ERROR;
			netclose(gcur->pnum);
			ErrorAlert(GetResource('STR ',1007));
			return(-1);
		}
	
		code = handlenet();

		if (code == 4) 				/* got some data back */
			startanswer = 1;
			
		if (startanswer) {			/* keep checking until we are done now */
					
			if (gcur->constat == COPEN && 4 <= netqlen(gcur->pnum)) {
			
				cnt = netread(gcur->pnum,checktype,4);		/* tag type */
				
				if (cnt == 4  && !strncmp(checktype,"DSfn",4))	{			/* DS data array */
					gcur->constat = CSIZE;
				}
				else {						
					if (0 < (i = netqlen(gcur->pnum)))		/* received any messages? */
						netread(gcur->pnum,&checktype[4],i);
					answer->kind = DS_ERROR;
					netclose(gcur->pnum);
					netalert(checktype);					/* tell the user */
					ErrorAlert(GetResource('STR ',1006));
					return(-1);
				}
			}
			else if (gcur->constat == CSIZE && 13 <= netqlen(gcur->pnum)) {
			
				cnt = netread(gcur->pnum,&answer->kind,1);
				cnt = netread(gcur->pnum,&answer->cval,4);
				cnt = netread(gcur->pnum,&ar,4);
				cnt = netread(gcur->pnum,&ac,4);
	/*
	*  receive answering array, if we have one
	*/
				if (answer->kind == DS_ARRAY) {
					answer->kind = DS_ERROR;
					itemp = 1;
					if (answer->ncols == ac &&
						answer->nrows == ar &&
						!fullread(gcur->pnum,answer->rows,4*answer->nrows) &&
						!fullread(gcur->pnum,answer->cols,4*answer->ncols) &&
						!fullread(gcur->pnum,
								  answer->vals,
								  4*answer->nrows*answer->ncols))
					   {answer->kind = DS_ARRAY;
						itemp = 0;
					   }
				}	
				
				gcur->constat = CDONE;
				netclose(gcur->pnum);			/* done with calcs */
				if (itemp == 1)	ErrorAlert(GetResource('STR ',1005));
				return(0);
			}
		}
	
	} while (gcur->constat != CNONE);
	
	answer->kind = DS_ERROR;
	ErrorAlert(GetResource('STR ',1004));
	netclose(gcur->pnum);
	return(-1);
	
}

/**********************************************************************/
/* readtonul
*  read from the stream until reaching a NUL
*
*  limit includes the NULL byte at end, will not overrun, lim==0 is not
*  allowed.
*/
readtonul(skt,p,lim)
    int skt,lim;
    char *p;
    {
    char cc;
    int ret;

	*p = 0;

    do {
		if (calcidle())			/* allow processor time */
			return(-1);
		handlenet();			/* take care of net events */

		while ((ret = netread(skt,&cc,1)) == 0);
		
        if (0 > ret)
            return(-1);
			
	    *p++ = cc;

		if (--lim <= 0)
			p--;				/* keep from overrunning */

    } while (ret && cc);

    return(0);
}

/**********************************************************************/
/*  fullwrite
*   write a full segment to the network.
*   Check for user interrupt while doing so.
*   returns 0 for successful send, -1 for error in sending or user interrupt.
*/
fullwrite(skt,wherewrite,towrite)
	int skt,towrite;
	char *wherewrite;
{
	int cnt;

	while (towrite > 0) {			/* count of remaining bytes to send */
		if (calcidle())				/* check for user interrupt */
			return(-1);

		cnt = netwrite(skt,wherewrite,towrite);		/* send a chunk */
		if (cnt < 0)				/* connection broken */
			return(-1);

		towrite -= cnt;				/* adjust counters for what was sent */
		wherewrite += cnt;
	}

	return(0);
}

/**********************************************************************/
/*  fullread
*   read a full segment from the network.
*   Check for user interrupt while doing so.
*   returns 0 for success, -1 for error in reading or user interrupt.
*/
fullread(skt,whereread,toread)
	int skt,toread;
	char *whereread;
{
	int cnt;

	while (toread > 0) {			/* count of remaining bytes to read */
		if (calcidle())				/* check for user interrupt */
			return(-1);
		handlenet();				/* take care of net events */

		cnt = netread(skt,whereread,toread);		/* read a chunk */
		if (cnt < 0)				/* connection broken */
			return(-1);

		toread -= cnt;				/* adjust counters for what was read */
		whereread += cnt;
	}

	return(0);
}

/**********************************************************************/
/*  netparm
*  Write one of the function parameters to the remote host.
*/
netparm(skt,sar)
	int skt;
	scope_array *sar;
{
	
	while (16 > netroom(skt))		/* room for small stuff? */
		if (calcidle())
			return(-1);
	
	netwrite(skt,&sar->kind,1);		/* type of argument */
	netwrite(skt,&sar->cval,4);		/* constant value, 0.0 if not used */
	netwrite(skt,&sar->nrows,4);	/* number of rows */
	netwrite(skt,&sar->ncols,4);	/* number of columns, 0 if not used? */

	if (sar->kind == DS_ARRAY) {	/* send the array over */
	
		if (fullwrite(skt,(char *)sar->rows, 4*sar->nrows))
			return(-1);
			
		if (fullwrite(skt,(char *)sar->cols, 4*sar->ncols))
			return(-1);

		if (fullwrite(skt,(char *)sar->vals, 4*sar->ncols*sar->nrows))
			return(-1);
	
	}
	
	return(0);
}

/**********************************************************************/
/*
*   fnet.c
*   TCP/IP stubs for the NCSA TCP/IP kernel.
*
*   This version assumes MacTCP support.
*/
#include "whatami.h"
#include "hostform.h"

struct machinfo *mp;
char *neterrstring();

startnet() {
/*
*  assumes MacTCP because we didn't worry about setting IP number.
*/
	if (Snetinit()) {			/* call session initialization */
		errhandle();			/* Snetinit() reads config.tel file */
		netfailed = 1;
		return(-1);
	}
	
	return(0);
}

stopnet()
{
	if (gin.constat > 1)
		netclose(gin.pnum);
		
	netshut();
	
	gin.constat = 0;
}

startlisten()
	{
	
	gin.pnum = netlisten(GETPORT);
	gin.constat = CWAIT;
	gin.toread = 0;
	
	SetCursor(&qd.arrow);
}

tryconnect(m,tcport)
	char *m;
	int tcport;
{

	gcur->constat=CWAIT;		/* attempting connection */
	
	mp = Sgethost(m);			/* look up in hosts cache */
	if (!mp)
		gcur->pnum = Sdomain(m);		/* not in hosts, try domain */
	else {
		if (0 > (gcur->pnum = Snetopen(mp,tcport))) {
			errhandle();
			netshut();
			return(-1);
		}
	}
	
	return(0);
	
}


handlenet()
{
	int i,cnt,ev,what,dat;
	char *errmsg;

/*
*  get event from network, these two classes return all of the events
*  of user interest.
*/
	ev = Sgetevent(USERCLASS | CONCLASS | ERRCLASS,&what,&dat);
	if (!ev)
		return(0);

	if (what == ERRCLASS) {				/* error event */
		errmsg = neterrstring(dat);
		putln(errmsg);
	}
	else if (what == CONCLASS) {		/* event of user interest */
		switch (ev) {
			case CONOPEN:				/* connection opened or closed */
				if (dat == gin.pnum)
					gin.constat = COPEN;
				else if (dat == gout.pnum)
					gout.constat = COPEN;
				break;
			default:
				break;
			case CONDATA:				/* data arrived for me */
				if (dat == gin.pnum)
					return(3);
				else if (dat == gout.pnum)
					return(4);
				else
					return(-1);
				break;
				
			case CONFAIL:
				if (dat == gin.pnum)
					gin.constat = CNONE;
				else if (dat == gout.pnum)
					gout.constat = CNONE;
				netalert("Cannot connect to requested host");
				return(-1);
				
			case CONCLOSE:
				if (0 < netqlen(dat))
					netputuev(CONCLASS,CONCLOSE,dat);	/* repost back to me */
					
				else if (dat == gin.pnum) {
					netclose(gin.pnum);
					startlisten();			/* restart listener */
				}
				else if (dat == gout.pnum)
					gout.constat = CNONE;	/* indicate close */
				return(-1);

		}
	}
	else if (what == USERCLASS) {
		switch (ev) {
			case DOMOK:						/* domain worked */
				mp = Slooknum(dat);			/* get machine info */
				i = Snetopen(mp,exconnect);	/* open to host name */
				if (dat == gin.pnum)
					gin.pnum = i;
				else if (dat == gout.pnum)
					gout.pnum = i;
				break;
			case DOMFAIL:	/* domain failed */
				if (dat == gin.pnum)
					gin.constat = CNONE;
				else if (dat == gout.pnum)
					gout.constat = CNONE;
				netalert("Cannot find address of requested host");
				return(-1);
			default:
				break;
		}
	}

	return(0);

}


/*********************************************************************/
/*  errhandle
*   write error messages to the console window
*/
errhandle()
	{
	char *errmsg;
	int i,j;

	while (ERR1 == Sgetevent(ERRCLASS,&i,&j)) {
		errmsg = neterrstring(j);
		netalert(errmsg);				/* tell user */
	}

}

putln(s)
	char *s;
	{
	
	/* puts(s); */
	
}

quit()
	{ /* exit(2) ; */
		netfailed = 1;
		}
	
isHFS()
	{
	return(1);
	}
	
DisplayMacBinary()
	{  }
	

initipnum()
	{ }
	

int KIP=0;
