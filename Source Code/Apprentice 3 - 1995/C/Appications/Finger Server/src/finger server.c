// finger server.c
// Darrell Anderson

#include "finger server.h"
#include "easy tcp structs.h"
#include "tcp easy.h"
//#include "tcp DLOGs.h"
#include "MacTCPCommonTypes.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

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
	printf("[starting finger server]\n");

	// initialize mactcp
	err = EasyTCPInit();
	if(err) {
		printf("[error opening MacTCP driver]\n");
		result = 0;
	}
	
	// keep acceptings fingers until something bad happens
	while(result) {
		result = AcceptFinger();
	}
	
	printf("\n[done.  Choose �Quit� from the �File� menu to exit]\n");
}

#define LINESIZE 255

int AcceptFinger(void) {
	HostNPort hostNport;
	StreamPtr stream;
	int result, amtRead, i;
	char lineoftext[LINESIZE];
	OSErr err;
	FILE *plan;
	
	// prep the port number to the finger port
	hostNport.port = 79;
	
	// create a stream
	err = EasyTCPCreate(&stream, DEFAULT_TIMEOUT, DEFAULT_BUFFERSIZE );
	if(err != noErr) return(0);
	
	printf("[listening for a connection on port 79]\n");
	
	// wait indefinitely for a the connection
	err = EasyTCPOpen( stream, &hostNport, PASSIVE, 32767 );
	if(err != noErr) {
		printf("[error opening connection]\n");
		err = EasyTCPRelease(stream, DEFAULT_TIMEOUT);
		return(0);
	}
		
	// read the username being fingered
	err = EasyTCPReceive( stream, (Ptr)lineoftext, LINESIZE, &amtRead, DEFAULT_TIMEOUT );
	if(err != noErr) return(0);
	for( i=0 ; i<amtRead ; i++ ) 
		if( lineoftext[i]=='\n' || lineoftext[i]=='\r' )
			lineoftext[i] = '\0';
	printf("�%s� fingered.\n",lineoftext);
	
	// try to open an appropriate plan file
	plan = fopen(lineoftext,"r");

	// if there's a problem opening the file, say so
	if( !plan ) {
		err = EasyTCPSend( stream, "No such user\n\r", 14, DEFAULT_TIMEOUT );
		if(err != noErr) return(0);
	} else {
		
		// otherwise send back the plan file's text
		while( !feof(plan) ){
			fgets(lineoftext,LINESIZE,plan);
			err = EasyTCPSend( stream, lineoftext, strlen(lineoftext), DEFAULT_TIMEOUT );
			if(err != noErr) return(0);
		}
	}
	
	// close the connection
	err = EasyTCPClose( stream, DEFAULT_TIMEOUT );
	if(err != noErr) return(0);
	
//	printf("[closed connection]\n");

	// release the stream
	err = EasyTCPRelease( stream, DEFAULT_TIMEOUT );
	if(err != noErr) return(0);

	// if the user was "quit" then exit the program
	if( !strncmp(lineoftext,"quit",4) ) {
		printf("[exiting]\n");
		exit(0);
	}

	return(1);
}