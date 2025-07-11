;;; The Allied Order-of-Battle for Normandy.

(game-module "ob-nor-allied"
  (default-base-module "ww2-bn")
  )

;; Coding is ccddrrrrb, country/division/regiment/battalion

; top-level codes
; 10,11 US inf/para
; 12 US armor
; 20,21 GB inf/para
; 22 GB armor
; 25 Can inf
; 26 Can armor
; 30 Fr
; 31 Po
; 32 Be
; 33 Dutch

(unit-defaults (s 1))

(hq-inf-div (nb 100100000) (n "1"))
(inf-mot (nb 100100161) (n "1/16"))
(inf-mot (nb 100100162) (n "2/16"))
(inf-mot (nb 100100163) (n "3/16"))
(inf-mot (nb 100100181) (n "1/18"))
(inf-mot (nb 100100182) (n "2/18"))
(inf-mot (nb 100100183) (n "3/18"))
(inf-mot (nb 100100261) (n "1/26"))
(inf-mot (nb 100100262) (n "2/26"))
(inf-mot (nb 100100263) (n "3/26"))
(arty-towed (nb 100100050) (n "5"))
(arty-towed (nb 100100070) (n "7"))
(arty-towed (nb 100100320) (n "32"))
(arty-towed (nb 100100330) (n "33"))

(hq-inf-div (nb 100200000) (n "2"))
(inf-mot (nb 100200091) (n "1/9"))
(inf-mot (nb 100200092) (n "2/9"))
(inf-mot (nb 100200093) (n "3/9"))
(inf-mot (nb 100200231) (n "1/23"))
(inf-mot (nb 100200232) (n "2/23"))
(inf-mot (nb 100200233) (n "3/23"))
(inf-mot (nb 100200381) (n "1/38"))
(inf-mot (nb 100200382) (n "2/38"))
(inf-mot (nb 100200383) (n "3/38"))
(arty-towed (nb 100200120) (n "12"))
(arty-towed (nb 100200150) (n "15"))
(arty-towed (nb 100200370) (n "37"))
(arty-towed (nb 100200380) (n "38"))

(hq-inf-div (nb 100400000))
(inf-mot (nb 100400081))
(inf-mot (nb 100400082))
(inf-mot (nb 100400083))
(inf-mot (nb 100400121))
(inf-mot (nb 100400122))
(inf-mot (nb 100400123))
(inf-mot (nb 100400221))
(inf-mot (nb 100400222))
(inf-mot (nb 100400223))
(arty-towed (nb 100400200) (n "20"))
(arty-towed (nb 100400290) (n "29"))
(arty-towed (nb 100400420) (n "42"))
(arty-towed (nb 100400440) (n "44"))

(hq-inf-div (nb 100500000))
(inf-mot (nb 100500021) (n "1/2"))
(inf-mot (nb 100500022) (n "2/2"))
(inf-mot (nb 100500023) (n "3/2"))
(inf-mot (nb 100500101) (n "1/10"))
(inf-mot (nb 100500102) (n "2/10"))
(inf-mot (nb 100500103) (n "3/10"))
(inf-mot (nb 100500111) (n "1/11"))
(inf-mot (nb 100500112) (n "2/11"))
(inf-mot (nb 100500113) (n "3/11"))
(arty-towed (nb 100500190))
(arty-towed (nb 100500210))
(arty-towed (nb 100500460))
(arty-towed (nb 100500500))

(hq-inf-div (nb 100800000))
(inf-mot (nb 100800131))
(inf-mot (nb 100800132))
(inf-mot (nb 100800133))
(inf-mot (nb 100800281))
(inf-mot (nb 100800282))
(inf-mot (nb 100800283))
(inf-mot (nb 100800341))
(inf-mot (nb 100800342))
(inf-mot (nb 100800343))
(arty-towed (nb 100800280))
(arty-towed (nb 100800430))
(arty-towed (nb 100800450))
(arty-towed (nb 100800560))

(hq-inf-div (nb 100900000))
(inf-mot (nb 100900391))
(inf-mot (nb 100900392))
(inf-mot (nb 100900393))
(inf-mot (nb 100900471))
(inf-mot (nb 100900472))
(inf-mot (nb 100900473))
(inf-mot (nb 100900601))
(inf-mot (nb 100900602))
(inf-mot (nb 100900603))
(arty-towed (nb 100900260))
(arty-towed (nb 100900340))
(arty-towed (nb 100900600))
(arty-towed (nb 100900840))

(hq-inf-div (nb 102800000))
(inf-mot (nb 102801091))
(inf-mot (nb 102801092))
(inf-mot (nb 102801093))
(inf-mot (nb 102801101))
(inf-mot (nb 102801102))
(inf-mot (nb 102801103))
(inf-mot (nb 102801121))
(inf-mot (nb 102801122))
(inf-mot (nb 102801123))
(arty-towed (nb 102801070))
(arty-towed (nb 102801080))
(arty-towed (nb 102801090))
(arty-towed (nb 102802290))

(hq-inf-div (nb 102900000))
(inf-mot (nb 102901151))
(inf-mot (nb 102901152))
(inf-mot (nb 102901153))
(inf-mot (nb 102901161))
(inf-mot (nb 102901162))
(inf-mot (nb 102901163))
(inf-mot (nb 102901751))
(inf-mot (nb 102901752))
(inf-mot (nb 102901753))
(cav-mech (nb 102900000))
(arty-towed (nb 102901100))
(arty-towed (nb 102901110))
(arty-towed (nb 102902240))
(arty-towed (nb 102902270))

(hq-inf-div (nb 103000000))
(inf-mot (nb 103001171))
(inf-mot (nb 103001172))
(inf-mot (nb 103001173))
(inf-mot (nb 103001191))
(inf-mot (nb 103001192))
(inf-mot (nb 103001193))
(inf-mot (nb 103001201))
(inf-mot (nb 103001202))
(inf-mot (nb 103001203))
(arty-towed (nb 103001130))
(arty-towed (nb 103001180))
(arty-towed (nb 103001970))
(arty-towed (nb 103002300))

(hq-inf-div (nb 103500000))
(inf-mot (nb 103501341))
(inf-mot (nb 103501342))
(inf-mot (nb 103501343))
(inf-mot (nb 103501371))
(inf-mot (nb 103501372))
(inf-mot (nb 103501373))
(inf-mot (nb 103503201))
(inf-mot (nb 103503202))
(inf-mot (nb 103503203))
(arty-towed (nb 103501270))
(arty-towed (nb 103501610))
(arty-towed (nb 103502160))
(arty-towed (nb 103502190))

(hq-inf-div (nb 107900000))
(inf-mot (nb 107903131))
(inf-mot (nb 107903132))
(inf-mot (nb 107903133))
(inf-mot (nb 107903141))
(inf-mot (nb 107903142))
(inf-mot (nb 107903143))
(inf-mot (nb 107903151))
(inf-mot (nb 107903152))
(inf-mot (nb 107903153))
(arty-towed (nb 107903100))
(arty-towed (nb 107903110))
(arty-towed (nb 107903120))
(arty-towed (nb 107909040))

(hq-inf-div (nb 108000000))
(inf-mot (nb 108003171))
(inf-mot (nb 108003172))
(inf-mot (nb 108003173))
(inf-mot (nb 108003181))
(inf-mot (nb 108003182))
(inf-mot (nb 108003183))
(inf-mot (nb 108003191))
(inf-mot (nb 108003192))
(inf-mot (nb 108003193))
(arty-towed (nb 108003130))
(arty-towed (nb 108003140))
(arty-towed (nb 108003150))
(arty-towed (nb 108009050))

(hq-inf-div (nb 108200000))
(inf-mot (nb 108203251))
(inf-mot (nb 108203252))
(inf-mot (nb 108203253))
(inf-mot (nb 108205041))
(inf-mot (nb 108205042))
(inf-mot (nb 108205043))
(inf-mot (nb 108205051))
(inf-mot (nb 108205052))
(inf-mot (nb 108205053))
(arty-towed (nb 108203190))
(arty-towed (nb 108203200))
(arty-towed (nb 108203760))
(arty-towed (nb 108204560))

(hq-inf-div (nb 108300000))
(inf-mot (nb 108303291))
(inf-mot (nb 108303292))
(inf-mot (nb 108303293))
(inf-mot (nb 108303301))
(inf-mot (nb 108303302))
(inf-mot (nb 108303303))
(inf-mot (nb 108303311))
(inf-mot (nb 108303312))
(inf-mot (nb 108303313))
(arty-towed (nb 108303220))
(arty-towed (nb 108303230))
(arty-towed (nb 108303240))
(arty-towed (nb 108309080))

(hq-inf-div (nb 109000000))
(inf-mot (nb 109003571))
(inf-mot (nb 109003572))
(inf-mot (nb 109003573))
(inf-mot (nb 109003581))
(inf-mot (nb 109003582))
(inf-mot (nb 109003583))
(inf-mot (nb 109003591))
(inf-mot (nb 109003592))
(inf-mot (nb 109003593))
(cav-mech (nb 109000000))
(arty-towed (nb 109003430))
(arty-towed (nb 109003440))
(arty-towed (nb 109003450))
(arty-towed (nb 109009150))

(hq-inf-div (nb 110100000))
(inf-mot (nb 110103271))
(inf-mot (nb 110103272))
(inf-mot (nb 110103273))
(inf-mot (nb 110104011))
(inf-mot (nb 110104012))
(inf-mot (nb 110104013))
(inf-mot (nb 110105021))
(inf-mot (nb 110105022))
(inf-mot (nb 110105023))
(arty-towed (nb 110103210))
(arty-towed (nb 110103770))
(arty-towed (nb 110104630))
(arty-towed (nb 110109070))

(hq-armor-div (nb 120200000))
(inf-armored (nb 120200411))
(inf-armored (nb 120200412))
(inf-armored (nb 120200413))
(arty-armored (nb 120200140))
(arty-armored (nb 120200780))
(arty-armored (nb 120200920))
(tank (nb 120200661))
(tank (nb 120200662))
(tank (nb 120200663))
(tank (nb 120200671))
(tank (nb 120200672))
(tank (nb 120200673))

(hq-armor-div (nb 120300000))
(inf-armored (nb 120300361))
(inf-armored (nb 120300362))
(inf-armored (nb 120300363))
(arty-armored (nb 120300540))
(arty-armored (nb 120300670))
(arty-armored (nb 120303910))
(tank (nb 120300321))
(tank (nb 120300322))
(tank (nb 120300323))
(tank (nb 120300331))
(tank (nb 120300332))
(tank (nb 120300333))

(hq-armor-div (nb 120400000))
(inf-armored (nb 120400100))
(inf-armored (nb 120400510))
(inf-armored (nb 120400530))
(arty-armored (nb 120400220))
(arty-armored (nb 120400660))
(arty-armored (nb 120400940))
(tank (nb 120400080))
(tank (nb 120400350))
(tank (nb 120400370))

(hq-armor-div (nb 120500000))
(inf-armored (nb 120500150))
(inf-armored (nb 120500460))
(inf-armored (nb 120500470))
(cav-mech (nb 120500850))
(arty-armored (nb 120500470))
(arty-armored (nb 120500710))
(arty-armored (nb 120500950))
(tank (nb 120500100))
(tank (nb 120500340))
(tank (nb 120500810))

(hq-armor-div (nb 120600000))
(inf-armored (nb 120600090))
(inf-armored (nb 120600440))
(inf-armored (nb 120600500))
(arty-armored (nb 120601280))
(arty-armored (nb 120602120))
(arty-armored (nb 120602310))
(tank (nb 120600150))
(tank (nb 120600680))
(tank (nb 120600690))
(cav-mech (nb 120600860))

(hq-armor-div (nb 120700000))
(inf-armored (nb 120700230))
(inf-armored (nb 120700380))
(inf-armored (nb 120700480))
(arty-armored (nb 120704340))
(arty-armored (nb 120704400))
(arty-armored (nb 120704890))
(tank (nb 120700170))
(tank (nb 120700310))
(tank (nb 120700400))

; Non-divisional units.

(inf-mot (nb 100000990))

(cav-mech (nb 100000280))

(arty-mortar (nb 100000810))

(arty-armored (nb 100000580))
(arty-armored (nb 100000620))
(arty-armored (nb 100000650))
(arty-armored (nb 100000830))
(arty-armored (nb 100000870))
(arty-armored (nb 100001740))
(arty-armored (nb 100002530))
(arty-armored (nb 100002580))
(arty-armored (nb 100002750))
(arty-armored (nb 100004000))
(arty-armored (nb 100005570))
(arty-armored (nb 100006950))
(arty-armored (nb 100006960))
(arty-armored (nb 100009870))
(arty-armored (nb 100009910))

(tank (nb 100000700))
(tank (nb 100007010))
(tank (nb 100007020))
(tank (nb 100007070))
(tank (nb 100007090))
(tank (nb 100007120))
(tank (nb 100007350))
(tank (nb 100007360))
(tank (nb 100007370))
(tank (nb 100007410))
(tank (nb 100007430))
(tank (nb 100007440))
(tank (nb 100007450))
(tank (nb 100007460))
(tank (nb 100007470))
(tank (nb 100007480))
(tank (nb 100007490))
(tank (nb 100007590))
(tank (nb 100007610))
(tank (nb 100007740))

(arty-sp-at (nb 100006020))
(arty-sp-at (nb 100006140))
(arty-sp-at (nb 100006280))
(arty-sp-at (nb 100006290))
(arty-sp-at (nb 100006340))
(arty-sp-at (nb 100006440))
(arty-sp-at (nb 100006540))
(arty-sp-at (nb 100007020))
(arty-sp-at (nb 100007030))
(arty-sp-at (nb 100007040))
(arty-sp-at (nb 100007730))
(arty-sp-at (nb 100007740))
(arty-sp-at (nb 100008030))
(arty-sp-at (nb 100008170))
(arty-sp-at (nb 100008180))
(arty-sp-at (nb 100008930))
(arty-sp-at (nb 100008990))

; British.

(hq-inf-div (nb 200300000))
(inf-mot (nb 200300000) (n "1 SLR"))
(inf-mot (nb 200300000) (n "2 EYR"))
(inf-mot (nb 200300000) (n "2 LR"))
(inf-mot (nb 200300000) (n "2 RWR"))
(inf-mot (nb 200300000) (n "1 KONB"))
(inf-mot (nb 200300000) (n "1 RNR"))
(inf-mot (nb 200300000) (n "2 RUR"))
(inf-mot (nb 200300000) (n "1 SR"))
(inf-mot (nb 200300000) (n "2 KSLI"))
(arty-towed (nb 200300070))
(arty-towed (nb 200300330))
(arty-towed (nb 200300760))

(hq-inf-div (nb 200600000))

(hq-inf-div (nb 201500000))
(inf-mot (nb 201500000) (n "2 ASH"))
(inf-mot (nb 201500000) (n "2 GH"))
(inf-mot (nb 201500000) (n "2 GH"))
(inf-mot (nb 201500000) (n "6 RSF"))
(inf-mot (nb 201500000) (n "7 KOSB"))
(inf-mot (nb 201500000) (n "7 SH"))
(inf-mot (nb 201500000) (n "8 RSF"))
(inf-mot (nb 201500000) (n "9 C"))
(inf-mot (nb 201500000) (n "10 HLI"))
(arty-towed (nb 201501310))
(arty-towed (nb 201501810))
(arty-towed (nb 201501900))

(hq-inf-div (nb 204300000))
(inf-mot (nb 204300000) (n "4 SLI"))
(inf-mot (nb 204300000) (n "7 SLI"))
(inf-mot (nb 204300000) (n "1 WR"))
(inf-mot (nb 204300000) (n "4 WR"))
(inf-mot (nb 204300000) (n "5 WR"))
(inf-mot (nb 204300000) (n "7 HR"))
(inf-mot (nb 204300000) (n "4 DR"))
(inf-mot (nb 204300000) (n "5 DR"))
(inf-mot (nb 204300000) (n "5 DCLI"))
(arty-towed (nb 204300940))
(arty-towed (nb 204301120))
(arty-towed (nb 204301790))

(hq-inf-div (nb 204900000))
(inf-mot (nb 204900000) (n "10 DLI"))
(inf-mot (nb 204900000) (n "11 DLI"))
(inf-mot (nb 204900000) (n "1 TS"))
(inf-mot (nb 204900000) (n "4 LR"))
(inf-mot (nb 204900001) (n "1/4 KO"))
(inf-mot (nb 204900000) (n "HYLR"))
(inf-mot (nb 204900000) (n "11 RSF"))
(inf-mot (nb 204900000) (n "6 DWR"))
(inf-mot (nb 204900000) (n "7 DWR"))
(arty-towed (nb 204900690))
(arty-towed (nb 204901430))
(arty-towed (nb 204901850))

(hq-inf-div (nb 205000000))

(hq-inf-div (nb 205100000))

(hq-inf-div (nb 205300000))

(hq-inf-div (nb 205900000))

(hq-armor-div (nb 220100000) (n "Guards"))

(hq-armor-div (nb 220700000))

(hq-armor-div (nb 221100000))
(inf-mot (nb 221100000))
(inf-mot (nb 221100000))
(inf-mot (nb 221100000))
(inf-mot (nb 221100000))
(arty-towed (nb 221101510))
(arty-armored (nb 221100130))
(tank (nb 221100000) (n "23 H"))
(tank (nb 221100000) (n "FFY"))
(tank (nb 221100000) (n "3 RTR"))
(tank (nb 221100000) (n "2 NY"))
(tank (nb 221100000) (n "1 N/33"))
(tank (nb 221100000) (n "144/33"))
(tank (nb 221100000) (n "148/33"))

(hq-armor-div (nb 227900000) (n "specialized"))

; Nondivisional.

(arty-towed (nb 200000010))
(arty-towed (nb 200000070))
(arty-towed (nb 200000090))
(arty-towed (nb 200000100))
(arty-towed (nb 200000110))
(arty-towed (nb 200000130))
(arty-towed (nb 200000150))
(arty-towed (nb 200000250))
(arty-towed (nb 200000510))
(arty-towed (nb 200000520))
(arty-towed (nb 200000530))
(arty-towed (nb 200000590))
(arty-towed (nb 200000610))
(arty-towed (nb 200000630))
(arty-towed (nb 200000640))
(arty-towed (nb 200000650))
(arty-towed (nb 200000670))
(arty-towed (nb 200000720))
(arty-towed (nb 200000770))
(arty-towed (nb 200000790))
(arty-towed (nb 200000840))
(arty-towed (nb 200001070))
(arty-towed (nb 200001210))
(arty-towed (nb 200001460))

(arty-at (nb 200000060))

(arty-armored (nb 200000040) (n "4 RH"))

(tank (nb 200001070) (n ""))
(tank (nb 200001470) (n ""))
(tank (nb 200001530) (n ""))

(tank (nb 200000016) (n "GG/6"))
(tank (nb 200000026) (n "CG/6"))
(tank (nb 200000036) (n "SG/6"))

; Canadian.

(hq-inf-div (nb 250200000))

(hq-inf-div (nb 250300000))
(inf-mot (nb 250300000) (n "RWR"))
(inf-mot (nb 250300000) (n "RRR"))
(inf-mot (nb 250300000) (n "QORC"))
(inf-mot (nb 250300000) (n "NNR"))
(inf-mot (nb 250300000) (n "CNR"))
(inf-mot (nb 250300000) (n "LRC"))
(inf-mot (nb 250300000) (n "HLIC"))
(inf-mot (nb 250300000) (n "NDGH"))
(inf-mot (nb 250300000) (n "NNSH"))

(hq-armor-div (nb 260400000))

(arty-towed (nb 250000030))
(arty-towed (nb 250000040))
(arty-towed (nb 250000070))

(tank (nb 250000000) (n "6/2Can"))
(tank (nb 250000000) (n "10/2Can"))
(tank (nb 250000000) (n "27/2Can"))

; French.

(hq-armor-div (nb 300200000))
(inf-armored (nb 300200001))
(inf-armored (nb 300200002))
(inf-armored (nb 300200003))
(cav-mech (nb 300200000))
(arty-armored (nb 300200001))
(arty-armored (nb 300200002))
(arty-armored (nb 300200003))
(tank (nb 300200001))
(tank (nb 300200002))
(tank (nb 300200003))

; Polish armored division.

(hq-armor-div (nb 310100000))
(tank (nb 310100010))
(tank (nb 310100020))
(tank (nb 310100240))
(tank (nb 310100100) (n "10 PMR"))
(inf-mot (nb 310100010))
(inf-mot (nb 310100080))
(inf-mot (nb 310100090))
(inf-mot (nb 310100100))
(arty-armored (nb 310100010))
(inf-mg-co (nb 310100010))
(arty-towed (nb 310100020))

; Belgian brigade.

(inf-mot (nb 320000001))
(inf-mot (nb 320000002))
(inf-mot (nb 320000003))

; Dutch brigade (actually just a battalion).

(inf-mot (nb 330000001))
