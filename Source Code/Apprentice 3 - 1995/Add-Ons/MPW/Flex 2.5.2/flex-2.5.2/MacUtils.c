/***
 *
 * MacUtils.c - by Christopher E. Hyde, 95-06-29
 *
 ***/

#include	<stdio.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<ioctl.h>

#include	<Memory.h>
#include	<Resources.h>


int
isatty (int fd)
{
	return ioctl(fd, FIOINTERACTIVE, NULL) == 0;
}


typedef Boolean bool;
typedef struct {
	Handle	data;
	Size	size;
} CSkel;

static CSkel aSkel = { nil, 0 };


bool
HGets (CSkel* this, char* buf)
{
	char* s;
	if (skel_ind >= this->size) {
		HPurge(this->data);
		return false;
	}

//	if (!*this->data)
		LoadResource(this->data);
	s = &(*this->data)[skel_ind];
	do {
		++skel_ind;
	} while ((*buf++ = *s++) != '\n');
	*buf = '\0';

	return true;
}


/* skelout - write out one section of the skeleton file
 *
 * Description
 *    Copies skelfile or skel array to stdout until a line beginning with
 *    "%%" or EOF is found.
 */
void
skelout (void)
{
	char	buf[MAXLINE];
	bool	do_copy = true;

		// Loop pulling lines either from the skelfile,
		// if we're using one, or from the skel[] array.

	if (!skelfile && skel_ind == 0) {
		aSkel.data = Get1Resource('TEXT', 128);
		aSkel.size = GetHandleSize(aSkel.data);
	}

	while (skelfile ? (fgets(buf, MAXLINE, skelfile) != NULL)
					: HGets(&aSkel, buf)) {	/* copy from skel array */
		if (buf[0] == '%') {	/* control line */
			switch ( buf[1] ) {
				case '%':	return;
				case '+':	do_copy = C_plus_plus;			break;
				case '-':	do_copy = !C_plus_plus;			break;
				case '*':	do_copy = true;					break;
				default:	flexfatal( _( "bad line in skeleton file" ) );
			}
		} else if (do_copy)
			out(buf);
	}
}
