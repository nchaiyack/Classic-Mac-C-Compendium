/*
 * enc_des.c
 */
extern void cfb64_init(CDATA *tw, long server);
extern void ofb64_init(CDATA *tw, long server);
extern void fb64_init(register struct fb *fbp);
extern long cfb64_start(CDATA *tw, long dir, long server);
extern long ofb64_start(CDATA *tw, long dir, long server);
extern long cfb64_is(CDATA *tw, unsigned char *data, long cnt);
extern long ofb64_is(CDATA *tw, unsigned char *data, long cnt);
extern long fb64_is(CDATA *tw, unsigned char *data, long cnt, struct fb *fbp);
extern long cfb64_reply(CDATA *tw, unsigned char *data, long cnt);
extern long ofb64_reply(CDATA *tw, unsigned char *data, long cnt);
extern long fb64_reply(CDATA *tw, unsigned char *data, long cnt, struct fb *fbp);
extern void cfb64_session(CDATA *tw, Session_Key *key, long server);
extern void ofb64_session(CDATA *tw, Session_Key *key, long server);
extern long cfb64_keyid(CDATA *tw, long dir, unsigned char *kp, long *lenp);
extern long ofb64_keyid(CDATA *tw, long dir, unsigned char *kp, long *lenp);
extern long fb64_keyid(long dir, unsigned char *kp, long *lenp, struct fb *fbp);
extern void fb64_printsub(unsigned char *data, long cnt, unsigned char *buf, long buflen, unsigned char *type);
extern void cfb64_printsub(unsigned char *data, long cnt, unsigned char *buf, long buflen);
extern void ofb64_printsub(unsigned char *data, long cnt, unsigned char *buf, long buflen);
extern void fb64_stream_iv(Block seed, register struct stinfo *stp);
extern void fb64_stream_key(Block key, register struct stinfo *stp);
extern void cfb64_encrypt(CDATA *tw, register unsigned char *s, long c);
extern long cfb64_decrypt(CDATA *tw, long data);
extern void ofb64_encrypt(CDATA *tw, register unsigned char *s, long c);
extern long ofb64_decrypt(CDATA *tw, long data);
extern void printd(unsigned char *data, long cnt);
