
/* parse.c */
void fpe(void);
int evaluate_at(struct at_type *, struct value *);
struct value *const_express(struct value *);
struct at_type *temp_at(void);
struct at_type *perm_at(void);
int express(void);
int xterm(void);
int aterm(void);
int bterm(void);
int cterm(void);
int dterm(void);
int eterm(void);
int fterm(void);
int gterm(void);
int hterm(void);
int factor(void);
int xterms(void);
int aterms(void);
int bterms(void);
int cterms(void);
int dterms(void);
int eterms(void);
int fterms(void);
int gterms(void);
int hterms(void);
int iterms(void);
int unary(void);
