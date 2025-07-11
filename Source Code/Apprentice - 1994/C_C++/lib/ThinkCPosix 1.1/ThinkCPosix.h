/* $Id: $ */

/*
 * The aim of this project is to create a library ("Posix")
 * including as many Posix functions as possible
 * not included in the Think C ANSI and Unix libraries.
 *
 * The code has been collected from various sources.
 * The following is a partial list of acknowledgements:
 * mkdir(), opendir(), readdir(), closedir(), stat() --
 *   Guido van Rossum, CWI, Amsterdam (July 1987)
 *
 * All code here is placed in the public domain
 * Timothy Murphy School of Mathematics, Trinity College Dublin
 * (tim@maths.tcd.ie)
 */
 
#pragma once

#define const

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <unix.h>
#include <ansi_private.h>
#include "sys/dir.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "sys/times.h"
#include "dirent.h"
#include "pwd.h"
#include "grp.h"
#include "unistd.h"
#include "utime.h"

#define EOS '\0'
#define SEP ':'

#define NAME_MAX FILENAME_MAX
#define PATH_MAX FILENAME_MAX
#define OPEN_MAX FOPEN_MAX

extern int __pid, __ppid, __uid, __gid;
#define ROOT_UID	0
#define ROOT_GID	0

#define ENOEXEC	26
#define ENOSYS	38
#define EPERM	1

#define SIGQUIT SIGINT
#define SIGHUP SIGINT
#define SIGPIPE SIGINT

void *alloca(unsigned);

char *strupr(char*);
int chdir(char*);
int chmod(char*, mode_t);
int mkdir(char*, int);
FILE *popen(char*, char*);
int pclose(FILE*);
int fcntl(int, int, int, ...);
int ioctl(int, int, ...);
struct passwd *getpwent(void);
struct passwd *getpwnam(char*);
struct passwd *getpwuid(uid_t);
struct group *getgrent(void);
struct group *getgrname(char*);
struct group *getgrgid(gid_t);
void rewinddir(DIR*);
/* clock_t times(struct tms*); */
mode_t umask(mode_t);
int wait(int*);

/* Possible replacements */

int Stat(char*, long, struct stat*);
void Abort(void);
int Open(char*, int, ...);
void Perror(char*);
char *Getenv(char*);

int SetType(char*, FInfo);
