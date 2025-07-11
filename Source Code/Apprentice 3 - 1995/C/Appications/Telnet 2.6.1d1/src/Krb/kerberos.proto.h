/*
 * kerberos.c
 */
extern long krb_telnet(WDATA *cr, char *tp, long doit);
extern void tn_kerberos_reply(WDATA *tw, unsigned char *subbuffer);
extern void tn_send_challenge(WDATA *tw, unsigned char *subbuffer);
extern short lowcall(long cscode, krbParmBlock *klopb, short kdriver);
extern short hicall(long cscode, krbHiParmBlock *khipb, short kdriver);
extern void xbcopy(void *src, void *dst, long n);
extern long xbcmp(void *src, void *dst, long n);
extern void xbzero(void *dst, long n);
