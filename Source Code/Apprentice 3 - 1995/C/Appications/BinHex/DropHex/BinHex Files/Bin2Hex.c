#include "Bin2Hex.h"





/*	local defines	*/
#define	kHeaderMsg			"(This file must be converted with BinHex 4.0)"
#define	kHeaderMsgLen		sizeof(kHeaderMsg) - 1
#define	kMaxHeaderLength	89 + kHeaderMsgLen
#define	kForkDone			-1
#define	kCR					'\015'
#define	kLF					'\012'





extern unsigned short CRCMagic[256];
static unsigned char Bin2HexTranslation[64] = {
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x30, 0x31, 0x32,
	0x33, 0x34, 0x35, 0x36, 0x38, 0x39, 0x40, 0x41,
	0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x50, 0x51, 0x52,
	0x53, 0x54, 0x55, 0x56, 0x58, 0x59, 0x5A, 0x5B,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x68,
	0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x70, 0x71, 0x72 };





short NewBin2Hex(Bin2HexParamPtr pb)
{
	long	zero = 0;
	
		
	//	do buffer size sanity check
	if (pb->outBufferSize < kMaxHeaderLength)
		return -1;
	
	// add BinHex preamble
	pb->context.out = (unsigned char*)pb->outBuffer + kHeaderMsgLen;
	BlockMoveData(kHeaderMsg,pb->outBuffer,kHeaderMsgLen);
	*pb->context.out++ = kCR;
	if (pb->addLFs)
		*pb->context.out++ = kLF;
	
	// add opening delimiter
	*pb->context.out++ = ':';
	pb->context.line = 63;
	
	// initialize encoding engine
	pb->context.state = -1;
	pb->context.bits = 0;
	pb->context.check = 0;
	pb->context.dfState = pb->info.dfLength;
	pb->context.rfState = pb->info.rfLength;
	
	//	encode file information header
	EncodeBuffer(pb,(unsigned char*)pb->info.name,pb->info.name[0]+1);
	EncodeBuffer(pb,(unsigned char*)&zero,1);
	EncodeBuffer(pb,(unsigned char*)&pb->info.type,4);
	EncodeBuffer(pb,(unsigned char*)&pb->info.creator,4);
	EncodeBuffer(pb,(unsigned char*)&pb->info.flags,2);
	EncodeBuffer(pb,(unsigned char*)&pb->info.dfLength,4);
	EncodeBuffer(pb,(unsigned char*)&pb->info.rfLength,4);
	EncodeCheckSum(pb);
	
	//	check for zero length data fork
	if (!pb->context.dfState)
	{
		EncodeCheckSum(pb);
		pb->context.dfState = kForkDone;
		
		//	check for zero length resource fork
		if (!pb->context.rfState)
		{
			EncodeCheckSum(pb);
			pb->context.rfState = kForkDone;
			
			//	fix 8to6 rounding problems
			if (pb->context.state+1)
				EncodeBuffer(pb,(unsigned char*)&zero,1);
			
			//	add ending delimiter
			*pb->context.out++ = ':';
			*pb->context.out++ = kCR;
			if (pb->addLFs)
				*pb->context.out++ = kLF;
		}
	}
	
	// return length of encoded data
	pb->outAmount = (long)(pb->context.out - (unsigned char*)pb->outBuffer);
	return noErr;
}





short DataBin2Hex(Bin2HexParamPtr pb)
{
	long	offset = pb->inOffset;
	short	err = noErr;
		
	
	//	do a sanity check
	if (pb->context.dfState == kForkDone)
		return -1;
		
	//	flag overruns as an error, but encode up to overrun point
	if (pb->inAmount > pb->context.dfState)
	{
		pb->inAmount = pb->context.dfState;
		err = -1;
	}
	
	//	encode data
	pb->context.out = (unsigned char*)pb->outBuffer;
	EncodeBuffer(pb,(unsigned char*)(pb->inBuffer+offset),pb->inAmount);
	
	//	calculate amount left to be encoded
	pb->inAmount -= (pb->context.in - (pb->inBuffer+offset));
	pb->inOffset += (pb->context.in - (pb->inBuffer+offset));
		
	//	check for end-of-datafork
	pb->context.dfState -= (pb->context.in - (pb->inBuffer+offset));
	if (!pb->context.dfState)
	{
		EncodeCheckSum(pb);
		pb->context.dfState = kForkDone;
		
		//	check for zero length resource fork
		if (!pb->context.rfState)
		{
			EncodeCheckSum(pb);
			pb->context.rfState = kForkDone;
			
			//	fix 8to6 rounding problems
			if (pb->context.state+1)
			{
				long	zero = 0;
				EncodeBuffer(pb,(unsigned char*)&zero,1);
			}
			
			//	add ending delimiter
			*pb->context.out++ = ':';
			*pb->context.out++ = kCR;
			if (pb->addLFs)
				*pb->context.out++ = kLF;
		}
	}
	
	// return length of encoded data
	pb->outAmount = (long)(pb->context.out - (unsigned char*)pb->outBuffer);	
	return err;
}





short ResourceBin2Hex(Bin2HexParamPtr pb)
{
	long	offset = pb->inOffset;
	short	err = noErr;
	
	
	//	do a sanity check
	if (pb->context.rfState == kForkDone)
		return -1;
		
	//	flag overruns as an error, but encode up to overrun point
	if (pb->inAmount > pb->context.rfState)
	{
		pb->inAmount = pb->context.rfState;
		err = -1;
	}
	
	//	encode data
	pb->context.out = (unsigned char*)pb->outBuffer;
	EncodeBuffer(pb,(unsigned char*)(pb->inBuffer+offset),pb->inAmount);
	
	//	calculate amount left to be encoded
	pb->inAmount -= (pb->context.in - (pb->inBuffer+offset));
	pb->inOffset += (pb->context.in - (pb->inBuffer+offset));
	
	//	check for end-of-resourcefork
	pb->context.rfState -= (pb->context.in - (pb->inBuffer+offset));
	if (!pb->context.rfState)
	{
		EncodeCheckSum(pb);
		pb->context.rfState = kForkDone;
		
		//	fix 8to6 rounding problems
		if (pb->context.state+1)
		{
			long	zero = 0;
			EncodeBuffer(pb,(unsigned char*)&zero,1);
		}
		
		//	add ending delimiter
		*pb->context.out++ = ':';
		*pb->context.out++ = kCR;
		if (pb->addLFs)
			*pb->context.out++ = kLF;
	}
	
	// return length of encoded data
	pb->outAmount = (long)(pb->context.out - (unsigned char*)pb->outBuffer);
	return err;
}





void EncodeBuffer(Bin2HexParamPtr pb,unsigned char *buffer,long len)
{
	register	long	bits = pb->context.bits;
	register long	state = pb->context.state;
	register	short	check = pb->context.check;
	register unsigned char *out = pb->context.out;
	register	unsigned char *in = buffer;
	register long	unrolled = len-(len%3);
	register long	cur = 0;
	long	end = pb->outBufferSize-66;
	
	
	if (unrolled)
	{
		len -= unrolled;
		
		if (state<0)
			goto entry1;
		else if (!state)
			goto entry2;
		else
			goto entry3;
		
		while(unrolled)
		{
entry1:	//	initial state
			if ((unsigned char)cur == 0x90)
				cur = 0;
			else
			{
				register	unsigned short	temp;
				cur = *in++;
				unrolled--;
				
				temp = check;
				temp >>= 8;
				check <<= 8;
				check |= cur;			
				check ^= CRCMagic[temp];
			}
			
			*out++ = Bin2HexTranslation[cur>>2];
			bits = (cur&3)<<4;
			
			if (!--pb->context.line)
			{
				pb->context.line = 64;
				*out++ = kCR;
				if (pb->addLFs)
					*out++ = kLF;
				
				if ((out - pb->context.out) > end)
				{
					state = 0;
					goto exit;
				}
			}
			
entry2:	// middle state
			if ((unsigned char)cur == 0x90)
				cur = 0;
			else
			{
				register	unsigned short	temp;
				cur = *in++;
				unrolled--;
				
				temp = check;
				temp >>= 8;
				check <<= 8;
				check |= cur;			
				check ^= CRCMagic[temp];
			}
			
			*out++ = Bin2HexTranslation[bits|(cur>>4)];
			bits = (cur&0x0F)<<2;
			
			if (!--pb->context.line)
			{
				pb->context.line = 64;
				*out++ = kCR;
				if (pb->addLFs)
					*out++ = kLF;
				
				if ((out - pb->context.out) > end)
				{
					state = 1;
					goto exit;
				}
			}

entry3:	//	final state
			if ((unsigned char)cur == 0x90)
				cur = 0;
			else
			{
				register	unsigned short	temp;
				cur = *in++;
				unrolled--;
				
				temp = check;
				temp >>= 8;
				check <<= 8;
				check |= cur;			
				check ^= CRCMagic[temp];
			}
			
			*out++ = Bin2HexTranslation[bits|(cur>>6)];
			if (!--pb->context.line)
			{	
				pb->context.line = 64;
				*out++ = kCR;
				if (pb->addLFs)
					*out++ = kLF;
				
				if ((out - pb->context.out) > end)
				{
					*out++ = Bin2HexTranslation[cur&0x3F];
					pb->context.line--;
					state = -1;
					goto exit;
				}
			}
				
			*out++ = Bin2HexTranslation[cur&0x3F];
			if (!--pb->context.line)
			{
				pb->context.line = 64;
				*out++ = kCR;
				if (pb->addLFs)
					*out++ = kLF;
				
				if ((out - pb->context.out) > end)
				{
					state = -1;
					goto exit;
				}
			}
			
			if (unrolled < 3)
				break;
		}
		
		state = -1;
		len += unrolled;
	}
	
	//	clean up left-overs from unrolled loop
	while(len)
	{	
		if ((unsigned char)cur == 0x90)
			cur = 0;
		else
		{
			register	unsigned short	temp;
			cur = *in++;
			len--;
			
			temp = check;
			temp >>= 8;
			check <<= 8;
			check |= cur;			
			check ^= CRCMagic[temp];
		}
		
		if (state<0)
		{
			state = 0;
			*out++ = Bin2HexTranslation[cur>>2];
			bits = (cur&3)<<4;
		}
		else if (!state)
		{
			state = 1;
			*out++ = Bin2HexTranslation[bits|(cur>>4)];
			bits = (cur&0x0F)<<2;
		}
		else
		{
			state = -1;
			*out++ = Bin2HexTranslation[bits|(cur>>6)];
			if (!--pb->context.line)
			{	
				pb->context.line = 64;
				*out++ = kCR;
				if (pb->addLFs)
					*out++ = kLF;
				
				if ((out - pb->context.out) > end)
				{
					*out++ = Bin2HexTranslation[cur&0x3F];
					pb->context.line--;
					break;
				}
			}
			
			*out++ = Bin2HexTranslation[cur&0x3F];
		}
		
		if (!--pb->context.line)
		{
			pb->context.line = 64;
			*out++ = kCR;
			if (pb->addLFs)
				*out++ = kLF;
			
			if ((out - pb->context.out) > end)
				break;
		}
	}
	
exit:
	//	check last byte for RLE escape code
	if ((unsigned char)cur == 0x90)
	{
		cur = 0;
		if (state<0)
		{
			*out++ = Bin2HexTranslation[cur>>2];
			bits = (cur&3)<<4;
			state = 0;
		}
		else if (!state)
		{
			*out++ = Bin2HexTranslation[bits|(cur>>4)];
			bits = (cur&0x0F)<<2;
			state = 1;
		}
		else
		{
			*out++ = Bin2HexTranslation[bits|(cur>>6)];
			if (!--pb->context.line)
			{	
				pb->context.line = 64;
				*out++ = kCR;
				if (pb->addLFs)
					*out++ = kLF;
			}
			
			*out++ = Bin2HexTranslation[cur&0x3F];
			state = -1;
		}
		
		if (!--pb->context.line)
		{
			pb->context.line = 64;
			*out++ = kCR;
			if (pb->addLFs)
				*out++ = kLF;
		}
	}
	
	pb->context.in = in;
	pb->context.out = out;
	pb->context.bits = bits;
	pb->context.state = state;
	pb->context.check = check;
}





void EncodeCheckSum(Bin2HexParamPtr pb)
{
	register long	index = 16;
	long				check = pb->context.check;
	
	
	//	checksum 2 zero bytes for placeholder
	while(index--)
	{
		check <<= 1;
		if (check & 0x10000)
			check ^= 0x1021;
	}
	
	//	encode it and reset checksum engine
	EncodeBuffer(pb,(unsigned char*)&check+2,2);
	pb->context.check = 0;
}