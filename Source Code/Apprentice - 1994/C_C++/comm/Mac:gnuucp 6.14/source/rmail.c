#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <pascal.h>

#define MAILCMD "rmail"
#define MAXTRIES 20
#include "includes.h"
#include "uucp.h"

#ifdef VMS
#define TERMINATOR "CTRL-Z"
#define MAILSTR "MAIL %s %s"
#else
#define MAILSTR "/bin/mail < %s %s"
#define TERMINATOR "CTRL-D"
#endif

void print_out(char *, int);

void
print_out (str, i)
char *str;
int i;
{
	int count;
	printf("String: \n");
	for (count = 0; count < i; count++)
		{
			putchar(str[count]);
			}
	putchar('\n');
	}

char *rmail_version = "Mac/rmail version 6.14";

int process_queue;

int doing_alias = 0;

char curr_dir[NAMESIZE];

int
rmail (argc, argv)
int argc;
char *argv[];

{
    int i;
    for (i = 1; i < argc; i++) {
	if (argv[i][0] != '-')
	    break;
	switch (argv[i][1]) {

	default:
	    printf("rmail: unknown flag ignored\n");

	case 'x':
	    debug = atoi(&argv[i][2]);
	    printf("rmail: debug level set to %d\n", debug);
	    break;
	case 'q':
		process_queue = true;
		printf("rmail: entering server mode\n");
		}
    }
	read_params(0);

    cuserid(who);
    strcpy(host_name, "rmail");

    if (Spool == NULL) {
	printf("cannot read Spool directory from Control file\n");
	exit(EXIT_ERR);
    }
    if (Sysfile == NULL) {
	printf("cannot read Sysfile directory from Control file\n");
	exit(EXIT_ERR);
    }
    if (Myname[0] == (char)NULL) {
	printf("cannot read Node name from Control file\n");
	exit(EXIT_ERR);
    }
    /* getwd(curr_dir); */

	if (chdir(Spool) != 0) {
	printf("cannot change to %s\n", Spool);
	perror("can't change directory");
	exit(EXIT_ERR);
    }
	if (process_queue)
			{
				do_rmail_forever();
				return(EXIT_OK);
				}
		else
			return(do_rmail_interactive(argc, argv));
	}
    
    
int
do_rmail_interactive(argc, argv)
	int argc;
	char *argv[];
	{
		FILE *fd;
		int i;
		int err;
		char *argvuser = NULL;
		char *argvfile = NULL;
		for (i = 1; i < argc; i++) {
			if (argv[i][0] == '-')
	    	continue;

		if (argvuser == NULL) 
			{
	    		argvuser = argv[i];
	    		continue;
				}
		if (argvuser != NULL) 
			{
	    		argvfile = argvuser;
	    		argvuser = argv[i];
	    		break;
				}
    	}

    	if (argvuser == NULL) 
    		{
				printf("usage: rmail [-xN] [file] \"site!site!...user[@site]\"\n");
				exit(EXIT_ERR);
    			}
    	for (i = 0; i < argc; i++)
		DEBUG(2, "arg=%s\n", argv[i]);

    	DEBUG(5, "user=%s\n", (argvuser == NULL ? "NULL" : argvuser));
    	DEBUG(5, "file=%s\n", (argvfile == NULL ? "NULL" : argvfile));
    	if (argvfile == NULL) 
    		{
				mlogit("Can't find input file for", "RMAIL");
				exit(EXIT_ERR);
				/* argvfile = "stdin";
				fd = stdin; */
    			} 
    		else
    		{
				if ((fd = fopen(argvfile, "r")) == NULL) 
					{
	    				mlogit("CAN'T OPEN", argvfile);
	    				perror("can't open file for reading");
	    				exit(EXIT_ERR);
						}
				}
		/* if (fd == stdin)
			printf("\nEnter Message terminated with %s\n", TERMINATOR); */
		err = do_one_rmail(argvuser, argvfile, fd);
		if (fd != stdin)
			{
				fclose(fd);
				FlushVol(NULL, 0);
				}
		return(err);
		}
	
char *strip_local_host (address)
char *address;
	{
		int delim;
		char tmp_address[1024];
		delim = mindex(address, '!');
		if ( (delim > 0) &&
		     ( (strncmp(address, Myname, strlen(Myname)) == 0) ||
		       (strncmp(address, Mynamealias, strlen(Myname)) == 0)))
			{
				strcpy(tmp_address, address+delim+1);
				strcpy(address, tmp_address);
				}
		delim = mindex(address, '@');
		if ( (delim > 0) &&
		   	 ( (strcmp(address+delim+1, Myname) == 0) ||
		       (strcmp(address+delim+1, Mynamealias) == 0) ) )
		   {
		   		address[delim] = '\0';
		   		}
		 return(address);
		 }
   
int known_host (address)
char *address;
	{
		int delim;
		char host[256];
		int retval = 0;
		delim = mindex(address, '!');
		if (delim > 0)
			{
				strncpy(host, address, delim);
				host[delim]='\0';
				retval = validate_site(host, 0);
				}
		if (retval == EXIT_ERR)
			{
				retval = 0;
				}
		return(retval);
		}

int
string_contains(st1, st2)
char *st1;
char *st2;
{
	int c1;
	int c2;
	int len1;
	int len2;
	int success;
	len1 = strlen(st1);
	len2 = strlen(st2);
	success = 1;
	for (c2 = 0; c2 < len2; c2++)
		{
			success = 1;
			for (c1 = 0; c1 < len1; c1++)
				{
					if (st1[c1] == st2[c2])
						{
							c2++;
							if (c2 > len2)
								{
									success = 0;
									break;
									}
							}
						else
						{
							success = 0;
							break;
							}
					}
			 if (success == 1)
			 	break;
			}
	return(success);
	}
								
int
do_one_rmail(argvuser, argvfile, fd)
char *argvuser;
char *argvfile;
FILE *fd;
	{
		int i;
		int local;
		int seq;
		char rmtsite[256];
		char rmtuser[256];
		char local_data[256];
		char tmp_str[256];
    	char *cp, data[256], datetime[40];
    	char c, user[256], line[NAMESIZE];
    	char filebuf[256];
    	char recipient[256];
    	long nread;
    	unsigned long temptime;
		FILE *tfd;
		local = 0;
		strcpy(recipient, argvuser);
    /* if you can't find a "!" or an @, assume local delivery */
        argvuser = strip_local_host(recipient);
    	if ((i = mindex(argvuser, '!')) <= 0 &&
    		(i = mindex(argvuser, '@')) <= 0 )
    		{
				argvuser = argvuser + i+1;
				strcpy(rmtsite, "tmp");	/* VMS needs this or filenames aren't legal */
				local++;
    			} 
    		else 
    		{
				rmtsite[0] = '\0';
				if (!known_host(argvuser))
					{
						strcpy(rmtsite, Forwarder);
						strcpy(rmtuser, &argvuser[0]);
						}
					else
					  {
					  	strcat(rmtsite, argvuser);
						rmtsite[i] = '\0';
						strcpy(rmtuser, &argvuser[i+1]);
						}
				if (validate_site(rmtsite, 1) != EXIT_OK)
					{
						local++;
						strcpy(rmtsite, "");
						strcpy(argvuser, postmaster);
						}
    			}
    	sprintf(tmp_str, "D.%.30sa", rmtsite);
    	strcpy(tmp_str, munge_filename(tmp_str));
	    seq = get_seq(Spool, data);
    	HandleEvents();
    	/* sprintf(data, "D.%.7sA%04d", rmtsite, seq); */
    	sprintf(data, "%s%04d", tmp_str, seq);
    	sprintf(local_data, "%s:%s", Spool, data);
		DEBUG(2, "Using data name %s\n", local_data);

    	if ((tfd = fopen(local_data, "wb")) == NULL) 
    		{
				mlogit("CAN'T CREATE", local_data);
				fprintf(stderr, "errno = %x\n", errno);
				perror("can't open file for writing");
				exit(EXIT_ERR);
    			}
    	time(&temptime);
    	sprintf(datetime, "%s%s", currtime(), Timezone);
    	for (cp = datetime; *cp != '\n'; cp++);
    	*cp = ' ';

    	/* forward to rmtsite */
    	cuserid(user);		/* who am I ? */
    	HandleEvents();
    	for (cp = user; *cp; cp++)
		if (isupper(*cp))
	    	*cp = tolower(*cp);

    	DEBUG(2, "reading %s\n", argvfile);

    	/* copy data into spool file */
    	/* Get first line or first 255 chars */
    	i = 0;
    	while ( ((c = fgetc(fd)) != EOF) && (i < 255) && (c != '\n'))
		{
			HandleEvents();
			tmp_str[i] = c;
			i++;
			}
		tmp_str[i] = '\0';
		/* Found one line and it begins with "From " */
		if ((c == '\n') && (strncmp("From ", tmp_str, 5) == 0))
			{
				char *name_end;
				char *remote_node;
				char remote_name[256];
				char user_name[256];
				char new_user_name[256];
				sscanf(tmp_str+5, "%s", user_name);
				remote_node = strstr(tmp_str, "remote from");
    			if (remote_node != NULL)
    				{
    					remote_node = remote_node + 12;
    					sscanf(remote_node, "%s", remote_name);
    					if (strcmp(remote_name, Myname) == 0)
    						sprintf(new_user_name, "%s", user_name);
    					   else
    					   	sprintf(new_user_name, "%s!%s", remote_name, user_name);
    					}
    				else
    				{
    					strcpy(new_user_name, user_name);
    					}
    			fprintf(tfd, "From %s %s remote from %s\n", 
    						  new_user_name, datetime, Myname);
    			if (doing_alias == 0)
    				{
    					fprintf(tfd, "Received: by %s (Mac/gnuucp v6.14) %s\n", 
    			             		  Myname, datetime);
    					}
    			}
			else
				{
					if (doing_alias == 0)
    					{
    						fprintf(tfd, "Received: by %s (Mac/gnuucp v6.14) %s\n", 
    			              		  	  Myname, datetime);
    						}
    				fprintf(tfd, "%s", tmp_str);
    				if (tmp_str[strlen(tmp_str)-1] != '\n')
    					fputc('\n', tfd);
    				}
	 	while (!feof(fd))
		{
			nread = fread(filebuf, sizeof(char), 255L, fd);
			HandleEvents();
			/* printf("Read: %ld chars from %s to %s\n", 
						nread, argvfile, local_data); */
			if (nread > 0)
				fwrite(filebuf, sizeof(char), nread, tfd);
			if (ferror(fd))
				mlogit("Error reading file", argvfile);
			if (ferror(tfd))
				mlogit("Error writing file", local_data);
			}

   	 	DEBUG(2, "%s read\n", argvfile);
		if (fclose(tfd))
    		mlogit("Can't close:", local_data);
		HandleEvents();
		if (local)
    		{
    			if (aliasp(argvuser))
    				return(deliver_alias(argvuser, local_data));
    		 	 else
					return(deliver(argvuser, local_data));
					}
    		else
				{
					/* Send a copy to our local postmaster */
					if ( (string_contains(postmaster, rmtuser) > 0) ||
						 (string_contains("Postmaster", rmtuser) > 0) ||
						 (string_contains("POSTMASTER", rmtuser) > 0) ||
						 (string_contains("daemon", rmtuser) > 0) ||
						 (string_contains("Daemon", rmtuser) > 0) ||
						 (string_contains("DAEMON", rmtuser) > 0) )
						 {
						 	doing_alias++;
						 	do_rmail(postmaster, local_data, NULL_DEVICE);
						 	doing_alias--;
						 	}
					return(forward(rmtsite, rmtuser, data, user, seq));
					}
			}

int
aliasp (name)
char *name;
{
	int retval;
	FILE *fd;
	char filename[256];
	retval = 0;
	if (strcmp(name, "") != 0)
		{
			sprintf(filename, "%s:%s", Alias, name);
			if (fd = fopen(filename, "r"))
			{
				retval = 1;
				fclose(fd);
				}
			}
	return(retval);
	}

int
deliver_alias (name, input)
char *name;
char *input;
{
	char filename[256];
	char newname[256];
	char in_line[256];
	char *to_tokenize;
	FILE *fd;
	int	retval;
	int err;
	retval = EXIT_OK;
	doing_alias = doing_alias + 1;
	sprintf(filename, "%s:%s", Alias, name);
	if (!(fd = fopen(filename, "r")))
		mlogit("Mail error can't open", filename);
	while (!feof(fd))
	{
		strcpy(in_line, "");
		fgets(in_line, 255, fd);
		if (in_line[strlen(in_line)-1] == '\n')
			in_line[strlen(in_line)-1] = '\0';
		if ((strcmp(in_line, "") != 0) && 
			(in_line[0] != '#'))
			{
				if (((sscanf(in_line, "%s", newname)) != 0) && 
						strcmp(newname, "") != 0)
					{
						err = do_rmail(newname, input, NULL_DEVICE);
						if (err != EXIT_OK)
							retval = err;
						}
				}
			}
	doing_alias = doing_alias - 1;
	fclose(fd);
	remove(input);
	FlushVol(NULL, 0);
	return(retval);
	}
		
		
int
forward(rmtsite, rmtuser, dfile, user, seq)
char *rmtsite, *rmtuser, *dfile, *user;
int seq;

{
    FILE *fbfile, *fcfile;
	char tmp_str[256];
    char bfile[256], cfile[256];
    char umbfile[256], umcfile[256];
    char umdfile[256];
    char rmtdfile[256], rmtxfile[256];
    char tmp[256];

    /* create a Bfile and Cfile. Dfile already exists  */
	strcpy(umdfile, unmunge_filename(dfile));
    sprintf(bfile, "B.%.30sa%04d", Myname, seq);
    strcpy(bfile, munge_filename(bfile));
    sprintf(cfile, "C.%.30sa%04d", rmtsite, seq);
    strcpy(cfile, munge_filename(cfile));
	sprintf(umbfile, "B.%.30sa%04d", Myname, seq);
    sprintf(umcfile, "C.%.30sa%04d", rmtsite, seq);

    sprintf(rmtxfile, "X.%.7sa%04d", Myname, seq);
    sprintf(rmtdfile, "D.%.7sa%04d", rmtsite, seq);

    /* The Bfile... */
    sprintf(tmp_str, "%s:%s", Spool, bfile);
    if ((fbfile = fopen(tmp_str, "wb")) == NULL) {
	perror("cannot open bfile for writing");
	return(EXIT_ERR);
    }
    fprintf(fbfile, "U %s %s\n", user, Myname);
    fprintf(fbfile, "F %s\n", rmtdfile);
    fprintf(fbfile, "I %s\n", rmtdfile);

    fprintf(fbfile, "C %s %s\n", MAILCMD, rmtuser);

    if (fclose(fbfile) == EOF) {
	printf("cannot close cfile\n");
	return(EXIT_ERR);
    }
    /* The Cfile... */

    sprintf(tmp_str, "%s:%s", Spool, cfile);
   	if ((fcfile = fopen(tmp_str, "wb")) == NULL) {
	perror("cannot open cfile for writing");
	return(EXIT_ERR);
    }
    fprintf(fcfile, "S %s %s %s - %s 0666\n", umdfile, rmtdfile, user, umdfile);
    fprintf(fcfile, "S %s %s %s - %s 0666\n", umbfile, rmtxfile, user, umbfile);

    if (fclose(fcfile) == EOF) {
	perror("cannot close fcfile\n");
	return(EXIT_ERR);
    }
    sprintf(tmp, "C %s %s", MAILCMD, rmtuser);
    mlogit("XQT QU'ED", tmp);
    return(EXIT_OK);
}

int
validate_site(name, errp)
char *name;
int errp;

{
    char tmp[NAMESIZE];
    FILE *fd;
    int found = 0;

    if ((fd = fopen(Sysfile, "r")) == NULL) {
	printf("cannot open %s\n", Sysfile);
	mlogit("FAIL", "can't read Sysfile");
	exit(EXIT_ERR);
    }
    while (fgets(tmp, sizeof(tmp), fd) != NULL)
	if (strncmp(tmp, name, strlen(name)) == 0) {
	    found = 1;
	    break;
	}
    if (fclose(fd)) {
	printf("can't close %s\n", Sysfile);
    }
    if (!found) 
    	{
			if (errp)
				{
					printf("%s -- unknown site\n", name);
					mlogit("FAIL", "bad site");
					}
			return(EXIT_ERR);
			}
	/* exit(EXIT_ERR);*/
    return(EXIT_OK);
}

 /* Deliver file using local mailer If mail fails, send to it to postmaster.
  * If that fails, give up and die */

int
deliver(addressee, data)
char *addressee;
char *data;

{
    int mail_err;
    char line[NAMESIZE];
    char tmp_str[256];
      /* Invoke the mailer */
    if (  ((mail_err = mail("mail", addressee, data)) != EXIT_OK) &&
          (strcmp(postmaster, addressee) == 0) )
          {
			sprintf(line, "%s %s %s", "mail", addressee, data);
			mlogit("FAILED", line);
			doing_alias++;
			if (do_rmail(postmaster, data, NULL_DEVICE) != EXIT_OK)
				/* mail("mail", postmaster, data) != EXIT_OK) */
				{
	    			doing_alias--;
	    			sprintf(line, "%s %s %s", "mail", postmaster, data);
	    			mlogit("FAILED", line);
	    			exit(EXIT_ERR);
					}
				else
    				doing_alias--;
    		}
    sprintf(line, "%s %s %s", "mail", addressee, data);
    mlogit("OK", line);
    if (remove(data) == 0)
			mlogit("DELETED", data);
    	else
			{
				mlogit("CAN'T DELETE", data);
				printf("\nErrno: %d\n", errno);
				}
	return(mail_err);
}

#define MAXOPENS 5

int
mail (command, addressee, input)
char *command, *addressee, *input;

{
    char line[256];
    char tmp_out[256];
    FILE *out;
    FILE *in;
    int c;
    int i;
    int status;
    long nread;
    char filebuf[256];
	sprintf(tmp_out, "%s:%s", Mail, addressee);
	for (i = 0; i< MAXOPENS; i++)
		{
			/* Some one else may have the mail file open */
			DEBUG(4, "Trying to open: %s\n", tmp_out);
			if ((out = fopen(tmp_out, "a")) != NULL)
			{
				break;
				}
			gnusleep(1);
		}
	if (out == NULL)
		{
			DEBUG(0, "Couldn't open: %s\n", tmp_out);
			return(EXIT_ERR);
			}
	if ((in = fopen(input, "r")) == NULL)
		{
			DEBUG(4, "Couldn't open: %s\n", input);
			fclose(out);
			return(EXIT_ERR);
			}
    /* while ((c = fgetc(in)) != EOF)
    	{
    		HandleEvents();
    		fputc(c, out);
    		} */
    while (!feof(in))
		{
			nread = fread(filebuf, sizeof(char), 255, in);
			HandleEvents();
			if (nread > 0)
				fwrite(filebuf, sizeof(char), nread, out);
			if (ferror(in))
				mlogit("Error reading file", input);
			if (ferror(out))
				mlogit("Error reading file", tmp_out);
			}

   	fputc(255, out);
    fputc('\r', out);
    fclose(in);
    fclose(out);
    return(EXIT_OK);
}

int
get_seq(dir, partial)
char *dir;
char *partial;
{
    /* pick a random num and make a unique file name from it /* FIXME: How do
     * real systems do this? What about ensuring uniqueness at the remote
     * site? */

    int i;
    int seq;
    char file[256];

    for (i = 0; i <= MAXTRIES; i++) {
	seq = (rand()*9999.0)/RAND_MAX;
	sprintf(file, "%s:%s%04d", dir, partial, seq);
	if (access(file, 0) != 0)	/* doesn't already exist */
	    return seq;
    }
	if (i == MAXTRIES) {
	    printf("can't generate unique file name\n");
	    exit(EXIT_ERR);
	}
}

int
do_rmail_forever ()
	{
		long i;
		EventRecord event;
		while (true)
			{
				do_rmail_queue();
				gnusleep(10);
				}
				
		}

int
do_rmail_queue ()
{
	char *work;
	FILE *fd;
	int len;
	char work_str[256];
	char user[256];
	char file[256];
	while (work_scan(NULL, "RMAIL"))
		{
			HandleEvents();
			work = work_next();
			sprintf(work_str,"%s:%s", Spool, work);
			fd = fopen(work_str, "r");
			if (fd == NULL)
				{
					mlogit("Couldn't open RMAIL work file", work_str);
					return(1);
					}
			/* fscanf(fd, "%s%s", user, file); */
			fgets(file, 255, fd);
			len = strlen(file);
			if (file[len-1] = '\n')
				{
					file[len-1] = '\0';
					}
			fgets(user, 255, fd);
			len = strlen(user);
			if (user[len-1] = '\n')
				{
					user[len-1] = '\0';
					}
			fclose(fd);
			fd = fopen(file, "r");
			if (fd == NULL)
				{
					mlogit("Couldn't open RMAIL input file", file);
					return(EXIT_ERR);
					}
			if (do_one_rmail(user, file, fd) == EXIT_OK)
				{
					fclose(fd);
					remove(work_str);
					remove(file);
					}
				else
					{
						fclose(fd);
						mlogit("Couldn't send mail for work file", work_str);
						return(EXIT_ERR);
						}
			
			}
		return(0);
	}
	

int
do_rmail (addressee, input, output)
char *addressee;
char *input;
char *output;
	{
		int argc;
		char *argv[5];
		argc = 3;
		argv[0] = "";
		argv[2] = addressee;
		argv[1] = input;
		/* argv[2] = output; */
		return(rmail(argc, argv));
		}
