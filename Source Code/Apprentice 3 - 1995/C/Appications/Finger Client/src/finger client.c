// finger client.c
// Darrell Anderson

#include "finger client.h"
#include "easy tcp structs.h"
#include "tcp easy.h"
#include "tcp DLOGs.h"
#include "MacTCPCommonTypes.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

void main(void) {
	int result = 1;
	OSErr err;
	
	// initialize the toolbox
	InitGraf( &qd.thePort ); 
	InitFonts(); 
	InitWindows();
	InitMenus(); 
	TEInit(); 
	InitDialogs( nil ); 
	InitCursor();

	// open a console window
	printf("[starting finger]\n");

	// initialize mactcp
	err = EasyTCPInit();
	if(err) {
		printf("[error opening MacTCP driver]\n");
		result = 0;
	}
	
	// keep fingering until the user hits "Cancel"
	while(result) {
		result = Finger();
	}
	
	printf("\n[done.  Choose �Quit� from the �File� menu to exit]\n");
}

#define LINESIZE 255

int Finger(void) {
	// initially user and host are empty, but remembers last value between calls
	static HostNPort hostNport;
	static UserNPass userNpass;
	StreamPtr stream;
	int result, amtRead, i;
	char lineoftext[LINESIZE];
	OSErr err;

	// get username and hostname info
	result = QueryUserNHost( &userNpass, &hostNport );
	if( result == 0 ) return(0);
	
//	printf("[preparing to finger %s@%s]\n", userNpass.username, hostNport.hostname);
	
	// prep the port number to the finger port
	hostNport.port = 79;
	
	// create a stream
	err = EasyTCPCreate(&stream, DEFAULT_TIMEOUT, DEFAULT_BUFFERSIZE );
	if(err != noErr) return(0);
	
	// open the connection
	err = EasyTCPOpen( stream, &hostNport, ACTIVE, DEFAULT_TIMEOUT );
	if(err != noErr) {
		printf("[error opening connection]\n");
		err = EasyTCPRelease(stream, DEFAULT_TIMEOUT);
		return(0);
	}
	
	printf("\n[%s]:\n",hostNport.hostname);
	
	// send the username over the connection
	err = EasyTCPSend( stream, userNpass.username, strlen(userNpass.username), DEFAULT_TIMEOUT );
	if(err != noErr) return(0);
	err = EasyTCPSend( stream, "\n\r", 2, DEFAULT_TIMEOUT );
	if(err != noErr) return(0);

//	printf("[sent username]\n");

	// read until all the data's here
	do {
		err = EasyTCPReceive( stream, (Ptr)lineoftext, LINESIZE, &amtRead, DEFAULT_TIMEOUT );
		if( !err ) {
			for( i=0 ; i<amtRead ; i++ ) {
				if( isprint(lineoftext[i]) || lineoftext[i]=='\n' || lineoftext[i]=='\t' )
					printf("%c",lineoftext[i]);	
				//else printf("{\%2d}",(int)lineoftext[i]);
			}
		}
	} while( !err );
	
	// close the connection
	err = EasyTCPClose( stream, DEFAULT_TIMEOUT );
	if(err != noErr) return(0);
	
//	printf("[closed connection]\n");

	// release the stream
	err = EasyTCPRelease( stream, DEFAULT_TIMEOUT );
	if(err != noErr) return(0);

	return(1);
}