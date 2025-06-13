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
;;; Utilities for Apple event calls
;;;

(defmacro unix-filename-to-FSSpec (filename spec)
  (` (let ((temp (unix-filename-to-FSSpec-internal (, filename))))
	   (setq (, spec) (car temp))
	   (cdr temp))))

(defun deref (address)
  (extract-internal address 0 'unsigned-long))

(defun throw-err (err)
  (if (not (zerop err))
      (throw 'panic err)))

;;;
;;; A convenient way to create Apple events to a specific target
;;;

(defun AE-create-apple-event-internal (targetID eventClass eventID)
  (let* ((target (make-string (c:sizeof 'AEDesc) 0))
		 have-target
		 (event (make-string (c:sizeof 'AppleEvent) 0))
		 have-event
		 (actualSize (make-string 4 0))
		 (resultType (make-string 4 0))
		 (transactionID (make-string 4 0))
		 (result
		  (catch 'panic
			(throw-err (AECreateDesc typeApplSignature targetID 4 target))
			(setq have-target t)
			(throw-err (AECreateAppleEvent eventClass eventID target
										   kAutoGenerateReturnID
										   kAnyTransactionID event))
			(setq have-event t)
			(throw-err (AEGetAttributePtr event keyReturnIDAttr typeLongInteger
										  resultType transactionID 4 actualSize))
			noErr)))
    (if have-target (AEDisposeDesc target))
    (if (zerop result)
		(cons result (cons event (extract-internal transactionID 0 'long)))
      (cons result (cons nil nil)))))

(defmacro AE-create-apple-event (targetID eventClass eventID event transactionID)
  (` (let ((temp (AE-create-apple-event-internal (, targetID)
												 (, eventClass) (, eventID))))
       (setq (, event) (car (cdr temp)))
       (setq (, transactionID) (cdr (cdr temp)))
       (car temp))))

(defvar AE-history nil "A list of Apple events sent from Emacs.	 This list is used to associate replies.")

(defun AE-have-required-parameters (event)
  (let* ((actualSize (make-string 4 0))
		 (returnedType (make-string 4 0))
		 (data (make-string 0 0))
		 (err (AEGetAttributePtr event keyMissedKeywordAttr typeWildCard
								 returnedType data 0 actualSize)))
    (cond
     ((= err errAEDescNotFound)
      noErr)
     ((= err noErr)
      errAEEventNotHandled)
     (t
      err))))

(defun short-time-string ()
  "Returns a string representing the time of day."
  (let* ((s (current-time-string))
		 (blank-3 10)
		 (blank-4 19))
    (substring s (1+ blank-3) blank-4)))

(defun insert-reply (&rest s)
  (let ((errors-buffer (get-buffer-create "*replies*"))
		(original-window (selected-window)))
    (if (not (get-buffer-window errors-buffer))
		(let ((errors-window 
			   (if (eq (next-window) original-window)
				   (split-window original-window
								 (- (window-height (selected-window)) 8))
				 (display-buffer errors-buffer))))
		  (set-window-buffer errors-window errors-buffer)))
    (select-window (get-buffer-window errors-buffer))
    (set-buffer errors-buffer)
    (goto-char (point-max))
    (apply (function insert) s)
    (select-window original-window)))

(defun do-AE-answer (event reply refCon)
  (let* ((actualSize (make-string 4 0))
		 (resultType (make-string 4 0))
		 (transactionID-string (make-string 4 0))
		 (err (AEGetAttributePtr event keyReturnIDAttr typeLongInteger
								 resultType transactionID-string 4 actualSize)))
    (if (not (zerop err))
		(insert-reply "Received an AppleEvent reply, but cannot determine original request\n")
      (let* ((transactionID-number (extract-internal transactionID-string 0 'long))
			 (history (assoc transactionID-number AE-history)))
		(if (not history)
			(insert-reply "Received an AppleEvent reply with ID "
						  (int-to-string transactionID-number)
						  ", but cannot determine original request\n")
		  (let ((handler (cdr (assoc 'handler (cdr history)))))
			(if handler
				(funcall handler event history)
			  noErr)))))))

;;;
;;; A simple reply handler
;;;

(defun announce-reply (history)
  (let ((description (cdr (assoc 'description (cdr history)))))
    (insert-reply "Reply at " (short-time-string)
				  (if description (concat " to Ò" description "Ó") "")
				  ":\n")))

(defun do-simple-reply-internal (event history show-all-replies)
  (let* ((error-number-data (make-string 4 0))
		 (returnedType (make-string 4 0))
		 (actualSize (make-string 4 0))
		 (err (AEGetParamPtr event keyErrorNumber typeLongInteger returnedType
							 error-number-data (length error-number-data) actualSize)))
    (cond
     ((zerop err)
	  (announce-reply history)
      (let ((error-number (extract-internal error-number-data 0 'long)))
		(insert-reply "	 Error " (error-string error-number) "\n"))
      noErr)
     ((= err errAEDescNotFound)
	  (if show-all-replies
		  (progn
			(announce-reply history)
			(insert-reply "  No data was sent in reply.\n")))
	  noErr)
     (t
	  (announce-reply history)
      (insert-reply "  Could not read result, got error " (error-string err) ".\n")
      err))))

(defun do-simple-reply (event history)
  (do-simple-reply-internal event history t))

(defun do-error-reply (event history)
  (do-simple-reply-internal event history nil))

(defun error-string (error-number)
  (concat (int-to-string error-number)
		  (let ((s (lookup-error-string error-number)))
			(if s (concat ", Ò" s "Ó") ""))))

(defun report-error-in-message-line (err)
  (if (not (zerop err))
      (let ((error-string (lookup-error-string err)))
		(message (concat "While sending Apple event, got error "
						 (int-to-string err)
						 (if error-string (concat ", Ò" error-string "Ó") ""))))))

(defun launch-application (name)
  "Launch the application named APPLICATION in ~/etc."
  (let* (target
		 event
		 have-event
		 (reply (make-string (c:sizeof 'AppleEvent) 0))
		 transactionID
		 spec
		 (alias-string (make-string 4 0))
		 alias-handle
		 alias-data
		 (ae-list (make-string (c:sizeof 'AEDescList) 0))
		 have-ae-list
		 (result
		  (catch 'panic
			(progn
			  (throw-err (AE-create-apple-event "MACS" kAEFinderEvents kAEOpenSelection
												event transactionID))
			  (setq have-event t)
			  
			  (throw-err (unix-filename-to-FSSpec "/bin" spec))
			  (throw-err (NewAlias 0 spec alias-string))
			  (setq alias-handle (extract-internal alias-string 0 'unsigned-long))
			  (HLock alias-handle)
			  (let ((alias-size (extract-internal (deref alias-handle) 4 'short)))
				(setq alias-data (extract-internal (deref alias-handle) 0 'string alias-size )))
			  (DisposHandle alias-handle)
			  (throw-err (AEPutParamPtr event keyDirectObject typeAlias
										alias-data (length alias-data)))
			  
			  (throw-err (unix-filename-to-FSSpec (concat "/bin/" name) spec))
			  (throw-err (NewAliasMinimal spec alias-string))
			  (setq alias-handle (extract-internal alias-string 0 'unsigned-long))
			  (HLock alias-handle)
			  (let ((alias-size (extract-internal (deref alias-handle) 4 'short)))
				(setq alias-data (extract-internal (deref alias-handle) 0 'string alias-size )))
			  (DisposHandle alias-handle)
			  (throw-err (AECreateList 0 0 0 ae-list))
			  (setq have-ae-list t)
			  (throw-err (AEPutPtr ae-list 0 typeAlias alias-data (length alias-data)))
			  (throw-err (AEPutParamDesc event keySelection ae-list))
			  
			  (throw-err (AESend event reply (+ kAEQueueReply kAENeverInteract)
								 kAENormalPriority kAEDefaultTimeout 0 0))
			  (setq AE-history (cons (cons transactionID
										   (list
											(cons 'description (concat "launch " name))))
									 AE-history))
			  noErr))))
    (if have-event (AEDisposeDesc event))
    (if have-ae-list (AEDisposeDesc ae-list))
    result))
    
(AEInstallEventHandler kCoreEventClass kAEAnswer 'do-AE-answer 0 0)
