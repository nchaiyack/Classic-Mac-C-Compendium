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

/**********************************************************************
 *    SCENE - Lexical - Version 1.3.2                                 *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, November 1991          *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, August 1992            *
 **********************************************************************/

#include "scn2sff_extern.h"
#ifdef dos
#include "y_tab.h"
#else
#include "y.tab.h"
#endif
%}
%e 1500
%p 7000
%n 600
%a 35000
%o 42000

P					"."
L					[A-Za-z]
C					[A-Za-z0-9_]
D					[0-9]
E					[DEde][-+]?{D}+
U					_?
F					("."|"/"|"~")
G					("."|"/")

%%
[ \t]					;
\#.*$					;
\%.*$					;
\;.*$					;
\n					yylinecount++;

from					return FROM;
eye					return FROM;

at					return AT;
look					return AT;
to					return AT;

up					return UP;

angle					return ANGLE;
fov					return ANGLE;

back					return BACKGROUND;
background				return BACKGROUND;

ambient					return AMBIENT;

refraction				return REFRACTION;
ior					return REFRACTION;

light					return LIGHT;
point					return POINTLIGHT;
dir					return DIRLIGHT;
directional				return DIRLIGHT;
spot					return SPOTLIGHT;
ext					return EXTLIGHT;
extended				return EXTLIGHT;

surface					return SURFACE;
strauss					return STRAUSS;
mono					return MONO;
matte					return MATTE;
plastic					return PLASTIC;
metal					return METAL;
metallic				return METAL;
die					return DIELECTRIC;
dielectric				return DIELECTRIC;
glass					return GLASS;

open					return OPENOBJECT;
trunc					return TRUNCATEDOBJECT;
truncated				return TRUNCATEDOBJECT;
file					return FILEOBJECT;
normal					return NORMALOBJECT;
data					return DATAOBJECT;

begin					return BEGINGROUP;
group					return BEGINGROUP;
end					return ENDGROUP;
ungroup					return ENDGROUP;

next					return NEXTGROUP;

sphere					return SPHERE;
cube					return CUBE;
box					return BOX;
cone					return CONE;
cylinder				return CYLINDER;
wedge					return WEDGE;
tetra					return TETRA;
prism					return PRISM;
pyramid					return PYRAMID;
disc					return DISC;
ring					return RING;
patch					return PATCH;
bicubic{U}patch				return PATCH;
polygon					return POLYGON;
triangle				return TRIANGLE;
quadrangle				return QUADRANGLE;
torus					return TORUS;
text3d					return TEXT3D;
csg					return CSG;
list					return LIST;
hemisphere				return HEMISPHERE;

transform				return TRANSFORM;
none					return NONE;
reset					return NONE;
scale					return SCALE;
translate				return TRANSLATE;
rotate					return ROTATE;
general					return GENERAL;

texture					return NULLTEXTURE;
local					return LOCALTEXTURE;
invariant				return LOCALTEXTURE;
checkers?				return CHECKERSTEXTURE;
blotch					return BLOTCHTEXTURE;
bump					return BUMPTEXTURE;
marble					return MARBLETEXTURE;
fbm					return FBMTEXTURE;
fbm{U}bump				return FBMBUMPTEXTURE;
wood					return WOODTEXTURE;
round					return ROUNDTEXTURE;
bozo					return BOZOTEXTURE;
ripples					return RIPPLESTEXTURE;
waves					return WAVESTEXTURE;
spotted					return SPOTTEDTEXTURE;
dents					return DENTSTEXTURE;
agate					return AGATETEXTURE;
wrinkles				return WRINKLESTEXTURE;
granite					return GRANITETEXTURE;
gradient				return GRADIENTTEXTURE;
image{U}map				return IMAGEMAPTEXTURE;
map{U}image				return IMAGEMAPTEXTURE;
gloss					return GLOSSTEXTURE;
bump3					return BUMP3TEXTURE;

int					return INTFUNCTION;
sin					return SINFUNCTION;
cos					return COSFUNCTION;
tan					return TANFUNCTION;
asin					return ASINFUNCTION;
acos					return ACOSFUNCTION;
atan					return ATANFUNCTION;
sqrt					return SQRTFUNCTION;
rtod					return RTODFUNCTION;
dtor					return DTORFUNCTION;
exp					return EXPFUNCTION;
log					return LOGFUNCTION;
abs					return ABSFUNCTION;
max					return MAXFUNCTION;
min					return MINFUNCTION;

dotvector				return DOT3FUNCTION;
addvector				return ADD3FUNCTION;
diffvector				return DIFF3FUNCTION;
scalevector				return SCALE3FUNCTION;
crossvector				return CROSS3FUNCTION;
normvector				return NORM3FUNCTION;

union					return CSG_UNION;
subtraction				return CSG_SUBTRACTION;
difference				return CSG_SUBTRACTION;
intersection				return CSG_INTERSECTION;

{D}*{P}{D}+({E})? 			return NUM;
{D}+({P}{D}+)?{E}			return NUM;
{D}+					return NUMINT;
{L}{C}+					return TOKEN;
{F}*{C}+({G}|{C})*{P}{C}+		return FILETOKEN;

.					return yytext[0];

%%
