/*
 * Copyright (c) 1983 Regents of the University of California.
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

/* Reworked to GNU style by Ian Lance Taylor, ian@airs.com, August 93.  */

/*
 * uuencode [input] output
 *
 * Encode a file so it can be mailed to a remote system.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include "getopt.h"

/* Get definitions for the file permission bits.  */

#ifndef S_IRWXU
#define S_IRWXU 0700
#endif
#ifndef S_IRUSR
#define S_IRUSR 0400
#endif
#ifndef S_IWUSR
#define S_IWUSR 0200
#endif
#ifndef S_IXUSR
#define S_IXUSR 0100
#endif

#ifndef S_IRWXG
#define S_IRWXG 0070
#endif
#ifndef S_IRGRP
#define S_IRGRP 0040
#endif
#ifndef S_IWGRP
#define S_IWGRP 0020
#endif
#ifndef S_IXGRP
#define S_IXGRP 0010
#endif

#ifndef S_IRWXO
#define S_IRWXO 0007
#endif
#ifndef S_IROTH
#define S_IROTH 0004
#endif
#ifndef S_IWOTH
#define S_IWOTH 0002
#endif
#ifndef S_IXOTH
#define S_IXOTH 0001
#endif

#define	RW (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

static struct option longopts[] =
{
  { "version", 0, 0, 'v' },
  { "help", 0, 0, 'h' },
  { NULL, 0, 0, 0 }
};

#if __STDC__
static void encode (void);
static void usage (FILE *, int);
#else
static void encode ();
static void usage ();
#endif

extern char version[];
static char *program_name;

int
main (argc, argv)
     int argc;
     char **argv;
{
  int opt;
  struct stat sb;
  int mode;

  program_name = argv[0];

  while ((opt = getopt_long (argc, argv, "hv", longopts, (int *) NULL))
	 != EOF)
    {
      switch (opt)
	{
	case 'h':
	  printf ("Encode a file in a seven bit format\n");
	  usage (stdout, 0);

	case 'v':
	  printf ("%s\n", version);
	  exit (0);

	case 0:
	  break;

	default:
	  usage (stderr, 1);
	}
    }

  switch (argc - optind)
    {
    case 2:			/* optional first argument is input file */
      if (! freopen (argv[optind], "r", stdin)
	  || fstat (fileno (stdin), &sb))
	{
	  fprintf (stderr, "%s:", program_name);
	  perror (argv[optind]);
	  exit(1);
	}
      mode = sb.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
      ++optind;
      break;
    case 1:
      mode = RW & ~umask (RW);
      break;
    case 0:
    default:
      usage (stderr, 1);
    }

#if S_IRWXU != 0700
 #error Must translate mode argument
#endif

  printf ("begin %o %s\n", mode, argv[optind]);
  encode ();
  printf ("end\n");
  if (ferror (stdout))
    {
      fprintf (stderr, "uuencode: write error\n");
      exit (1);
    }
  exit (0);
}

/* ENC is the basic 1 character encoding function to make a char printing */
#define	ENC(c) ((c) ? ((c) & 077) + ' ': '`')

/*
 * copy from in to out, encoding as you go along.
 */
static void
encode ()
{
  register int ch, n;
  register char *p;
  char buf[80];

  while ((n = fread (buf, 1, 45, stdin)) != 0)
    {
      ch = ENC (n);
      if (putchar (ch) == EOF)
	break;
      for (p = buf; n > 0; n -= 3, p += 3)
	{
	  ch = *p >> 2;
	  ch = ENC (ch);
	  if (putchar (ch) == EOF)
	    break;
	  ch = ((*p << 4) & 060) | ((p[1] >> 4) & 017);
	  ch = ENC (ch);
	  if (putchar (ch) == EOF)
	    break;
	  ch = ((p[1] << 2) & 074) | ((p[2] >> 6) & 03);
	  ch = ENC (ch);
	  if (putchar (ch) == EOF)
	    break;
	  ch = p[2] & 077;
	  ch = ENC (ch);
	  if (putchar (ch) == EOF)
	    break;
	}
      if (putchar ('\n') == EOF)
	break;
    }
  if (ferror (stdin))
    {
      fprintf (stderr, "uuencode: read error\n");
      exit(1);
    }
  ch = ENC ('\0');
  putchar (ch);
  putchar ('\n');
}

static void
usage (f, status)
     FILE *f;
     int status;
{
  fprintf (f, "Usage: %s [infile] remotefile\n", program_name);
  exit (status);
}
