
/* graph3d_1.c */
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
