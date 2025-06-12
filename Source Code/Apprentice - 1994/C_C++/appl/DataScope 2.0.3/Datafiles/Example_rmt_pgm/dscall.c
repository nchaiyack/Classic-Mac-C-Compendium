/**************************************************************************/
/*  NCSA DataScope
*   An experiment with real numbers.
*   by Tim Krauskopf
*   
*   National Center for Supercomputing Applications
*   University of Illinois at Urbana-Champaign
*   605 E. Springfield Ave.
*   Champaign, IL  61820
*
*   email:  		softdev@ncsa.uiuc.edu
*   bug reports:	bugs@ncsa.uiuc.edu
*   server:         ftp.ncsa.uiuc.edu  (128.174.20.50)
*
*   NCSA DataScope is in the public domain.  See the manual for a complete
*   permissions statement.  We ask that the following message be included
*   in all derivative works:
*   Portions developed at the National Center for Supercomputing Applications
*   University of Illinois at Urbana-Champaign.
*
*   version                      comments
*   -----------                -------------
*   1.0 TKK  December 1988
*   1.1 TKK  May 1989       -- new polar, computations, interpolation
*   1.2 TKK  January 1990   -- networking additions
*/
/*
*
*  dscall.c
*  
*  Code to use Berkeley style sockets to connect to DataScope 1.2 on
*  a Macintosh.  Includes complete source for DS_SEND for FORTRAN and C
*  written for UNIX and UNICOS systems.
*
*  To compile and link on UNICOS, using your code in myprog.c:
*
*  cc -DUNICOS dscall.c
*  cc myprog.c dscall.o -lnet
*
*  On Sun UNIX, you don't need the -lnet and use DSUN instead of DUNICOS.
*  For more information see the documentation for NCSA DataScope 1.2.
*
*
*  UNSUPPORTED:
*  HyperCard server.
*  This server sits on a unix machine with Berkeley sockets and waits 
*  for a hypercard client which is acting as a front-end.  The alternate
*  version can be used with rexecd for socket-less connection from HC
*  to the Mac.  These are unsupported and should be used as examples for
*  original work only.
*  
*  October, 1989
*/



#include <stdio.h>

#ifdef UNICOS
#include <fortran.h>

#define SOCKETS
#define INTOFF 4		/* distance from beginning of int to 32 bit half */
#define FLOFF 4			/* offset for floating point length diff */
#define FLEN 8			/* floating point number length, for readability */

#endif


#ifdef SUN
#define SOCKETS
#define INTOFF 0
#define FLOFF 0
#define FLEN 4

#include <sgtty.h>
#endif SUN

#ifdef SOCKETS

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#endif

int 
	sendport=7799,
	slport=7888;

#ifdef SOCKETS 
struct sockaddr_in myaddr,hisaddr;
struct hostent *me,*him;
char myname[32],hisname[32];
int sock;
#endif

char 
	*malloc(),
	*xspace,
	*yspace,			/* storage for scale info */
	*slspace,			/* intermediate storage */
	*rspace;


/**********************************************************************/
/*  fullread
*   read a full segment from the network.
*   returns 0 for successful read, -1 for error 
*/
fullread(skt,whereread,toread)
	int skt,toread;
	char *whereread;
{
	int cnt;

	while (toread > 0) {			/* count of remaining bytes to read */

		cnt = read(skt,whereread,toread);		/* read a chunk */
		if (cnt < 0)				/* connection broken */
			return(-1);

		toread -= cnt;				/* adjust counters for what was read */
		whereread += cnt;
	}

	return(0);
}


/**********************************************************************/
/*  fullwrite
*   write a full segment to the network.
*   returns 0 for successful write, -1 for error
*/
fullwrite(skt,wherewrite,towrite)
    int skt,towrite;
    char *wherewrite;
{
    int cnt;

    while (towrite > 0) {            /* count of remaining bytes to read */

        cnt = write(skt,wherewrite,towrite);       /* write a chunk */
        if (cnt < 0)                /* connection broken */
            return(-1);

        towrite -= cnt;              /* adjust counters for what was written */
        wherewrite += cnt;
    }

    return(0);
}

/**********************************************************************/
/* readto13
*  read from the stream until reaching a NUL
*/
readto13(skt,p)
	int skt;
	char *p;
	{
	char cc;
	int ret;

	do {
		if (0 >= (ret = read(skt,&cc,1)))
			return(-1);
		*p++ = cc;
	} while (ret && cc != 13);

	*p++ = '\0';

	return(0);
}

/**********************************************************************/
/* readtonul
*  read from the stream until reaching a NUL
*/
readtonul(skt,p)
	int skt;
	char *p;
	{
	char cc;
	int ret;

	do {
		if (0 >= (ret = read(skt,&cc,1)))
			return(-1);
		*p++ = cc;
	} while (ret && cc);

	return(0);
}

/**********************************************************************/


#ifdef SOCKETS
/***************************************************************************/
/*  commclose
*  close the communications connection
*/
commclose(skt)
	int skt;
	{
	return(close(skt));

}

/***************************************************************************/
/*  comminit
*  Start communications with another host.  Goes looking for a server who
*  is waiting on the port number.
*  Returns an integer which will identify the connection.
*  Returns -1 on error.
*/
comminit(towho,slport)
	char *towho;
	int slport;
	{
	int a,b,c,d;
	unsigned char *p;

	bzero( (char *)&hisaddr, sizeof(hisaddr));

	hisaddr.sin_family = AF_INET;
	hisaddr.sin_port = htons(slport);

	if (4 == sscanf(towho,"%d.%d.%d.%d",&a,&b,&c,&d)) {
		p = (unsigned char *)&hisaddr.sin_addr;
        *p++ = a;
        *p++ = b;
        *p++ = c;
        *p = d;
	}
	else if (NULL == (him = gethostbyname(towho))) {
		perror("*** His host name");
		return(-1);
	}
	else
		/* copy in my host address */
		bcopy(him->h_addr, (char *)&hisaddr.sin_addr, him->h_length);

/*
*  Create the socket and bind it
*/
	if (0 > (sock = socket(AF_INET, SOCK_STREAM, 0))) {
		perror("*** Making socket");
		return(-1);
	}

	if (0 > connect(sock, &hisaddr, sizeof(hisaddr))) {
		perror("*** Connect");
		return(-1);
	}

	puts("=== Connect.");

	return(sock);
	
}

/**************************************************************************/
/*  commlisten
*   Wait for a connection attempt from the client
*/
commlisten(theport)
	int theport;
	{
	int ret,len;

/*
*  Who am I? 
*/
	gethostname(myname,32);

	if (NULL == (me = gethostbyname(myname))) {
		perror("*** My host name");
		return(-1);
	}

	bzero( (char *)&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(theport);
	bcopy(me->h_addr, (char *)&myaddr.sin_addr, me->h_length);
/*
*  Create the socket and bind it
*/
	if (0 > (sock = socket(AF_INET, SOCK_STREAM, 0))) {
		perror("*** Making socket");
		return(-1);
	}

	if (0 > bind(sock, &myaddr, sizeof(myaddr))) {
		perror("*** Binding socket");
		return(-1);
	}

	listen(sock,2);

	puts("=== Listening . . .");

	return(0);
}

commaccept(sock)
{
	int ret,len;
/*
*  wait for connection
*/
	len = 32;
	if ( 0 > (ret = accept(sock, hisname, &len))) {
		perror("*** Accept");
		return(-1);
	}

	puts("=== Connection.");

	return(ret);

}

#endif

#ifdef SUN
flconvert()
	{

}

flbackconvert()
	{

}
#endif


#ifdef UNICOS


/*
*  Convert floats from 4 bytes IEEE-32 to/from Cray-64 8 bytes.
*  Also responsible for unpacking and packing the 4 byte numbers.
*
*  These routines are not responsible for space allocation whatsoever.
*  They are assured that the space given is 8-bytes per float for as
*  many floats are given.  When extra space is generated in backconvert,
*  it lets the calling routine still take care of it.
*/


#define MINEXP    0x3f81000000000000  /* min valid Cray masked exponent */
#define MAXEXP    0x407e000000000000  /* max valid Cray masked exponent */

#define C_FMASK   0x00007fffff000000  /* Cray fraction mask (1st 23 bits)*/
#define C_EMASK   0x7fff000000000000  /* Cray exponent mask */
#define C_SMASK   0x8000000000000000  /* Cray sign mask */
#define C_IMPLICIT 0x0000800000000000 /* Cray implicit bit */

#define I_FMASK   0x007fffff          /* IEEE fraction mask */
#define I_EMASK   0x7f800000          /* IEEE exponent mask */
#define I_SMASK   0x80000000          /* IEEE sign mask     */

#define IEEE_BIAS 0177
#define CRAY_BIAS 040000

static long C2I_diff;
static long I2C_diff;



flbackconvert(farr,nf)
	char *farr;
	int nf;
	{
	int i;
	long tmp,newnum;
	char *to,*p;

	to = farr;			/* counts 4 byte IEEE numbers */

	for (i=0; i< nf; i++) {
		bcopy(farr, &newnum, 8);
		farr += 8;

		if (!newnum)
			tmp = 0;
		else {
			 
			tmp = (C_EMASK & newnum);
			if (tmp < MINEXP) {
				newnum = 1e-30;			/* should be -INF */
				tmp = (C_EMASK & newnum);
			}
			else if (tmp > MAXEXP) {
				newnum = 1e30;			/* should be +INF */
				tmp = (C_EMASK & newnum);
			}

			C2I_diff = (IEEE_BIAS - CRAY_BIAS - 1) << 48;
			tmp = (( tmp + C2I_diff ) << 7)
				| ( (newnum & C_FMASK) << 8 )
				| ( (newnum & C_SMASK));

		}

		bcopy(&tmp,to,4);
		to += 4;

	}

}


 
/* Conversion from IEEE floating point format to Cray format */
 
flconvert(farr,nf)
	char *farr;
	int nf;
	{
	int i;
	long tmp,targ;
	char *from,*to;

	from = farr + 4*(nf-1);		/* end of IEEE array, work backwards */
	to = farr + 8*(nf-1);		/* end of Cray array, work backwards */

	for (i=0; i<nf; i++) {		/* for each float */
		tmp = 0;
		bcopy(from, FLOFF+(char *)&tmp, 4); 
		from -= 4;

		if (!(targ = (tmp & I_EMASK))) {
			targ = 0;
		}
		else {
			I2C_diff = (CRAY_BIAS - IEEE_BIAS + 1) << 23;
			targ += I2C_diff;
			targ = (targ<< 25)  | ( (tmp & I_FMASK) << 24)
			   | ( (tmp & I_SMASK) << 32) | C_IMPLICIT;

		}
		bcopy(&targ, to, 8);
		to -= 8;				/* room for next one */

	}

}



#endif

/************************************************************************/
/* DSDELIVER
*  uses a FORTRAN calling interface and delivers a dataset to the
*  new DataScope which receives datasets from the network.
*
*  The var name must be declared as CHARACTER*80
*
*/

static int sendcd;

#ifdef UNICOS
DS_SEND1(name,flags,maxp,minp,nrowsp,ncolsp,rows,cols,vals)
#else
ds_send1_(name,flags,maxp,minp,nrowsp,ncolsp,rows,cols,vals)
#endif
	char *name,*flags;
	int *nrowsp,*ncolsp;
	float *maxp,*minp,*vals,*rows,*cols;
{

	int i,xd,yd;
	float max,min;
	char newname[100],newflags[100];

/*
*  Character string conversions for UNICOS
*
*  Currently, these are macros which return the converted pointer
*  from the original.
*/
#ifdef UNICOS

	xd = _fcdlen(name);
	name = _fcdtocp(name);
	for (i=0; i<xd; i++)			/* copy it over */
		newname[i] = *name++;
	newname[i] = 0;
	name = newname;

	xd = _fcdlen(flags);
	flags = _fcdtocp(flags);
	for (i=0; i<xd; i++)			/* copy it over */
		newflags[i] = *flags++;
	newflags[i] = 0;
	flags = newflags;

#endif

/*
*  do the character conversions from FORTRAN to C for non-UNICOS
*  This trimming operation is required for converted UNICOS strings also.
*/
	for (i=0; i<80; i++)
		if (!name[i])
			break;
	i--;

	for ( /* i is already set */ ; name[i] == ' '; i--)
		;

	name[++i] = '\0';		/* trim FORTRAN string */

	for (i=0; flags[i] > ' '; i++)
		;

	flags[i] = '\0';

/*
*  call the C version to actually send the formatted data.
*/
	return( 
		ds_send1(name,flags,*maxp,*minp,*nrowsp,*ncolsp,rows,cols,vals) );

}

/************************************************************************/
/* DS_SEND
*  uses a FORTRAN calling interface and delivers a dataset to the
*  new DataScope which receives datasets from the network.
*
*
*/

#ifdef UNICOS
DS_SEND(host,name,flags,maxp,minp,nrowsp,ncolsp,rows,cols,vals)
#else
ds_send_(host,name,flags,maxp,minp,nrowsp,ncolsp,rows,cols,vals)
#endif
	char *name,*flags,*host;
	int *nrowsp,*ncolsp;
	float *maxp,*minp,*vals,*rows,*cols;
{

#ifdef UNICOS
	DS_OPEN(host);
	
	DS_SEND1(name,flags,maxp,minp,nrowsp,ncolsp,rows,cols,vals);
	
	DS_CLOSE();
#else
	ds_open_(host);
	
	ds_send1_(name,flags,maxp,minp,nrowsp,ncolsp,rows,cols,vals);
	
	ds_close_();

#endif
}

/************************************************************************/
/* Open and close for DS_SND1
*
*/

#ifdef UNICOS
DS_OPEN(host)
#else
ds_open_(host)
#endif
	char *host;
{
	int i,xd;
	char newhost[100];
/*
*  Character string conversions for UNICOS
*
*  Currently, these are macros which return the converted pointer
*  from the original.
*/
#ifdef UNICOS

	xd = _fcdlen(host);
	host = _fcdtocp(host);
	for (i=0; i<xd; i++)			/* copy it over */
		newhost[i] = *host++;
	newhost[i] = 0;
	host = newhost;

#endif

	for (i=0; host[i] > ' '; i++)		/* trim hostname string */
		;

	host[i] = '\0';


	
	if (0 > (sendcd = comminit(host,sendport))) {
		puts("*** Cannot open network connection ");
		return(-1);
	}

	return(0);

}

#ifdef UNICOS
DS_CLOSE()
#else
ds_close_()
#endif
{
	commclose(sendcd);

}

/************************************************************************/
/*  C versions of ds_send, etc.
*
*
*/
ds_open(host)
	char *host;
{
	
	if (0 > (sendcd = comminit(host,sendport))) {
		puts("*** Cannot open network connection ");
		return(-1);
	}

	return(0);
}

ds_close()
{
	commclose(sendcd);
}

ds_send(host,name,flags,max,min,nrows,ncols,rows,cols,vals)
	char *name,*flags,*host;
	int nrows,ncols;
	float max,min,*vals,*rows,*cols;
{

	ds_open(host);
	
	ds_send1(name,flags,max,min,nrows,ncols,rows,cols,vals);
	
	ds_close();
}

/************************************************************************/
/*  ds_send1
*
*   This routine implements the actual protocol which sends the data
*   to the Macintosh.
*   The protocol is only documented in this source code.
*   The first four characters sent over the line indicate that the rest
*   of the data will be in fixed DataScope format.
*/

ds_send1(name,flags,max,min,nrows,ncols,rows,cols,vals)
	char *name,*flags;
	int nrows,ncols;
	float max,min,*vals,*rows,*cols;
{

	int i,xd,yd;

/*
*  copy the data so that we don't destroy it by converting it to IEEE format.
*/
	yd = nrows;
	xd = ncols;

	if (NULL == (slspace = malloc(xd*yd*sizeof(float) + 1000))) {
		perror("*** Memory alloc");
		return(1);
	}
	bcopy(vals,slspace,xd*yd*sizeof(float));

	if (NULL == (xspace = malloc(xd*sizeof(float) + 1000))) {
		perror("*** Memory alloc");
		return(1);
	}
	bcopy(cols,xspace,xd*sizeof(float));

	if (NULL == (yspace = malloc(yd*sizeof(float) + 1000))) {
		perror("*** Memory alloc");
		return(1);
	}
	bcopy(rows,yspace,yd*sizeof(float));

/*
*  If on a foreign architecture from DataScope, convert the
*  floating point and integers to IEEE and Motorola formats.
*
*  Then send them to DataScope in order.
*
*  The first four characters are a tag to mark what follows.
*/
	fullwrite(sendcd,"DSa1",4);
	fullwrite(sendcd,name,strlen(name)+1);			/* nul-terminated var name */
	fullwrite(sendcd,flags,strlen(flags)+1);		/* flags field */

	fullwrite(sendcd,INTOFF + (char *)&yd,4);
	fullwrite(sendcd,INTOFF + (char *)&xd,4);
	flbackconvert(&max,1);
	flbackconvert(&min,1);
	fullwrite(sendcd,&max,4);
	fullwrite(sendcd,&min,4);

	flbackconvert(yspace,yd);
	fullwrite(sendcd,yspace,yd*4);
	
	flbackconvert(xspace,xd);
	fullwrite(sendcd,xspace,xd*4);
	
	flbackconvert(slspace,xd*yd);
	fullwrite(sendcd,slspace,xd*yd*4);

	free(slspace);
	free(xspace);
	free(yspace);

	return(0);
}


#ifdef HC_SOCKETS
/************************************************************************/
/* Hypercard interface callable from FORTRAN
*  uses a FORTRAN calling interface and checks for HyperCard generated
*  input on the connection stream.
*
*
*/

static int cd;

#ifdef UNICOS
HC_INIT(port)
#else
hc_init_(port)
#endif
int *port;
{

	if (0 > commlisten(*port)) {
		puts("*** Cannot open network connection ");
		return(-1);
	}
	
	return(0);
}

#ifdef UNICOS
HC_DONE()
#else
hc_done_()
#endif
{

return(commclose(cd));
}

#ifdef UNICOS
HC_WAIT()
#else
hc_wait_()
#endif
{
	if (0 < (cd = commaccept(sock)))
		return(0);
	else
		return(-1);
		
}

/*************************************************************************/
/*  hc_get
*  Get a string from HyperCard front-end.
*  waitflag = 0 means do not wait for HC, return -1
*  waitflag = 1 means wait for input from HC
*
*/
#ifdef UNICOS
HC_GET(waitflag,stuff)
	_fcd stuff;
#else
hc_get_(waitflag,stuff)
	char *stuff;
#endif
	int *waitflag;
	
{
	int stufflen,i;
	char *mystuff;


/*
*  Character string conversions for UNICOS
*
*  Currently, these are macros which return the converted pointer
*  from the original.
*/
#ifdef UNICOS

	stufflen = _fcdlen(stuff);
	mystuff = _fcdtocp(stuff);
#else
	stufflen = 256;
	mystuff = stuff;
#endif


	if (!(*waitflag)) {			/* use select to check on input */
		struct timeval timeout;
		int mask;
		
		mask = 1<<cd;			/* select on socket cd */
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		i = select(16,&mask,0,0,&timeout);
		
		if (i <= 0)				/* nothing to read */
			return(-1);
	}	
/*
*  read from HyperCard
*/

	if (0 > readto13(cd,mystuff))
		return(-2);
		
	for (i=strlen(mystuff)-1; i<stufflen; i++)	/* pad for FORTRAN with ' ' */
		mystuff[i] = ' ';
		

	return(0);
}

/*************************************************************************/
/*  hc_put
*  Send a string to HyperCard front-end.
*/
#ifdef UNICOS
HC_PUT(stuff)
	_fcd stuff;
#else
hc_put_(stuff)
	char *stuff;
#endif
	
{
	int stufflen,i;
	char *mystuff;


/*
*  Character string conversions for UNICOS
*
*  Currently, these are macros which return the converted pointer
*  from the original.
*/
#ifdef UNICOS

	stufflen = _fcdlen(stuff);
	mystuff = _fcdtocp(stuff);
#else
	stufflen = 256;
	mystuff = stuff;
#endif

/*
*  do the character conversions from FORTRAN to C for non-UNICOS
*  This trimming operation is required for converted UNICOS strings also.
*/
	for (i=0; i<stufflen; i++)
		if (!mystuff[i])
			break;
	i--;

	for ( /* i is already set */ ; mystuff[i] == ' '; i--)
		;

	mystuff[++i] = '\0';		/* trim FORTRAN string */


	if (0 > fullwrite(cd,mystuff,strlen(mystuff)))
		return(-2);		

	return(0);
}

#else

/*************************************************************************/
/*  Hypercard input/output
*   for a program which is exec'd from the Mac.
*
*   reads and writes stdin/stdout
*
*   All init and open/close routines are NIL
*/

#define HCread 0
#define HCwrite 1

#ifdef UNICOS
HC_INIT(port)
#else
hc_init_(port)
#endif
int *port;
{
	return(0);
}

#ifdef UNICOS
HC_DONE()
#else
hc_done_()
#endif
{
	return(0);
}

#ifdef UNICOS
HC_WAIT()
#else
hc_wait_()
#endif
{
		return(0);		
}

/*************************************************************************/
/*  hc_get
*  Get a string from HyperCard front-end.
*  waitflag = 0 means do not wait for HC, return -1
*  waitflag = 1 means wait for input from HC
*
*/
#ifdef UNICOS
HC_GET(waitflag,stuff)
	_fcd stuff;
#else
hc_get_(waitflag,stuff)
	char *stuff;
#endif
	int *waitflag;
	
{
	int stufflen,i;
	char *mystuff;


/*
*  Character string conversions for UNICOS
*
*  Currently, these are macros which return the converted pointer
*  from the original.
*/
#ifdef UNICOS

	stufflen = _fcdlen(stuff);
	mystuff = _fcdtocp(stuff);
#else
	stufflen = 256;
	mystuff = stuff;
#endif


	if (!(*waitflag)) {			/* use select to check on input */
		struct timeval timeout;
		int mask;
		
		mask = 1<<HCread;			/* select on socket HCread */
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		i = select(16,&mask,0,0,&timeout);
		
		if (i <= 0)				/* nothing to read */
			return(-1);
	}	
/*
*  read from HyperCard
*/

	if (0 > readto13(HCread,mystuff))
		return(-2);
		
	for (i=strlen(mystuff)-1; i<stufflen; i++)	/* pad for FORTRAN with ' ' */
		mystuff[i] = ' ';
		

	return(0);
}

/*************************************************************************/
/*  hc_put
*  Send a string to HyperCard front-end.
*/
#ifdef UNICOS
HC_PUT(stuff)
	_fcd stuff;
#else
hc_put_(stuff)
	char *stuff;
#endif
	
{
	int stufflen,i;
	char *mystuff;


/*
*  Character string conversions for UNICOS
*
*  Currently, these are macros which return the converted pointer
*  from the original.
*/
#ifdef UNICOS

	stufflen = _fcdlen(stuff);
	mystuff = _fcdtocp(stuff);
#else
	stufflen = 256;
	mystuff = stuff;
#endif

/*
*  do the character conversions from FORTRAN to C for non-UNICOS
*  This trimming operation is required for converted UNICOS strings also.
*/
	for (i=0; i<stufflen; i++)
		if (!mystuff[i])
			break;
	i--;

	for ( /* i is already set */ ; mystuff[i] == ' '; i--)
		;

	mystuff[++i] = '\r';		/* carriage return for Mac display */
	mystuff[++i] = '\0';		/* trim FORTRAN string */


	if (0 > fullwrite(HCwrite,mystuff,strlen(mystuff)))
		return(-2);	

	return(0);
}

#endif
