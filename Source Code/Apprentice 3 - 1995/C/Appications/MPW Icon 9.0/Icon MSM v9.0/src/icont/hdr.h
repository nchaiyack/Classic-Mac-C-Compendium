/*
 *  Header file for MPW Icon is a MPW Shell Script to invoke iconx.
 */
static unsigned char iconxhdr[MaxHdr+1] =
	"\"{icon}iconx\" \"{Command}\" {\"Parameters\"} || Exit {Status}\n"
	"Exit {Status}\n";
