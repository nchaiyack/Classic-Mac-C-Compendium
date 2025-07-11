/*
 * ip.h
 *
 * ANSI Header file for ip.c.
 *
 */

#ifndef __IP_HEADER__
#define __IP_HEADER__

#include "iptime.h"

/* #defines */

#define kNumSockets 32		/* PLEASE GOD DON'T CHANGE THIS NUMBER!! */

#define AF_INET			2
#define IPPROTO_TCP		6
#define IPPROTO_UDP		17
#define IPPORT_RESERVED	1024

#define kHeaderSize 20		/* 8 for UDP + ?? for IP */

/* types */

struct mac_socket {
	StreamPtr		stream;
	unsigned long	remoteHost;
	unsigned short	remotePort;
	unsigned long	localHost;
	unsigned short	localPort;
	unsigned short	mtu;				/* for UDP */
	/* flags - in future, pack bits! */
	char			type;				/* IPPROTO_TDP or IPPROTO_UDP */
	char			connected;
	char			hasData;
	char			bound;				/* boolean */
};
typedef struct mac_socket mac_socket;

#define sockaddr sockaddr_in
struct  sockaddr_in {
	unsigned short		sin_family;	/* type of connection - unused! */
	unsigned short		sin_port;	/* connection port */
	unsigned long		sin_addr;	/* host IP number */
	char				sin_zero[8];
};


/*
 *Function Headers
 */
 
typedef OSErr (*Spin)(void);

/* Mac-Specific */
OSErr InitMacTCP(void);
void DisposeMacTCP(void);
void SetSpin(Spin spinRoutine);

/* DNS */
void num2dot(unsigned long ip, char *dot);
OSErr ConvertStrToAddr(char *name, unsigned long *ipNum);
unsigned long GetHostByName(char *name);
int GetProtoByName(char *name);
int GetHostName(char *name, int namelen);
int GetHostNameOnly(char *name);
int GetMyIPDot(char *num);
unsigned long GetMyIPNum(void);
int getsockname (int sock, struct sockaddr_in *localaddr, int *addrlen);


#define gethostname GetHostName
#define gethostbyname GetHostByName
#define getprotobyname GetProtoByName

/* Generic */
int socket(int family, int type, int protocol);
int connect(int sock, struct sockaddr_in *raddr, int alen);
int write (int sock, Ptr data, int len);
int read (int sock, Ptr buf, int len);
int close(int sock);

/* TCP Only*/
int bind (int sock, struct sockaddr_in *name, int alen);
int listen(int socket, int queuelen);
int accept (int sock, struct sockaddr_in *sin, int *alen);
int select (int nfds, unsigned long *readfs, struct timeval *timeout);
int old_select (int nfds, unsigned long *readfs);

/* UDP Only */
int recvfrom (int sock, char *buf, int len, int flags, struct sockaddr_in *sin, int *alen);
int sendto (int sock, char *data, int len, int flags, struct sockaddr_in *sin, int alen);


#endif /* __IP_HEADER__ */