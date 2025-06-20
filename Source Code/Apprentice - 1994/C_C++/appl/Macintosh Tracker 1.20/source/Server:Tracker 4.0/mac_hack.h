/* mac_hack.h */

#pragma once

/* All the stuff in this file was written by Thomas R. Lawrence. */
/* See the "mac_readme" or "mac_programmer_info" files for more information */
/* about the Macintosh port */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <unix.h>

/********************************************************************************/
/* High-level hackery */


int MYprintf(...);
#define printf MYprintf

int MYfprintf(...);
#define fprintf MYfprintf


/* note: this structure supports reading only.  There is no copyback of dirty info */
#define FILEBUFFERSIZE (4096)
typedef struct
	{
		short			MacFileHandle;
		short			BufPtr;
		long			Index;
		long			EndOfFile;
		char			Buffer[FILEBUFFERSIZE];
	} MyFILE;

#define FILE MyFILE

int MYfgetc(FILE* FileToGetFrom);
#define fgetc(FileToGetFrom)\
(((FileToGetFrom->BufPtr != FILEBUFFERSIZE)\
	&& (FileToGetFrom->Index < FileToGetFrom->EndOfFile))\
?\
	(\
		FileToGetFrom->BufPtr += 1,\
		FileToGetFrom->Index += 1,\
		(int)(unsigned char)FileToGetFrom->Buffer[FileToGetFrom->BufPtr - 1]\
	)\
:\
	(MYfgetc(FileToGetFrom)))

int MYfputc(int CharToPut, FILE *FileToPutTo);
#define fputc MYfputc

FILE* MYfopen(char* FileName, char* Mode);
#define fopen MYfopen

int MYfclose(FILE* FileToClose);
#define fclose MYfclose

int MYfread(char* PlaceToPut, int SizeOfElement, int NumElements, FILE* TheFile);
#define fread MYfread

void* MYmalloc(long SizeOfBlock);
#define malloc MYmalloc

void MYfree(void* Block);
#define free MYfree

/* to allow for my own antialiasing-friendly readin routine */
void *MYcalloc(size_t NumThings, size_t SizeOfThing);
#define calloc MYcalloc

#define fflush(stupid)

void MYexit(int Value);
#define exit MYexit

char* MYgetenv(char* MeaninglessParameter);
#define getenv MYgetenv

void MYperror(char* ErrorMessage);
#define perror MYperror

void MYputs(char* Message);
#define puts MYputs

#undef putchar
#define putchar(thang)

#undef getchar
#define getchar(thang)

#undef stdin
#define stdin NULL
#undef stdout
#define stdout NULL
#undef stderr
#define stderr NULL

#undef isalnum
#define isalnum(x) 0
#undef isalpha
#define isalpha(x) 0
#undef iscntrl
#define iscntrl(x) 0
#undef isdigit
#define isdigit(x) 0
#undef isgraph
#define isgraph(x) 0
#undef islower
#define islower(x) 0
#undef isprint
#define isprint(x) 0
#undef ispunct
#define ispunct(x) 0
#undef isspace
#define isspace(x) 0
#undef isupper
#define isupper(x) 0
#undef isxdigit
#define isxdigit(x) 0

#define NDEBUG /* for assert */

long double MYfloor(long double Base);
#define floor MYfloor

long double MYpow(long double Base, long double Exponent);
#define pow MYpow


/********************************************************************************/



/*#include "defs.h"*/
#define BOOL int  /* taken from "defs.h"; #undef'd later in this file */

#undef LOCAL

#undef NULL
#define NULL (0L)

/* prototypes */

struct channel;
struct event;

/* altered_audio.c */
void init_tables(int oversample, int frequency);
void reset_note(struct channel *ch, int note, int pitch);
void set_current_pitch(struct channel *ch, int pitch);
void set_current_volume(struct channel *ch, int volume);

/* automaton.c */
/* void init_automaton(struct automaton *a, struct song *song, int start, BOOL s); */
void next_tick(struct automaton *a);

/* commands.c */
void do_nothing(struct channel *ch);
void do_slide(struct channel *ch);
void do_vibrato(struct channel *ch);
void do_arpeggio(struct channel *ch);
void do_slidevol(struct channel *ch);
void do_retrig(struct channel *ch);
void do_latestart(struct channel *ch);
void do_cut(struct channel *ch);
void do_portamento(struct channel *ch);
void do_portaslide(struct channel *ch);
void do_vibratoslide(struct channel *ch);
void set_nothing(struct automaton *a, struct channel *ch);
void set_upslide(struct automaton *a, struct channel *ch);
void set_downslide(struct automaton *a, struct channel *ch);
void set_vibrato(struct automaton *a, struct channel *ch);
void set_arpeggio(struct automaton *a, struct channel *ch);
void set_slidevol(struct automaton *a, struct channel *ch);
void set_extended(struct automaton *a, struct channel *ch);
void set_portamento(struct automaton *a, struct channel *ch);
void set_portaslide(struct automaton *a, struct channel *ch);
void set_vibratoslide(struct automaton *a, struct channel *ch);
void set_speed(struct automaton *a, struct channel *ch);
void set_skip(struct automaton *a, struct channel *ch);
void set_fastskip(struct automaton *a, struct channel *ch);
void set_offset(struct automaton *a, struct channel *ch);
void set_volume(struct automaton *a, struct channel *ch);
void parse_slidevol(struct channel *ch, int para);
void init_effects(void (*table[])());

/* dump_song.c */
void dump_song(struct song *song);

/* getopt.c */
int getopt(int argc, char *argv[], struct long_option *options);

/* macintosh_audio.c */
void      set_mix(int percent);
int       open_audio(int SampleRate, int StereoFlag);
void      actually_flush_buffer(void);
void      flush_buffer(void);
void      close_audio(void);
void      set_synchro(int s);
int       update_frequency(void);
void      discard_buffer(void);
void      resample(int oversample, int number);

/* main.c */
void      end_all(char *s);

/* notes.c */
int find_note(int pitch);
void create_notes_table(void);
int transpose_song(struct song *s, int transpose);

/* open.c */
struct exfile;
struct exfile *open_file(char *fname, char *fmode, char *path);
void close_file(struct exfile *file);

/* player.c */
void init_player(int o, int f);

/* read.c */
void release_song(struct song *song);
struct song *read_song(struct exfile *f, int type);

/* setup_audio.c */
/* void setup_audio(int f, BOOL s, int o, BOOL sync); */
void do_close_audio(void);

/* tools.c */
int read_env(char *name, int def);

/* termio.c KLUDGES */
BOOL run_in_fg(void);
void sane_tty(void);
void nonblocking_io(void);
void* popen(char* pipe, char* Mode);
void pclose(FILE* file);
int may_getchar(void);


/* to allow for my own main routine */
#define main main2
int main2(int argc, char **argv);


#undef BOOL  /* must get rid of it because a typedef comes later */
