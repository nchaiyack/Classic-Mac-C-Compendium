
/* command_2.c */
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
