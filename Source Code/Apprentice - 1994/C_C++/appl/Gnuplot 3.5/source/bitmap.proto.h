
/* bitmap.c */
void b_setpixel(unsigned int, unsigned int, unsigned int);
void b_makebitmap(unsigned int, unsigned int, unsigned int);
void b_freebitmap(void);
void b_setmaskpixel(unsigned int, unsigned int, unsigned int);
void b_line(unsigned int, unsigned int, unsigned int, unsigned int);
void b_charsize(unsigned int);
void b_putc(unsigned int, unsigned int, char, unsigned int);
int b_setlinetype(int);
void b_setvalue(unsigned int);
int b_move(unsigned int, unsigned int);
int b_vector(unsigned int, unsigned int);
int b_put_text(unsigned int, unsigned int, char *);
int b_text_angle(int);
unsigned int b_getpixel(unsigned int, unsigned int);
