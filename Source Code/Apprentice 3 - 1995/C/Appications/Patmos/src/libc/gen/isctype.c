/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)isctype.c	5.2 (Berkeley) 6/1/90";
#endif /* LIBC_SCCS and not lint */

#define _ANSI_LIBRARY
#include <ctype.h>

#define	toupper_(c)	(islower(c) ? (c)^0x20 : (c))
#define	tolower_(c)	(isupper(c) ? (c)^0x20 : (c))

static isalnum_(int c) { return isalnum(c); }
static isalpha_(int c) { return isalpha(c); }
static iscntrl_(int c) { return iscntrl(c); }
static isdigit_(int c) { return isdigit(c); }
static isgraph_(int c) { return isgraph(c); }
static islower_(int c) { return islower(c); }
static isprint_(int c) { return isprint(c); }
static ispunct_(int c) { return ispunct(c); }
static isspace_(int c) { return isspace(c); }
static isupper_(int c) { return isupper(c); }
static isxdigit_(int c) { return isxdigit(c); }

#undef isalnum
#undef isalpha
#undef iscntrl
#undef isdigit
#undef isgraph
#undef islower
#undef isprint
#undef ispunct
#undef isspace
#undef isupper
#undef isxdigit
#undef tolower
#undef toupper

int isalnum(int c) { return isalnum_(c); }
int isalpha(int c) { return isalpha_(c); }
int iscntrl(int c) { return iscntrl_(c); }
int isdigit(int c) { return isdigit_(c); }
int isgraph(int c) { return isgraph_(c); }
int islower(int c) { return islower_(c); }
int isprint(int c) { return isprint_(c); }
int ispunct(int c) { return ispunct_(c); }
int isspace(int c) { return isspace_(c); }
int isupper(int c) { return isupper_(c); }
int isxdigit(int c) { return isxdigit_(c); }
int tolower(int c) { return tolower_(c); }
int toupper(int c) { return toupper_(c); }
