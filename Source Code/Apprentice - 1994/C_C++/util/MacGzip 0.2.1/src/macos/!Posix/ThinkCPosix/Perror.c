/* $Id: $ */

/*
 *  Perror.c
 *
 *  Copyright (c) 1991 Symantec Corporation.  All rights reserved.
 *  Modified (from perror.c) by Timothy Murphy <tim@maths.tcd.ie>
 *  to give meaningful error messages.
 *
 */

#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "ThinkCPosix.h"
#include "ansi_private.h"

static void error(FILE *, int);


void
Perror(char *s)
{
	int i = errno;
	
	if (s && *s)
		fprintf(stderr, "%s: ", s);
	error(stderr, i);
	fputc('\n', stderr);
}


static void
error(FILE *fp, int i)
{
	switch(i) {
	case ENOTDIR:	fprintf(fp, "ENOTDIR"); break;
	case EACCES:	fprintf(fp, "Permission denied"); break;
	case EEXIST:	fprintf(fp, "File already exists"); break;
	case EBUSY:		fprintf(fp, "File busy"); break;
	case EROFS:		fprintf(fp, "EROFS"); break;
	case ENOENT:	fprintf(fp, "No such file or directory"); break;
	case ENFILE:	fprintf(fp, "ENFILE"); break;
	case EIO:		fprintf(fp, "EIO"); break;
	case ENOSPC:	fprintf(fp, "ENOSPC"); break;

	case ESRCH:		fprintf(fp, "ESRCH"); break;
	case EINTR:		fprintf(fp, "EINTR"); break;
	case EBADF:		fprintf(fp, "Bad file number"); break;
	case ENODEV:	fprintf(fp, "No such device"); break;
	case EINVAL:	fprintf(fp, "Invalid argument"); break;
	case EMFILE:	fprintf(fp, "Too many open files"); break;
	case EDOM:		fprintf(fp, "Domain error"); break;
	case ERANGE:	fprintf(fp, "Result too large"); break;
	case ENOSYS:	fprintf(fp, "Function not implemented"); break;
	case ENOEXEC:	fprintf(fp, "Exec format error"); break;
	case EPERM:		fprintf(fp, "Operation not permitted"); break;
	default:		fprintf(fp, "Unknown error %d", i);
	}
}
