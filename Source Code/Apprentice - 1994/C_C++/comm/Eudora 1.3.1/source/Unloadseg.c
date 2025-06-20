#pragma load EUDORA_LOAD
#pragma segment Text
void __SegText(void);
void __SegText(void){}
#pragma segment Balloon
void __SegBalloon(void);
void __SegBalloon(void){}
#pragma segment UUPCOut
void __SegUUPCOut(void);
void __SegUUPCOut(void){}
#pragma segment TEFuncs
void __SegTEFuncs(void);
void __SegTEFuncs(void){}
#pragma segment utl
void __Segutl(void);
void __Segutl(void){}
#pragma segment Misc
void __SegMisc(void);
void __SegMisc(void){}
#pragma segment Squish
void __SegSquish(void);
void __SegSquish(void){}
#pragma segment Print
void __SegPrint(void);
void __SegPrint(void){}
#pragma segment POP
void __SegPOP(void);
void __SegPOP(void){}
#pragma segment BinHex
void __SegBinHex(void);
void __SegBinHex(void){}
#pragma segment SMTP
void __SegSMTP(void);
void __SegSMTP(void){}
#pragma segment TcpTrans
void __SegTcpTrans(void);
void __SegTcpTrans(void){}
#pragma segment Message
void __SegMessage(void);
void __SegMessage(void){}
#pragma segment Mailbox
void __SegMailbox(void);
void __SegMailbox(void){}
#pragma segment STE
void __SegSTE(void);
void __SegSTE(void){}
#pragma segment Progress
void __SegProgress(void);
void __SegProgress(void){}
#pragma segment UUPCIn
void __SegUUPCIn(void);
void __SegUUPCIn(void){}
#pragma segment Util
void __SegUtil(void);
void __SegUtil(void){}
#pragma segment Find
void __SegFind(void);
void __SegFind(void){}
#pragma segment Log
void __SegLog(void);
void __SegLog(void){}
#pragma segment NickMng
void __SegNickMng(void);
void __SegNickMng(void){}
#pragma segment NickWin
void __SegNickWin(void);
void __SegNickWin(void){}
#pragma segment Lib
void __SegLib(void);
void __SegLib(void){}
#pragma segment MBoxWin
void __SegMBoxWin(void);
void __SegMBoxWin(void){}
#pragma segment HexBin
void __SegHexBin(void);
void __SegHexBin(void){}
#pragma segment CTBTrans
void __SegCTBTrans(void);
void __SegCTBTrans(void){}
#pragma segment MyWindow
void __SegMyWindow(void);
void __SegMyWindow(void){}
#pragma segment MsgOps
void __SegMsgOps(void);
void __SegMsgOps(void){}
#pragma segment MD5
void __SegMD5(void);
void __SegMD5(void){}
#pragma segment LMgr
void __SegLMgr(void);
void __SegLMgr(void){}
#pragma segment BuildTOC
void __SegBuildTOC(void);
void __SegBuildTOC(void){}
#pragma segment AliasWin
void __SegAliasWin(void);
void __SegAliasWin(void){}
#pragma segment Boxes
void __SegBoxes(void);
void __SegBoxes(void){}
#pragma segment Ph
void __SegPh(void);
void __SegPh(void){}
#pragma segment Abomination
void __SegAbomination(void);
void __SegAbomination(void){}
#pragma segment NickExp
void __SegNickExp(void);
void __SegNickExp(void){}
#pragma segment DNR
void __SegDNR(void);
void __SegDNR(void){}
#pragma segment Menu
void __SegMenu(void);
void __SegMenu(void){}
#pragma segment Transport
void __SegTransport(void);
void __SegTransport(void){}
#pragma segment Outgoing
void __SegOutgoing(void);
void __SegOutgoing(void){}
#pragma segment StringUtils
void __SegStringUtils(void);
void __SegStringUtils(void){}
#pragma segment Ends
void __SegEnds(void);
void __SegEnds(void){}
#pragma segment Prefs
void __SegPrefs(void);
void __SegPrefs(void){}
#pragma segment MboxWin
void __SegMboxWin(void);
void __SegMboxWin(void){}
#pragma segment Main
void UnloadSegments(void)
{
  extern void _DataInit();
  extern char *strchr();
  UnloadSeg(_DataInit);
  UnloadSeg(strchr);
  UnloadSeg(__SegText);
  UnloadSeg(__SegBalloon);
  UnloadSeg(__SegUUPCOut);
  UnloadSeg(__SegTEFuncs);
  UnloadSeg(__Segutl);
  UnloadSeg(__SegMisc);
  UnloadSeg(__SegSquish);
  UnloadSeg(__SegPrint);
  UnloadSeg(__SegPOP);
  UnloadSeg(__SegBinHex);
  UnloadSeg(__SegSMTP);
  UnloadSeg(__SegTcpTrans);
  UnloadSeg(__SegMessage);
  UnloadSeg(__SegMailbox);
  UnloadSeg(__SegSTE);
  UnloadSeg(__SegProgress);
  UnloadSeg(__SegUUPCIn);
  UnloadSeg(__SegUtil);
  UnloadSeg(__SegFind);
  UnloadSeg(__SegLog);
  UnloadSeg(__SegNickMng);
  UnloadSeg(__SegNickWin);
  UnloadSeg(__SegLib);
  UnloadSeg(__SegMBoxWin);
  UnloadSeg(__SegHexBin);
  UnloadSeg(__SegCTBTrans);
  UnloadSeg(__SegMyWindow);
  UnloadSeg(__SegMsgOps);
  UnloadSeg(__SegMD5);
  UnloadSeg(__SegLMgr);
  UnloadSeg(__SegBuildTOC);
  UnloadSeg(__SegAliasWin);
  UnloadSeg(__SegBoxes);
  UnloadSeg(__SegPh);
  UnloadSeg(__SegAbomination);
  UnloadSeg(__SegNickExp);
  UnloadSeg(__SegDNR);
  UnloadSeg(__SegMenu);
  UnloadSeg(__SegTransport);
  UnloadSeg(__SegOutgoing);
  UnloadSeg(__SegStringUtils);
  UnloadSeg(__SegEnds);
  UnloadSeg(__SegPrefs);
  UnloadSeg(__SegMboxWin);
}
