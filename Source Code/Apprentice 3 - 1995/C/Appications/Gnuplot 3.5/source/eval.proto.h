
/* eval.c */
struct udvt_entry *add_udv(int);
struct udft_entry *add_udf(int);
union argument *add_action(enum operators);
int standard(int);
int execute_at(struct at_type *);
