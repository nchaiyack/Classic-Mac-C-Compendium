/* Mac-specific code for Xconq.
   Copyright (C) 1992, 1993, 1994, 1995 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"

#ifdef THINK_C
#include <MacHeaders>
#endif /* THINK_C */

#ifndef THINK_C /* assume MPW */
#include <Values.h>
#include <Types.h>
#include <Resources.h>
#include <QuickDraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Windows.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Desk.h>
#include <ToolUtils.h>
#include <Memory.h>
#include <SegLoad.h>
#include <Files.h>
#include <Folders.h>
#include <OSUtils.h>
#include <OSEvents.h>
#include <DiskInit.h>
#include <Packages.h>
#include <Traps.h>
#include <Lists.h>
#include <StandardFile.h>
#endif


#include <signal.h>

/* We need this in order to find string resources that have filenames
   in them. */
#include "macdefs.h"

/* #include <sys/time.h> */

#ifndef c2p
#define c2p(STR,PBUF) \
  strcpy(((char *) PBUF) + 1, STR);  \
  PBUF[0] = strlen(STR);
#endif

#ifndef p2c
#define p2c(PSTR,BUF)  \
  strncpy(BUF, ((char *) (PSTR) + 1), PSTR[0]);  \
  BUF[PSTR[0]] = '\0';
#endif


/* The HFS volume that the program started with. */

short initialvrefnum;

static char *news_fname;
static char *save_fname;
static char *checkpoint_fname;
static char *error_save_fname;
static char *statistics_fname;

#ifndef XCONQLIB
#define XCONQLIB ":lib"
#endif

char *
default_library_filename()
{
    return XCONQLIB;
}

char *
news_filename()
{
	Str255 tmpstr;
	char tmpbuf[255];

	if (news_fname == NULL) {
		GetIndString(tmpstr, sFilenames, siNews);
		p2c(tmpstr, tmpbuf);
		news_fname = copy_string(tmpbuf);
	}
	return news_fname;
}

char *
saved_game_filename()
{
	Str255 tmpstr;
	char tmpbuf[255];

	if (save_fname == NULL) {
		GetIndString(tmpstr, sFilenames, siSavedGame);
		p2c(tmpstr, tmpbuf);
		save_fname = copy_string(tmpbuf);
	}
	return save_fname;
}

char *
checkpoint_filename()
{
	Str255 tmpstr;
	char tmpbuf[255];

	if (checkpoint_fname == NULL) {
		GetIndString(tmpstr, sFilenames, siCheckpoint);
		p2c(tmpstr, tmpbuf);
		checkpoint_fname = copy_string(tmpbuf);
	}
	return checkpoint_fname;
}

char *
error_save_filename()
{
	Str255 tmpstr;
	char tmpbuf[255];

	if (error_save_fname == NULL) {
		GetIndString(tmpstr, sFilenames, siErrorSave);
		p2c(tmpstr, tmpbuf);
		error_save_fname = copy_string(tmpbuf);
	}
	return error_save_fname;
}

char *
statistics_filename()
{
	Str255 tmpstr;
	char tmpbuf[255];

	if (statistics_fname == NULL) {
		GetIndString(tmpstr, sFilenames, siStatistics);
		p2c(tmpstr, tmpbuf);
		statistics_fname = copy_string(tmpbuf);
	}
	return statistics_fname;
}

/* Attempt to open a library file. */

FILE *
open_module_library_file(Module *module)
{
	short curvrefnum;
	char fullnamebuf[255];
	FILE *fp = NULL;
	extern short initialvrefnum;
	
	/* Can't open anonymous library modules. */
	if (module->name == NULL)
	  return NULL;
	/* Generate library pathname. */
	make_pathname(xconqlib, module->name, "g", fullnamebuf);
	/* Now try to open the file. */
	fp = fopen(fullnamebuf, "r");
	if (fp != NULL) {
		/* Remember the filename where we found it. */
		module->filename = copy_string(fullnamebuf);
	} else {
		GetVol(NULL, &curvrefnum);
		SetVol(NULL, initialvrefnum);
		fp = fopen(fullnamebuf, "r");
		if (fp != NULL) {
			/* Remember the filename (what about volume?) where we found it. */
			module->filename = copy_string(fullnamebuf);
		}
		SetVol(NULL, curvrefnum);
	}
	return fp;
}

FILE *
open_module_explicit_file(Module *module)
{
	short curvrefnum;
	char fullnamebuf[255];
	FILE *fp = NULL;
	extern short initialvrefnum;

	if (module->filename == NULL) {
		/* Try guessing a filename, since none supplied. */
		if (module->name != NULL) {
			make_pathname(xconqlib, module->name, "g", fullnamebuf);
			fp = fopen(fullnamebuf, "r");
			if (fp != NULL) {
				return fp;
			} else {
				GetVol(NULL, &curvrefnum);
				SetVol(NULL, initialvrefnum);
				fp = fopen(fullnamebuf, "r");
				SetVol(NULL, curvrefnum);
			}
		}
	} else {
		sprintf(fullnamebuf, "%s", module->filename);
		fp = fopen(module->filename, "r");
		if (fp != NULL) {
			return fp;
		}
		sprintf(fullnamebuf, ":%s", module->filename);
		fp = fopen(fullnamebuf, "r");
		if (fp != NULL) {
			return fp;
		}
		sprintf(fullnamebuf, "%s%s", ":lib:", module->filename);
		fp = fopen(fullnamebuf, "r");
		if (fp != NULL) {
			return fp;
		}
		/* Try opening a library module under where the program started. */
		GetVol(NULL, &curvrefnum);
		SetVol(NULL, initialvrefnum);
		fp = fopen(fullnamebuf, "r");
		SetVol(NULL, curvrefnum);
	}
    return fp;
}

/* (should reindent) */
FILE *
open_library_file(char *filename)
{
	short curvrefnum;
	char fullnamebuf[255];
	FILE *fp;
	extern short initialvrefnum;
	
	/* Now try to open the file. */
	fp = fopen(filename, "r");
	if (fp != NULL)
	  return fp;
    /* Generate library pathname. */
    make_pathname(xconqlib, filename, NULL, fullnamebuf);
    fp = fopen(fullnamebuf, "r");
	if (fp != NULL)
	  return fp;
	/* Change to volume where the program started. */
	GetVol(NULL, &curvrefnum);
	SetVol(NULL, initialvrefnum);
	fp = fopen(fullnamebuf, "r");
	SetVol(NULL, curvrefnum);
	return fp;
}

void
make_pathname(char *path, char *name, char *extn, char *pathbuf)
{
    strcpy(pathbuf, "");
    if (!empty_string(path)) {
	strcat(pathbuf, path);
	strcat(pathbuf, ":");
    }
    strcat(pathbuf, name);
    /* Don't add a second identical extension, but do add if extension
       is different (in case we want "foo.12" -> "foo.12.g" for instance) */
    if (strrchr(name, '.')
	&& extn
	&& strcmp(strrchr(name, '.') + 1, extn) == 0)
      return;
    if (!empty_string(extn)) {
	strcat(pathbuf, ".");
	strcat(pathbuf, extn);
    }
}

/* Remove a saved game from the system. */

void
remove_saved_game()
{
}

void
init_signal_handlers()
{
}

int last_ticks = 0;

int
n_seconds_elapsed(n)
int n;
{
	int ticks = TickCount();

	if (((ticks - last_ticks) / 60) > n) {
		last_ticks = ticks;
    	return TRUE;
	} else {
		return FALSE;
	}
}

/* Instead of coredumping, which is not a normal Mac facility, we
   drop into Macsbug.  If we then "g" from Macsbug, the program will
   exit cleanly. */

void
mac_abort ()
{
  /* Make sure no output still buffered up, then zap into MacsBug. */
#if 0 /* how to know if stdio in use? */
  fflush(stdout);
  fflush(stderr);
  printf("## Abort! ##\n");
#endif
#ifdef MPW_SADE
  SysError(8005);
#else 
  Debugger();
#endif
  /* "g" in MacsBug will then cause a regular error exit. */
  exit(1);
}

