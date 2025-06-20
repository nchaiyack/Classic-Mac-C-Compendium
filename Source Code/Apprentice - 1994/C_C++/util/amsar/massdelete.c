/*
 * $Header: /afs/andrew.cmu.edu/usr13/aw0g/amsar/RCS/massdelete.c,v 1.1 90/04/30 17:41:52 aw0g Exp $
 */

#include <andrewos.h>		/* sys/file.h */
#include <ms.h>
#include <errprntf.h>
#include <sys/stat.h>

extern char *sys_errlist[];

static void loose_files(dirname,deleteme,deletecount)
char *dirname;
char *deleteme;
int deletecount;
{
  char filename[MAXPATHLEN+1];
  int failed_count=0;
  while(deletecount-- >0) {
    sprintf(filename,"%s/+%s",dirname,AMS_ID(deleteme));
    if(unlink(filename)!=0)
      failed_count++;
    deleteme+=AMS_SNAPSHOTSIZE;
  }

  if(failed_count==0)
    return;
  sprintf(filename,"couldn't unlink %d messages because:%s",failed_count,sys_errlist[errno]);
  BizarreError(filename,ERR_WARNING);
}

/*
 * MS_MassDeleteSnapshots - delete the passed snapshots
 */
int MS_MassDeleteSnapshots(dirname,deleteme,deletecount)
char *dirname;
char *deleteme;
int deletecount;
{
  struct MS_Directory *Dir;
  int errsave;
  char TmpFileName[MAXPATHLEN+1];
  char body_path[MAXPATHLEN+1];
  int new_MessageCount=0;
  int wfd=0;
  int savefd=0;
  int msgnum;
  TmpFileName[0]=0;

  CloseDirsThatNeedIt();

  /*
   * need to do this first to make space in case directory is
   * totaly full there would be no room otherwise
   */
  loose_files(dirname,deleteme,deletecount);

  if (ReadOrFindMSDir(dirname,&Dir,MD_APPEND))
    goto error_exit;

  sprintf(TmpFileName, "%s/%s.p", dirname, MS_DIRNAME);
  if((wfd=open(TmpFileName, O_RDWR | O_CREAT | O_TRUNC, 0644))<0)
    goto error_exit;
  if (osi_ExclusiveLockNoBlock(wfd))
    goto error_exit;
  if(lseek(Dir->fd, 0, L_SET) < 0)
    goto error_exit;
  {
    char HeadDum[AMS_DIRHEADSIZE];
    if (read(Dir->fd, HeadDum, AMS_DIRHEADSIZE) != AMS_DIRHEADSIZE)
      goto error_exit;
    if (writeall(wfd, HeadDum, AMS_DIRHEADSIZE) != AMS_DIRHEADSIZE)
      goto error_exit;
  }

  for (msgnum = 0; msgnum < Dir->MessageCount; ++msgnum) {
    char SnapDum[AMS_SNAPSHOTSIZE];
    struct stat stat_buf;
    if (read(Dir->fd, SnapDum, AMS_SNAPSHOTSIZE) != AMS_SNAPSHOTSIZE)
      goto error_exit;
    sprintf(body_path, "%s/+%s",dirname,AMS_ID(SnapDum));
    if(lstat(body_path,&stat_buf)!=0) { /*remove the captions for files that don't exist*/
      if(vdown(errno))
	goto error_exit;
    } else if (writeall(wfd, SnapDum, AMS_SNAPSHOTSIZE) != AMS_SNAPSHOTSIZE)
      goto error_exit;
    else
      new_MessageCount++;
  }

  Dir->MessageCount = new_MessageCount;

  /* Start to make the new file be the one pointed to by the Dir structure. */
  savefd = Dir->fd;
  Dir->fd = wfd;
  wfd=0;
  if (DestructivelyWriteDirectoryHead(Dir))
    goto error_exit;

  if (CloseMSDir(Dir, MD_APPEND))
    goto error_exit;

  sprintf(body_path,"%s/%s",dirname,MS_DIRNAME);
  if (RenameEvenInVice(TmpFileName,body_path))
    goto error_exit;

  if(savefd>0)
    close(savefd);
  return 0;

error_exit:
  errsave = errno;
  if(TmpFileName[0]!=0)
    unlink(TmpFileName);
  if(wfd>0)
    close(wfd);
  if(savefd>0)
    close(savefd);
  CloseMSDir(Dir, MD_APPEND);
  AMS_RETURN_ERRCODE(errsave, EIN_RENAME, EVIA_PURGEDELETEDMESSAGES);
}
