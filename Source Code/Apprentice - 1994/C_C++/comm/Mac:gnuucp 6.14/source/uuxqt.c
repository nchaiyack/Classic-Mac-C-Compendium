/*  uuxqt.c: Lol Grant. 23rd. Sept 1987.
	     read uucp C.<hostname>XXXXX files and execute the
	     commands in them.
*/

#include "includes.h"
#include "uucp.h"

#define RMAIL

int
uuxqt(debug)
int debug;
 	{

    	int i;
    	char command[256], addressee[256], input[256], output[256], line[256];
    	char file[NAMESIZE];
    	char *xfile;
    	char tmp_str[256];
    	FILE *fd;
    	int pid, status;
		cuserid(who);		/* Finger out who we are */
    	strcpy(host_name, "uuxqt");	/* We know who this is */

    	read_params(NULL);

    	if (chdir(Spool) != 0)
    		{
				printf("can't change to %s\n", Spool);
				logit("FAILED", "can't change to Spool");
				return(EXIT_ERR);
    			}
    	logit("OK", "startup");

    	if (!work_scan(NULL, "X"))
    		{
				logit("Completed", "No Work");
				return(EXIT_OK);
    			}
    	while ((xfile = work_next()) != NULL)
    		{
				sprintf(file, "%s:%s", Spool, (char *)munge_filename(xfile));
				if ((fd = fopen(file, "rb")) == NULL)
					{
	    				logit("CAN'T READ", file);
	    				remove(file);
	    				continue;
						}
				logit("FILE", file); 
				input[0] = output[0] = (char)NULL;

				while (fgets(line, sizeof(line), fd) != NULL) {
	    		line[strlen(line) - 1] = '\0';
	    		DEBUG(2, "uuxqt:  %s\n", line);
	    		switch (line[0]) {
	   			case 'U':
					break;
	    		case 'I':
					sprintf(input, "%s:%s", Spool, munge_filename(&line[2]));
					DEBUG(2, "uuxqt: input %s\n", input);
					break;
	    		case 'O':
					sprintf(output, "%s:%s", Spool, munge_filename(&line[2]));
					DEBUG(2, "uuxqt: output %s\n", output);
					break;
	    		case 'C':
					sscanf((char *)&line[2], "%s %s", command, addressee);
					DEBUG(2, "uuxqt: command %s\n", command);
					DEBUG(2, "uuxqt: addressee %s\n", addressee);
					break;
	    		case 'R':
					break;
	    		default:
					break;
	    		}
		}
		fclose(fd);
		if ((i = strlen(input)) == 0)
	    	strcpy(input, NULL_DEVICE);
		if ((i = strlen(output)) == 0)
	    	strcpy(output, NULL_DEVICE);
		sprintf(line, "%s %s < %s > %s", command, addressee, input, output);
        logit("XQT", line);
		if (invoke(command, addressee, input, output) != EXIT_OK)
			{
	    		sprintf(line, "%s %s %s %s", command, addressee, input, output);
	    		logit("FAILED", line);
	    		if (invoke(command, postmaster, input, output) != EXIT_OK) 
	    			{
						sprintf(line, "%s %s %s %s", command, postmaster, 
								input, output);
						logit("FAILED", line);
						logit("Check Spool Directory for Inconsistencies", xfile);
						exit(EXIT_ERR);
						/* continue; */
	    				}
				}
		if (remove(file) == 0)
	    	{
	    		DEBUG(2, "deleted %s\n", file);
	    		}
		  else
	    	{
	    		DEBUG(0, "Can't delete %s\n", file);
	    		}
		if (strcmp(input, NULL_DEVICE))
	    if (remove(input) == 0)
			{
				DEBUG(2, "deleted %s\n", input);
				}
	    	else
			{
				DEBUG(0, "Can't delete %s\n", input);
				}
		if (strcmp(output, NULL_DEVICE))
	    	if (remove(output) == 0)
				{
					DEBUG(2, "deleted %s\n", output);
					}
	    	else
				{
					DEBUG(0, "Can't delete %s\n", output);
					}
    	}
    	work_done();
		logit("OK", "finished");
    	return(EXIT_OK);
	}

/* Invoke the local program. Return its status (0 == ok) or -1 */
int invoke(command, addressee, input, output)
char *command, *addressee, *input, *output;
{
    int i, pid;
    struct STATUS status;
	char tmp[256];
	DEBUG(5, "Invoking: %s\n", command);
	DEBUG(5, "Invoke addressee: %s\n", addressee);
	DEBUG(5, "Invoke input: %s\n", input);
	DEBUG(5, "Invoke output: %s\n", output);
	if (strcmp(command, "rmail") == 0)
		{
			return(do_rmail(addressee, input, output));
			}
	/* Try to turn news into mail and proceed */
	if (strcmp(command, "rnews") == 0)
		{
			return(do_rmail(addressee, input, output));
			}
	logit("Unknown Command", command);
	/* Don't stop processing if we get an unrecognized command */
	return(EXIT_OK);
	}
