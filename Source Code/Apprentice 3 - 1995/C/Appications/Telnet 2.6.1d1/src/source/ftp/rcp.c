#if 0

#define ga()  while (!netwrite(rsnum,"",1)) netsleep(0)

/************************************************************************/
/*  unsetrshd
*   remove the acceptance of rshd calls (rcp)
*/
unsetrshd()
	{
	netclose(rsnum);
	rsnum = -1;
	rcpenable = 0;
}

/************************************************************************/

setrshd()
{
	int i;
/*
*  set up to receive a rsh call connection 
*/
	if (rsnum >= 0)
		return(0);
	curstate = 199;					/* waiting for connection */
	i = netsegsize(RCPSEGSIZE);
	rsnum = netlisten(HRSHD,0);		/* BYU 2.4.15 */
	netsegsize(i);
	if (rsnum >= 0)
		Sptypes[rsnum] = PRCP;

	rcpenable = 1;
}


/************************************************************************/
/*  rshell
*   take an incoming rshell request and service it.  Designed to handle
*   rcp primarily.
*/
rshd(code)
	int code;
	{
	int i,j;

	if (!rcpenable)
		return(0);

	switch (curstate) {
		case 199:					/* wait to get started */
			if (code != CONOPEN)
				break;

			curstate = 0;
			netputuev(SCLASS,RCPACT,rsnum);		/* keep us alive */

			break;
			
/*
* in effect, this is a subroutine that captures network traffic while
* waiting for a specific character to be received
*/
		case 50:
			while (0 < (cnt = netread(rsnum,&xs[waitpos],1))) {
				if (xs[waitpos] == waitchar) {
					curstate = retstate;
					netputuev(SCLASS,RCPACT,rsnum);		/* keep us alive */
					break;
				}
				else 
					waitpos += cnt;
			}
			netpush(rsnum);
			break;

		case 51:				/* for recursion, passes straight through */
			break;

		case 0:					/* waiting for first string */
			retstate = 1;
			curstate = 50;
			waitchar = 0;
			waitpos = 0;
			netputuev(SCLASS,RCPACT,rsnum);		/* keep us alive */
			break;

		case 1:					/* we have received stderr port number */
			i = atoi(xs);		/* port number */
			curstate = 51;
#ifdef notneeded
/*
*  caution, netrespond calls netsleep()
*  which will call this routine
*  careful with the synchronicity!
*/
			if (i)		/* zero means, don't bother */
				rserr = netrespond(i,rsnum,1);	/* respond to rsh */
			else
#else
			if (i) {
				cnt = -1;		/* abort it all, we don't take rsh */
				break;
			}
			else
#endif
				rserr = -1;

			retstate = 2; curstate = 50;
			waitpos = 0; waitchar = 0;
			break;

		case 2:				/* get user name, my machine */
			strncpy(myuser,xs,16);

			retstate = 3; curstate = 50;
			waitpos = 0; waitchar = 0;
			break;

		case 3: 			/* get user name, his machine */
			strncpy(hisuser,xs,16);
/*			ftransinfo(hisuser); */

			retstate = 4; curstate = 50;
			waitchar = 0; waitpos = 0;

			break;

		case 4:
/*			ftransinfo(xs);*/
/*
* ACK receipt of command line
*/
			if (rserr >= 0)
				netwrite(rserr,&xp,1);		/* send null byte */
			else {
				ga();			/* send NULL on main connection */
			}

			if (!strncmp(xs,"rcp ",4)) {
/*
*  rcp will be using wildcards, target must be a directory
*/
				if (!strncmp(&xs[4],"-d -t",5)) {
					strncpy(pathname,&xs[10],PATHLEN);
					if (direxist(pathname)) {
/*						ftransinfo("no directory by that name ");*/
						netwrite(rsnum,"\001 No dir found ",16);
						netpush(rsnum);
						cnt = -1;
						break;
					}

					isdir = 1;
					retstate = 20; curstate = 50;
					waitchar = '\012'; waitpos = 0;

					ga();		/* ready for them to start */
					break;
				}
/*
* target could be a directory or a complete file spec
*/
				if (!strncmp(&xs[4],"-t",2)) {
					strncpy(pathname,&xs[7],PATHLEN);
					if (!direxist(pathname)) 
						isdir = 1;
					else
						isdir = 0;

					retstate = 20 ; curstate = 50;
					waitchar = '\012'; waitpos = 0;

					ga();			/* ready for rcp to start */
					break;
				}
/*
*  rcp is requesting me to transfer file(s) (or giving directory name)
*/
				if (!strncmp(&xs[4],"-f",2)) {
					strncpy(pathname,&xs[7],PATHLEN);

/*
*  direxist returns whether the path spec refers to a directory, and if
*  it does, prepares it as a prefix.  Therefore, if it is a dir, we append
*  a '*' to it to wildcard all members of the directory.
*  Firstname() takes a file spec (with wildcards) and returns a pointer
*  to a prepared ACTUAL file name.  nextname() returns successive ACTUAL
*  filenames based on firstname().
*/
					if (!direxist(pathname)) {
						i = strlen(pathname);
						pathname[i] = '*';		/* all members of directory*/
						pathname[++i] = '\0';
					}
					nextfile = firstname(pathname);

					if (nextfile == NULL) {
/*						ftransinfo(" file or directory not found ");*/
						netwrite(rsnum,"\001 File not found ",18);
						netpush(rsnum);
						cnt = -1;
					}
					else {
						/* wait for other side to be ready */
						retstate = 30;	curstate = 50;
						waitchar = 0; waitpos = 0;
					}
					break;
				}
			}

			break;

		case 20:
			xs[waitpos] = '\0';		/* add terminator */

/*
*  get working values from command line just received
*  open file for receive
*/
			if (xs[0] != 'C' || xs[5] != ' ') {
/*				ftransinfo(" Cannot parse filename line "); */
				netwrite(rsnum,"\001 Problem with file name ",26);
				cnt = -1;
				break;
			}

			filelen = atol(&xs[6]);

			for (i = 6; xs[i] != ' '; i++) 
				if (!xs[i]) {
/*					ftransinfo(" premature EOL ");*/
					netwrite(rsnum,"\001 Problem with file name ",26);
					cnt = -1;
					break;
				}

			strcpy(newfile,pathname);		/* path spec for file */

			if (isdir)						/* add file name for wildcards */
				strcat(newfile,&xs[++i]);

			if (0 > (fh = creat(newfile,O_RAW))) {
				netwrite(rsnum,"\001 Cannot open file for write ",29);
				cnt = -1;
				break;
			}
			netputevent(USERCLASS,RCPBEGIN,-1);
			ga();							/* start sending the file to me */
			xp = len = 0;
			curstate = 21;					/* receive file, fall through */
			break;

		case 21:
			do {
			/* wait until xs is full before writing to disk */
				if (len <= 0) {
					if (xp) {
						write(fh,xs,xp);
						xp = 0;
					}
					if (filelen > (long)BUFFERS)
						len = BUFFERS;
					else
						len = (int)filelen;
				}

				cnt = netread(rsnum,&xs[xp],len);

				filelen -= (long)cnt;
				len -= cnt;
				xp += cnt;

/*				printf(" %ld %d %d %d ",filelen,len,xp,cnt);
				n_row(); putln(""); */

				if (filelen <= 0L || cnt < 0) {
					write(fh,xs,xp);		/* write last block */
					close(fh);
					fh = 0;
					
					/* wait for NULL byte at end after closing file */
					curstate = 50;  retstate = 22;
					waitchar = 0;   waitpos = 0;
					break;
				}

			} while (cnt > 0);
			break;

		case 22:
			/* cause next sequence of bytes to be saved as next filename
				to transfer     */
			ga();			/* tell other side, I am ready */
			waitchar = '\012'; waitpos = 0;
			curstate = 50; retstate = 20;
			break;

/*
*  transfer file(s) to the sun via rcp
*/
		case 30:
			if (0 > (fh = open(nextfile,O_RAW))) {
				netwrite(rsnum,"\001 File not found ",19);
/*				ftransinfo("Cannot open file to transfer: ");
				ftransinfo(nextfile); */
				cnt = -1;
				break;
			}
			netputevent(USERCLASS,RCPBEGIN,-1);
			filelen = lseek(fh,0L,(short)2);	/* how long is file? */
			lseek(fh,0L,0);				/* back to beginning */

			for (i=0,j=-1; nextfile[i] ; i++)
				if (nextfile[i] == '\\')
					j = i;

			sprintf(xs,"C0755 %lu %s\012",filelen,&nextfile[j+1]);
			netwrite(rsnum,xs,strlen(xs));	/* send info to other side */

/*			ftransinfo(xs);					 check it */

			retstate = 31; curstate = 50;
			waitchar = 0;  waitpos = 0;

			towrite = xp = 0;
			break;

		case 31:
/*
*   we are in the process of sending the file 
*/
			netputuev(SCLASS,RCPACT,rsnum);		/* keep us alive */

			if (towrite <= xp) {
				towrite = read(fh,xs,BUFFERS);
				xp = 0;
				filelen -= (long)towrite;
			}
			i = netwrite(rsnum,&xs[xp],towrite-xp);
			if (i > 0)
				xp += i;

/*			printf(" %d %d %d %ld\012",i,xp,towrite,filelen);
			n_row();
*/
/*
*  done if:  the file is all read from disk and all sent
*  or other side has ruined connection
*/
			if ((filelen <= 0L && xp >= towrite) || netest(rsnum)) {
				close(fh);
				fh = 0;
				nextfile = nextname();		/* case of wildcards */
				ga(); 
				netputuev(SCLASS,RCPACT,rsnum);
				if (nextfile == NULL)
					retstate = 32;
				else
					retstate = 30;
				curstate = 50;
				waitchar = 0;	waitpos = 0;
			}
			break;
		case 32:
			cnt = -1;
			break;
		case 5:
			break;
		default:
			break;

	}

/*
*  after reading from connection, if the connection is closed,
*  reset up shop.
*/
	if (cnt < 0) {
		if (fh > 0) {
			close(fh);
			fh = 0;
		}
		curstate = 5;
		cnt = 0;
		netclose(rsnum);
		rsnum = -1;
		netputevent(USERCLASS,RCPEND,-1);

		setrshd();					/* reset for next transfer */
	}


}

#endif
