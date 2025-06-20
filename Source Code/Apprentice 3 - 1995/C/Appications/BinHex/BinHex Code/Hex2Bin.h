#ifndef	_HEX2BIN_
#define	_HEX2BIN_





#define	badInternalStateErr		-31100
#define	badCheckSumErr				-31101
#define	endOfDataErr				-31102





typedef enum
{
	kScanning = 0,
	kDecodingHeader,
	kDecodedHeader,
	kDecodingDataFork,
	kDecodedDataFork,
	kDecodingResourceFork,
	kDecodedResourceFork,
	kFileDecoded
} decoderStates;





typedef struct
{
	unsigned long	inAmount;
	unsigned long	inOffset;
	unsigned long	outAmount;
	short				decodeState;
	short				err;
	
	struct
	{
		unsigned char	*inBuffer;
		unsigned long	inBufferSize;
		unsigned char	*outBuffer;
		unsigned long	outBufferSize;
	} buffer;
	
	struct
	{
		Str32				name;
		OSType			type;
		OSType			creator;
		short				flags;
		unsigned long	dfLength;
		unsigned long	rfLength;
	} info;
	
	struct
	{
		unsigned char	*out;
		unsigned char	*in;
		long				header;
		long				state;
		long				resume;
		unsigned long	bits;
		unsigned char	last;
		unsigned char	count;
		short				check;
		short				checkStorage;
		short				oldCheck;
		unsigned long	dfLen;
		unsigned long	rfLen;
	} context;
} Hex2BinParam, *Hex2BinParamPtr;





short NewHex2Bin(Hex2BinParamPtr pb);
short ParseHexData(Hex2BinParamPtr pb);
short DecodePreamble(Hex2BinParamPtr pb);
short DecodeHeader(Hex2BinParamPtr pb);
short DecodeDataFork(Hex2BinParamPtr pb);
short DecodeResourceFork(Hex2BinParamPtr pb);
short DecodeBuffer(Hex2BinParamPtr pb,unsigned long inLen,unsigned long outLen);
short CheckSumByte(short check,unsigned char cur);
short ComputeCheckSum(Hex2BinParamPtr pb);


#endif	_HEX2BIN_