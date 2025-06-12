/* $Id: $ */

/*
 * This file contains a number of dummy functions,
 * which do nothing but report success or failure (as appropriate).
 */
 
#pragma once

#include "ThinkCPosix.h"

char *mytty = "MacConsole";

unsigned alarm(unsigned secs)
{
	return 0;
}

int chmod(char *path, mode_t mode)
{
	 
	return 0;	/* success! */
}

int chown(char *name, int owner, int grp)
{
	return 0;	/* success! */
}

#define L_ctermid 16

char *ctermid(char *namespace)
{
	if (namespace) {
		strcpy(namespace, mytty);
		return namespace;
	}
	return mytty;
}

#define L_cuserid 16

char *cuserid(char *username){
	if (username) {
		strcpy(username, getlogin());
		return username;
	}
	return getlogin();
}

int fcntl(int fd, int cmd, int name, ...)
{
	errno = ENOSYS;
	return -1;	
}

int ioctl(int fd, int func, ...)
{
	errno = ENOSYS;
	return -1;	
}

int fork(void)
{
	errno = ENOSYS;
	return -1;	
}

int link(char *from, char *to)
{
	errno = ENOSYS;
	return -1;	
}

#define MYDIR "Macintosh HD"
#define FINDER "Finder 7.0.1"

struct passwd mypasswd =
{"", 0, 0, MYDIR, FINDER, ""};

static struct passwd *getpasswd(void)
{
	mypasswd.pw_name = getlogin();
	mypasswd.pw_uid = __uid;
	mypasswd.pw_gid = __gid;
	return &mypasswd;
}

struct passwd *getpwent(void)
{
	return getpasswd();
}

struct passwd *getpwnam(char* name)
{
	return getpasswd();
}

struct passwd *getpwuid(uid_t uid)
{
	return getpasswd();
}

struct group *getgrent(void);
struct group *getgrname(char*);
struct group *getgrgid(gid_t);

int link(char*, char*);

int mkfifo(char *name, int mode)
{
	errno = ENOSYS;
	return -1;	
}

int pause(void)
{
	errno = ENOSYS;
	return -1;	
}

int pipe(int fd[2])
{
	errno = ENOSYS;
	return -1;	
}

FILE *
popen(char *command, char *t)
{
	return NULL;
}

int
pclose(FILE *f)
{
	return EOF;
}

/* clock_t times(struct tms*); word "times" reserved in Think C */

char* ttyname(int fd)
{
	return mytty;
}

mode_t umask(mode_t mask)
{
	return 0;	
}


int wait(int* status)
{
	while (1);
}


