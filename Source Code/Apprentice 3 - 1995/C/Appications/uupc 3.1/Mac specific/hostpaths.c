#ifdef THINK_C
# include "unixlibproto.h"
#endif THINK_C

#include	"host.h"
#ifndef NULL
#define NULL 0L
#endif

#include "hostpaths.proto.h"

/* canonical name conversion routines

	importpath	canonical -> host
	exportpath	host -> canonical

	host		your local pathname format
	canonical	unix style
*/

importpath(char *host, char *canon)
{
	int i;
	strcpy(host, canon);
}

exportpath(char *canon, char *host)
{
	/* copy the string replacing ":" by "/" */
	register	char *h = host,
					 *c = canon;
						
	while (*c = *h++) {
		if (*c == ':') *c = '/';
		c++;
	}
}

/* converts Unix path to Mac path syntax */
/* Permit full Mac-style "diskname:dir:dir:...:file" specifications... dplatt */
cnvMac (char *upath, char *mpath)
{
	register	char	*u = upath,
						*m = mpath;
	int			vRef, len;
	
	if (*u == SEPCHAR) {
		/* put volume name on the front */
		(void)GetVol((StringPtr)m, &vRef);
		len = (int)(m[0]);
		PtoCstr(m);
		m += len;
	}
	else if (strchr(u, DIRCHAR) == NULL) {
		*m++ = ':';
	}
	
	while ( *m = *u++ ) {
		if (*m == SEPCHAR) *m = ':';
		m++;
	}
}

/*
   Do ugly character-remapping on Mac files to avoid collisions between
   uppercase and lowercase letters.  Gaak.  Letters in the A-Z range are
   mapped up into a range of accented lowercase letters.
   
   Handle names of the sort "L.xxxx" or ":L.xxxx" (where L is any capital letter)
   on the assumption that these are spool-directory message files.  Leave other file
   names unbashed, for lack of a more universal solution to the problem.
   
   Fixed reverse-mapping code for 3.1b30 so that it doesn't blow its mind if some
   program has dropped in a file whose name wasn't character-mapped.  Older versions
   became seriously dazed and confused.
*/

void mapMacCaseness(char *mpath)
{
  char *c;
  if (mpath[0] == DIRCHAR) {
  	c = mpath + 1;
  } else if (strchr(mpath, DIRCHAR) == NULL) {
  	c = mpath;
  } else {
  	return;
  }
  if (c[1] != '.' || c[0] < 'A' || c[1] > 'Z') {
    return;
  }
  c += 2;
  while (*c) {
    if (*c >= 'A' && *c <= 'Z') {
      *c += 0x46;
    }
    c++;
  }
}

void unmapMacCaseness(char *mpath)
{
  char *c, cc;
  if (mpath[0] == DIRCHAR) {
  	c = mpath + 1;
  } else if (strchr(mpath, DIRCHAR) == NULL) {
  	c = mpath;
  } else {
  	return;
  }
  if (c[1] != '.' || c[0] < 'A' || c[1] > 'Z') {
    return;
  }
  c += 2;
  while (cc = *c) {
  	if (cc >= 'A' + 0x46 && cc <= 'Z' + 0x46) {
      *c = cc - 0x46;
    }
    c++;
  }
}



