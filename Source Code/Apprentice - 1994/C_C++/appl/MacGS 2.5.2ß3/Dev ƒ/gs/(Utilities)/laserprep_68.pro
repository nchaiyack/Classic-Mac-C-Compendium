% Description:
% 	This is the Postscript code for LaserPrep that is used
% 	on the Apple Laserwriter. It was captured using
% 	Clover-K in MacDraw. This is version 68 of the LaserPrep.
%
% 	All lines that that have `-lmm-' were commented out. The choice
%	for commenting out lines was by looking at some work done by
%	Jim Stuckey (?) of Arbortext.
%
% Work by: Louis McDonald
%	   May 1, 1987 (while at Hughes Aircraft)
%	   email as of 1/27/90: louis@aerospace.aero.org
%
% NOTES: please leave the Apple Computer copyright, and my name.
%
%%%Title: "Laser Prep -- The Apple PostScript Dictionary (md)"
%%Creator: Apple Software Engineering
%%CreationDate: Thursday, March 19, 1987
%{appledict version #68 0
% ) CopyRight Apple Computer, Inc. 1984,1985,1986,1987,1988 All Rights Reserved.
%%EndComments
%%BeginProcSet: "(AppleDict md)" 68 0
%-lmm- statusdict begin waittimeout 300 lt{0 60 300 setdefaulttimeouts}if end
systemdict/currentpacking known{currentpacking true setpacking}if
/LW{save statusdict/product get(LaserWriter)anchorsearch
exch pop{length 0 eq{1}{2}ifelse}{0}ifelse exch restore}bind def
/LW+{LW 2 eq}bind def
/ok{systemdict/statusdict known dup{LW 0 gt and}if}bind def
%-lmm- ok{statusdict begin 9 sccinteractive 3 ne exch 0 ne or{9 0 3 setsccinteractive}if end}if
/md 250 dict def md begin
/av 0 def
/T true def/F false def/mtx matrix def/s75 75 string def/s8 8 string def/s1 ( ) def/pxs 1 def/pys 1 def
1 0 mtx defaultmatrix dtransform exch atan/pa exch def/nlw .24 def/ppr [-32 -29.52 762 582.48] def
/pgs 1 def/por true def/xb 500 array def/so true def/tso true def/fillflag false def/pnm 1 def/fmv true def
/sfl false def/ma 0 def/invertflag false def/dbinvertflag false def/xflip false def
/yflip false def/noflips true def/scaleby96 false def/fNote true def/fBitStretch true def
/fg (Rvd\001\001\000\000\177) def
/bdf{bind def}bind def
/xdf{exch def}bdf
/xl{neg exch neg translate}bdf
/fp{pnsh 0 ne pnsv 0 ne and}bdf
/nop{}bdf/lnop[/nop load]cvx bdf
/vrb[
{fp{fg 6 get 0 ne{gsave stroke grestore}{gsave 1 setlinewidth pnsh pnsv scale stroke grestore}ifelse}
if newpath}bind
/eofill load
dup
/newpath load
2 index
dup
{clip newpath}bind
{}bind
dup
2 copy
]def
currentscreen/spf xdf/rot xdf/freq xdf
/doop{vrb exch get exec}bdf
/psu{/tso xdf /fNote xdf/fBitStretch xdf/scaleby96 xdf/yflip xdf/xflip xdf
/invertflag xdf/dbinvertflag invertflag statusdict begin version cvr 47.0 ge product (LaserWriter) 
eq not and end invertflag and {not}if def
xflip yflip or{/noflips false def}if
/pgs xdf 2 index .72 mul exch div/pys xdf div .72 mul/pxs xdf ppr astore pop/por xdf sn and/so xdf}bdf
/tab{statusdict /11x17 known{statusdict begin /11x17 load end}{statusdict 
/setpage known{statusdict begin 792 1224 1 setpage end}{statusdict 
/setpageparams known{statusdict begin 792 1224 0 1 setpageparams end}if}ifelse}ifelse}bdf
/txpose{fNote{smalls}{bigs}ifelse pgs get exec pxs pys scale ppr aload pop 
por{noflips{pop exch neg exch translate pop 1 -1 scale}if
xflip yflip and{pop exch neg exch translate 180 rotate 1 -1 scale ppr 3 get ppr 1 
get neg sub neg ppr 2 get ppr 0 get neg sub neg translate}if 
xflip yflip not and{pop exch neg exch translate pop 180 rotate ppr 3 get ppr 1 get neg sub neg 0 translate}
if yflip xflip not and{ppr 1 get neg ppr 0 get neg translate}if}
{noflips{translate pop pop 270 rotate 1 -1 scale}if xflip yflip and
{translate pop pop 90 rotate 1 -1 scale ppr 3 get ppr 1 get neg sub neg ppr 2 get 
ppr 0 get neg sub neg translate}if
xflip yflip not and{translate pop pop 90 rotate ppr 3 get ppr 1 get neg sub neg 0 translate}if 
yflip xflip not and{translate pop pop 270 rotate ppr 2 get ppr 0 get neg sub neg 0 exch translate}if}ifelse
statusdict begin waittimeout 300 lt{/waittimeout 300 def}if end scaleby96{ppr aload pop 4 -1 
roll add 2 div 3 1 roll add 2 div 2 copy translate .96 dup scale neg exch neg exch translate}if}bdf
/fr{4 copy ppr aload pop 3 -1 roll add 3 1 roll exch add 6 2 roll 3 -1 roll
sub 3 1 roll exch sub 3 -1 roll exch div 3 1 roll div exch scale pop pop xl}bdf
/obl{{0.212557 mul}{pop 0}ifelse}bdf
/sfd{ps fg 5 -1 roll get mul 100 div 0 ps 5 -1 roll obl ps neg 0 0 6a astore makefont setfont}bdf
/fnt{findfont sfd}bdf
/bt{sa 3 1 roll 3 index and put}bdf
/sa(\000\000\000\000\000\000\000\000\000\000)def
/fs{0 1 bt 1 2 bt 2 4 bt 3 8 bt 4 16 bt 5 32 bt 6 64 bt 7 128 bt sa exch 8 exch put}bdf
/mx1 matrix def
/mx2 matrix def
/mx3 matrix def
/bu{currentpoint currentgray currentlinewidth currentlinecap currentlinejoin currentdash exch aload length
fg 5 sfl{1}{0}ifelse put pnsv pnsh
2t aload pop 3a aload pop mx2 aload pop mx1 aload pop mtx currentmatrix aload pop
mx3 aload pop ps pm restore/ps xdf mx3 astore pop}bdf
/bn{/pm save def mx3 setmatrix newpath 0 0 moveto ct dup 39 get 0 exch getinterval cvx exec
mtx astore setmatrix mx1 astore pop mx2 astore pop 3a astore pop
2t astore pop/pnsh xdf/pnsv xdf gw
/sfl fg 5 get 0 ne def array astore exch setdash setlinejoin setlinecap
setlinewidth setgray moveto}bdf
/fc{save vmstatus exch sub 50000 lt
{(%%[|0|]%%)=print flush}if pop restore}bdf
/tc{32768 div add 3 1 roll 32768 div add 2t astore pop}bdf
/3a [0 0 0] def
/2t 2 array def
/tp{3a astore pop}bdf
/tt{mx2 currentmatrix pop currentpoint 2 copy 2t aload pop qa 2 copy translate 3a aload pop exch dup 0 eq
{pop}{1 eq{-1 1}{1 -1}ifelse scale}ifelse rotate pop neg exch neg exch translate moveto}bdf
/te{mx2 setmatrix}bdf
/th{3 -1 roll div 3 1 roll exch div 2 copy mx1 scale pop scale/sfl true def}bdf
/tu{1 1 mx1 itransform scale/sfl false def}bdf
/ts{1 1 mx1 transform scale/sfl true def}bdf
/fz{/ps xdf}bdf
/dv{dup 0 ne{div}{pop}ifelse}bdf
/pop4{pop pop pop pop}bdf
/it{sfl{mx1 itransform}if}bdf
/gm{exch it moveto}bdf/rm{it rmoveto}bdf
/lm{currentpoint sfl{mx1 transform}if exch pop sub 0 exch it rmoveto}bdf
/fm{statusdict/manualfeed known}bdf
/se{statusdict exch/manualfeed exch put}bdf
/mf{dup/ma exch def 0 gt{fm se/t1 5 st ok ma 1 gt and{/t2 0 st/t3 0 st
statusdict/manualfeedtimeout 3600 put
}if}if}bdf
/jn{/statusdict where exch pop{statusdict exch /jobname exch put}if}bdf
/pen{pnm mul/pnsh xdf pnm mul/pnsv xdf pnsh setlinewidth}bdf
/min{2 copy gt{exch}if pop}bdf
/max{2 copy lt{exch}if pop}bdf
/dh{fg 6 1 put array astore dup {1 pxs div mul exch}forall astore exch pop exch pop exch setdash}bdf
/ih[currentdash]def
/rh{fg 6 0 put ih aload pop setdash}bdf
/dl{gsave nlw pys div setlinewidth 0 setgray}bdf
/dlin{exch currentpoint currentlinewidth 2 div dup
translate newpath moveto lineto currentpoint stroke grestore moveto}bdf
/lin{fg 6 get 0 ne{exch lineto currentpoint 0 doop moveto}
{exch currentpoint/pnlv xdf/pnlh xdf gsave newpath/@1 xdf/@2 xdf fp{pnlh @2 lt{pnlv @1 ge
{pnlh pnlv moveto @2 @1 lineto pnsh 0 rlineto
0 pnsv rlineto pnlh pnsh add pnlv pnsv add lineto pnsh neg 0 rlineto}
{pnlh pnlv moveto pnsh 0 rlineto @2 pnsh add @1 lineto 0 pnsv rlineto
pnsh neg 0 rlineto pnlh pnlv pnsv add lineto}ifelse}{pnlv @1 gt
{@2 @1 moveto pnsh 0 rlineto pnlh pnsh add pnlv lineto 0 pnsv rlineto
pnsh neg 0 rlineto @2 @1 pnsv add lineto}{pnlh pnlv moveto pnsh 0 rlineto
0 pnsv rlineto @2 pnsh add @1 pnsv add lineto pnsh neg 0 rlineto
0 pnsv neg rlineto}ifelse}ifelse
closepath fill}if @2 @1 grestore moveto}ifelse}bdf
/gw{/pnm fg 3 get fg 4 get div def}bdf
/lw{fg exch 4 exch put fg exch 3 exch put gw pnsv pnsh pen}bdf
/barc{/@1 xdf/@2 xdf/@3 xdf/@4 xdf/@5 xdf
/@6 xdf/@7 xdf/@8 xdf gsave
@5 @7 add 2 div @6 @8 add 2 div translate newpath 0 0 moveto
@5 @7 sub @6 @8 sub mtx currentmatrix pop scale @1{newpath}if
0 0 0.5 @4 @3 arc @4 @3 sub abs 360 ge{closepath}if
mtx setmatrix @2 doop grestore}bdf
/ar{dup 0 eq barc}bdf
/ov{0 exch 360 exch true barc}bdf
/rc{/@t xdf currentpoint 6 2 roll newpath 4 copy 4 2 roll exch moveto
6 -1 roll lineto lineto lineto closepath @t doop moveto}bdf
/mup{dup pnsh 2 div le exch pnsv 2 div le or}bdf
/rr{/@1 xdf 2. div/@2 xdf 2. div/@3 xdf
/@4 xdf/@5 xdf/@6 xdf/@7 xdf
@7 @5 eq @6 @4 eq @2 mup or or{@7 @6 @5 @4 @1 rc}
{@4 @6 sub 2. div dup @2 lt{/@2 xdf}{pop}ifelse
@5 @7 sub 2. div dup @2 lt{/@2 xdf}{pop}ifelse
@1 0 eq{/@2 @2 pnsh 2 div 2 copy gt{sub def}{0 pop4}ifelse}if
currentpoint newpath
@4 @6 add 2. div @7 moveto
@4 @7 @4 @5 @2 arcto pop4
@4 @5 @6 @5 @2 arcto pop4
@6 @5 @6 @7 @2 arcto pop4
@6 @7 @4 @7 @2 arcto pop4
closepath @1 doop moveto}ifelse}bdf
/pr{gsave newpath/pl{exch moveto/pl{exch lineto}def}def}bdf
/pl{exch lineto}bdf
/ep{dup 0 eq{{moveto}{exch lin}{}{(%%[|1|]%%)= flush}pathforall
pop grestore}{doop grestore}ifelse currentpoint newpath moveto}bdf
/gr{64. div setgray}bdf
/pat{s8 copy pop 9.375 pa por not{90 add}if{1 add 4 mul cvi s8 exch get exch 1 add 4 mul cvi 
7 sub bitshift 1 and}setscreen gr}bdf
/sg{freq rot/spf load setscreen gr}bdf
/dc{transform round .5 sub exch round .5 sub exch itransform}bdf
/sn{userdict/smooth4 known}bdf
/x8{3 bitshift}bdf
/x4{2 bitshift}bdf
/d4{-2 bitshift}bdf
/d8{-3 bitshift}bdf
/rb{15 add -4 bitshift 1 bitshift}bdf
/db{/@7 save def/@1 xdf/@2 xdf/@3 xdf/@4 xdf/@5 xdf/@6 @5 @3 4 add mul def
dc translate scale/xdbit 1 1 idtransform abs/ydbit exch def abs def{0 0 1 ydbit add 1 10 rc clip}if
@1 0 eq @1 4 eq or{1 setgray ydbit 0 1 ydbit add 1 2 rc}if
@1 3 eq @1 7 eq or{1}{0}ifelse setgray/@9 @1 0 eq @1 1 eq @1 3 eq or or dbinvertflag xor def/@13 @6 def
@2 fBitStretch or{/@10 @4 x4 def/@11 @3 x4 def/@12 @10 rb def/@13 @12 @11 mul def
/@15 1 1 dtransform abs/calcY 1 index def round cvi/@14 exch def
abs/calcX 1 index def round cvi scaleby96 not{1 add}if def/@16 @15 rb def/@17 @16 @14 mul def}if
sn @13 60000 lt and @2 fBitStretch or and{mtx currentmatrix dup 1 get exch 2 get 0. eq exch 0. eq and 
@17 60000 lt and fBitStretch and{@16 3 bitshift @14 @9 [calcX 0 0 calcY 0 0]{@17 string @13 string
currentfile @6 string readhexstring pop 1 index @4 @3 @5 @12 @2 smooth4
@10 @11 @12 dup string 5 index @15 @14 @16 dup string stretch}imagemask}
{@12 x8 @11 @9 [@10 0 0 @11 0 0]{@13 string
currentfile @6 string readhexstring pop 1 index @4 @3 @5 @12 @2 smooth4}imagemask}ifelse}
{@5 3 bitshift @3 4 add @9 [@4 0 0 @3 0 2]{currentfile @6 string readhexstring pop}imagemask}ifelse
@7 restore}bdf
/multibit{/mbdeep exch def/mbY exch def/mbX exch def
save mbX mbY mbdeep[mbX 0 0 mbY 0 0]{currentfile picstr readhexstring pop}image
restore}bdf
/wd 16 dict def
/mfont 14 dict def
/mdf{mfont wcheck not{/mfont 14 dict def}if mfont begin xdf end}bdf
/cf{{1 index/FID ne{def}{pop pop}ifelse}forall}bdf/rf{/@1 exch def/@2 exch def
FontDirectory @2 known{cleartomark pop}{findfont dup begin dup length @1 add dict begin
cf{/Encoding macvec def}{Encoding dup length array copy/Encoding exch def
counttomark 2 idiv{Encoding 3 1 roll put}repeat}ifelse
pop
exec currentdict end end @2 exch definefont pop}ifelse}bdf
/bmbc{exch begin wd begin
/cr xdf
save
CharTable cr 6 mul 6 getinterval{}forall
/bitheight xdf/bitwidth xdf
.96 div/width xdf
Gkernmax add/XOffset xdf Gdescent add/YOffset xdf/rowbytes xdf
rowbytes 255 eq{0 0 0 0 0 0 setcachedevice}
{Gnormsize dup scale
width 0 XOffset YOffset bitwidth XOffset add bitheight YOffset add
setcachedevice
rowbytes 0 ne{
XOffset YOffset translate newpath 0 0 moveto
bitwidth bitheight scale
sn{
/xSmt bitwidth x4 def
/ySmt bitheight x4 def
/rSmt xSmt rb def
rSmt x8 ySmt true
[xSmt 0 0 ySmt neg 0 ySmt]
{rSmt ySmt mul string CharData cr get
1 index bitwidth bitheight rowbytes rSmt tso smooth4}
}{rowbytes 3 bitshift bitheight 4 add true
[bitwidth 0 0 bitheight neg 0 bitheight 2 add]
{CharData cr get}
}ifelse
imagemask
}if
}ifelse
restore
end end
}bdf
/bb{.96 exch div/Gnormsize mdf 2 index
/Gkernmax mdf 1 index/Gdescent mdf
3 index div 4 1 roll
2 index div 1. 5 2 roll
exch div 4 1 roll
4 array astore/FontBBox mdf
}bdf
/cdf{mfont/CharData get 3 1 roll put}bdf
/bf{
mfont begin
/FontType 3 def
/FontMatrix [1 0 0 1 0 0] def
/Encoding macvec def
/BuildChar/bmbc load def
end
mfont definefont pop
}bdf
/wi LW 1 eq{{gsave 0 0 0 0 0 0 0 0 moveto lineto lineto lineto closepath clip stringwidth grestore}bind}
{/stringwidth load}ifelse def
/aps{0 get 124 eq}bdf
/xc{s75 cvs dup}bdf
/xp{put cvn}bdf
/scs{xc 3 67 put dup 0 95 xp}bdf
/sos{xc 3 79 xp}bdf
/sbs{xc 1 66 xp}bdf
/sis{xc 2 73 xp}bdf
/sob{xc 2 79 xp}bdf
/sss{xc 4 83 xp}bdf
/dd{exch 1 index add 3 1 roll add exch}bdf
/smc{moveto dup show}bdf
/kwn{FontDirectory 1 index known{findfont exch pop}}bdf
/gl{1 currentgray sub setgray}bdf
/mm{/mfont 10 dict def mfont begin
/FontMatrix [1 0 0 1 0 0] def
/FontType 3 def
/Encoding macvec def
/df 4 index findfont def
/FontBBox [0 0 1 1] def
/xda xdf/mbc xdf
/BuildChar{wd begin/cr xdf/fd xdf/cs s1 dup 0 cr put def fd/mbc get exec end}def
exec end mfont definefont}bdf
/ac{dup scs kwn{exch findfont dup length 1 add dict begin{1 index/FID ne 2 index/UniqueID ne 
and{def}{pop pop}ifelse}forall
fmv{/Encoding macvec def}if/StrokeWidth nlw 1000 mul pys div ps div dup 12 lt{pop 12}if def
/PaintType 2 def currentdict end definefont}ifelse}bdf
/mb{dup sbs kwn{exch{pop}{bbc}{}mm}ifelse sfd}bdf
/mo{dup sos kwn{exch{pop}{boc}{}mm}ifelse sfd}bdf
/ms{dup sss kwn{exch{pop}{bsc}{}mm}ifelse sfd}bdf
/ou{dup sos kwn{exch dup ac pop{scs findfont /df2 xdf}{aoc}{}mm}ifelse sfd}bdf
/su{dup sss kwn{exch dup ac pop{scs findfont /df2 xdf}{asc}{}mm}ifelse sfd}bdf
/ao{/fmv true def ou}bdf/as{/fmv true def su}bdf
/vo{/fmv false def ou}bdf/vs{/fmv false def su}bdf
/bbc{/da .03 def fd/df get setfont
gsave cs wi 1 index 0 ne{exch da add exch}if grestore setcharwidth
cs 0 0 smc da 0 smc da da smc 0 da moveto show}bdf
/boc{/da 1 ps div def fd/df get setfont
gsave cs wi 1 index 0 ne{exch da add exch}if grestore setcharwidth
cs 0 0 smc da 0 smc da da smc 0 da smc gl da 2. div dup moveto show}bdf
/bsc{/da 1 ps div def
/ds .05 def/da2 da 2. div def fd/df get setfont
gsave cs wi 1 index 0 ne{exch ds add da2 add exch}if grestore setcharwidth
cs ds da2 add .01 add 0 smc 0 ds da2 sub translate 0 0 smc
da 0 smc da da smc 0 da smc gl da 2. div dup moveto show}bdf
/aoc{fd/df get setfont
gsave cs wi grestore setcharwidth
gl cs 0 0 smc fd/df2 get setfont gl 0 0 moveto show}bdf
/asc{/da .05 def fd/df get setfont
gsave cs wi 1 index 0 ne{exch da add exch}if grestore setcharwidth
cs da .01 add 0 smc 0 da translate gl 0 0 smc gl fd/df2 get setfont 0 0 moveto show}bdf
/st{1000 mul usertime add dup 2147483647 gt{2147483647 sub}if def}bdf
/the{usertime sub dup 0 lt exch -2147483648 gt and}bdf
/6a 6 array def
/2a 2 array def
/3q 3 array def
/qs{3 -1 roll sub exch 3 -1 roll sub exch}bdf
/qa{3 -1 roll add exch 3 -1 roll add exch}bdf
/qm{3 -1 roll 1 index mul 3 1 roll mul}bdf
/qn{6a exch get mul}bdf
/qA .166667 def/qB .833333 def/qC .5 def
/qx{6a astore pop
qA 0 qn qB 2 qn add   qA 1 qn qB 3 qn add
qB 2 qn qA 4 qn add   qB 3 qn qA 5 qn add
qC 2 qn qC 4 qn add   qC 3 qn qC 5 qn add}bdf
/qp{6 copy 12 -2 roll pop pop}bdf
/qc{exch qp qx curveto}bdf
/qi{{exch 4 copy 2a astore aload pop qa .5 qm newpath moveto}{exch 2 copy 6 -2 roll 2 qm qs 4 2 roll}ifelse}bdf
/qq{{qc 2a aload pop qx curveto}{exch 4 copy qs qa qx curveto}ifelse}bdf
/pt{currentpoint newpath moveto}bdf
/qf{/fillflag true def}bdf
/ec{1 and 0 ne{0 doop}if grestore currentpoint newpath moveto/fillflag false def}bdf
/eu{currentpoint fp{0 ep}{grestore newpath}ifelse moveto/fillflag false def}bdf
/bp{currentpoint newpath 2 copy moveto}bdf
/ef{gsave fillflag{gsave eofill grestore}if}bdf
/sm{0 exch{@1 eq{1 add}if}forall}bdf
/lshow{4 1 roll exch/@1 exch def{1 index wi pop sub 1 index sm dv 0 @1 4 -1 roll widthshow}{1 index wi pop sub
1 index dup sm 10 mul exch length 1 sub add dv dup 10. mul 0 @1 4 -1 roll 0 6 -1 roll awidthshow}ifelse}bdf
/setTxMode{sa 9 2 index put 3 eq{1}{0}ifelse setgray}bdf
/SwToSym{{}mark false/Symbol/|______Symbol 0 rf 0 sa 6 get 0 ne{pop 1}{sa 7 get 0 eq{pop 2}if}ifelse
sa 1 get 0 ne/|______Symbol
sa 4 get 0 ne{vs}{sa 3 get 0 ne{vo}{fnt}ifelse}ifelse}bdf
/mc{0 3 1 roll transform neg exch pop}bdf
/ul{dup 0 ne sa 2 get 0 ne and{gsave 0 0
/UnderlinePosition kif{mc}{ps -10 div}ifelse/UnderlineThickness kif{mc}{ps 15 div}ifelse
abs setlinewidth neg rmoveto
sa 4 get 0 ne{gsave currentlinewidth 2. div dup rmoveto currentpoint newpath moveto
2 copy rlineto stroke grestore}if
sa 3 get sa 4 get or 0 ne{gsave gl 2 copy rlineto stroke grestore rlineto strokepath nlw pys div 
setlinewidth}{rlineto}ifelse
stroke grestore}{pop}ifelse}bdf
/sgt{2 copy known{get true}{pop pop false}ifelse}bdf
/kif{currentfont dup/FontMatrix get exch/FontInfo sgt{true}{currentfont/df sgt
{dup/FontInfo sgt{3 1 roll/FontMatrix get mtx concatmatrix exch true}{pop pop pop false}
ifelse}{pop pop false}ifelse}ifelse{3 -1 roll sgt{exch true}{pop false}ifelse}{false}ifelse}bdf
/blank/Times-Roman findfont/CharStrings get/space get def
/macvec 256 array def
/NUL/SOH/STX/ETX/EOT/ENQ/ACK/BEL/BS/HT/LF/VT/FF/CR/SO/SI
/DLE/DC1/DC2/DC3/DC4/NAK/SYN/ETB/CAN/EM/SUB/ESC/FS/GS/RS/US
macvec 0 32 getinterval astore pop
macvec 32/Times-Roman findfont/Encoding get
32 96 getinterval putinterval macvec dup 39/quotesingle put 96/grave put
/Adieresis/Aring/Ccedilla/Eacute/Ntilde/Odieresis/Udieresis/aacute
/agrave/acircumflex/adieresis/atilde/aring/ccedilla/eacute/egrave
/ecircumflex/edieresis/iacute/igrave/icircumflex/idieresis/ntilde/oacute
/ograve/ocircumflex/odieresis/otilde/uacute/ugrave/ucircumflex/udieresis
/dagger/degree/cent/sterling/section/bullet/paragraph/germandbls
/registered/copyright/trademark/acute/dieresis/notequal/AE/Oslash
/infinity/plusminus/lessequal/greaterequal/yen/mu/partialdiff/summation
/product/pi/integral/ordfeminine/ordmasculine/Omega/ae/oslash
/questiondown/exclamdown/logicalnot/radical/florin/approxequal/Delta/guillemotleft
/guillemotright/ellipsis/blank/Agrave/Atilde/Otilde/OE/oe
/endash/emdash/quotedblleft/quotedblright/quoteleft/quoteright/divide/lozenge
/ydieresis/Ydieresis/fraction/currency/guilsinglleft/guilsinglright/fi/fl
/daggerdbl/periodcentered/quotesinglbase/quotedblbase/perthousand/Acircumflex/Ecircumflex/Aacute
/Edieresis/Egrave/Iacute/Icircumflex/Idieresis/Igrave/Oacute/Ocircumflex
/apple/Ograve/Uacute/Ucircumflex/Ugrave/dotlessi/circumflex/tilde
/macron/breve/dotaccent/ring/cedilla/hungarumlaut/ogonek/caron
macvec 128 128 getinterval astore pop
{}mark true/Courier/|______Courier 0 rf
{/Metrics 21 dict begin/zero 600 def/one 600 def/two 600 def/three 600 def/four 600 def/five 600 def/six 600 def/seven 600 def/eight 600 def
/nine 600 def/comma 600 def/period 600 def/dollar 600 def/numbersign 600 def/percent 600 def/plus 600 def/hyphen 600 def/E 600 def/parenleft 600 def/parenright 600 def/space 600 def
currentdict end def currentdict/UniqueID known{/UniqueID 16#800000 def}if/FontBBox FontBBox 4 array astore def}mark true/Helvetica/|______Seattle 1 rf
/oldsettransfer/settransfer load def
/concatprocs{/proc2 exch cvlit def/proc1 exch cvlit def/newproc proc1 length proc2 length add array def
newproc 0 proc1 putinterval newproc proc1 length proc2 putinterval newproc cvx}def
/settransfer{currenttransfer concatprocs oldsettransfer}def
/PaintBlack{{1 exch sub}settransfer gsave newpath clippath 1 setgray fill grestore}def
/od{(Rvd\001\001\000\000\177) fg copy pop txpose
1 0 mtx defaultmatrix dtransform exch atan/pa exch def
newpath clippath mark
{transform{itransform moveto}}{transform{itransform lineto}}
{6 -2 roll transform 6 -2 roll transform 6 -2 roll transform
{itransform 6 2 roll itransform 6 2 roll itransform 6 2 roll curveto}}
{{closepath}}pathforall newpath counttomark array astore/gc xdf pop ct 39 0 put
10 fz 0 fs 2 F/|______Courier fnt invertflag{PaintBlack}if}bdf
/cd{}bdf
/op{/sfl false def/pm save def}bdf

/cp {
	not
	{
		userdict /#copies 0 put
	} if

	ma 0 gt
	{
		{
			t1 the
			{exit} if
		} loop
	} if

	{copypage}
	{showpage}
	ifelse

	/laserprep_cp_used	true	def	

	pm restore
} bdf

/px{0 3 1 roll tp tt}bdf
/psb{/us save def}bdf
/pse{us restore}bdf
/ct 40 string def
/nc{currentpoint initclip newpath gc{dup type dup/arraytype eq exch/packedarraytype eq or{exec}if}
forall clip newpath moveto}def
/kp{ct 0 2 index length 2 index 39 2 index put getinterval copy cvx exec mx3 currentmatrix pop}bdf
/av 68 def
end
LW 1 eq userdict/a4small known not and{/a4small
[[300 72 div 0 0 -300 72 div -120 3381]
280 3255
{statusdict/jobstate (printing) put 0 setblink
margins
exch 196 add exch 304 add 8 div round cvi frametoroket
statusdict/jobstate (busy) put
1 setblink}
/framedevice load
60 45{dup mul exch dup mul add 1.0 exch sub}/setscreen load
{}/settransfer load/initgraphics load/erasepage load]cvx
statusdict begin bind end readonly def}if
md begin/bigs[lnop userdict/letter known{/letter load}{lnop}ifelse userdict/legal known{/legal load}{lnop}ifelse userdict/a4 known{/a4 load}{lnop}ifelse userdict/b5 known{/b5 load}{lnop}ifelse 
lnop lnop lnop /tab load]def
/smalls[lnop userdict/lettersmall known{/lettersmall load}{userdict/note known{/note load}{lnop}ifelse}ifelse
userdict/legal known{/legal load}{lnop}ifelse userdict/a4small known{/a4small load}{lnop}ifelse 
userdict/b5 known{/b5 load}{userdict/note known{/note load}{lnop}ifelse}ifelse lnop lnop lnop /tab load]def end
systemdict/currentpacking known{setpacking}if
%-lmm- currentfile ok userdict/stretch known not and{eexec}{flushfile}ifelse
%-lmm- 0000000000000000000000000000000000000000000000000000000000000000
%-lmm- 0000000000000000000000000000000000000000000000000000000000000000
%-lmm- 0000000000000000000000000000000000000000000000000000000000000000
%-lmm- 0000000000000000000000000000000000000000000000000000000000000000
%-lmm- 0000000000000000000000000000000000000000000000000000000000000000
%-lmm- 0000000000000000000000000000000000000000000000000000000000000000
%-lmm- 0000000000000000000000000000000000000000000000000000000000000000
%-lmm- cleartomark
%-lmm- currentfile ok userdict/smooth4 known not and{eexec}{flushfile}ifelse
%-lmm- 0000000000000000000000000000000000000000000000000000000000000000
%-lmm- 0000000000000000000000000000000000000000000000000000000000000000
%-lmm- 0000000000000000000000000000000000000000000000000000000000000000
%-lmm- 0000000000000000000000000000000000000000000000000000000000000000
%-lmm- 0000000000000000000000000000000000000000000000000000000000000000
%-lmm- 0000000000000000000000000000000000000000000000000000000000000000
%-lmm- 0000000000000000000000000000000000000000000000000000000000000000
%-lmm- cleartomark
%%EndProcSet
%%EOF
