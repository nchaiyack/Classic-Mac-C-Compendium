
/* hostpaths.c */
int importpath(char *host, char *canon);
int exportpath(char *canon, char *host);
int cnvMac(char *upath, char *mpath);
void mapMacCaseness(char *mpath);
void unmapMacCaseness(char *mpath);
