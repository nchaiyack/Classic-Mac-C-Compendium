/* Definitions of all the resources. */

/* The menu bar. */

#define mbMain 128

/* Menus. */

#define mApple 128
#define mFile 129
#define mEdit 130
#define mFind 131
#define mPlay 132
#define mSide 133
#define mWindows 134
#define mViewMap 135

#define mViewList 235
#define mViewCloseup 335

#define mSides 200
#define mViewWeather 201
#define mMaterialTypes 202
#define mViewFontSizes 203
#define mTerrainTypes 204
#define mUnitTypes 205
#define mAITypes 206
#define mAgreements 208
#define mMagnifications 209

#define mFeatures 212

/* Menu items. */

#define miAppleAbout 1
#define miAppleHelp 2
/* 3-5 are debug flags */

#define miFileNew 1
#define miFileOpen 2
/* 3 */
#define miFileConnect 4
/* 5 */
#define miFileSave 6
#define miFileSaveAs 7
/* 8 */
#define miFilePreferences 9
/* 10 */
#define miFilePageSetup 11
#define miFilePrintWindow 12
/* 13 */
#define miFileQuit 14

#define miEditUndo 1
/* 2 */
#define miEditCut 3
#define miEditCopy 4
#define miEditPaste 5
#define miEditClear 6
/* 7 */
#define miEditSelectAll 8
/* 9 */
#define miEditDesign 10

#define miFindPrevious 1
#define miFindNext 2
/* 3 */
#define miFindLocation 4
#define miFindUnitByName 5
/* 6 */
#define miFindSelected 7

#define miPlayCloseup 1
/* 2 */
#define miPlayMove 3
#define miPlayPatrol 4
#define miPlayReturn 5
/* 6 */
#define miPlayWake 7
#define miPlaySleep 8
#define miPlayDoneMoving 9
/* 10 */
#define miPlayBuild 11
#define miPlayRepair 12
/* 13 */
#define miPlayAttack 14
#define miPlayOverrun 15
#define miPlayFire 16
#define miPlayDetonate 17
/* 18 */
#define miPlayTake 19
#define miPlayDrop 20
#define miPlayGive 21
#define miPlayDisband 22
/* 23 */
#define miPlayRename 24

#define miSideCloseup 1
#define miSideFinishedTurn 2
/* 3 */
#define miSideMoveOnClick 4
#define miSideAutoSelect 5
#define miSideAutoFinish 6
/* 7 */
#define miSideSound 8
/* 9 */
/* 10 is AI Type submenu */
/* 11 */
#define miSideDoctrine 12
/* 13 */
#define miSideRename 14

#define miWindowsGame 1
#define miWindowsHistory 2
#define miWindowsConstruction 3
#define miWindowsAgreements 4
/* 5 */
#define miWindowsNewMap 6
#define miWindowsNewList 7
/* 8 */
#define miWindowsWorldMap 9
/* 10 */
#define miWindowsFirst 11

#define miViewCloser 1
#define miViewFarther 2
/* 3 is mag submenu */
/* 4 */
#define miViewNames 5
#define miViewElevations 6
#define miViewPeople 7
#define miViewWeather 8
#define miViewMaterials 9
#define miViewDaylight 10
/* 11 */
#define miViewGrid 12
#define miViewTopline 13
#define miViewOtherMaps 14
/* 15 is font size submenu */

#define miViewByType 1
#define miViewByName 2
#define miViewBySide 3
#define miViewByActOrder 4
#define miViewByLocation 5
/* 6 */
#define miViewWithTransport 7
#define miViewWithCommander 8
/* 9 */
#define miViewIconSize 10

#define miWeatherTemperature 1
#define miWeatherWinds 2
#define miWeatherClouds 3
#define miWeatherStorms 4

/* Dialogs. */

#define dSplash 128

#define diSplashNew 1
#define diSplashOpen 2
#define diSplashConnect 3
#define diSplashQuit 4
#define diSplashVersion 5
#define diSplashCopyright 6
#define diSplashPicture 7

#define dNewGame 129

#define diNewGameOK 1
#define diNewGameCancel 2
#define diNewGameList 3
#define diNewGameBlurb 4
#define diNewGamePicture 5
/* 6 is a fixed pict */

#define dPlayerSetup 130

#define diPlayerSetupOK 1
#define diPlayerSetupCancel 2
#define diPlayerSetupList 3
#define diPlayerSetupAdd 4
#define diPlayerSetupRemove 5
#define diPlayerSetupAdvantage 6
#define diPlayerSetupRename 7
#define diPlayerSetupAI 8
#define diPlayerSetupRemote 9
#define diPlayerSetupExchange 10

#define dVariants 131

#define diVariantsOK 1
#define diVariantsCancel 2
#define diVariantsText 3
#define diVariantsFirstCheckBox 4
/* and next 9 also */
#define diVariantsFirstSlider 14
/* and next 2 also */
#define diVariantsWorldSeen 17
#define diVariantsSeeAll 18
#define diVariantsWorldSize 19
#define diVariantsHelp 20

#define dProgress 132

#define diProgressText 1
#define diProgressBar 2
#define diProgressCancel 3

#define dAbout 133

#define diAboutOK 1
#define diAboutVersion 2
#define diAboutCopyright 3
#define diAboutPicture 4

#define dWorldShape 134

#define diWorldShapeOK 1
#define diWorldShapeCancel 2
#define diWorldShapePicture 3
#define diWorldShapeCircumference 4
#define diWorldShapeWidth 5 
#define diWorldShapeHeight 6
#define diWorldShapeLatitude 7
#define diWorldShapeLongitude 8

#define dPreferences 150

#define diPrefsOK 1
#define diPrefsCancel 2
#define diPrefsGrid 3
#define diPrefsNames 4
#define diPrefsCheckpoint 5
#define diPrefsEvery 6
#define diPrefsInterval 7
#define diPrefsTurns 8

#define dDesignerPalette 500

#define diDesignerPaletteTPal 1
#define diDesignerPaletteUPal 2
#define diDesignerPaletteExplanation 3
#define diDesignerPaletteSide 4

#define dDesignerSave 501

#define diDesignerSaveOK 1
#define diDesignerSaveCancel 2
#define diDesignerSaveName 3
#define diDesignerSaveModule 4
#define diDesignerSaveTypes 5
#define diDesignerSaveTables 6
#define diDesignerSaveGlobals 7
#define diDesignerSaveWorld 8
#define diDesignerSaveAreas 9
#define diDesignerSaveAreaTerrain 26
#define diDesignerSaveAreaMisc 27
#define diDesignerSaveAreaWeather 28
#define diDesignerSaveAreaMaterial 29
#define diDesignerSaveAreas 9
#define diDesignerSaveAreas 9
#define diDesignerSaveSides 10
#define diDesignerSavePlayers 11
#define diDesignerSaveUnits 12
#define diDesignerSaveScoring 13
#define diDesignerSaveHistory 14
#define diDesignerSaveReshape 15
#define diDesignerSaveCompress 16
#define diDesignerSaveSideNames 17
#define diDesignerSaveSideProps 18
#define diDesignerSaveSideViews 19
#define diDesignerSaveUnitProps 20
#define diDesignerSaveUnitMoves 21
#define diDesignerSaveUnitPlans 22
#define diDesignerSaveDoctrine 25
#define diDesignerSaveUnitIds 30

#define dDesignerReshape 502

#define diDesignerReshapeOK 1
#define diDesignerReshapeCancel 2
#define diDesignerReshapeOrigWidth 3
#define diDesignerReshapeOrigHeight 4
#define diDesignerReshapeOrigWorld 5
#define diDesignerReshapeOrigSubWidth 6
#define diDesignerReshapeOrigSubHeight 7
#define diDesignerReshapeOrigSubX 8
#define diDesignerReshapeOrigSubY 9
#define diDesignerReshapeOutputSubWidth 10
#define diDesignerReshapeOutputSubHeight 11
#define diDesignerReshapeOutputSubX 12
#define diDesignerReshapeOutputSubY 13
#define diDesignerReshapeOutputWidth 14
#define diDesignerReshapeOutputHeight 15
#define diDesignerReshapeOutputWorld 16
#define diDesignerReshapeFillTerrain 17

#define dSideCloseup 200

#define diSideCloseupOK 1
#define diSideCloseupApply 2
#define diSideCloseupCancel 3
#define diSideCloseupIcon 4
#define diSideCloseupName 6
#define diSideCloseupPlayer 7
#define diSideCloseupMachineStrategy 8
#define diSideCloseupAutofinish 9

#define dSideRename 201

#define diSideRenameOK 1
#define diSideRenameCancel 2
#define diSideRenameRandom 3
#define diSideRenameName 4
#define diSideRenameFullName 5
#define diSideRenameAcronym 6
#define diSideRenameNoun 7
#define diSideRenamePluralNoun 8
#define diSideRenameAdjective 9
#define diSideRenameEmblemName 10
#define diSideRenameColorScheme 11

#define dRename 202

#define diRenameOK 1
#define diRenameCancel 2
#define diRenameRandom 3
#define diRenameName 4
#define diRenameText 5

#define dFeatureRename 203

#define diFeatureRenameOK 1
#define diFeatureRenameCancel 2
#define diFeatureRenameType 3
#define diFeatureRenameName 4

#define dCommand 204

#define diCommandOK 1
#define diCommandCancel 2
#define diCommandText 3

#define dBuildPlan 301

#define diBuildPlanOK 1
#define diBuildPlanCancel 2
#define diBuildPlanUnit 3
#define diBuildPlanCurNext 4
#define diBuildPlanNumber 5
#define diBuildPlanCurType 6

#define dMultiBuild 302

#define diMultiBuildBuild 1
#define diMultiBuildUnitList 2
#define diMultiBuildTypeList 3
#define diMultiBuildRunLength 4
/* 5 */
#define diMultiBuildCurType 6
#define diMultiBuildNextType 7

#define dInstructions 600

#define diInstructionsTitle 1
#define diInstructionsHelp 2
#define diInstructionsText 3

#define dUnitTypeDesc 601

#define diUnitTypeDescIcon 1
#define diUnitTypeDescName 2
#define diUnitTypeDescHelp 3
#define diUnitTypeDescACP 5
#define diUnitTypeDescMP 7
#define diUnitTypeDescAvail 8
#define diUnitTypeDescNotes 9

#define dMaterialTypeDesc 602

#define diMaterialTypeDescIcon 1
#define diMaterialTypeDescName 2
#define diMaterialTypeDescHelp 3
#define diMaterialTypeDescPeople 5
#define diMaterialTypeDescAvail 7
#define diMaterialTypeDescNotes 8

#define dTerrainTypeDesc 603

#define diTerrainTypeDescIcon 1
#define diTerrainTypeDescName 2
#define diTerrainTypeDescHelp 3
#define diTerrainTypeDescElevMin 5
#define diTerrainTypeDescElevMax 7
#define diTerrainTypeDescNotes 8
#define diTerrainTypeDescTempMin 10
#define diTerrainTypeDescTempMax 12
#define diTerrainTypeDescCapacity 14
#define diTerrainTypeDescAvail 15

#define dGameModuleDesc 604

#define dWinGame 900

#define diWinGameQuit 1
#define diWinGameKeepGoing 2

#define dLoseGame 901

#define diLoseGameQuit 1
#define diLoseGameKeepGoing 2

#define dGameOver 902

#define diGameOverQuit 1
#define diGameOverKeepGoing 2

/* Alerts. */

#define aInitWarning 1000

#define aiInitWarningOK 1
#define aiInitWarningText 4

#define aInitError 1001

#define aiInitErrorOK 1
#define aiInitErrorText 3

#define aRunWarning 1002

#define aiRunWarningOK 1
#define aiRunWarningText 4

#define aRunError 1003

#define aiRunErrorOK 1
#define aiRunErrorText 3

#define aImagesMissing 1004

#define aResignGame 1999

#define aiResignGameOK 1
#define aiResignGameCancel 2
#define aiResignGameText 3

#define aQuitGame 2000

#define aiQuitGameOK 1
#define aiQuitGameCancel 2
#define aiQuitGameDontSave 3

#define aOutOfGame 2002

#define aConfirmDesign 3001

#define aiConfirmDesignOK 1
#define aiConfirmDesignCancel 2

/* Generic windows. */

#define wGame 128
#define wDesign 129
#define wMap 130
#define wList 131
#define wUnitCloseup 132
#define wHistory 133
#define wConstruction 134
#define wHelp 135

/* Controls. */

#define cConstructButton 128
#define cResearchButton 129
#define cRunLengthPopup 210

#define cTopicsButton 131
#define cHelpButton 132
#define cPrevButton 133
#define cNextButton 134
#define cBackButton 135

#define cNewFeatureButton 401
#define cFeatureRenameButton 402

/* Cursors. */

#define cOpenCross 154
#define cAllMove 155
#define cNoMove 156
#define cGrayArrow 157

#define cParens1 128
#define NUMcParens 6
#define cSynth1 139
#define NUMcSynth 8
#define cMove1 148
#define NUMcMoves 6

#define cCell 201
#define cUnit 202
#define cPeople 203
#define cMaterial 204
#define cFeature 205
#define cElevation 206
#define cTemperature 207
#define cClouds 208
#define cWinds 209

#define cBord 291
#define cConn 292
#define cCoat 293

/* Pictures. */

#define pEmptyPict 128
#define pMapControlsBL 129
#define pMapControlsTL 130
#define pUpDownPicture 131

#define pSplashBW 2000
#define pSplashColor 2001
#define pNewGameDecor 2002

/* Strings. */

#define sFilenames 128

#define siPreferences 1
#define siLibFolder 2
#define siLibMacFolder 3
#define siResources 4
#define siImages 5

/* Small icons. */

#define sicnWinds1 2101
