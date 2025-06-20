
/* parse.c */
void parse(struct WindRec *tw, unsigned char *st, short cnt);
void parse2(struct WindRec *tw, unsigned char *st, short cnt);
void	SendNAWSinfo(WindRec *s, short horiz, short vert);
void net_write(struct WindRec *tw, char *buf, short length);
void net_write(struct WindRec *tw, char *buf, short length);
void send_auth_opt (struct WindRec *tw);
void tn_sendsub(struct WindRec *tw, short code, short request, char *cp, short length);
char *getcname(struct WindRec *tw);
void encryptStatechange(struct WindRec *tw);
void DemangleLineMode(char *s, short mode);
void DemangleLineModeShort(char *s, short mode);