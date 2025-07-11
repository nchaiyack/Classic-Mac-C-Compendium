/*
 * SoftKiss - software emulation of a tnc in tnc mode
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1992
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#include "sfk_core.h"
#include "sfk_core_private.h"
#include "driver_shell.h"
#include <Serial.h>

#define STNC_put(xx_p,xx_str) \
	sfk_cu_write(&p->K_IN.io_buf,(xx_str),strlen(xx_str))

/*
 * return an estimate of the number of bytes to read
 */
static long soft_tnc_bytes_available(sfk_prt_pt p)
{
	return sfk_cu_read_size(&p->K_IN.io_buf);
}

/*
 * read some data from packets into the serial input buffer
 */
static int soft_tnc_bytes_read(sfk_prt_pt p,int is_async)
{
	register IOParam *ipb= p->K_IN.kp_pb;
	if(ipb==0)
		return 1;	//no serial buffer to read into
	ipb->ioActCount=0;  //read zero so far ???
	ipb->ioActCount+=sfk_cu_read(
		&p->K_IN.io_buf,
		ipb->ioBuffer+ipb->ioActCount,
		ipb->ioReqCount-ipb->ioActCount);

/*
 * done for one of two reasons, no more data to send
 * or no more room to put it.  In eithor case
 * let the pending read request finish
 * since something was read
 */
done:
	if(is_async)
		sfk_mark_read_done(p,0);	//read all we need to
	else {
		p->K_IN.kp_pb->ioResult=0;
		p->K_IN.kp_pb=0;
	}
	return 0;
}

/*
 * echo a character up arrowifiying control chars
 */
static void soft_tnc_echo_quoted_ch(sfk_prt_pt p,char ch)
{
	if(ch=='\t')ch=' ';
	if(ch<' ') {
		sfk_cu_write(&p->K_IN.io_buf,"^",1);
		ch+='@';
		sfk_cu_write(&p->K_IN.io_buf,&ch,1);
	} else
		sfk_cu_write(&p->K_IN.io_buf,&ch,1);
}

/*
 * print a command prompt and echo line buffer text
 */
static void soft_tnc_prompt(sfk_prt_pt p)
{
	long len=p->cmd_buf_len;
	char *s=p->cmd_buf;
	p->cmd_did_prompt=TRUE;
	STNC_put(p,sfk_PN(p->cmd_port)->port_name);
	STNC_put(p,SFK_TEXT(53));
	while(--len>=0)		//echo command line so far
	  soft_tnc_echo_quoted_ch(p,*s++);
}

#define CTL(xx_ch) ((xx_ch)-'@')

/*
 * check last character that caused read_line to wake up
 * return FALSE if a compleate line has been read
 */
static int soft_tnc_read_action(sfk_prt_pt p)
{
	char ch= *(p->cmd_buf+(p->cmd_buf_len-1));
	ch&=0x7f;
	switch(ch) {
	  case 0x7f:
	  case CTL('H'):
	  	p->cmd_buf_len--;
	  	if(p->cmd_buf_len==0)break; //^H at start of line
	  	soft_tnc_echo_quoted_ch(p,ch);
	  	p->cmd_buf_len--;
	  	ch= *(p->cmd_buf+p->cmd_buf_len);
	  	if(ch<' ') //erase a control character by spacing over it???
	  		STNC_put(p,"\010\010  \010\010");
	  	else
	  		STNC_put(p,"\010 \010");
	  	break;
	  case CTL('U'):
	  case CTL('C'):
	  	p->cmd_buf_len=1;
	  case CTL('R'): // fall into
	  	soft_tnc_echo_quoted_ch(p,ch);
	  	p->cmd_did_prompt=FALSE;
	  	STNC_put(p,SFK_TEXT(59));
	  	p->cmd_buf_len--;
	  	break;
	  case CTL('J'):
	  case CTL('M'):
	  	p->cmd_buf_len--;
	  	return FALSE;
	  	break;
	  default:
	  	soft_tnc_echo_quoted_ch(p,ch);
	}  
	return TRUE;
}

/*
 * print a c string converting \n to \n\r for terminal output
 */
static void soft_tnc_print_c_string(sfk_prt_pt p,char *s)
{
  char *tail;
  long slen;
  if((*s)==0)
  	return;
  while((tail=strchr(s,'\n'))!=0) {
	sfk_cu_write(&p->K_IN.io_buf,s,(tail-s)+1);
	s=tail+1;
	STNC_put(p,SFK_TEXT(60));
  }
  slen=strlen(s);
  sfk_cu_write(&p->K_IN.io_buf,s,slen);
  if(slen>1)
	STNC_put(p,SFK_TEXT(59)); 	//force end of line
}

/*
 * execute the passed command
 */
static void soft_tnc_execute_cmd(sfk_prt_pt p,char *execute_me)
{
	sfk_io_record cmd;
	char out_buf[4000];
	sfk_INIT_CPB(cmd,-1000,sfk_CMD_msg);
	out_buf[0]=0;
	sfk_parse_from(&cmd,execute_me,strlen(execute_me),out_buf,sizeof(out_buf),p->cmd_port);
	sfk_control(&cmd);
	p->cmd_port=cmd.do_this.cknd.sfk_msg.portno;
	if(out_buf[0]!=0)
	  soft_tnc_print_c_string(p,out_buf);
}

/*
 * process command lines in tnc mode
 */
static void soft_tnc_cmd(sfk_prt_pt p)
{
	long read_len;
	if(p->sfk_IVAR(kiss))
		return;		/*no command lines in kiss mode*/
	if(!p->cmd_did_prompt)
		soft_tnc_prompt(p);
	read_len=sfk_cu_read_line(&p->K_OUT.io_buf,
		p->cmd_buf+p->cmd_buf_len,
		p->cmd_buf_limit-p->cmd_buf_len);
	if(read_len==0)
		return;
	//all but the last character are safe to do as a chunk
	sfk_cu_write(&p->K_IN.io_buf,p->cmd_buf+p->cmd_buf_len,sfk_imax(0,read_len-1));
	p->cmd_buf_len+=read_len;
	if(soft_tnc_read_action(p))	//handle character tha caused end of line
		return;					//don't have a full line yet
	*(p->cmd_buf+p->cmd_buf_len)=0;
	p->cmd_did_prompt=FALSE;
	STNC_put(p,SFK_TEXT(59));
	p->cmd_buf_len=0;
	soft_tnc_execute_cmd(p,p->cmd_buf);
}

/*
 * called periodicly
 */
static void soft_tnc_tick(sfk_prt_pt p)
{
	soft_tnc_bytes_read(p,TRUE);
	soft_tnc_cmd(p);
	soft_tnc_bytes_read(p,TRUE);
}

/*
 * fake driver for serial input in tnc mode
 */
static short soft_tnc_in(CntrlParam *pb,DCtlPtr de,short op,void *extra)
{
	short csCode=pb->csCode;
	register IOParam *ipb= (IOParam *)pb;
	sfk_prt_pt p=extra;
	switch(op) {
	case sh_OPEN:
		if(sfk_open_me(de))
			return ioErr;
		break;
	case sh_CLOSE:
		sfk_close_me(de);
		break;
	case sh_STATUS:
		if(csCode==2) { //SetGetBuf
			long *iocnt= (long*)&pb->csParam;
			*iocnt=soft_tnc_bytes_available(p);
		} else if(csCode==8) { //SetStatus
			SerStaRec st;
			memset(&st,0,sizeof(st));
			st.rdPend= (soft_tnc_bytes_available(p)>0);
			memcpy(&pb->csParam,&st,5);	//6? instead of 5?
		}
		break;
	case sh_PRIME:
		p->K_IN.kp_pb=(IOParam *)pb;
		return soft_tnc_bytes_read(p,FALSE);
	case sh_CONTROL:
		if (csCode==1)
			sfk_mark_read_done(p,-27);
		else if (csCode==accRun)
			soft_tnc_tick(p);
		break;
	default:
		DBO_fprintf((&p->dbo,"soft_tnc op=%d csCode=%d\n",op,pb->csCode));
		break;
	}
	return 0;
}

/*
 * fake driver for serial output in tnc mode
 */
static short soft_tnc_out(CntrlParam *pb,DCtlPtr de,short op,void *extra)
{
	register IOParam *ipb= (IOParam *)pb;
	sfk_prt_pt p=extra;
	switch(op) {
	case sh_OPEN:
		if(sfk_open_me(de))
			return ioErr;
		break;
	case sh_CLOSE:
		sfk_close_me(de);
		break;
	case sh_PRIME:
		sfk_cu_write(&p->K_OUT.io_buf,ipb->ioBuffer,ipb->ioReqCount);
		break;
	default:
		DBO_fprintf((&p->dbo,"soft_tnc_out csCode=%d\n",pb->csCode));
		break;
	}
	return 0;
}

/*
 * allocate memory for soft_tnc
 */
void sfk_allocate_soft_tnc_memory(sfk_iio_pt cmd,sfk_prt_pt p)
{
#define IO_BUF_SIZE (5000)
	if(p->K_IN.io_buf.cu_size==0)
		sfk_cu_init(&p->K_IN.io_buf,IO_BUF_SIZE);
	if(p->K_OUT.io_buf.cu_size==0)
		sfk_cu_init(&p->K_OUT.io_buf,IO_BUF_SIZE);
	if(p->cmd_buf_limit==0) {
		p->cmd_port=p->pnum;
		p->cmd_buf_limit=IO_BUF_SIZE;
		p->cmd_buf_len=0;
		p->cmd_buf=sfk_malloc(p->cmd_buf_limit);
	}
	if((p->K_IN.io_buf.cu_buf==0)||
	   (p->K_OUT.io_buf.cu_buf==0)||
	   (p->cmd_buf_limit==0))
	  sfk_parse_fail(cmd,TEXT_NUM(28),0);
}

/*
 * install soft_tnc as the current fake serial driver
 */
void sfk_soft_tnc_install(sfk_iio_pt cmd,sfk_prt_pt p)
{
	sfk_fake_serial_install(cmd,p,
		&p->K_IN,(long)&soft_tnc_in,
		&p->K_OUT,(long)&soft_tnc_out);
	sfk_allocate_soft_tnc_memory(cmd,p);
}

/*
 * find the low memory variable with this ports configuration info
 */
static uint8 *find_port(sfk_prt_pt p)
{
	uint8 *port_config= (uint8 *)0x0290; /*assume port a*/
	if(p->pnum!=1)		/*check PortAUse and PortBUse*/
  	  port_config= (uint8 *)0x0291;	/*port b use byte*/
  	return port_config;
}

/*
 * mark the port as being free
 */
static void release_port(sfk_prt_pt p)
{
	volatile uint8 *port_config=find_port(p);
	volatile uint8 *xSPConfig = (uint8 *)0x1FB;
	if(p->pnum==0)
		(*xSPConfig)= ((*xSPConfig)&0xf0)|2;
	else
		(*xSPConfig)=  ((*xSPConfig)&0x0f)|0x20;
	(*port_config)= 0xFF;
}

/*
 * make sure the scc hardware isn't in use by appletalk or async
 */
static aquire_port(sfk_iio_pt cmd,sfk_prt_pt p)
{
	volatile uint8 *port_config=find_port(p);
	volatile uint8 *xSPConfig = (uint8 *)0x1FB;
	uint8 sp_byte=(*xSPConfig);
	if(p->pnum!=0)
		sp_byte>>=4;
	if(!p->sfk_IVAR(force_port)) {
	  if((sp_byte&0x0F)!=2)
	    sfk_parse_fail(cmd,TEXT_NUM(32),0);
	  if((*port_config)!=0x0FF)
	    sfk_parse_fail(cmd,TEXT_NUM(33),0);
	}
	if(p->pnum==0)
		(*xSPConfig)= ((*xSPConfig)&0xf0)|2;
	else
		(*xSPConfig)=  ((*xSPConfig)&0x0f)|0x20;
	(*port_config)= (*xSPConfig);
}

/*
 * turn off all usage of passed port
 */
static void soft_tnc_off(sfk_iio_pt cmd,sfk_prt_pt p)
{
	fake_serial_off(p);
	p->sfk_IVAR(soft_tnc)=FALSE;
	release_port(p);
}

/*
 * turn on port in tnc emulation mode
 */
static void soft_tnc_on(sfk_iio_pt cmd,sfk_prt_pt p)
{
	aquire_port(cmd,p);
	sfk_soft_tnc_install(cmd,p);
	fake_serial_on(cmd,p);
	STNC_put(p,SFK_TEXT(54));
	STNC_put(p,__DATE__);
	STNC_put(p,SFK_TEXT(55));
	STNC_put(p,__TIME__);
	STNC_put(p,SFK_TEXT(59));
	STNC_put(p,SFK_TEXT(56));
	STNC_put(p,sfk_gl.vers);
	STNC_put(p,SFK_TEXT(59));
	p->sfk_IVAR(soft_tnc)=TRUE;
}

/*
 * change tnc mode to on or off
 */
void sfk_change_soft_tnc(sfk_iio_pt cmd,int new_setting)
{
	if(cmd->pqx.p->sfk_IVAR(soft_tnc)==new_setting)
		return;
	if(cmd->pqx.p->pnum>1)
	  sfk_parse_fail(cmd,TEXT_NUM(39),0);
	if(is_serial_busy(cmd->pqx.p))
	  sfk_parse_fail(cmd,TEXT_NUM(40),0);
	if(new_setting)
		soft_tnc_on(cmd,cmd->pqx.p);
	else
		soft_tnc_off(cmd,cmd->pqx.p);
}

