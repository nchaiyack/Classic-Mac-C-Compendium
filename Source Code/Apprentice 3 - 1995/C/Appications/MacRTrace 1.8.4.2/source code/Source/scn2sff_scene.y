%{
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
#include "scn2sff_extern.h"

/**********************************************************************
 *    SCENE - Grammar - Version 1.3.2                                 *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, November 1991          *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, August 1992            *
 **********************************************************************/

#include <stdio.h>


extern char    *yytext;

/*******************
#ifdef dos
extern char    *yytext;
#else
#ifdef mips
extern char    *yytext;
#else
#ifdef BUFSIZ
#if BUFSIZ > 4096
char            yytext[BUFSIZ];
#else
char            yytext[4096];
#endif
#else
char            yytext[4096];
#endif
#endif
#endif
 ********************/

#define CSG_LEVEL_MAX  (256)
#define LIST_LEVEL_MAX (1)

int             point_index, point_index_max;
xyz_ptr         point_list;
real            height, scale;
char            csg[CSG_LEVEL_MAX];
char            list[LIST_LEVEL_MAX];
boolean         local_surface, local_refraction;
%}

%union {
	int			int_type;
	real			real_type;
	xyz_struct		xyz_type;
	rgb_struct		rgb_type;
	char_ptr		char_ptr_type;
	char			char_type[CHAR_MAX];
};

%token <real_type>	NUM
%token <int_type>	NUMINT
%token <char_ptr_type>	TOKEN FILETOKEN

%token FROM AT UP ANGLE BACKGROUND AMBIENT REFRACTION
%token LIGHT POINTLIGHT DIRLIGHT SPOTLIGHT EXTLIGHT
%token SURFACE STRAUSS MONO MATTE PLASTIC METAL DIELECTRIC GLASS
%token OPENOBJECT TRUNCATEDOBJECT FILEOBJECT NORMALOBJECT DATAOBJECT
%token BEGINGROUP ENDGROUP NEXTGROUP
%token SPHERE BOX CUBE CONE CYLINDER
%token WEDGE TETRA PRISM PYRAMID DISC RING
%token PATCH POLYGON TRIANGLE QUADRANGLE TORUS HEMISPHERE TEXT3D CSG LIST
%token TRANSFORM NONE SCALE TRANSLATE ROTATE GENERAL
%token NULLTEXTURE LOCALTEXTURE CHECKERSTEXTURE BLOTCHTEXTURE
%token BUMPTEXTURE MARBLETEXTURE FBMTEXTURE FBMBUMPTEXTURE
%token WOODTEXTURE ROUNDTEXTURE BOZOTEXTURE RIPPLESTEXTURE
%token WAVESTEXTURE SPOTTEDTEXTURE DENTSTEXTURE AGATETEXTURE
%token WRINKLESTEXTURE GRANITETEXTURE GRADIENTTEXTURE
%token IMAGEMAPTEXTURE GLOSSTEXTURE BUMP3TEXTURE
%token SINFUNCTION COSFUNCTION TANFUNCTION ASINFUNCTION ACOSFUNCTION
%token ATANFUNCTION SQRTFUNCTION INTFUNCTION RTODFUNCTION DTORFUNCTION
%token EXPFUNCTION LOGFUNCTION ABSFUNCTION
%token MAXFUNCTION MINFUNCTION
%token DOT3FUNCTION ADD3FUNCTION DIFF3FUNCTION SCALE3FUNCTION
%token CROSS3FUNCTION NORM3FUNCTION
%token CSG_UNION CSG_SUBTRACTION CSG_INTERSECTION

%type <int_type>	simpleinteger integer num_int num_int_pos csg_op
%type <real_type>	simplereal real
%type <real_type>	num num_pos num_0_to_1 num_angle_90 num_angle_360
%type <real_type>	parenthexpr expr functionexpr
%type <xyz_type>	point vector
%type <rgb_type>	color color_extended
%type <char_type>	filename

%left	'+' '-'
%left	'*' '/' '|'
%left	UMINUS
%right	'^'

%%
scene:
	elementlist
	;

elementlist:
	element
	| element elementlist
	;

element:
	view
	| light
	| attribute
	| object
	| groupobject
	;

view:
	from
	| at
	| up
	| angle
	| ambient
	| background
	;

light:
	LIGHT pointlight
	| LIGHT dirlight
	| LIGHT spotlight
	| LIGHT extlight
	;

attribute:
	SURFACE surface
	| TRANSFORM transform
	| refraction
	| texture
	;

localattribute:
	SURFACE
		{
		  local_surface = TRUE;
		}
	surface
	| TRANSFORM localtransform
	| refraction
	| localtexture
	;

localattributelist:
	localattribute
	| localattribute localattributelist
	;

surface:
	surface1
	| surface2
	| matte
	| plastic
	| metal
	| dielectric
	| glass
	;

texturesurface:
	SURFACE texturesurface1
	| SURFACE texturesurface2
	| SURFACE texturematte
	| SURFACE textureplastic
	| SURFACE texturemetal
	;

transform:
	notransform
	| scale
	| translate
	| rotate
	| general3d
	| general4d
	;

localtransform:
	transform
	;

texturetransform:
	texturescale
	| texturetranslate
	| texturerotate
	| texturegeneral3d
	| texturegeneral4d
	;

texture:
	notexture
	| nulltexture
	| checkers
	| blotch
	| bump
	| marble
	| fbm
	| fbmbump
	| wood
	| round
	| bozo
	| ripples
	| waves
	| spotted
	| dents
	| agate
	| wrinkles
	| granite
	| gradient
	| imagemap
	| gloss
	| bump3
	;

localtexture:
	texture
	;

object:
	sphere
	| box
	| cube
	| cone
	| opencone
	| cylinder
	| opencylinder
	| truncatedcone
	| opentruncatedcone
	| wedge
	| tetra
	| prism
	| openprism
	| pyramid
	| openpyramid
	| opentruncatedpyramid
	| disc
	| ring
	| patch
	| polygon
	| triangle
	| quadrangle
	| trianglenormal
	| patchfile
	| polygonfile
	| trianglenormalfile
	| opentorus
	| torus
	| hemisphere
	| text3dfile
	| csg
	| list
	;

grouplist:
	groupobject
	| object
	| localattribute grouplist
	| groupobject grouplist
	| object grouplist
	;

groupdata:
	grouplist
	;

groupobject:
	BEGINGROUP
		{
		  save_level();
		}
	groupdata
		{
		  restore_level();
		}
	ENDGROUP
	;

from:
	FROM point
		{
		  if (from_defined)
		    yyerror("FROM POINT already defined");
		  STRUCT_ASSIGN(from, $2);
		  from_defined = TRUE;
		}
	;

at:
	AT point
		{
		  if (at_defined)
		    yyerror("AT POINT already defined");
		  STRUCT_ASSIGN(at, $2);
		  at_defined = TRUE;
		}
	;

up:
	UP vector
		{
		  if (up_defined)
		    yyerror("UP VECTOR already defined");
		  STRUCT_ASSIGN(up, $2);
		  up_defined = TRUE;
		}
	;

angle:
	ANGLE num_angle_90
		{
		  if (angle_defined)
		    yyerror("VIEW ANGLES already defined");
		  angle_h = $2;
		  angle_v = $2;
		  angle_defined = TRUE;
		}
	| ANGLE num_angle_90 num_angle_90
		{
		  if (angle_defined)
		    yyerror("VIEW ANGLES already defined");
		  angle_h = $2;
		  angle_v = $3;
		  angle_defined = TRUE;
		}
	;

ambient:
	AMBIENT color
		{
		  if (ambient_defined)
		    yyerror("AMBIENT COLOR already defined");
		  STRUCT_ASSIGN(ambient, $2);
		  ambient_defined = TRUE;
		}
	;

background:
	BACKGROUND color
		{
		  if (background_defined)
		    yyerror("BACKGROUND COLOR already defined");
		  STRUCT_ASSIGN(background, $2);
		  background_defined = TRUE;
		}
	;

refraction:
	REFRACTION num_pos
		{
		  if ($2 < 1.0)
		    yyerror("REFRACTION index smaller than 1");
		  current_refraction = $2;
		  local_refraction = TRUE;
		}
	;

pointlight:
	POINTLIGHT point
		{
		  rgb_struct      t;

		  t.r = 1.0;
		  t.g = 1.0;
		  t.b = 1.0;
		  create_point_light(&$2, &t);
		}
	| POINTLIGHT point color_extended
		{
		  create_point_light(&$2, &$3);
		}
	;

dirlight:
	DIRLIGHT vector
		{
		  rgb_struct      t;

		  t.r = 1.0;
		  t.g = 1.0;
		  t.b = 1.0;
		  create_dir_light(&$2, &t);
		}
	| DIRLIGHT vector color
		{
		  create_dir_light(&$2, &$3);
		}
	;

spotlight:
	SPOTLIGHT point vector color_extended
		{
		  create_spot_light(&$2, &$4, &$3, 45.0, 1.0);
		}
	| SPOTLIGHT point vector color_extended num_angle_90
		{
		  create_spot_light(&$2, &$4, &$3, $5, 1.0);
		}
	| SPOTLIGHT point vector color_extended num_angle_90 num_pos
		{
		  create_spot_light(&$2, &$4, &$3, $5, $6);
		}
	;

extlight:
	EXTLIGHT point color_extended num_pos num_int_pos
		{
		  create_ext_light(&$2, &$3, $4, $5);
		}
	;

surface1:
	color
		{
		  rgb_struct      td, t;

		  td.r = 0.9;
		  td.g = 0.9;
		  td.b = 0.9;
		  t.r = 0.1;
		  t.g = 0.1;
		  t.b = 0.1;
		  create_surface1(&$1, &td, &t, 3.0, 0.0, &t);
		}
	| color color color num_pos num_0_to_1
		{
		  rgb_struct      t;

		  t.r = 0.0;
		  t.g = 0.0;
		  t.b = 0.0;
		  create_surface1(&$1, &$2, &$3, $4, $5, &t);
		}
	| color color color num_pos num_0_to_1 color
		{
		  create_surface1(&$1, &$2, &$3, $4, $5, &$6);
		}
	;

surface2:
	STRAUSS color color color
		{
		  rgb_struct      t;

		  t.r = 0.0;
		  t.g = 0.0;
		  t.b = 0.0;
		  create_surface2(&$2, &$3, &$4, &t);
		}
	| STRAUSS color color color color
		{
		  create_surface2(&$2, &$3, &$4, &$5);
		}
	;

matte:
	MATTE color
		{
		  rgb_struct      td, t;

		  td.r = 0.999;
		  td.g = 0.999;
		  td.b = 0.999;
		  t.r = 0.0;
		  t.g = 0.0;
		  t.b = 0.0;
		  create_surface1(&$2, &td, &t, 0.0, 0.0, &t);
		}
	;

plastic:
	PLASTIC color color num_0_to_1
		{
		  rgb_struct      td, ts, t;
		  real            phong;

		  ts.r = 0.1 * $3.r;
		  ts.g = 0.1 * $3.g;
		  ts.b = 0.1 * $3.b;
		  td.r = 1.0 - ts.r;
		  td.g = 1.0 - ts.g;
		  td.b = 1.0 - ts.b;
		  t.r = 0.0;
		  t.g = 0.0;
		  t.b = 0.0;
		  phong = 10.0 + 90.0 * $4;
		  create_surface1(&$2, &td, &ts, phong, 0.0, &t);
		}
	;

metal:
	METAL color color num_0_to_1
		{
		  rgb_struct      td, ts, t;
		  real            phong;

		  ts.r = 0.9 + 0.1 * $3.r;
		  ts.g = 0.9 + 0.1 * $3.g;
		  ts.b = 0.9 + 0.1 * $3.b;
		  td.r = 1.0 - ts.r;
		  td.g = 1.0 - ts.g;
		  td.b = 1.0 - ts.b;
		  t.r = 0.0;
		  t.g = 0.0;
		  t.b = 0.0;
		  phong = 10.0 + 90.0 * $4;
		  create_surface1(&$2, &td, &ts, phong, 1.0, &t);
		}
	;

dielectric:
	DIELECTRIC color color num_pos
		{
		  rgb_struct      td, ts;

		  if ($4 < 1.0)
		    yyerror("REFRACTION index smaller than 1");
		  ts.r = 1.0 - $3.r;
		  ts.g = 1.0 - $3.g;
		  ts.b = 1.0 - $3.b;
		  td.r = 0.0;
		  td.g = 0.0;
		  td.b = 0.0;
		  create_surface1(&$2, &td, &ts, 100.0, 0.0, &$3);
		  current_refraction = $4;
		  local_refraction = TRUE;
		}
	;

glass:
	GLASS color color
		{
		  rgb_struct      td, ts;

		  ts.r = 1.0 - $3.r;
		  ts.g = 1.0 - $3.g;
		  ts.b = 1.0 - $3.b;
		  td.r = 0.0;
		  td.g = 0.0;
		  td.b = 0.0;
		  create_surface1(&$2, &td, &ts, 100.0, 0.0, &$3);
		  current_refraction = 1.52;
		  local_refraction = TRUE;
		}
	;

texturesurface1:
	color
		{
		  rgb_struct      td, t;

		  td.r = 0.9;
		  td.g = 0.9;
		  td.b = 0.9;
		  t.r = 0.1;
		  t.g = 0.1;
		  t.b = 0.1;
		  create_text_surface1(&$1, &td, &t, 3.0, 0.0, &t);
		}
	| color color color num_pos num_0_to_1
		{
		  rgb_struct      t;

		  t.r = 0.0;
		  t.g = 0.0;
		  t.b = 0.0;
		  create_text_surface1(&$1, &$2, &$3, $4, $5, &t);
		}
	| color color color num_pos num_0_to_1 color
		{
		  create_text_surface1(&$1, &$2, &$3, $4, $5, &$6);
		}
	;

texturesurface2:
	STRAUSS color color color
		{
		  rgb_struct      t;

		  t.r = 0.0;
		  t.g = 0.0;
		  t.b = 0.0;
		  create_text_surface2(&$2, &$3, &$4, &t);
		}
	| STRAUSS color color color color
		{
		  create_text_surface2(&$2, &$3, &$4, &$5);
		}
	;

texturematte:
	MATTE color
		{
		  rgb_struct      td, t;

		  td.r = 0.999;
		  td.g = 0.999;
		  td.b = 0.999;
		  t.r = 0.0;
		  t.g = 0.0;
		  t.b = 0.0;
		  create_text_surface1(&$2, &td, &t, 0.0, 0.0, &t);
		}
	;

textureplastic:
	PLASTIC color color num_0_to_1
		{
		  rgb_struct      td, ts, t;
		  real            phong;

		  ts.r = 0.1 * $3.r;
		  ts.g = 0.1 * $3.g;
		  ts.b = 0.1 * $3.b;
		  td.r = 1.0 - ts.r;
		  td.g = 1.0 - ts.g;
		  td.b = 1.0 - ts.b;
		  t.r = 0.0;
		  t.g = 0.0;
		  t.b = 0.0;
		  phong = 10.0 + 90.0 * $4;
		  create_text_surface1(&$2, &td, &ts, phong, 0.0, &t);
		}
	;

texturemetal:
	METAL color color num_0_to_1
		{
		  rgb_struct      td, ts, t;
		  real            phong;

		  ts.r = 0.9 + 0.1 * $3.r;
		  ts.g = 0.9 + 0.1 * $3.g;
		  ts.b = 0.9 + 0.1 * $3.b;
		  td.r = 1.0 - ts.r;
		  td.g = 1.0 - ts.g;
		  td.b = 1.0 - ts.b;
		  t.r = 0.0;
		  t.g = 0.0;
		  t.b = 0.0;
		  phong = 10.0 + 90.0 * $4;
		  create_text_surface1(&$2, &td, &ts, phong, 1.0, &t);
		}
	;

notransform:
	NONE
		{
		  if (transforms > 0)
		    remove_transf_level();
		}
	;

scale:
	SCALE num
		{
		  if (ABS($2) < ROUNDOFF)
		    yyerror("null SCALE");
		  create_transf_scale($2, $2, $2);
		}
	| SCALE num num num
		{
		  if (ABS($2) < ROUNDOFF
		      AND ABS($3) < ROUNDOFF
		      AND ABS($4) < ROUNDOFF)
		    yyerror("null SCALE");
		  create_transf_scale($2, $3, $4);
		}
	;

translate:
	TRANSLATE point
		{
		  if (ABS($2.x) > ROUNDOFF
		      OR ABS($2.y) > ROUNDOFF
		      OR ABS($2.z) > ROUNDOFF)
		  create_transf_translate(&$2);
		}
	;

rotate:
	ROTATE 'x' num
		{
		  if (ABS($3) > ROUNDOFF)
		    create_transf_rotatex($3);
		}
	| ROTATE 'y'  num
		{
		  if (ABS($3) > ROUNDOFF)
		    create_transf_rotatey($3);
		}
	| ROTATE 'z'  num
		{
		  if (ABS($3) > ROUNDOFF)
		    create_transf_rotatez($3);
		}
	| ROTATE vector num
		{
		  if (ABS($3) > ROUNDOFF)
		    create_transf_rotate(&$2, $3);
		}
	;

general3d:
	GENERAL point point point
		{
		  create_transf_general(&$2, &$3, &$4);
		}
	;

general4d:
	GENERAL point point point point
		{
		  create_transf_general_4(&$2, &$3, &$4, &$5);
		}
	;

texturescale:
	SCALE num
		{
		  if (ABS($2) < ROUNDOFF)
		    yyerror("null TEXTURE SCALE");
		  create_text_transf_scale($2, $2, $2);
		}
	| SCALE num num num
		{
		  if (ABS($2) < ROUNDOFF
		      AND ABS($3) < ROUNDOFF
		      AND ABS($4) < ROUNDOFF)
		    yyerror("null TEXTURE SCALE");
		  create_text_transf_scale($2, $3, $4);
		}
	;

texturetranslate:
	TRANSLATE point
		{
		  if (ABS($2.x) > ROUNDOFF
		      OR ABS($2.y) > ROUNDOFF
		      OR ABS($2.z) > ROUNDOFF)
		  create_text_transf_translate(&$2);
		}
	;

texturerotate:
	ROTATE 'x' num
		{
		  if (ABS($3) > ROUNDOFF)
		    create_text_transf_rotatex($3);
		}
	| ROTATE 'y' num
		{
		  if (ABS($3) > ROUNDOFF)
		    create_text_transf_rotatey($3);
		}
	| ROTATE 'z' num
		{
		  if (ABS($3) > ROUNDOFF)
		    create_text_transf_rotatez($3);
		}
	| ROTATE vector num
		{
		  if (ABS($3) > ROUNDOFF)
		    create_text_transf_rotate(&$2, $3);
		}
	;

texturegeneral3d:
	GENERAL point point point
		{
		  create_text_transf_general(&$2, &$3, &$4);
		}
	;

texturegeneral4d:
	GENERAL point point point point
		{
		  create_text_transf_general_4(&$2, &$3, &$4, &$5);
		}
	;

notexture:
	NULLTEXTURE NONE
		{
		  if (textures > 0)
		    remove_text_level();
		}
	;

nulltexture:
	NULLTEXTURE texturetransform
		{
		  create_text_null();
		}
	| NULLTEXTURE LOCALTEXTURE
		{
		  create_text_transf_local();
		}
	;

checkers:
	CHECKERSTEXTURE texturesurface
		{
		  create_text_checkers();
		}
	| CHECKERSTEXTURE texturesurface texturetransform
		{
		  create_text_checkers();
		}
	;

blotch:
	BLOTCHTEXTURE num_0_to_1 texturesurface
		{
		  create_text_blotch($2, NULL);
		}
	| BLOTCHTEXTURE num_0_to_1 texturesurface filename
		{
		  create_text_blotch($2, $4);
		}
	| BLOTCHTEXTURE num_0_to_1 texturesurface texturetransform
		{
		  create_text_blotch($2, NULL);
		}
	| BLOTCHTEXTURE num_0_to_1 texturesurface filename texturetransform
		{
		  create_text_blotch($2, $4);
		}
	;

bump:
	BUMPTEXTURE num_0_to_1
		{
		  create_text_bump($2);
		}
	| BUMPTEXTURE num_0_to_1 texturetransform
		{
		  create_text_bump($2);
		}
	;

marble:
	MARBLETEXTURE
		{
		  create_text_marble(NULL);
		}
	| MARBLETEXTURE filename
		{
		  create_text_marble($2);
		}
	| MARBLETEXTURE texturetransform
		{
		  create_text_marble(NULL);
		}
	| MARBLETEXTURE filename texturetransform
		{
		  create_text_marble($2);
		}
	;

fbm:
	FBMTEXTURE num_0_to_1 num_0_to_1 num_0_to_1 num_pos num_0_to_1
		   num_int_pos
		{
		  create_text_fbm($2, $3, $4, $5, $6, $7, NULL);
		}
	| FBMTEXTURE num_0_to_1 num_0_to_1 num_0_to_1 num_pos num_0_to_1
		     num_int_pos filename
		{
		  create_text_fbm($2, $3, $4, $5, $6, $7, $8);
		}
	| FBMTEXTURE num_0_to_1 num_0_to_1 num_0_to_1 num_pos num_0_to_1
		     num_int_pos texturetransform
		{
		  create_text_fbm($2, $3, $4, $5, $6, $7, NULL);
		}
	| FBMTEXTURE num_0_to_1 num_0_to_1 num_0_to_1 num_pos num_0_to_1
		     num_int_pos filename texturetransform
		{
		  create_text_fbm($2, $3, $4, $5, $6, $7, $8);
		}
	;

fbmbump:
	FBMBUMPTEXTURE num_0_to_1 num_0_to_1 num_pos num_int_pos
		{
		  create_text_fbmbump($2, $3, $3, $4, $5);
		}
	| FBMBUMPTEXTURE num_0_to_1 num_0_to_1 num_pos num_int_pos
	                 texturetransform
		{
		  create_text_fbmbump($2, $3, $3, $4, $5);
		}
	;

wood:
	WOODTEXTURE
		{
		  create_text_wood(NULL);
		}
	| WOODTEXTURE color
		{
		  create_text_wood(&$2);
		}
	| WOODTEXTURE texturetransform
		{
		  create_text_wood(NULL);
		}
	| WOODTEXTURE color texturetransform
		{
		  create_text_wood(&$2);
		}
	;

round:
	ROUNDTEXTURE num_0_to_1
		{
		  create_text_round($2);
		}
	| ROUNDTEXTURE num_0_to_1 texturetransform
		{
		  create_text_round($2);
		}
	;

bozo:
	BOZOTEXTURE num_0_to_1
		{
		  create_text_bozo($2, NULL);
		}
	| BOZOTEXTURE num_0_to_1 filename
		{
		  create_text_bozo($2, $3);
		}
	| BOZOTEXTURE num_0_to_1 texturetransform
		{
		  create_text_bozo($2, NULL);
		}
	| BOZOTEXTURE num_0_to_1 filename texturetransform
		{
		  create_text_bozo($2, $3);
		}
	;

ripples:
	RIPPLESTEXTURE num_pos num num_0_to_1
		{
		  create_text_ripples($2, $3, $4);
		}
	| RIPPLESTEXTURE num_pos num num_0_to_1 texturetransform
		{
		  create_text_ripples($2, $3, $4);
		}
	;

waves:
	WAVESTEXTURE num_pos num num_0_to_1
		{
		  create_text_waves($2, $3, $4);
		}
	| WAVESTEXTURE num_pos num num_0_to_1 texturetransform
		{
		  create_text_waves($2, $3, $4);
		}
	;

spotted:
	SPOTTEDTEXTURE
		{
		  create_text_spotted(NULL);
		}
	| SPOTTEDTEXTURE filename
		{
		  create_text_spotted($2);
		}
	| SPOTTEDTEXTURE texturetransform
		{
		  create_text_spotted(NULL);
		}
	| SPOTTEDTEXTURE filename texturetransform
		{
		  create_text_spotted($2);
		}
	;

dents:
	DENTSTEXTURE num_0_to_1
		{
		  create_text_dents($2);
		}
	| DENTSTEXTURE num_0_to_1 texturetransform
		{
		  create_text_dents($2);
		}
	;

agate:
	AGATETEXTURE
		{
		  create_text_agate(NULL);
		}
	| AGATETEXTURE filename
		{
		  create_text_agate($2);
		}
	| AGATETEXTURE texturetransform
		{
		  create_text_agate(NULL);
		}
	| AGATETEXTURE filename texturetransform
		{
		  create_text_agate($2);
		}
	;

wrinkles:
	WRINKLESTEXTURE num_0_to_1
		{
		  create_text_wrinkles($2);
		}
	| WRINKLESTEXTURE num_0_to_1 texturetransform
		{
		  create_text_wrinkles($2);
		}
	;

granite:
	GRANITETEXTURE
		{
		  create_text_granite(NULL);
		}
	| GRANITETEXTURE filename
		{
		  create_text_granite($2);
		}
	| GRANITETEXTURE texturetransform
		{
		  create_text_granite(NULL);
		}
	| GRANITETEXTURE filename texturetransform
		{
		  create_text_granite($2);
		}
	;

gradient:
	GRADIENTTEXTURE num_0_to_1 vector
		{
		  create_text_gradient($2, &$3, NULL);
		}
	| GRADIENTTEXTURE num_0_to_1 vector filename
		{
		  create_text_gradient($2, &$3, $4);
		}
	| GRADIENTTEXTURE num_0_to_1 vector texturetransform
		{
		  create_text_gradient($2, &$3, NULL);
		}
	| GRADIENTTEXTURE num_0_to_1 vector filename texturetransform
		{
		  create_text_gradient($2, &$3, $4);
		}
	;

imagemap:
	IMAGEMAPTEXTURE num_0_to_1 num_int_pos num_int_pos num_int_pos
			filename
		{
		  if ($4 < 1 OR $4 > 3)
		    yyerror("invalid U AXIS code for texture");
		  if ($5 < 1 OR $5 > 3)
		    yyerror("invalid V AXIS code for texture");
		  create_text_imagemap($2, $3, $4, $5, $6);
		}
	| IMAGEMAPTEXTURE num_0_to_1 num_int_pos num_int_pos num_int_pos
			  filename texturetransform
		{
		  if ($4 < 1 OR $4 > 3)
		    yyerror("invalid U AXIS code for texture");
		  if ($5 < 1 OR $5 > 3)
		    yyerror("invalid V AXIS code for texture");
		  create_text_imagemap($2, $3, $4, $5, $6);
		}
	;

gloss:
	GLOSSTEXTURE num_0_to_1
		{
		  create_text_gloss($2);
		}
	| GLOSSTEXTURE num_0_to_1 texturetransform
		{
		  create_text_gloss($2);
		}
	;

bump3:
	BUMP3TEXTURE num_0_to_1
		{
		  create_text_bump3($2, 0.001);
		}
	| BUMP3TEXTURE num_0_to_1 num_0_to_1
		{
		  create_text_bump3($2, $3);
		}
	| BUMP3TEXTURE num_0_to_1 texturetransform
		{
		  create_text_bump3($2, 0.001);
		}
	| BUMP3TEXTURE num_0_to_1 num_0_to_1 texturetransform
		{
		  create_text_bump3($2, $3);
		}
	;

sphere:
	SPHERE spheredata
	| SPHERE
		{
		  save_level();
		}
	localattributelist DATAOBJECT spheredata
		{
		  restore_level();
		}
	;

spheredata:
	point num_pos
		{
		  if ($2 < ROUNDOFF)
		    yyerror("null SPHERE RADIUS");
		  create_sphere(&$1, $2);
		}
	;

box:
	BOX boxdata
	| BOX
		{
		  save_level();
		}
	localattributelist DATAOBJECT boxdata
		{
		  restore_level();
		}
	;

boxdata:
	point num_pos
		{
		  if ($2 < ROUNDOFF)
		    yyerror("null BOX SIZE");
		  create_box(&$1, $2, $2, $2);
		}
	| point num_pos num_pos num_pos
		{
		  if ($2 < ROUNDOFF
		      OR $3 < ROUNDOFF
		      OR $4 < ROUNDOFF)
		    yyerror("null BOX SIZE");
		  create_box(&$1, $2, $3, $4);
		}
	;

cube:
	CUBE cubedata
	| CUBE
		{
		  save_level();
		}
	localattributelist DATAOBJECT cubedata
		{
		  restore_level();
		}
	;

cubedata:
	point num_pos
		{
		  if ($2 < ROUNDOFF)
		    yyerror("null CUBE SIZE");
		  create_box(&$1, $2, $2, $2);
		}
	;

cone:
	CONE conedata
	| CONE
		{
		  save_level();
		}
	localattributelist DATAOBJECT conedata
		{
		  restore_level();
		}
	;

conedata:
	point point num_pos
		{
		  if ($3 < ROUNDOFF)
		    yyerror("null CONE BASE RADIUS");
		  create_cone(&$1, &$2, $3);
		}
	;

opencone:
	CONE OPENOBJECT openconedata
	| CONE OPENOBJECT
		{
		  save_level();
		}
	localattributelist DATAOBJECT openconedata
		{
		  restore_level();
		}
	;

openconedata:
	point point num_pos
		{
		  if ($3 < ROUNDOFF)
		    yyerror("null CONE BASE RADIUS");
		  create_open_cone(&$1, &$2, $3);
		}
	;

cylinder:
	CYLINDER cylinderdata
	| CYLINDER
		{
		  save_level();
		}
	localattributelist DATAOBJECT cylinderdata
		{
		  restore_level();
		}
	;

cylinderdata:
	point point num_pos
		{
		  if ($3 < ROUNDOFF)
		    yyerror("null CYLINDER RADIUS");
		  create_cylinder(&$1, &$2, $3);
		}
	;

opencylinder:
	CYLINDER OPENOBJECT opencylinderdata
	| CYLINDER OPENOBJECT
		{
		  save_level();
		}
	localattributelist DATAOBJECT opencylinderdata
		{
		  restore_level();
		}
	;

opencylinderdata:
	point point num_pos
		{
		  if ($3 < ROUNDOFF)
		    yyerror("null CYLINDER RADIUS");
		  create_open_cylinder(&$1, &$2, $3);
		}
	;

truncatedcone:
	CONE TRUNCATEDOBJECT truncatedconedata
	| CONE TRUNCATEDOBJECT
		{
		  save_level();
		}
	localattributelist DATAOBJECT truncatedconedata
		{
		  restore_level();
		}
	;

truncatedconedata:
	point num_pos point num_pos
		{
		  if ($2 < ROUNDOFF OR $4 < ROUNDOFF)
		    yyerror("null TRUNCATED CONE RADIUS");
		  if ($2 < $4)
		    create_truncated_cone(&$1, $2, &$3, $4);
		  else
		    create_truncated_cone(&$3, $4, &$1, $2);
		}
	;

opentruncatedcone:
	CONE TRUNCATEDOBJECT OPENOBJECT opentruncatedconedata
	| CONE TRUNCATEDOBJECT OPENOBJECT
		{
		  save_level();
		}
	localattributelist DATAOBJECT opentruncatedconedata
		{
		  restore_level();
		}
	;

opentruncatedconedata:
	  point num_pos point num_pos
		{
		  if ($2 < ROUNDOFF OR $4 < ROUNDOFF)
		    yyerror("null TRUNCATED CONE RADIUS");
		  if ($2 < $4)
		    create_open_truncated_cone(&$1, $2, &$3, $4);
		  else
		    create_open_truncated_cone(&$3, $4, &$1, $2);
		}
	;

wedge:
	WEDGE wedgedata
	| WEDGE
		{
		  save_level();
		}
	localattributelist DATAOBJECT wedgedata
		{
		  restore_level();
		}
	;

wedgedata:
	point point point num_pos
		{
		  if ($4 < ROUNDOFF)
		    yyerror("null WEDGE SIZE");
		  create_wedge(&$1, &$2, &$3, $4);
		}
	;

tetra:
	TETRA tetradata
	| TETRA
		{
		  save_level();
		}
	localattributelist DATAOBJECT tetradata
		{
		  restore_level();
		}
	;

tetradata:
	point point point point
		{
		  create_tetra(&$1, &$2, &$3, &$4);
		}
	;

prism:
	PRISM prismdata
	| PRISM
		{
		  save_level();
		}
	localattributelist DATAOBJECT prismdata
		{
		  restore_level();
		}
	;

prismdata:
	num_pos num_int_pos
		{
		  if ($1 < ROUNDOFF)
		    yyerror("null PRISM HEIGHT");
		  height = $1;
		  if ($2 < 3)
		    yyerror("not enough VERTICES");
		  ALLOCATE(point_list, xyz_ptr, $2 * sizeof(xyz_struct));
		  point_index_max = $2;
		  point_index = 0;
		}
		pointlist
		{
		  create_prism(height, point_index, point_list);
		  FREE(point_list);
		}
	;

openprism:
	PRISM OPENOBJECT openprismdata
	| PRISM OPENOBJECT
		{
		  save_level();
		}
	localattributelist DATAOBJECT openprismdata
		{
		  restore_level();
		}
	;

openprismdata:
	num_pos num_int_pos
		{
		  if ($1 < ROUNDOFF)
		    yyerror("null PRISM HEIGHT");
		  height = $1;
		  if ($2 < 3)
		    yyerror("not enough VERTICES");
		  ALLOCATE(point_list, xyz_ptr, $2 * sizeof(xyz_struct));
		  point_index_max = $2;
		  point_index = 0;
		}
		pointlist
		{
		  create_open_prism(height, point_index, point_list);
		  FREE(point_list);
		}
	;

pyramid:
	PYRAMID pyramiddata
	| PYRAMID
		{
		  save_level();
		}
	localattributelist DATAOBJECT pyramiddata
		{
		  restore_level();
		}
	;

pyramiddata:
	num_pos num_int_pos
		{
		  if ($1 < ROUNDOFF)
		    yyerror("null PYRAMID HEIGHT");
		  height = $1;
		  if ($2 < 3)
		    yyerror("not enough VERTICES");
		  ALLOCATE(point_list, xyz_ptr, $2 * sizeof(xyz_struct));
		  point_index_max = $2;
		  point_index = 0;
		}
		pointlist
		{
		  create_pyramid(height, point_index, point_list);
		  FREE(point_list);
		}
	;

openpyramid:
	PYRAMID OPENOBJECT openpyramiddata
	| PYRAMID OPENOBJECT
		{
		  save_level();
		}
	localattributelist DATAOBJECT openpyramiddata
		{
		  restore_level();
		}
	;

openpyramiddata:
	num_pos num_int_pos
		{
		  if ($1 < ROUNDOFF)
		    yyerror("null PYRAMID HEIGHT");
		  height = $1;
		  if ($2 < 3)
		    yyerror("not enough VERTICES");
		  ALLOCATE(point_list, xyz_ptr, $2 * sizeof(xyz_struct));
		  point_index_max = $2;
		  point_index = 0;
		}
		pointlist
		{
		  create_open_pyramid(height, point_index, point_list);
		  FREE(point_list);
		}
	;

opentruncatedpyramid:
	PYRAMID TRUNCATEDOBJECT OPENOBJECT opentruncatedpyramid
	| PYRAMID TRUNCATEDOBJECT OPENOBJECT
		{
		  save_level();
		}
	localattributelist DATAOBJECT opentruncatedpyramiddata
		{
		  restore_level();
		}
	;

opentruncatedpyramiddata:
	num_pos num_0_to_1 num_int_pos
		{
		  if ($1 < ROUNDOFF)
		    yyerror("null TRUNCATED PYRAMID HEIGHT");
		  height = $1;
		  if ($2 < ROUNDOFF)
		    yyerror("null TRUNCATED PYRAMID SCALE");
		  scale = $2;
		  if ($3 < 3)
		    yyerror("not enough VERTICES");
		  ALLOCATE(point_list, xyz_ptr, $3 * sizeof(xyz_struct));
		  point_index_max = $3;
		  point_index = 0;
		}
		pointlist
		{
		  create_open_truncated_pyramid(height, scale,
						point_index, point_list);
		  FREE(point_list);
		}
	;

disc:
	DISC discdata
	| DISC
		{
		  save_level();
		}
	localattributelist DATAOBJECT discdata
		{
		  restore_level();
		}
	;

discdata:
	point vector num_pos
		{
		  if ($3 < ROUNDOFF)
		    yyerror("null DISC RADIUS");
		  create_disc(&$1, &$2, $3);
		}
	;

ring:
	RING ringdata
	| RING
		{
		  save_level();
		}
	localattributelist DATAOBJECT ringdata
		{
		  restore_level();
		}
	;

ringdata:
	point vector num_pos num_pos
		{
		  if ($3 < ROUNDOFF)
		    yyerror("null outer RING RADIUS");
		  if ($4 < ROUNDOFF)
		    yyerror("null inner RING RADIUS");
		  if ($3 > $4)
		    create_ring(&$1, &$2, $3, $4);
		  else
		    create_ring(&$1, &$2, $4, $3);
		}
	;

patch:
	PATCH patchdata
	| PATCH
		{
		  save_level();
		}
	localattributelist DATAOBJECT patchdata
		{
		  restore_level();
		}
	;

patchdata:
	point point point point point point point point point point point point
		{
		  create_patch(&$1, &$2, &$3, &$4, &$5, &$6,
                               &$7, &$8, &$9, &$10, &$11, &$12);
		}
	;

patchfile:
	PATCH FILEOBJECT patchfiledata
	| PATCH FILEOBJECT
		{
		  save_level();
		}
	localattributelist DATAOBJECT patchfiledata
		{
		  restore_level();
		}
	;

patchfiledata:
	filename
		{
		  xyz_struct      tt, ts;

		  tt.x = 0.0;
		  tt.y = 0.0;
		  tt.z = 0.0;
		  ts.x = 1.0;
		  ts.y = 1.0;
		  ts.z = 1.0;
		  create_patch_file(&tt, &ts, $1);
		}
	| point filename
		{
		  xyz_struct      ts;

		  ts.x = 1.0;
		  ts.y = 1.0;
		  ts.z = 1.0;
		  create_patch_file(&$1, &ts, $2);
		}
	| point point filename
		{
		  create_patch_file(&$1, &$2, $3);
		}
	;

polygon:
	POLYGON polygondata
	| POLYGON
		{
		  save_level();
		}
	localattributelist DATAOBJECT polygondata
		{
		  restore_level();
		}
	;

polygondata:
	num_int_pos
		{
		  if ($1 < 3)
		    yyerror("not enough VERTICES");
		  ALLOCATE(point_list, xyz_ptr, $1 * sizeof(xyz_struct));
		  point_index_max = $1;
		  point_index = 0;
		}
		pointlist
		{
		  create_polygon(point_index, point_list);
		  FREE(point_list);
		}
	;

polygonfile:
	POLYGON FILEOBJECT polygonfiledata
	| POLYGON FILEOBJECT
		{
		  save_level();
		}
	localattributelist DATAOBJECT polygonfiledata
		{
		  restore_level();
		}
	;

polygonfiledata:
	filename
		{
		  xyz_struct      tt, ts;

		  tt.x = 0.0;
		  tt.y = 0.0;
		  tt.z = 0.0;
		  ts.x = 1.0;
		  ts.y = 1.0;
		  ts.z = 1.0;
		  create_polygon_file(&tt, &ts, $1);
		}
	| point filename
		{
		  xyz_struct      ts;

		  ts.x = 1.0;
		  ts.y = 1.0;
		  ts.z = 1.0;
		  create_polygon_file(&$1, &ts, $2);
		}
	| point point filename
		{
		  create_polygon_file(&$1, &$2, $3);
		}
	;

triangle:
	TRIANGLE triangledata
	| TRIANGLE
		{
		  save_level();
		}
	localattributelist DATAOBJECT triangledata
		{
		  restore_level();
		}
	;

triangledata:
	point point point
		{
		  create_triangle(&$1, &$2, &$3);
		}
	;

quadrangle:
	QUADRANGLE quadrangledata
	| QUADRANGLE
		{
		  save_level();
		}
	localattributelist DATAOBJECT quadrangledata
		{
		  restore_level();
		}
	;

quadrangledata:
	point point point point
		{
		  create_quadrangle(&$1, &$2, &$3, &$4);
		}
	;

trianglenormal:
	TRIANGLE NORMALOBJECT trianglenormaldata
	| TRIANGLE NORMALOBJECT
		{
		  save_level();
		}
	localattributelist DATAOBJECT trianglenormaldata
		{
		  restore_level();
		}
	;

trianglenormaldata:
	point vector point vector point vector
		{
		  create_triangle_normal(&$1, &$2, &$3, &$4, &$5, &$6);
		}
	;

trianglenormalfile:
	TRIANGLE NORMALOBJECT FILEOBJECT trianglenormalfiledata
	| TRIANGLE NORMALOBJECT FILEOBJECT
		{
		  save_level();
		}
	localattributelist DATAOBJECT trianglenormalfiledata
		{
		  restore_level();
		}
	;

trianglenormalfiledata:
	filename
		{
		  xyz_struct      tt, ts;

		  tt.x = 0.0;
		  tt.y = 0.0;
		  tt.z = 0.0;
		  ts.x = 1.0;
		  ts.y = 1.0;
		  ts.z = 1.0;
		  create_triangle_normal_file(&tt, &ts, $1);
		  restore_level();
		}
	| point filename
		{
		  xyz_struct      ts;

		  ts.x = 1.0;
		  ts.y = 1.0;
		  ts.z = 1.0;
		  create_triangle_normal_file(&$1, &ts, $2);
		}
	| point point filename
		{
		  create_triangle_normal_file(&$1, &$2, $3);
		}
	;

opentorus:
	TORUS OPENOBJECT opentorusdata
	| TORUS OPENOBJECT
		{
		  save_level();
		}
	localattributelist DATAOBJECT opentorusdata
		{
		  restore_level();
		}
	;

opentorusdata:
	num_pos num_pos num_angle_360 num_angle_360
		{
		  if (($1 <= 0.0) OR($2 <= 0.0))
		    yyerror("external/internal RADIUS smaller than zero");
		  if (ABS($4 - $3) < ROUNDOFF)
		    yyerror("no ANGLE");
		  if (ABS($4 - $3) > 360.0 + ROUNDOFF)
		    yyerror("ANGLE more than 360 DEGREES");
		  create_open_torus($1, $2, $3, $4, 16, 8);
		}
	| num_pos num_pos num_angle_360 num_angle_360
	  num_int_pos num_int_pos
		{
		  if (($1 <= 0.0) OR($2 <= 0.0))
		    yyerror("external/internal RADIUS smaller than zero");
		  if (ABS($4 - $3) < ROUNDOFF)
		    yyerror("no ANGLE");
		  if (ABS($4 - $3) > 360.0 + ROUNDOFF)
		    yyerror("ANGLE more than 360 DEGREES");
		  if ($5 <= 4)
		    yyerror("external SAMPLES too low");
		  if ($6 <= 4)
		    yyerror("internal SAMPLES too low");
		  create_open_torus($1, $2, $3, $4, $5, $6);
		}
	;

torus:
	TORUS torusdata
	| TORUS
		{
		  save_level();
		}
	localattributelist DATAOBJECT torusdata
		{
		  restore_level();
		}
	;

torusdata:
	num_pos num_pos
		{
		  if (($1 <= 0.0) OR($2 <= 0.0))
		    yyerror("external/internal RADIUS smaller than zero");
		  create_torus($1, $2, 0.0, 360.0, 16, 8);
		}
	| num_pos num_pos num_angle_360 num_angle_360
		{
		  if (($1 <= 0.0) OR($2 <= 0.0))
		    yyerror("external/internal RADIUS smaller than zero");
		  if (ABS($4 - $3) < ROUNDOFF)
		    yyerror("no ANGLE");
		  if (ABS($4 - $3) > 360.0 + ROUNDOFF)
		    yyerror("ANGLE more than 360 DEGREES");
		  create_torus($1, $2, $3, $4, 16, 8);
		}
	| num_pos num_pos num_angle_360 num_angle_360
	  num_int_pos num_int_pos
		{
		  if (($1 <= 0.0) OR($2 <= 0.0))
		    yyerror("external/internal RADIUS smaller than zero");
		  if (ABS($4 - $3) < ROUNDOFF)
		    yyerror("no ANGLE");
		  if (ABS($4 - $3) > 360.0 + ROUNDOFF)
		    yyerror("ANGLE more than 360 DEGREES");
		  if ($5 <= 4)
		    yyerror("external SAMPLES too low");
		  if ($6 <= 4)
		    yyerror("internal SAMPLES too low");
		  create_torus($1, $2, $3, $4, $5, $6);
		}
	;

text3dfile:
	TEXT3D FILEOBJECT text3dfiledata
	| TEXT3D FILEOBJECT
		{
		  save_level();
		}
	localattributelist DATAOBJECT text3dfiledata
		{
		  restore_level();
		}
	;

text3dfiledata:
	filename
		{
		  create_text3d_file($1);
		}
	;

csg:
	CSG csg_op BEGINGROUP
		{
		  csg_level++;
		  create_csg_begin(csg_level, $2, 0, 0.0);
		  if (csg_level == CSG_LEVEL_MAX)
		    yyerror("too many CSG LEVELS");
		  csg[csg_level] = 0;
		}
	| CSG csg_op
		{
		  save_level();
                  local_surface = FALSE;
                  local_refraction = FALSE;
		}
	localattributelist DATAOBJECT
		{
		  create_csg_begin(csg_level + 1, $2,
		    local_surface ? current_surface : 0,
		    local_refraction ? current_refraction : 0.0);
		  restore_level();
		}
	BEGINGROUP
		{
		  csg_level++;
		  if (csg_level == CSG_LEVEL_MAX)
		    yyerror("too many CSG LEVELS");
		  csg[csg_level] = 0;
		}
	| CSG NEXTGROUP
		{
		  if (csg_level < 0 OR csg[csg_level] != 0)
		    yyerror("invalid CSG NEXT");
		  csg[csg_level] = 1;
		  create_csg_next(csg_level);
		}
	| CSG ENDGROUP
		{
		  if (csg_level < 0 OR csg[csg_level] != 1)
		    yyerror("invalid CSG END");
		  create_csg_end(csg_level);
		  csg_level--;
		}
	;

csg_op:
	CSG_UNION
		{
		  $$ = 0;
		}
	| CSG_SUBTRACTION
		{
		  $$ = 1;
		}
	| CSG_INTERSECTION
		{
		  $$ = 2;
		}
	;

list:
	LIST BEGINGROUP
		{
		  list_level++;
		  create_list_begin(list_level, 0, 0.0);
		  if (list_level == LIST_LEVEL_MAX)
		    yyerror("too many LIST LEVELS");
		  list[list_level] = 1;
		}
	| LIST
		{
		  save_level();
                  local_surface = FALSE;
                  local_refraction = FALSE;
		}
	localattributelist DATAOBJECT
		{
		  create_list_begin(list_level + 1,
		    local_surface ? current_surface : 0,
		    local_refraction ? current_refraction : 0.0);
		  restore_level();
		}
	BEGINGROUP
		{
		  list_level++;
		  if (list_level == LIST_LEVEL_MAX)
		    yyerror("too many LIST LEVELS");
		  list[list_level] = 1;
		}
	| LIST ENDGROUP
		{
		  if (list_level < 0 OR list[list_level] != 1)
		    yyerror("invalid LIST END");
		  create_list_end(list_level);
		  list_level--;
		}
	;

hemisphere:
	HEMISPHERE
		{
		  yyerror("HEMISPHERE not yet implemented");
		}
	;

color:
	num_0_to_1 num_0_to_1 num_0_to_1
		{
	  	  $$.r = $1;
	  	  $$.g = $2;
	  	  $$.b = $3;
		}
	| MONO num_0_to_1
		{
	  	  $$.r = $2;
	  	  $$.g = $2;
	  	  $$.b = $2;
		}
	| TOKEN
		{
	  	  if (NOT find_color_by_name(yytext, &$$))
	  	  {
	    	    sprintf(buffer, "cannot find COLOR \"%s\"", yytext);
	    	    yyerror(buffer);
	  	  }
		}
	;

color_extended:
	num num num
		{
	  	  $$.r = $1;
	  	  $$.g = $2;
	  	  $$.b = $3;
		}
	| MONO num
		{
	  	  $$.r = $2;
	  	  $$.g = $2;
	  	  $$.b = $2;
		}
	| TOKEN
		{
	  	  if (NOT find_color_by_name(yytext, &$$))
	  	  {
	    	    sprintf(buffer, "cannot find COLOR \"%s\"", yytext);
	    	    yyerror(buffer);
	  	  }
		}
	;

vector:
	point
		{
		  if (ABS($$.x) < ROUNDOFF
		      AND ABS($$.y) < ROUNDOFF
		      AND ABS($$.z) < ROUNDOFF)
		    yyerror("null VECTOR");
		}
	| CROSS3FUNCTION '(' vector ',' vector ')'
		{
		  CROSS_PRODUCT($$, $3, $5);
		}
	| NORM3FUNCTION '(' vector ')'
		{
                  $$.x = $3.x;
                  $$.y = $3.y;
                  $$.z = $3.z;
		  NORMALIZE($$);
		}
	;

point:
	num num num
		{
		  $$.x = $1;
		  $$.y = $2;
		  $$.z = $3;
		}
	| ADD3FUNCTION '(' point ',' point ')'
		{
		  $$.x = $3.x + $5.x;
		  $$.y = $3.y + $5.y;
		  $$.z = $3.z + $5.z;
		}
	| DIFF3FUNCTION '(' point ',' point ')'
		{
		  $$.x = $3.x - $5.x;
		  $$.y = $3.y - $5.y;
		  $$.z = $3.z - $5.z;
		}
	| SCALE3FUNCTION '(' point ',' num ')'
		{
		  $$.x = $3.x * $5;
		  $$.y = $3.y * $5;
		  $$.z = $3.z * $5;
		}
	;

pointlist:
	pointlist point
		{
		  STRUCT_ASSIGN(point_list[point_index], $2);
		  point_index++;
		  if (point_index > point_index_max)
		    yyerror("too many POINTS");
		}
	|
	;

num_pos:
	num
		{
		  if ($$ < 0.0)
		    yyerror("negative REAL");
		}
	;

num_0_to_1:
	num
		{
		  if ($$ < 0.0 OR $$ > 1.0)
		    yyerror("REAL outside [0,1]");
		}
	;

num_angle_90:
	num
		{
		  if ($$ < 0.0 OR $$ > 90.0)
		    yyerror("REAL outside [0,90] (degrees)");
		}
	;

num_angle_360:
	num
		{
		  if ($$ < -360.0 OR $$ > 360.0)
		    yyerror("REAL outside [-360,360] (degrees)");
		}
	;

num:
	real
	| parenthexpr
	| functionexpr
	;
	
real:
	simplereal
	| '-' simplereal
		{
		  $$ = -$2;
		}
	| '+' simplereal
		{
		  $$ = $2;
		}
	| integer
		{
		  $$ = (real) $1;
		}
	;

simplereal:
	NUM
		{
		  double          num;

		  if (sscanf(yytext, "%lf", &num) != 1)
		  {
		    sprintf(buffer, "not a valid REAL \"%s\"", yytext);
		    yyerror(buffer);
		  }
		  $$ = num;
		}
	;

num_int_pos:
	num_int
		{
		  if ($$ < 0)
		    yyerror("negative INTEGER");
		}
	;

num_int:
	integer
	;

integer:
	simpleinteger
	| '-' simpleinteger
		{
		  $$ = -$2;
		}
	| '+' simpleinteger
		{
		  $$ = $2;
		}
	| INTFUNCTION parenthexpr
		{
		  if (ABS($2) > (real) MAXINT)
		    yyerror("cannot convert REAL to INTEGER");
		  $$ = (int) $2;
		}
	;

simpleinteger:
	NUMINT
		{
		  int             num;

		  if (sscanf(yytext, "%d", &num) != 1)
		  {
		    sprintf(buffer, "not a valid INTEGER \"%s\"",
                            yytext);
		    yyerror(buffer);
		  }
		  $$ = num;
		}
	;

parenthexpr:
	'(' expr ')'
		{
		  $$ = $2;
		}
	;

expr:
	num
	| expr '+' expr
		{
		  $$ = $1 + $3;
		}
	| expr '-' expr
		{
		  $$ = $1 - $3;
		}
	| expr '*' expr
		{
		  $$ = $1 * $3;
		}
	| expr '/' expr
		{
		  if (ABS($3) < ROUNDOFF)
		    yyerror("division by ZERO");
		  $$ = $1 / $3;
		}
	| expr '^' expr
		{
		  if (ABS($1) < ROUNDOFF)
		    yyerror("power base is ZERO");
		  if (ABS($3) < ROUNDOFF)
		    $$ = 1.0;
		  else if (ABS(1.0 - $3) < ROUNDOFF)
		    $$ = $1;
		  else if (ABS(-1.0 - $3) < ROUNDOFF)
		    $$ = 1.0 / $1;
		  else
		    $$ = POWER($1, $3);
		}
	| expr '|' expr
		{
		  if (ABS(ROUND($3)) < ROUNDOFF)
		    yyerror("remainder base is ZERO");
		  $$ = (real) (ROUND($1) % ROUND($3));
		}
	;

functionexpr:
	'-' parenthexpr
		{
		  $$ = -$2;
		}
	| '+' parenthexpr
		{
		  $$ = $2;
		}
	| SINFUNCTION parenthexpr
		{
		  $$ = (real) sin((double) ($2));
		}
	| COSFUNCTION parenthexpr
		{
		  $$ = (real) cos((double) ($2));
		}
	| TANFUNCTION parenthexpr
		{
		  $$ = (real) tan((double) ($2));
		}
	| ASINFUNCTION parenthexpr
		{
		  if (ABS($2) > 1.0 + ROUNDOFF)
		    yyerror("invalid ASIN ARGUMENT");
		  $$ = (real) asin((double) ($2));
		}
	| ACOSFUNCTION parenthexpr
		{
		  if (ABS($2) > 1.0 + ROUNDOFF)
		    yyerror("invalid ACOS ARGUMENT");
		  $$ = (real) acos((double) ($2));
		}
	| ATANFUNCTION parenthexpr
		{
		  $$ = (real) atan((double) ($2));
		}
	| SQRTFUNCTION parenthexpr
		{
		  if (ABS($2) < ROUNDOFF)
		    $$ = 0.0;
		  else if ($2 < 0.0)
		    yyerror("invalid SQRT ARGUMENT");
		  $$ = (real) sqrt((double) ($2));
		}
	| RTODFUNCTION parenthexpr
		{
		  $$ = $2 / PI * 180.0;
		}
	| DTORFUNCTION parenthexpr
		{
		  $$ = $2 / 180.0 * PI;
		}
	| EXPFUNCTION parenthexpr
		{
		  $$ = (real) exp((double) ($2));
		}
	| LOGFUNCTION parenthexpr
		{
		  if ($2 < ROUNDOFF)
		    yyerror("invalid LOG ARGUMENT");
		  $$ = (real) log((double) ($2));
		}
	| ABSFUNCTION parenthexpr
		{
		  $$ = ABS($2);
		}
	| MAXFUNCTION '(' expr ',' expr ')'
		{
		  $$ = $3 > $5 ? $3 : $5;
		}
	| MINFUNCTION '(' expr ',' expr ')'
		{
		  $$ = $3 < $5 ? $3 : $5;
		}
	| DOT3FUNCTION '(' vector ',' vector ')'
		{
		  $$ = DOT_PRODUCT($3, $5);
		}
	;

filename:
	FILETOKEN
		{
		  strncpy($$, yytext, CHAR_MAX);
		}
	;

%%
yyerror(message)
  char            message[];
{
  fprintf(stderr, "Error: at line %d, reading \"%s\"\n", yylinecount, yytext);
  fprintf(stderr, "Info: %s\n", message);
  exit(1);
}
#ifndef dos
yywrap()
{
  return 1;
}
#endif
