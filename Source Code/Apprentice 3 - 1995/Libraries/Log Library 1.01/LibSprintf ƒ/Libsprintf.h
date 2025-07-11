/*
	Libsprintf.h
	
	Header file for Libsprintf.c
	
*/

#pragma once

#ifndef __H_Libsprintf__
#define __H_Libsprintf__

#ifdef __cplusplus
extern "C" {
#endif

int libsprintf(char *fp, const char *fmt,...);
int libvsprintf(char *fp, const char *fmt,void*);
void libfixnl(char* buf);
char libstrlen(char* buf);
void * libmemchr(const void *s, int c,long n);

#ifdef __cplusplus
}
#endif

#endif
