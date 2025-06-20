/* This shell sort is basically an insertion sort which uses   a gap of h instead of a gap of 1. In fact, this version   of shell sort was created by a straight forward hack   on the code for the insertion sort (I later continued   hacking to get better optimization).      Based on information in "Algorithms", by Robert Sedgewick,   2nd ed., Addison-Wesley, 1988. */#include <stdlib.h>#include <string.h>#include "sort_private.h"static void shsort1(void *base, size_t nmemb, size_t size,		    int (*compar)(const void *, const void *), void *v);static void shsort2(void *base, size_t nmemb, size_t size,		    int (*compar)(const void *, const void *));/* Call shsort just like you would the standard library routine qsort.   First, shsort tries to allocate a temporary variable; if this is   successful, then it uses the function shsort1, which performs about 1/3   of the data moves of shsort2. Otherwise, it uses shsort2, which uses   the swap function.   The code for both shsort1 and shsort2 has been optimized to use   pointers, instead of indirectly accessing the elements of the array   using a multiplication. This, unfortunately, resulted in slightly more   obscure code. */void shsort(void *base, size_t n, size_t size,	    int (*compar)(const void *, const void *)){  void *v;    v = SORT_MALLOC(size);  if (v) {    shsort1(base, n, size, compar, v);    SORT_FREE(v);  }  else    shsort2(base, n, size, compar);}static void shsort1(void *base, size_t n, size_t size,		    int (*compar)(const void *, const void *), void *v){  size_t h;     /* Distance between each two items. */  size_t gap;   /* Gap in bytes between each two items (equal to h*size) */  void *i;      /* Index for middle loop. */  char *j;	/* Index for inner loop. */  void *first;	/* Second item in first file; serves as a sentinel for j. */  void *last;	/* Last item in array; serves as a sentinel for i. */    last = get(n);  for (h = 0; h < n; h = 3 * h + 1)    ;  for (h /= 3; h > 0; h /= 3) {    gap = h * size;    for (i = first = get(h); i < last; i = succ(i)) {      COPY(v, i, size);      for (j = i; (void *) j >= first && COMPARE(j - gap, v) > 0; j -= gap)	COPY(j, j - gap, size);      COPY(j, v, size);    }  }}static void shsort2(void *base, size_t n, size_t size,		    int (*compar)(const void *, const void *)){  size_t h;	/* Distance between each two items. */  size_t gap;	/* Gap in bytes between each two items (equal to h*size) */  void *i;	/* Index for middle loop. */  char *j;	/* Index for inner loop. */  void *first;	/* Second item in first file; serves as a sentinel for j. */  void *last;	/* Last item in array; serves as a sentinel for i. */    last = get(n);  for (h = 0; h < n; h = 3 * h + 1)    ;  for (h /= 3; h > 0; h /= 3) {    gap = h * size;    for (i = first = get(h); i < last; i = succ(i))      for (j = i; (void *) j >= first && COMPARE(j - gap, j) > 0; j -= gap)	SWAP(j, j - gap, size);  }}   NV��/UO?<��Hn��B�B��` JgT0.��S@gU@g4U@g0S@g@U@g<_@g8`6UO/.��Hn��,06 C f Hn��/.�쩳` .���   �r��fp�`p &N^Nu�SpinDefault  NV��H�0$n B.��B�N�   X6 XOg0`  �G�  $Hn��/Hn��/. N�   h6 C��O� f` m��N�J@gv`@J.��g�`JCf4J���g
�������f  JC��� �f� Jp�R�Jf�S�B2 v `6.��N�   `0L�N^Nu�LowAddrToName  NV   n ���N^ _PONЌMYRESULTPROC   NV��/Hx $Hn��N�  �=| ��=m����A��ܤ6 JCPOg0`( m��N�J@gp`Jn��n�6.��g0`
 n  ���0&N^Nu�LowGetMyIP   NV  /
$n Hx :/
N�  �5|  5m�� PO$_N^Nu�InitUDPPB  NV��H�0$n v  <  ��&HUO>� 06 g0`jHn��N���=| ��-K��-|  ���A�  �-H�� 
XOfp `0=@��-n ��A��Ƥ6 ` m��N�Jn��n� n  ��� 
gJRf4���6.��0L�N^Nu�LowUDPCreate   NV��/v Hn��N��=| ��-n ��-n ��=n ��-n ��B.��A��Ƥ6 XOg0` m��N�J@gp`
Jn��n�0.��&N^Nu�LowUDPWrite  NV��/v Hn��N���=| ��-n ��A��Ƥ6 XOg
0`& m��N�Jn��n�Jn��g0.��` n��UO>� 0&N^Nu�LowUDPRelease  NV��/v Hn��N��<=| ��-n ��-n ��A��Ƥ6 XOg0`  m��N�J@gp`Jn��n� n 0���0.��&N^Nu�LowUDPMTU  NV  /
$n Hx f/
N�  �5|  5m�� PO$_N^Nu�InitTCPPB  NV��/v  <  ��-H��UO>� 06 g0`XHn��N���=| ��-n����-|  ���A�  �-H��-n ��A����6 JCXOg
0` m��N�Jn��n� n  ���6.��0&N^Nu�LowTCPCreate   NV��H�0. &n $n v Hn��N��=| ��-n ��D��| ��|����D��-S��=R�� n =P��A����6 JCXOg0`" m��N�J@gp`Jn��n�6.��&���4���0L�N^Nu�LowTCPPassiveOpen  NV��/v Hn��N���=| "��-n ��n ��| ��|����-n ��A����6 JCXOg0` m��N�J@gp`Jn��n�6.��0&N^Nu�LowTCPSend   NV��/v Hn��N��=| &��-n ��n ��| ��|����A����6 JCXOg
0` m��N�Jn��n�6.��0&N^Nu�LowTCPClose  NV��/v Hn��N���=| *��-n ��A����6 JCXOg
0`. m��N�Jn��n�6.��g0`-n���� n���UO>� 06 0&N^Nu�LowTCPRelease  NV  H� *. $n x�v `p�ú� f8`RCC  m�JDm0. S@g`
p��� L�8N^ _O� NЏMYUDPNOTIFYPROC  NV  H� *. $n x�v `p�ú� f8`RCC  m�JDm0. Y@g`
p��� L�8N^ _O� NЏMYTCPNOTIFYPROC  NV��N�  ��c�6�6�6�6N�  J@g?< �ȩ�N�  �B-��`"UO?<��Hn��HxB��`gHn��N�  2XOJ-��g�N�  TN^Nu�main   NV  Hm�ʨn����0��B��{�PN^Nu�InitToolBox  NV  /
YO?< ��� _$H 
f?< �ȩ�/
�< J�#YO?< ��I _$H/
/<DRVR�M�7$_N^Nu�MenuBarInit  NV��/
$n 0S@g
U@gU@g`</
N�  �XO`0 * �   �@��0* @ gYO.��H�? �> / N�  XO$_N^Nu�DoEvent  NV��H� $n UO/* 
Hn���,06 0S@g
S@gU@g$`0YO/* 
�= & /N�  XO`/
/.����`/.��/* 
Hm�V�%L�N^Nu�HandleMouseDown  NV� H� (. J�gj*r�60@ �g
S@g:U@gD`L0S@g`N�  �`<YO?< ��I _$H/
?Hn� �FUOHn� ��06 `C f| ��`
?N�  �TOBg�8L�8N^Nu�HandleMenuChoice   NV�6/
Bn��0. S@g
S@g R` &BgHn��N�  �J@\Ol
?< ��` 
A���p�R�Jf�@��YO?< �B�Hx���| _$H f
?< ��` �/
�s/
?< Hn��Hn��Hn����/.��Hn����/
N�  "XO`B�Hn����n ��gn ��f�n ��f/
��` �Bn��/
?< Hn��Hn��Hn����/.��Hn����/
��.��H�R@A���B0  Hn�6Hn��N�  �?< �B�Hx���| _$H XOf
?< ��`  /
�s/
?< Hn��Hn��Hn����/.��Hn�6��/
N�  "XO`B�Hn����n ��f�/
��`  �Hn��N�  �J@XOlA���C��� �f�`A���p�R�Jf�@��Hn��N�  @J@XOl
?< ��`  �A���p�R�Jf�@��YO?< �B�Hx���| _$H f?< ��``/
�s/
?< Hn��Hn��Hn����/.��Hn����/
?< Hn��Hn��Hn����/.��Hn����/
N�  "XO`B�Hn����n ��f�/
��$_N^Nu�HandleConnection   NV��H�8(. $n v?< B�N�  �: JE\OlHm��/
N�   @PO` �?< 
B�/N�   H=@��BgHn��?N�  �J@O� gHm��/
N�   @PO` FYO?< �B�Hx���| _&H fHm��/
N�   @PO` /N�  "Hy  tN�  0POB�Hn��?N�  �6 R@O� 
fHm��/
N�   @/��PO`  �Hn��N�   0Hn��N�   P(H Lp�R�Jf�? /?N�  	&A� "L �f�A�p�R�Jf��?N�  
�Hn��/.��N�  LA���p�R�Jf�@��?.��Hm��/N�   @/?< Hn��Hn��Hn����/.��Hn����/?< Hn��Hn��Hn����/.��/��/?< Hn��Hn��Hn����/.��/
��O� $` ��L�8N^Nu�DoServe  NV��/
Bn��YO?< �B�Hx���| _$H 
f?< �ȩ�/
�s/
N�  "XO`B�Hn����n ��f�/
��$_N^Nu�DoAbout  NV��/. �s/. ?< Hn��Hn��Hn����/<  ��Hn��/<������Hn��/<  ��N^Nu�DrawDefault  NV��/UO?<��Hn��Hx B��` UOHn��g$UOHn��Hn��Hn�ꩀgn ��fN�  T��Jg  �0.��S@gU@gDU@g@@ 
gZ`pUO/.��Hn��,06 0U@gU@g`THn��/.�쩳`H/.��/.��Hm�V�%`8 .���   �W�gQ�U�g`"N�  T��`p����S�fYO�$ _/N���XOp &N^Nu�DoMySpin     �  �����%d.%d.%d.%dA�.I P�No DNS Namesocket e r�or
bind e r�orDLOG resource e r�or a c�ept e r�or%d@    (    ( ���8x'�(F(�B�tA�PTZ TIMEZONE :EST:EDT:+03 0D��A�%C� ; Z x � � � �0N%a %c
A�Day Mon  d�  h�: m�: s� "y�
B��A��A�vA�hB��A�j:Jan:January:Feb:February:Mar:March:Apr:April:May:May:Jun:June:Jul:July:Aug:August:Sep:September:Oct:October:Nov:November:Dec:�DecemberA�|%b %D %H:%M:%S %Y|%b %D %Y|%H:%M:%SA�:Sun:Sunday:Mon:Monday:Tue:Tuesday:Wed:Wednesday:Thu:Thursday:Fri:Friday:Sat:Saturday :AM:PMC�!  
#	%+@�0123456789abcdefghijklmnopqrstuvwxyzA>0A> A�hlLA�    A� +-#0 0123456789ABCDEFA�0123456789abcdefC�H}�H{�B�� ���������	�
���������������������� �!�"�#�$�%�&�'�(�)�*�+�,�-�.�/�0�1�2�3�4�5�6�7�8�9�:�;�<�=�>�?��@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]�^�_�`�a�b�c�d�e�f�g�h�i�j�k�l�m�n�o�p�q�r�s�t�u�v�w�x�y�z�{�|�}�~�������������������������������������������������������������������������������������������������࿿����������������������������������������������������������������!��������������������������������������������������������������A�L 	
 !"#$%&'()*+,-./0123456789:;<=>?@abcdefghijklmnopqrstuvwxyz[\]^_`abcdefghijklmnopqrstuvwxyz{|}�~��������������������������������������������������������������������������ʈ�� χ�������� إ����������䉐���������𘜞������������A�P'�#@0��-(!%$2$$2"��%7, , (�"�&�))�"�'@    ���������@��A"@�           C�C�@v��F&����@Q�   A@n��B����@L���������@��@C@M����@����@G@h@d��@ٳA�����@U�����A �����@k��������@b@��      )� ( 	   (  )
pBgQ���/o :  O�  o 60�  o 2O� >>�N�"_0Jg�F`�F.�N�NV�� O1n  �=@ 
N^ _T�N�NV�� O!n 
 1n  �=@ N^"_\�N�"_ _f�`�>�N�"_ _f�`�>�N�"_ _fp�``p�`>�N�"_ _f�
`�
>�N�NV��/-n ��U�Hn��p  N���> n 0��� n  ���=G ..��N^ _O� NЇHGETVOL  NV��-n ��=n ��-n ��U�Hn��p  N��J=_ N^ _O� 
NЇHSETVOL  NV��/=n ��-n ��-n ��B.��n ��p -@��U�Hn��p  N��6> n 0���=G ..��N^ _O� NЇHOPENRF  NV��/=n ��Bn��p -@��U�Hn��p  N���> n 0��� n  ��� n  ���=G ..��N^ _O� NЉGETWDINFO  NV��H�. (n 
*. <. x Zm40<���F-H��Y�?<�p N��� .�谟gU�?//�=_ `  �=|����U�?//Hn��N���>fU�?.��N���>JGgG��f  �U�p / Hn��Hn��N��,>fzU�?.��Hn��Hn��Hn��N���>f`U�p / ?/N��H>fU� �U/ p ? ��=_��U���>p����fU�p / ?.��/.��N��8`U�p / ?.��N��J8JGf>1�
`=n�� L����N^ _O� NАHOPENRESFILEGLUE   NV�|H�&n (n :. ,. BU�/<foldHn�|N��>f .�|   gxJgU�?/. //p �#=_ `  ��macsgB�tempg:�prefg2�extng*�ctrlg"�amnug�strtg�prntg
=|�� `  �p -@��E� f&=x
X��Bn��p-@��C��� Ip�`> fp=n����`=E��p-@��A���-H��C��� Ip0�`> G��gJGf: <   ����f.Bn��p -@��-@��C��� I�> fJ���g
8���&���`
~�`JGf~�=G L���dN^ _O� NЊFINDFOLDER   Jx�k"0< ��F"H0< ��F��g"_ _0/	��?@ Nu o 0< "H` 0< �@S@BXQ���2�  x�3|�� ( � 	gBJ( n 3|�� Jx�k03|  Jx"j$3|  `Bi (  ng3|  `3|  Bi 8 /n
8/R@@ Jx�k8 "g|  x?��b|  	8A��[2< � W���RA3A 
J8�k8�  f
 x�h  3x Jx�mDBi 2< <BgQ��� O1x
X p�`f$1h 4 �f!h Z 0!|ERIK p�`f3h  O� z I"_\O>��N�NV   <  ���F/ <  ���F��g . ��"n "�`&A� 60<�Q". ��gJ�g`�C�  ��N�"n "�B@=@ N^ _P�N�0<�R`�vers   `mach   dsysv   �proc   �fpu    �qd     �kbd   atlk  Bmmu   dram   �lram  �        p`�"x�pi u giv fR@`8�\�` �`p 08Z` �Vp 8/R@` �J8 /g88 "g4 O�  �'0.H@g@?g@?8g@8gp `p`
p`p`p ` � x?��n0<���F$ <  ��F <   ��f`
p ` <   ` ��8A� "Hg �Ҳ f��� ` ��	 p J8�k8�  f x�( ` ��8 /mp 8�  g  mS@`p ` �`0<���F$ <  ���F 8��g
Y�?< �� ` �:"_ .�g @p `� Q���BN�YO// N��� _ NuH�0&o  Kp�R�Jf�&  y   p$P`4///
N�  J@O� f2 =8 f
 R� J��` Jp�R�Jf�R���Jfȑ�L�NuYOA� �J� g o  � XONu o "o "/  J�g�S�f� @�� NuH�0&o E� X�/
// /Hy   8N�  �6 JCO� mB30 0L�NuH�8O���$oL/P.�/j  ?j  &j v | Jj "l5|  "`Jj "f gg Gf5|  "j  "o5|  "?j " &H�N�  � , E g  E  G  e 
 f  g Ro &B/ $`|  $Ho 
// 
// 
?/ Ho 2N�  (�B/ Ho ;Ho  Ho >N�  (�A� Gp�R�Jf�@ F gO� g Gf  �z ~p / (I� (��`H�@�����E�@S�H��� 
 eg +g -f� -fDEE��m\�j "lV|  $0* "�ES@?@ &Ho 
// 
// 
?/ Ho 2N�  (�B/ Ho ;Ho  Ho >N�  (�A� Gp�R�Jf�@ FO� /   )W�D H�R@: `  �A� (0 eP g0 .P f|A� (�P `r*  )f& Gg gf`S�SC+ 0��g�+ .��fS�SCJj "f*  )g� .RC Gg
 Egpe`pE�p / (�EU@fREA� (�P � 0TCRCREp / (�@o �`*  )fH Gg gf</  $f4JFg0+ .��fS�SC`"+ 0��f`S�SC+ 0��g�+ .��fS�SCJj "gj  "f*  )g/  $f
JFf� .RC5C  j B00 0* (@ @ f0* &�C�j 5@ Jj lBj O�(L��NuH�0O���$o D/ H XgA�  �`A�  �&H og xg Xfp`p
`p8 v* dg ifJ�lD�J�fJj "fD f0D "N�  FSCA� � 0 0D "N�  �$�`40D//Ho N�  �/o  /o  $�  / SCA� � 0 O� J�oJCn�D f*  )gA� 0 00 gSC� 00 0Cp��5@ 0j " j C� C�0  J�g�S�f�0* �j "l0* "�j 5@ j�� (`.Jj "l(0* (@ @ f0* &�j �j �j 6 J@o5C O� (L�xNuH�0O���$o �?y  z @/o � N/o � RBo hB/ CHo @Hx�/
Ho ON�  >6 J@O� n Jp�R�Jf�6 / % Cg0`0S@8 J@o&0D//
// Z o ZN�/H ^ O� fp�` ��o h0C��/ % CgJ/ Cf�0/ h` xBo dBo bBo `Bo ^Bo \Bo ZBo l`A�  �"K�� 	ЀA�  �00 �o lR�H�? Hy  �N�  �&H \Of� *f(T� � o �?h�� jJo jl0/ jD@?@ j o  lR�`8Bo j`o� jlH�r
�� j�@A��?A jR�p  y  L0  @  f� .g?|�� f`RR� *fT� � o �?h�� fR�`8Bo f`o� flH�r
�� f�@A��?A fR�p  y  L0  @  f�H�? Hy  �N�  � \Of.�p `H�@ nHo Ho �Ho NN�  �J@O� fHo Ho HN�  �J@POl �
p�O� pL�NuH�:O���$o .(o 2/ 6&o 8H�N�  �� % x  %l E� G� X c . d x e� f� g~ i h n* o � pd s� u � x �* l *gT� T0* Rj ���  ` :T� T6(��C B/ 5|�� "Ho /
N�  xJ@POl p�` * l *gT� T2h�� 	`X� T (��$�* h *f0j $�J�l0* Rj � -  `.*  )g0* Rj � +  `*   )g0* Rj �     K�� %H /
N�  \O` �* l *gT� T2h�� 	`X� T (��$�* h *f
p 0* $�`J* *fp 0* $�*  )g*J�g& xg Xf0* Rj � 0  0* Rj �   K�� %H /
N�  \O` �* L *g*p
є T/h�� 
/h�� ?h�� .� 
/o  ?o  `p
є T.���/h�� ?h�� $�%o  5o   g0* Rj � -  `.*  )g0* Rj � +  `*   )g0* Rj �     K�� %H /
N�  	 \O` D* h *fX� T h��0� $` ,* l *gX� T h��0� $` 0j $X�"T"i��"�` X� T$��� K�� %H < x/
N�  \O`  �* l *g  �X� T%h�� *  )g8Jj "m j p �j "m0* "` j p ` j p 5@ R� `  �Jj "m.0j "/Bg/* N�  �,H O� 
f0j " ` N��  ` j p�R�Jf�5@ `BX� T/(��/
N�  xJ@POl,p�`*0* Rj � %  `Jfp%`H�2* Rj � p O� L�\NuH�8O���$o j(o nJj "m0* "`0<�: 9  DH��j &H�>�  x0* (@ fp@��bG� 
x@`/N�  �&H XOfp�`  �| `vHWHo N�  �6 J@O� 
n|�`^Jg0`SC0�@mN0* �CH���dBj &//
N�  �Bj PO0C/0* A�  "C�  J�g�S�f��j JgR��CJEn�//
N�  �Bj &Bj A� ��POg
/N�  XO0O� JL�xNuH� $o 6* &�j �j �j �j �j �j  0* (@ fPJCoL:`D0Ep ��b0E `p 8 JDo,0D/Hy  h/*  j 
N�%H  O� g�j $`p�` �DJEn�Jj o00j /// /*  j 
N�%H  O� g
0* �j $`p�` �Jj oN:* `D0Ep ��b0E `p 8 JDo,0D/Hy  F/*  j 
N�%H  O� g�j $`p�` |�DJEn�Jj o00j //* /*  j 
N�%H  O� g
0* �j $`p�` @Jj oN:* `D0Ep ��b0E `p 8 JDo,0D/Hy  F/*  j 
N�%H  O� g�j $`p�`  �DJEn�Jj o80j / j 0* Hp  /*  j 
N�%H  O� g
0* �j $`p�`  �Jj  oL:*  `B0Ep ��b0E `p 8 JDo*0D/Hy  F/*  j 
N�%H  O� g�j $`p�``�DJEn�0* (@ gNJCoJ8`B0Dp ��b0D `p 6 JCo*0C/Hy  h/*  j 
N�%H  O� g�j $`p�`�CJDn�p L�8NuH� YO&o  g  �"KY�p��bp��g`  �.�J�  tg��  td#y  t #�  t`F&y  t`&k J� g�� b�(K�ӳ�eN��f ѓ"K`J� g
 I�ѱ� b2#k  'I J� g" I�ѱ� fB�  p i  ё i #h  XOL� NuQO / "/ N�  $.� / "N�  �"/ ��/A  o  �!o  PONuH�     L   L   2J
gWH�̖g�#Gf&�aans8)�mgHistory   TEXTMPS  ����    H�  TEXTMPS  ����                  ���   �  ~ (n U-@��/N����fwingf<(��(z�R/ T P"h�N�z_hTg�H�/ Z_PZ'm�[L���%��\&n (n /<----(�J P�g8QX� (   H 	Monaco                             * m= L L���   /   /           L   L   2 �o�     2  MPSR   
���     �n�H�@ l*H�H��A�  ,p  gRFF�dp Ѐ86 f3� $  |p�`p:E �M0@� g0@ �H�A� �@0@ gH�@ �H��I�@0@@ g
JgR�S�| 0@  g �ZJ�  g o  ��E  K�� L�X�NuH�:QO(o 0&o 46/ 8$L`R�p  y  L0  @ Df� -g
 +gp+`H� JCmC gC $o g&�p ` "JCo C f< 0f6* x g* X f&T�`" 0gv
`* x g* X fvT�`v.
`R� 0g�x /J `&.�A�   "N��*	0C "N�  �H�H�Ҁ(R�0C/p  y  Hr 0  ?Hy   N�  �,H O� 
f���f
 g&�p `f J�� C�  �10 H�H���.J�m:J�n(H�H�"����eH�H�"��0C "N�  ذ�g3� "  |$Lx�|+ -fD� g&� POL�\�NuH�8UO(o &/ *$o , f4�  l y  0@ `  �* ~ BW| H�@ l>H�H��A�  �&p  g(9  DH��FoRWW�dH�@ �r 2 ҁ63 f3� $  |p�`v:E �M0@� g0@ �H�A� �  0 MMCCH�  Sort Library, Version 2.0, Copyright (c) 1990-1994, Ari HalberstadtCompiled on Dec 01 1994 at 14:53:55Hardware:          Macintosh Quadra 950System:            System 7.1.1Compiler:          MetroWerkssizeof(data_type)= 4Preparing data	Allocating arrays	Creating 8196 random integers	Presorting data	Reversing dataRunning tests. If you see any failure messages then it's timeto fix something.It will take at least 165 seconds to run