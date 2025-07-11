/*
 * amsar ams archiver
 * $Header: /afs/andrew.cmu.edu/usr13/aw0g/amsar/RCS/amsar.c,v 1.5 90/05/28 07:10:57 aw0g Exp $
 */

#include <andrewos.h>
#include <ms.h>
#include <hdrparse.h>
#include <errprntf.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>

extern char *sys_errlist[],
	   *ms_errlist[],
	   *ms_errcauselist[],
	   *ms_errvialist[],
	   *rpc_errlist[];
extern int  sys_nerr,
	    ms_nerr,
	    ms_nerrcause,
	    ms_nerrvia,
	    rpc_nerr;

#define STREQ(xx_s1,xx_s2) \
  (ULstrcmp(xx_s1,xx_s2)==0)

/*
 * when iterating over all the headers, if this much time
 * goes by print how out how far along (percent done) we are.
 */
#define SECOND (1)
#define TALK_TIME (15*SECOND)

#define AMSAR_FILE_VERSION (0x7EBF3751L)

extern char *malloc();
extern FILE *fopen();
extern char *strchr();
extern char *strrchr();

#define EOF_STR "?*EOFx"

/*size of io buffers when copying files*/
#define BUF_SIZE (12000)

#undef NIL
#define NIL 0L

#ifndef TRUE
#define TRUE (0==0)
#define FALSE (0==1)
#endif
#define x stderr
/*
 * general fatal error not io related
 */
#define BARF(xx_arg) \
 do {fprintf(stderr,"%s:",pname);fprintf xx_arg ;fprintf(stderr,"\n");exit(1);}while(FALSE)

/*
 * handle an io (ie errno is interesting) error
 */
#define BIO(xx_arg) \
 do{fprintf(stderr,"%s:",pname);fprintf xx_arg ;perror("\n - "); fprintf(stderr,"\n");exit(1); \
 }while(FALSE)

/*
 * do a MS_ message server call, fail if it does
 */
#define MSCALL(xx_result,xx_arg) \
 if((mserrcode=(xx_result))!=0) \
  {fprintf(stderr,"%s:",pname); \
   fprintf xx_arg ;end_ams_err();}

#define imin(xx1,xx2) (((xx1)<(xx2))?(xx1):(xx2))
#define imax(xx1,xx2) (((xx1)>(xx2))?(xx1):(xx2))
#define iabs(xx1) ((xx1)<0)?(-(xx1)):(xx1)

/*
 * The matching operators (last, since, and, ...) all return
 * an array.  There is one element for each header.  This
 * element is TRUE if the matching header is selected
 */
typedef char *selection;	/*a term of an expression*/
selection parse_expr();		/*forward refrence due to recursion*/

char *pname;  /*name of this program (from argv[0]) */

/*
 * parser globals
 */
struct {
  int argc;
  char **argv;
  char *p_token;		/*current token*/
  FILE *cons;			/*log descriptor or nil to shut up*/
  FILE *pakfile;		/*pakfile*/
  int pak_isdesc;		/*pakfile is std desc so don't close it*/
  char *pakname;		/*name of packfile*/
  int anycase;			/*care about upper/lower case in compare?*/
  char *shortname;		/*folder name*/
  char longname[MAXPATHLEN];	/*folder path*/
  long count;			/*number of entries*/
  int can_write;		/*access to this folder*/
}pgl;

/*
 * set up parse input from argv
 */
void parse_from(argc,argv)
int argc;
char **argv;
{
  pgl.argc=argc;
  pgl.argv=argv;
}

/*
 * get the next token
 */
char *p_get_token()
{
  if(pgl.argc-- <=0)
    pgl.p_token=EOF_STR;
  else
    pgl.p_token= *pgl.argv++;
  return pgl.p_token;
}

/*
 * peek ahead in the token stream
 */
char *p_peek_token(lookahead)
int lookahead;
{
  if(pgl.argc-lookahead <=0)
    return EOF_STR;
  else
    return *(pgl.argv+lookahead);
}

/*
 * get a token, EOF is an error
 */
char *p_really_get_token()
{
  if(pgl.argc <=0)
    BARF((x,"not enough input, try '%s help'",pname));
  return p_get_token();
}

/*
 * replace dots in a string with slashes
 */
char *slashify(instr)
char *instr;
{
  register char *s=instr;
  while((s=strchr(s,'.'))!=NIL)
    *s='/';
  return instr;
}

/*
 * return a dynamicly allocated copy of a string
 */
char *scopy(s)
char *s;
{
  int len=strlen(s)+1;
  char *result;
  if((result=malloc(len+1))==0)
    BARF((x,"cant allocate memory for a string temporary for '%s'",s));
  memcpy(result,s,len);
  return result;
}

/*
 * tail end of MSCALL error handling
 */
void end_ams_err()
{
  fprintf(stderr,":%d %s\n",AMS_ERRNO,
	  ((AMS_ERRNO >= EMSBASE) ? ms_errlist[AMS_ERRNO - EMSBASE]
		 : sys_errlist[AMS_ERRNO]));
  fflush(stderr);
  exit(1);
}

/*
 * create the named folder, fail if we can not
 */
void create_folder(afolder)
char *afolder;
{
  char *dirshortname=slashify(scopy(afolder));
  char *newfolder=strrchr(dirshortname,'/');
  char dirname[MAXPATHLEN+1];
  if(newfolder!=0) {
    *newfolder++ =0;
    MSCALL(MS_DisambiguateFile(dirshortname,dirname,
			       AMS_DISAMB_DIREXISTS),
	   (x,"parent directory '%s' doesnot exist",dirshortname));
  } else {			/*no slash do must be personal*/
    MSCALL(MS_GetSearchPathEntry(AMS_MAILPATH,dirname,
			MAXPATHLEN),
	   (x,"can't get path for home message directory"));
    newfolder=afolder;
  }
  strcat(dirname,"/");
  strcat(dirname,newfolder);
  if((strchr(dirname,'+')!=0)||
     (strchr(dirname,' ')!=0))
    BARF((x,"illegal folder name:%",dirname));
  MSCALL(MS_CreateNewMessageDirectory(dirname,FALSE,dirname),
	 (x,"can't create '%'",newfolder));
  if(pgl.cons)
    fprintf(pgl.cons,"created folder %s\n",newfolder);
}

/*
 * parse the a, it may be source or dest
 */
void parse_folder(we_can_create_if_we_want_to,afolder)
int we_can_create_if_we_want_to;
char *afolder;
{
  long protcode=0;
  char *slash_shortname=slashify(scopy(afolder));
  int er=MS_DisambiguateFile(slash_shortname,
			     pgl.longname,AMS_DISAMB_DIREXISTS);
  pgl.shortname=afolder;
  if((er!=0)&&(we_can_create_if_we_want_to)) {
    create_folder(afolder);
    er=MS_DisambiguateFile(slash_shortname,
			     pgl.longname,AMS_DISAMB_DIREXISTS);
  }
  MSCALL(er,
	 (x,"can't access folder '%s'",pgl.shortname));

  MSCALL(MS_GetDirInfo(pgl.longname,
		       &protcode,
		       &pgl.count),
	 (x,"can't count files in folder '%s'",pgl.shortname));
  switch(protcode) {
  case AMS_DIRPROT_MODIFY:
  case AMS_DIRPROT_MBOX:
  case AMS_DIRPROT_FULLMAIL:
    pgl.can_write=TRUE;
    break;
  case AMS_DIRPROT_READ:
  case AMS_DIRPROT_LOCALBB:
  case AMS_DIRPROT_OFFBB:
  case AMS_DIRPROT_EXTBB:
  case AMS_DIRPROT_AWFUL:
    pgl.can_write=FALSE;
    break;
  default:
    BARF(("unknown ams folder DIRPROT %d",protcode));
  }
}

/*
 * parse a folder name and be sure it is writable
 */
void parse_write_folder(we_can_create_if_we_want_to,amsfolder)
int we_can_create_if_we_want_to;
char *amsfolder;
{
  parse_folder(we_can_create_if_we_want_to,amsfolder);
  if(!pgl.can_write)
    BARF((x,"You don't have write access to '%s'",pgl.shortname));
}

/*
 * see how long the the loops that iterate over the snapshots
 * are taking and print chatty info
 */
struct loop_status_R {
  long last_time;
  char *lname;
};
typedef struct loop_status_R loop_status,*loop_status_pt;

/*
 * a table of headers to match eithor from a folder
 * or being restored from an archive
 */
struct {
  int count;			/*number of entries*/
  int matching;			/*number of matching entries*/
  char *data;			/*pointer to snapshots*/
  char *flags;			/*per snapshot flags*/
}hed;

/*
 * init the loop status info
 */
void new_loop(aloop,lname)
loop_status_pt aloop;
char *lname;
{
  aloop->last_time=time(0L);
  aloop->lname=lname;
}

/*
 * in a loop see if it is time to tell the user how we are doing
 */
void loop_step(aloop,position)
loop_status_pt aloop;
char *position;
{
  long now;
  long num_matched;
  if(pgl.cons==NIL)return;
  if(aloop->lname==NIL)return;
  now=time(0L);
  if((now-aloop->last_time)<TALK_TIME)return;
  num_matched=(position-hed.data)/AMS_SNAPSHOTSIZE;
  fprintf(pgl.cons,
	  "%s %d/%d %d%%\n",aloop->lname,num_matched,hed.matching,
	  ((long)((((double)num_matched)*100.0/((double)hed.matching))+0.5)));
  aloop->last_time=now;
}

#define FOR_ALL_HEADERS(xx_loop_name,xx_arg) \
  {int fah_i; \
   char *xx_arg; \
   loop_status xx_lstatus; \
   new_loop(&xx_lstatus,xx_loop_name); \
   for(fah_i=0,xx_arg=hed.data;fah_i<hed.count;fah_i++,xx_arg+=AMS_SNAPSHOTSIZE) { \
     loop_step(&xx_lstatus,xx_arg); {

#define END_FOR_ALL_HEADERS } } }

#define FOR_SEL(xx_asel,xx_entry) \
  { char *xx_entry; \
    int fors_i; \
    for(fors_i=hed.count,xx_entry=(xx_asel);fors_i>0;fors_i--,xx_entry++) {

#define END_FOR_SEL }}

/*
 * set up headers for matching loading them from ams
 */
void read_headers_from_ams()
{
  hed.count=pgl.count;
  if((hed.data=malloc(AMS_SNAPSHOTSIZE*hed.count))==0)
    BARF((x,"not enough memory to allocate %d headers, try a machine with more virtual memory",hed.count));
  MSCALL(MS_GetBufNthSnapshot(pgl.longname,0,hed.data,&hed.count),
	 (x,"can't read headers from ams folder"));
  if(pgl.cons)
    fprintf(pgl.cons,"loaded %d header records from ams\n",hed.count);
}

/*
 * parse one field of a caption
 */
char *parse_capfield(src,dst,limit)
char *src;
char **dst;
char *limit;
{
  register char ch;
  if((limit-src)<=0)
    *dst="";
  else
    *dst=src;
  while((limit-src)>0) {
    ch= *src++;			/*scan for the deviding tab*/
    if(ch==0)			/*nulls mean it extends further*/
      continue;
    if(ch==9) {			/*tabs seperate fields*/
      *(src-1)=0;		/*bash tab into field terminator*/
      return src;
    }
  }
  return src;
}

struct upak_snapshot_R {
  long hed_time;		/*ams time (positon in folder)*/
  char *pdate;			/*time from message text*/
  char *subj;
  char *sender;
  char *size;			/*the "(200)" size field */
  char buf[AMS_SNAPSHOTSIZE];
};
typedef struct upak_snapshot_R upak_snapshot,*upak_shapshot_pt;

/*
 * unpack a snapshot
 */
void unpack_snapshot(result,ahed)
upak_snapshot *result;
char *ahed;
{
  char *acap=AMS_CAPTION(result->buf);
  char *limit=acap+AMS_CAPTIONSIZE;
  memcpy(result->buf,ahed,AMS_SNAPSHOTSIZE);
  result->hed_time=conv64tolong(AMS_DATE(result->buf));
  acap=parse_capfield(acap,&result->pdate,limit);
  acap=parse_capfield(acap,&result->subj,limit);
  acap=parse_capfield(acap,&result->sender,limit);

  /*
   * split the size off from the sender
   */
  result->size=strlen(result->sender)+result->sender;
  if((*(result->size-1))==')')
    while((result->size-result->sender)>2) {
      if((*result->size)=='(') {
	*(result->size-1)=0;
	break;
      }
      result->size--;
    }
}

/*
 * create a new selection
 */
selection sel_new()
{
  selection result;
  if((result=malloc(hed.count))==0)
    BARF((x,"not enough virtual memory to allocate a matching expression term %d bytes",hed.count));
  memset(result,FALSE,hed.count);
  return result;
}

#define DOWNCASE(x) (isascii(x) && isalpha(x) && isupper(x) ? (tolower(x)) : (x) )

/*
 * downcase the given string if doing non case sensitive compares
 */
char *downcase(s)
char *s;
{
  register char ch;
  register char *result=s;
  while((ch=(*result))!=0)
    *result++ =DOWNCASE(ch);
  return s;
}

/*
 * match a sender against snapshots
 */
selection sel_match(mstr,against)
char *mstr;
char against;
{
  char *er;
  extern char *re_comp();
  upak_snapshot asnap;
  char **against_pt;
  selection result=sel_new();
  selection scan=result;
  if((er=re_comp(downcase(scopy(mstr))))!=NIL)
    BARF((x,"can't compile '%s' because:%s",mstr,er));
  switch(against) {
  case 'f':
    against_pt= &asnap.sender;
    break;
  case 's':
    against_pt= &asnap.subj;
    break;
  }
  FOR_ALL_HEADERS("string matching",ahed) {
    unpack_snapshot(&asnap,ahed);
    switch(re_exec(downcase(*against_pt))) {
    case 0:
      *scan++ =FALSE;
      break;
    case 1:
      *scan++ =TRUE;
      break;
    default:
      BARF((x,"regular expression corrupted"));
    }
  } END_FOR_ALL_HEADERS;
  return result;
}

/*
 * select a range of messages
 */
selection sel_range(start,count)
long start;
long count;
{
  selection result=sel_new();
  long end=start+count-1;
  /*
   * check for null selections
   */
  if(end<start)
    return result;
  if(start>=hed.count)
    return result;
  if(end<0)
    return result;
  start=imin(imax(0,start),hed.count-1);
  end=imin(imax(0,end),hed.count-1);
  memset(result+start,TRUE,end-start+1);
  return result;
}

/*
 * parse a term as a long
 */
long argtolong(aarg,name)
char *aarg;
char *name;
{
  register long result=0;
  register char ch;
  while((ch=(*aarg++))!=0) {
    if(!isdigit(ch))
      BARF((x,"character '%c' non numeric as argument to %s",ch,name));
    result*=10;
    result+=ch-'0';
  }
  return result;
}

/*
 * invert a selection
 */
selection sel_not(asel)
selection asel;
{
  FOR_SEL(asel,entry) {
    (*entry)= !(*entry);
  } END_FOR_SEL;
  return asel;
}

/*
 * parse a time into an unsigned long
 */
unsigned long parse_time(atime)
char *atime;
{
  long year,month,day,hour,min,sec,wday,gtm;
  if(MS_ParseDate(atime,&year,&month,&day,&hour,&min,&sec,&wday,&gtm))
    BARF((x,"can't parse date '%s'",atime));
  return gtm;
}

/*
 * make a selection based on time
 */
selection sel_time(atime,past_or_future)
char *atime;
register char past_or_future;
{
  selection result=sel_new();	/*default to false*/
  register unsigned long latime=parse_time(atime);
  register unsigned long entry_time;
  upak_snapshot asnap;
  char *entry=result;
  FOR_ALL_HEADERS("time matching",ahed) {
    unpack_snapshot(&asnap,ahed);
    entry_time=parse_time(asnap.pdate);
    switch(past_or_future) {
    case '>':			/*since*/
      if(entry_time>=latime)
	(*entry)= TRUE;
      break;
    case '<':			/*before*/
      if(entry_time<latime)
	(*entry)= TRUE;
      break;
    default:
      BARF((x,"memory damaged in select time '%c'",past_or_future));
    }
    entry++;
  } END_FOR_ALL_HEADERS;
  return result;
}

/*
 * parse a term of an expression
 */
selection parse_term()
{
  char *aterm=pgl.p_token;
  char *aarg=p_get_token();  
  if(STREQ("(",aterm)) {
    selection result=parse_expr();
    if(!STREQ(")",pgl.p_token))
      BARF((x,"missmatched ()"));
    p_get_token();
    return result;
  }
  if(STREQ("not",aterm))
    return sel_not(parse_term());
  if(STREQ("all",aterm))
    return sel_not(sel_new());

  /*
   * the rest take an argument
   */
  if(STREQ(pgl.p_token,EOF_STR))
    BARF((x,"argument to aterm %s missing",aterm));
  p_get_token();
  if(STREQ("first",aterm))
    return sel_range(0,argtolong(aarg,"first"));
  if(STREQ("last",aterm))
    return sel_range(hed.count-argtolong(aarg,"last"),hed.count);
  if(STREQ("from",aterm))
    return sel_match(aarg,'f');
  if(STREQ("subject",aterm))
    return sel_match(aarg,'s');
  if(STREQ("since",aterm))
    return sel_time(aarg,'>');
  if(STREQ("before",aterm))
    return sel_time(aarg,'<');

  BARF((x,"invalid term '%s' in expression",aterm));
}

/*
 * apply a binary operator to two selections
 */
selection binop(a1,a2,ch)
selection a1;
selection a2;
register char ch;
{
  char *step2=a2;
  FOR_SEL(a1,asel) {
    char s2value=(*step2++);
    switch(ch) {
    case '|':
      (*asel)= (*asel)||s2value;
      break;
    case '&':
      (*asel)= (*asel)&&s2value;
      break;
    default:
      BARF((x,"invalid internal binary operator '%c'",ch));
    }
  } END_FOR_SEL;
  free(a2);
  return a1;
}

/*
 * parse a select equation
 */
selection parse_expr()
{
  selection result;
  result=parse_term();
  while(TRUE) {
    if(STREQ("or",pgl.p_token)) {
      p_really_get_token();
      result=binop(result,parse_term(),'|');
    } else if(STREQ("and",pgl.p_token)) {
      p_really_get_token();
      result=binop(result,parse_term(),'&');
    } else return result;
  }
}

/*
 * count number of matches in a selection
 */
int count_sel(asel)
selection asel;
{
  long result=0;
  FOR_SEL(asel,entry) {
    if(*entry)
      result++;
  } END_FOR_SEL;
  return result;
}


#ifndef SHIFT_SIZE
#define SHIFT_SIZE (2)
#endif
/*
 * should be near wordsize but relativly
 * prime to word size
 */
#ifndef WORD_SIZE
#define WORD_SIZE (31)
#endif
#define KEEP_BLOCK_SIZE (WORD_SIZE-SHIFT_SIZE)

/*
 * checksum a string
 * uses hash function from Aaron Wohl's
 * macintosh snap hash table
 */
void pak_checksum(sum,key,keylen)
unsigned long *sum;
char *key;
int keylen;
{
  register unsigned long hhtval=(*sum);
  while(keylen-- >0) {
    hhtval = (hhtval << SHIFT_SIZE)    		/*rotate former hash*/
           ^ (hhtval >> KEEP_BLOCK_SIZE)
           ^ ((*key++)&0x0FF);	/*include new char*/
  }
  (*sum)=hhtval;
}

/*
 * write a number to the pak file
 */
void pak_write_number(checksum,anum)
unsigned long *checksum;
long anum;
{
  long netanum=htonl(anum);
  if(fwrite((char*)&netanum,1,4,pgl.pakfile)!=4)
    BIO((x,"write error on '%s' divider",pgl.pakname));
  if(checksum!=NIL)
    pak_checksum(checksum,(char *)&netanum,4);
}

/*
 * read a number from the pak file
 */
long pak_read_number(checksum)
unsigned long *checksum;
{
  long result;
  if(fread(&result,1,4,pgl.pakfile)!=4)
    BIO((x,"bad header in '%s'",pgl.pakname));
  result=ntohl(result);
  if(checksum!=NIL)
    pak_checksum(checksum,(char *)&result,4);
  return result;
}

/*
 * write all the headers
 */
void pak_headers()
{
  unsigned long checksum=0;
  long write_size=hed.count*AMS_SNAPSHOTSIZE;
  pak_write_number(&checksum,write_size);
  pak_checksum(&checksum,hed.data,write_size);
  if(fwrite(hed.data,1,write_size,pgl.pakfile)!=write_size)
    BIO((x,"write error packing headers on '%s'",pgl.pakname));
  pak_write_number(NIL,checksum);
}

/*
 * print a header in human readable form
 */
char *nicely_format_a_header(ahed)
char *ahed;
{
  static char result[300];
  upak_snapshot asnap;
  char pading[100];
  int subjlen;
  unpack_snapshot(&asnap,ahed);
  subjlen=strlen(asnap.subj);
  subjlen=imax(30-subjlen,0);
  if(subjlen!=0)
    memset(pading,' ',subjlen);
  pading[subjlen]=0;
  sprintf(result,"%s %s %s %s %s",
	     asnap.pdate,
	     asnap.subj,
	     pading,
	     asnap.sender,
	     asnap.size);
  return result;
}

/*
 * list all the headers in human readable format
 */
void list_headers()
{
  FOR_ALL_HEADERS(NIL,ahed) {
    if(fprintf(stdout,"%s\n",nicely_format_a_header(ahed))==EOF)
      BIO((x,"io error listing"));
  } END_FOR_ALL_HEADERS;
}

/*
 * pak one file
 */
void pak_1file(ahed)
char *ahed;
{
  unsigned long checksum=0;
  char bodypath[MAXPATHLEN+1];
  long body_len;
  long len_left;
  FILE *inbody;
  struct stat stat_buf;
  GetBodyFileName(pgl.longname,AMS_ID(ahed),bodypath);
  if((inbody=fopen(bodypath,"r"))==NIL)
    BIO((x,"can't open message '%s' :\n%s",bodypath,nicely_format_a_header(ahed)));
  if(fseek(inbody,0,2)!=0)	/*go to the end*/
    BIO((x,"can't seek on message '%s' :\n%s",bodypath,nicely_format_a_header(ahed)));
  if((body_len=ftell(inbody))<0)
    BIO((x,"can't get position on message '%s' :\n%s",bodypath,nicely_format_a_header(ahed)));
  if(fseek(inbody,0,0)!=0)	/*go back to the start*/
    BIO((x,"can't seek on message '%s' :\n%s",bodypath,nicely_format_a_header(ahed)));
  if(fstat(fileno(inbody),&stat_buf)!=0)
    BIO((x,"can't fstat message '%s' :\n%s",bodypath,nicely_format_a_header(ahed)));

  pak_write_number(&checksum,stat_buf.st_mtime);
  pak_write_number(&checksum,body_len);

  len_left=body_len;
  while(TRUE) {
    char body_buf[BUF_SIZE];
    long chunk_size;
    if((chunk_size=imin(sizeof(body_buf),len_left))<=0)
      break;
    len_left-=chunk_size;
    if(fread(body_buf,1,chunk_size,inbody)!=chunk_size)
      BIO((x,"read error on '%s' header :\n%s",bodypath,nicely_format_a_header(ahed)));
    pak_checksum(&checksum,body_buf,chunk_size);
    if(fwrite(body_buf,1,chunk_size,pgl.pakfile)!=chunk_size)
      BIO((x,"write error on '%s' file :\n%s",bodypath,nicely_format_a_header(ahed)));
  }
  if(vfclose(inbody)!=0)
    BIO((x,"error closing '%s' :\n%s",bodypath,nicely_format_a_header(ahed)));
  pak_write_number(NIL,checksum);
}

/*
 * write all the files that matched
 */
void pak_files()
{
  FOR_ALL_HEADERS("pack each file",ahed) {
    pak_1file(ahed);
  } END_FOR_ALL_HEADERS;
  if(pgl.cons)
    fprintf(pgl.cons,"all files packed, updating pak file\n");
  if(fflush(pgl.pakfile))
    BIO((x,"can't flush pak file output buffers"));
}

/*
 * read ams heards in from the pakfile
 */
void read_headers_from_file()
{
  unsigned long pak_crc;	/*crc read from pak file*/
  unsigned long got_crc=0;	/*crc computed*/
  long read_size=pak_read_number(&got_crc);
  if((read_size<0)||((hed.count%AMS_SNAPSHOTSIZE)!=0))
    BARF((x,"pak file size is %d bytes, which is not a multiple of the record size",hed.count));
  if(pgl.cons)
    fprintf(pgl.cons,"unpacking %d bytes of captions\n",read_size);
  if((hed.data=malloc(read_size))==0)
    BARF((x,"not enough memory to allocate %d headers, try a machine with more virtual memory",read_size));
  if(fread(hed.data,1,read_size,pgl.pakfile)!=read_size)
    BIO((x,"read error on '%s' headers",pgl.pakname));
  pak_crc=pak_read_number(NIL);
  pak_checksum(&got_crc,hed.data,read_size);
  if(got_crc!=pak_crc)
    BARF((x,"packed messages checksum error"));
  hed.count=read_size/AMS_SNAPSHOTSIZE;
}

/*
 * match headers agains the selection equation
 */
void match_headers()
{
  if(STREQ(pgl.p_token,EOF_STR)) /*no expression defaults to all*/
    hed.flags=sel_not(sel_new());
  else
    hed.flags=parse_expr();
  if(!STREQ(pgl.p_token,EOF_STR))
    BARF((x,"extra input '%s'... at end of select expression",pgl.p_token));
  hed.matching=count_sel(hed.flags);
  if(pgl.cons)
    fprintf(pgl.cons,"matched %d header(s)\n",hed.matching);
}

/*
 * close the pak file
 */
void close_pak()
{
  if(!pgl.pak_isdesc)
    if(vfclose(pgl.pakfile)!=0)
      BIO((x,"error closing '%s'",pgl.pakname));
  pgl.pakfile=0;
}

/*
 * loose records that where not matched
 */
void loose_unmatched()
{
  char *new_heds=hed.data;
  char *entry=hed.flags;
  FOR_ALL_HEADERS(NIL,ahed) {
    if(*(entry++)) {
      if(new_heds!=ahed)
	memcpy(new_heds,ahed,AMS_SNAPSHOTSIZE);
      new_heds+=AMS_SNAPSHOTSIZE;
    }
  } END_FOR_ALL_HEADERS;
  hed.count=(new_heds-hed.data)/AMS_SNAPSHOTSIZE;
  memset(hed.flags,TRUE,hed.count);
}

/*
 * create a new folder
 */
void arcreate()
{
  char *amsfolder=p_really_get_token();
  p_get_token();
  if(!STREQ(pgl.p_token,EOF_STR))
    BARF((x,"extra input '%s'... at end of create expression",pgl.p_token));
  create_folder(amsfolder);
}

/*
 * delete matching files
 */
void delete_files()
{
  MSCALL(MS_MassDeleteSnapshots(pgl.longname,hed.data,hed.count),
	 (x,"can't delete headers from ams folder",pgl.shortname));
}

/*
 * delete the matching messages from
 */
void ardelete()
{
  char *amsfolder;
  amsfolder=p_really_get_token();
  p_get_token();
  parse_write_folder(FALSE /*don't make folder*/,amsfolder);
  read_headers_from_ams();
  match_headers();
  loose_unmatched();
  delete_files();
}

/*
 * delete the given folder
 */
void ar_rmfolder()
{
  char *amsfolder;
  amsfolder=p_really_get_token();
  p_get_token();
  parse_write_folder(FALSE /*don't make folder*/,amsfolder);
  MSCALL(MS_RemoveDirectory(pgl.longname,0),
	 (x,"remove of '%s' failed",amsfolder));
  if(pgl.cons)
    fprintf(pgl.cons,"deleted folder %s\n",amsfolder);
}

/*
 * list the contents of a folder
 */
void arlist()
{
  char *amsfolder;
  amsfolder=p_really_get_token();
  p_get_token();
  parse_folder(FALSE /*don't create folder*/, amsfolder);
  pgl.cons=NIL;			/*list is never chatty*/
  read_headers_from_ams();
  match_headers();
  loose_unmatched();
  list_headers();
}

/*
 * set input file to the given file which may be stdin
 */
void set_input_file(ifile)
char *ifile;
{
  long fversion;
  if(STREQ(ifile,"-")) {
    pgl.pakname="standard in";
    pgl.pak_isdesc=TRUE;
    pgl.pakfile=stdin;
  } else {
    pgl.pakname=ifile;
    pgl.pak_isdesc=FALSE;
    if((pgl.pakfile=fopen(pgl.pakname,"r"))==NIL)
      BIO((x,"can't open '%s' for input",pgl.pakname));
  }

  /*
   * check the file version
   */
  fversion=pak_read_number(NIL);
  if(fversion!=AMSAR_FILE_VERSION)
    BARF((x,".pak file has a bad version number or is not a .pak file"));
}

/*
 * list the contents of pak file
 */
void ardir()
{
  set_input_file(p_really_get_token());
  p_get_token();
  pgl.cons=NIL;			/*list is never chatty*/
  read_headers_from_file();
  match_headers();
  loose_unmatched();
  list_headers();
  close_pak();
}

/*
 * pack messages
 */
void arpack()
{
  char *amsfolder;
  char *dst;
  int rmfiles=FALSE;		/*delete messages after successfull pack*/
  amsfolder=p_really_get_token();
  if(STREQ(amsfolder,"-delete")) {
    rmfiles=TRUE;
    amsfolder=p_really_get_token();
  }
  dst=p_really_get_token();
  p_get_token();
  if(rmfiles)
    parse_write_folder(FALSE /*don't make folder*/,amsfolder);
  else
    parse_folder(FALSE,amsfolder);
  if(STREQ(dst,"-")) {
    pgl.pakname="standard out";
    if(pgl.cons)		/*if not silent, then*/
      pgl.cons=stderr;		/*chatty output moves over to stderr*/
    pgl.pakfile=stdout;
    pgl.pak_isdesc=TRUE;
  }else {
    pgl.pakname=dst;
    pgl.pak_isdesc=FALSE;
    if((pgl.pakfile=fopen(pgl.pakname,"w"))==NIL)
      BARF((x,"can't open '%s' for output",pgl.pakname));
  }
  if(pgl.cons) {
    fprintf(pgl.cons,"pack from folder:%s\n",pgl.shortname);
    fprintf(pgl.cons,"pack to file:%s\n",pgl.pakname);
  }
  /*
   * read and match input headers
   */
  read_headers_from_ams();
  match_headers();
  loose_unmatched();

  /*
   * write .pak file
   */
  pak_write_number(NIL,AMSAR_FILE_VERSION);
  pak_headers();
  pak_files();
  close_pak();

  if(rmfiles)
    delete_files();
  if(pgl.cons)
    fprintf(pgl.cons,"done.\n");
}

/*
 * skip one file we don't need in the pak stream
 */
void unpak_skip1()
{
  unsigned long checksum=0;
  unsigned long read_checksum;
  long len_left;
  pak_read_number(&checksum);	/*skip the write time*/
  len_left=pak_read_number(&checksum);
  while(TRUE) {
    char body_buf[BUF_SIZE];
    long chunk_size;
    if((chunk_size=imin(sizeof(body_buf),len_left))<=0)
      break;
    len_left-=chunk_size;
    if(fread(body_buf,1,chunk_size,pgl.pakfile)!=chunk_size)
      BIO((x,"read error on '%s' header",pgl.pakname));
    pak_checksum(&checksum,body_buf,chunk_size);
  }
  read_checksum=pak_read_number(NIL);
  if(read_checksum!=checksum)
    BARF((x,"checksum error reading '%s'",pgl.pakname));
}

/*
 * restore the messages time
 */
void set_write_time(fname,write_time)
char *fname;
unsigned long write_time;
{
  struct timeval tvp[2];

  tvp[0].tv_sec = write_time;
  tvp[0].tv_usec = 0;
  tvp[1].tv_sec = write_time;
  tvp[1].tv_usec = 0;
  if (utimes(fname,tvp))
    BIO((x,"can't restore file time on '%s'",fname));
}
/*
 * read one file from the input stream
 * returns true if the file was already present
 */
int unpak_1file(ahed)
char *ahed;
{
  unsigned long checksum=0;
  unsigned long read_checksum;
  long write_time;
  long len_left;
  char body_path[MAXPATHLEN+1];
  struct stat stat_buf;
  upak_snapshot asnap;
  FILE *outbody;
  sprintf(body_path,"%s/+%s",pgl.longname,AMS_ID(ahed));

  unpack_snapshot(&asnap,ahed);

  if(lstat(body_path,&stat_buf)==0) {
    if(pgl.cons) {
      static already_printed_header=FALSE;
      if(!already_printed_header) {
	already_printed_header=TRUE;
	fprintf(pgl.cons,"These messages already present:\n");
      }
      fprintf(pgl.cons,"%s %s from %s\n",
	      asnap.pdate,asnap.subj,asnap.sender);
    }
    unpak_skip1();
    return TRUE;
  }

  write_time=pak_read_number(&checksum);
  len_left=pak_read_number(&checksum);
  if((outbody=fopen(body_path,"w"))==NIL)
    BIO((x,"can't open message '%s'",body_path));
  while(TRUE) {
    char body_buf[BUF_SIZE];
    long chunk_size;
    if((chunk_size=imin(sizeof(body_buf),len_left))<=0)
      break;
    len_left-=chunk_size;
    if(fread(body_buf,1,chunk_size,pgl.pakfile)!=chunk_size)
      BIO((x,"read error on '%s' header",pgl.pakname));
    pak_checksum(&checksum,body_buf,chunk_size);
    if(fwrite(body_buf,1,chunk_size,outbody)!=chunk_size)
      BIO((x,"write error on '%s' header",pgl.pakname));
  }
  read_checksum=pak_read_number(NIL);
  if(read_checksum!=checksum)
    BARF((x,"checksum error reading '%s'",pgl.pakname));
  if(vfclose(outbody)!=0)
    BIO((x,"error closing '%s'",body_path));
  set_write_time(body_path,write_time);
  return FALSE;
}

/*
 * append the passed snapshots to a directory
 */
int AppendSnapshots(Dir,somesnaps,snapcount)
struct MS_Directory *Dir;
char *somesnaps;
int snapcount;
{
  long last_time=0;
  int snapcount_bytes=AMS_SNAPSHOTSIZE*snapcount;

  /*
   * to maintian directory ordering,
   * get the time of the last snapshot
   * everything we append must be after it
   */
  if(Dir->MessageCount > 0) {
    char LastSnapshot[AMS_SNAPSHOTSIZE];
    if (GetSnapshotByNumber(Dir,Dir->MessageCount-1,LastSnapshot))
      return (mserrcode);
    last_time=conv64tolong(AMS_DATE(LastSnapshot));
  }

  /*
   * put each appended message in its own chain
   */
  {
    int i;
    long achain;
    char *scansnaps=somesnaps;
    for(i=0;i<snapcount;i++,scansnaps+=AMS_SNAPSHOTSIZE) {
      long this_time;
      achain=htonl(i+Dir->MaxChainVal);
      memcpy(AMS_CHAIN(scansnaps),&achain,sizeof(achain));
      this_time=conv64tolong(AMS_DATE(scansnaps));
      if(this_time<=last_time) {
	this_time=last_time+1;
	strcpy(AMS_DATE(scansnaps),convlongto64(this_time,0));
      }
      last_time=this_time;
    }
  }
  if (lseek(Dir->fd, AMS_DIRHEADSIZE + (Dir->MessageCount * AMS_SNAPSHOTSIZE), L_SET) < 0)
    AMS_RETURN_ERRCODE(errno, EIN_LSEEK, EVIA_GETSNAPSHOTBYID);
	
  if(write(Dir->fd,somesnaps,snapcount_bytes)!=snapcount_bytes)
    AMS_RETURN_ERRCODE(errno, EIN_LSEEK, EVIA_GETSNAPSHOTBYID);

  Dir->MessageCount+=snapcount;
  Dir->MaxChainVal+=snapcount;

  if(DestructivelyWriteDirectoryHead(Dir))
    AMS_RETURN_ERRCODE(errno, EIN_LSEEK, EVIA_GETSNAPSHOTBYID);

  return 0;
}

/*
 * unpack the selected files from the pak file
 */
void unpak_matched_files()
{
  char *entry=hed.flags;
  struct MS_Directory *Dir;
  CloseDirsThatNeedIt();

  MSCALL(ReadOrFindMSDir(pgl.longname,&Dir,MD_APPEND),
	 (x,"can't open folder '%s'",pgl.longname));

  FOR_ALL_HEADERS("unpack each file",ahed) {
    if(*(entry)) {
      if(unpak_1file(ahed))
	*entry=FALSE;		/*was already present, we didn't add it*/
    } else
      unpak_skip1();
    entry++;
  } END_FOR_ALL_HEADERS;

  loose_unmatched();
  if(hed.count!=0) {
    int er=AppendSnapshots(Dir,hed.data,hed.count);
    if(er!=0)
      fprintf(stderr,"Folder '%s' is damanged and needs to be reconstructed.\n");
    MSCALL(er,(x,"can't rewrite '%s'",pgl.longname));
  }
  CloseMSDir(Dir, MD_APPEND);

#ifdef RUBBISH
  MarkQuietlyInProgress(pgl.longname);
  UnmarkInProgress(pgl.longname);
#endif
}

/*
 * unpack messages
 */
void arunpack()
{
  char *amsfolder;
  char *dst;
  int crdir=FALSE;
  amsfolder=p_really_get_token();
  if(STREQ(amsfolder,"-create")) {
    crdir=TRUE;
    amsfolder=p_really_get_token();
  }
  dst=p_really_get_token();
  p_get_token();
  parse_write_folder(crdir,amsfolder);
  set_input_file(dst);
  if(pgl.cons) {
    fprintf(pgl.cons,"unpack to folder:%s\n",pgl.shortname);
    fprintf(pgl.cons,"unpack from file:%s\n",pgl.pakname);
  }
  read_headers_from_file();
  match_headers();

  unpak_matched_files();
  close_pak();

  if(pgl.cons)
    fprintf(pgl.cons,"done.\n");
}

/*
 * help when no arguments given on command line
 */
void simple_help()
{
  fprintf(stderr,"%s:see ~aw0g/amsar/amsar.doc for now\n",pname);
  exit(0);
}

/*
 * help on a topic
 */
void arhelp()
{
  fprintf(stderr,"%s:see ~aw0g/amsar/amsar.doc for now\n",pname);
  exit(0);
}

/*
 * processes options proceeding the command
 */
char *read_options()
{
  char *command=p_really_get_token();
  pgl.anycase=TRUE;
  pgl.cons=stdout;
  while(TRUE) {
    if(STREQ(command,"pack")) {
      if(STREQ(p_peek_token(1),"-"))
	if(pgl.cons!=NIL)
	  pgl.cons=stderr;
      return command;
    }
    if(STREQ(command,"-quiet")){
      pgl.cons=NIL;
      command=p_really_get_token();
    }else if(STREQ(command,"-yescase")){
      pgl.anycase=FALSE;
      command=p_really_get_token();
    }else
      return command;
  }
}

/*
 * read and process all input lines
 */
void read_all_input(argc,argv)
int argc;
char **argv;
{
  char *command;
  parse_from(argc,argv);
  command=read_options();
  if(pgl.cons!=NIL)
    fprintf(pgl.cons,"%s:$Revision: 1.5 $\n",pname);
  if(STREQ(command,"pack"))
    arpack();
  else if(STREQ(command,"list"))
    arlist();
  else if(STREQ(command,"dir"))
    ardir();
  else if(STREQ(command,"unpack"))
    arunpack();
  else if(STREQ(command,"help"))
    arhelp();
  else if(STREQ(command,"delete"))
    ardelete();
  else if(STREQ(command,"create"))
    arcreate();
  else if(STREQ(command,"rmfolder"))
    ar_rmfolder();
  else
    BARF((x,"unrecognized command '%s'",command));
}

void Machine_HandleClientSignal()
{
}

char *SnapVersionString="0.0";
int MSDebugging = 0;

BizarreError(text, level)
char *text;
int level;
{
  if (level >= ERR_WARNING)
    fprintf(stderr,"%s\n",text);
  else
    BARF((x,"bizzare ams error '%s'",text));
}

/*
 * fire up ams and do all its initialization
 */
void start_ams(argc,argv)
int argc;
char **argv;
{
  int bigest_buf;
  MSCALL(MS_Initialize(&bigest_buf,FALSE /*using snap*/ ),
	 (x,"can't initialize messgae server"));
#ifdef RUBBISH
  amsconfig(argc,argv, "amsar");
extern char CUI_VersionString[];
extern int CUI_SnapIsRunning;
  if (CUI_Initialize(NIL,NIL))
    BARF((x,"cui initialize failed"));
  CUI_SetClientSignalHandler(HandleCUISignal);
  CUI_SetClientVersion("amsar");
#endif
}

/*
 * ams archiver
 */
int main(argc,argv)
int argc;
char **argv;
{
  pname=argv[0];
  if(argc<2)
    simple_help();
  start_ams(argc,argv);
  read_all_input(argc-1,++argv);
}
