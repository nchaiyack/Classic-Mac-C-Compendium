/* getopt.h */

#pragma once


extern int optind;
extern char *optarg;

int getopt(int argc, char *argv[], char *options);
