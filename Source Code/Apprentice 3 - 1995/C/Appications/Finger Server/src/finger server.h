// finger server.h
// Darrell Anderson

/*

a program to receive and deal with a unix finger

listens for connections on port 79, when one comes in, it reads the username 
being fingered, and tries to open a file by that name in the system folder.

if it succeeds, it reports the contents of that file, otherwise reports 
"No such user"

*/

// accept a finger.  return 1 if it worked, 0 if anything bad happened.
int AcceptFinger(void);
