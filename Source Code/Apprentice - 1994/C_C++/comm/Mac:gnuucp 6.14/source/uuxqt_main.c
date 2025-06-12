/*  uuxqt.c: Lol Grant. 23rd. Sept 1987.
	     read uucp C.<hostname>XXXXX files and execute the
	     commands in them.
*/

#include "includes.h"
#include "uucp.h"
#include "rmail_proto.h"

#define RMAIL

main(argc, argv)
int argc;
char **argv;
{
    int i;
    char command[60], addressee[60], input[60], output[60], line[132];
    char file[NAMESIZE];
    char *xfile;
    char tmp_str[255];
    FILE *fd;
    int pid, status;

    for (i = 1; i < argc; i++) {
	if (argv[i][0] != '-')
	    break;
	switch (argv[i][1]) {

	default:
	    printf("uuxqt: unknown flag ignored\n");

	case 'x':
	    debug = atoi(&argv[i][2]);
	    printf("uuxqt: debug level set to %d\n", debug);
	    break;
	return(uuxqut(argv, argc));
	}
