/* TranslateLinefeeds.h
This header should be included in all THINK C files that may use the MPW C
stdio library. It intercepts all i/o calls to the stdio library (except scanf
and fscanf), causing them to go through TranslateLinefeeds.c, which, if the i/o
is to a text (i.e. not binary) stream, interposes the filtering function 
TranslateLinefeeds to exchange /n and /r, between the caller and the (MPW) 
stdio library.

VideoToolbox.h automatically includes TranslateLinefeeds.h if
MATLAB is true.
*/
#pragma once
#define _TranslateLinefeeds_
#include <stdio.h>
#include <stdarg.h>

/*
The following typedef and define are copied from the MPW C 3.2 StdIO.h header file,
adding the prefix "MPW_". It is essential that they match the version used by MATLAB.
*/
typedef struct {
	long int 		_cnt;	/* dgp: added "long" */
	unsigned char	*_ptr;
	unsigned char	*_base;
	unsigned char	*_end;
	unsigned short	_size;
	unsigned short	_flag;
	unsigned short	_file;
} MPW_FILE;
#define MPW_IOBINARY	(1<<9)		/* Binary stream */

#if MATLAB
	/* Use MPW C's FILE struct */
	#define IS_TEXT(stream) !(((MPW_FILE *)(stream))->_flag&MPW_IOBINARY)
#else
	/* Use THINK C's FILE struct */
	#define IS_TEXT(stream) !(stream)->binary
#endif

int fgetcTL(FILE *stream);
char *fgetsTL(char *string,int n,FILE *stream);
int fprintfTL(FILE *stream,const char *format,...);
int fputsTL(char *string,FILE *stream);
int printfTL(char *format,...);
void TranslateLinefeeds(char *string,size_t n);
int vfprintfTL(FILE *stream,const char *format,va_list args);
int vprintfTL(char *format,va_list args);
size_t freadTL(void *s,size_t size,size_t numitems,FILE *stream);
size_t fwriteTL(void *s,size_t size,size_t numitems,FILE *stream);
char *getsTL(char *string);
int fputcTL(int ch,FILE *stream);
extern unsigned char translateLinefeed[256];

#define getc_STD getc
#define getchar_STD getchar
#define putc_STD putc
#define putchar_STD putchar
#define ungetc_STD ungetc
#undef getc
#undef getchar
#undef printf	/* discard cmex.h definition */
#undef putc
#undef putchar
#undef ungetc
#define fgetc(stream) fgetcTL(stream)
#define fgets fgetsTL
#define fprintf fprintfTL
#define fputc fputcTL
#define fputs fputsTL
#define fread freadTL
#define fwrite fwriteTL
#define getc(stream) (IS_TEXT(stream)?(int)translateLinefeed[(unsigned char)getc_STD(stream)]:getc_STD(stream))
#define getchar() ((int)translateLinefeed[(unsigned char)getchar_STD()])
#define gets getsTL
#define printf printfTL
#define putc(c,stream) putc_STD(IS_TEXT(stream)?translateLinefeed[(unsigned char)c]:c,stream)
#define putchar(c) putchar_STD(translateLinefeed[(unsigned char)(c)])
#define puts(string) fputsTL(string,stdout)
#define ungetc(c,stream) ungetc_STD(IS_TEXT(stream)?translateLinefeed[(unsigned char)c]:c,stream)
#define vfprintf vfprintfTL
#define vprintff vprintfTL
