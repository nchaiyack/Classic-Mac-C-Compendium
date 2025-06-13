;;;
;;; File system errors
;;;

(defconst noErr			    0	"no error")
(defconst dskFulErr		    -34 "disk full")
(defconst nsvErr		    -35 "no such volume")
(defconst fnfErr		    -43 "file not found")
(defconst fBsyErr		    -47 "file is busy")
(defconst paramErr			-50 "parameter error")
(defconst eLenErr			-92 "buffer too big to send")

(defconst userCanceledErr	-128 "user canceled")

(defconst bufferIsSmall		  		-607)
(defconst noOutstandingHLE			-608)
(defconst connectionInvalid			-609	"nonexistent target or session ID")
(defconst noUserInteractionAllowed  -610)

;;;
;;; Apple event errors
;;;

(defconst errAECoercionFail			-1700)
(defconst errAEDescNotFound			-1701	"descriptor record not found")
(defconst errAECorruptData			-1702)
(defconst errAEWrongDataType		-1703)
(defconst errAENotAEDesc			-1704)
(defconst errAEBadListItem			-1705)
(defconst errAENewerVersion			-1706)
(defconst errAENotAppleEvent		-1707)
(defconst errAEEventNotHandled		-1708	"event not handled")
(defconst errAEReplyNotValid		-1709)
(defconst errAEUnknownSendMode		-1710)
(defconst errAEWaitCanceled			-1711)
(defconst errAETimeout				-1712)
(defconst errAENoUserInteraction	-1713	"no user interaction allowed")
(defconst errAENotASpecialFunction	-1714)
(defconst errAEParamMissed			-1715)
(defconst errAEUnknownAddressType	-1716)
(defconst errAEHandlerNotFound		-1717)
(defconst errAEReplyNotArrived		-1718)
(defconst errAEIllegalIndex			-1719)

(defconst errAEImpossibleRange		-1720)
(defconst errAEWrongNumberArgs		-1721)
(defconst errAEAccessorNotFound		-1723)
(defconst errAENoSuchLogical		-1725)
(defconst errAEBadTestKey			-1726)
(defconst errAENotAnObjSpec			-1727)
(defconst errAENoSuchObject			-1728	"no such object")
(defconst errAENegativeCount		-1729)
(defconst errAEEmptyListContainer	-1730)

(defconst errAEBadKeyForm -10002)
(defconst errAECantHandleClass -10010)
(defconst errAECantSupplyType -10009)
(defconst errAEEventFailed -10000)
(defconst errAEIndexTooLarge -10007)
(defconst errAEInTransaction -10011)
(defconst errAELocalOnly -10016)
(defconst errAENoSuchTransaction -10012)
(defconst errAENotAnElement -10008)
(defconst errAENotASingleObject -10014)
(defconst errAENotModifiable -10003)
(defconst errAENoUserSelection -10013)
(defconst errAEPrivilegeError -10004)
(defconst errAEReadDenied -10005)
(defconst errAETypeError -10001)
(defconst errAEWriteDenied -10006)

;;;
;;; Elisp support
;;;

;;; Using a property might be a better way of doing this
(defvar documented-error-list
  (mapcar (function (lambda (x) (cons (eval x) x)))
		  '(dskFulErr nsvErr fnfErr fBsyErr paramErr eLenErr userCanceledErr connectionInvalid
					  errAEEventNotHandled errAENoSuchObject errAEDescNotFound
					  errAENoUserInteraction)))

(defun lookup-error-string (err)
  (let ((match (assoc err documented-error-list)))
	(if match
		(get (cdr match) 'variable-documentation)
	  nil)))
