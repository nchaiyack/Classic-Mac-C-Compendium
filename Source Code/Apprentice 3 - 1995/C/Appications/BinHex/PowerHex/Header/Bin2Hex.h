#ifndef	_BIN2HEX_
#define	_BIN2HEX_





typedef struct
{
	unsigned char	*inBuffer;
	long				inBufferSize;
	unsigned char	*outBuffer;
	long				outBufferSize;
	long				inAmount;
	long				inOffset;
	long				outAmount;
	short				addLFs;
	
	struct
	{
		Str32		name;
		OSType	type;
		OSType	creator;
		short		flags;
		long		dfLength;
		long		rfLength;
	} info;
	
	struct
	{
		unsigned char	*out;
		unsigned char	*in;
		long				line;
		long				state;
		long				bits;
		long				check;
		long				dfState;
		long				rfState;
	} context;
} Bin2HexParam, *Bin2HexParamPtr;


short NewBin2Hex(Bin2HexParamPtr pb);
short DataBin2Hex(Bin2HexParamPtr pb);
short ResourceBin2Hex(Bin2HexParamPtr pb);
void EncodeBuffer(Bin2HexParamPtr pb,unsigned char *buffer,long len);
long ComputeCheckSum(long check,long cur);
void EncodeCheckSum(Bin2HexParamPtr pb);


#endif	_BIN2HEX_