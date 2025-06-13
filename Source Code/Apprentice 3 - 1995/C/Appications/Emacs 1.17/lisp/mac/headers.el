;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1993, 1994 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

;;;
;;; Header file for Macintosh traps
;;;
;;; This file is included by site-init.el, so that this file is part of
;;; the dump and available at startup.	If you hack it, you can make
;;; your changes part of the dump by once launching Emacs and holding
;;; down the option key.
;;;

;;;
;;; *** From Processes.h
;;;

(defconst kNoProcess 0)
(defconst kSystemProcess 1)
(defconst kCurrentProcess 2)

;;;
;;; *** From EPPC.h
;;;

(defconst receiverIDMask		    61440 "0x0000F000")
(defconst receiverIDisPSN	    	32768 "0x00008000")
(defconst receiverIDisSignature	    28672 "0x00007000")
(defconst receiverIDisSessionID	    24576 "0x00006000")
(defconst receiverIDisTargetID	    20480 "0x00005000")

(defconst systemOptionsMask	    3840 "0x00000F00")
(defconst nReturnReceipt	    512 "0x00000200")

(defconst priorityMask		    255 "0x000000FF")
(defconst nAttnMsg		    	1 "0x00000001")

;;;
;;; *** From AEObjects.h ***
;;;

(defconst kAEAND					"AND ")
(defconst kAEOR						"OR	 ")
(defconst kAENOT					"NOT ")
(defconst kAEFirst					"firs")
(defconst kAELast					"last")
(defconst kAEMiddle					"midd")
(defconst kAEAny					"any ")
(defconst kAEAll					"all ")
(defconst kAENext					"next")
(defconst kAEPrevious				"prev")
(defconst keyAEDesiredClass			"want")
(defconst keyAEContainer			"from")
(defconst keyAEKeyForm				"form")
(defconst keyAEKeyData				"seld")
(defconst keyAERangeStart			"star")
(defconst keyAERangeStop			"stop")
(defconst formAbsolutePosition		"indx")
(defconst formRelativePosition		"rele")
(defconst formTest					"test")
(defconst formRange					"rang")
(defconst formPropertyID			"prop")
(defconst formName					"name")
(defconst typeObjectSpecifier		"obj ")
(defconst typeObjectBeingExamined	"exmn")
(defconst typeCurrentContainer		"ccnt")
(defconst typeToken					"toke")
(defconst typeRelativeDescriptor	"rel ")
(defconst typeAbsoluteOrdinal		"abso")
(defconst typeIndexDescriptor		"inde")
(defconst typeRangeDescriptor		"rang")
(defconst typeLogicalDescriptor		"logi")
(defconst typeCompDescriptor		"cmpd")
(defconst keyAECompOperator			"relo")
(defconst keyAELogicalTerms			"term")
(defconst keyAELogicalOperator		"logc")
(defconst keyAEObject1				"obj1")
(defconst keyAEObject2				"obj2")
(defconst keyDisposeTokenProc		"xtok")
(defconst keyAECompareProc			"cmpr")
(defconst keyAECountProc			"cont")
(defconst keyAEMarkTokenProc		"mkid")
(defconst keyAEMarkProc				"mark")
(defconst keyAEAdjustMarksProc		"adjm")
(defconst keyAEGetErrDescProc		"indc")

(defconst kAEIDoMinimum			0)
(defconst kAEIDoWhose			1)
(defconst kAEIDoMarking			4)

(defconst typeWhoseDescriptor		"whos")
(defconst formWhose					"whos")
(defconst typeWhoseRange			"wrng")
(defconst keyAEWhoseRangeStart		"wstr")
(defconst keyAEWhoseRangeStop		"wstp")
(defconst keyAEIndex				"kidx")
(defconst keyAETest					"ktst")

;;;
;;; Other constants.  By now we should be using c:sizeof instead of these.
;;;

;(defconst sizeof-char 1)
;(defconst sizeof-short 2)
;(defconst sizeof-long 4)
;(defconst sizeof-int 4)
;(defconst sizeof-AEDesc 8)
;(defconst sizeof-AEDescList 8)
;(defconst sizeof-AERecord 8)
;(defconst sizeof-AppleEvent 8)
;(defconst sizeof-FInfo 16)
