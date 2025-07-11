#include "Hex2Bin.h"





/*	local defines	*/
#define	kDecodeHeaderMsg			"(This file must be converted with BinHex "
#define	kDecodeHeaderMsgLen		(sizeof(kDecodeHeaderMsg) - 1)
#define	FAIL							0xFF
#define	SKIP							0xFE
#define	DONE							0xFD





extern unsigned short CRCMagic[256];
static unsigned char Hex2BinTranslation[256] = {
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, SKIP, FAIL, FAIL, SKIP, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, FAIL, FAIL,
	0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, FAIL,
	0x14, 0x15, DONE, FAIL, FAIL, FAIL, FAIL, FAIL,
	0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
	0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, FAIL,
	0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, FAIL,
	0x2C, 0x2D, 0x2E, 0x2F, FAIL, FAIL, FAIL, FAIL,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, FAIL,
	0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, FAIL, FAIL,
	0x3D, 0x3E, 0x3F, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
	FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL };





short NewHex2Bin(Hex2BinParamPtr pb)
{
	//	do buffer size sanity check
	if ((pb->buffer.inBufferSize < 1) || (pb->buffer.outBufferSize < 1))
		return -1;
	
	// initialize state for 6to8 bit decoding engine
	pb->inOffset = 0;
	pb->context.bits = 0;
	pb->context.check = 0;
	
	//	initialize state for decoding preamble
	pb->decodeState = 0;
	pb->err = noErr;
	pb->context.header = 0;
	pb->context.state = 2;
	pb->inAmount = 0;
	
	return noErr;
}





short ParseHexData(Hex2BinParamPtr pb)
{
	//	set per run variables
	pb->err = badInternalStateErr;
	pb->outAmount = 0;
	
	switch(pb->decodeState)
	{
		//	decoding preamble
		case kScanning:
			pb->err = noErr;
			if (DecodePreamble(pb))
				return ParseHexData(pb);
			break;
		
		//	decode header
		case kDecodingHeader:
			pb->err = DecodeHeader(pb);
			break;
		
		//	header decoded
		case kDecodedHeader:
			pb->err = noErr;
			pb->decodeState = kDecodingDataFork;
			break;
		
		// decode data fork
		case kDecodingDataFork:
			pb->err = DecodeDataFork(pb);
			break;
		
		//	data fork decoded
		case kDecodedDataFork:
			pb->err = noErr;
			pb->decodeState = kDecodingResourceFork;
			break;
		
		//	decode resource fork
		case kDecodingResourceFork:
			pb->err = DecodeResourceFork(pb);
			break;
		
		//	resource fork decoded
		case kDecodedResourceFork:
			pb->err = noErr;
			pb->decodeState = kFileDecoded;
			break;
		
		//	file completely decoded
		case kFileDecoded:
			pb->err = noErr;
			break;
	}
	
	return pb->err;
}





short DecodePreamble(Hex2BinParamPtr pb)
{
	unsigned char	cur,*end,*msg = (unsigned char *)kDecodeHeaderMsg;
	
	
	// setup input and range check pointers
	pb->context.in = pb->buffer.inBuffer + pb->inOffset;
	end = pb->context.in + pb->inAmount;
	
	//	make sure we stay inside the input buffer
	while(pb->context.in < end)
	{
		switch(pb->context.state)
		{
			// looking for newline (CR or LF or CRLF)
			case 0:
				cur = *pb->context.in++;
				if ((cur == '\n') || (cur == '\r'))
					pb->context.state = 1;
				break;
			
			//	found newline, looking for non newline sequence
			case 1:
				cur = *pb->context.in;
				if ((cur == '\n') || (cur == '\r'))
					pb->context.in++;
				else
					pb->context.state = 2;
				break;
			
			// comparing input against preamble string
			case 2:
				cur = *pb->context.in;
				if (cur == msg[pb->context.header])
				{
					pb->context.in++;
					pb->context.header++;
					if (pb->context.header == kDecodeHeaderMsgLen)
						pb->context.state = 3;
				}
				else
				{
					pb->context.header = 0;
					pb->context.state = 0;
				}
				break;
			
			// skip remaining preamble up to a newline
			case 3:
				cur = *pb->context.in++;
				if ((cur == '\n') || (cur == '\r'))
					pb->context.state = 4;
				break;
			
			// look for opening colon
			case 4:
				cur = *pb->context.in;
				if ((cur == '\n') || (cur == '\r'))
					pb->context.in++;
				else if (cur != ':')
					pb->context.state = 2;
				else
				{
					// setup next decode state
					pb->decodeState = kDecodingHeader;
					pb->context.header = 0;
					pb->context.state = 0;
					pb->context.check = 0;
					
					//	fix and advance buffer pointers
					pb->context.in++;
					pb->inOffset = pb->context.in - pb->buffer.inBuffer;
					pb->inAmount = end - pb->context.in;
					return true;
				}
				break;
		}
	}
	
	// haven't detected a file yet...fix and advance buffer pointers
	pb->inOffset = pb->context.in - pb->buffer.inBuffer;
	pb->inAmount = end - pb->context.in;
	return false;
}





short DecodeHeader(Hex2BinParamPtr pb)
{
	unsigned long	len;
	unsigned char	*end;
	
	
	// setup input and range check pointers
	pb->context.in = pb->buffer.inBuffer + pb->inOffset;
	end = pb->context.in + pb->inAmount;
	
	//	make sure we stay inside the input buffer
	while(pb->context.in < end)
	{
		switch(pb->context.header)
		{
			// get length of encoded file name
			case 0:
				pb->context.out = (unsigned char*)&pb->info.name[0];
				if (DecodeBuffer(pb,pb->inAmount,1))
					return endOfDataErr;
				
				//	fix and advance input buffer pointers
				pb->inOffset = pb->context.in - pb->buffer.inBuffer;
				pb->inAmount = end - pb->context.in;
				
				// have we decoded what we were looking for?
				if ((pb->context.out - (unsigned char*)&pb->info.name[0]) == 1)
				{
					pb->context.out = (unsigned char*)&pb->info.name[1];
					pb->context.header = 1;
				}
				break;
			
			//	get encoded file name and NULL byte pad
			case 1:
				len = (1 + pb->info.name[0]) - (pb->context.out - (unsigned char*)&pb->info.name[1]);
				len = ((1 + pb->info.name[0]) < len) ? (1 + pb->info.name[0]) : len;
				if (DecodeBuffer(pb,pb->inAmount,len))
					return endOfDataErr;
				
				//	fix and advance input buffer pointers
				pb->inOffset = pb->context.in - pb->buffer.inBuffer;
				pb->inAmount = end - pb->context.in;
				
				// have we decoded what we were looking for?
				if ((pb->context.out - (unsigned char*)&pb->info.name[1]) == (1 + pb->info.name[0]))
				{
					pb->context.out = (unsigned char*)&pb->info.type;
					pb->context.header = 2;
				}
				break;
			
			//	get file information
			case 2:
				len = 18 - (pb->context.out - (unsigned char*)&pb->info.type);
				len = (18 < len) ? 18 : len;
				if (DecodeBuffer(pb,pb->inAmount,len))
					return endOfDataErr;
				
				//	fix and advance input buffer pointers
				pb->inOffset = pb->context.in - pb->buffer.inBuffer;
				pb->inAmount = end - pb->context.in;
				
				// have we decoded what we were looking for?
				if ((pb->context.out - (unsigned char*)&pb->info.type) == 18)
				{
					//	clear onDesk, invisible and inited finder flags
					pb->info.flags &= 0xBEFE;
					
					//	setup context information for decoder
					pb->context.dfLen = pb->info.dfLength;
					pb->context.rfLen = pb->info.rfLength;
					
					//	compute checksum for header
					pb->context.checkStorage = ComputeCheckSum(pb);
					pb->context.out = (unsigned char*)&pb->context.oldCheck;
					pb->context.header = 3;
				}
				break;
			
			//	get and confirm header checksum
			case 3:
				len = 2 - (pb->context.out - (unsigned char*)&pb->context.oldCheck);
				len = (2 < len) ? 2 : len;
				if (DecodeBuffer(pb,pb->inAmount,len))
					return endOfDataErr;
				
				//	fix and advance input buffer pointers
				pb->inOffset = pb->context.in - pb->buffer.inBuffer;
				pb->inAmount = end - pb->context.in;
				
				// have we decoded what we were looking for?
				if ((pb->context.out - (unsigned char*)&pb->context.oldCheck) == 2)
				{
					// setup next decode state
					pb->decodeState = kDecodedHeader;
					pb->context.check = 0;
					pb->context.header = 0;
					
					//	fix and advance buffer pointers
					pb->inOffset = pb->context.in - pb->buffer.inBuffer;
					pb->inAmount = end - pb->context.in;
					
					if (pb->context.checkStorage != pb->context.oldCheck)
						return badCheckSumErr;
					else
						return noErr;
				}
				break;
		}
	}
	
	// haven't detected a file yet...
	pb->inOffset = pb->context.in - pb->buffer.inBuffer;
	pb->inAmount = end - pb->context.in;
	return noErr;
}





short DecodeDataFork(Hex2BinParamPtr pb)
{
	unsigned char	*inEnd,*outEnd;
	unsigned	long	len;
	
	
	// setup input and range check pointers
	pb->context.in = pb->buffer.inBuffer + pb->inOffset;
	inEnd = pb->context.in + pb->inAmount;
	
	if (pb->context.header == 0)
	{
		pb->context.out = pb->buffer.outBuffer;
		outEnd = pb->context.out + pb->buffer.outBufferSize;
	}
	else
		outEnd = (unsigned char*)&pb->context.oldCheck + 2;
	
	//	make sure we stay inside the input and output buffers
	while((pb->context.in < inEnd) && (pb->context.out < outEnd))
	{
		switch(pb->context.header)
		{
			//	decode data fork
			case 0:
				len = (pb->context.dfLen < pb->buffer.outBufferSize) ? pb->context.dfLen : pb->buffer.outBufferSize;
				if (DecodeBuffer(pb,pb->inAmount,len))
					return endOfDataErr;
				
				//	fix and advance input and output buffer pointers
				pb->inOffset = pb->context.in - pb->buffer.inBuffer;
				pb->inAmount = inEnd - pb->context.in;
				pb->context.dfLen -= pb->context.out - (unsigned char*)pb->buffer.outBuffer;
				pb->outAmount += pb->context.out - (unsigned char*)pb->buffer.outBuffer;
				
				// have we decoded what we were looking for?
				if (!pb->context.dfLen)
				{
					//	compute checksum for data fork
					pb->context.checkStorage = ComputeCheckSum(pb);
					pb->context.out = (unsigned char*)&pb->context.oldCheck;
					
					//	setup next state
					pb->context.header = 1;
				}
				break;
			
			//	get and confirm header checksum
			case 1:
				len = 2 - (pb->context.out - (unsigned char*)&pb->context.oldCheck);
				len = (2 < len) ? 2 : len;
				if (DecodeBuffer(pb,pb->inAmount,len))
					return endOfDataErr;
				
				//	fix and advance input buffer pointers
				pb->inOffset = pb->context.in - pb->buffer.inBuffer;
				pb->inAmount = inEnd - pb->context.in;
				
				// have we decoded what we were looking for?
				if ((pb->context.out - (unsigned char*)&pb->context.oldCheck) == 2)
				{
					// setup next decode state
					pb->decodeState = kDecodedDataFork;
					pb->context.header = 0;
					pb->context.check = 0;
					
					if (pb->context.checkStorage != pb->context.oldCheck)
						return badCheckSumErr;
					else
						return noErr;
				}
				break;
		}
	}
	
	//	haven't finished decoding data fork...
	return noErr;
}





short DecodeResourceFork(Hex2BinParamPtr pb)
{
	unsigned char	*inEnd,*outEnd;
	unsigned	long	len;
	
	
	// setup input and range check pointers
	pb->context.in = pb->buffer.inBuffer + pb->inOffset;
	inEnd = pb->context.in + pb->inAmount;
	
	if (pb->context.header == 0)
	{
		pb->context.out = pb->buffer.outBuffer;
		outEnd = pb->context.out + pb->buffer.outBufferSize;
	}
	else
		outEnd = (unsigned char*)&pb->context.oldCheck + 2;
	
	//	make sure we stay inside the input and output buffers
	while((pb->context.in < inEnd) && (pb->context.out < outEnd))
	{
		switch(pb->context.header)
		{
			//	decode resource fork
			case 0:
				len = (pb->context.rfLen < pb->buffer.outBufferSize) ? pb->context.rfLen : pb->buffer.outBufferSize;
				if (DecodeBuffer(pb,pb->inAmount,len))
					return endOfDataErr;
				
				//	fix and advance input and output buffer pointers
				pb->inOffset = pb->context.in - pb->buffer.inBuffer;
				pb->inAmount = inEnd - pb->context.in;
				pb->context.rfLen -= pb->context.out - (unsigned char*)pb->buffer.outBuffer;
				pb->outAmount += pb->context.out - (unsigned char*)pb->buffer.outBuffer;
				
				// have we decoded what we were looking for?
				if (!pb->context.rfLen)
				{
					//	compute checksum for resource fork
					pb->context.checkStorage = ComputeCheckSum(pb);
					pb->context.out = (unsigned char*)&pb->context.oldCheck;
					pb->context.header = 1;
				}
				break;
			
			//	get and confirm header checksum
			case 1:
				len = 2 - (pb->context.out - (unsigned char*)&pb->context.oldCheck);
				len = (2 < len) ? 2 : len;
				if (DecodeBuffer(pb,pb->inAmount,len))
					return endOfDataErr;
				
				//	fix and advance input buffer pointers
				pb->inOffset = pb->context.in - pb->buffer.inBuffer;
				pb->inAmount = inEnd - pb->context.in;
				
				// have we decoded what we were looking for?
				if ((pb->context.out - (unsigned char*)&pb->context.oldCheck) == 2)
				{
					// setup next decode state
					pb->decodeState = kDecodedResourceFork;
					pb->context.header = 0;
					pb->context.check = 0;
					
					if (pb->context.checkStorage != pb->context.oldCheck)
						return badCheckSumErr;
					else
						return noErr;
				}
				break;
		}
	}
	
	//	haven't finished decoding resource fork...
	return noErr;
}





short DecodeBuffer(Hex2BinParamPtr pb,unsigned long inLen,unsigned long outLen)
{
	register short	check = pb->context.check;
	unsigned long	bits = pb->context.bits;
	long 				state = pb->context.state;
	unsigned char	*in = pb->context.in;
	unsigned char	*inEnd = in + inLen;
	unsigned char	*out = pb->context.out;
	unsigned char	*outEnd = out + outLen;
	unsigned char	lastByte = pb->context.last;
	unsigned long	cur;
	unsigned char	tmp;
	unsigned short	crcTemp;
	
	
	while((in < inEnd) && (out < outEnd))
	{
		switch(state)
		{
			//	normal 0 saved bits
			case 0:
				cur = Hex2BinTranslation[*in++];
				if (cur == SKIP)
					break;
				else if (cur == DONE)
					return true;
								
				bits = cur;
				state = 1;
				break;
			
			//	normal 6 saved bits
			case 1:
				cur = Hex2BinTranslation[*in++];
				if (cur == SKIP)
					break;
				else if (cur == DONE)
					return true;
								
				tmp = (cur>>4) | (bits<<2);
				bits = cur&0x0F;
				if (tmp == 0x90)
					state = 6;
				else
				{
					lastByte = tmp;
					*out++ = tmp;
					state = 2;
					
					//	compute checksum for byte
					crcTemp = check;
					crcTemp >>= 8;
					check <<= 8;
					check |= tmp;			
					check ^= CRCMagic[crcTemp];
				}
				break;
			
			//	normal 4 saved bits
			case 2:
				cur = Hex2BinTranslation[*in++];
				if (cur == SKIP)
					break;
				else if (cur == DONE)
					return true;
								
				tmp = (cur>>2) | (bits<<4);
				bits = cur&0x03;
				if (tmp == 0x90)
					state = 7;
				else
				{
					lastByte = tmp;
					*out++ = tmp;
					state = 3;
					
					//	compute checksum for byte
					crcTemp = check;
					crcTemp >>= 8;
					check <<= 8;
					check |= tmp;			
					check ^= CRCMagic[crcTemp];
				}
				break;
			
			// normal 2 saved bits
			case 3:
				cur = Hex2BinTranslation[*in++];
				if (cur == SKIP)
					break;
				else if (cur == DONE)
					return true;
								
				tmp = cur | (bits<<6);
				if (tmp == 0x90)
					state = 4;
				else
				{
					lastByte = tmp;
					*out++ = tmp;
					state = 0;
					
					//	compute checksum for byte
					crcTemp = check;
					crcTemp >>= 8;
					check <<= 8;
					check |= tmp;			
					check ^= CRCMagic[crcTemp];
				}
				break;
			
			//	RLE count 0 saved bits
			case 4:
				cur = Hex2BinTranslation[*in++];
				if (cur == SKIP)
					break;
				else if (cur == DONE)
					return true;
								
				bits = cur;
				state = 5;
				break;
			
			// RLE count 6 saved bits
			case 5:
				cur = Hex2BinTranslation[*in++];
				if (cur == SKIP)
					break;
				else if (cur == DONE)
					return true;
								
				tmp = (cur>>4) | (bits<<2);
				bits = cur&0x0F;
				if (!tmp)
				{
					lastByte = 0x90;
					*out++ = 0x90;
					state = 2;
					
					//	compute checksum for byte
					crcTemp = check;
					crcTemp >>= 8;
					check <<= 8;
					check |= 0x90;			
					check ^= CRCMagic[crcTemp];
				}
				else
				{
					pb->context.count = tmp;
					pb->context.resume = 2;
					state = 8;
				}
				break;
			
			// RLE count 4 saved bits
			case 6:
				cur = Hex2BinTranslation[*in++];
				if (cur == SKIP)
					break;
				else if (cur == DONE)
					return true;
								
				tmp = (cur>>2) | (bits<<4);
				bits = cur&0x03;
				if (!tmp)
				{
					lastByte = 0x90;
					*out++ = 0x90;
					state = 3;
					
					//	compute checksum for byte
					crcTemp = check;
					crcTemp >>= 8;
					check <<= 8;
					check |= 0x90;			
					check ^= CRCMagic[crcTemp];
				}
				else
				{
					pb->context.count = tmp;
					pb->context.resume = 3;
					state = 8;
				}
				break;
			
			// RLE count 2 saved bits
			case 7:
				cur = Hex2BinTranslation[*in++];
				if (cur == SKIP)
					break;
				else if (cur == DONE)
					return true;
								
				tmp = cur | (bits<<6);
				if (!tmp)
				{
					lastByte = 0x90;
					*out++ = 0x90;
					state = 0;
					
					//	compute checksum for byte
					crcTemp = check;
					crcTemp >>= 8;
					check <<= 8;
					check |= 0x90;			
					check ^= CRCMagic[crcTemp];
				}
				else
				{
					pb->context.count = tmp;
					pb->context.resume = 0;
					state = 8;
				}
				break;
			
			//	RLE expand
			case 8:
				pb->context.count--;
				if (!pb->context.count)
					state = pb->context.resume;
				else
				{
					*out++ = lastByte;
					
					//	compute checksum for byte
					crcTemp = check;
					crcTemp >>= 8;
					check <<= 8;
					check |= lastByte;			
					check ^= CRCMagic[crcTemp];
				}
				break;
		}
	}
	
	
	pb->context.check = check;
	pb->context.last = lastByte;
	pb->context.bits = bits;
	pb->context.state = state;
	pb->context.in = in;
	pb->context.out = out;
	return false;
}





short ComputeCheckSum(Hex2BinParamPtr pb)
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
	
	//	return computed checksum value
	return check;
}