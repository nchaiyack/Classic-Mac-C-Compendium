short GetUUPCMail(Boolean quietly);
int UUPCRecvLine(UPtr buffer, long *size);
int UUPCSendTrans(short count, UPtr text,long size, ...);
int UUPCPrime(UPtr server);
void UUPCDry(void);
int UUPCSendMessage(TOCHandle tocH,short sumNum);
int UUPCWriteAddr(UPtr addr);
