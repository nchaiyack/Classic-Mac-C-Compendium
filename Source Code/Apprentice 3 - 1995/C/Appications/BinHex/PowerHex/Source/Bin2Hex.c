#include "Bin2Hex.h"





/*	codegen switches	*/
#define	PROFILE		0





/*	local defines	*/
#define	kHeaderMsg			"(This file must be converted with BinHex 4.0)"
#define	kHeaderMsgLen		sizeof(kHeaderMsg) - 1
#define	kMaxHeaderLength	89 + kHeaderMsgLen
#define	kForkDone			-1
#define	kCR					'\015'
#define	kLF					'\012'


static unsigned char Bin2HexTranslation[64] = {
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x30, 0x31, 0x32,
	0x33, 0x34, 0x35, 0x36, 0x38, 0x39, 0x40, 0x41,
	0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x50, 0x51, 0x52,
	0x53, 0x54, 0x55, 0x56, 0x58, 0x59, 0x5A, 0x5B,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x68,
	0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x70, 0x71, 0x72 };


static unsigned short CRCMagic[256] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF, 
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE, 
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, 
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D, 
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC, 
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, 
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B, 
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, 
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78, 
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, 
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067, 
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256, 
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, 
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634, 
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3, 
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, 
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1, 
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0 };





#if	PROFILE
	long	cntStart;
	long	cntTotal;
	long	cntOverall;

	void ShowByteCount(Bin2HexParamPtr pb);
	void ShowByteCount(Bin2HexParamPtr pb)
	{
		Str255	total,totalK,overall,overallK;
		long		temp;
		
		
		temp = ((60*(pb->info.dfLength+pb->info.rfLength))/(cntTotal ? cntTotal : 1))/1024;
		NumToString(temp,totalK);
		
		cntOverall = TickCount() - cntOverall;
		temp = ((60*(pb->info.dfLength+pb->info.rfLength))/(cntOverall ? cntOverall : 1))/1024;
		NumToString(temp,overallK);
		
		NumToString(cntTotal,total);
		NumToString(cntOverall,overall);
		ParamText(total,totalK,overall,overallK);
		Alert(128,nil);
	}
#endif





short NewBin2Hex(Bin2HexParamPtr pb)
{
	long	zero = 0;
	
	
	#if	PROFILE
		cntTotal = 0;
		cntOverall = TickCount();
	#endif
	
	//	do buffer size sanity check
	if (pb->outBufferSize < kMaxHeaderLength)
		return -1;
	
	// add BinHex preamble
	pb->context.out = (unsigned char*)pb->outBuffer + kHeaderMsgLen;
	BlockMove(kHeaderMsg,pb->outBuffer,kHeaderMsgLen);
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
			
			#if	PROFILE
				ShowByteCount(pb);
			#endif
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
	
	#if	PROFILE
		cntStart = TickCount();
	#endif
	
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
			
			#if	PROFILE
				cntTotal += TickCount()-cntStart;
				ShowByteCount(pb);
			#endif
		}
	}
	
	// return length of encoded data
	pb->outAmount = (long)(pb->context.out - (unsigned char*)pb->outBuffer);
	
	#if	PROFILE
		cntTotal += TickCount()-cntStart;
	#endif
	
	return err;
}





short ResourceBin2Hex(Bin2HexParamPtr pb)
{
	long	offset = pb->inOffset;
	short	err = noErr;
	
	
	//	do a sanity check
	if (pb->context.rfState == kForkDone)
		return -1;
	
	#if	PROFILE
		cntStart = TickCount();
	#endif
	
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
		
		#if	PROFILE
			cntTotal += TickCount()-cntStart;
			ShowByteCount(pb);
		#endif
	}
	
	// return length of encoded data
	pb->outAmount = (long)(pb->context.out - (unsigned char*)pb->outBuffer);
	
	#if	PROFILE
		cntTotal += TickCount()-cntStart;
	#endif
	
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