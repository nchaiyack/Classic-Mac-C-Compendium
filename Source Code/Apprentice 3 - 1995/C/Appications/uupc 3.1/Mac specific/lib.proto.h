
/* lib.c */
int MKDIR(char *path);
int CHDIR(char *path);
int OPEN(char *name, int mode);
FILE *FOPEN(char *name, char *mode, char ftype);
int CREAT(char *name, int mode, char ftyp);
int UNLINK(char *path);
int getargs(char *line, char **flds);
