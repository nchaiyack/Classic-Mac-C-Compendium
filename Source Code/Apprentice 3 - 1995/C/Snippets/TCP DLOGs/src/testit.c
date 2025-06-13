// testit.c
// Darrell Anderson

/*
run the rest of the code through some hoops..
*/

#include "tcp DLOGs.h"

void main(void) {
	short result;
	UserNPass userNpass;
	HostNPort hostNport;
	
	char *hostname = "specified hostname";
	short port = 99;
	char *username = "specified username";

	// initialize the toolbox..
	InitGraf( &qd.thePort );
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( nil );
	InitCursor();

	hostNport.hostname[0] = '\0';
	hostNport.port = 0;
	userNpass.username[0] = '\0';

	// test the code
	result = QueryHostNPort( &hostNport );	
	if( result ) SysBeep(0);
	
	result = QueryUserNPass( &userNpass );
	if( result ) SysBeep(0);

	result = QueryUserNHost(&userNpass, &hostNport);
	if( result ) SysBeep(0);

	result = QueryFailedAttempt();
	if( result ) SysBeep(0);
}