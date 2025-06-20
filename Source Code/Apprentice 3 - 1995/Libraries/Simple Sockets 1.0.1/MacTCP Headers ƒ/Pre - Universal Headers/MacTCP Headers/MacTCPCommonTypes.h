/* 	MacTCPCommonTypes.h  	C type definitions used throughout MacTCP.						    Copyright Apple Computer, Inc. 1988-91     All rights reserved	*/#ifndef __TYPES__#include <Types.h>#endif /* __TYPES__ *//* MacTCP return Codes in the range -23000 through -23049 */#define inProgress				1				/* I/O in progress */#define ipBadLapErr				-23000			/* bad network configuration */#define ipBadCnfgErr			-23001			/* bad IP configuration error */#define ipNoCnfgErr				-23002			/* missing IP or LAP configuration error */#define ipLoadErr				-23003			/* error in MacTCP load */#define ipBadAddr				-23004			/* error in getting address */#define connectionClosing		-23005			/* connection is closing */#define invalidLength			-23006#define connectionExists		-23007			/* request conflicts with existing connection */#define connectionDoesntExist	-23008			/* connection does not exist */#define insufficientResources	-23009			/* insufficient resources to perform request */#define invalidStreamPtr		-23010#define streamAlreadyOpen		-23011#define connectionTerminated	-23012#define invalidBufPtr			-23013#define invalidRDS				-23014#define invalidWDS				-23014#define openFailed				-23015#define commandTimeout			-23016#define duplicateSocket			-23017/* Error codes from internal IP functions */#define ipDontFragErr			-23032			/* Packet too large to send w/o fragmenting */#define ipDestDeadErr			-23033			/* destination not responding */#define icmpEchoTimeoutErr 		-23035			/* ICMP echo timed-out */#define ipNoFragMemErr			-23036			/* no memory to send fragmented pkt */#define ipRouteErr				-23037			/* can't route packet off-net */#define nameSyntaxErr 			-23041		#define cacheFault				-23042#define noResultProc			-23043#define noNameServer			-23044#define authNameErr				-23045#define noAnsErr				-23046#define dnrErr					-23047#define	outOfMemory				-23048#define BYTES_16WORD   			2				/* bytes per 16 bit ip word */#define BYTES_32WORD    		4				/* bytes per 32 bit ip word */#define BYTES_64WORD    		8				/* bytes per 64 bit ip word */typedef unsigned char b_8;				/* 8-bit quantity */typedef unsigned short b_16;			/* 16-bit quantity */typedef unsigned long b_32;				/* 32-bit quantity */typedef b_32 ip_addr;					/* IP address is 32-bits */typedef struct ip_addrbytes {	union {		b_32 addr;		char byte[4];		} a;	} ip_addrbytes;	typedef struct wdsEntry {	unsigned short	length;						/* length of buffer */	char *	ptr;						/* pointer to buffer */	} wdsEntry;typedef struct rdsEntry {	unsigned short	length;						/* length of buffer */	char *	ptr;						/* pointer to buffer */	} rdsEntry;typedef unsigned long BufferPtr;typedef unsigned long StreamPtr;typedef enum ICMPMsgType {	netUnreach, hostUnreach, protocolUnreach, portUnreach, fragReqd,	sourceRouteFailed, timeExceeded, parmProblem, missingOption,	lastICMPMsgType = 32767	} ICMPMsgType;	typedef b_16 ip_port;typedef struct ICMPReport {	StreamPtr streamPtr;	ip_addr localHost;	ip_port localPort;	ip_addr remoteHost;	ip_port remotePort;	enum ICMPMsgType reportType;	unsigned short optionalAddlInfo;	unsigned long optionalAddlInfoPtr;	} ICMPReport;	typedef OSErr (*OSErrProcPtr)();typedef Ptr (*PtrProcPtr)();typedef Boolean (*BooleanProcPtr)();typedef void (*voidProcPtr)(); n� I1| : 2!o  $!o   1|�� "/ ��
f�`!A C� X!I �J@ga"_O� N� x
��
g"P!� 4<�#!�

�NuNV��A���1|�� �Bx�a�N^Nua� 8<o� @N�H��� x
� PJ( 9gP� 8`h0( 2"( g"AH瀀N�L�@ :fN���`F"HA� :#X #X $#X  3|�� rF�Ag2 H�"H�� �.A :`r: x
� P1A 2C���!I �L�Nu"_ _0�1R@�N�"_0Jg�F`�F.�N�" "_ _�."AN�"_ _�%.�jB�N�"_ �".�N�"_ �".�N�"_ �.�N�"_ �.�N�+|  ���B-��B-��B���B���B���B���| ��B���B���Nu/&/ J�m ` D�/@ &.�NuH�0O���</,E�.X�x ` rX�&*�� �   b �@0; N� �  ^ � �,�X�&j��X�&*��J�o(/Hm�Ho N�  /Ho Ho$N�  O� ` �/Hm� HoN�  O� ` �T�:*��X�&*���   �fB/` �J�o??Hm�$HoN�  O� ` b?Hm�(HoN�  O� 
` JX�**��X�&*��J�o/?Hm�+HoN�  O� `  /Hm�0HoN�  O� ` T�:*���EX�&*��JEgHm�5HoN�  "4PO`  �Hm�:HoN�  "4PO`  �X�&j��X�&*��X�**��J�l"/+ /+ ?+  Hm�AHoN�  O� `  �J�l/Hm�FHo N�  O� `//Hm�MHo N�  O� /+ /+ ?+  Ho HoN�  O� `DX�&j��X�**��X�&*��J�o��n KC� �.A�B08 ` KC� �. S�A�B0 �   �gRHoN�  "N&  Є�   �XOo" R� o.C� A� <   ����.(<   �` R� o.C� A� �.؃SFJFf �� o.�O�L�xNu// // // N�  !��� O� NuH�0&o E� X�/
// /Hy  �N�  T6 JCO� mB30 0L�NuH�8O���$oL/P.�/j  ?j  &j v | Jj "l5|  "`Jj "f gg Gf5|  "?j "&H�N�  T , E g  E  G  e 
 f  g Ro&B/$`| $Ho$Ho Ho ?< ��B/ 
Ho$Ho Ho ,?< �� gg Gf  �z ~p / $I� $��`H�@�����E�@S�H��� 
 eg +g -f� -fDEE��m@�j "l:| $0* "�ES@?@&Ho$Ho Ho ?< ��B/ 
Ho$Ho Ho ,?< ��/   %W�D H�R@: `  �A� $0 eP g0 .P f|A� $�P `r*  )f& Gg gf`S�SC+ 0��g�+ .��fS�SCJj "f*  )g� .RC Gg
 Egpe`pE�p / $�EU@fREA� $�P � 0TCRCREp / $�@o �`*  )fH Gg gf</ $f4JFg0+ .��fS�SC`"+ 0��f`S�SC+ 0��g�+ .��fS�SCJj "gj  "f*  )g/ $f
JFf� .RC5C  j B00 0* (@ @ f0* &�C�j 5@ Jj lBj O�(L��NuH�0O���$o D/ H XgA���`A���&H og xg Xfp`p
`p8 v* dg ifJ�lD�J�fJj "fD f0D "N�  �SCA� � 0 0D "N�  �$�`40D//Ho N�  r/o  /o  $�  / SCA� � 0 O� J�oJCn�D f*  )gA� 0 00 gSC� 00 0Cp��5@ 0j /A� Hp0 /* N�  !�0* �j "O� l0* "�j 5@ j�� (`.Jj "l(0* (@ @ f0* &�j �j �j 6 J@o5C O� (L�xNuH�0O���$o �?m� @/o � N/o � RBo hB/ CHo @Hx�/
Ho ON�  �6 J@O� n/
N�  "N6 XO/ % Cg0`0S@8 J@o&0D//
// Z o ZN�/H ^ O� fp�` ��o h0C��/ % CgJ/ Cf�0/ h` pBo dBo bBo `Bo ^Bo \Bo ZBo l`A��"K�� 	ЀA��00 �o lR�H�? Hm�N�  "&H \Of� *f(T� � o �?h�� jJo jl0/ jD@?@ j o  lR�`8Bo j`o� jlH�r
�� j�@A��?A jR�H� m�Z��00  @  f� .g?|�� f`RR� *fT� � o �?h�� fR�`8Bo f`o� flH�r
�� f�@A��?A fR�H� m�Z��00  @  f�H�? Hm�N�  " \Of.�p `H�@ nHo Ho �Ho NN�  rJ@O� fHo Ho HN�  JJ@POl �p�O� pL�NuH�:O���$o .(o 2/ 6&o 8H�N�  T� % x  %l E� G� X c . d x e� f� g~ i h n* o � pd s� u � x �* l *gT� T0* Rj ���  ` :T� T6(��C B/ 5|�� "Ho /
N�  >J@POl p�` * l *gT� T2h�� 	`X� T (��$�* h *f0j $�J�l0* Rj � -  `.*  )g0* Rj � +  `*   )g0* Rj �     K�� %H /
N�  �\O` �* l *gT� T2h�� 	`X� T (��$�* h *f
p 0* $�`J* *fp 0* $�*  )g*J�g& xg Xf0* Rj � 0  0* Rj �   K�� %H /
N�  �\O` �* L *g*p
є T/h�� 
/h�� ?h�� .� 
/o  ?o  `p
є T.���/h�� ?h�� $�%o  5o   g0* Rj � -  `.*  )g0* Rj � +  `*   )g0* Rj �     K�� %H /
N�  <\O` D* h *fX� T h��0� $` ,* l *gX� T h��0� $` 0j $X�"T"i��"�` X� T$��� K�� %H < x/
N�  �\O`  �* l *g  �X� T%h�� *  )g8Jj "m j p �j "m0* "` j p ` j p 5@ R� `  �Jj "m.0j "/Bg/* N�  !�,H O� 
f0j " ` N��  `/* N�  "NXO5@ `BX� T/(��/
N�  >J@POl,p�`*0* Rj � %  `Jfp%`H�2* Rj � p O� L�\NuH�8O���$o j(o nJj "m0* "`0<�: -�`H��j &H�>��0* (@ fp@��bG� 
x@`/N�  N&H XOfp�`  �| `rHWHo N�   �6 J@O� 
n|�`ZJg0`SC0�@mJ0* �CH���dBj &//
N�  JBj PO0C/Ho 0* Hs  N�  !��j JO� gR��CJEn�//
N�  JBj &Bj A� ��POg
/N�  �XO0O� JL�xNuH� $o 6* &�j �j �j �j �j �j  0* (@ fNJCoJ:`B0Ep ��b0E `p 8 JDo*0D/Hm�/*  j 
N�%H  O� g�j $`p�` ��DJEn�Jj o00j /// /*  j 
N�%H  O� g
0* �j $`p�` �Jj oL:* `B0Ep ��b0E `p 8 JDo*0D/Hm�>/*  j 
N�%H  O� g�j $`p�` v�DJEn�Jj o00j //* /*  j 
N�%H  O� g
0* �j $`p�` :Jj oL:* `B0Ep ��b0E `p 8 JDo*0D/Hm�>/*  j 
N�%H  O� g�j $`p�`  �DJEn�Jj o80j / j 0* Hp  /*  j 
N�%H  O� g
0* �j $`p�`  �Jj  oJ:*  `@0Ep ��b0E `p 8 JDo(0D/Hm�>/*  j 
N�%H  O� g�j $`p�`^�DJEn�0* (@ gLJCoH8`@0Dp ��b0D `p 6 JCo(0C/Hm�/*  j 
N�%H  O� g�j $`p�`�CJDn�p L�8NuH� YO&o  g  �"KY�p��bp��g`  �.�J��g���d#m� +I�`D&m�`&k J� g�� b�(K�ӳ�eL��f ѓ"K`J� g
 I�ѱ� b0#k  'I J� g  I�ѱ� fB�� i  ё i #h  XOL� NuQO / "/ N�  �.� / "N�  f"/ ��/A  o  �!o  PONuH�0(/ $m� 
f.E��X�` R��b J`| RX�$HJ�f�`6 R��b J`f RX�$HJ�f� m�&PE��X�` R��b J`D RX�$H��f�&<  � ��d&/N�  �&H XOf��f��`�`�&�Hk N�  �XO` �lL�NuH�0(/ p�&Z�ƀ��b��`Lp��dv/N�  �&H XOf��`0$S Q���d&� ` J��&� S!j   �� S �$�+K� JX�L�Nu/YO&/ J�g �.� g W`��XO&NuH�(o $./ ("o , f2�� m�b0@ `  �) | &LJ�f�E p�`  �H�@ l(H�H��A��b,p  gRFF�dp Ѐ86 f
;| $�p�`p:E �M0@� g0@ �H�A� �@0@ gH�@ �H��I�@0@@ g
JgR�S�| 0@  g �^J�  g o  ��E  K�� L�X�NuH�8UO(o &/ *$o , f4�� m�b0@ `  �* ~ BW| H�@ l:H�H��A���&p  g&-�`H��FoRWW�dH�@ �r 2 ҁ63 f
;| $�p�`v:E �M0@� g0@ �H�A� �@0@ gH�@ �H��I�@0@  g0@ �fH�`02RF� f~BW0@@ fJGg �DE 0TOL��Nu"/ 
/ 	 o `
�fNuR�S�J�f��Nu/(o  /  L"o `�R�R�S�J�f� L(_NuUO o o  `Jf��TONuR��/ f�TONu/&o "K o R� �f� K&_Nu"o `R�Jf� I��  Nu  �  ������Caught:  Caught:  	TestSoundTime to catch   disks:  	 seconds.  %�#.%lds %#s %*c %c %*ld %ldA�TRUE FALSEA�%10f !%�lde !%�ld.%ldfC�� � � � � � � � � @ @ @ @ @ � � � � � � � � � � � � � � � � � �                 ! ! ! ! ! ! ! ! ! !       �                                                               �{�ZA�� ���������	�
���������������������� �!�"�#�$�%�&�'�(�)�*�+�,�-�.�/�0�1�2�3�4�5�6�7�8�9�:�;�<�=�>�?��@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]�^�_�`�a�b�c�d�e�f�g�h�i�j�k�l�m�n�o�p�q�r�s�t�u�v�w�x�y�z�{�|�}�~�������������������������������������������������������������������������������������������������࿿����������������������������������������������������������������!�������������������������������������������������������������� ��`w�`{�0123456789abcdefA�0123456789ABCDEFA� +-#0A�    A�hlL > A>0A�snd A�snd M�qd  K�PICTA�cicnA�cicnA�cicnA�PICTE�RBltA�MBltA�RBltA�MBltA�RBltA�MBltA�RBltA�MBltA�RBltA�MBltA�RBltA�MBltA�RBltA�MBltA�OK|�Quitz�7Out of memory! Try increasing the memory in 'Get info'.G�Internal e r�or. (No scr e�n?)b� Couldn't load the backdrop PICT!^�Internal e r�or. (No window?)d    (    ( L��n���>�R��2\3�404�88J8�E0E�G\Rt    |Q�~�           @v   1@lG(����������@|�@|����Ac���@J�@��@���A�~�As�@�@l�@T��㎜A��@I��      W, X    X  VFNV  J��Zg?. /-�Z _N�N^ _TONЍSATSOUNDERROR  NV  H�8(n  J�g &T$KBRYO/<  �i %@ YO/N�  � ]�"<  �N�  �"<  �N�  f( J�f  �YO/N�  � ( YOHx�N�  � _,/ _�) F&PBSYO/<  �i '@ v ` K\�� �0 RCC�o� <  䐄rN�  �Є: v ` J\�"K\��0 0 RCi�Eo�/ _�*/ _�#(F$K(<  �v `*0�C J\��0 �  } l0�C J\�2C"��S�� RCC @o�-L L�xN^.�Nu�SDPRELOAD  NV� /
 n C�� p?"�Q���YO/-�`Hn� �� _$H 
J�g/
 _�d/
 _�)YO/
N��� _-H $_N^.�Nu�SDMAKENAMEDSOUNDHANDLE   NV� /
 n C�� p?"�Q���Jm�fN�XJ-�gYOHn� N��j _-H `&YO/-�fHn� �� _$H 
J�g/
 _�d/
 _�)-J $_N^.�Nu�SATGETNAMEDSOUND   NV��H� 8. 0D S��A��J� gzE���4� Bj B� UO0D S��A��/0 Hn���06 JCg?N��dE���4� Bj B� UO0D S��A��/0 Hn���06 JCg?N��.0D S�A��B0 L�N^ _TONЇSILENCE  NV��/ n -P��-h ��J� g&n09��fYO/.��/M *_ & YO//M *_ & &N^ _PONЈCALLBACK   NV  N�  �B-�N^Nu�SDSHUTUP   NV��/
$n  
J�g  �J�g  �UON�  
�fN�  �YO/
N�  � ]�"<  �N�  �"<  �N�  f-@��//.��Hx  N�  
�YO/
N�  � r<N�  f"<  *�N�  �-@��YO�u Ю��+@�| �$_N^.�Nu�SDPLAYIT   NV   n "PQ fpЉ @4p��H�Љ"@pЉ"@4p��H�r҉Ҁ-A N^.�Nu�SNDDATA  NV��/
$n J-�g2UO/
?.��Hn�� <  � 0J@gj��  V�D @ `n�� `j��  V�D @ $_N^.�Nu�QCHANBUSY  NV��H�0$n :. 
6. JCg vJ-�g;E��/
N��z` b0C S��A��J� f2UO0C S��A��Hp ?< /-�Hm `�08 JDg*?N���` UO0C S��A��/0 N��g?N���0C S��A��J� g � 
J�g �J�g �G���6� QBk YO/
N��� _'H UO0C S��A��/0 Hn��B'�08 JDg
?N��B` �pЮ�� @8JDfx n��(   �f  � n��0( 8gW@gJW@g  �`  �YO/
N�  � r<N�  f0D"N�  �-@��YO�u Ю��0C"S��A��!� `  �YO/
N�  � "<   �N�  f0D"N�  �-@��YO�u Ю��0C"S��A��!� `  �YO/
N�  � "<  hN�  f0D"N�  �-@��YO�u Ю��0C"S��A��!� `F?<��N��8`<YO/
N�  � r<N�  f0D"N�  �-@��YO�u Ю��0C"S��A��!� 0C S�A���  0C S�ЀA���1� E���4� 
5|  B� UO0C S��A��/0 Hn��B'�08 L�8N^ _PONІPLAYIT   NV��H� 6. J-�g<�m��^�D -�W�D@��A��UON�  
��.���.��g
=|  
`  �Bn 
`zBn 
8:-�v`h0C S��A��J� f=C 
`T0C S�A��J0 f=C 
`>0C S�A��J0 f&0C S�ЀA���p o0C S�ЀA���80 =C 
RCi�Eo�L� 8N^ _TONЎGETBESTCHANNEL   NV  Bm�&Bm�$Bm�"N^Nu�RESETQ   NV  /Jm�&gdUOp�m�"�� A��(?0N���06 JCgD0-�&S@;@�&YO0-�"R@H�/ Hx �X ;@�"p���"A��(/0 p���"A��(?0?N���&N^Nu�PLAYQ  NV  0-�&R@;@�&YO0-�$R@H�/ Hx �X ;@�$p���$A��(!� 
 p���$A��(1� N^ _\ONЄADDQ   NV��/Jm�n  �UO?< Hn��N�  �06 n��]�D  n ��m(YO <  � -_��-n����.��H�@ \�D @�`B-�C�v`.0C S��A��B� 0C S�A��B0 0C S�A��B0 RCC o�;| �J��f+|   ��B��Zv`0C S�A��B0 RCC o�B-�N��8B��&N^Nu�SATSOUNDINIT   NV  H� J-�gN��`>8-�v`20C S��A��J� gUO0C S��A��/0 < �0: RCi�Do�8-�v`0C S��A��B� RCi�Do�8-�v`0C S�A��B0 RCi�Do�B��L� 8N^Nu�SATSOUNDRESET  NV��/6. J-�gN��X`Z�m�nTC mN0C S��A��J� gUO0C S��A��/0 < �0=@��0C S��A��B� 0C S�A��B0 &N^ _TONЕSATSOUNDSHUTUPCHANNEL  NV  N���N���N^Nu�SATSOUNDSHUTUP   NV  J��gJ-�g/-�?-�N��>n �;n 
�+n �N^ _PONІDLAYIT   NV  H� $n :. 
. J-�f J-�gUON�  
�g`B-�`Z8-�v`N0C S��A��J� f0C S�A��B0 `(UO0C S��A��/0 N��f0C S�A��B0 RCi�Do�UOp�E? N���06 JCfJgx/
?N��l`n0C S�A��J��V�D�0 gLE \�D 0C"S�ҁA���p W�D� f60C S�ЀA���p m/
?N���`Jg/
?N��`
/
?N���L�xN^ _PONЌSATSOUNDPLAY   NV  H� J-�gUON�  
�g`B-�`Z8-�v`N0C S��A��J� f0C S�A��B0 `(UO0C S��A��/0 N���f0C S�A��B0 RCi�Do�YO�u * 8-�v`,0C S�A��J0 g0C S��A�򤺰 o?N��"RCi�Do�J��g<UO?-�N���06 JCg/-�?-�?N���-�W�D JCV�D� gB��Bm�`
Jm�&oN��\L� 8N^Nu�SATSOUNDEVENTS   NV��H�8(n $n &n R f  �YO/* Hx ��X ( 6YO0j /Hx �X J�g8C .f2p�m�^6�/p�m�^? Hn��Hn��Hn����/.��?< �]|  `JC W�D C W�D� g2p�m�`6�/p�m�`? Hn��Hn��Hn����/.��?< �]|  `B. L�N^ _O� NІFILTER   NV  ;| �bBm�dBm�f;| 
�h;| �j;| Z�l;| ��n;|��p;|^0�r;|^1�t;|^2�v;|^3�xBm�zBm�|;| h�~;| ��;| |�;| ��;| �Bm�Bm�;| h�;| �;| |�;| d�;| �Bm�Bm�;| H�;| �;| \�;| d�;| �N^Nu�INITITEMLIST   NV�� n 
C���p?"�Q���/. ?. Hn��Hn��Hn����/.��Hn��_J. g.Hn�樘��/<  ��Hn��/<������Hn��/<  ��Hn�標N^ _O� NЋSETDCONTROL  NV  ?< �� n /(���sJ-��g
 n /(���1,n N�DN^.�Nu�FakeBarf0  NV��H�?88. 6.  n &C���p?"�Q��� n "C���p?"�Q��� n C���p?"�Q��� n C���p?"�Q��� n C���p?"�Q��� n C���p?"�Q��� n C���p?"�Q���;C�`;n �^N���PHn���tJ-��gYO�2 _-H��YO�* _/�1YOHx N�  � _&H J�f/N���/ _�);D�bHm�b/Hx N�  �Hn��/< P s/< �c��YOHx  Hn��Hm�lB'?< Hx��B'B�/�} _$H 
J�f/N���Hn��Hn��Hn��Hn�/
�s/
�0S@g  �S@gRS@g`  �/
?< Hn��C W�D  N���/
?< Hn �MMCCW�D    // test ser_v3.c// Darrell Anderson#include "ser_v3.h"void main(void) {	;}< Hn��C W�D  N��ZHm xHn����n ��f=C��`
0.��S@=@��/ _�*/
��/.���sJ-��g/.���1=n�� *`O���L��N^ _O� "NЌSATFAKEALERT   NV�� n C�� p?"�Q���UOHn� Hm�nHm�pHm�r/<  BgHm��Hm�tHm�vN��r0=@��N^.�Nu�SATREPORTSTR   NV  /UO�a06 JClDC0C2n  "	N�  <=@ 
&N^ _TONЇSATRAND  NV  Hm�ƨn����0��Hx  �{�P�cN^Nu�SATINITTOOLBOX   NV��H�88. :. (n  n -P��-h ��v B�����D n-n ��UOHn��N�  �06 `,YOHx N�  � _&HYO/     T   T   F�/B'0<	�BW/<core/<clonHm�/iperr.c   TEXTMMCC ����    Hn  TEXTMMCC ����                  ��T   �  �Hm�/B'0<	�BW/<core/<deloHm�/B'0<	�BW/<core/<doexHm�/B'0<	�BW/<core/<getdHm�/B'0<	�BW/<core/<dsizHm�/B'0<	�BW/<   H 	Monaco                             � �| � �|��u�                     T   T   F ��r    F MPSR   MWBB   ���        ���   L    �U.� _$H&J J�g  �YOHx N�  � _$H(J J�g  �$SBRBj  D Ph   nBj `5|  %L 5|  
B� B� Bj %D 