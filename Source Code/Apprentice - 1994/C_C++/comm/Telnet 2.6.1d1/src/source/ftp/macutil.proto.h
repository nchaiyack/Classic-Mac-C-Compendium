void GetDirectoryName(short vRefNum, long dirID, Str32 name);
short VolumeNameToRefNum(Str32 volumeName);
void PathNameFromDirID(long dirID, short vRefNum, StringPtr fullPathName);
short ChangeDirectory(long *dirID, short *vRefNum,char *pathname);

short wccheck(char *file, char *template);
char *firstname(char *spec, long dirID, short vRefNum, CInfoPBRec *finfo);
char *nextname(char *spec, CInfoPBRec *finfo);
