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

/* AIX requires this to be the first thing in the file.  */
#if defined (_AIX) && !defined (__GNUC__)
 #pragma alloca
#endif

/*
 * uudecode [file ...]
 *
 * create the specified file, decoding as you go.
 * used with uuencode.
 */
#include <sys/stat.h>
#include <pwd.h>
#include <stdio.h>
#include "getopt.h"

#ifdef	__GNUC__
#undef	alloca
#define	alloca(n)	__builtin_alloca (n)
#else	/* Not GCC.  */
#ifdef	HAVE_ALLOCA_H
#include <alloca.h>
#else	/* Not HAVE_ALLOCA_H.  */
#ifndef	_AIX
extern char *alloca ();
#endif	/* Not AIX.  */
#endif	/* HAVE_ALLOCA_H.  */
#endif	/* GCC.  */

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

static struct option longopts[] =
{
  { "version", 0, 0, 'v' },
  { "help", 0, 0, 'h' },
  { NULL, 0, 0, 0 }
};

#if __STDC__
static int decode (char *);
static void usage (FILE *, int);
#else
static int decode ();
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
  int rval;

  program_name = argv[0];

  while ((opt = getopt_long (argc, argv, "hv", longopts, (int *) NULL))
	 != EOF)
    {
      switch (opt)
	{
	case 'h':
	  printf ("Decode a file created by uuencode\n");
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

  if (optind == argc)
    rval = decode ((char *) "stdin");
  else
    {
      char *filename;

      rval = 0;
      do
	{
	  if (freopen (argv[optind], "r", stdin) != NULL)
	    rval |= decode (argv[optind]);
	  else
	    {
	      fprintf (stderr, "%s:", program_name);
	      perror (argv[optind]);
	      rval = 1;
	    }
	  ++optind;
	}
      while (optind < argc);
    }

  exit (rval);
}

#define	DEC(c)	(((c) - ' ') & 077) /* single character decode */

static int
decode (filename)
     char *filename;
{
  struct passwd *pw;
  register int n;
  register char ch, *p;
  int mode, n1;
  char buf[2 * BUFSIZ];
  char *outname;

  /* search for header line */
  do
    {
      if (fgets (buf, sizeof (buf), stdin) == NULL)
	{
	  fprintf (stderr,
		  "%s:%s: no \"begin\" line\n", program_name, filename);
	  return 1;
	}
    }
  while (strncmp (buf, "begin ", 6) != 0);

  sscanf (buf, "begin %o %s", &mode, buf);

  /* handle ~user/file format */
  if (buf[0] != '~')
    outname = buf;
  else
    {
      p = buf + 1;
      while (*p != '/')
	++p;
      if (*p == '\0')
	{
	  fprintf (stderr, "%s:%s: illegal ~user\n", program_name,
		   filename);
	  return 1;
	}
      *p++ = '\0';
      pw = getpwnam (buf + 1);
      if (pw == NULL)
	{
	  fprintf (stderr, "%s:%s: no user %s\n", program_name,
		   filename, buf + 1);
	  return 1;
	}
      n = strlen (pw->pw_dir);
      n1 = strlen (p);
      outname = (char *) alloca (n + n1 + 2);
      memcpy (outname + n + 1, p, n1 + 1);
      memcpy (outname, pw->pw_dir, n);
      outname[n] = '/';
    }

  /* create output file, set mode */
  if (freopen (outname, "w", stdout) == NULL
      || fchmod (fileno (stdout),
		 mode & (S_IRWXU | S_IRWXG | S_IRWXO)))
    {
      fprintf (stderr, "%s:%s:", program_name, outname);
      perror (filename);
      return 1;
    }

  /* for each input line */
  while (1)
    {
      if (fgets (buf, sizeof(buf), stdin) == NULL)
	{
	  fprintf (stderr, "%s:%s: short file.\n", program_name,
		   filename);
	  return 1;
	}
      p = buf;
      /*
       * `n' is used to avoid writing out all the characters
       * at the end of the file.
       */
      n = DEC (*p);
      if (n <= 0)
	break;
      for (++p; n > 0; p += 4, n -= 3)
	{
	  if (n >= 3)
	    {
	      ch = DEC (p[0]) << 2 | DEC (p[1]) >> 4;
	      putchar (ch);
	      ch = DEC (p[1]) << 4 | DEC (p[2]) >> 2;
	      putchar (ch);
	      ch = DEC (p[2]) << 6 | DEC (p[3]);
	      putchar (ch);
	    }
	  else
	    {
	      if (n >= 1)
		{
		  ch = DEC (p[0]) << 2 | DEC (p[1]) >> 4;
		  putchar (ch);
		}
	      if (n >= 2)
		{
		  ch = DEC (p[1]) << 4 | DEC (p[2]) >> 2;
		  putchar (ch);
		}
	    }
	}
    }

  if (fgets (buf, sizeof(buf), stdin) == NULL
      || strcmp (buf, "end\n"))
    {
      fprintf (stderr, "%s:%s: no \"end\" line.\n", program_name,
	       filename);
      return 1;
    }

  return 0;
}

static void
usage (f, status)
     FILE *f;
     int status;
{
  fprintf (f, "usage: %s [file ...]\n", program_name);
  exit (status);
}
