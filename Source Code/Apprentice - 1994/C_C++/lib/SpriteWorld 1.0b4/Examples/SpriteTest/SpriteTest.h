///--------------------------------------------------------------------------------------
// SpriteTest.h
//
// Created: 8/14/91 at 1:53 AM
// By: Tony Myles
//
// Copyright � 1991-94 Tony Myles, All rights reserved worldwide.
///--------------------------------------------------------------------------------------


#ifndef __SPRITETEST__
#define __SPRITETEST__

#ifndef __MENUS__
#include <Menus.h>
#endif

#ifndef __SPRITEWORLD__
#include "SpriteWorld.h"
#endif

#ifndef __SPRITE__
#include "Sprite.h"
#endif


enum
{
	kTestCIconID = 128,
	kNumberOfTestSprites = 4,
	kNumberOfTestFrames = 29,
	kTestSpriteMoveTime = 20,
	kTestSpriteFrameTime = 20,
	kBackDropPixPatID = 128,
	kPerformanceAlertID = 200,
	kTestTime = (30 * 60)
};

enum
{
	kNumberOfCommands = 9,
	kSpriteTestTitleCommand = 0,
	kBouncingBallsCommand,
	kCollisionDetectionCommand,
	kCopyBitsTestCommand,
	kBlitPixieTestCommand,
	kSpriteTestCommand,
	kCompileSpriteCommand,
	kRunCompiledSpriteCommand,
	kRunSafeCompiledSpriteCommand
};


typedef struct SpriteTestRec SpriteTestRec;
typedef SpriteTestRec *SpriteTestPtr, **SpriteTestHdl;


struct SpriteTestRec
{
	SpriteWorldPtr spriteWorldP;
	SpriteLayerPtr spriteLayerP;
	SpritePtr testSpriteArray[kNumberOfTestSprites];
	SpritePtr titleSpriteP;
	FramePtr testFrameP;
	FramePtr titleFrameP;
	Boolean isTestRunning;
	char pad1;
	Boolean isCommandActive[kNumberOfCommands];
};


OSErr CreateSpriteTest(
	SpriteTestPtr* spriteTestP,
	CWindowPtr srcWindowP);

void DisposeSpriteTest(
	SpriteTestPtr spriteTestP);

void SetupSpriteTest(
	SpriteTestPtr spriteTestP);

void SetupTestSprite(
	SpritePtr testSpriteP,
	Rect *moveBoundsRect,
	short horizLocation,
	short vertLocation);

void SWBounceSpriteCollideProc(
	SpritePtr srcSpriteP,
	SpritePtr dstSpriteP,
	Rect* sectRect);

void RunSpriteTest(
	SpriteTestPtr spriteTestP);

void UpdateSpriteTest(
	SpriteTestPtr spriteTestP);

void HandleCreateSpriteCommand(
	SpriteTestPtr spriteTestP);

void HandleGetSpriteInfoCommand(
	SpriteTestPtr spriteTestP);

void HandleRunPauseCommand(
	SpriteTestPtr spriteTestP);

void HandleSpriteTestTitleCommand(
	SpriteTestPtr spriteTestP);

void HandleBouncingBallsCommand(
	SpriteTestPtr spriteTestP);

void CopyBitsSpeedTestCommand(
	SpriteTestPtr spriteTestP);

void BlitPixieSpeedTestCommand(
	SpriteTestPtr spriteTestP);

void SpriteTestCommand(
	SpriteTestPtr spriteTestP,
	CWindowPtr srcWindowP);

void DisplayPerformance(
	long frames,
	long seconds);

void CompileSpriteCommand(
	SpriteTestPtr spriteTestP);

void RunCompiledSpriteCommand(
	SpriteTestPtr spriteTestP,
	Boolean useQuickDraw);


#endif