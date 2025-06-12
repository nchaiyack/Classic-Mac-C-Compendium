COptions = -b -b2 -b3 -mf -opt full -w -d OLDROUTINENAMES=0 -d SystemSevenOrLater -d SystemSixOrLater {SymOptions}
CPlusOptions = -b -b2 -b3 -mf -opt full -d OLDROUTINENAMES=0 -w -d SystemSevenOrLater -d SystemSixOrLater {SymOptions}
SymOptions =  -sym full -mbg off
LinkOptions = -sym full -mf "{Libraries}"Interface.o "{Libraries}"Runtime.o
ObjDir = :Objects:
CdevSrcDir = :Cdev:
InitSrcDir = :Init:

InitObjects = ¶
"{ObjDir}"Install.a.o ¶
"{ObjDir}"Init.c.o ¶
"{ObjDir}"nbpaction.c.o ¶
"{ObjDir}"patch.a.o ¶
"{ObjDir}"ShowInit.a.o

CdevObjects = ¶
"{ObjDir}"CControlPanelMain.cp.o ¶
"{ObjDir}"ChooserDialog.cp.o ¶
"{ObjDir}"CControlPanel.cp.o ¶
"{ObjDir}"CListManager.cp.o ¶
"{ObjDir}"New&Del.c.o

{ObjDir} Ä {InitSrcDir} {CdevSrcDir}

c.o Ä .c
	flush
   {C} {DepDir}{Default}.c -o {ObjDir}{Default}.c.o

Incognito ÄÄ Incog.r Incognito.rsrc
	rez Incog.r -a -o Incognito
Incognito ÄÄ {InitObjects}
	Link {InitObjects} -o Incognito -m INITInstall -ra Main=resSysHeap,resLocked -rt INIT=0  {LinkOptions} "{Libraries}"GestaltValue.o
	setfile Incognito  -t cdev -c HEID -a B
	rename Incognito.SYM 'Incognito/INIT_0.SYM' -y
Incognito ÄÄ {CdevObjects}
	Link {CdevObjects} -m ENTRYPOINT -o Incognito -rt cdev=-4064 {LinkOptions} "{Libraries}"DialogUtil.o
	setfile Incognito -t cdev -c HEID -a B
	rename Incognito.SYM 'Incognito/cdev_F020.SYM' -y
