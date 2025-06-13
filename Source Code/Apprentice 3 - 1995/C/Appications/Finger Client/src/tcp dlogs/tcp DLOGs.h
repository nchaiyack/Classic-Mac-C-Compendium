// tcp DLOGs.h
// Darrell Anderson

/*

tcp DLOGs is a set of routines to interact with the user to get tcp
related information such as hostname & port, username & password, and
username & host combinations.  

Note that it doesn't _do_ anything with this information, just provides
a nice interface for the programmer who wants to ask for it at runtime.

Note: the Toolbox _must_ be initialized for these routines to work!
(a la: 	
	InitGraf( &qd.thePort ); InitFonts(); InitWindows();
	InitMenus(); TEInit(); InitDialogs( nil ); InitCursor();
)

Also Note: Assumes the presence of "tcp_DLOGs.rsrc" in the project
*/

#ifndef __tcp_dlogs__
#define __tcp_dlogs__

#include "easy tcp structs.h"

// resource id's for DLOGs, DITLs, cicn's, ICONs, associated with each.
#define ok 1
#define cancel 2
#define kHostNPortRSRCid 10128
#define kHost 3
#define kPort 4
#define kUserNPassRSRCid 11128
#define kUsername 3
#define kVis 4
#define kInvis 5
#define kUhOhRSRCid 12128
#define kUserNHostRSRCid 13128
#define kUNHuser 3
#define kUNHhost 4

// query the user for a hostname and a port number.
// takes a HostNPort struct, uses it's values as defaults,
//  and fills it in with entered data.
// returns 1 if the user hit OK, 0 if Cancel
int QueryHostNPort( HostNPort *data );

// query the user for a username and password.
// note that the password is displayed using ¥'s, and not cleartext.
// takes a UserNPass struct, uses it's values as defaults,
// and fills it in with the entered data.
// returns 1 if the user hit OK, 0 if Cancel
int QueryUserNPass( UserNPass *data );

// query for a username and a host
// takes a UserNPass for the username, and a HostNPort for a hostname.
// it uses the username and hostname values as defaults and fills them
// in with the entered data.
// returns 1 if the user hit OK, 0 if Cancel
int QueryUserNHost( UserNPass *user, HostNPort *host );

// display a "sorry you messed up" dialog, 
// returns 1 if the user wants to retry, 0 on Cancel.
int QueryFailedAttempt( void );

#endif

