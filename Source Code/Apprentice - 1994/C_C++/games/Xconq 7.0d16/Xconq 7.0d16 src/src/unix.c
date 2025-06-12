/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Unix interface stuff.  Do NOT attempt to use this file in a non-Unix
   system, not even an ANSI one! */

#include "conq.h"

#include <signal.h>

#include <sys/time.h>

/* Attempt to open a library file. */

FILE *
open_library_file(module)
Module *module;
{
    FILE *fp = NULL;

    /* Don't try to do on anon modules? */
    if (module->name == NULL) return NULL;
    /* Generate library pathname. */
    make_pathname(xconqlib, module->name, "g", spbuf);
    /* Now try to open the file. */
    if ((fp = fopen(spbuf, "r")) != NULL) {
	/* Remember the filename where we found it. */
	module->filename = copy_string(spbuf);
    } else {
	/* Extra hack to find experimental games. */
	make_pathname("../lib2", module->name, "g", spbuf);
	if ((fp = fopen(spbuf, "r")) != NULL) {
	    /* Remember the filename where we found it. */
	    module->filename = copy_string(spbuf);
	}
    }
    return fp;
}

FILE *
open_explicit_file(module)
Module *module;
{
    if (module->filename == NULL) return NULL;
    return (fopen(module->filename, "r"));
}

make_pathname(path, name, extn, pathbuf)
char *path, *name, *extn, *pathbuf;
{
    sprintf(pathbuf, "");
    if (path && strlen(path) > 0) {
	sprintf(pathbuf+strlen(pathbuf), "%s/", path);
    }
    sprintf(pathbuf+strlen(pathbuf), "%s", name);
    /* Don't add a second identical extension, but do add if extension
       is different (in case we want "foo.12" -> "foo.12.g" for instance) */
    if (strrchr(name, '.')
	&& strcmp((char *) strrchr(name, '.')+1, extn) == 0)
      return;
    if (extn && strlen(extn) > 0) {
	sprintf(pathbuf+strlen(pathbuf), ".%s", extn);
    }
}

char *
newsfile_name()
{
    make_pathname(xconqlib, NEWSFILE, "", spbuf);
    return spbuf;
}

char *
savefile_name()
{
    return "save.xconq";
}

char *
checkpoint_name()
{
    return "check.xconq";
}

/* Remove a saved game from the system. */

remove_saved_game()
{
    unlink(savefile_name());
}

/* Default behavior on explicit kill. */

void
stop_handler(sig, code, scp, addr)
int sig, code;
struct sigcontext *scp;
char *addr;     
{
    close_displays();
    exit(1);
}

/* This routine attempts to save the state before dying. */

void
crash_handler(sig, code, scp, addr)
int sig, code;
struct sigcontext *scp;
char *addr;     
{
    static int already_been_here = FALSE;

    if (!already_been_here) {
	already_been_here = TRUE;
	close_displays();  
	printf("Fatal error encountered. Signal %d code %d\n", sig, code);
	write_entire_game_state("ack!.xconq");
    }
    abort();
}

/* Accidental disconnection should save state. */

void
hup_handler(sig, code, scp, addr)
int sig, code;
struct sigcontext *scp;
char *addr;     
{
    static int already_been_here = FALSE;

    if (!already_been_here) {
	already_been_here = TRUE;
	close_displays();  
	write_entire_game_state("ack!.xconq");
    }
    abort();
}

init_sighandlers()
{
    signal(SIGINT, stop_handler);
    if (0 /* don't accidently quit */ && !Debug) {
	signal(SIGINT, SIG_IGN);
    } else {
	signal(SIGINT, SIG_DFL);
/*	signal(SIGINT, crash_handler);  */
    }
    signal(1, crash_handler);
    signal(SIGHUP, hup_handler);
    signal(SIGBUS, crash_handler);
    signal(SIGSEGV, crash_handler);
    signal(SIGFPE, crash_handler);
    signal(SIGILL, crash_handler);
    signal(SIGSYS, crash_handler);
    signal(SIGINT, crash_handler);
    signal(SIGQUIT, crash_handler);
    signal(SIGTERM, crash_handler);
}

struct timeval reallasttime = { 0, 0 };

struct timeval realcurtime;

int n_seconds_elapsed(n)
int n;
{
    gettimeofday(&realcurtime, NULL);
    if (realcurtime.tv_sec > (reallasttime.tv_sec + (n - 1))) {
	reallasttime = realcurtime;
	return TRUE;
    } else {
	return FALSE;
    }
}
