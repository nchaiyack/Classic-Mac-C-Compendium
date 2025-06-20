�4             �              ; 	�  ; 	�  K�    K�    K�   K� d  L2    L2    L2 8  Lj 4  L� F  L�    L�   M     M �     �       �                                    =�/��
�
��� �  d                                                          Chapter 1	Getting StartedChapter OverviewInstalling NCSA XDIFF/XFIXOn UNIX SystemsOn MacOS SystemsUsing NCSA XDIFF/XFIXSetting UpCreating an Update FileUpdating Your File on the Other MachineCleaning UpChapter OverviewThis chapter outlines the basic steps involved in using the NCSA XDIFF and XFIX programs on different systems. Specifically, the chapter discusses installing the programs, setting up your files on the local and remote machines, and the process of using NCSA XDIFF and XFIX to maintain duplicate files on different systems.Portions of this chapter refer you to another chapter in which the outlined procedure is discussed in detail.Installing NCSA XDIFF/XFIXInstall NCSA XDIFF on your local system and NCSA XFIX on your remote system. Neither XDIFF nor XFIX requires any unusual installation procedures.On UNIX SystemsInstall the programs somewhere within your path. For example, you could install NCSA XFIX in /usr/local/bin. Then, define the path environment variable to include this path. To set the path to /usr/local/bin, for example:If you are using C shell, enter:set path=($path /usr/local/bin)If you are using Bourne shell, enter:PATH=$PATH:/usr/local/binOn MacOS SystemsCurrently, only NCSA XDIFF runs on a Macintosh. As you would for any new Macintosh software, create a new folder and copy the NCSA XDIFF application to your hard disk. The application may also be used from a floppy disk.Using NCSA XDIFF/XFIXThis section outlines the steps involved in using NCSA XDIFF/XFIX to maintain identical copies of a text file on two machines.In the typical editing session, you will use NCSA XDIFF/XFIX to update source code that you keep on a remote machine, but write and edit on a local machine. This process involves the following basic steps:1.	Setting up the remote and local machines so that, in all, you have four files of your original source code�two on each machine.2.	Editing the source code on your home machine.3.	Running NCSA XDIFF on your local machine to compare the current, edited version of the code to the original version and to produce a file of the differences.4.	Transferring the resulting update file to the remote machine via FTP or Kermit.5.	Running NCSA XFIX to create the edited version of the source code identical to that on your home machine on the remote machine.6.	Cleaning up the remote and local machines by replacing the old control copies of the source with your revised, current version.Setting UpBefore you edit the code or use XDIFF or XFIX on either your remote or local machines:1.	Make sure that you have identical versions of the code on the local and remote systems.2.	Duplicate the original file on both the local and remote systems. The duplicate files will serve as control copies of the original file.Refer to the chapter that best describes your local or remote system, "UNIX-Based Systems" or "Macintosh Systems," for instructions on duplicating your text files.When your setup resembles that depicted in Figure 1.1, you may edit the code as much as you like. When you are finished editing, you need to create an update file of your edits as described in the following section.Figure 1.1	Stage 1:  Setting UpCreating an Update FileIn all, you should now have two files of the original, unedited code on the remote machine and one each of the original and edited files on the local machine.To create an update file, you run NCSA XDIFF on your local machine. NCSA XDIFF compares the current, edited text file to your saved control copy of that file and writes any changes you made to a special update file. NCSA XFIX uses this update file to produce a current version of the code on your remote machine.Refer to the chapter in this manual that best describes your local or remote system, "UNIX-Based Systems" or "Macintosh Systems," for complete instructions on creating an updatefile.When your setup resembles that depicted in Figure 1.2, you can FTP your update file to the remote machine and use it to update your original file.Figure 1.2	Stage 2:  Creating an Update FileUpdating Your File on the Other MachineUsing the update file you created using NCSA XDIFF to update your code on the remote system involves two steps1.	Uploading the update file from the local machine with FTP or Kermit, whichever is appropriate, to the remote machine.2.	Executing NCSA XFIX. The current, edited version of the code is recreated on the remote system.Refer to the section entitled "Updating Your File," in Chapter 2 or 3 for complete instructions on running NCSA XFIX. NCSA XFIX is virtually the same on all systems on which it runs. It compares a control file to the update file you uploaded from the local system to update your original file.Running NCSA XFIX causes the original code to be overwritten by an updated version of the code identical to the version current on the local system. After running NCSA XFIX, your setup should resemble that shown in Figure 1.3, where the original file has been overwritten by the edited copy created with NCSA XFIX.Figure 1.3	Stage 3: Updating File on Remote MachineCleaning UpAfter updating your file on the remote machine, you need to clean up your machines so that only the latest versions of your code are preserved. Specifically, you should delete your updatefiles and old control files, and make new control copies of your updated files. Neither XDIFF nor XFIX deletes update files or old control copies, though the update files may be overwritten the next time you run XDIFF. At the conclusion of your editing session, your setup should resemble that shown in Figure 1.4. (See also the section entitled "Model Update Cycle," in Chapter 4.)Figure 1.4	Stage 4:  Cleaning Up1.	NCSA XDIFF/XFIXGetting Started	1.National Center for Supercomputing ApplicationsJune 19891.	NCSA XDIFF/XFIXGetting Started	1.National Center for Supercomputing ApplicationsJune 19897>A?6>A?�  �      �  �       ������ �� �� � 
���� �	        1  � P	��������8	        1  � %  	��������0  � *	        1  � '0  � (  	��������"  �� �           � �        
 , � D " 
+�5	Original+File� �� � 
���� �	        1 k � �	��������8	        1 m � w  	��������0 k � |	        1 f � y0 e � z  	��������" k �� �           � �        
 ~ � �( � �Control+Copy� �� �� � 
���� �	        1 k  � :	��������8	        1 m , w 8  	��������0 k * | 9	        1 i - y :0 h , z ;  	��������" k +� �           � �        
 ~  � 7( � Control+Copy� �� �� �           � � ��   "   
      G( 	 Local System� �� �           � � ��   '   
   � )�Remote System� � 
���� �	        1   P 9	��������8	        1  + % 7  	��������0  ) * 8	        1  , ' 90  + ( :  	��������"  *� �           � �        
 ,  D 7 ( 5 	Original+File� �� �� ��r  \      �  N       �>���� �� �� �� �� � 
���� �	�  �  1   N /	��������8	        1   $ )  	��������0   ) .	        1  ! ' /0    ( 0  	��������"   	�  �  1 -  E &� �           � �        
 -  E & " 
+6Edited+Copy� �� �� � 
���� �	        1  K N y	��������8	        1  h % t  	��������0  i * x	        1  l ' y0  k ( z  	��������"  j� �           � �        
 , L D s( 5 NControl+Copy� �� �� �           � � ��   	   
 , 7 > M ( 9 8+� �� � 
���� �	     �1 � & � T	��������8	        1 � C � O  	��������0 � D � S	        1 � G � T0 � F � U  	��������" � E	        1 � ( � R� �           � �        
 � ( � R  
( � ,Update+File� �� � 
���� �  A h   � R� �  	�������� * *@ g " � W� �           � � ��      
 r '  S ( | (XDIFF� �� �� �           � � ��   	   
 V 7 h M ( c 8+� �� � 
���� �    " � 9  � �� � �t  � 9 � A � = � 9 � = � A � = 
        " � =��#  ##  #�#  #�#  � 
        � � 
���� �� 
        #��##�#�� �  " � = � �� �           � � ��   "   
     ^ 
( 	 Local System� �� � 
���� �	        1  � Q �	��������8	        1  � % �  	��������0  � + �	        1  � ( �0  � ) �  	��������"  �� �           � �        
 - � E � +�-	Original+File� � 
���� �	        1 l � � �	��������8	        1 m � w �  	��������0 l � } �	        1 g � z �0 f � { �  	��������" l �� �           � �        
  � � �( � �Control+Copy� �� �           � � ��   '   
  � ( 
 �Remote System� �� �� ��        �:         �:����� �9� �� �� � ��� � � ��� � ס �   	 
���� �9 "  � 6� �� � ��� � נ ١ � ��� � � ��� � �� ٠ � "  � " ) � " 5 � " A � " M � � �� � ��� � �� ٠ ס � ��� � � ��� � ס �   	 "  �) � �� � ��� � נ ١ � ��� � � ��� � �� ٠ � "  � "  � "  � "  � � �� � ��� � �� ٠ �	        1  � ( �  	��������0  � - �	        1  � * �0  � + �  	��������"  �� �           � �        
 / � G � " 
+�8	Original+File� �� � ��� � � ��� � ס �   	 
���� �9 "  � 6� �� � ��� � נ ١ � ��� � � ��� � �� ٠ � "  � " ( � " 4 � " @ � " L � � �� � ��� � �� ٠ ס � ��� � � ��� � ס �   	 " R �+ � �� � ��� � נ ١ � ��� � � ��� � �� ٠ � " R � " R � " R � " R � � �� � ��� � �� ٠ �	        1 � J � V� �	     �1 d � � �	��������8	        1 f � p �  	��������0 d � u �	        1 ] � r �0 \ � s �  	��������" d �	        1 w � � ͡ �           � �        
 w � � �( � �Update+File� �� �� � 
���� �91 d � �	��������8	        1 f � p
  	��������0 d � u	        1 _ r0 ^  s  	��������" d �� �           � �        
 w � �
( � �Control+Copy� �� �� �           � � ��   	   
 t � � � ( � �+� �� � 
���� �9	          A � � � �  	�������� * *@ � � � � �           � � ��      
 � � � � ( � �XFIX� �� �� �           � � ��      
 � � � � ( � �+� �� �           � � ��   '   
  �  � 
(  �Remote System� �� �           � � ��   
   
 � u � �( � vFTP� � 
���� �9	�  �  1 * � ^ �  	��������8	�  �  1 * � 3 �1 + � 8 �  	��������0 * � 9 �	        1 ( � 7 �0 ' � 8 �  	��������" * �	�  �  1 = � U � �           � �        
 = � U � ( F �Edited+Copy� � 
���� �9    " P �  � �� � t  F � P F � P � J � I F � 
        	��������" F �
#  #��#  #�#  #��#  � 
        � � 
���� �9� 
        #
#��#�#��� �� �	�  �  1   P /  	��������8	        1   & )  	��������0   + .	        1  ! ) /0    * 0  	��������"   	�  �  1 /  G &� �           � �        
 /  G &( 8 Edited+Copy� �� �� � 
���� �9	        1  B O p	��������8	        1  _ & k  	��������0  ` + o	        1  c ( p0  b ) q  	��������"  a� �           � �        
 - C E j( 6 EControl+Copy� �� �� �           � � ��   	   
 - 3 ? I ( : 4+� �� �           � � ��   	   
 W 3 i I**+� �� �           � � ��   !   
     W 
( 	 Local System� �� � 
���� �9	          A k  � Q  	�������� * *@ j  � S� �           � � ��      
 u $ � P +vXDIFF� �� � 
���� �9    " � 6  � �� � �t  � 6 � > � : � 6 � : � > � : 
        " � :��#  ##  #�#  #�#  � 
        � � 
���� �9� 
        #��##�#�� �  " � : � �� �	     �1 � $ � R	��������8	        1 � B � N  	��������0 � B � Q	        1 � E � R0 � D � S  	��������" � C	        1 � ' � Q� �           � �        
 � ' � Q  
+KUpdate+File� �� �� �� � 
���� �9	��������q  � � � � � � � � � � � � � �" � X?  �� �   @ � �   @ � �     #e�� �         � �    / /   ` \ |'   Z� ɡ �   @ � נ ١ �   @ � �   @ � �� � p V J � � J � M T \ b j q { � � � � � � � � � � � � �� �   @ � �� ٠ נ �� �� �(  �      �  �       ������ �� �� � 
���� �	�  �  1  � P �	��������8	        1  � % �  	��������0  � * �	        1  � ' �0  � ( �  	��������"  �� �           � �        
 , � D � " 
+�5	Original+File� � 
���� �	�  �  1 k � � �	��������8	�       1 m � w �1 l � { �  	��������0 k � | �	        1 f � y �0 e � z �  	��������" k �	�  �  1 ~ � � �� �           � �        
 ~ � � �( � �Control+Copy� � 
���� �1 k  � 9	��������8	�       1 m + w 71 l * { 7  	��������0 k ) | 8	        1 i , y 90 h + z :  	��������" k *	�  �  1 ~  � 6� �           � �        
 ~  � 6( � Control+Copy� �� �           � � ��   "   
      G( 	 Local System� �� �           � � ��   '   
   � )�Remote System� � 
���� �1  
 P 8	��������8	        1  * % 6  	��������0  ( * 7	        1  + ' 80  * ( 9  	��������"  )� �           � �        
 ,  D 6 ( 5 	Original+File� �� �� ��    ?      �   1       � ! r � x� 
 r � x�1 r � x��    ?      �   1       � ! r � x� 
 r � x�1 r � x��of the two applications, sor      @               *   [   f   i        	  
        �  �  a  �  �  �  �  �  �  � } w q k e e }X      X     ��         `�  �  �  @  K    '  (  )  �  �    9  :  ;  �  �  �} { } n } } a }T X     /� X     "� X     `@�  �  �  �  �  �  �  �            ;  <  >  n  z  {  | | v i | | v \    @       3� @       3�    @  {  �  �  47  4P  4Q  4R  4S  4U  D�  D�  D�  E  E(  EM  E^  Ez  E{  E�  E�  E�  E�  E�  E�  y     y                 @                  0  K  [  l  �  �  �  �  �{{w{l hXTThTTTT     ��  @T �
�
 �,0  �  �  �  �  /  0  �  �  �  �  �  N  O  P  `  >  ?{wsosksswgcss_[W   ..���    ?  `  a  �  �  �  �  �  �  �  �  �  �  �  �  �  K  L  {wswwwswwokwwgc_{k �.�
.      �  �  �  �  	r  	s  	�  	�  
J  
K  
�  
�  
�  
�  2  3  �  �{wssswsoswso{{kg{os    .�000  �      �  �  �  �  �  �  �  t  u  �  �  f  g{wswowk`\XwTPsw ...�  
�p	0  g  �  �  (  *  +  ,  -  U  �  �  >  ?  �  �{wsmwwwie_YUQU       000.��	  �  �      :  <  =  >  ?  K  �  �  �  �  �{wsodsss`\sXTP        .    	�
�p	    �  �  �  �                  (  )  =  >  n  o  y  z  |{w{q{m{{{m{i{w{q{m{{   �     (       |  }  ~    �  �  4Q  4S  E{  E�  E�  E�  E�  E�  E�  E�  F�  F�  F�  F�  F�  F�  F�{w{{s{{{{{{{{{{{{{{{{{ .  �         �������� footer odd,of,fo frft�����l4l3l2l1normindex subentry,suchapter name,cn'normal paragraph,np,NP,Normal Paragraphnpk	table3,t3enum. 10,e10hang. indent 0.5",hi1hang. indent 1.0",hi2command line,clindex entry,en	bullet,blfigure caption,fcfooter even,ef,feheader even,eh,hedata link.title,dltprog. listing,plfigure-long,figlinstyletab list 1-in,tl1tab list 2 -in,tl2table caption,tcdata link.byline,dblprog.list(short),plstable of contents,toctab list 3-in,tl3chap.firstpage3,cf3tab list 4-in,tl4normaltable,tl
3rd  title	4th title	index.stytitlepage,t1	2nd titleduties
figure,fig TOC:chapter header,toc1,TOCtitle#TOC:chap subentry,toc2,TOC"chapter"#TOC:group subentry,toc3,TOCcontentsheader odd,oh,hocopyrt.text,ctcopyrt.heads,chTOC:figures,toc4chapter #,c#,chapter numberchap.firstpage1,cf1npshang. indent 2.0",hi3
enum. 1,e1TOC:  Table of Contents,toc table caption-long,tclchap.firstpage2,cf2figure caption-long,fcltitle2,subtitle,t2titlepage date,date
Header odd	table4,t4��   ���������   � 
   �������   �    " � "        �    �                  @  (          �      �                            � "    �             � 
�   �      �@ �  ����������    0 ��     0 �  ��  0 �������    (�   � ��     h0     	  �0�L       @ 0      ��� $@   ��R0    ��00�   ��`
��p  	  
   h�� �0�    N��0N   D��T0     0#�@  
  0$@	         �p  	            ��` ��    �  
�p�  	  ��@     0    @	�p0      � ��p     �� 
���    �  ��
���       � 0    ���  ��T     0 	!  @ �"  ��8	#    �$   0 %   �� �0@ &      �@ 	'  � �(  
   0$@)   0 *     � �0 +   0�0 ,   @�� @ -     X  .  /  ��0@ 0    DN 1   0 2   ��@3   x0 
4    	5   @6  �T7  �0 !�@8  0�� �$ @�:   T 3�- %                  �   �            .� �
� 	
��$  3�   ! " # $%&%'&()*+,�-%/0 24345 6 7 8  :�to produce a file of the differences between theandupdate theoriginal file on the remote machine to include the edits you made to the corresponding file on your local machine.your original filefile2.	Make a dDBefore you create an updatefileWhenare ready to fileby: usingfilecurrent fileversion     �  E�  E�  E�  E�  E�  E�  E�  E�  E�  E�  E�  E�  E�  E�  E�  E�  F�  F�  F�  F�  F�  F�  F�  F�               yw  w   � @ Stage 3:   a File on the JulyJulyof the two applicationsrun  �  �  �  L&          .                 F�  F�  F�  F�  F�  F�  F�  F�  F�  F�  F�  H   HH  HI  HN  H�  H�  H�  H�  H�  H�  H�  H�  H�          y               @   F�  F�  F�  F�  F�  HI  H�  H�  I  IZ  I{  I}  J
  J  J  J  J  J{{{{{ws{{{oo{wdYY
p
�p  00     , though your text files may grow to exceed the capacity of floppy disksmain Setting aside one of the files on each machine to serve as a control copy.3.	the file4control copy.567explicit update filewillunedited both an edited and control file update fileTransferringupdate file all systems on which it runs. To update your original file, i.ble that shown in Figure 1.4. (S.(SD  H�  H�  H�  H�  H�  H�  I  I  I  I  I  I:  IX  IY  IZ  Iz  I{  I|  I}  I  J   J	  J
  J                   y      @ ee also tYou can specify that overwrite the original file with, or that it write the updated version to a new file       J  J  J  J  J  J&  JF  Ju  Jz  J{  K�  K�  K�  K�  K�  K�  K�         y                                            @   J  J  J  J{  K�  K�  K�  K�{wwwwww                                                                                   If you specify the former, after running NCSA XFIX       �  �   �   �           �   �  &  8  �  �  �&          .                    �     �               *   [   f   i   k   �   �   �   �   �   �   �     �  �  {  E�  F�  H�  J  K� i j k l � � � �     �  ?    �  g  �  �  |  F�  J  K� m n o p q r s t u � �            " e g ��      D  �  �  �  �  �  �  d  g  z  �  �  �  :  C  D  �  �  %  &  �  �  �  �  �        $  %  :  j  �  �  �  �  �  �  �  �  	  	  	  	=  	b  	s  	�  	�  	�  
  
  
  
   
V  
X  
Y  
v  
�  
�    $  '  (  (  1  Y  \  ]  u  y  ~  �  �  �  �  �  �  �  �  �  �  �  �    0  4  �    �  �  	      Z  [  `  l  �  �  �  �  �  q  s  �  �  �  �            @  q  }  ~  �  �  �  �  �  �  j  q  �  �  �  �    �  �  �  �  �  �  �  N  R  X  �  �  �  �  �      �  Y  �  �  �  I{     .  �  F�  �  4O  �  �  �  F�  �  �  �  H      �  �  HI  �  �  �  0  �  4Q     9  �  4S     �  �  �  �       J     �  �  �     HN  �  �  �  H�  �  H�     �  �  H�  �  �  �  D�  �  	  �  D�  �  	2  �  H�     	q  �  H�     	t  �  H�  �  	�  �  D�  �  	�  �  E  �  E(  �  EM  �  E^     
I  �  H�     
L     I|  �  I}  �  J   �  L  �  �     J	     
�  �  E{     
�  �  E�     l  �  E�  �  E�     �  �  H�     �     J     �     �  �  E�  �  E�  �  E�  �  E�  �  H�  �  �  �  H�  �  �  �  �  �  H�  �  �  �    �  H�  �  H�     ^  �  u  �  E�     �  �  H�     d  �  �     -  �  o  �  E�  �  |  �  E�     �  �  I  �  �  �  '  �  E�  �  �     <     �  �  I  �  I  �  I:  �  [  �  IX     �     J  �  J  �  �  �  J&  �  	  �  E�  �  A  �  J  �  JF  �  Ju  �  ]  �  K�     K�  �  w  �  E�     �  �  F�  �    �  F�     +  �  �  �  I  �  
  �  IZ     �.   �     K�     �     F�               F�     s     y     �                                                           