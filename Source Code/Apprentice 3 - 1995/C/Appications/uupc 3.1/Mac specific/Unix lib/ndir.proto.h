
/* ndir.c */
DIR *opendir(char *name);
struct direct *readdir(DIR *dirp);
void closedir(DIR *dirp);
