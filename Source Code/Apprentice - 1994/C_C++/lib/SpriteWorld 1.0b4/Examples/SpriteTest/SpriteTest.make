#   File:       SpriteTest.make
#   Created:    Tuesday, February 1, 1994 10:57:33 PM
#	By:			Tony Myles
#	Copyright � 1994 Tony Myles, All rights reserved worldwide.


SWSourceDir = :::Sources:
SWHeaderDir = :::Headers:
SWObjectDir = :::Objects:
SpriteTestObjectDir = :Objects:
UtilsDir = ::Utils:

COptions = -d MPW -r -mbg full -i "{SWHeaderDir}" -i "{UtilsDir}"
#COptions = -d MPW -r -mbg full -opt full -w2 -i "{SWHeaderDir}" -i "{UtilsDir}"
ASMOptions = -case on

OBJECTS = �
		"{SWObjectDir}Frame.c.o" �
		"{SWObjectDir}Sprite.c.o" �
		"{SWObjectDir}SpriteLayer.c.o" �
		"{SWObjectDir}SpriteWorld.c.o" �
		"{SWObjectDir}SpriteWorldUtils.c.o" �
		"{SWObjectDir}SpriteCompiler.c.o" �
		"{SWObjectDir}BlitPixie.c.o" �
		"{UtilsDir}DialogUtils.c.o" �
		"{UtilsDir}DebugUtils.c.o" �
		"{UtilsDir}StringUtils.c.o" �
		"{UtilsDir}GameUtils.c.o" �
		"{SpriteTestObjectDir}About.c.o" �
		"{SpriteTestObjectDir}Application.c.o" �
		"{SpriteTestObjectDir}SpriteTest.c.o"



SpriteTest �� SpriteTest.make {OBJECTS}
	Link -t APPL -c '????' �
		{OBJECTS} �
		"{Libraries}"Runtime.o �
		"{Libraries}"Interface.o �
		-o SpriteTest

SpriteTest �� SpriteTest.r SpriteTest.�.rsrc
	rez SpriteTest.r -a -o SpriteTest

"{SWObjectDir}Frame.c.o" � "{SWSourceDir}Frame.c" "{SWHeaderDir}Frame.h"
	C "{SWSourceDir}Frame.c" {COptions} -o "{SWObjectDir}Frame.c.o"

"{SWObjectDir}Sprite.c.o" � "{SWSourceDir}Sprite.c" "{SWHeaderDir}Sprite.h"
	C "{SWSourceDir}Sprite.c" {COptions} -o "{SWObjectDir}Sprite.c.o"

"{SWObjectDir}SpriteLayer.c.o" � "{SWSourceDir}SpriteLayer.c" "{SWHeaderDir}SpriteLayer.h"
	C "{SWSourceDir}SpriteLayer.c" {COptions} -o "{SWObjectDir}SpriteLayer.c.o"

"{SWObjectDir}SpriteWorld.c.o" � "{SWSourceDir}SpriteWorld.c" "{SWHeaderDir}SpriteWorld.h"
	C "{SWSourceDir}SpriteWorld.c" {COptions} -o "{SWObjectDir}SpriteWorld.c.o"

"{SWObjectDir}SpriteWorldUtils.c.o" � "{SWSourceDir}SpriteWorldUtils.c" "{SWHeaderDir}SpriteWorldUtils.h"
	C "{SWSourceDir}SpriteWorldUtils.c" {COptions} -o "{SWObjectDir}SpriteWorldUtils.c.o"

"{SWObjectDir}SpriteCompiler.c.o" � "{SWSourceDir}SpriteCompiler.c" "{SWHeaderDir}SpriteCompiler.h"
	C "{SWSourceDir}SpriteCompiler.c" {COptions} -o "{SWObjectDir}SpriteCompiler.c.o"

"{SWObjectDir}BlitPixie.c.o" � "{SWSourceDir}BlitPixie.c" "{SWHeaderDir}BlitPixie.h"
	C "{SWSourceDir}BlitPixie.c" {COptions} -o "{SWObjectDir}BlitPixie.c.o"

"{UtilsDir}DialogUtils.c.o" � "{UtilsDir}DialogUtils.c" "{UtilsDir}DialogUtils.h"
	C "{UtilsDir}DialogUtils.c" {COptions} -o "{UtilsDir}DialogUtils.c.o"

"{UtilsDir}DebugUtils.c.o" � "{UtilsDir}DebugUtils.c" "{UtilsDir}DebugUtils.h"
	C "{UtilsDir}DebugUtils.c" {COptions} -o "{UtilsDir}DebugUtils.c.o"

"{UtilsDir}StringUtils.c.o" � "{UtilsDir}StringUtils.c" "{UtilsDir}StringUtils.h"
	C "{UtilsDir}StringUtils.c" {COptions} -o "{UtilsDir}StringUtils.c.o"

"{UtilsDir}GameUtils.c.o" � "{UtilsDir}GameUtils.c" "{UtilsDir}GameUtils.h"
	C "{UtilsDir}GameUtils.c" {COptions} -o "{UtilsDir}GameUtils.c.o"

"{SpriteTestObjectDir}About.c.o" � About.c About.h
	C "About.c" {COptions} -o "{SpriteTestObjectDir}About.c.o"

"{SpriteTestObjectDir}Application.c.o" � Application.c Application.h
	C "Application.c" {COptions} -o "{SpriteTestObjectDir}Application.c.o"

"{SpriteTestObjectDir}SpriteTest.c.o" � SpriteTest.c SpriteTest.h
	C "SpriteTest.c" {COptions} -o "{SpriteTestObjectDir}SpriteTest.c.o"
