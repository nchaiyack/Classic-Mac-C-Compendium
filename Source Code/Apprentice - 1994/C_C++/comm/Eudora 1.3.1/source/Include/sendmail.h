/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * declarations for dealing with sendmail
 ************************************************************************/
int StartSMTP(UPtr serverName, short Port);
int SendMessage(TOCType **tocH,int sumNum);
int SMTPError(void);
int EndSMTP(void);
MessHandle SaveB4Send(TOCHandle tocH,short sumNum);
int DoRcptTos(MessType **messH, Boolean chatter);
int DoRcptTosFrom(TEHandle teh, Boolean chatter);
int TransmitMessage(MessType **messH, Boolean chatter);
void TimeStamp(TOCHandle tocH,short sumNum,uLong when,long delta);
void PtrTimeStamp(MSumPtr sum,uLong when,long delta);
int GetReply(UPtr buffer, int size,Boolean chatter);
int SendBodyLines(UHandle  text,long length,long offset,Boolean doWrap,Boolean forceLines,
									short *lineStarts,short nLines,Boolean partial);
void BuildDateHeader(UPtr buffer,long seconds);
typedef enum {
				SysStatCode=211,
				HelpCode=214,
				ReadyCode=220,
				CloseCode,
				OkCode=250,
				ForwardCode=251,
				
				StartInputCode=354,
				
				NoServiceCode=421,
				BoxBusyCode=450,
				LocalErrCode,
				SysFullCode,
				
				SyntaxCode=500,
				ArgsBadCode,
				CmdUnImpCode,
				OrderBadCode,
				ArgUnImpCode,
				NoBoxCode=550,
				YouForwardCode,
				BoxFullCode,
				BoxBadCode,
				PuntCode,
				
				TransErr=601,
				RecvErr,
				ReplyErr
} SMErrEnum;
