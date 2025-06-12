
/* Mac.c */
int MAC_init(void);
int MAC_graphics(void);
int MAC_text(void);
int MAC_linetype(int);
int MAC_move(unsigned int, unsigned int);
int MAC_vector(unsigned int, unsigned int);
int MAC_put_text(unsigned int, unsigned int, char*);
int MAC_reset(void);
pascal void PutPICTData(Ptr, int);
