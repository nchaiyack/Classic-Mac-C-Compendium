/* unistd.h - poorman subset of POSIX unistd.h */

#ifndef INC_unistd_h
#define INC_unistd_h

/*
modificiation history
---------------------
01b,03nov94,ejo  introduced :sys:types.h, corrected protos.
01a,12aug94,ejo  written.
 */

#include "sys/types.h"

#ifndef __STDIO__
#define SEEK_CUR	1
#define SEEK_END	2
#define SEEK_SET	0
#endif

/* Flags used by access () */
#define F_OK	0x0	/* test if file exists */
#define X_OK	0x1	/* test if file is executable */
#define W_OK	0x2	/* test if file is writable */
#define R_OK	0x4	/* test if file is readable */

int		access	(const char *path, int amode);
int		chown	(const char *path, uid_t owner, gid_t group);
int		close	(int fildes);
int		dup		(int fildes);
int		dup2	(int fildes, int fildes2);
char *	getcwd	(char *buf, size_t size);
gid_t	getgid 	(void);
uid_t	getuid 	(void);
int		isatty	(int fildes);
off_t	lseek	(int fildes, off_t offset, int whence);
int		read	(int fildes, void *buf, unsigned int nbyte);
int		rmdir	(const char *path);
int		unlink	(const char *path);
int		write	(int fildes, const void *buf, unsigned int nbyte);

#endif /* INC_unistd_h */
