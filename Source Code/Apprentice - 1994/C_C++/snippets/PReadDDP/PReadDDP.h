#define _H_PReadDDP

typedef struct DDPPacket{
	Boolean						PacketReceived;
	unsigned char			LAPDstNode;
	unsigned char			LAPSrcNode;
	unsigned char			LAPProtoType;
	unsigned short		Unused					: 2;
	unsigned short		HopCount				: 4;
	unsigned short		DatagramLength	: 10;
	unsigned short		CheckSum;
	unsigned short		DstNet;
	unsigned short		SrcNet;
	unsigned char			DstNode;
	unsigned char			SrcNode;
	unsigned char			DstSocket;
	unsigned char			SrcSocket;
	unsigned char			DDPProtoType;
	unsigned char			Data[586];
} DDPPacket;

#ifndef _No_PReadDDP_Prototypes_

void PReadDDP(Byte SocketNum, DDPPacket *PacketBuff, Boolean Async);
void PReadDDPFlush(Byte SocketNum);
void PReadDDPListener(void);

#endif