char global_merde[80];
static char *RCSid = "$Id: term.c%v 3.50.1.17 1993/08/27 05:21:33 woo Exp woo $";
char * alloc(size_t size, char *message);
extern short __F_MIN[], __F_MAX[], __F_EPSILON[];
extern short __D_MIN[], __D_MAX[], __D_EPSILON[];
extern short __X_MIN[], __X_MAX[], __X_EPSILON[];
typedef int TBOOLEAN;
typedef int (*FUNC_PTR)();
enum operators {
PUSH, PUSHC, PUSHD1, PUSHD2, PUSHD, CALL, CALLN, LNOT, BNOT, UMINUS,
LOR, LAND, BOR, XOR, BAND, EQ, NE, GT, LT, GE, LE, PLUS, MINUS, MULT,
DIV, MOD, POWER, FACTORIAL, BOOLE, JUMP, JUMPZ, JUMPNZ, JTERN, SF_START
};
enum DATA_TYPES {
INTGR, CMPLX
};
enum PLOT_TYPE {
FUNC, DATA, FUNC3D, DATA3D
};
enum PLOT_STYLE {
LINES, POINTSTYLE, IMPULSES, LINESPOINTS, DOTS, ERRORBARS, BOXES, BOXERROR, STEPS
};
enum JUSTIFY {
LEFT, CENTRE, RIGHT
};
struct cmplx {
double real, imag;
};
struct value {
enum DATA_TYPES type;
union {
int int_val;
struct cmplx cmplx_val;
} v;
};
struct lexical_unit {
TBOOLEAN is_token;
struct value l_val;
int start_index;
int length;
};
struct ft_entry {
char *f_name;
FUNC_PTR func;
};
struct udft_entry {
struct udft_entry *next_udf;
char udf_name[50+1];
struct at_type *at;
char *definition;
struct value dummy_values[5];
};
struct udvt_entry {
struct udvt_entry *next_udv;
char udv_name[50+1];
TBOOLEAN udv_undef;
struct value udv_value;
};
union argument {
int j_arg;
struct value v_arg;
struct udvt_entry *udv_arg;
struct udft_entry *udf_arg;
};
struct at_entry {
enum operators index;
union argument arg;
};
struct at_type {
int a_count;
struct at_entry actions[150];
};
enum coord_type {
INRANGE,
OUTRANGE,
UNDEFINED
};
typedef double coordval;
struct coordinate {
enum coord_type type;
coordval x, y, z;
coordval ylow, yhigh;
};
struct curve_points {
struct curve_points *next_cp;
enum PLOT_TYPE plot_type;
enum PLOT_STYLE plot_style;
char *title;
int line_type;
int point_type;
int p_max;
int p_count;
struct coordinate *points;
};
struct gnuplot_contours {
struct gnuplot_contours *next;
struct coordinate *coords;
char isNewLevel;
char label[12];
int num_pts;
};
struct iso_curve {
struct iso_curve *next;
int p_max;
int p_count;
struct coordinate *points;
};
struct surface_points {
struct surface_points *next_sp;
enum PLOT_TYPE plot_type;
enum PLOT_STYLE plot_style;
char *title;
int line_type;
int point_type;
int has_grid_topology;
int num_iso_read;
struct gnuplot_contours *contours;
struct iso_curve *iso_crvs;
};
struct TERMENTRY {
char *name;
char *description;
unsigned int xmax,ymax,v_char,h_char,v_tic,h_tic;
	FUNC_PTR options,init,reset,text,scale,graphics,move,vector,linetype;
	int (*put_text)(unsigned int, unsigned int, char *);
	FUNC_PTR text_angle,justify_text,point,arrow;
};
struct text_label {
struct text_label *next;
int tag;
double x,y,z;
enum JUSTIFY pos;
char text[1024+1];
};
struct arrow_def {
struct arrow_def *next;
int tag;
double sx,sy,sz;
double ex,ey,ez;
TBOOLEAN head;
};
struct ticdef {
int type;
union {
struct {
double start, incr;
double end;
} series;
struct ticmark *user;
} def;
};
struct ticmark {
double position;
char *label;
struct ticmark *next;
};
extern TBOOLEAN screen_ok;
extern TBOOLEAN term_init;
extern TBOOLEAN undefined;
extern struct TERMENTRY term_tbl[];
extern struct curve_points *cp_alloc();
extern int cp_extend();
extern int cp_free();
extern struct surface_points *sp_alloc();
extern int sp_replace();
extern int sp_free();
extern struct iso_curve *iso_alloc();
extern int iso_extend();
extern int iso_free();
typedef double transform_matrix[4][4];
struct gnuplot_contours *contour(int, struct iso_curve *, int, int, int, int, int, double *);
int add_cntr_point(double, double);
int end_crnt_cntr(void);
struct udvt_entry *add_udv(int);
struct udft_entry *add_udf(int);
union argument *add_action(enum operators);
int standard(int);
int execute_at(struct at_type *);
int scanner(char[]);
int get_num(char[]);
int substitute(char *, int);
int f_real(void);
int f_imag(void);
int f_arg(void);
int f_conjg(void);
int f_sin(void);
int f_cos(void);
int f_tan(void);
int f_asin(void);
int f_acos(void);
int f_atan(void);
int f_sinh(void);
int f_cosh(void);
int f_tanh(void);
int f_int(void);
int f_abs(void);
int f_sgn(void);
int f_sqrt(void);
int f_exp(void);
int f_log10(void);
int f_log(void);
int f_floor(void);
int f_ceil(void);
double jzero(double);
double pzero(double);
double qzero(double);
double yzero(double);
double rj0(double);
double ry0(double);
double jone(double);
double pone(double);
double qone(double);
double yone(double);
double rj1(double);
double ry1(double);
int f_besj0(void);
int f_besj1(void);
int f_besy0(void);
int f_besy1(void);
int chr_in_str(int, char);
int equals(int, char *);
int almost_equals(int, char *);
int isstring(int);
int isnumber(int);
int isletter(int);
int is_definition(int);
int copy_str(char[], int);
int quote_str(char[], int);
int quotel_str(char[], int);
int capture(char[], int, int);
int m_capture(char **, int, int);
int m_quote_capture(char **, int, int);
int convert(struct value *, int);
int disp_value(FILE *, struct value *);
double real(struct value *);
double imag(struct value *);
double magnitude(struct value *);
double angle(struct value *);
struct value *Gcomplex(struct value *, double, double);
struct value *Ginteger(struct value *, int);
int os_error(char[], int);
int int_error(char[], int);
void lower_case(char *);
void squash_spaces(char *);
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
int help(char *, char *, TBOOLEAN *);
void FreeHelp(void);
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
void tc_interrupt(void);
int gnu_main(int, int);
int unix_main(int, char **);
int purec_matherr(struct exception *);
int interrupt_setup(void);
char *gpfaralloc(unsigned long, char *);
char *gpfarrealloc(char *, unsigned long);
void gpfarfree(char *);
int is_binary_file(FILE *);
int fread_matrix(FILE *, float * * *, int *, int *, float * *, float * *);
int fwrite_matrix(FILE *, float * *, int, int, int, int, float *, float *);
float *vector(int, int);
void free_vector(float *, int, int);
float *extend_vector(float *, int, int, int);
float *retract_vector(float *, int, int, int);
float * *matrix(int, int, int, int);
void free_matrix(float * *, unsigned, unsigned, unsigned, unsigned);
float * *extend_matrix(float * *, int, int, int, int, int, int);
float * *retract_matrix(float * *, int, int, int, int, int, int);
float * *convert_matrix(float *, int, int, int, int);
void free_convert_matrix(float * *, int, int, int, int);
int get_binary_data(struct surface_points *, FILE *, struct iso_curve **);
int matherr(void);
int reset_stack(void);
int check_stack(void);
struct value *pop(struct value *);
int push(struct value *);
int f_push(union argument *);
int f_pushc(union argument *);
int f_pushd1(union argument *);
int f_pushd2(union argument *);
int f_pushd(union argument *);
int f_call(union argument *);
int f_calln(union argument *);
int f_lnot(void);
int f_bnot(void);
int f_bool(void);
int f_lor(void);
int f_land(void);
int f_bor(void);
int f_xor(void);
int f_band(void);
int f_uminus(void);
int f_eq(void);
int f_ne(void);
int f_gt(void);
int f_lt(void);
int f_ge(void);
int f_le(void);
int f_plus(void);
int f_minus(void);
int f_mult(void);
int f_div(void);
int f_mod(void);
int f_power(void);
int f_factorial(void);
int f_jump(union argument *);
int f_jumpz(union argument *);
int f_jumpnz(union argument *);
int f_jtern(union argument *);
int user_putc(int);
int user_puts(char *);
int backspace(void);
char *readline(char *);
void add_history(char *);
int f_erf(void);
int f_erfc(void);
int f_ibeta(void);
int f_igamma(void);
int f_gamma(void);
int f_lgamma(void);
int f_rand(void);
int f_rand(void);
int f_normal(void);
int f_inverse_normal(void);
int f_inverse_erf(void);
double inverse_normal_func(double);
double inverse_error_func(double);
int chdir(char *);
char *getcwd(char *, int);
int do_point(int, int, int);
int line_and_point(int, int, int);
int do_arrow(int, int, int, int, TBOOLEAN);
int options_null(void);
int list_terms(void);
int set_term(int);
int change_term(char *, int);
int init_terminal(void);
char *turboc_init(void);
char *ztc_init(void);
int UP_redirect(int);
int UP_redirect(int);
int test_term(void);
void reopen_binary(void);
char *vms_init(void);
void vms_reset(void);
void term_mode_tek(void);
void term_mode_native(void);
void term_pasthru(void);
void term_nopasthru(void);
void reopen_binary(void);
void fflush_binary(void);
void set_command(void);
enum PLOT_STYLE get_style(void);
TBOOLEAN load_range(double *, double *);
void show_command(void);
void show_version(void);
void set_command(void);
enum PLOT_STYLE get_style(void);
TBOOLEAN load_range(double *, double *);
void show_command(void);
void show_version(void);
void mat_mult(transform_matrix, transform_matrix, transform_matrix);
int setlinestyle(int);
void clip_move(int, int);
void clip_vector(int, int);
void clip_put_text(int, int, char *);
double make_3dtics(double, double, int, TBOOLEAN, double);
int do_3dplot(struct surface_points *, int, double, double, double, double, double, double);
int update_extrema_pts(int, int, int *, int *, int *, int *, double , double );
int draw_bottom_grid(struct surface_points *, double, double);
int draw_month_3dxtics(double);
int draw_month_3dytics(double);
int draw_month_3dztics(double, double, double, double);
int draw_day_3dxtics(double);
int draw_day_3dytics(double);
int draw_day_3dztics(double, double, double, double);
int xtick3d(double place, char *text, double spacing, double ticscale, double ypos);
int ytick3d(double place, char *text, double spacing, double ticscale, double xpos);
int ztick3d(double place, char *text, double spacing, double ticscale, double xpos, double ypos);
int mat_scale(double, double, double, transform_matrix);
int mat_rot_x(double, transform_matrix);
int mat_rot_z(double, transform_matrix);
void mat_mult(transform_matrix, transform_matrix, transform_matrix);
int map3d_xy(double, double, double, int *, int *);
int map3d_z(double, double, double);
int do_3dplot(struct surface_points *, int, double, double, double, double, double, double);
int draw_bottom_grid(struct surface_points *, double , double);
int draw_month_3dxtics(double);
int draw_month_3dytics(double);
int draw_month_3dztics(double, double, double, double);
int draw_day_3dxtics(double);
int draw_day_3dytics(double);
int draw_day_3dztics(double, double, double, double);
int xtick3d(double place, char *text, double spacing, double ticscale, double ypos);
int ytick3d(double place, char *text, double spacing, double ticscale, double xpos);
int ztick3d(double place, char *text, double spacing, double ticscale, double xpos, double ypos);
int purec_sscanf(const char *, const char *, ...);
int com_line(void);
int do_line(void);
int command(void);
int replotrequest(void);
int plotrequest(void);
int plot3drequest(void);
int define(void);
int get_data(struct curve_points *);
int store2d_point(struct curve_points *, int, double, double, double, double, double);
int adjust_yrange(struct curve_points *);
int grid_nongrid_data(struct surface_points *);
int get_3ddata(struct surface_points *);
int print_points(int);
int print_table(void);
int print_3dtable(int);
int eval_plots(void);
int eval_3dplots(void);
int done(int);
void parametric_fixup(struct curve_points *, int *, double *, double *);
void parametric_3dfixup(struct surface_points *, int *, double *, double *, double *, double *, double *, double *);
int read_line(char *);
int do_help(void);
int do_shell(void);
int do_system(void);
int purec_sscanf(const char *, const char *, ...);
int com_line(void);
int do_line(void);
int command(void);
int replotrequest(void);
int plotrequest(void);
int plot3drequest(void);
int define(void);
int get_data(struct curve_points *);
int store2d_point(struct curve_points *, int, double, double, double, double, double);
int adjust_yrange(struct curve_points *);
int grid_nongrid_data(struct surface_points *);
int get_3ddata(struct surface_points *);
int print_points(int);
int print_table(void);
int print_3dtable(int);
int eval_plots(void);
int eval_3dplots(void);
int done(int);
void parametric_fixup(struct curve_points *, int *, double *, double *);
void parametric_3dfixup(struct surface_points *, int *, double *, double *, double *, double *, double *, double *);
int read_line(char *);
int do_help(void);
int do_shell(void);
int do_system(void);
int do_help(void);
double LogScale(double, TBOOLEAN, double, char *, char *);
int boundary(TBOOLEAN);
double dbl_raise(double, int);
double make_tics(double, double, TBOOLEAN, double);
int do_plot(struct curve_points *, int, double, double, double, double);
void plot_impulses(struct curve_points *, int, int);
void plot_lines(struct curve_points *);
void plot_steps(struct curve_points *);
void plot_bars(struct curve_points *);
void plot_boxes(struct curve_points *, int);
void plot_points(struct curve_points *);
void plot_dots(struct curve_points *);
void edge_intersect(struct coordinate *, int, double *, double *);
void edge_intersect_steps(struct coordinate *, int, double *, double *);
TBOOLEAN two_edge_intersect_steps(struct coordinate *, int, double *, double *);
TBOOLEAN two_edge_intersect(struct coordinate *, int, double *, double *);
int polar_xform(struct curve_points *, int);
int draw_ytics(double, double, double);
int draw_xtics(double, double, double);
int draw_series_ytics(double, double, double);
int draw_series_xtics(double, double, double);
int draw_month_ytics(void);
int draw_month_xtics(void);
int draw_day_ytics(void);
int draw_day_xtics(void);
int draw_set_ytics(struct ticmark *);
int draw_set_xtics(struct ticmark *);
int ytick(double, char *, double, double);
int xtick(double, char *, double, double);
double LogScale(double, TBOOLEAN, double, char *, char *);
int boundary(TBOOLEAN);
double dbl_raise(double, int);
double make_tics(double, double, TBOOLEAN, double);
int do_plot(struct curve_points *, int, double, double, double, double);
void plot_impulses(struct curve_points *, int, int);
void plot_lines(struct curve_points *);
void plot_steps(struct curve_points *);
void plot_bars(struct curve_points *);
void plot_boxes(struct curve_points *, int);
void plot_points(struct curve_points *);
void plot_dots(struct curve_points *);
void edge_intersect(struct coordinate *, int, double *, double *);
void edge_intersect_steps(struct coordinate *, int, double *, double *);
TBOOLEAN two_edge_intersect_steps(struct coordinate *, int, double *, double *);
TBOOLEAN two_edge_intersect(struct coordinate *, int, double *, double *);
int polar_xform(struct curve_points *, int);
int draw_ytics(double, double, double);
int draw_xtics(double, double, double);
int draw_series_ytics(double, double, double);
int draw_series_xtics(double, double, double);
int draw_month_ytics(void);
int draw_month_xtics(void);
int draw_day_ytics(void);
int draw_day_xtics(void);
int draw_set_ytics(struct ticmark *);
int draw_set_xtics(struct ticmark *);
int ytick(double, char *, double, double);
int xtick(double, char *, double, double);
extern TBOOLEAN autoscale_r;
extern TBOOLEAN autoscale_t;
extern TBOOLEAN autoscale_u;
extern TBOOLEAN autoscale_v;
extern TBOOLEAN autoscale_x;
extern TBOOLEAN autoscale_y;
extern TBOOLEAN autoscale_z;
extern TBOOLEAN autoscale_lt;
extern TBOOLEAN autoscale_lu;
extern TBOOLEAN autoscale_lv;
extern TBOOLEAN autoscale_lx;
extern TBOOLEAN autoscale_ly;
extern TBOOLEAN autoscale_lz;
extern double boxwidth;
extern TBOOLEAN clip_points;
extern TBOOLEAN clip_lines1;
extern TBOOLEAN clip_lines2;
extern TBOOLEAN draw_border;
extern TBOOLEAN draw_surface;
extern TBOOLEAN timedate;
extern char dummy_var[5][50+1];
extern char xformat[];
extern char yformat[];
extern char zformat[];
extern enum PLOT_STYLE data_style, func_style;
extern TBOOLEAN grid;
extern int key;
extern double key_x, key_y, key_z;
extern TBOOLEAN is_log_x, is_log_y, is_log_z;
extern double base_log_x, base_log_y, base_log_z;
extern double log_base_log_x, log_base_log_y, log_base_log_z;
extern FILE* outfile;
extern char outstr[];
extern TBOOLEAN parametric;
extern TBOOLEAN polar;
extern TBOOLEAN hidden3d;
extern int angles_format;
extern int mapping3d;
extern int samples;
extern int samples_1;
extern int samples_2;
extern int iso_samples_1;
extern int iso_samples_2;
extern float xsize;
extern float ysize;
extern float zsize;
extern float surface_rot_z;
extern float surface_rot_x;
extern float surface_scale;
extern float surface_zscale;
extern int term;
extern char term_options[];
extern char title[];
extern char xlabel[];
extern char ylabel[];
extern char zlabel[];
extern int time_xoffset;
extern int time_yoffset;
extern int title_xoffset;
extern int title_yoffset;
extern int xlabel_xoffset;
extern int xlabel_yoffset;
extern int ylabel_xoffset;
extern int ylabel_yoffset;
extern int zlabel_xoffset;
extern int zlabel_yoffset;
extern double rmin, rmax;
extern double tmin, tmax, umin, umax, vmin, vmax;
extern double xmin, xmax, ymin, ymax, zmin, zmax;
extern double loff, roff, toff, boff;
extern int draw_contour;
extern TBOOLEAN label_contours;
extern int contour_pts;
extern int contour_kind;
extern int contour_order;
extern int contour_levels;
extern double zero;
extern int levels_kind;
extern double levels_list[30];
extern int dgrid3d_row_fineness;
extern int dgrid3d_col_fineness;
extern int dgrid3d_norm_value;
extern TBOOLEAN dgrid3d;
extern TBOOLEAN xzeroaxis;
extern TBOOLEAN yzeroaxis;
extern TBOOLEAN xtics;
extern TBOOLEAN ytics;
extern TBOOLEAN ztics;
extern float ticslevel;
extern struct ticdef xticdef;
extern struct ticdef yticdef;
extern struct ticdef zticdef;
extern TBOOLEAN tic_in;
extern struct text_label *first_label;
extern struct arrow_def *first_arrow;
extern void set_command();
extern void show_command();
extern enum PLOT_STYLE get_style();
extern TBOOLEAN load_range();
extern void show_version();
typedef unsigned int char_row;
typedef char_row * char_box;
extern char_row fnt5x9[96][9];
extern char_row fnt9x17[96][17];
extern char_row fnt13x25[96][25];
typedef unsigned char pixels;
typedef pixels **bitmap;
extern bitmap *b_p;
extern unsigned int b_currx, b_curry;
extern unsigned int b_xsize, b_ysize;
extern unsigned int b_planes;
extern unsigned int b_psize;
extern unsigned int b_rastermode;
extern unsigned int b_linemask;
extern unsigned int b_value;
extern unsigned int b_hchar;
extern unsigned int b_hbits;
extern unsigned int b_vchar;
extern unsigned int b_vbits;
extern unsigned int b_angle;
extern char_box b_font[96];
extern unsigned int b_pattern[];
extern int b_maskcount;
extern unsigned int b_lastx, b_lasty;
TBOOLEAN term_init;
extern FILE *outfile;
extern char outstr[];
extern TBOOLEAN term_init;
extern int term;
extern float xsize, ysize;
extern char input_line[];
extern struct lexical_unit token[];
extern int num_tokens, c_token;
extern struct value *const_express();
extern TBOOLEAN interactive;
extern double sqrt();
char *getenv();
int unixplot=0;
do_point(x,y,number)
int x,y;
int number;
{
register int htic,vtic;
register struct TERMENTRY *t = &term_tbl[term];
if (number < 0) {
(*t->move)(x,y);
(*t->vector)(x,y);
return(0);
}
number %= 6;
htic = (t->h_tic/2);
vtic = (t->v_tic/2);
switch(number) {
case 0:
(*t->move)(x-htic,y);
(*t->vector)(x,y-vtic);
(*t->vector)(x+htic,y);
(*t->vector)(x,y+vtic);
(*t->vector)(x-htic,y);
(*t->move)(x,y);
(*t->vector)(x,y);
break;
case 1:
(*t->move)(x-htic,y);
(*t->vector)(x-htic,y);
(*t->vector)(x+htic,y);
(*t->move)(x,y-vtic);
(*t->vector)(x,y-vtic);
(*t->vector)(x,y+vtic);
break;
case 2:
(*t->move)(x-htic,y-vtic);
(*t->vector)(x-htic,y-vtic);
(*t->vector)(x+htic,y-vtic);
(*t->vector)(x+htic,y+vtic);
(*t->vector)(x-htic,y+vtic);
(*t->vector)(x-htic,y-vtic);
(*t->move)(x,y);
(*t->vector)(x,y);
break;
case 3:
(*t->move)(x-htic,y-vtic);
(*t->vector)(x-htic,y-vtic);
(*t->vector)(x+htic,y+vtic);
(*t->move)(x-htic,y+vtic);
(*t->vector)(x-htic,y+vtic);
(*t->vector)(x+htic,y-vtic);
break;
case 4:
(*t->move)(x,y+(4*vtic/3));
(*t->vector)(x-(4*htic/3),y-(2*vtic/3));
(*t->vector)(x+(4*htic/3),y-(2*vtic/3));
(*t->vector)(x,y+(4*vtic/3));
(*t->move)(x,y);
(*t->vector)(x,y);
break;
case 5:
(*t->move)(x-htic,y);
(*t->vector)(x-htic,y);
(*t->vector)(x+htic,y);
(*t->move)(x,y-vtic);
(*t->vector)(x,y-vtic);
(*t->vector)(x,y+vtic);
(*t->move)(x-htic,y-vtic);
(*t->vector)(x-htic,y-vtic);
(*t->vector)(x+htic,y+vtic);
(*t->move)(x-htic,y+vtic);
(*t->vector)(x-htic,y+vtic);
(*t->vector)(x+htic,y-vtic);
break;
}
}
line_and_point(x,y,number)
int x,y,number;
{
(*term_tbl[term].linetype)(0);
do_point(x,y,number);
}
do_arrow(sx, sy, ex, ey, head)
int sx,sy;
int ex, ey;
TBOOLEAN head;
{
register struct TERMENTRY *t = &term_tbl[term];
int len = (t->h_tic + t->v_tic)/2;
(*t->move)(sx, sy);
(*t->vector)(ex, ey);
if (head) {
if (sx == ex) {
int delta = ((float)len / (1.41421) + 0.5);
if (sy < ey)
delta = -delta;
(*t->move)(ex - delta, ey + delta);
(*t->vector)(ex,ey);
(*t->vector)(ex + delta, ey + delta);
} else {
int dx = sx - ex;
int dy = sy - ey;
double coeff = len / sqrt(2.0*((double)dx*(double)dx
+ (double)dy*(double)dy));
int x,y;
x = (int)( ex + (dx + dy) * coeff );
y = (int)( ey + (dy - dx) * coeff );
(*t->move)(x,y);
(*t->vector)(ex,ey);
x = (int)( ex + (dx - dy) * coeff );
y = (int)( ey + (dy + dx) * coeff );
(*t->vector)(x,y);
}
}
}
static struct mif_line {
float fpos_x;
float fpos_y;
struct mif_line *next;
struct mif_line *prev;
} mif_line = {
(((float) (0)) / 1000.0),
(((float) ((10000 - 1))) / 1000.0),
&mif_line,
&mif_line
};
static char mif_justify[64];
static char mif_pen[64], mif_pen_width[64], mif_separation[64];
static int mif_text_ang = 0;
static int mif_pentype = 0;
static int mif_pattern_table[16] = {
0,
1,
2, 3, 4, 8, 12, 13,
5,
9, 10, 11, 12, 13, 14, 15
};
static struct mif_group_id {
int group_existance;
int group_id;
} mif_group_id[20];
static int mif_initialized = 0;
static int mif_in_frame = 0;
static int mif_frameno = -1;
static int mif_colour = 1;
static int mif_polyline = 1;
static int insert_mif_line(), proc_group_id();
MIF_options()
{
extern char term_options[];
if (!(c_token >= num_tokens || equals(c_token,";"))) {
if (!(c_token >= num_tokens || equals(c_token,";")) && almost_equals(c_token, "m$onochrome")) {
mif_colour = 0;
c_token++;
}
if (!(c_token >= num_tokens || equals(c_token,";")) && almost_equals(c_token, "c$olour")) {
mif_colour = 1;
c_token++;
}
if (!(c_token >= num_tokens || equals(c_token,";")) && almost_equals(c_token, "v$ectors")) {
mif_polyline = 0;
c_token++;
}
if (!(c_token >= num_tokens || equals(c_token,";")) && almost_equals(c_token, "p$olyline")) {
mif_polyline = 1;
c_token++;
}
if ( !(c_token >= num_tokens || equals(c_token,";")) &&
(almost_equals(c_token, "h$elp") ||
almost_equals(c_token, "?$")) ) {
fprintf((&__file[2]), "Usage: set terminal mif [options]\n");
fprintf((&__file[2]), "\toptions:\n");
fprintf((&__file[2]), "\t\tcolour /        Draw primitives with line types >= 0 in colour (sep. 2-7)\n");
fprintf((&__file[2]), "\t\tmonochrome      Draw primitives in black (sep. 0)                        \n");
fprintf((&__file[2]), "\n");
fprintf((&__file[2]), "\t\tpolyline /      Draw lines as continous curves                           \n");
fprintf((&__file[2]), "\t\tvectors         Draw lines as collections of vectors                     \n");
fprintf((&__file[2]), "\n");
fprintf((&__file[2]), "\t\thelp / ?        Print short usage description on stderr                  \n");
c_token++;
}
}
sprintf(term_options, "%s %s", (mif_colour == 1)? "colour": "monochrome",
(mif_polyline == 1)? "polyline": "vectors");
}
static int free_mif_line()
{
struct mif_line *tline;
while (mif_line.prev != &mif_line) {
tline = mif_line.prev;
mif_line.prev = mif_line.prev->prev;
mif_line.prev->next = &mif_line;
free(tline);
}
mif_line.prev = &mif_line;
mif_line.next = &mif_line;
}
static int put_mif_line()
{
int np, i;
struct mif_line *tline;
if (mif_initialized != 0 && mif_in_frame != 0) {
for (tline = mif_line.next, np = 1; tline != &mif_line; tline = tline->next, np++)
;
if (np >= 2) {
fprintf(outfile, "\t<PolyLine <GroupID %d> %s %s %s\n",
( 1 + (mif_pentype) ), mif_pen, mif_pen_width, mif_separation);
fprintf(outfile, "\t\t<NumPoints %d> ", np);
for (i = 0, tline = &mif_line; i < np; i++, tline = tline->next) {
if (i%4 == 0)
fprintf(outfile, "\n\t\t");
fprintf(outfile, "<Point  %.3f %.3f> ", tline->fpos_x, tline->fpos_y);
}
fprintf(outfile, "\n\t>\n");
proc_group_id(( 1 + (mif_pentype) ));
mif_pen[0] = '\0';
mif_pen_width[0] = '\0';
mif_separation[0] = '\0';
mif_line.fpos_x = mif_line.prev->fpos_x;
mif_line.fpos_y = mif_line.prev->fpos_y;
free_mif_line();
}
}
}
MIF_init()
{
int i;
extern char version[];
extern char patchlevel[];
if (mif_initialized == 0 && mif_in_frame == 0) {
mif_initialized = 1;
mif_in_frame = 0;
free_mif_line();
mif_line.fpos_x = (((float) (0)) / 1000.0);
mif_line.fpos_y = (((float) ((10000 - 1))) / 1000.0);
mif_pen[0] = '\0';
mif_pen_width[0] = '\0';
mif_separation[0] = '\0';
sprintf(mif_justify, " <TLAlignment Left> ");
for (i = 0; i < 20; i++) {
mif_group_id[i].group_id = 0;
mif_group_id[i].group_existance = 0;
}
fprintf(outfile, "<MIFFile 3.00> # Generated by gnuplot version %s patchlevel %s; identifies this as a MIF file\n", version, patchlevel);
fprintf(outfile, "#\n");
fprintf(outfile, "# Set a default pen pattern, pen width, unit and font for subsequent objects\n");
fprintf(outfile, "<Pen 0>\n");
fprintf(outfile, "<Fill 15>\n");
fprintf(outfile, "<PenWidth 0.5 pt>\n");
fprintf(outfile, "<Separation 0>\n");
fprintf(outfile, "<Units Ucm>\n");
fprintf(outfile, "<Font <FFamily `Times'> <FSize %d> <FPlain Yes>>\n", 9);
fprintf(outfile, "#\n");
}
}
MIF_reset()
{
if (mif_initialized != 0 && mif_in_frame == 0) {
fprintf(outfile, "#\n");
fprintf(outfile, "# End of MIFFile\n");
mif_initialized = 0;
}
}
MIF_graphics()
{
int i;
if (mif_initialized != 0 && mif_in_frame == 0) {
mif_in_frame = 1;
mif_frameno++;
free_mif_line();
mif_line.fpos_x = (((float) (0)) / 1000.0);
mif_line.fpos_y = (((float) ((10000 - 1))) / 1000.0);
mif_pen[0] = '\0';
mif_pen_width[0] = '\0';
mif_separation[0] = '\0';
sprintf(mif_justify, " <TLAlignment Left> ");
for (i = 0; i < 20; i++) {
mif_group_id[i].group_id = 0;
mif_group_id[i].group_existance = 0;
}
fprintf(outfile, "#\n");
fprintf(outfile, "# Frame number %d with plot of graphics\n", mif_frameno);
fprintf(outfile, "<Frame\n");
fprintf(outfile, "\t<Pen 15>\n");
fprintf(outfile, "\t<Fill 15>\n");
fprintf(outfile, "\t<PenWidth  0.5 pt>\n");
fprintf(outfile, "\t<Separation 0>\n");
fprintf(outfile, "\t<BRect 0.000 %.3f %.3f %.3f>\n",
((float) mif_frameno)*(((float) (10000+100)) / 1000.0), (((float) (15000)) / 1000.0), (((float) (10000)) / 1000.0));
fprintf(outfile, "\t<NSOffset  0.000>\n");
fprintf(outfile, "\t<BLOffset  0.000>\n");
}
}
MIF_text()
{
int i;
if (mif_initialized != 0 && mif_in_frame != 0) {
if (mif_polyline == 1)
put_mif_line();
fprintf(outfile, "\t#\n");
fprintf(outfile, "\t# Group the the objects in groups to make the chart easier to manipulate\n");
fprintf(outfile, "\t# after it's imported into FrameMaker.\n");
for (i = 0; i < 20; i++) {
if (mif_group_id[i].group_id != 0 &&
mif_group_id[i].group_existance == 1) {
fprintf(outfile, "\t<Group\n");
fprintf(outfile, "\t\t<ID %d>\n", mif_group_id[i].group_id);
fprintf(outfile, "\t>\n");
}
}
fprintf(outfile, ">\n");
fprintf(outfile, "# End of Frame number %d\n", mif_frameno);
fprintf(outfile, "#\n");
mif_in_frame = 0;
}
}
MIF_linetype(linetype)
int linetype;
{
if (mif_initialized != 0 && mif_in_frame != 0) {
if (mif_polyline == 1)
put_mif_line();
if (linetype < 0) {
if (linetype == -1) {
mif_pentype = 8+16;
if (mif_colour == 1)
sprintf(mif_separation, " <Separation 0> ");
}
else {
mif_pentype = 0+16;
if (mif_colour == 1)
sprintf(mif_separation, " <Separation 0> ");
}
sprintf(mif_pen_width, " <PenWidth 1.0 pt> ");
}
else {
mif_pentype = (linetype)%16;
sprintf(mif_pen_width, " <PenWidth 0.5 pt> ");
if (mif_colour == 1)
sprintf(mif_separation, " <Separation %d> ", 2+(mif_pentype%6));
}
sprintf(mif_pen, " <Pen %d> ", mif_pattern_table[mif_pentype%16]);
}
}
int MIF_text_angle(ang)
int ang;
{
if (ang != 0)
mif_text_ang = 1;
else
mif_text_ang = 0;
return(1);
}
MIF_justify_text(mode)
enum JUSTIFY mode;
{
int rval = 1;
if (mif_initialized != 0 && mif_in_frame != 0) {
switch (mode) {
case LEFT:
sprintf(mif_justify, " <TLAlignment Left> ");
break;
case CENTRE:
sprintf(mif_justify, " <TLAlignment Center> ");
break;
case RIGHT:
sprintf(mif_justify, " <TLAlignment Right> ");
break;
default:
rval = 0;
break;
}
}
else {
rval = 0;
}
return(rval);
}
MIF_vector(x, y)
unsigned int x, y;
{
if (mif_initialized != 0 && mif_in_frame != 0) {
insert_mif_line((((float) (x)) / 1000.0), (((float) ((10000 - 1)-(int)y)) / 1000.0));
if (mif_polyline == 0)
put_mif_line();
}
}
MIF_move(x, y)
unsigned int x, y;
{
if (mif_initialized != 0 && mif_in_frame != 0) {
if (mif_polyline == 1)
put_mif_line();
mif_line.fpos_x = (((float) (x)) / 1000.0);
mif_line.fpos_y = (((float) ((10000 - 1)-(int)y)) / 1000.0);
}
}
MIF_put_text(x, y, str)
unsigned int x, y;
char str[];
{
if (mif_initialized != 0 && mif_in_frame != 0) {
if (mif_polyline == 1)
put_mif_line();
MIF_move(x, y-(10000/31)/5);
if (strlen(str) > 0) {
fprintf(outfile, "\t<TextLine <GroupID %d> %s %s %s\n",
( 1 + (mif_pentype) ), mif_pen, mif_pen_width, mif_separation);
fprintf(outfile, "\t\t<TLOrigin  %.3f %.3f> %s %s <String `%s'>\n",
mif_line.fpos_x, mif_line.fpos_y, mif_justify,
(mif_text_ang == 1)? "<Angle 90>": "",
str);
fprintf(outfile, "\t>\n");
proc_group_id(( 1 + (mif_pentype) ));
mif_pen[0] = '\0';
mif_pen_width[0] = '\0';
mif_separation[0] = '\0';
mif_justify[0] = '\0';
}
}
}
static int insert_mif_line(fx, fy)
float fx, fy;
{
int rval = 1;
if ((mif_line.prev->next = (struct mif_line *) alloc(sizeof(struct mif_line),"MIF driver")) != (struct mif_line *) ((void *) 0)) {
mif_line.prev->next->next = &mif_line;
mif_line.prev->next->prev = mif_line.prev;
mif_line.prev = mif_line.prev->next;
mif_line.prev->fpos_x = fx;
mif_line.prev->fpos_y = fy;
rval = 1;
}
else {
mif_line.prev->next = &mif_line;
rval = 0;
}
return(rval);
}
static int proc_group_id(group_id)
int group_id;
{
int i, rval = 0;
if (mif_initialized != 0 && mif_in_frame != 0) {
for (i = 0; i < 20 &&
mif_group_id[i].group_id != 0 &&
mif_group_id[i].group_id != group_id;
i++) {
}
if (i < 20) {
if (mif_group_id[i].group_id == 0) {
mif_group_id[i].group_id = group_id;
mif_group_id[i].group_existance = 0;
}
else {
if (mif_group_id[i].group_id == group_id) {
mif_group_id[i].group_existance = 1;
rval = 1;
}
}
}
else {
rval = -2;
}
}
else {
rval = -1;
}
return(rval);
}
extern struct __copt {
short top;
short left;
unsigned char *title;
short procID;
short txFont;
short txSize;
short txFace;
short nrows;
short ncols;
short pause_atexit;
} __console_options;
enum { C_RAW, C_CBREAK, C_NOECHO, C_ECHO };
FILE *__fopenc(void);
FILE *__freopenc(FILE *, FILE *);
void __cgotoxy(int, int, FILE *);
void __cgetxy(int *, int *, FILE *);
void __ccleos(FILE *);
void __ccleol(FILE *);
void __csettabs(int, FILE *);
void __csetmode(int, FILE *);
void __cinverse(int, FILE *);
void __cshow(FILE *);
void __chide(FILE *);
void __cecho2file(char *, int, FILE *);
void __cecho2printer(FILE *);
int __ccommand(char ***);
static WindowPtr itsWindow =((void *) 0);
static GrafPtr savePort =((void *) 0);
static PicHandle itsPicture =((void *) 0);
static Handle saveMBar, myMBar;
static Boolean MAC_event(void);
static void SetUpMenu(void);
static void AdjustMenu(void);
static void RestoreMenu(void);
MAC_init()
{
Rect boundsRect = {50, 10, 200, 200};
itsWindow = NewWindow(((void *) 0), &boundsRect,"\pMac Graphic Window", 0,
noGrowDocProc, (void *)-1L, 0, 0L);
SizeWindow (itsWindow,448+40,271+20,0);
GetPort(&savePort);
SetPort(itsWindow);
TextFont(monaco);
TextSize(9);
TextFace(normal);
PenNormal();
SetUpMenu();
SetPort(savePort);
}
MAC_graphics()
{
GetPort(&savePort);
SetPort(itsWindow);
EraseRect(&(((GrafPtr) itsWindow)->portRect));
ShowWindow (itsWindow);
BringToFront (itsWindow);
HiliteWindow (itsWindow,1);
if(itsPicture != ((void *) 0)){
KillPicture(itsPicture);
itsPicture=((void *) 0);
}
itsPicture=OpenPicture (&(((GrafPtr) itsWindow)->portRect));
TextFont(monaco);
TextSize(9);
TextFace(normal);
PenNormal();
}
MAC_text()
{
Rect myRect;
ClosePicture();
if(itsPicture != ((void *) 0)){
myRect=itsWindow->portRect;
DrawPicture(itsPicture,&myRect);
}
if(itsWindow !=((void *) 0)){
SetWTitle(itsWindow, "\pHit any key to continue.");
while(!MAC_event());
SetWTitle(itsWindow, "\pMac Graphic Window");
}
SetPort(savePort);
__cshow((&__file[1]));
}
MAC_linetype(linetype)
int linetype;
{
int lt;
lt=(linetype > 1?linetype:1);
PenSize(lt,lt);
}
MAC_move(x,y)
unsigned int x,y;
{
MoveTo(x+(468-448),271-y);
}
MAC_vector(x,y)
unsigned int x,y;
{
LineTo(x+(468-448),271-y);
}
MAC_put_text(x,y,str)
unsigned int x,y;
char str[];
{
char test[80];
/*
 * Laval : bug 
 */
strcpy(test,global_merde);
MoveTo(x+(468-448),271-y+11/2);
PenNormal();
/*
 * DrawString(CtoPstr(str));
 */
DrawString(CtoPstr(test));
PtoCstr(test);
}
MAC_reset()
{
}
static Boolean MAC_event(){
EventRecord theEvent;
WindowPtr whichWindow;
SystemTask ();
if (GetNextEvent (everyEvent, &theEvent))
switch (theEvent.what){
case mouseDown:
switch(FindWindow(theEvent.where,&whichWindow)){
case inSysWindow:
SystemClick(&theEvent,whichWindow);
break;
case inDrag:{
Rect dragRect;
SetRect(&dragRect,4,24,
screenBits.bounds.right-4,screenBits.bounds.bottom-4);
DragWindow (whichWindow,theEvent.where, &dragRect);
}
break;
case inMenuBar:
AdjustMenu();
doMenuCommand(MenuSelect(theEvent.where));
RestoreMenu();
break;
};
break;
case keyDown:
case autoKey:
if ((theEvent.modifiers & cmdKey) != 0){
AdjustMenu();
doMenuCommand(MenuKey(theEvent.message & charCodeMask));
RestoreMenu();
}
else
return 1;
break;
case updateEvt:
BeginUpdate(itsWindow);
if(itsPicture != ((void *) 0)) DrawPicture(itsPicture,&(itsWindow->portRect));
EndUpdate(itsWindow);
break;
case activateEvt:
InvalRect(&itsWindow->portRect);
break;
}
return 0;
}
static doMenuCommand(long mCmd){
int item,menu;
Str255 daName;
short daRefNum;
item=LoWord(mCmd);
menu=HiWord(mCmd);
switch(menu){
case 1:
switch ( item ) {
default:
GetItem(GetMHandle(1), item, daName);
daRefNum = OpenDeskAcc(daName);
break;
}
break;
case 2:
switch(item){
case 5:
doSave();
break;
}
break;
case 3:
switch(item){
case 4:
doCopy();
break;
}
break;
}
HiliteMenu(0);
}
static doCopy(){
if( ZeroScrap() != 0) {
fprintf((&__file[2]), "Cann't initialize Clippboard\n");
exit(0);
}
else{
HLock(itsPicture);
PutScrap(GetHandleSize(itsPicture),'PICT', *itsPicture);
HUnlock(itsPicture);
}
}
long PICTCount;
int globalRef;
PicHandle newPICTHand;
pascal void PutPICTData(Ptr dataPtr, int byteCount){
long longCount;
int err;
longCount=byteCount;
PICTCount+=byteCount;
err=FSWrite(globalRef, &longCount, dataPtr);
if(newPICTHand != ((void *) 0)) (**newPICTHand).picSize=PICTCount;
}
static doSave(){
Point where={97,103};
SFReply reply;
OSErr err;
int i;
long longCount,longZero;
Rect pFrame;
QDProcs myProcs;
SFPutFile(where,"\pSave picture into", "\pUntitled", ((void *) 0), &reply);
if(reply.good) {
err=Create(reply.fName,reply.vRefNum,'????','PICT');
if( err == 0 || err == dupFNErr) {
FSOpen(reply.fName,reply.vRefNum,&globalRef);
SetStdProcs(&myProcs);
itsWindow->grafProcs=&myProcs;
myProcs.putPicProc=(Ptr) PutPICTData;
longZero=0L;
longCount=4;
PICTCount=sizeof(Picture);
for(i=1;i<=(512/4+sizeof(Picture));i++){
FSWrite(globalRef,&longCount,&longZero);
}
pFrame=(**itsPicture).picFrame;
newPICTHand=((void *) 0);
newPICTHand=OpenPicture(&pFrame);
DrawPicture(itsPicture,&pFrame);
ClosePicture();
SetFPos(globalRef, fsFromStart,512);
longCount=sizeof(Picture);
FSWrite(globalRef,&longCount,(Ptr) (*newPICTHand));
FSClose(globalRef);
itsWindow->grafProcs=((void *) 0);
KillPicture(newPICTHand);
}
else{
}
}
}
static void SetUpMenu(void){
MenuHandle mh;
saveMBar=GetMenuBar();
ClearMenuBar();
mh=NewMenu(1,"\p\024");
InsertMenu(mh,0);
AddResMenu(GetMHandle(1), 'DRVR');
mh=NewMenu(2,"\pFile");
AppendMenu(mh, "\p(New/N;(Open/O;(-;(Close/W;Save/S");
InsertMenu(mh,0);
mh=NewMenu(3,"\pEdit");
AppendMenu(mh, "\p(Undo/Z;(-;(Cut/X;Copy/C");
InsertMenu(mh,0);
DrawMenuBar();
myMBar=GetMenuBar();
}
static void AdjustMenu(void){
SetMenuBar(myMBar);
DrawMenuBar();
}
static void RestoreMenu(void){
SetMenuBar(saveMBar);
DrawMenuBar();
}
char ps_font[50+1] = "Helvetica" ;
int ps_fontsize = 14;
TBOOLEAN ps_portrait = 0;
TBOOLEAN ps_color = 0;
TBOOLEAN ps_solid = 0;
TBOOLEAN ps_eps = 0;
int ps_page=0;
int ps_path_count=0;
int ps_ang=0;
enum JUSTIFY ps_justify=LEFT;
char * PS_header[] = {
"/M {moveto} bind def\n",
"/L {lineto} bind def\n",
"/R {rmoveto} bind def\n",
"/V {rlineto} bind def\n",
"/vpt2 vpt 2 mul def\n",
"/hpt2 hpt 2 mul def\n",
"/Lshow { currentpoint stroke M\n",
"  0 vshift R show } def\n",
"/Rshow { currentpoint stroke M\n",
"  dup stringwidth pop neg vshift R show } def\n",
"/Cshow { currentpoint stroke M\n",
"  dup stringwidth pop -2 div vshift R show } def\n",
"/DL { Color {setrgbcolor Solid {pop []} if 0 setdash }\n",
" {pop pop pop Solid {pop []} if 0 setdash} ifelse } def\n",
"/BL { stroke gnulinewidth 2 mul setlinewidth } def\n",
"/AL { stroke gnulinewidth 2 div setlinewidth } def\n",
"/PL { stroke gnulinewidth setlinewidth } def\n",
"/LTb { BL [] 0 0 0 DL } def\n",
"/LTa { AL [1 dl 2 dl] 0 setdash 0 0 0 setrgbcolor } def\n",
"/LT0 { PL [] 0 1 0 DL } def\n",
"/LT1 { PL [4 dl 2 dl] 0 0 1 DL } def\n",
"/LT2 { PL [2 dl 3 dl] 1 0 0 DL } def\n",
"/LT3 { PL [1 dl 1.5 dl] 1 0 1 DL } def\n",
"/LT4 { PL [5 dl 2 dl 1 dl 2 dl] 0 1 1 DL } def\n",
"/LT5 { PL [4 dl 3 dl 1 dl 3 dl] 1 1 0 DL } def\n",
"/LT6 { PL [2 dl 2 dl 2 dl 4 dl] 0 0 0 DL } def\n",
"/LT7 { PL [2 dl 2 dl 2 dl 2 dl 2 dl 4 dl] 1 0.3 0 DL } def\n",
"/LT8 { PL [2 dl 2 dl 2 dl 2 dl 2 dl 2 dl 2 dl 4 dl] 0.5 0.5 0.5 DL } def\n",
"/P { stroke [] 0 setdash\n",
"  currentlinewidth 2 div sub M\n",
"  0 currentlinewidth V stroke } def\n",
"/D { stroke [] 0 setdash 2 copy vpt add M\n",
"  hpt neg vpt neg V hpt vpt neg V\n",
"  hpt vpt V hpt neg vpt V closepath stroke\n",
"  P } def\n",
"/A { stroke [] 0 setdash vpt sub M 0 vpt2 V\n",
"  currentpoint stroke M\n",
"  hpt neg vpt neg R hpt2 0 V stroke\n",
"  } def\n",
"/B { stroke [] 0 setdash 2 copy exch hpt sub exch vpt add M\n",
"  0 vpt2 neg V hpt2 0 V 0 vpt2 V\n",
"  hpt2 neg 0 V closepath stroke\n",
"  P } def\n",
"/C { stroke [] 0 setdash exch hpt sub exch vpt add M\n",
"  hpt2 vpt2 neg V currentpoint stroke M\n",
"  hpt2 neg 0 R hpt2 vpt2 V stroke } def\n",
"/T { stroke [] 0 setdash 2 copy vpt 1.12 mul add M\n",
"  hpt neg vpt -1.62 mul V\n",
"  hpt 2 mul 0 V\n",
"  hpt neg vpt 1.62 mul V closepath stroke\n",
"  P  } def\n",
"/S { 2 copy A C} def\n",
((void *) 0)
};
int PS_pen_x, PS_pen_y;
int PS_taken;
int PS_linetype_last;
TBOOLEAN PS_relative_ok;
PS_options()
{
extern struct value *const_express();
extern double real();
if (!(c_token >= num_tokens || equals(c_token,";"))) {
if (almost_equals(c_token,"p$ortrait")) {
ps_portrait=1;
ps_eps=0;
c_token++;
}
else if (almost_equals(c_token,"l$andscape")) {
ps_portrait=0;
ps_eps=0;
c_token++;
}
else if (almost_equals(c_token,"e$psf")) {
ps_portrait=1;
ps_eps = 1;
c_token++;
}
else if (almost_equals(c_token,"d$efault")) {
ps_portrait=0;
ps_eps=0;
ps_color=0;
strcpy(ps_font,"Helvetica");
ps_fontsize = 14;
term_tbl[term].v_char = (unsigned int)(ps_fontsize*(10));
term_tbl[term].h_char = (unsigned int)(ps_fontsize*(10)*6/10);
c_token++;
}
}
if (!(c_token >= num_tokens || equals(c_token,";"))) {
if (almost_equals(c_token,"m$onochrome")) {
ps_color=0;
c_token++;
}
else if (almost_equals(c_token,"c$olor")) {
ps_color=1;
c_token++;
}
}
if (!(c_token >= num_tokens || equals(c_token,";"))) {
if (almost_equals(c_token,"s$olid")) {
ps_solid=1;
c_token++;
}
else if (almost_equals(c_token,"d$ashed")) {
ps_solid=0;
c_token++;
}
}
if (!(c_token >= num_tokens || equals(c_token,";")) && isstring(c_token)) {
quote_str(ps_font,c_token);
c_token++;
}
if (!(c_token >= num_tokens || equals(c_token,";"))) {
struct value a;
ps_fontsize = (int)real(const_express(&a));
term_tbl[term].v_char = (unsigned int)(ps_fontsize*(10));
term_tbl[term].h_char = (unsigned int)(ps_fontsize*(10)*6/10);
}
sprintf(term_options,"%s %s %s \"%s\" %d",
ps_eps ? "eps" : (ps_portrait ? "portrait" : "landscape"),
ps_color ? "color" : "monochrome",
ps_solid ? "solid" : "dashed",
ps_font,ps_fontsize);
}
PS_init()
{
static char psi1[] = "%%%%Creator: gnuplot\n%%%%DocumentFonts: %s\n%%%%BoundingBox: %d %d ";
static char psi2[] = "%%%%EndComments\n/gnudict 40 dict def\ngnudict begin\n/Color %s def\n/Solid %s def\n/gnulinewidth %.3f def\n/vshift %d def\n/dl {%d mul} def\n/hpt %.1f def\n/vpt %.1f def\n";
struct TERMENTRY *t = &term_tbl[term];
int i;
ps_page = 0;
if (!ps_eps)
fprintf(outfile,"%%!PS-Adobe-2.0\n");
else
fprintf(outfile,"%%!PS-Adobe-2.0 EPSF-2.0\n");
fprintf(outfile, psi1, ps_font, 50, 50);
if (ps_portrait)
fprintf(outfile,"%d %d\n",
(int)(xsize*(ps_eps ? 0.5 : 1.0)*(7200)/(10)+0.5+50),
(int)(ysize*(ps_eps ? 0.5 : 1.0)*(5040)/(10)+0.5+50) );
else
fprintf(outfile,"%d %d\n",
(int)(ysize*(ps_eps ? 0.5 : 1.0)*(5040)/(10)+0.5+50),
(int)(xsize*(ps_eps ? 0.5 : 1.0)*(7200)/(10)+0.5+50) );
if (!ps_eps)
fprintf(outfile,"%%%%Pages: (atend)\n");
fprintf(outfile, psi2,
ps_color ? "true" : "false",
ps_solid ? "true" : "false",
(0.5*(10)),
(int)(t->v_char)/(-3),
(10),
(5040/80)/2.0,
(5040/80)/2.0);
for ( i=0; PS_header[i] != ((void *) 0); i++)
fprintf(outfile,"%s",PS_header[i]);
fprintf(outfile,"end\n%%%%EndProlog\n");
}
PS_graphics()
{
static char psg1[] = "0 setgray\n/%s findfont %d scalefont setfont\nnewpath\n";
struct TERMENTRY *t = &term_tbl[term];
ps_page++;
if (!ps_eps)
fprintf(outfile,"%%%%Page: %d %d\n",ps_page,ps_page);
fprintf(outfile,"gnudict begin\ngsave\n");
fprintf(outfile,"%d %d translate\n",50,50);
fprintf(outfile,"%.3f %.3f scale\n", (ps_eps ? 0.5 : 1.0)/(10),
(ps_eps ? 0.5 : 1.0)/(10));
if (!ps_portrait) {
fprintf(outfile,"90 rotate\n0 %d translate\n", (int)(-5040*ysize));
}
fprintf(outfile, psg1, ps_font, (t->v_char) );
ps_path_count = 0;
PS_relative_ok = 0;
PS_pen_x = PS_pen_y = -4000;
PS_taken = 0;
PS_linetype_last = -1;
}
PS_text()
{
ps_path_count = 0;
fprintf(outfile,"stroke\ngrestore\nend\nshowpage\n");
}
PS_reset()
{
fprintf(outfile,"%%%%Trailer\n");
if (!ps_eps)
fprintf(outfile,"%%%%Pages: %d\n",ps_page);
}
PS_linetype(linetype)
int linetype;
{
char *line = "ba012345678";
linetype = (linetype % 9) + 2;
PS_relative_ok = 0;
if (PS_linetype_last == linetype) return(0);
PS_linetype_last = linetype;
fprintf(outfile,"LT%c\n", line[linetype]);
ps_path_count = 0;
}
PS_move(x,y)
unsigned int x,y;
{
int dx, dy;
char abso[20],rel[20];
dx = x - PS_pen_x;
dy = y - PS_pen_y;
if (dx==0 && dy==0 && PS_relative_ok)
return(0);
sprintf(abso, "%d %d M\n", x, y);
sprintf(rel, "%d %d R\n", dx, dy);
if (strlen(rel) < strlen(abso) && PS_relative_ok){
fputs(rel, outfile);
PS_taken++;
}else
fputs(abso, outfile);
PS_relative_ok = 1;
ps_path_count += 1;
PS_pen_x = x;
PS_pen_y = y;
}
PS_vector(x,y)
unsigned int x,y;
{
int dx, dy;
char abso[20],rel[20];
dx = x - PS_pen_x;
dy = y - PS_pen_y;
if (dx==0 && dy==0) return(0);
sprintf(abso, "%d %d L\n", x, y);
sprintf(rel, "%d %d V\n", dx, dy);
if (strlen(rel) < strlen(abso) && PS_relative_ok){
fputs(rel, outfile);
PS_taken++;
}else
fputs(abso, outfile);
PS_relative_ok = 1;
ps_path_count += 1;
PS_pen_x = x;
PS_pen_y = y;
if (ps_path_count >= 400) {
fprintf(outfile,"currentpoint stroke M\n");
ps_path_count = 0;
}
}
PS_put_text(x,y,str)
unsigned int x, y;
char *str;
{
char ch;
if (!strlen(str)) return(0);
PS_move(x,y);
if (ps_ang != 0)
fprintf(outfile,"currentpoint gsave translate %d rotate 0 0 M\n"
,ps_ang*90);
((outfile)->cnt-- > 1 ? (int) (*(outfile)->ptr++ = ('(')) : __putc('(', outfile));
ch = *str++;
while(ch!='\0') {
if ( (ch=='(') || (ch==')') || (ch=='\\') )
((outfile)->cnt-- > 1 ? (int) (*(outfile)->ptr++ = ('\\')) : __putc('\\', outfile));
((outfile)->cnt-- > 1 ? (int) (*(outfile)->ptr++ = (ch)) : __putc(ch, outfile));
ch = *str++;
}
switch(ps_justify) {
case LEFT : fprintf(outfile,") Lshow\n");
break;
case CENTRE : fprintf(outfile,") Cshow\n");
break;
case RIGHT : fprintf(outfile,") Rshow\n");
break;
}
if (ps_ang != 0)
fprintf(outfile,"grestore\n");
ps_path_count = 0;
PS_relative_ok = 0;
}
int PS_text_angle(ang)
int ang;
{
ps_ang=ang;
return 1;
}
int PS_justify_text(mode)
enum JUSTIFY mode;
{
ps_justify=mode;
return 1;
}
PS_point(x,y,number)
int x,y;
int number;
{
char *point = "PDABCTS";
number %= 6;
if (number < -1)
number = -1;
fprintf(outfile,"%d %d %c\n", x, y, point[number+1]);
PS_relative_ok = 0;
ps_path_count = 0;
PS_linetype_last = -1;
}
char ai_font[50+1] = "Times-Roman" ;
int ai_fontsize = 14;
TBOOLEAN ai_color = 0;
TBOOLEAN ai_stroke = 0;
int ai_page=0;
int ai_path_count=0;
int ai_ang=0;
enum JUSTIFY ai_justify=LEFT;
AI_options()
{
extern struct value *const_express();
extern double real();
if (!(c_token >= num_tokens || equals(c_token,";"))) {
if (almost_equals(c_token,"d$efault")) {
ai_color=0;
strcpy(ai_font,"Times-Roman");
ai_fontsize = 14;
c_token++;
}
}
if (!(c_token >= num_tokens || equals(c_token,";"))) {
if (almost_equals(c_token,"m$onochrome")) {
ai_color=0;
c_token++;
}
else if (almost_equals(c_token,"c$olor")) {
ai_color=1;
c_token++;
}
}
if (!(c_token >= num_tokens || equals(c_token,";")) && isstring(c_token)) {
quote_str(ai_font,c_token);
c_token++;
}
if (!(c_token >= num_tokens || equals(c_token,";"))) {
struct value a;
ai_fontsize = (int)real(const_express(&a));
c_token++;
term_tbl[term].v_char = (unsigned int)(ai_fontsize*(10.0));
term_tbl[term].h_char = (unsigned int)(ai_fontsize*(10.0)*6/10);
}
sprintf(term_options,"%s \"%s\" %d",
ai_color ? "color" : "monochrome",ai_font,ai_fontsize);
}
AI_init()
{
ai_page = 0;
fprintf(outfile,"%%!PS-Adobe-2.0 EPSF-1.2\n");
fprintf(outfile,"%%%%BoundingBox: %d %d %d %d\n", 50,50,
(int)((5000)/(10.0)+0.5+50),
(int)((3500)/(10.0)+0.5+50) );
fprintf(outfile,"%%%%Template:\n");
fprintf(outfile,"%%%%EndComments\n");
fprintf(outfile,"%%%%EndProlog\n");
}
AI_graphics()
{
ai_page++;
fprintf(outfile,"0 G\n");
fprintf(outfile,"1 j\n");
fprintf(outfile,"1 J\n");
fprintf(outfile,"u\n");
ai_path_count = 0;
ai_stroke = 0;
}
AI_text()
{
if (ai_stroke) {
fprintf(outfile,"S\n");
ai_stroke = 0;
}
fprintf(outfile,"U\n");
ai_path_count = 0;
}
AI_reset()
{
fprintf(outfile,"%%%%Trailer\n");
}
AI_linetype(linetype)
int linetype;
{
if (ai_stroke) {
fprintf(outfile,"S\n");
ai_stroke = 0;
}
switch(linetype) {
case -2 : fprintf(outfile,"%.2f w\n",(0.5*(10.0))/(10.0)*2.0);
if (ai_color) {
fprintf(outfile,"0 0 0 1 K\n");
}
else {
fprintf(outfile,"[] 0 d\n");
}
break;
case -1 : fprintf(outfile,"%.2f w\n",(0.5*(10.0))/(10.0)/2.0);
if (ai_color) {
fprintf(outfile,"0 0 0 1 K\n");
}
else {
fprintf(outfile,"[1 2] 0 d\n");
}
break;
case 0 : fprintf(outfile,"%.2f w\n",(0.5*(10.0))/(10.0));
if (ai_color) {
fprintf(outfile,"1 0 1 0 K\n");
}
else {
fprintf(outfile,"[] 0 d\n");
}
break;
case 1 : fprintf(outfile,"%.2f w\n",(0.5*(10.0))/(10.0));
if (ai_color) {
fprintf(outfile,"1 1 0 0 K\n");
}
else {
fprintf(outfile,"[4 2] 0 d\n");
}
break;
case 2 : fprintf(outfile,"%.2f w\n",(0.5*(10.0))/(10.0));
if (ai_color) {
fprintf(outfile,"0 1 1 0 K\n");
}
else {
fprintf(outfile,"[2 3] 0 d\n");
}
break;
case 3 : fprintf(outfile,"%.2f w\n",(0.5*(10.0))/(10.0));
if (ai_color) {
fprintf(outfile,"0 1 0 0 K\n");
}
else {
fprintf(outfile,"[1 1.5] 0 d\n");
}
break;
case 4 : fprintf(outfile,"%f w\n",(0.5*(10.0))/(10.0));
if (ai_color) {
fprintf(outfile,"1 0 0 0 K\n");
}
else {
fprintf(outfile,"[5 2 1 2] 0 d\n");
}
break;
case 5 : fprintf(outfile,"%.2f w\n",(0.5*(10.0))/(10.0));
if (ai_color) {
fprintf(outfile,"0 0 1 0 K\n");
}
else {
fprintf(outfile,"[4 3 1 3] 0 d\n");
}
break;
case 6 : fprintf(outfile,"%.2f w\n",(0.5*(10.0))/(10.0));
if (ai_color) {
fprintf(outfile,"0 0 0 1 K\n");
}
else {
fprintf(outfile,"[2 2 2 4] 0 d\n");
}
break;
case 7 : fprintf(outfile,"%.2f w\n",(0.5*(10.0))/(10.0));
if (ai_color) {
fprintf(outfile,"0 0.7 1 0 K\n");
}
else {
fprintf(outfile,"[2 2 2 2 2 4] 0 d\n");
}
break;
case 8 : fprintf(outfile,"%.2f w\n",(0.5*(10.0))/(10.0));
if (ai_color) {
fprintf(outfile,"0.5 0.5 0.5 0 K\n");
}
else {
fprintf(outfile,"[2 2 2 2 2 2 2 4] 0 d\n");
}
break;
}
ai_path_count = 0;
}
AI_move(x,y)
unsigned int x,y;
{
if (ai_stroke) fprintf(outfile,"S\n");
fprintf(outfile,"%.2f %.2f m\n", x/(10.0), y/(10.0));
ai_path_count += 1;
ai_stroke = 1;
}
AI_vector(x,y)
unsigned int x,y;
{
fprintf(outfile,"%.2f %.2f l\n", x/(10.0), y/(10.0));
ai_path_count += 1;
ai_stroke = 1;
if (ai_path_count >= 400) {
fprintf(outfile,"S\n%.2f %.2f m\n",x/(10.0),y/(10.0));
ai_path_count = 0;
}
}
AI_put_text(x,y,str)
unsigned int x, y;
char *str;
{
char ch;
if (ai_stroke) {
fprintf(outfile,"S\n");
ai_stroke = 0;
}
switch(ai_justify) {
case LEFT : fprintf(outfile,"/_%s %d 0 0 0 z\n",ai_font,ai_fontsize);
break;
case CENTRE : fprintf(outfile,"/_%s %d 0 0 1 z\n",ai_font,ai_fontsize);
break;
case RIGHT : fprintf(outfile,"/_%s %d 0 0 2 z\n",ai_font,ai_fontsize);
break;
}
if (ai_ang==0) {
fprintf(outfile,"[ 1 0 0 1 %.2f %.2f] e\n",
x/(10.0),y/(10.0) - ai_fontsize/3.0);
}
else {
fprintf(outfile,"[ 0 1 -1 0 %.2f %.2f] e\n",
x/(10.0) - ai_fontsize/3.0,y/(10.0));
}
((outfile)->cnt-- > 1 ? (int) (*(outfile)->ptr++ = ('(')) : __putc('(', outfile));
ch = *str++;
while(ch!='\0') {
if ( (ch=='(') || (ch==')') || (ch=='\\') )
((outfile)->cnt-- > 1 ? (int) (*(outfile)->ptr++ = ('\\')) : __putc('\\', outfile));
((outfile)->cnt-- > 1 ? (int) (*(outfile)->ptr++ = (ch)) : __putc(ch, outfile));
ch = *str++;
}
fprintf(outfile,") t\nT\n");
ai_path_count = 0;
}
int AI_text_angle(ang)
int ang;
{
ai_ang=ang;
return 1;
}
int AI_justify_text(mode)
enum JUSTIFY mode;
{
ai_justify=mode;
return 1;
}
static int null_text_angle()
{
return 0 ;
}
static int null_justify_text()
{
return 0 ;
}
static int null_scale()
{
return 0 ;
}
static int do_scale()
{
return 1 ;
}
options_null()
{
term_options[0] = '\0';
}
static UNKNOWN_null()
{
}
struct TERMENTRY term_tbl[] = {
{"unknown", "Unknown terminal type - not a plotting device",
100, 100, 1, 1,
1, 1, options_null, UNKNOWN_null, UNKNOWN_null,
UNKNOWN_null, null_scale, UNKNOWN_null, UNKNOWN_null, UNKNOWN_null,
UNKNOWN_null, UNKNOWN_null, null_text_angle,
null_justify_text, UNKNOWN_null, UNKNOWN_null}
,{"table", "Dump ASCII table of X Y [Z] values to output",
100, 100, 1, 1,
1, 1, options_null, UNKNOWN_null, UNKNOWN_null,
UNKNOWN_null, null_scale, UNKNOWN_null, UNKNOWN_null, UNKNOWN_null,
UNKNOWN_null, UNKNOWN_null, null_text_angle,
null_justify_text, UNKNOWN_null, UNKNOWN_null}
,{"aifm", "Adobe Illustrator 3.0 Format",
5000, 3500, (14*(10.0)), (14*(10.0)*6/10),
(3500/80), (3500/80), AI_options, AI_init, AI_reset,
AI_text, null_scale, AI_graphics, AI_move, AI_vector,
AI_linetype, AI_put_text, AI_text_angle,
AI_justify_text, do_point, do_arrow}
,{"mif", "Frame maker MIF 3.00 format",
15000, 10000, (10000/31), (15000/95),
(10000/150), (15000/225), MIF_options, MIF_init, MIF_reset,
MIF_text, null_scale, MIF_graphics, MIF_move, MIF_vector,
MIF_linetype, MIF_put_text, MIF_text_angle,
MIF_justify_text, line_and_point, do_arrow}
,{"postscript", "PostScript graphics language [mode \042fontname\042 font_size]",
7200, 5040, (14*(10)), (14*(10)*6/10),
(5040/80), (5040/80), PS_options, PS_init, PS_reset,
PS_text, null_scale, PS_graphics, PS_move, PS_vector,
PS_linetype, PS_put_text, PS_text_angle,
PS_justify_text, PS_point, do_arrow}
,{"Mac", "Macintosh Graphic Window",
448, 271, 11, 6,
(271/80), (448/80), options_null,MAC_init, MAC_reset,
MAC_text, null_scale, MAC_graphics, MAC_move, MAC_vector,
MAC_linetype, MAC_put_text, null_text_angle,
null_justify_text, line_and_point, do_arrow}
};
list_terms()
{
register int i;
fprintf((&__file[2]),"\nAvailable terminal types:\n");
for (i = 0; i < (sizeof(term_tbl)/sizeof(struct TERMENTRY)); i++)
fprintf((&__file[2]),"  %15s  %s\n",
term_tbl[i].name, term_tbl[i].description);
(void) (((&__file[2]))->cnt-- > 1 ? (int) (*((&__file[2]))->ptr++ = ('\n')) : __putc('\n', (&__file[2])));
}
int
set_term(c_token)
int c_token;
{
register int t;
char *input_name;
if (!token[c_token].is_token)
int_error("terminal name expected",c_token);
t = -1;
input_name = input_line + token[c_token].start_index;
t = change_term(input_name, token[c_token].length);
if (t == -1)
int_error("unknown terminal type; type just 'set terminal' for a list",
c_token);
if (t == -2)
int_error("ambiguous terminal name; type just 'set terminal' for a list",
c_token);
return(t);
}
int
change_term(name, length)
char *name;
int length;
{
int i, t = -1;
for (i = 0; i < (sizeof(term_tbl)/sizeof(struct TERMENTRY)); i++) {
if (!strncmp(name,term_tbl[i].name,length)) {
if (t != -1)
return(-2);
t = i;
}
}
if (t == -1)
return(t);
term = t;
term_init = 0;
name = term_tbl[term].name;
if (!strncmp("unixplot",name,8)) {
UP_redirect (2);
} else if (unixplot) {
UP_redirect (3);
}
if (interactive)
fprintf((&__file[2]), "Terminal type set to '%s'\n", name);
return(t);
}
init_terminal()
{
int t;
char *term_name = ((void *) 0);
char *gnuterm = ((void *) 0);
gnuterm = getenv("GNUTERM");
if (gnuterm != (char *)((void *) 0)) {
term_name = gnuterm;
}
else {
}
if (term_name != ((void *) 0) && *term_name != '\0') {
t = change_term(term_name, (int)strlen(term_name));
if (t == -1)
fprintf((&__file[2]), "Unknown terminal name '%s'\n", term_name);
else if (t == -2)
fprintf((&__file[2]), "Ambiguous terminal name '%s'\n", term_name);
else
;
}
}
UP_redirect(caller) int caller;
{
caller = caller;
}
test_term()
{
register struct TERMENTRY *t = &term_tbl[term];
char *str;
int x,y, xl,yl, i;
unsigned int xmax, ymax;
char label[50];
int scaling;
if (!term_init) {
(*t->init)();
term_init = 1;
}
screen_ok = 0;
scaling = (*t->scale)(xsize, ysize);
xmax = (unsigned int)(t->xmax * (scaling ? 1 : xsize));
ymax = (unsigned int)(t->ymax * (scaling ? 1 : ysize));
(*t->graphics)();
(*t->linetype)(-2);
(*t->move)(0,0);
(*t->vector)(xmax-1,0);
(*t->vector)(xmax-1,ymax-1);
(*t->vector)(0,ymax-1);
(*t->vector)(0,0);
(void) (*t->justify_text)(LEFT);
(*t->put_text)(t->h_char*5,ymax-t->v_char*3,"Terminal Test");
(*t->linetype)(-1);
(*t->move)(xmax/2,0);
(*t->vector)(xmax/2,ymax-1);
(*t->move)(0,ymax/2);
(*t->vector)(xmax-1,ymax/2);
(*t->linetype)(-2);
(*t->move)( xmax/2-t->h_char*10,ymax/2+t->v_char/2);
(*t->vector)(xmax/2+t->h_char*10,ymax/2+t->v_char/2);
(*t->vector)(xmax/2+t->h_char*10,ymax/2-t->v_char/2);
(*t->vector)(xmax/2-t->h_char*10,ymax/2-t->v_char/2);
(*t->vector)(xmax/2-t->h_char*10,ymax/2+t->v_char/2);
(*t->put_text)(xmax/2-t->h_char*10,ymax/2,
"12345678901234567890");
(void) (*t->justify_text)(LEFT);
(*t->put_text)(xmax/2,ymax/2+t->v_char*6,"left justified");
str = "centre+d text";
if ((*t->justify_text)(CENTRE))
(*t->put_text)(xmax/2,
ymax/2+t->v_char*5,str);
else
(*t->put_text)(xmax/2-strlen(str)*t->h_char/2,
ymax/2+t->v_char*5,str);
str = "right justified";
if ((*t->justify_text)(RIGHT))
(*t->put_text)(xmax/2,
ymax/2+t->v_char*4,str);
else
(*t->put_text)(xmax/2-strlen(str)*t->h_char,
ymax/2+t->v_char*4,str);
str = "rotated ce+ntred text";
if ((*t->text_angle)(1)) {
if ((*t->justify_text)(CENTRE))
(*t->put_text)(t->v_char,
ymax/2,str);
else
(*t->put_text)(t->v_char,
ymax/2-strlen(str)*t->h_char/2,str);
}
else {
(void) (*t->justify_text)(LEFT);
(*t->put_text)(t->h_char*2,ymax/2-t->v_char*2,"Can't rotate text");
}
(void) (*t->justify_text)(LEFT);
(void) (*t->text_angle)(0);
(*t->move)(xmax/2+t->h_tic*2,0);
(*t->vector)(xmax/2+t->h_tic*2,t->v_tic);
(*t->move)(xmax/2,t->v_tic*2);
(*t->vector)(xmax/2+t->h_tic,t->v_tic*2);
(*t->put_text)(xmax/2+t->h_tic*2,t->v_tic*2+t->v_char/2,"test tics");
x = xmax - t->h_char*4 - t->h_tic*4;
y = ymax - t->v_char;
for ( i = -2; y > t->v_char; i++ ) {
(*t->linetype)(i);
(void) sprintf(label,"%d",i+1);
if ((*t->justify_text)(RIGHT))
(*t->put_text)(x,y,label);
else
(*t->put_text)(x-strlen(label)*t->h_char,y,label);
(*t->move)(x+t->h_char,y);
(*t->vector)(x+t->h_char*4,y);
if ( i >= -1 )
(*t->point)(x+t->h_char*4+t->h_tic*2,y,i);
y -= t->v_char;
}
(*t->linetype)(0);
x = xmax/4;
y = ymax/4;
xl = t->h_tic*5;
yl = t->v_tic*5;
(*t->arrow)(x,y,x+xl,y,1);
(*t->arrow)(x,y,x+xl/2,y+yl,1);
(*t->arrow)(x,y,x,y+yl,1);
(*t->arrow)(x,y,x-xl/2,y+yl,0);
(*t->arrow)(x,y,x-xl,y,1);
(*t->arrow)(x,y,x-xl,y-yl,1);
(*t->arrow)(x,y,x,y-yl,1);
(*t->arrow)(x,y,x+xl,y-yl,1);
(*t->text)();
}
