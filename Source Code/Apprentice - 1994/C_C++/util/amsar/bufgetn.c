/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/*
$Header: /afs/andrew.cmu.edu/usr13/aw0g/amsar/RCS/bufgetn.c,v 1.1 90/04/15 06:39:36 aw0g Exp Locker: aw0g $
$Source: /afs/andrew.cmu.edu/usr13/aw0g/amsar/RCS/bufgetn.c,v $
*/

#include <andyenv.h>
#include <ms.h>
#include <netinet/in.h>
#include <util.h>
#include <sys/stat.h>
#include <andrewos.h>		/* sys/file.h */
#include <sys/dir.h>
#include <pfio.h>

static void AnnounceBadDirFormat(line)
char           *line;
{
    char            ErrorText[100 + MAXPATHLEN];

    sprintf(ErrorText, "The folder '%s' is corrupted & needs reconstruction.", ap_Shorten(line));
    CriticalBizarreError(ErrorText);
}

/* Note in the following routine that numbers start a zero 
	and go to Dir->MessageCount -1 */
GetBufSnapshotByNumber(Dir, msgnum, snapshot,count)
struct MS_Directory *Dir;
char           *snapshot;
int             msgnum;
int	*count;
{
    int             readct;
    unsigned long int uLong;
    long read_size;
    long acount;

    if (msgnum >= Dir->MessageCount || (msgnum < 0)) {
	AMS_RETURN_ERRCODE(EINVAL, EIN_PARAMCHECK, EVIA_GETSNAPSHOTBYNUMBER);
    }
    *count=acount=(imin((*count),Dir->MessageCount-msgnum+1));

    if (Dir->fd < 0) {
	AMS_RETURN_ERRCODE(EMSDIRNOTOPEN, EIN_PARAMCHECK, EVIA_GETSNAPSHOTBYNUMBER);
    }
    if (lseek(Dir->fd, AMS_DIRHEADSIZE + (msgnum * AMS_SNAPSHOTSIZE), L_SET) < 0) {
	AMS_RETURN_ERRCODE(errno, EIN_LSEEK, EVIA_GETSNAPSHOTBYNUMBER);
    }
    read_size=AMS_SNAPSHOTSIZE*acount;
    readct = read(Dir->fd, snapshot,read_size);
    if (readct != read_size) {
	if (readct >= 0) {
	    AnnounceBadDirFormat(Dir->UNIXDir);
	    errno = EMSBADDIRFORMAT;
	}
	AMS_RETURN_ERRCODE(errno, EIN_READ, EVIA_GETSNAPSHOTBYNUMBER);
    }

    while(acount-- >0) {
      fixDate(AMS_DATE(snapshot));
      if (Dir->Writable) {
	AMS_SET_ATTRIBUTE(snapshot, AMS_ATT_MAYMODIFY);
      }
      else {
	AMS_UNSET_ATTRIBUTE(snapshot, AMS_ATT_MAYMODIFY);
      }
      if (msgnum < Dir->NumIDs) {
	bcopy(AMS_CHAIN(snapshot), &uLong, sizeof(unsigned long));
	Dir->IDs[msgnum].Chn = ntohl(uLong);
	bcopy(AMS_MIDHASH(snapshot), &uLong, sizeof(unsigned long));
	Dir->IDs[msgnum].midH = ntohl(uLong);
	bcopy(AMS_REPLYHASH(snapshot), &uLong, sizeof(unsigned long));
	Dir->IDs[msgnum].repH = ntohl(uLong);
      }
      snapshot+=AMS_SNAPSHOTSIZE;
    }
    return (0);
}

MS_GetBufNthSnapshot(DirName,n,SnapshotBuf,count)
char *DirName, *SnapshotBuf;
int n;
int *count;
{
    struct MS_Directory *Dir;
    long errsave;

    if (ReadOrFindMSDir(DirName, &Dir, MD_READ)) {
	return(mserrcode);
    }
    if (n == -1) {
	n = Dir->MessageCount - 1;
    }
    errsave = GetBufSnapshotByNumber(Dir, n, SnapshotBuf,count);
    CloseMSDir(Dir, MD_READ);
    return(errsave);
}
