     �   �   rP manager equates   (c) CopyriUDPPB.h   TEXTSPM   < `    BO  TEXTSPM   < `                  ��\�  �  typedef OSErr (*LapStatProcPtr)(struct LAPInfo *lap, struct LAPStats *lapStatsPtr);#define LAPINFO	\	b_32		our_ip_addr;	/* LAP    (XB (XB  B                  
 	Geneva    H 	Geneva                             ) �B ( �����  �  �  *     ������  @                    �   �   r(��*�    r MPSR  ETAB   >EFNT   J���        ���   8(�8'��   �(�L���   "    ���   *    ore info.The code was written/compiled using Think C 7.0.3 with both the old headersand the new universal headers.  I took advantage of the fact that in"Mac #includes.c" for the universal headers there is a constant defined asNEW_HEADERS_AVAILABLE.  If you are using the universal headers with adifferent compiler that doesn't define this constant, just add the followingline somewhere: #define NEW_HEADERS_AVAILABLE 1.  If you don't havethe universal headers, you don't have anything to worry about.This is all copyrighted to me (Dave Nebinger).  Feel free to email me withyour questions and comments.  Also drop me a line if you plan to usethe routines (I am just curious ;-)Dave Nebinger.// same as above but use an FSSpec that has already been created.pascal ComponentResult LogSetupFSSpec(ComponentInstance logcomp,FSSpec* fsspecptr)\		ComponentCallNow(kLogSetupFSp,0x04);// add the text pointed to by buffer to the logpascal ComponentResult LogText(ComponentInstance logcomp,char* buffer)\		ComponentCallNow(kLogText,0x04);// add the date, time, and the text pointed to by buffer to the logpascal ComponentResult LogTime(ComponentInstance logcomp,char* buffer)\		ComponentCallNow(kLogTime,0x04);// write a buffer to the log filepascal ComponentResult LogWrite(ComponentInstance logcomp,long* size,Ptr buffer)\		ComponentCallNow(kLogWrite,0x08);// add the pascal string pointed to by buffer to the logpascal ComponentResult LogPText(ComponentInstance logcomp,unsigned char* buffer)\		ComponentCallNow(kLogPText,0x04);// add the date, time, and the pascal string pointed to by buffer to the logpascal ComponentResult LogPTime(ComponentInstance logcomp,unsigned char* buffer)\		ComponentCallNow(kLogPTime,0x04);		#ifdef __cplusplus}#endif#endif,L� �"Nu / /A "/ /_ H�1 N�  L� �"NuJ�jJ�jD�D�N�  D�NuD�N� D�D�NuJ�j
D�N� D�Nu.<  ����c" p Nu��b��H@2 B@H@Nu��b. B@H@��H@HG> HG��0HG2Nu$ &�≲�b�����2��.HG��HG҇e��bD�NuS@`�Nu   �   �  �   �     ?< �� �?< �� �?< �� �?< �� �?< �� ?< ��@?< ��b?< ���?< ��<?< ��j?< ��B?< �� ?< ���?< ��8?< ��D?< ���?< ��  ?< ��   
            ( ( �        8
     � �   . �        ����About List Demo...    -        C �        ����File
New Window N  Close Window W  -    Quit Q       ( ( �        �    x      Z U s � �      
 ! ��Linked List Demo     ( 
 = �copyright 1995, Bridge Software      A 
 Q U�release 1.1   7=`
  CDEF�  H���N� �A��� �UN� �L�N��A�����  6� �UNuH� YO o "o ,/ �X X X *t `D@ �g�H�H�Ԁ`(�X @ @g0�H�@H�Ԁ`X X $��ݱ( S�J�n�XOL� `NuH� $@ ,��&
��g@J�fB,��0<���F/0<���F��V�D @��//
N��D/ N��NO� )J��J,��gp��L�NuA�����  �2 �U��NuNuNV   n  ���!n  !n  )H�� n N^NuNV  /
`$ l��)P��J� g?<��"j  j N�TO�� g$l�� 
f�$_N^NuNV  /
$n  
gY� D�$�f J�$_N^NuNV   <  ���F/ <  ���F��g . ��"n "�`&A� 60<�Q". ��gJ�g`�C�  ��N�"n "�B@=@ N^ _P�N�0<�R`�vers   `mach   dsysv   �proc   �fpu    �qd     �kbd   atlk  Bmmu   dram   �lram  �        p`�"x�pi u giv fR@`8�\�` �`p 08Z` �Vp 8/R@` �J8 /g88 "g4 O�  �'0.H@g@?g@?8g@8gp `p`
p`p`p ` � x?��n0<���F$ <  ��F <   ��f`
p ` <   ` ��8A� "Hg �Ҳ f��� ` ��	 p J8�k8�  f x�( ` ��8 /mp 8�  g  mS@`p ` �`0<���F$ <  ���F 8��g
Y�?< �� ` �:"_ 8 �f�U.�N���.�N�NV��N��>-@��/. ?. /. ?. N��-@�� .����/n�� (N^ _O� N�NV��B���UO/<qdrwHn��N���0=@��Jn��fp����fp `pN^NuNV  /
$H 
g JN�+,Jn o/
N��*XO J$_N^NuNV��-H�� n��Hh  n N�+N^NuNV  N^NuNV  /
$H 
g A�P%H Bg JN� Jn TOo/
N���XO J$_N^NuNV  /
$H 
g A��%H Bg JN��Jn TOo/
N���XO J$_N^NuNV��/
$HA���N�)�Hj A���N�)�Sn��Sn��Hn��Hz-pHn��N��A���N�-HHj (�?.��?.����?.��?.����?.��?.����Hj .�?.��0.��R@? ��?.��?.����0.��R@? ?.����Hn��N���O� $_N^NuNV��/-H��Hn��Hz,�Hn��N��~A���N�,�A���N�)  n��Hh A���N�)Hn��/<  ��J. 
O� gHx �cHn���� n��N�	R6 0g  �S@gS@g  �S@g>`  �?.��?.����0.��S@? 0.��S@? ��0.��S@? ?.����?.��0.��S@? ��`z| ���| ���| ���| ���| "��| "��| "��| "��J. g?<wwN�?<��N�$XOHn��Hn��`&6.���n��HÇ� 0.��T@? ?��0.��W@? ?��Hn��N���XO&N^NuNV��/
$HHj �� JN��J g(A���N�'�Hj A���N�'�Hn��/<  ��Hn����XO$_N^NuNV  /
$HHj ./<?   Hj ?<��N��Hj (/<>���Hj BgN��O� $_N^NuNV��-H��J. g n��Hh �`Hx �cN^NuNV  N^NuNV��-H��A���N�'*Hn�� n��N��Hn����N^NuNV��H� $HA���N�'Hn�� JN�b6.���n��HÇ� 0]@5@ p�j 5@ 5n��  5n�� $>��0W�D H� JTOg0.��T@5@ p�j 5@ `0.��U@5@ p��j 5@ Jg0* V@5@ "5n�� &`0* W@5@ &5n�� "L�N^NuNV��/
$H JN��-H��BgHj  /.��N�� n  �  !j $ O� 
$_N^NuNV�@H� $n 
:. (. | ��B.��B.��Hn��Hn��Hn��?. N�� 
O� gJ�fp ` \ J�i  J�)v p .��g  �S@gZS@g
S@g  �` ,Hn�dHzDHn��N��.��.��.��/
A���N��A� -H��/?A���N��& Hn�dN��O�  `  �Hn�XHz�6Hn��N���.��.��.��/
A���N��A�P-H��/?A���N�x& Hn�XN���O�  `  �Hn�LHz6Hn��N��~.��.��.��/
A���N� �A�0-H��/?A���N�,& Hn�LN��nO�  `HHn�@Hz RHn�pN��4.��.��.��/
A��pN� �A��-H��/?A��pN��& Hn�@N��$O�   J�j L�xN^NuNV  /
$H 
g A��%H Bg JN� Jn TOo/
N�� XO J$_N^NuNV  /
$H 
g A�`%H Bg JN�BJn TOo/
N���XO J$_N^NuNV  /
$H. . . /.  JN� �A�`%H  JO� 
$_N^NuNV  /
$H 
g A�0%H Bg JN���Jn TOo/
N���XO J$_N^NuNV  /
$H. . . /.  JN� ZA��%H A� N�#�A�  N�#z JO� 
$_N^NuNV  /
$H 
g A� %H Bg JN��XJn TOo/
N��XO J$_N^NuNV  /
$HA쁘%H $� n  n  n  5|�� 5|�� 
5|�� Bj Bj Bj  J$_N^NuNV  /
$H 
gA쁘%H Jn o/
N���XO J$_N^NuNV  N^NuNV��H�0$H&. 
x 0. gS@gS@g6Q@gT`h JN� �``-l����=C�� r�=@��/.�� JN�4r  (XO`8YO/N��\ _&H/ J"h �� "i N�XO`&C/ J"h �� "i N�XO L�N^NuNV  /6. 0@  n 
�0@ V�D H� n �0@ V�D H� n �&N^NuNV�n/
$H JN��J g Hn�zHzBHn��N��0A���N�$�A���N�&A���N�&�A���N�%^ JN�� J"h �� "i N� JN��A���N�!~Hn�� JN��Hn�nHz �Hn��N���Hn��A���N��B.��B.��A���N�!HO�  `^Hn��Hz#Hn��N��Hn��A���N�"�*  O� fA���N�#�J* g
A���N�%�`N�&
��.��.�� JN�~Hn��N��xPOHn��Hn��Hn��A���N��J O� f�Hn�zN��RXO$_N^NuNV  /
$H 
gBgA� N�$Jn TOo/
N��`XO J$_N^NuNV  /
$H 
g6BgA� N�$HBgA� N�%�BgA� N�$�Bg JN�#�Jn POo/
N��XO J$_N^NuNV��H�0&Hv YO�� _$H 
g(/
 K"h �� "i N�/. /
�� /
�ٕ�TO`"A���N��Hn�� KN� X/. Hn���� TOL�N^NuNV��/
-H��$n  
gJ�f`A���N��Hn�� n��N� /
Hn����XO$_N^NuNV��-H�� n�� P P"n "� #h  N^NuNV��-H�� n�� P P0( N^NuNV��-H�� n�� P PA� (N^NuNV��-H�� n�� P PJ( V�D H�N^NuNV��-H�� n�� P P( � gp`p N^NuNV��-H�� n�� P Pp ( @ m n�� P Pp ( @ �np`p N^NuNV  /
$HN��J g JN�  JN� � J"h �� "i N�$_N^NuNV��/
$HA���N��/A���N��Hj ?< A���N�~J O� 
fFB���Hn���tA���N�N/.��A���N��Hj BgA���N�JJ O� 
f5|�� 5|�� 
5|�� $_N^NuNV��-H�� n��Hh /<>�ff n��Hh BgN�N^NuNV��-H��Hn��Hz�jHn��N��JB���A���N�P n��HP n��"h �� "i N�Hn��Hz�6Hn��N��B���A���N�A���N�� n��HPA���N��A���N��Hn��N��N^NuNV��H� $H. . 
 JN��4  JN��N  J"h �� $"i  N� J"h �� ,"i (N� JN� �A���N�Hn�� J"h �� 4"i 0N�Hn�� JN� �JO� fJg J"h �� T"i PN� J"h �� L"i HN� J"h �� <"i 8N�J\OgJg J"h �� D"i @N�L�xN^NuNV��-H��J. gHx !�b`*J. g n��Hh �`J. 
g?<��N� .TO`Hx !�bN^NuNV  J. fJ. 
f
/. N�dXON^NuNV��=n ��=n ��=n ��Hn���N^NuNV��=n ��=n ��=n ��Hn���N^NuNV��"n 
 n -n ��B���B���Bn��Hn��Hn��?<��Hn��Hn��?< ��lB���`(-|?�� ��B���Bn��Hn��Hn��?<��l-|?�  ��p 0. -@��Hn��Hn��?<(��Hn��Hn��?<��-n����p 0-@��Hn��Hn��?<(��Hn��Hn��?<��-n����p 0) -@��Hn��Hn��?<(��Hn��Hn��?<��-n����p 0) -@��Hn��Hn��?<(��Hn��Hn��?<��-n����Hn��Hn��?<��Hn��Hn��?<��Hn��Hn��?<��-n����Hn��Hn��?<��Hn��Hn��?<��Hn��Hn��?<��-n����Hn��Hn��?<��Hn��Hn��?<��Hn��Hn��?<��-n����Hn��Hn��?<��Hn��Hn��?<��Hn��Hn��?<��-n����Hn��Hn��?<��Hn��Hn��?<��Hn��Hn��?<��-n����Hn��Hn��?<��Hn��Hn��?<��Hn��Hn��?<��-n����Hn��Hn��?<��Hn��Hn��?< ��Hn��Hn��?<��-n����Hn��Hn��?<��Hn��Hn��?< ��Hn��Hn��?<��-n����Hn��Hn��?<��Hn��Hn��?< ��Hn��Hn��?<��-n����Hn��Hn��?<��Hn��?< ��Hn��Hn��?<(��0���Hn��Hn��?<��Hn��?< ��Hn��Hn��?<(��1n�� Hn��Hn��?<��Hn��?< ��Hn��Hn��?<(��1n�� N^NuNV��H�0&n $n A���N��=j ��=j ��/N� t6 8�j Hĉ� 0C �� 2�@=A��0.���C=@��Hk p / Hn��?. ��XOL�N^NuNV  "n rt  IR�`
 fRAR�0SBJ@f�0N^NuNV��Hn����0.���n��=@��/. N���=@��0.�������n��N^NuNV��| ���| ���| ���| ���| U��| U��| U��| U��Hn����?< ��/. ����N^NuNV��/
$H JN��pJ g �A���N�fHn�� JN���Hn��/<  ��?<DDN�� ?.��0.��U@? ��?.��?.����0.��U@? ?.����?<��N���?.��0.��S@? ��0.��S@? 0.��S@? ��0.��S@? ?.����?<""N���?.��?.����B���?<wwN���0.��S@? ?.����B���?<wwN���?.��0.��S@? ��B���?<��N��r0.��S@? 0.��S@? ��B��� R P-h ��-h ��Hn��/<  ��?<UUN��8?.��0.��U@? ��?.��?.��0.��U@? ?.��?<��N��
?.��0.��S@? ��0.��S@? 0.��S@? ��0.��S@? ?.��?<DDN���?.��?.��B���?<��N���0.��S@? ?.��B���?<��N���?.��0.��S@? ��B���?<��N���0.��S@? 0.��S@? ��B���O� ` � R P-h ��-h ��Hn��/<  ��?<��N��H?.��0.��U@? ��?.��?.�訑0.��U@? ?.�訑?<wwN��?.��0.��S@? ��0.��S@? 0.��S@? ��0.��S@? ?.�訑?<��N���?.��?.�訓B���?<��N���0.��S@? ?.�訓B���?<��N���?.��0.��S@? ��B���?<DDN���0.��S@? 0.��S@? ��B��� R P-h ��-h ��Hn��/<  ��?<��N��`?.��0.��U@? ��?.��?.�਑0.��U@? ?.�਑?<��N��2?.��0.��S@? ��0.��S@? 0.��S@? ��0.��S@? ?.�਑?<��N���?.��?.�ਓB���?<��N���0.��S@? ?.�ਓB���?<��N���?.��0.��S@? ��B���?<UUN���0.��S@? 0.��S@? ��B���O� $_N^NuNV  N^NuNV  p N^NuNV  pN^NuNV��H� $H.  JN��8 JDg  �A���N�zHn�� JN���Hn��/<  ��:.���n��<.���n��v �FXOl6`6C  mv `C ov:.���n��Hŋ� <.���n��Hƍ� 0C �� 2�@=A��0C �� 2�@=A��0.���C=@��0.���C=@��v J. 
f C Jg C@ J* gJ. gJgHn��/<  ��UOHn��Bg??0< ��0L��N^NuNV��H�0&H$n /
 KN���/
/<  ��Sj Sj J+ XOgJ. gJ. 
g
/
/<  ��Hn��Hz�Hn��N��A���N�~Hn��Hz�Hn��N��B���/.��A���N�Hn��Hz�Hn��N��xB���/
A���N�� n��HPA���N�LA���N�( KN��&?< /
/N��8Hn��N��\O� >L�N^NuNV  N^NuNV  pN^NuNV  p
N^NuNV��/
$H JN��@��J.��g VA���N��Hn�� JN���Hn��/<  ��?<DDN��:?.��0.��W@? ��?.��0.��T@? ��0.��T@? ?.����0.��W@? ?.����?<��N���0.��T@? 0.��S@? ��0.��W@? 0.��S@? ��0.��S@? 0.��W@? ��0.��S@? 0.��T@? ��?<UUN���0.��R@? 0.��R@? ��B���?<UUN��0.��U@? 0.��R@? ��B���?<UUN��n0.��R@? 0.��U@? ��B���?<��N��P0.��U@? 0.��U@? ��B���A���N�ZHn�� JN��Hn��/<  ��?<UUN��?.��0.��U@? ��?.��0.��R@? ��0.��R@? ?.��0.��U@? ?.��?<��N���0.��R@? 0.��S@? ��0.��U@? 0.��S@? ��0.��S@? 0.��U@? ��0.��S@? 0.��R@? ��?<DDN��0.��R@? 0.��R@? ��B���?<DDN��f0.��U@? 0.��R@? ��B���?<DDN��H0.��R@? 0.��U@? ��B���?<��N��*0.��U@? 0.��U@? ��B���O�  ` A���N�,Hn�� JN��Hn��/<  ��?<UUN���0.��T@? 0.��S@? ��0.��W@? 0.��S@? ��0.��S@? 0.��W@? ��0.��S@? 0.��T@? ��?<��N��0.��R@? 0.��R@? ��B���?<��N��x0.��U@? 0.��R@? ��B���?<��N��Z0.��R@? 0.��U@? ��B���?<UUN��<0.��U@? 0.��U@? ��B���A���N�FHn�� JN��Hn��/<  ��?<��N�� ?.��0.��U@? ��?.��0.��R@? ��0.��R@? ?.�訓0.��U@? ?.�訑?<��N���0.��R@? 0.��S@? ��0.��U@? 0.��S@? ��0.��S@? 0.��U@? ��0.��S@? 0.��R@? ��?<��N��p0.��R@? 0.��R@? ��B���?<��N��R0.��U@? 0.��R@? ��B���?<��N��40.��R@? 0.��U@? ��B���?<��N��0.��U@? 0.��U@? ��B���O� $_N^NuNV��-H��J. g0J. 
gJ. g?<��N���TO`?<��N���TO` n��Hh �`Hx �cN^NuNV��/-H��A���N��Hn�� n��N��"Hn��/<  �� n��"h �� d"i `N�6 Hn��??��XO&N^NuNV��/-H��A���N�tHn�� n��N��� n��"h �� \"i XN�6 Hn��??��XO&N^NuNV��/-H��A���N�0Hn�� n��N��Hn��/<  ��Hn��Hz�Hn��N��vA���N��Hx !�bHx �c n��"h �� d"i `N�6 Hn��??��Hn��N��`O� &N^NuNV��/-H����A���N��Hn�� n��N�� n��"h �� \"i XN�6 Hn��??��/. ��XO&N^NuNV��-H���� n��Hh �� n  PHP��N^NuNV��/
$H JN��DJ f �Hn��HzHn��N�ݠA���N�� JN��=@��0.��O� J@g
S@g H` dHx �cHj A���N�Hn��/<  ��Hn����?<��N��0* \@? 0* T@? ��0* V@? 0* Z@? ��0* X@? 0* Z@? ��0* ^@? 0* T@? ��0* V@? 0* Z@? ��0* V@? 0* ^@? ��?<��N��B0* V@? 0* P@? ��0* X@? 0* P@? ��0* X@? 0* \@? ��0* \@? 0* \@? ��0* \@? 0* X@? ��0* ^@? 0* X@? ��0* ^@? 0* V@? ��0* P@? 0* V@? ��0* P@? 0* T@? ��0* Z@? 0* Z@? ��B���0* ^@? 0* Z@? ��B���0* Z@? 0* ^@? ��B���?<��N��V0* V@? p	�j ? ��0* Z@? p	�j ? ��0* Z@? 0* P@? ��0* \@? 0* P@? ��0* \@? 0* ^@? ��0* ^@? 0* ^@? ��0* ^@? 0* \@? ��0* P@? 0* \@? ��0* P@? 0* X@? ��?<��N��0* \@? p	�j ? ��0* ^@? p	�j ? ��0* ^@? 0* P@? ��0* P@? 0* P@? ��0* P@? 0* ^@? ��p	�j ? 0* ^@? ��p	�j ? 0* T@? ��?<��N��&0* X@? p
�j ? ��0* ^@? p
�j ? ��0* P@? p	�j ? ��p	�j ? p	�j ? ��p	�j ? 0* P@? ��p
�j ? 0* ^@? ��p
�j ? 0* X@? ��O� ` "Hx �cHj A���N��Hn��/<  ��Hn��?<��N��tp	�j ? 0* Z@? ��p	�j ? 0* ^@? ��0* Z@? p	�j ? ��0* ^@? p	�j ? ��?<��N��$0* V@? p	�j ? ��0* X@? p	�j ? ��p	�j ? 0* V@? ��p	�j ? 0* X@? ��?<��N���0* T@? p	�j ? ��0* T@? 0* P@? ��0* V@? 0* P@? ��0* P@? 0* V@? ��0* P@? 0* T@? ��p	�j ? 0* T@? ��?<��N��`0* T@? 0* ^@? ��0* T@? 0* X@? ��0* X@? 0* T@? ��0* ^@? 0* T@? ��?<��N��0* R@? 0* ^@? ��0* R@? 0* X@? ��0* X@? 0* R@? ��0* ^@? 0* R@? ��0* T@? 0* T@? ��B���A���N�	�A���N��Hj A���N��Hn��/<  ��Hn�訸O� Hn��N��2XO$_N^NuNV��H� $H. Hn��Hz	VHn��N���A���N�	.A���N�jHj A���N�|Hn��/<  �� JN���6 J. 
O� g&JgJCgC f?<��N��"TO`Hx �cHn����0g  �S@gS@g  �S@g,`|A���N��Hn��A���N�Hn��/<  ��Hn�訸XO`x| ���| ���| ���| ���| "��| "��| "��| "��Jg?<wwN��v?<��N��XOHn��Hn��`&6.���n��HÇ� 0.��T@? ?��0.��W@? ?��Hn��N���XOL�N^NuNV��/
$HHj �� JN��8J g(A���N�0Hj A���N�BHn��/<  ��Hn����XO$_N^NuNV��-H�� n��B� n��N^NuNV��-H�� n��B�B���UO/. Hn���D@��J.��gRJ���g n��J�f`BB���UOB�Hn���DJ���g n��J�g n�� P"n��"Q"i �� g n�� P"n��"� N^NuNV��-H�� n��B�B���UO/. Hn���B@��J.��gRJ���g n��J�f`BB���UOB�Hn���BJ���g n��J�g n�� P"n��"Q"i �� g n�� P"n��"� N^NuNV  H�0$H:. &n 
Bk Bk BSJ�g RJ�fp `@ R P8( RDv `,0C � R"P�qf0C � R P��&� 
7h  p`RC�Dm�p L�8N^NuNV  /
$H JN�tA�  �PJMM N�״  {************************************************************}{*															*}{* Sample LList												*}{*															*}{* The following code demonstrates the use of an LList in 	*}{* a modal dialog. See LList.p for documentation on each	*}{* LList procedure.											*}{*															*}{************************************************************}program SampleLList;	uses		LList;	const		DIALOG_RSRC_ID = 128;		DIALOG_LIST_ITEM = 3;		DIALOG_SORT_FWD_ITEM = 4;		DIALOG_SORT_BKWD_IT MRSED;		DI       E  E  ?   develop                    LogLibComponent.rsrcC   rsrcRS  rsrcRSED  (                    ���      �            �%    Norton FileSaver Data             %    	Project �                        �%    Think Reference �           ,LogC        �       CODE,STR ,STR -ICN#,   ����  �  �  ���� ��� �� ��� J�  M��I  A  A��A  A� A  A  A��A  I�  M�� J�  H�� H�  H�� H�  H���Ȁ  �  ������������������������������������������������� � � �KAHL�����       &  &   �	ulong		tcpOctetsInDup;	ulong	68k MacTCP.xcoffk   LIB KAHL   LIB KAHL  (                    �2      �Pkts;	ulong		tcpRetransPkts;};#if defined(powerc) || defined(__powerc)#pragma options align=reset#endif	#if defined(powerc)  � NumToolboxTraps NGetTrapAddress 
OpenDriver  OpenResolver    Sync    	SyncAsync   TCPSend 
TCPRelease  TCPClose    	TCPCreate   TrapAvailable   UDPMultiRead    UDPMultiSend    UDPRead StrLen  UDPMultiCre tSPM UDPMa  /* 	MacTCPCommonTypes.h  	C type definitions used throughout MacTCP.							Copyright Apple Computer, Inc. 1988-91 	All rights reserved		Modifications by Dave Nebinger (dnebing@andy.bgsu.edu), 01/30/94.		06/12/95 - Added standard UPP's for pascal and C routines.*/#pragma once#ifndef __H_MacTCPCommonTypes__#define __H_MacTCPCommonTypes__#ifndef __TYPES__#include <Types.h>#endif /* __TYPES__ */#ifndef NEWBLOCK#define NEWBLOCK#define NEWPTR(a,b) ((a)NewPtr(b))#define DISPO