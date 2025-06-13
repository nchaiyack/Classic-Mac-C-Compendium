long			s_socket(long domain, long type, long protocol);
long			s_bind(long s, struct sockaddr_in *name, long namelen);
long			s_connect(long s, struct sockaddr_in *addr, long addrlen);
long			s_listen(long s, long qlen);
long			s_accept(long s, struct sockaddr_in *addr, long *addrlen);
long			s_close(long s);
long			s_read(long s, char *buffer, long buflen);
long			s_recv(long s, char *buffer, long buflen, long flags);
long			s_recvfrom (long s, char *buffer, long buflen, long flags, struct sockaddr_in *from, long *fromlen);
long			s_write(long s, char *buffer, long buflen);
long			s_writev(long s, struct iovec *iov, long count);
long			s_send(long s, char *buffer, long buflen, long flags);
long			s_sendto(long s, char *buffer, long buflen, long flags, struct sockaddr_in *to, long tolen);
long			s_really_send(long s, char *buffer, long count, Boolean vector, long flags, struct sockaddr_in *to);
long			s_select(long width, unsigned long *readfds, unsigned long *writefds, unsigned long *exceptfds, struct timeval *timeout);
long			s_getdtablesize(void);
long			s_getsockname(long s, struct sockaddr_in *name, long *namelen);
long			s_getpeername(long s, struct sockaddr_in *name, long *namelen);
long			s_shutdown(long s, long how);
long			s_fcntl(long s, unsigned long cmd, long arg);
long			s_dup(long s);
long			s_dup2(long s, long s1);
long			s_ioctl(long d, long request, Ptr argp);
long			s_setsockopt(long s, long level, long optname, char *optval, long optlen);
long			s_spinroutine(ProcPtr routine);
struct hostent	*gethostbyname(char *name);
struct hostent	*gethostbyaddr(ip_addr *addrP, long len, long type);

char 			*inet_ntoa(ip_addr inaddr);
ip_addr			inet_addr(char *address);
long			gethostname(char *machname, long buflen);
struct servent	*getservbyname (char *name, char *proto);








