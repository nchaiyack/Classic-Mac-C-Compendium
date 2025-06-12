/*
 * SoftKiss utilities for sending driver messages
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#include "sfk_core.h"

/*
 * setup an io record to send the driver a message
 */
void sfk_parse_from(sfk_io_record_pt cmd,
	char *in_str,
	int in_len,
	char *out_str,
	int out_len,
	int def_port)
{
	cmd->do_this.cknd.sfk_msg.imsg.sfk_in_cnt=in_len;
	cmd->do_this.cknd.sfk_msg.imsg.sfk_out_cnt=0;
	cmd->do_this.cknd.sfk_msg.imsg.sfk_str=in_str;
	cmd->do_this.cknd.sfk_msg.omsg.sfk_in_cnt=out_len;
	cmd->do_this.cknd.sfk_msg.omsg.sfk_out_cnt=0;
	cmd->do_this.cknd.sfk_msg.omsg.sfk_str=out_str;
	cmd->do_this.cknd.sfk_msg.portno=def_port;
}
