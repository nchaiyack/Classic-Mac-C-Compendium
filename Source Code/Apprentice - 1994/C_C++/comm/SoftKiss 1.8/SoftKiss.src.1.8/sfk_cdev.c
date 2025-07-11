/*
 * SoftKiss control panel
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 */

#include <cdev.h>
#include "sfk_core.h"
#include "sfk_core_private.h"
#include "sfk_drvr_find.h"
#include "sfk_os_preserve.h"

/*
 * Think has dp as a dialog peek
 * with strict checking on the compiler complains on use of
 * dp so we use dpp
 */
#define dptr ((DialogPtr)dp)

/*
 * number of ports handled by cdev
 */
#define CDEV_PORTS (2)

/*
 * info for each ditem
 */
struct a_ditem_R {
	short type;
	Handle ctl;
	Rect box;
};
typedef struct a_ditem_R a_ditem,*a_ditem_pt;

/*
 * indexes into ditl
 */
enum {
	fn_a_off=1,
	fn_a_kiss=2,
	fn_a_tnc=3,
	fn_a_text=4,

	fn_b_off=5,
	fn_b_kiss=6,
	fn_b_tnc=7,
	fn_b_text=8,

	fn_line1=9,
	fn_line2=10,

	fn_title=11,
	fn_modem_text=12,
	fn_printer_text=13,
	fn_LAST=13
};

/*
 * allocate some vars with newptr so they don't float around and
 * can be passed to system calls
 */
struct fs_t_R {
	a_ditem items[fn_LAST+1];
	Pattern cdev_gray;	/*no quickdraw vars to access gray*/

	short ref_num;		/*SoftKiss driver number*/
	int active;			/*are in the front*/
	int port_err[CDEV_PORTS]; /*non zero if last control call on port failed*/
	int init_done;
	int port_a_mode;	/*setting for a port*/
	int port_b_mode;	/*setting for b port*/
	dyn_text dt;		/*dynamic text info*/
};
typedef struct fs_t_R fs_t,*fs_t_pt;

struct SoftKiss : cdev {
	fs_t *fs;			/*fixed allocated stuff*/
	void set_item_value(int item,int setting);
    void set_item_hilite(int item,int setting);
	void find_driver(void);
	void set_display(void);
	void working_SetIText(int item,char *new_text);
	void ReadItems(void);
	void send_cmd(int portno,char *msg);
	void read_driver_status(char *out_buf,long out_buf_size);
	void read_1_driver_status(int portno,char *out_buf,long out_buf_size);
	void send_msg(int portno,char *msg,char *out_buf,long out_buf_size);
	void change_mode(int *old_mode,int new_mode);
	void SetVersion(void);

	// Messages from control panel
	void 	Idle(void);
	void	Init(void);
	void	Close(void);
	void	Activate(void);
	void	Update(void);
	void	ItemHit(short);
	void	Deactivate(void);
	char	*CTXT(unsigned short idx);
};

/*
 * return a string from a resource
 */
char *SoftKiss::CTXT(unsigned short idx)
{
	return (char *)
	  (fs->dt.tptr+(*(fs->dt.tstrs+idx)));
}

/*
 *  Runnable - should the cdev appear in the Control Panel?
 *
 *  This implements the "macDev" message.
 *
 */
Boolean Runnable()
{
	return true;
}


/*
 *  New - create the cdev object
 *
 */
cdev *New()
{
	return(new(SoftKiss));
}

/*
 * set ref_num for the driver refrence number
 */
void SoftKiss::find_driver()
{
 	short err;
 	memset(fs->port_err,-1,sizeof(fs->port_err));
	if((fs->ref_num=sfk_drvr_find(sfk_driver_name,0L))==0)
		return;
 	err=OpenDriver(sfk_driver_name,&fs->ref_num);
 	if(err==0)
 		memset(fs->port_err,0,sizeof(fs->port_err));
 	else
 		fs->ref_num=0;
}

/*
 * send the passed message to the driver
 */
void SoftKiss::send_msg(int portno,char *msg,char *out_buf,long out_buf_size)
{
  sfk_io_record cmd;
  int iErr;
  sfk_INIT_CPB(cmd,fs->ref_num,sfk_CMD_msg);
  out_buf[0]=0;
  sfk_parse_from(&cmd,msg,strlen(msg),out_buf,out_buf_size,portno);
  iErr = PBControlSync((ParmBlkPtr)&cmd);
  fs->port_err[portno]=iErr;
  if(iErr!=0)
  	working_SetIText(portno?fn_a_text:fn_b_text,out_buf);
}

/*
 * send the passed message to the driver
 * Then read then update the controls to match the driver status
 */
void SoftKiss::send_cmd(int portno,char *msg)
{
  char out_buf[400];
  if(fs->ref_num==0) {
  	SysBeep(20);
  	return;
  }
  send_msg(portno,msg,out_buf,sizeof(out_buf));
  if(fs->port_err[portno]==0)	
  	read_driver_status(out_buf,sizeof(out_buf));
}

/*
 * change a port mode and update the display if it really changed
 */
void SoftKiss::change_mode(int *old_mode,int new_mode)
{
	char buf[3000];
	if((*old_mode)==new_mode)
		return;
	*old_mode=new_mode;
	set_display();
	switch(fs->port_a_mode) {
		case fn_a_off:
		  strcpy(buf,CTXT(1));
		  break;
		case fn_a_kiss:
		  strcpy(buf,CTXT(2));
		  break;
		case fn_a_tnc:
		  strcpy(buf,CTXT(3));
		  break;
	}
	working_SetIText(fn_a_text,buf);
	switch(fs->port_b_mode) {
		case fn_b_off:
		  strcpy(buf,CTXT(4));
		  break;
		case fn_b_kiss:
		  strcpy(buf,CTXT(5));
		  break;
		case fn_b_tnc:
		  strcpy(buf,CTXT(6));
		  break;
	}
	working_SetIText(fn_b_text,buf);
}

/*
 * read the device driver status of one port and update the display
 */
void SoftKiss::read_1_driver_status(int portno,char *out_buf,long out_buf_size)
{
	char *s=out_buf;
	int kiss=FALSE;
	int soft=FALSE;
	send_msg(portno,CTXT(7),out_buf,out_buf_size);
	if((s=strchr(s,' '))!=0) {
		s++;
		kiss= (*s)=='t';
		if((s=strchr(s,' '))!=0)
			soft= (*(s+1))=='t';
	}
	if(portno==sfk_PRINTER_PORT)
		if(!soft)
			change_mode(&fs->port_b_mode,fn_b_off);
		else if(kiss)
			change_mode(&fs->port_b_mode,fn_b_kiss);
		else
			change_mode(&fs->port_b_mode,fn_b_tnc);
	else
		if(!soft)
			change_mode(&fs->port_a_mode,fn_a_off);
		else if(kiss)
			change_mode(&fs->port_a_mode,fn_a_kiss);
		else
			change_mode(&fs->port_a_mode,fn_a_tnc);
}

/*
 * read the device driver status and update the display
 */
void SoftKiss::read_driver_status(char *out_buf,long out_buf_size)
{
	read_1_driver_status(sfk_MODEM_PORT,out_buf,out_buf_size);
	read_1_driver_status(sfk_PRINTER_PORT,out_buf,out_buf_size);
}

/*
 * idle message, update status area
 */
void SoftKiss::Idle()
{
	static long next_update=0;
	long cur_ticks=TickCount();
	if(fs->ref_num==0)
		return;
	if(cur_ticks<next_update)return;
	next_update=cur_ticks+20;
	{
		char out_buf[400];
		read_driver_status(out_buf,sizeof(out_buf));
	}
}

/*
 * read in item list
 */
void SoftKiss::ReadItems(void)
{
  int i;
  for(i=1;i<=fn_LAST;i++) {
  	a_ditem_pt ai= &fs->items[i];
  	GetDItem(dptr, lastItem + i, &ai->type, &ai->ctl, &ai->box);
  	}
}

/*
 * SetIText doesn't seem to invalidate....
 */
void SoftKiss::working_SetIText(int item,char *new_text)
{
  unsigned char buf[256];
  int slen=strlen(new_text);
  GrafPtr old_port;
  slen=sfk_imin(slen,254);
  buf[0]=slen;
  memcpy(buf+1,new_text,slen);
//  GetPort(&old_port);
//  SetPort(&dp->port);
  SetIText(fs->items[item].ctl,buf);
  InvalRect(&fs->items[item].box);
//  SetPort(old_port);
}

/*
 * set a checkbox as it is displayed
 */
void SoftKiss::set_item_hilite(int item,int setting)
{
	HiliteControl((ControlHandle)fs->items[item].ctl,(setting)?1:0);
}

/*
 * set a checkbox as it is displayed
 */
void SoftKiss::set_item_value(int item,int setting)
{
	SetCtlValue((ControlHandle)fs->items[item].ctl,(setting)?1:0);
	if((fs->ref_num!=0)&&(fs->active))
		HiliteControl((ControlHandle)fs->items[item].ctl,0);
	else
		HiliteControl((ControlHandle)fs->items[item].ctl,255);
}

/*
 * set the user interface to match fs.cstate
 */
void SoftKiss::set_display()
{
	set_item_value(fn_a_off,fs->port_a_mode==fn_a_off);
	set_item_value(fn_a_kiss,fs->port_a_mode==fn_a_kiss);
	set_item_value(fn_a_tnc,fs->port_a_mode==fn_a_tnc);

	set_item_value(fn_b_off,fs->port_b_mode==fn_b_off);
	set_item_value(fn_b_kiss,fs->port_b_mode==fn_b_kiss);
	set_item_value(fn_b_tnc,fs->port_b_mode==fn_b_tnc);
}

/*
 * allocate a dynamic text block
 */
static void *dt_aloc(unsigned long alloc_size)
{
	return NewPtr(alloc_size);
}

/*
 * free a dynamic text block
 */
static void dt_free(void *buf)
{
	DisposPtr(buf);
}

/*
 *  SoftKiss::Init
 *
 */
void SoftKiss::Init()
{
	inherited::Init();
	fs=(fs_t_pt)NewPtr(sizeof(*fs));
	memset(fs,0,sizeof(*fs));
	ReadItems();
	GetIndPattern(fs->cdev_gray,sysPatListID,24);
	fs->active=TRUE;
	find_driver();
	fs->port_a_mode=0;
	fs->port_b_mode=0;		//make sure it updates
	fs->init_done=TRUE;
	if(!sfk_dt_load(1026,&fs->dt,dt_aloc,dt_free))
		Error(cdevResErr);
	SetVersion();
	Idle();
}


/*
 * at open time set the version text to match the vers resource
 */
void SoftKiss::SetVersion()
{
	Handle vers=Get1Resource('vers',1);
	unsigned char ptext[256];
	unsigned char len;
	char *vers_str;
	char *dst;
	if(vers==0)return;
	vers_str= (*vers)+6;
  	GetIText(fs->items[fn_title].ctl,ptext);
  	if(ptext[ptext[0]]!='#')
  		return;
  	len= (*vers_str++)&0x0F;
  	memcpy(&ptext[ptext[0]],vers_str,len);
  	ptext[0]+=len;
  	ptext[ptext[0]]=0;
  	working_SetIText(fn_title,(char*)&ptext[1]);
}

/*
 *  SoftKiss::Close
 *
 */
void SoftKiss::Close()
{
	if(fs!=0)
		DisposPtr((void*)fs);
	fs=0;
	inherited::Close();
}

/*
 *  SoftKiss::Update
 *
 */
void SoftKiss::Update()
{
	a_ditem_pt aline;
	int i;
	PenNormal();
	PenPat(fs->cdev_gray);
	for(i=fn_line1;i<=fn_line2;i++) {
		aline= &fs->items[i];
		MoveTo(aline->box.left+1,aline->box.top);
		LineTo(aline->box.right,aline->box.top);
	}
}

void SoftKiss::Activate()
{
	fs->active=TRUE;
	set_display();
}


void SoftKiss::Deactivate()
{
	fs->active=FALSE;
	set_display();
}


void SoftKiss::ItemHit(item)
short item;
{
	Point where;

	switch(item) {
	case fn_a_off:
		send_cmd(sfk_MODEM_PORT,CTXT(8));
		break;
	case fn_a_kiss:
		send_cmd(sfk_MODEM_PORT,CTXT(9));
		break;
	case fn_a_tnc:
		send_cmd(sfk_MODEM_PORT,CTXT(10));
		break;
	case fn_b_off:
		send_cmd(sfk_PRINTER_PORT,CTXT(11));
		break;
	case fn_b_kiss:
		send_cmd(sfk_PRINTER_PORT,CTXT(12));
		break;
	case fn_b_tnc:
		send_cmd(sfk_PRINTER_PORT,CTXT(13));
		break;
	default:
		break;
	}
	set_display();
}
