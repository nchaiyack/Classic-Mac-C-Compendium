/*
 * ipdr.h
 *
 * Internet Protocol Data Representation
 *
 * This file should remove some of the burden from ip.c
 *
 * MDT 19 JAN 95
 */
 
#ifndef __IPDATAREP_HEADER__
#define __IPDATAREP_HEADER__

/* Data Representation */

int htons(int x);
int ntohs(int x);

long htonl(long y);
long ntohl(long y);

#endif /* __IPDATAREP_HEADER__ */