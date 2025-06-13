/*
 * Copyright (c) 1991, 1992 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Antonio Costa, at INESC-Norte. The name of the author and
 * INESC-Norte may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "extern.h"

/**********************************************************************
 *    SCENE - External declarations - Version 1.3.1                   *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, November 1991          *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, July 1992              *
 **********************************************************************/

extern FILE    *yyin;
extern int      yylinecount;
extern int      yyerror(char *message);
extern int      yyparse(void);

/***** Others *****/
extern boolean  from_defined;
extern boolean  at_defined;
extern boolean  up_defined;
extern boolean  angle_defined;
extern boolean  ambient_defined;
extern boolean  background_defined;

extern xyz_struct from, at, scn2sff_up;
extern real     angle_h, angle_v;
extern rgb_struct scn2sff_background, ambient;

extern int      scn2sff_lights;
extern int      scn2sff_surfaces;
extern int      scn2sff_objects;
extern int      transforms;
extern int      textures;
extern int      csg_level;
extern int      list_level;

extern int      current_surface;
extern int      current_text_surface;

extern real     current_refraction;

extern char     buffer[STRING_MAX];

extern char_ptr light_list[LIGHTS_MAX];
extern char_ptr surface_list[SURFACES_MAX];
extern char_ptr object_list[OBJECTS_MAX];
extern char_ptr texture_list[TEXTURES_MAX];
extern char_ptr transform_list[TRANSFORMS_MAX];

extern int      level_scene;
extern char     level_type[LEVELS_MAX];
extern int      level_textures[LEVELS_MAX];
extern int      level_transforms[LEVELS_MAX];
extern int      level_current_surface[LEVELS_MAX];
extern int      level_current_text_surface[LEVELS_MAX];
extern real     level_current_refraction[LEVELS_MAX];

/***** Functions *****/
boolean         find_color_by_name(char *name, rgb_ptr color);
void            create_point_light(xyz_ptr point, rgb_ptr color);
void            create_dir_light(xyz_ptr direct, rgb_ptr color);
void            create_spot_light(xyz_ptr point, rgb_ptr color, xyz_ptr direct,
									real angle, real factor);
void            create_ext_light(xyz_ptr point, rgb_ptr color, real radius, int samples);
void            create_sphere(xyz_ptr center, real radius);
void            create_box(xyz_ptr center, real sx, real sy, real sz);
void            create_open_cone(xyz_ptr apex, xyz_ptr base, real radius);
void            create_disc(xyz_ptr center, xyz_ptr normal, real radius);
void            create_ring(xyz_ptr center, xyz_ptr normal, real oradius, real iradius);
void            create_cone(xyz_ptr apex, xyz_ptr base, real radius);
void            create_open_cylinder(xyz_ptr apex, xyz_ptr base, real radius);
void            create_cylinder(xyz_ptr apex, xyz_ptr base, real radius);
void            create_open_truncated_cone(xyz_ptr apex, real aradius, xyz_ptr base, real bradius);
void            create_truncated_cone(xyz_ptr apex, real aradius, xyz_ptr base, real bradius);
void            create_patch(xyz_ptr p1, xyz_ptr p2, xyz_ptr p3, xyz_ptr p4,
							xyz_ptr p5, xyz_ptr p6, xyz_ptr p7, xyz_ptr p8,
							xyz_ptr p9, xyz_ptr p10, xyz_ptr p11, xyz_ptr p12);
void            create_patch_file(xyz_ptr vector, xyz_ptr scale, char_ptr name);
void            create_polygon(int points, xyz_ptr point_list);
void            create_polygon_file(xyz_ptr vector, xyz_ptr scale, char_ptr name);
void            create_triangle_normal(xyz_ptr p1, xyz_ptr n1, xyz_ptr p2,
										xyz_ptr n2, xyz_ptr p3, xyz_ptr n3);
void            create_triangle_normal_file(xyz_ptr vector, xyz_ptr scale, char_ptr name);
void            create_wedge(xyz_ptr p1, xyz_ptr p2, xyz_ptr p3, real width);
void            create_tetra(xyz_ptr p1, xyz_ptr p2, xyz_ptr p3, xyz_ptr p4);
void            create_triangle(xyz_ptr p1, xyz_ptr p2, xyz_ptr p3);
void            create_quadrangle(xyz_ptr p1, xyz_ptr p2, xyz_ptr p3, xyz_ptr p4);
void            create_open_truncated_pyramid(real height, real scale, int points, xyz_ptr point_list);
void            create_open_prism(real height, int points, xyz_ptr point_list);
void            create_prism(real height, int points, xyz_ptr point_list);
void            create_open_pyramid(real height, int points, xyz_ptr point_list);
void            create_pyramid(real height, int points, xyz_ptr point_list);
void            create_open_torus(real oradius, real iradius, real sangle, real eangle,
									int osamples, int isamples);
void            create_torus(real oradius, real iradius, real sangle, real eangle,
									int osamples, int isamples);
void            create_text3d_file(char_ptr name);
void            create_csg_begin(int level, int op, int surface, double refraction);
void            create_csg_next(int level);
void            create_csg_end(int level);
void            create_list_begin(int level, int surface, double refraction);
void            create_list_end(int level);
void            create_surface_default(void);
void            create_text_transf_scale(real sx, real sy, real sz);
void            create_text_transf_translate(xyz_ptr vector);
void            create_text_transf_rotatex(real angle);
void            create_text_transf_rotatey(real angle);
void            create_text_transf_rotatez(real angle);
void            create_text_transf_rotate(xyz_ptr axis, real angle);
void            create_text_transf_general(xyz_ptr p1, xyz_ptr p2, xyz_ptr p3);
void			create_text_transf_general_4(xyz_ptr p1, xyz_ptr p2, xyz_ptr p3, xyz_ptr p4);
void            create_text_transf_local(void);
void            create_text_transf_default(void);
void            define_text_transform(void);
void            remove_text_level(void);
void            create_text_null(void);
void            create_text_checkers(void);
void            create_text_blotch(real scale, char_ptr name);
void            create_text_bump(real scale);
void            create_text_marble(char_ptr name);
void            create_text_fbm(real offset, real scale, real omega, real lambda, real threshold,
								int octaves, char_ptr name);
void            create_text_fbmbump(real offset, real scale, real lambda, int octaves, real dummy);
void            create_text_wood(rgb_ptr color);
void            create_text_round(real scale);
void            create_text_bozo(real turbulence, char_ptr name);
void            create_text_ripples(real frequency, real phase, real scale);
void            create_text_waves(real frequency, real phase, real scale);
void            create_text_spotted(char_ptr name);
void            create_text_dents(real scale);
void            create_text_agate(char_ptr name);
void            create_text_wrinkles(real scale);
void            create_text_granite(char_ptr name);
void            create_text_gradient(real turbulence, xyz_ptr direction, char_ptr name);
void            create_text_imagemap(real turbulence, int mode, int u_axis, int v_axis,
										char_ptr name);
void            create_text_gloss(real scale);
void            create_text_bump3(real scale, real size);
void            remove_transf_level(void);
void            create_transf_scale(real sx, real sy, real sz);
void            create_transf_translate(xyz_ptr vector);
void            create_transf_rotatex(real angle);
void            create_transf_rotatey(real angle);
void            create_transf_rotatez(real angle);
void            create_transf_rotate(xyz_ptr axis, real angle);
void            create_transf_general(xyz_ptr p1, xyz_ptr p2, xyz_ptr p3);
void			create_transf_general_4(xyz_ptr p1, xyz_ptr p2, xyz_ptr p3, xyz_ptr p4);
void            save_level();
void            restore_level();
void			create_surface1(rgb_ptr color, rgb_ptr diffuse, rgb_ptr specular,
								real phong, real factor, rgb_ptr transp);
void			create_surface2(rgb_ptr color, rgb_ptr smooth, rgb_ptr metal, rgb_ptr transp);
void			create_text_surface1(rgb_ptr color, rgb_ptr diffuse, rgb_ptr specular,
								real phong, real factor, rgb_ptr transp);
void			create_text_surface2(rgb_ptr color, rgb_ptr smooth, rgb_ptr metal, rgb_ptr transp);
void			save_level(void);
void			restore_level(void);




