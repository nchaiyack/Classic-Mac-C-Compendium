#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/nameser.h>
#include <resolv.h>

extern int errno;

extern char *inet_ntoa();
char *progname;
#define MAXDATA		256   /* really should get definition from named/db.h */
main(argc, argv)
	char **argv;
{
	register char *cp;
	char buf[BUFSIZ];
	char packet[PACKETSZ];
	char answer[PACKETSZ];
	struct rrec NewRR;
	char OldRRData[MAXDATA];
	int n;

	NewRR.r_data = (char *) malloc(MAXDATA);
	NewRR.r_data = (char *) malloc(MAXDATA);
	progname = argv[0];
	while (argc > 1 && argv[1][0] == '-') {
		argc--;
		cp = *++argv;
		while (*++cp)
			switch (*cp) {
			case 'i':
				_res.options |= RES_IGNTC;
				break;

			case 'v':
				_res.options |= RES_USEVC|RES_STAYOPEN;
				break;

			case 'r':
				_res.options &= ~RES_RECURSE;
				break;

			default:
				usage();
			}
	}
	for (;;) {
		fprintf(stderr,"> \n");
		if ((cp = (char *)gets(buf)) == NULL)
			break;
		switch (*cp++) {
		case 'a':
			n = res_mkquery(QUERY, cp, C_IN/*C_ANY*/, T_A, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

		case 'A':
			n = ntohl(inet_addr(cp));
			putlong(n, cp);
			n = res_mkquery(IQUERY, "", C_IN, T_A, cp, sizeof(long),
				NULL, packet, sizeof(packet));
			break;

		case 'f':
			n = res_mkquery(QUERY, cp, C_ANY, T_UINFO, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

		case 'g':
			n = res_mkquery(QUERY, cp, C_ANY, T_GID, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

		case 'G':
			*(int *)cp = htonl(atoi(cp));
			n = res_mkquery(IQUERY, "", C_ANY, T_GID, cp,
				sizeof(int), NULL, packet, sizeof(packet));
			break;

		case 'c':
			n = res_mkquery(QUERY, cp, C_IN, T_CNAME, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

		case 'h':
			n = res_mkquery(QUERY, cp, C_IN, T_HINFO, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

#ifdef	T_UNAME
		case 'H':
			n = res_mkquery(QUERY, cp, C_IN, T_UNAME, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

#endif	/* T_UNAME */
		case 'm':
			n = res_mkquery(QUERY, cp, C_IN, T_MX, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

		case 'M':
			n = res_mkquery(QUERY, cp, C_IN, T_MAILB, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

		case 'n':
			n = res_mkquery(QUERY, cp, C_IN, T_NS, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

		case 'p':
			n = res_mkquery(QUERY, cp, C_IN, T_PTR, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

		case 's':
			n = res_mkquery(QUERY, cp, C_IN, T_SOA, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

		case 'u':
			n = res_mkquery(QUERY, cp, C_ANY, T_UID, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

		case 'U':
			*(int *)cp = htonl(atoi(cp));
			n = res_mkquery(IQUERY, "", C_ANY, T_UID, cp,
				sizeof(int), NULL, packet, sizeof(packet));
			break;

		case 'x':
			n = res_mkquery(QUERY, cp, C_IN, T_AXFR, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

		case 'w':
			n = res_mkquery(QUERY, cp, C_IN, T_WKS, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

		case 'b':
			n = res_mkquery(QUERY, cp, C_IN, T_MB, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

		case 'B':
			n = res_mkquery(QUERY, cp, C_IN, T_MG, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

		case 'i':
			n = res_mkquery(QUERY, cp, C_IN, T_MINFO, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

		case 'r':
			n = res_mkquery(QUERY, cp, C_IN, T_MR, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

#ifdef	T_MP
		case '@':
			n = res_mkquery(QUERY, cp, C_IN, T_MP, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

#endif	/* T_MP */
		case '*':
			n = res_mkquery(QUERY, cp, C_IN, T_ANY, (char *)0, 0,
				NULL, packet, sizeof(packet));
			break;

#ifdef ALLOW_UPDATES
		case '^':
			{
			    char IType[10], TempStr[50];
			    int Type, oldnbytes, nbytes, i;
#ifdef ALLOW_T_UNSPEC
			    fprintf(stderr,"Data type (a = T_A, u = T_UNSPEC): \n");
			    gets(IType);
			    if (IType[0] == 'u') {
			    	Type = T_UNSPEC;
			    	fprintf(stderr,"How many data bytes? \n");
			    	gets(TempStr); /* Throw away CR */
			    	sscanf(TempStr, "%d", &nbytes);
			    	for (i = 0; i < nbytes; i++) {
			    		(NewRR.r_data)[i] = (char) i;
			    	}
			    } else {
#endif ALLOW_T_UNSPEC
			    	Type = T_A;
			    	nbytes = sizeof(u_long);
			    	fprintf(stderr,"Inet addr for new dname (e.g., 192.4.3.2): \n");
			    	gets(TempStr);
			    	putlong(ntohl(inet_addr(TempStr)), NewRR.r_data);
#ifdef ALLOW_T_UNSPEC
			    }
#endif ALLOW_T_UNSPEC
			    NewRR.r_class = C_IN;
			    NewRR.r_type = Type;
			    NewRR.r_size = nbytes;
			    NewRR.r_ttl = 99999999;
			    fprintf(stderr,"Add, modify, or modify all (a/m/M)? \n");
			    gets(TempStr);
			    if (TempStr[0] == 'a') {
			    	n = res_mkquery(UPDATEA, cp, C_IN, Type,
			    			OldRRData, nbytes,
			    			&NewRR, packet,
			    			sizeof(packet));
			    } else {
			    	if (TempStr[0] == 'm') {
			    	    fprintf(stderr,"How many data bytes in old RR? \n");
			    	    gets(TempStr); /* Throw away CR */
			    	    sscanf(TempStr, "%d", &oldnbytes);
				    for (i = 0; i < oldnbytes; i++) {
					    OldRRData[i] = (char) i;
				    }
					n = res_mkquery(UPDATEM, cp, C_IN, Type,
							OldRRData, oldnbytes,
							&NewRR, packet,
							sizeof(packet));
				} else { /* Modify all */
					n = res_mkquery(UPDATEMA, cp,
							C_IN, Type, NULL, 0,
							&NewRR, packet,
							sizeof(packet));

				}
			    }
			}
			break;

#ifdef ALLOW_T_UNSPEC
		case 'D':
			n = res_mkquery(UPDATEDA, cp, C_IN, T_UNSPEC, (char *)0,
					0, NULL, packet, sizeof(packet));
			break;

		case 'd':
			{
				char TempStr[100];
				int nbytes, i;
				fprintf(stderr,"How many data bytes in oldrr data? \n");
				gets(TempStr); /* Throw away CR */
				sscanf(TempStr, "%d", &nbytes);
				for (i = 0; i < nbytes; i++) {
					OldRRData[i] = (char) i;
				}
				n = res_mkquery(UPDATED, cp, C_IN, T_UNSPEC,
						OldRRData, nbytes, NULL, packet,
						sizeof(packet));
			}
			break;
#endif ALLOW_T_UNSPEC
#endif ALLOW_UPDATES

		default:
			fprintf(stderr,"a{host} - query  T_A\n");
			fprintf(stderr,"A{addr} - iquery T_A\n");
			fprintf(stderr,"b{user} - query  T_MB\n");
			fprintf(stderr,"B{user} - query  T_MG\n");
			fprintf(stderr,"f{host} - query  T_UINFO\n");
			fprintf(stderr,"g{host} - query  T_GID\n");
			fprintf(stderr,"G{gid}  - iquery T_GID\n");
			fprintf(stderr,"h{host} - query  T_HINFO\n");
#ifdef	T_UNAME
			fprintf(stderr,"H{host} - query  T_UNAME\n");
#endif	/* T_UNAME */
			fprintf(stderr,"i{host} - query  T_MINFO\n");
			fprintf(stderr,"p{host} - query  T_PTR\n");
			fprintf(stderr,"m{host} - query  T_MX\n");
			fprintf(stderr,"M{host} - query  T_MAILB\n");
			fprintf(stderr,"n{host} - query  T_NS\n");
			fprintf(stderr,"r{host} - query  T_MR\n");
			fprintf(stderr,"s{host} - query  T_SOA\n");
			fprintf(stderr,"u{host} - query  T_UID\n");
			fprintf(stderr,"U{uid}  - iquery T_UID\n");
			fprintf(stderr,"x{host} - query  T_AXFR\n");
			fprintf(stderr,"w{host} - query  T_WKS\n");
			fprintf(stderr,"c{host} - query  T_CNAME\n");
#ifdef	T_MP
			fprintf(stderr,"@{host} - query  T_MP\n");
#endif	/* T_MP */
			fprintf(stderr,"*{host} - query  T_ANY\n");
#ifdef ALLOW_UPDATES
			fprintf(stderr,"^{host} - add/mod/moda    (T_A/T_UNSPEC)\n");
#ifdef ALLOW_T_UNSPEC
			fprintf(stderr,"D{host} - deletea T_UNSPEC\n");
			fprintf(stderr,"d{host} - delete T_UNSPEC\n");
#endif ALLOW_T_UNSPEC
#endif ALLOW_UPDATES
			continue;
		}
		if (n < 0) {
			fprintf(stderr,"res_mkquery: buffer too small\n");
			continue;
		}
		fprintf(stderr,"SEND QUERY\n");
		fp_query(packet, stderr);
		n = res_send(packet, n, answer, sizeof(answer));
		if (n < 0)
			fprintf(stderr,"res_send: send error\n");
		else 
		{
			fprintf(stderr, "GOT ANSWER\n");
			fp_query(answer, stderr);
		}
	}
}

usage()
{
	fprintf(stderr, "Usage: %s [-v] [-i] [-r] [-d] [-p port] hostaddr\n",
		progname);
	exit(1);
}

