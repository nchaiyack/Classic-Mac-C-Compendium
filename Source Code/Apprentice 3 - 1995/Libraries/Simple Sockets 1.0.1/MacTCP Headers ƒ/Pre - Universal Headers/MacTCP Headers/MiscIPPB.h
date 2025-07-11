/* 	IPPB.h		C definitions of parameter block entries needed for IP calls    Copyright Apple Computer, Inc. 1988-91     All rights reserved	*//* control codes */#define ipctlEchoICMP		17			/* send icmp echo */#define ipctlLAPStats		19			/* get lap stats */#define icmpEchoTimeoutErr 		-23035			/* ICMP echo timed-out */#define ParamBlockHeader 	\	struct QElem *qLink; 	\	short qType; 			\	short ioTrap; 			\	Ptr ioCmdAddr; 			\	ProcPtr ioCompletion; 	\	OSErr ioResult; 		\	StringPtr ioNamePtr; 	\	short ioVRefNum;		\	short ioCRefNum;		\	short csCode	typedef void (*ICMPEchoNotifyProc) (struct ICMPParamBlock *iopb);struct IPParamBlock {	ParamBlockHeader;					/* standard I/O header */	union {		struct {			ip_addr	dest;				/* echo to IP address */			wdsEntry data;			short timeout;			Ptr options;			unsigned short optLength;			ICMPEchoNotifyProc icmpCompletion;			unsigned long userDataPtr;			} IPEchoPB;		struct {			struct LAPStats *lapStatsPtr;			} LAPStatsPB;		} csParam;	};struct ICMPParamBlock {	ParamBlockHeader;					/* standard I/O header */	short params[11];	struct {		unsigned long echoRequestOut;	/* time in ticks of when the echo request went out */		unsigned long echoReplyIn;		/* time in ticks of when the reply was received */		struct rdsEntry echoedData;		/* data received in responce */		Ptr options;		unsigned long userDataPtr;		} icmpEchoInfo;	};	typedef struct LAPStats {	short	ifType;	char	*ifString;	short	ifMaxMTU;	long	ifSpeed;	short	ifPhyAddrLength;	char	*ifPhysicalAddress;	union {		struct arp_entry *arp_table;			struct nbp_entry *nbp_table;		} AddrXlation;	short	slotNumber;	};	#define NBP_TABLE_SIZE	20				/* number of NBP table entries */#define NBP_MAX_NAME_SIZE	16+10+2struct nbp_entry {	ip_addr 	ip_address;				/* IP address */	AddrBlock	at_address;				/* matching AppleTalk address */	Boolean		gateway;				/* TRUE if entry for a gateway */	Boolean		valid;					/* TRUE if LAP address is valid */	Boolean		probing;				/* TRUE if NBP lookup pending */	int			age;					/* ticks since cache entry verified */	int			access;					/* ticks since last access */	char		filler[116];			/* for internal use only !!! */	};#define ARP_TABLE_SIZE	20				/* number of ARP table entries */typedef struct Enet_addr {	b_16 en_hi;    b_32 en_lo;	} Enet_addr;typedef struct arp_entry {	short		age;			/* cache aging field */	b_16		protocol;		/* Protocol type */	ip_addr 	ip_address;		/* IP address */	Enet_addr	en_address;		/* matching Ethernet address */	};RHh ��``/
Hn����`V R-h ��-h ��G���Hn��0.���n���k �k H�rN�  �? 0.���n���k �SH�rN�  �? ��/
Hn��� n��B� h/
 _�#/-󬩣L�N^ _PONЙINTERNALGETANDDRAWPICTRES  NV�� n -P��-h ��?. Hn��< N��N^ _\ONКSATGETANDDRAWPICTRESINRECT   NV�� n -P��-h ��?. Hn��< N���N^ _\ONМSATGETANDCENTERPICTRESINRECT   NV��H�0:. $n 
 n -P��-h ��J�g/ _�YO0.���n��H��/ N�  X _&H$�J�g`0.���� H�rN�  �2����Ү Ҁ R �|�n��8.��SD6`*0E0�n��S@H��"R 1 Ј2�n��H�� R!� RCi�Do�L�xN^ _O� NЎSATMAKEROWLIST   NV��H� 8$n  n -P��-h ��J-��gYO�2 _(HHn���tJ-��g <J��&f
Hm��N�  � m�& P h  P&h *UOHn�� m�& P h  P?(  //
Hj N�  �0J@g
Hm��N�  � R/(  _�d R/(  _�)YO R h / �U.� _&H R h  �YO j / �U.� _&H j  �UO>� 0J@g
Hm��N�  �/�1/.���s j  P h  P%P  j  P h  P%h  %h 
 YO j  P h  P2h /	Hx?��X 5@ &RHk  k  P/YO k  P2h /	Hx?��X ? Hj ?-�N���`  �B� YOHx lN�  X _&H$�/�o R!n�� !n��  R"R#h  #h   R/( Hn����Hn���{ R"R0) �h @ H�r N�  �rN�  f R1@ YO R"R0(  R2( �i ��X�/N�  X _"R#H  RJ� g RpШ  R!@ /�s RHh �� R%h   R%h  %h   R5h  &RHk /+ ?+ Hj ?-�N���/.���sL� N^ _PONАSATMAKEOFFSCREEN   NV  J-��g0YOHx�  m�& P h  P0. ��  @ H���/ �[ =@ 
`YOHx� p�n H���/ �[ =@ 
N^ _TONЎGETROWBYTESDEV   NV  J-��g$ m�& P h  P0. ��  @ H���=@ 
`p�n H���=@ 
N^ _TONЏGETROWBYTES2DEV  NV��/
J��g/-�/-�N�  @YO?< � _$H 
J�fB��`: m�-h ��-h ��UOHn��?< /
Hm�Hm�N�  �0=@��Jn��gB��$_N^Nu�SATINITDITHER  NV��/
$n Hn���tYO�2 _-H��J��fN��\`0 m�"m�0) �h ]�D  m�"m�2) �h ]�D� gN��*/
/-�/-� RHh FN��Hm�N�� m�Hh  m�Hh  RHh F RHh F?< @Hx  ��/.���s/.���1$_N^.�Nu�DITHERICON   NV  H�8(n (.  DJ�g D/ _� DB�l  ^�D �-��g>$L6* 
�j �� //Hx N�  �YO/N�  X _"D"�/ D//N�  �`  �$D%l  %l 
 
0* �j @ H�rN�  �rN�  f5@ YO0* 
�j �� / N�  X _$�$L6* 
�j �� &T D$P:, 
SEv `�pЋ&@pЊ$@RCi�Eo�YO0l /Hx �X : JEgr$L6* 
�j �� 0l  ЈS�&@ D2h  D ЉS�$@p�E,<   ��:, 
SEv `.YOH�H�/ 0F/�X �0l  Ј&@ D2h  
Љ$@RCi�Eo�L�xN^ _PONЈCOPYBMAP   NV��/
 n -P��-h ��$n %n�� %n�� 
p�j H�r N�  �rN�  f5@ YO0* �� 
H�/ N�  X _$�$_N^ _PONЈMAKEBMAP   NV  H�06. J-�fN�  E�J-��g YO?� _$HYO/-��?�� _&H/ _�I`RYO/-��?�� _&H$K 
J�g</
 _�)/
 _�J&RG� @6+ 
�k �� 0C"RC� R 	Ј R!@ @ RA� R"R#H 2-J 
L�N^ _TONЊSATGETCICN   NV��H� 8$n &n (n  n -P��-h �� 
J�g  �Hn���t J�g/�s`&n��J-��g YO�2 _&H J�g/�1Hn��/
�/�1`| Rh  6o$ RHh @ RHh 2Hk  RHh F RHh FHn���`N RHh @Hn��N��� RHh 2Hn��N��� RHh @ RHh 2Hk  RHh F RHh FHn���/.�� _�/.�� _�/.���sL� N^ _O� NЋSATPLOTCICN  NV  /
$n  
J�gJ-��g/
�%`/
��$_N^.�Nu�SATDISPOSECICN   NV  H� 0$n 
0. S@gW@g``J�gd RJ�g R/ _�/ _�`N&R/ _�/+  _�/+  _�J�  g/+   _�B�  J� $g/+ $ _�B� $/ _�B�`/ _�B�J� g/*  _�B� J� "g/* " _�B� "L� N^ _\ONЏDISPOSEFACEDATA  NV��H�0$n 
8. -j ��-j ��0S@gW@gf` PYOHx N�  X _$�&R'n�� 'n�� 
UO?+ N��p07@ 6.���n���� YO/N�  X _&�Hk /?+ Hj ?< N��x` @YOHx (N�  X _$�&RUO?.��N��05@ 6.���� YO/N�  X _&�Hn��/?* Hj ?< N��&UOp�n��? N���07@ 6.���� H�YO/N�  X _'H 'n�� 'n�� Hk Hx ��Hk /+ ?+ Hk  ?< N���Hk Hk N���Bk Hk /+ ?+ Hk $?< N���Hj /* ?* 
Hj "?< N��`RUO?.��N��@05@ 6.���� YO/N�  X _$�Hn��/?* Hj ?N��NHj /* ?* 
Hj "?< N��6L�N^ _\ONЋNEWFACEDATA  NV��H� 0&n  n -P��-h ��J-�fN�  E�Hn��0.��D@? 0.��D@? �� J�fYOHx &N�  X _&H$K`$KHj Hn��N���/
?-�N���%m�� +J��-J L� N^ _PONЌSATNEWFACEPP   NV  /. ?. 
N���/. ?. N��jN^ _PONЋREBUILDFACE  NV��/
 n -P��-h ��J-��gt m� h  P �  m� h  P!n�� !n�� 
UO?.��N��\0 m� h  P1@  m�!n�� !n�� /-�sJ��gR m� P!n�� "!n�� &/-�1`8$m�%n  %n�� %n�� UO?.��N��f05@ %n�� %n�� /-�s m� h  P0( �n��]�D  m� h  P2( �n��]�D� g m�/( Hn����Hn���{$_N^ _PONЋSETICONPORT  NV��/
 n -P��-h ��J-��gt m�� h  P �  m�� h  P!n�� !n�� 
UO?.��N��:0 m�� h  P1@  m��!n�� !n�� /-�ƨsJ���gR m�� P!n�� "!n�� &/-�ʪ1`8$m��%n  %n�� %n�� UO?.��N��D05@ %n�� %n�� /-�ƨs m�� h  P0( �n��]�D  m�� h  P2( �n��]�D� g m��/( Hn����Hn���{$_N^ _PONЌSETICONPORT2   NV��H�8$n 
6. Hn���tJ-��gYO�2 _(JCf6-�(J0Y@g``&T/Hl N��z-k ��-k ��Bn��/+ Hn��N��� m�Hh  m��Hh Hl Hk BgHx  ��Hl Hk Hl Hk BgHx  ��J� f
YO�� _%H UO?<��N�  F�-��gUO/* Hj ��0J@g B� `UO/* Hj N�  �0J@gB� J-��g/�1/.���sL�N^ _\ONГINTERNALCHANGEDFACE  NV  H�8&n 
8. Jk g  �YO?+ N�� _$H 
J�g  �0S@gW@g4`b(RI� @6, 
�l �� H� RA� R Ѓ R!@ @ RHh @/N���`N S/Hk N��&J��g/
N��`4/
/-�/-� RHh FN��T`/Hk N���/
/-�/-� RHh FN��4/?N��"/
N��*L�N^ _\ONЊREDRAWFACE   NV  YO?. Hx  N�  _-H 
N^ _TONЊSATGETFACE   NV��H�86. (n J-�fN�  E� J�f$m��`�j f-J `  �$j  
J�f�Hn��N�"YO/-��?�� _$H&J J�fB� `  �UO/N�  
� / _�J/ _�)YO SHh 8/N��6 _$H5C B�  SA� R"S#H 2 SHh 2Hj N��Hj /* ?* 
Hj "?< N��/N�  
���/
?-�N��-J Hn��N� �L�N^ _\ONЌSATGETFACEPP   NV  /
$m��`4/
?-� m�& P h  P?(  N��,/
 m�& P h  P?(  N���$j  
J�f�$_N^Nu�SATRELOADICONS   NV  /
$n /
�tJ-��gYO�2 _%H `B� B� $_N^.�Nu�SATGETPORT   NV�� n C���p"�Q���2�/.��sJ-��gJ���g/.��1N^.�Nu�SATSETPORT   NV  Hm�@N���N^Nu�SATSETPORTOFFSCREEN  NV  Hm�ZN���N^Nu�SATSETPORTBACKSCREEN   NV  Hm�"N��fN^Nu�SATSETPORTSCREEN   NV  /
J-��g($m�& R h  P+h �+h 
� R h  P;h  �`+m�R�+m�V�;| �$_N^Nu�SATSETUPSCREENVARS   NV��H� 8. 
:. ;D�|;E�~m �^�D JDV�D�-��� g6`6JCg  �N��B'��YO/-��?�� _$H< �� 
J�f
Hm��N�  �Hn��B�?-�<?-�>��N���J-�g?Hn��N�   �`?Hn��N�  !N��� m�ZHh  m�@Hh  m�@Hh  m�@Hh BgHx  ��L�8N^.�Nu�SATDRAWPICTS   NV��;n 
�|;n �~Hn��B�p�m�<? ?-�>��J��@g
Hm�@N�  J��Zg
Hm�ZN�  Hm�@Hn��N��Hm�ZHn��N��J��"f
Hm��N�  �-m���-m���Hn��0.��D@? 0.��D@? ��N��� m�@Hh �{ m�Z/(  m�@Hh �� m�@Hh ��N��� m�ZHh �{ m�Z/(  m�ZHh �� m�ZHh ��?. 
?. N��2N���N^.�Nu�SATSETUPOFFANDBACK   NV��Hn��N���N��|Hn�� m�"?(  m�"?( Bg m�"?( ��Hn����Hn��Bg m�"?( ?-�<Bg��Hn����Hn��Bg?-�> m�"?(  m�"?( ��Hn����Hn��?-�< m�"?(  m�"?( ?-�>��Hn����Hn��B�?-�<?-�>�� m�@Hh  m�"Hh Hn��Hn��BgHx  ��Hn��N��,N^Nu�SATREDRAW  NV  n �;n �;n 
�;n �N^ _PONЌSATCONFIGURE   NV   n  PHh  n  PHh /. /. BgHx  /-� _N�N^ _O� NЌSAFERECTBLIT   NV��H�8$n (n &n 8. 
6. -n ��-n ��0.���D=@��0.���C=@��0.���D=@��0.���C=@��m �f  YO0.���n��H�/ Hx �X J�f  � R"m�"i "Q"� m� h  P!j  !j  
YO0j /Hx� �[  m� h  P1@ J� f$ m�Hh Hj  SHh Hn��Hn��Hn��` �/* /-�/-�� _N�/-�0.��n��? 0.��n��? /-�� _N� J�g//-�/-�/-�� _N� m�Hh  SHh Hn��Hn��Bg/-�/-� _N�` t R"m�"i "Q"�  R"m�"i "Q#h  #h  
 m� h  PBh p�n��=@��p�n��=@��YO R2h /	Hx� �[  m� h  P1@ J� f$ m�Hh Hj  SHh Hn��Hn��Hn��` �/* /-�/-�� _N�/-�0.��n��R@? 0.��n��? /-�� _N� J�g//-�/-�/-�� _N� m�Hh  SHh Hn��Hn��Bg/-�/-� _N�` nm �o  � m� h  P � m� h  P!j  !j  
YO0j /Hx� �[  m� h  P1@ J� f$ m�Hh Hj  SHh Hn��Hn��Hn��`  �/* /-�/-�� _N�/-�0.��n��? 0.��n��? /-�� _N� J�g//-�/-�/-�� _N� m�Hh  SHh Hn��Hn��Bg/-�/-� _N�`  �J� f/Hj  SHh Hn��Hn��Hn��`f/* /-�/-�� _N�/-�0.��n��? 0.��n��? /-�� _N� J�g//-�/-�/-�� _N�/ SHh Hn��Hn��Bg/-�/-� _N�L�N^ _O� NЌSAFEMASKBLIT   NV  /
$n  
J�fA� �"n "�`/
 _�d/
 _�)YO/ �U.� _"n "�$_N^ _O� NЌSetRectBlit0   NV  /
$n  
J�fA� �"n "�`/
 _�d/
 _�)YO/ �U.� _"n "�$_N^ _O� NЌSetMaskBlit1   NV��H�0&n $n . .  n -P��-h ��| �J��f
YO�� _+H�>-�UO?< Hn��N�  �06 n����Hn��N�  3�J-��gUON�  
�@��`| ��J��fp\+@�YO?<��< N�  � & +C�YO?<��< N�  � & +C��YO?<��< N�  � & +C��YO?<��< N�  � & +C��YO?<�< N�  � & +C��YO?<�< N�  � & +C��YO?<�s< N�  � & +C�YO?<�t< N�  � & +C�YO?<�1< N�  � & +C�YO?<�2< N�  � & +C�J-��fB��&` 
J�g+J�&`
YO�* _+H�&J-��gYO�* _���&gxN�  4�Jgv `v J�f  �J. g-m���-m���0.���C=@��`-n����-n����J��"g m�""m�"0) �h f/-�"�J-��g&YOHx  Hn��Hm��B'?< Hx��B'B��E _+H�"`dYOHx  Hn��Hm��B'?< Hx��B'B�� _+H�"`>+K�"Hn�¨t/-�"�s m�"-h ��-h ��Hn��pHn���pHn���pHn���p/.�¨sJ. gZ:.���n��8.���n��0-�m�EH�rN�  �=@��0�n��=@��0-�C�m�DH�rN�  �0CЈ=@��0�n��=@��Jg@0.���n��l=n����0.���n��o=n����0.���n��l=n����0.���n��o=n����-m���-m���Hn��0.��D@? 0.��D@? ��JgF0.���m�l=m���0.���m�o=m���0-��C�n��o
0-��C=@��0.���m�o=m���Jg,YOp�n��H�/ Hx��X =@��YO0n��/Hx���X =@��0.���n��;@�<0.���n��;@�>N�  4�0.��D@�m�H�+@�t0.��D@�m�H�+@�x0.��D@�n��? 0.��D@�n��? �x m�& P h  P+P�6 m�& P h  P+h �.+h 
�2YO m�& P h  P2h /	Hx?��X ;@�:Hn�� m�& P h  P/YOHx?� m�& P h  P2h /	�X ? Hm�*?-�N�  !fN�  	RHn��B�/< @ B��Hm�N�  bHm��N�  bJ���g/-��}/-�� _�B���Hm�Hn��N�  ".J-��g m� h  P/ _�`0 m�J� g m� ( Y� m�!@  m�/(  _� m�B� Hm��Hn��N�  ".J-��g m�� h  P/ _�`0 m��J� g m�� ( Y� m��!@  m��/(  _� m��B� YOHx lN�  X _$H+J��/-��o m��!n�� !n��  m��"m��#h  #h   m��/( Hn���Hn��{ m��"m��0) �h @ H�r N�  �rN�  f m��1@  m��B� Jm�f;|  �B��J. 
gJ-��gN�  %�J���g�m�gN�  3�?.  ?. N�  6(J-��g <YO/-��?< � _/Hm��/N���YO/-��?< � _/Hm��/N���YO/-��?< � _/Hm��/N���YO/-��?< � _/Hm��/N���YO/-��?< � _/Hm��/N��VYO/-��?< � _/Hm��/N���YO/-��?< � _/Hm�/N��"YO/-��?< � _/Hm�/N��^YO/-��?< � _/Hm�
/N���YO/-��?< � _/Hm�/N��*YO/-��?<  � _/Hm�/N���YO/-��?<  � _/Hm�/N���`0YO/-��Bg� _/Hm��/N���YO/-��Bg� _/Hm��/N���Hn��N�  40L��N^ _O� NЍSATCUSTOMINIT  NV��Hn��B�?. 
?. ��?. ?. Hn��Hx  Hx  B'< < < < N���/-�"�/-�"�N�  76N^ _PONЇSATINIT  NV��Hn��B�B���B�Hn��Hx  Hx  B'< B'B'< N��bN^Nu�SATINITEMPTY   NV��/
 n -P��-h ��Jn��lBn��0.���m�>o=m�>��Jn��lBn��0.���m�<o=m�<��YOHx N�  X _-H��$n��%m� $���%n�� +J�$_N^.�Nu�SATBACKCHANGED   NV  H�8(n &n &. J-�fN�� J�fYO/-�N�  X _&H$K`$KB� B� B� "B� B� & 
J�gX&J6� 7n  7n  'k  * J�f'm� .B� 2+J�J� .g& k .!J 2`'l . .'L 2)J .J� .g k .!J 2 
J�g&J�g"/
/ _N�%j  %j 
 Hj ?* ?* ��-J L�N^ _O� NЎSATNEWSPRITEPP   NV  YOHx  Hx  ?. ?. ?. /. N��� _-H N^ _O� 
NЌSATNEWSPRITE   NV�� n -P��-h �� n -P��-h �� n -P��-h ��0.���n��]�D 2.��n��]�D4.���n��]�D@��0.��n��]�D ���.��@ N^ _O� NЋMySectRect3  NV��H�0&m�B-�`  �JSo  �8+ �m�6+ �m�$k .`\JRlFUOHk Hj Hn��/. N��2g,6� 
4���J� g//
/+  _N�J� g/
//*  _N�$j . 
J�g�j l�� 
J�f�$k 2`\JRlFUOHk Hj Hn��/. N���g,6� 
4���J� g//
/+  _N�J� g/
//*  _N�$j 2 
J�g�j o�� 
J�f�S��]�D �-�@�&k . J�f � L�N^.�Nu�KindSearch4  NV��H�0&m�B-�`h6+ �m�$k .`PUOHk Hj Hn��/. N��g$J� g//
/+  _N�J� g/
//*  _N�$j . 
J�g�j l�� 
J�f�&k . J�f�L�N^.�Nu�ForwardSearch5   NV��H�0&m�B-�`j6+ �m�$k .`RUOHk Hj Hn��/. N��|g&J� g//
/+  _N�`J� g/
//*  _N�$j . 
J�g�j l�� 
J�f�&k . J�f�L�N^.�Nu�ForwardOneSearch6  NV��H�0&m�B-�`h6+ �m�$k 2`PUOHk Hj Hn��/. N���g$J� g//
/+  _N�J� g/
//*  _N�$j 2 
J�g�j o�� 
J�f�&k . J�f�L�N^.�Nu�BackwardSearch7  NV��H�0&m�B-�`  �JSo  �8+ L�m�6+ L�m�$k .`\JRlFUOHk Hj Hn��/. N��,g,6� 
4���J� g//
/+  _N�J� g/
//*  _N�$j . 
J�g�j Ll�� 
J�f�$k 2`\JRlFUOHk Hj Hn��/. N���g,6� 
4���J� g//
/+  _N�J� g/
//*  _N�$j 2 
J�g�j Lo�� 
J�f�S��]�D �-�@�&k . J�f � L�N^.�Nu�AppKindSearch8   NV��H�0&m�B-�`h6+ L�m�$k .`PUOHk Hj Hn��/. N��g$J� g//
/+  _N�J� g/
//*  _N�$j . 
J�g�j Ll�� 
J�f�&k . J�f�L�N^.�Nu�AppForwardSearch9  NV��H�0&m�B-�`j6+ L�m�$k .`RUOHk Hj Hn��/. N��pg&J� g//
/+  _N�`J� g/
//*  _N�$j . 
J�g�j Ll�� 
J�f�&k . J�f�L�N^.�Nu�AppForwardOneSearch10  NV��H�0&m�B-�`h6+ L�m�$k 2`PUOHk Hj Hn��/. N���g$J� g//
/+  _N�J� g/
//*  _N�$j 2 
J�g�j Lo�� 
J�f�&k . J�f�L�N^.�Nu�AppBackwardSearch11  NV��H� 0&m�B-�`rJSoZ$m�`NJRlFUOHk Hj Hn��/. N��,g,6� 
4���J� g//
/+  _N�J� g/
//*  _N�$j . 
J�f�S��]�D �-�@�&k . J�f�L� N^.�Nu�AllKindSearch12  NV��H� 0&m�B-�`R$k .`BUOHk Hj Hn��/. N���g$J� g//
/+  _N�J� g/
//*  _N�$j . 
J�f�&k . J�f�L� N^.�Nu�AllForwardSearch13   NV��H� 0&m�B-�`T$k .`DUOHk Hj Hn��/. N��g&J� g//
/+  _N�`J� g/
//*  _N�$j . 
J�f�&k . J�f�L� N^.�Nu�AllForwardOneSearch14  NV��H� 0&m�B-�`R$k 2`BUOHk Hj Hn��/. N��vg$J� g//
/+  _N�J� g/
//*  _N�$j 2 
J�f�&k . J�f�L� N^.�Nu�AllBackwardSearch15  NV  0-�gS@gPS@g  �`  �0-�@ b  ��@0; N�  
   � (/N��b`  �/N���`  �/N���`  �/N��`z0-�@ bp�@0; N�  
   f "/N��$`T/N��N`L/N���`D/N���`<0-�@ b2�@0; N�  
   ( "/N���`/N���`/N���`/N��N^Nu�SEARCHHITS   NV  H� J��g  �m �g  �Jm�fd m�"h .`R(i . i 20) �h l>&i 2J� 2g k 2!I .J� .g i .!K 2#k 2 2'i . .#K .'I 2���f+I�"L 	J�f�`jm �fb m�"h .`R(i . i 20) L�h Ll>&i 2J� 2g k 2!I .     T   T   F��������������������test ser_v3.c   TEXTMMCC  h �  TEXTMMCC  h �                  ��bj   Q  �������������������������������������������������������������������������������������������������������������������������   H 	Monaco                             X ]c X ]c��   O   O               T   T   F =�(:    F MPSR   MWBB   ���        ���   L     B&k . J�f�$m�`Hm�ZHm�@/
/ _N�$j  
J�f�&m�`  �$KJ� g  �J� g  �%j  6 j 0* �h  @MMCC 0*   /* * iperr.h * * Constants that define the UNIX errors for the socket-like interface * * Mike Trent 8/94 * */#ifndef _IPERR_#define _IPERR_#include <Types.h>#define	EINTR			4		/* interrupted system call		*/#define	EIO				5		/* I/O error		*/#define	EBADF			9		/* Bad file number */#define	EINVAL			22		/* Invalid argument */#define	EMFILE			24		/* Too many open files */#define	EFBIG			27		/* File too large			*/#define	EPIPE			32		/* Broken pipe				*/#define	ENOTSOCK		38		/* Socket operatio