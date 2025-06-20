/*
 * SoftKiss dispatch for control calls
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#include "sfk_core.h"
#include "sfk_core_private.h"
#include "driver_shell.h"

static char inited=FALSE;

/*
 * is one time startup init done
 */
int sfk_init_done(void)
{
	return inited;
}

/*
 * a control call to the us has failed
 * unwind the stack and return the error
 */
void sfk_control_fail(sfk_iio_pt iicmd,short io_err,short iio_err)
{
	if(io_err==0)io_err= -1;
	if(iio_err==0)iio_err= 1;
	if((iicmd->uio!=0)&&(iicmd->uio->csCode>=sfk_CMD_base)) {
		iicmd->uio->do_this.sfk_hdr.sys_err_code=io_err;
		iicmd->uio->do_this.sfk_hdr.sfk_err_code=iio_err;
	}
	longjmp(iicmd->err_throw,1);
}

/*
 * dispatch to handle opening the driver
 */
int sfk_init(void)
{
	sfk_io_record cmd;
	sfk_INIT_CPB(cmd,0,sfk_CMD_run);
	sfk_control(&cmd);
	if(inited)
		return noErr;
	return -ioErr;
}

/*
 * dispatch to handle closing the driver
 */
int sfk_uninit(sfk_io_record_pt pb)
{
	sfk_io_record cmd;
	char out_buf[20];
	char *cmd_text=SFK_TEXT(57);
	sfk_INIT_CPB(cmd,0,sfk_CMD_msg);
	sfk_parse_from(&cmd,cmd_text,strlen(cmd_text),out_buf,sizeof(out_buf),0);
	sfk_control(&cmd);
	return noErr;
}

/*
 * dispatch to handle a control call depending on command chosen
 */
int sfk_control(sfk_io_record_pt pb)
{
	sfk_iio cmd;
	cmd.uio=pb;
	if(pb->csCode>=sfk_CMD_base) {
		pb->do_this.sfk_hdr.sys_err_code=0;
		pb->do_this.sfk_hdr.sfk_err_code=0;
	}
	if(setjmp(cmd.err_throw)!=0)
		return ioErr;

	if(!inited)
		sfk_init_commands(&cmd);
	inited=TRUE;

	switch(pb->csCode) {
		case sfk_CMD_run:
			sfk_tick(&cmd);
			break;
		case sfk_CMD_attach:
			sfk_control_fail(&cmd,0,sfk_NOT_YET_IMPLEMENTED);
			break;
		case sfk_CMD_detach:
			sfk_control_fail(&cmd,0,sfk_NOT_YET_IMPLEMENTED);
			break;
		case sfk_CMD_msg:
			sfk_parse_command(&cmd);
			break;
		case sfk_CMD_write:
			sfk_write(&cmd);
			break;
		case sfk_CMD_read:
			sfk_read(&cmd);
			break;
		case sfk_CMD_goodbye:
		case sfk_CMD_killio:
			break;
		default:
				return -17;		/*don't like this control call*/			
	}
	return noErr;
}
