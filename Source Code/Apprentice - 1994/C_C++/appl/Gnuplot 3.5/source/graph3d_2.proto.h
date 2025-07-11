
/* graph3d_2.c */
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
