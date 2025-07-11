/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * declarations for dealing with ctb streams
 ************************************************************************/
int CTBConnectTrans(UPtr serverName, short port);
int CTBSendTrans(short count, UPtr text,long size, ...);
int CTBRecvTrans(UPtr line,long *size);
int CTBDisTrans(void);
int CTBDestroyTrans(void);
int CTBTransError(void);
int GenSendWDS(wdsEntry *theWDS);
void CTBSilenceTrans(Boolean silence);
Boolean InitCTB(Boolean makeUser);
short DialThePhone(void);
void HangUpThePhone(void);
short CTBNavigateSTRN(short id);
UPtr CTBWhoAmI(Uptr who);
void MyCMIdle(void);
