;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1993, 1994 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

(defconst typeBoolean			"bool")
(defconst typeChar				"TEXT")
(defconst typeSMInt				"shor")
(defconst typeInteger			"long")
(defconst typeSMFloat			"sing")
(defconst typeFloat				"doub")
(defconst typeLongInteger		"long")
(defconst typeShortInteger		"shor")
(defconst typeLongFloat			"doub")
(defconst typeShortFloat		"sing")
(defconst typeExtended			"exte")
(defconst typeComp				"comp")
(defconst typeMagnitude			"magn")
(defconst typeAEList			"list")
(defconst typeAERecord			"reco")
(defconst typeAppleEvent		"aevt")
(defconst typeTrue				"true")
(defconst typeFalse				"fals")
(defconst typeAlias				"alis")
(defconst typeEnumerated		"enum")
(defconst typeType				"type")
(defconst typeAppParameters		"appa")
(defconst typeProperty			"prop")
(defconst typeFSS				"fss ")
(defconst typeKeyword			"keyw")
(defconst typeSectionH			"sect")
(defconst typeWildCard			"****")
(defconst typeApplSignature		"sign")
(defconst typeQDRectangle		"qdrt")
(defconst typeFixed				"fixd")
(defconst typeSessionID			"ssid")
(defconst typeTargetID			"targ")
(defconst typeProcessSerialNumber	"psn ")
(defconst typeNull				"null")
;;; Keywords for Apple event parameters
(defconst keyDirectObject			"----")
(defconst keyErrorNumber			"errn")
(defconst keyErrorString			"errs")
(defconst keyProcessSerialNumber	"psn ")
;;; Keywords for Apple event attributes
(defconst keyTransactionIDAttr		"tran")
(defconst keyReturnIDAttr			"rtid")
(defconst keyEventClassAttr			"evcl")
(defconst keyEventIDAttr			"evid")
(defconst keyAddressAttr			"addr")
(defconst keyOptionalKeywordAttr	"optk")
(defconst keyTimeoutAttr			"timo")
(defconst keyInteractLevelAttr		"inte")
(defconst keyEventSourceAttr		"esrc")
(defconst keyMissedKeywordAttr		"miss")
(defconst keyOriginalAddressAttr	"from")
;;; Keywords for special handlers
(defconst keyPreDispatch			"phac")
(defconst keySelectProc				"selh")
;;; Keyword for recording
(defconst keyAERecorderCount		"recr")
;;; Keyword for version information
(defconst keyAEVersion				"vers")
;;; Event class
(defconst kCoreEventClass			"aevt")
;;; Event ID's
(defconst kAEOpenApplication		"oapp")
(defconst kAEOpenDocuments			"odoc")
(defconst kAEPrintDocuments			"pdoc")
(defconst kAEQuitApplication		"quit")
(defconst kAEAnswer					"ansr")
(defconst kAEApplicationDied		"obit")
;(defconst kAECreatorType			"crea")
;(defconst kAEQuitAll				"quia")
;(defconst kAEShutDown				"shut")
;(defconst kAERestart				"rest")

;;; Constants for use in AESend mode
(defconst kAENoReply				1)
(defconst kAEQueueReply				2)
(defconst kAEWaitReply				3)
(defconst kAENeverInteract			16)
(defconst kAECanInteract			32)
(defconst kAEAlwaysInteract			48)
(defconst kAECanSwitchLayer			64)
(defconst kAEDontReconnect			128)
(defconst kAEWantReceipt			nReturnReceipt)
(defconst kAEDontRecord				4096)
(defconst kAEDontExecute			8196)
;;; Constants for the sendPriority parameter of AESend
(defconst kAENormalPriority			0)
(defconst kAEHighPriority			nAttnMsg)

;;; Constants for recording
(defconst kAEStartRecording			"reca")
(defconst kAEStopRecording			"recc")
(defconst kAENotifyStartRecording	"rec1")
(defconst kAENotifyStopRecording	"rec0")
(defconst kAENotifyRecording		"recr")

;;; Constant for the returnID parameter of AECreateAppleEvent
(defconst kAutoGenerateReturnID -1)
;;; Constant for transaction ID's
(defconst kAnyTransactionID 0)
;;; Constants for timeout durations
(defconst kAEDefaultTimeout		-1)
(defconst kNoTimeOut			-2)



;;; Constants for AEResumeTheCurrentEvent
(defconst kAENoDispatch			0)
(defconst kAEUseStandardDispatch	-1)
;;; Constants for Refcon in AEResumeTheCurrentEvent with kAEUseStandardDispatch */
(defconst kAEDoNotIgnoreHandler		0)
(defconst kAEIgnoreAppPhacHandler	1)
(defconst kAEIgnoreAppEventHandler	2)
(defconst kAEIgnoreSysPhacHandler	4)
(defconst kAEIgnoreSysEventHandler	8)
(defconst kAEIngoreBuiltInEventHandler 16)
(defconst kAEDontDisposeOnResume	"0x80000000") ;; available only in vers 1.0.1 and greater

;;; Apple event manager data types

(c:typedef DescType (array char 4))
(c:typedef AEKeyword (array char 4))

(c:defstruct AEDesc ((DescType descriptorType)
					 (Handle dataHandle)))
(c:defstruct AEKeyDesc ((AEKeyword descKey)
						(AEDesc descContent)))

(c:typedef AEDescList AEDesc)
(c:typedef AERecord AEDescList)
(c:typedef AppleEvent AEDesc)

;pascal OSErr
;AEDisposeDesc( AEDesc *theAEDesc )
;   = {0x303C,0x0204,0xA816};
(deftrap AEDisposeDesc ("303c" "0204" "a816")
  ((theAEDesc address))
  short)
 
;pascal OSErr
;AESend( const AppleEvent *theAppleEvent, AppleEvent *reply,
;	AESendMode sendMode, AESendPriority sendPriority, long timeOutInTicks,
;	 IdleProcPtr idleProc, EventFilterProcPtr filterProc )
;   = {0x303C,0x0D17,0xA816};
(deftrap AESend ("303c" "0d17" "a816")
  ((theAppleEvent address)
   (reply address)
   (sendMode long)
   (sendPriority short)
   (timeOutInTicks long)
   (idleProc long)
   (filterProc long))
  short)

;pascal OSErr
;AECountItems( const AEDescList *theAEDescList, long *theCount )
;   = {0x303C,0x0407,0xA816}; 
(deftrap AECountItems ("303c" "0407" "a816")
  ((theAEDescList address)
   (theCount address))
  short)

;pascal OSErr
;AESizeOfNthItem( const AEDescList *theAEDescList, long index,
;		 DescType *typeCode, Size *dataSize )
;   = {0x303C,0x082A,0xA816};
(deftrap AESizeOfNthItem ("303c" "082a" "a816")
  ((theAEDescList address)
   (index long)
   (typeCode address)
   (dataSize address))
  short)

;pascal OSErr
;AESizeOfParam( const AppleEvent *theAppleEvent, AEKeyword theAEKeyword,
;		DescType *typeCode, Size *dataSize )
;   = {0x303C,0x0829,0xA816};
(deftrap AESizeOfParam ("303c" "0829" "a816")
  ((theAppleEvent address)
   (theAEKeyword immediate-string)
   (typeCode address)
   (dataSize address))
  short)

; pascal OSErr
; AEPutParamPtr( const AppleEvent *theAppleEvent, AEKeyword theAEKeyword,
;		DescType typeCode, const void* dataPtr, Size dataSize )
;   = {0x303C,0x0A0F,0xA816};
(deftrap AEPutParamPtr ("303c" "0a0f" "a816")
  ((theAppleEvent address)
   (theAEKeyword immediate-string)
   (typeCode immediate-string)
   (dataPtr address)
   (dataSize long))
  short)

; pascal OSErr
; AEPutParamDesc( const AppleEvent *theAppleEvent, AEKeyword theAEKeyword,
;		 const AEDesc *theAEDesc )
;   = {0x303C,0x0610,0xA816};
(deftrap AEPutParamDesc ("303c" "0610" "a816")
  ((theAppleEvent address)
   (theAEKeyword immediate-string)
   (theAEDesc address))
  short)

; pascal OSErr
; AEPutPtr( const AEDescList *theAEDescList, long index, DescType typeCode,
;	   const void* dataPtr, Size dataSize )
;   = {0x303C,0x0A08,0xA816};
(deftrap AEPutPtr ("303c" "0a08" "a816")
  ((theAEDescList address)
   (index long)
   (typeCode immediate-string)
   (dataPtr address)
   (dataSize long))
  short)

; pascal OSErr
; AEInstallEventHandler( AEEventClass theAEEventClass, AEEventID theAEEventID,
;			EventHandlerProcPtr handler, long handlerRefcon,
;			Boolean isSysHandler )
;   = {0x303C,0x091F,0xA816};
(deftrap AEInstallEventHandler-internal ("303c" "091f" "a816")
  ((theAEEventClass immediate-string)
   (theAEEventID immediate-string)
   (handler address)
   (handlerRefCon address)
   (isSysHandler short))
  short)

;;; This list is really only used now to protect the cons cells stored in the
;;; refCon slots of the dispatch table from being garbage collected, and for
;;; documentation.
(defvar AE-callback-list nil "The list of Apple event handlers")

;;; This is called from C when receiving an Apple event registered from elisp.
(defun AE-receive-function (event reply refCon)
  (funcall (car refCon) event reply (cdr refCon)))

(defun AEInstallEventHandler (class type callback handlerRefCon isSysHandler)
  (let* ((callback-cons (cons callback handlerRefCon))
		 (err (AEInstallEventHandler-internal class type AE-receive-function
											  callback-cons isSysHandler)))
    (if (not (zerop err))
		err
      (setq AE-callback-list (cons (list class type callback-cons) AE-callback-list))
      noErr)))

; pascal OSErr
; AEGetParamPtr( const AppleEvent *theAppleEvent, AEKeyword theAEKeyword,
;		DescType desiredType, DescType *typeCode, void* dataPtr,
;		Size maximumSize, Size *actualSize )
;   = {0x303C,0x0E11,0xA816};
(deftrap AEGetParamPtr ("303c" "0e11" "a816")
  ((theAppleEvent address)
   (theAEKeyword immediate-string)
   (desiredType immediate-string)
   (typeCode address)
   (dataPtr address)
   (maximumSize long)
   (actualSize address))
  short)

; pascal OSErr
; AEGetAttributePtr( const AppleEvent *theAppleEvent, AEKeyword theAEKeyword,
;		    DescType desiredType, DescType *typeCode, void* dataPtr,
;		    Size maximumSize, Size *actualSize )
;   = {0x303C,0x0E15,0xA816};
(deftrap AEGetAttributePtr ("303c" "0e15" "a816")
  ((theAppleEvent address)
   (theAEKeyword immediate-string)
   (desiredType immediate-string)
   (typeCode address)
   (dataPtr address)
   (maximumSize long)
   (actualSize address))
  short)

; pascal OSErr
; AEGetNthPtr( const AEDescList *theAEDescList, long index, DescType desiredType,
;	      AEKeyword *theAEKeyword, DescType *typeCode, void* dataPtr,
;	      Size maximumSize, Size *actualSize )
;   = {0x303C,0x100A,0xA816}; 
(deftrap AEGetNthPtr ("303c" "100a" "a816")
  ((theAEDescList address)
   (index long)
   (desiredType immediate-string)
   (theAEKeyword address)
   (typeCode address)
   (dataPtr address)
   (maximumSize long)
   (actualSize address))
  short)

; extern pascal OSErr
; AEGetNthDesc(const AEDescList *theAEDescList, long index, DescType desiredType,
; AEKeyword *theAEKeyword, AEDesc *result) THREEWORDINLINE(0x303C, 0x0A0B, 0xA816);
(deftrap AEGetNthDesc ("303c" "0a0b" "a816")
  ((theAEDescList address)
   (index long)
   (desiredType immediate-string)
   (theAEKeyword address)
   (result address))
  short)

; pascal OSErr
; AEGetParamDesc( const AppleEvent *theAppleEvent, AEKeyword theAEKeyword,
;		 DescType desiredType, AEDesc *result )
;   = {0x303C,0x0812,0xA816};
(deftrap AEGetParamDesc ("303c" "0812" "a816")
  ((theAppleEvent address)
   (theAEKeyword immediate-string)
   (desiredType immediate-string)
   (result address))
  short)

; pascal OSErr
; AECreateList( const void* factoringPtr, Size factoredSize, Boolean isRecord,
;	       AEDescList *resultList )
;   = {0x303C,0x0706,0xA816}; 
(deftrap AECreateList ("303c" "0706" "a816")
  ((factoringPtr address)
   (factoredSize long)
   (isRecord char)
   (resultList address))
  short)

; pascal OSErr
; AECreateDesc( DescType typeCode, const void* dataPtr, Size dataSize, AEDesc *result )
;   = {0x303C,0x0825,0xA816};
(deftrap AECreateDesc ("303c" "0825" "a816")
  ((typeCode immediate-string)
   (dataPtr address)
   (dataSize long)
   (result address))
  short)

; pascal OSErr
; AECreateAppleEvent( AEEventClass theAEEventClass, AEEventID theAEEventID,
;		     const AEAddressDesc *target, short returnID,
;		     long transactionID, AppleEvent *result )
;   = {0x303C,0x0B14,0xA816};
(deftrap AECreateAppleEvent ("303c" "0b14" "a816")
  ((theAEEventClass immediate-string)
   (theAEEventID immediate-string)
   (target address)
   (returnID short)
   (transactionID long)
   (result address))
  short)

;extern pascal OSErr AEPutAttributePtr(AppleEvent *theAppleEvent, AEKeyword theAEKeyword,
; DescType typeCode, const void *dataPtr, Size dataSize)
; THREEWORDINLINE(0x303C, 0x0A16, 0xA816);
(deftrap AEPutAttributePtr ("303c" "0a16" "a816")
  ((theAppleEvent address)
   (theAEKeyword immediate-string)
   (typeCode immediate-string)
   (dataPtr address)
   (dataSize long))
  short)

;extern pascal OSErr AEPutAttributeDesc(AppleEvent *theAppleEvent, AEKeyword theAEKeyword,
; const AEDesc *theAEDesc)
; THREEWORDINLINE(0x303C, 0x0627, 0xA816);
(deftrap AEPutAttributeDesc ("303c" "0627" "a816")
  ((theAppleEvent address)
   (theAEKeyword immediate-string)
   (theAEDesc address))
  short)

(defmacro AEGetKeyPtr (theAERecord theAEKeyword desiredType typeCode dataPtr maxSize actualSize)
  (list 'AEGetParamPtr theAERecord theAEKeyword desiredType typeCode dataPtr maxSize actualSize))
