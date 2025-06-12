/* News.c */
/*****************************************************************************/
/*                                                                           */
/*    Offline USENET News Dump Generator                                     */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This software is Public Domain; it may be used for any purpose         */
/*    whatsoever without restriction.                                        */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/
/* gcc -g -Wall -ansi -pedantic -o newsdump NewsDump.c */
/* gcc -o newsdump NewsDump.c */

/* these settings can be changed */
#define MAXBYTESPERSECOND (2048) /* don't transfer more than this per second */
#define SERVERCMDWAIT (2) /* seconds of delay per NNTP command */
#define NEWSFILENAME ".newslist" /* name of news file */
#define NEWSSERVERNAME "spool.cs.wisc.edu" /* name of news server */
#define SPOOLDIRPATH "/usr/spool/news/" /* path to the news spool directory */
#define USENNTP (1) /* 0 = use /usr/spool/news, 1 = use NNTP server */

/* This program is a non-interactive news reader which can use either an */
/* NNTP server or a news spool directory hierarchy to obtain USENET */
/* articles.  A file ".newslist" in the home directory remembers the */
/* number of the last article read for each subscribed group.  Articles */
/* fetched by the program are written to standard output.  The format of */
/* the articles in the stream is as follows: */
/*   - newsgroup name on a line by itself */
/*   - as many lines as needed for the complete article */
/*   - a line containing a single period */
/* If there is a line with a single period inside of the article, then it */
/* is stored as a double period.  Articles are always written in ascending */
/* order of article index, which should also be chronological order. */
/* Usage:  */
/*   - subscribing to a group */
/*       newsdump +<groupname> */
/*     this will not fetch any articles, but only update the .newslist file */
/*     note that no articles from this group will be fetched the next time */
/*     newsdump is run with no arguments.  It must be run another time */
/*     after that in order to get articles from the group.  Also, an added */
/*     group is assumed to be up to date.  Of course, the .newslist file is */
/*     in editable format, so the article index can be adjusted by hand. */
/*   - unsubscribing from a group */
/*       newsdump -<groupname> */
/*     this will not fetch any articles, but only update the .newslist file */
/*   - fetching new articles */
/*       newsdump */
/*     (no arguments.)  this will fetch all available articles, write them */
/*     to standard output, and update the .newslist file. */
/*   - catching up all groups */
/*       newsdump zap */
/*     this will update the article counter for each group to be the most */
/*     recent article, effectively discarding any new news. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#if defined(sparc)
#include <varargs.h>
#else
#include <stdarg.h>
#endif

#if USENNTP
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#else
#include <dirent.h>
#endif

/* stupidity for SunOS */
#ifndef EXIT_FAILURE
#define EXIT_FAILURE (1)
#endif
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS (0)
#endif

#define True (0 == 0)
#define False (0 != 0)
typedef int Bool;
#define strequ(left,right) (0 == strcmp(left,right))
#define MAXSERVERLINE (4096)
#define MYMAXPATHLEN (1024)


/* set this to 1 for audit generation or 0 for no debugging */
#define DEBUG (0)


/******************************************************************************/
/* debugging */


#if DEBUG
void    AuditPrint(char* First, ...);
#define APRINT(param) AuditPrint param
#define ERROR(cond,cmd) if (cond) cmd
#else
#define APRINT(param)
#define ERROR(cond,cmd)
#endif


#if DEBUG

static FILE*       BugFile = NULL;

void          AuditPrint(char* First, ...)
  {
    va_list     ap;

    if (BugFile == NULL)
      {
        BugFile = fopen("audit","w");
        if (BugFile == NULL)
          {
            perror("Couldn't open 'audit' debug file for writing");
            exit(EXIT_FAILURE);
          }
      }
    va_start(ap,First);
    vfprintf(BugFile,First,ap);
    va_end(ap);
    putc('\n',BugFile);
    fflush(BugFile);
  }
#endif


/******************************************************************************/
/* group list stuff */


typedef struct GroupRec
  {
    char*             GroupName;
    long              LastReadArticle;
    struct GroupRec*  Next;
  } GroupRec;


typedef struct GroupListRec
  {
    GroupRec*     List;
  } GroupListRec;


GroupListRec*     ReadNewsList(void);
void              Subscribe(GroupListRec* List, char* Name);
void              Unsubscribe(GroupListRec* List, char* Name);
void              WriteAndDisposeNewsList(GroupListRec* List);
long              GetGroupLastReadArticle(GroupListRec* List, char* Name);
void              SetGroupLastReadArticle(GroupListRec* List, char* Name,
                    long LastRead);
long              GroupListLength(GroupListRec* List);
char*             GetGroupName(GroupListRec* List, long Index);


/* read the .newslist file in */
GroupListRec*     ReadNewsList(void)
  {
    GroupListRec*   List;
    FILE*           ReadFile;
    char            Path[MYMAXPATHLEN];
    char*           Home;

    APRINT(("+ReadNewsList"));
    List = malloc(sizeof(GroupListRec));
    List->List = NULL;
    Home = getenv("HOME");
    if (Home == NULL)
      {
        Home = "";
      }
    if (strlen(Home) + strlen(NEWSFILENAME) + 1 > MYMAXPATHLEN - 1)
      {
        fprintf(stderr,"path to "NEWSFILENAME" is too long\n");
        exit(EXIT_FAILURE);
      }
    strcpy(Path,Home);
    strcat(Path,"/");
    strcat(Path,NEWSFILENAME);
    ReadFile = fopen(Path,"rb");
    if (ReadFile != NULL)
      {
        GroupRec*     Tail;

        Tail = NULL;
        while (!feof(ReadFile))
          {
            int           NameLen;
            char*         Name;
            int           Value;
            GroupRec*     New;
            Bool          Negative;

            Name = malloc(0);
            NameLen = 0;
            while ((Value = getc(ReadFile)),(Value != EOF) && (Value != ':'))
              {
                Name = realloc(Name,NameLen + 1);
                Name[NameLen] = Value;
                NameLen += 1;
              }
            if ((Value == EOF))
              {
                goto EndOfFileSkipTownNow;
              }
            Name = realloc(Name,NameLen + 1);
            Name[NameLen] = 0;
            fprintf(stderr,"Newsgroup '%s' info loaded.\n",Name);
            New = malloc(sizeof(GroupRec));
            New->GroupName = Name;
            New->LastReadArticle = 0;
            Negative = False;
            while ((Value = getc(ReadFile)),(Value != EOF) && (Value != '\n'))
              {
                if (Value == '-')
                  {
                    Negative = True;
                  }
                 else
                  {
                    New->LastReadArticle = (10 * New->LastReadArticle)
                      + (Value - '0');
                  }
              }
            if (Negative)
              {
                New->LastReadArticle = - New->LastReadArticle;
              }
            /* link the data structure */
            New->Next = NULL;
            if (Tail == NULL)
              {
                List->List = New;
              }
             else
              {
                Tail->Next = New;
              }
            Tail = New;
          }
       EndOfFileSkipTownNow:
        fclose(ReadFile);
      }
    APRINT(("-ReadNewsList %p",List));
    return List;
  }


/* add a new group to the news list */
void              Subscribe(GroupListRec* List, char* Name)
  {
    GroupRec*       Scan;
    GroupRec*       Lag;

    APRINT(("+Subscribe %p %s",List,Name));
    Scan = List->List;
    Lag = NULL;
    while ((Scan != NULL) && !strequ(Name,Scan->GroupName))
      {
        Lag = Scan;
        Scan = Scan->Next;
      }
    if (Scan == NULL)
      {
        GroupRec*       New;

        /* if Scan == NULL then we got to the end of the list without */
        /* finding the group, so we can append it */
        New = malloc(sizeof(GroupRec));
        New->GroupName = malloc(strlen(Name) + 1);
        strcpy(New->GroupName,Name);
        New->LastReadArticle = -1;
        New->Next = NULL;
        if (Lag != NULL)
          {
            Lag->Next = New;
          }
         else
          {
            List->List = New;
          }
      }
    APRINT(("-Subscribe"));
  }


/* remove a group from the list */
void              Unsubscribe(GroupListRec* List, char* Name)
  {
    GroupRec*       Scan;
    GroupRec*       Lag;

    APRINT(("+Unsubscribe %p %s",List,Name));
    Scan = List->List;
    Lag = NULL;
    while ((Scan != NULL) && !strequ(Name,Scan->GroupName))
      {
        Lag = Scan;
        Scan = Scan->Next;
      }
    if (Scan != NULL)
      {
        if (Lag != NULL)
          {
            Lag->Next = Scan->Next;
          }
         else
          {
            List->List = Scan->Next;
          }
        free(Scan->GroupName);
        free(Scan);
      }
    APRINT(("-Unsubscribe"));
  }


/* write the newslist back to the .newslist file */
void              WriteAndDisposeNewsList(GroupListRec* List)
  {
    FILE*           Out;
    GroupRec*       Scan;
    char            Path[MYMAXPATHLEN];
    char*           Home;

    APRINT(("+WriteAndDisposeNewsList %p",List));
    Home = getenv("HOME");
    if (Home == NULL)
      {
        Home = "";
      }
    if (strlen(Home) + strlen(NEWSFILENAME) + 1 > MYMAXPATHLEN - 1)
      {
        fprintf(stderr,"path to "NEWSFILENAME" is too long\n");
        exit(EXIT_FAILURE);
      }
    strcpy(Path,Home);
    strcat(Path,"/");
    strcat(Path,NEWSFILENAME);
    Out = fopen(Path,"wb");
    if (Out != NULL)
      {
        Scan = List->List;
        while (Scan != NULL)
          {
            fprintf(Out,"%s:%ld\n",Scan->GroupName,Scan->LastReadArticle);
            fprintf(stderr,"Writing newsgroup '%s' info.\n",Scan->GroupName);
            Scan = Scan->Next;
          }
        fclose(Out);
      }
     else
      {
        fprintf(stderr,"Couldn't write updated "NEWSFILENAME" file.\n");
      }
    Scan = List->List;
    while (Scan != NULL)
      {
        GroupRec*     Temp;

        Temp = Scan;
        Scan = Scan->Next;
        free(Temp->GroupName);
        free(Temp);
      }
    free(List);
    APRINT(("-WriteAndDisposeNewsList"));
  }


long              GetGroupLastReadArticle(GroupListRec* List, char* Name)
  {
    GroupRec*       Scan;

    APRINT(("+GetGroupLastReadArticle %p %s",List,Name));
    Scan = List->List;
    while ((Scan != NULL) && !strequ(Name,Scan->GroupName))
      {
        Scan = Scan->Next;
      }
    if (Scan != NULL)
      {
        APRINT(("-GetGroupLastReadArticle %ld",Scan->LastReadArticle));
        return Scan->LastReadArticle;
      }
    fprintf(stderr,"Internal error:  GetGroupLastReadArticle unknown group.\n");
    exit(EXIT_FAILURE);
  }

void              SetGroupLastReadArticle(GroupListRec* List, char* Name,
                    long LastRead)
  {
    GroupRec*       Scan;

    APRINT(("+SetGroupLastReadArticle %p %s %ld",List,Name,LastRead));
    Scan = List->List;
    while ((Scan != NULL) && !strequ(Name,Scan->GroupName))
      {
        Scan = Scan->Next;
      }
    if (Scan != NULL)
      {
        Scan->LastReadArticle = LastRead;
      }
     else
      {
        fprintf(stderr,
          "Internal error:  SetGroupLastReadArticle unknown group.\n");
        exit(EXIT_FAILURE);
      }
    APRINT(("-SetGroupLastReadArticle"));
  }


long              GroupListLength(GroupListRec* List)
  {
    long            Count;
    GroupRec*       Scan;

    Count = 0;
    Scan = List->List;
    while (Scan != NULL)
      {
        Count += 1;
        Scan = Scan->Next;
      }
    return Count;
  }


char*             GetGroupName(GroupListRec* List, long Index)
  {
    GroupRec*       Scan;

    Scan = List->List;
    while (Index > 0)
      {
        Index -= 1;
        Scan = Scan->Next;
      }
    return Scan->GroupName;
  }


/******************************************************************************/
/* stuff for getting articles from wherever they should be gotten from */


void              OpenServer(void);
void              CloseServer(void);
Bool              GetServerGroupInfo(char* GroupName, long* Lowbound,
                    long* Highbound);
Bool              ServerReadArticle(char* GroupName, long ArticleIndex);


#if USENNTP /* { */
/* TCP stream stuff */

/* these two routines are only needed for TCP stuff */
void              ServerWrite(char* String);
void              ServerRead(char Buffer[MAXSERVERLINE]);

/* these file descriptors are fdopened on the socket */
static FILE*      NNTPIn;
static FILE*      NNTPOut;


void              OpenServer(void)
  {
    int             TheSocket;
    int             TheOtherSocket;
    struct sockaddr_in Where;
    char            MyStupidBuffer[MAXSERVERLINE];
    struct hostent* HostEnt;

    APRINT(("+OpenServer"));
    /* obtain a TCP socket */
    TheSocket = socket(PF_INET,SOCK_STREAM,0);
    if (TheSocket < 0)
      {
        perror("OpenServer:  Can't open socket");
        exit(EXIT_FAILURE);
      }
    /* establish a connection on the socket */
    HostEnt = gethostbyname(NEWSSERVERNAME);
    if (HostEnt == NULL)
      {
        perror("OpenServer:  NULL response from gethostbyname");
        exit(EXIT_FAILURE);
      }
    if (HostEnt->h_length < 1)
      {
        fprintf(stderr,"OpenServer:  gethostbyname gave zero addresses");
        exit(EXIT_FAILURE);
      }
    memset((char *)&Where,0,sizeof(Where));
    Where.sin_family = AF_INET;
    Where.sin_port = htons(119);
    memcpy((char *)&Where.sin_addr,HostEnt->h_addr,HostEnt->h_length);
    if (0 > connect(TheSocket,(void*)&Where,sizeof(Where)))
      {
        perror("OpenServer:  Can't connect to "NEWSSERVERNAME);
        exit(EXIT_FAILURE);
      }
    /* duplicate the socket so that fclose works properly */
    TheOtherSocket = dup(TheSocket);
    if (TheOtherSocket < 0)
      {
        perror("OpenServer:  Can't duplicate the socket");
        exit(EXIT_FAILURE);
      }
    /* make the input and output streams */
    NNTPIn = fdopen(TheSocket,"r");
    NNTPOut = fdopen(TheSocket,"w");
    if ((NNTPIn == NULL) || (NNTPOut == NULL))
      {
        perror("OpenServer:  Can't fdopen socket");
      }
    /* turn off buffering on the output stream */
    setbuf(NNTPOut,NULL);
    /* read in the welcome crud from the server */
    do
      {
        /* we wait for the message 200 or 201 from the server */
        ServerRead(MyStupidBuffer);
      } while (MyStupidBuffer[0] != '2');
    APRINT(("-OpenServer"));
  }


void              CloseServer(void)
  {
    /* close socket */
    APRINT(("+CloseServer"));
    ServerWrite("QUIT");
    fclose(NNTPOut);
    fclose(NNTPIn);
    APRINT(("-CloseServer"));
  }


void              ServerWrite(char* String)
  {
    APRINT(("+ServerWrite '%s'",String));
    sleep(SERVERCMDWAIT);
    fprintf(NNTPOut,"%s\n",String);
    APRINT(("-ServerWrite"));
  }


void              ServerRead(char Buffer[MAXSERVERLINE])
  {
    int             Character;
    int             Index;
    static long     SleepByteCounter = 0;

    APRINT(("+ServerRead"));
    Index = 0;
   LoopPoint:
    Character = getc(NNTPIn);
    SleepByteCounter += 1;
    if (SleepByteCounter > MAXBYTESPERSECOND)
      {
        /* sleep for 1 second & reset counter */
        /* this reduces the load on the news server */
        SleepByteCounter = 0;
        sleep(1);
      }
    if (Character == '\n')
      {
        while ((Index > 0) && ((Buffer[Index - 1] == '\n')
          || (Buffer[Index - 1] == '\r')))
          {
            Index -= 1;
          }
        Buffer[Index] = 0;
        APRINT(("-ServerRead '%s'",Buffer));
        return;
      }
    if (Index < MAXSERVERLINE - 1)
      {
        Buffer[Index] = Character;
        Index += 1;
      }
    goto LoopPoint;
  }


Bool              GetServerGroupInfo(char* GroupName, long* Lowbound,
                    long* Highbound)
  {
    char            Buffer[MAXSERVERLINE];
    long            ReturnCode;
    long            NumArticles;

    APRINT(("+GetServerGroupInfo %s",GroupName));
    sprintf(Buffer,"GROUP %s",GroupName);
    ServerWrite(Buffer);
    ServerRead(Buffer);
    if (Buffer[0] == '2')
      {
        if (4 != sscanf(Buffer,"%ld %ld %ld %ld",&ReturnCode,&NumArticles,
          /*NB*/Lowbound,/*NB*/Highbound))
          {
            APRINT(("-GetServerGroupInfo False"));
            return False;
          }
        APRINT(("-GetServerGroupInfo %ld..%ld",*Lowbound,*Highbound));
        return True;
      }
    APRINT(("-GetServerGroupInfo False"));
    return False;
  }


/* read an article from the server.  it assumes that the group has been */
/* selected on the server. */
Bool              ServerReadArticle(char* GroupName, long ArticleIndex)
  {
    char            Buffer[MAXSERVERLINE];

    APRINT(("+ServerReadArticle %s %ld",GroupName,ArticleIndex));
    sprintf(Buffer,"ARTICLE %ld",(long)ArticleIndex);
    ServerWrite(Buffer);
    ServerRead(Buffer);
    if (0 == memcmp(Buffer,"400",3))
      {
        fprintf(stderr,"Unexpected server termination.\n");
        exit(EXIT_FAILURE);
      }
    else if (Buffer[0] == '4')
      {
        APRINT(("-ServerReadArticle False"));
        return False; /* article nonexistent */
      }
    /* else article follows */
    printf("%s\n",GroupName);
    do
      {
        ServerRead(Buffer);
        printf("%s\n",Buffer);
      } while (!strequ(Buffer,"."));
    APRINT(("-ServerReadArticle True"));
    return True;
  }


#else /* } { */


void              OpenServer(void)
  {
    /* no action taken when using spool directory */
  }


void              CloseServer(void)
  {
    /* no action taken */
  }


/* make a newsgroup name into a path name. */
static void       MakeNewsgroupPath(char FilenameBuf[MYMAXPATHLEN],
                    char* NewsgroupName)
  {
    int             Index;
    int             NameScan;

    strcpy(FilenameBuf,SPOOLDIRPATH);
    Index = strlen(FilenameBuf);
    NameScan = 0;
    while ((NameScan < MYMAXPATHLEN - 1) && (NewsgroupName[NameScan] != 0))
      {
        if (NewsgroupName[NameScan] == '.')
          {
            FilenameBuf[Index] = '/';
          }
         else
          {
            FilenameBuf[Index] = NewsgroupName[NameScan];
          }
        NameScan += 1;
        Index += 1;
      }
    FilenameBuf[Index] = 0;
  }


#define DIRTYPE dirent
Bool              GetServerGroupInfo(char* GroupName, long* Lowbound,
                    long* Highbound)
  {
    DIR*            Directory;
    struct DIRTYPE* DirEntry;
    char            PathName[MYMAXPATHLEN];

    MakeNewsgroupPath(PathName,GroupName);
    *Lowbound = LONG_MAX;
    *Highbound = 0;
    Directory = opendir(PathName);
    if (Directory == NULL)
      {
        return False;
      }
    while ((DirEntry = readdir(Directory)) != NULL)
      {
        long            Candidate;

        Candidate = atol(DirEntry->d_name);
        if ((Candidate < *Lowbound) || (Candidate > *Highbound))
          {
            char*           Scan;

            for (Scan = DirEntry->d_name; *Scan != 0; Scan += 1)
              {
                if ((*Scan < '0') || (*Scan > '9'))
                  {
                    goto NotAnArticle;
                  }
              }
            if (Candidate < *Lowbound)
              {
                *Lowbound = Candidate;
              }
            if (Candidate > *Highbound)
              {
                *Highbound = Candidate;
              }
          }
       NotAnArticle:
        ;
      }
    closedir(Directory);
    if (*Lowbound == LONG_MAX)
      {
        *Lowbound = 0;
      }
    return True;
  }


Bool              ServerReadArticle(char* GroupName, long ArticleIndex)
  {
    char          PathName[MYMAXPATHLEN];
    char          ArtName[64];
    FILE*         DaFile;

    MakeNewsgroupPath(PathName,GroupName);
    sprintf(ArtName,"/%ld",ArticleIndex);
    if (strlen(PathName) + strlen(ArtName) > MYMAXPATHLEN - 1)
      {
        return False; /* string too long */
      }
    strcat(PathName,ArtName);
    DaFile = fopen(PathName,"r");
    if (DaFile == NULL)
      {
        return False;
      }
    printf("%s\n",GroupName);
    while (!feof(DaFile))
      {
        char          Buffer[MAXSERVERLINE];

        fgets(Buffer,MAXSERVERLINE,DaFile);
        if ((strlen(Buffer) > 0) && (Buffer[strlen(Buffer) - 1] == '\n'))
          {
            Buffer[strlen(Buffer) - 1] = 0;
          }
        if (strequ(Buffer,"."))
          {
            puts("..");
          }
         else
          {
            puts(Buffer);
          }
      }
    fclose(DaFile);
    puts("."); /* finally, mark end of thang */
    return True;
  }


#endif /* } */


/******************************************************************************/
/* main thing */


int               main(int argc, char** argv)
  {
    GroupListRec*   GroupList;

    APRINT(("+main"));
    GroupList = ReadNewsList();
    if (GroupList != NULL)
      {
        if ((argc == 1) || ((argc == 2) && (strequ("zap",argv[1]))))
          {
            long              GroupScan;
            int               DontReadGroups;

            DontReadGroups = ((argc == 2) && (strequ("zap",argv[1])));
            OpenServer();
            for (GroupScan = 0; GroupScan < GroupListLength(GroupList);
              GroupScan += 1)
              {
                char*             GroupName;
                int               SuccessFlag;
                long              Lowbound;
                long              Highbound;

                GroupName = GetGroupName(GroupList,GroupScan);
                fprintf(stderr,"Group %s\n",GroupName);
                SuccessFlag = GetServerGroupInfo(GroupName,&Lowbound,&Highbound);
                if (DontReadGroups)
                  {
                    long              OldLastRead;

                    fprintf(stderr,"Zapping group %s\n",GroupName);
                    SetGroupLastReadArticle(GroupList,GroupName,Highbound);
                  }
                else if (!SuccessFlag)
                  {
                    fprintf(stderr,"Can't get group %s\n",GroupName);
                  }
                else
                  {
                    long              OldLastRead;
                    long              ArtScan;

                    OldLastRead = GetGroupLastReadArticle(GroupList,GroupName);
                    if (OldLastRead < 0)
                      {
                        /* new group, don't want to haul all articles over */
                        OldLastRead = Highbound;
                      }
                    if (OldLastRead < Lowbound - 1)
                      {
                        OldLastRead = Lowbound - 1;
                      }
                    if (OldLastRead > Highbound)
                      {
                        /* has newsgroup been reset maybe? */
                        OldLastRead = Lowbound - 1;
                      }
                    for (ArtScan = OldLastRead + 1; ArtScan <= Highbound;
                      ArtScan += 1)
                      {
                        if (ServerReadArticle(GroupName,ArtScan))
                          {
                            /* we successfully read the article */
                            fprintf(stderr,"Got %s: %ld\n",GroupName,ArtScan);
                          }
                         else
                          {
                            /* article wasn't available */
                            fprintf(stderr,"Unavail %s: %ld\n",GroupName,
                              ArtScan);
                          }
                      }
                    SetGroupLastReadArticle(GroupList,GroupName,Highbound);
                  }
              }
            CloseServer();
          }
         else
          {
            int           Scan;

            for (Scan = 1; Scan < argc; Scan += 1)
              {
                if (argv[Scan][0] == '+')
                  {
                    Subscribe(GroupList,&(argv[Scan][1]));
                  }
                else if (argv[Scan][0] == '-')
                  {
                    Unsubscribe(GroupList,&(argv[Scan][1]));
                  }
                else
                  {
                    printf("Misformed argument '%s'\n",argv[Scan]);
                    exit(EXIT_FAILURE);
                  }
              }
          }
        WriteAndDisposeNewsList(GroupList);
      }
    APRINT(("-main"));
    exit(EXIT_SUCCESS);
  }
