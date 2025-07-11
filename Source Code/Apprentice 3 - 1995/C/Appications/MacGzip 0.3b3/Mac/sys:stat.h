/* stat.h - poorman version to POSIX sys/stat.h */

/*
Revision history
----------------
01a,01nov94,ejo  written.
*/

#ifndef INC_sys_stat_h
#define INC_sys_stat_h

#include <time.h>
#include "sys/types.h"

#define S_IFMT		0170000	/* type of file */
#define S_IFIFO		0010000	/* fifo */
#define S_IFDIR		0040000	/* directory */
#define S_IFCHR		0020000	/* character special */
#define S_IFBLK		0060000	/* block special */
#define S_IFREG		0100000	/* regular */

#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)

#define S_ISUID		0004000
#define S_ISGID		0002000

#define S_IRUSR		00400
#define S_IWUSR		00200
#define S_IXUSR		00100
#define S_IRWXU		(S_IRUSR | S_IWUSR | S_IXUSR)

#define S_IRGRP		00040
#define S_IWGRP		00020
#define S_IXGRP		00010
#define S_IRWXG		(S_IRGRP | S_IWGRP | S_IXGRP)

#define S_IROTH		00004
#define S_IWOTH		00002
#define S_IXOTH		00001
#define S_IRWXO		(S_IROTH | S_IWOTH | S_IXOTH)

/* historical file modes */
#define	S_IREAD		00400
#define	S_IWRITE	00200
#define	S_IEXEC		00100

struct stat
	{
	mode_t	st_mode;
	ino_t	st_ino;
	dev_t	st_dev;
	nlink_t	st_nlink;	
	uid_t	st_uid;
	gid_t	st_gid;
	off_t	st_size;
	time_t	st_atime;
	time_t	st_mtime;
	time_t	st_ctime;
	};

#ifdef __cplusplus
extern "C" {
#endif

int		chmod		(const char *path, mode_t mode);                                     
int		fstat		(int fildes, struct stat *buf);
int		mkdir		(const char *path, mode_t mode);
int		mkfifo		(const char *path, mode_t mode);
int		stat		(const char * path, struct stat *buf);
mode_t	umask		(mode_t cmask);

#define chmod(a,b)	(0)
#define mkfifo(a,b)	(-1)
#define umask(a)	(0)

#ifdef __cplusplus
}
#endif

#endif /* INC_sys_stat_h */
