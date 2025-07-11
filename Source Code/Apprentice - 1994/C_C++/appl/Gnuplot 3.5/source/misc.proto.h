
/* misc.c */
char *alloc(unsigned long, char *);
struct curve_points *cp_alloc(int);
int cp_extend(struct curve_points *, int);
int cp_free(struct curve_points *);
struct iso_curve *iso_alloc(int);
int iso_extend(struct iso_curve *, int);
int iso_free(struct iso_curve *);
struct surface_points *sp_alloc(int, int, int, int);
int sp_replace(struct surface_points *, int, int, int, int);
int sp_free(struct surface_points *);
int save_functions(FILE *);
int save_variables(FILE *);
int save_all(FILE *);
int save_set(FILE *);
int save_set_all(FILE *);
int save_tics(FILE *, TBOOLEAN, char, struct ticdef *);
int load_file(FILE *, char *);
FILE *lf_top(void);
int load_file_error(void);
int instring(char *, char);
int show_functions(void);
int show_at(void);
int disp_at(struct at_type *, int);
