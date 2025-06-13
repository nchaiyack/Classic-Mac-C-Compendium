;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1995 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

(defconst cwEvent "MMPR")
(defconst cwSignatureCW68K "MMCC")
(defconst cwSignatureCWPPC "MPCC")

(defun cw:send-event (eventID event-description use-external-editor enable-debugging filename)
  (let* (event
		 (reply (make-string (c:sizeof 'AppleEvent) 0))
		 (one (make-string (c:sizeof 'char) 1))
		 filespec
		 transactionID
		 (first-target-choice (if cw:68K-first cwSignatureCW68K cwSignatureCWPPC))
		 (second-target-choice (if cw:68K-first cwSignatureCWPPC cwSignatureCW68K))
		 (result
		  (catch 'panic
			(throw-err (AE-create-apple-event first-target-choice cwEvent eventID event transactionID))
			(if filename
				(progn
				  (throw-err (unix-filename-to-FSSpec filename filespec))
				  (throw-err (AEPutParamPtr event keyDirectObject
											typeFSS filespec (length filespec)))))
			(if use-external-editor
				(throw-err (AEPutParamPtr event "Errs" typeBoolean one (length one))))
			(if enable-debugging
				(throw-err (AEPutParamPtr event "DeBg" typeBoolean one (length one))))

			(let ((sendErr (AESend event reply (+ kAEQueueReply kAENeverInteract)
									 kAENormalPriority kAEDefaultTimeout 0 0)))
			  (cond 
			   ((zerop sendErr)
				nil)
			   ((and (= sendErr connectionInvalid) (not cw:first-only))
				(throw-err (AEPutAttributePtr event keyAddressAttr typeApplSignature
											   second-target-choice (c:sizeof 'long)))
				(throw-err (AESend event reply (+ kAEQueueReply kAENeverInteract)
								   kAENormalPriority kAEDefaultTimeout 0 0)))
			   (t
				(throw-err sendErr))))

			(setq AE-history
				  (cons (cons transactionID
							  (list (cons 'description event-description)
									(cons 'handler 'cw:do-reply)))
						AE-history))
			noErr)))
    (if event (AEDisposeDesc event))
    result))

(defun cw:do-run (enable-debugging)
  (cw:send-event "RunP" "run" t enable-debugging nil))

(defun cw:do-bring-up-to-date ()
  (cw:send-event "UpdP" "bring up to date" t nil nil))

(defun cw:do-reset-file-paths ()
  (cw:send-event "ReFP" "reset file paths" nil nil nil))

(defun cw:do-compile (filename)
  (cw:send-event "Comp" (concat "compile " (file-name-nondirectory filename)) t nil filename))

(defun cw:do-check-syntax (filename)
  (cw:send-event "Chek" (concat "check syntax of " (file-name-nondirectory filename)) t nil filename))

(defun cw:do-bring-up-to-date ()
  (cw:send-event "UpdP" "bring up to date" t nil nil))

(defun cw:do-make ()
  (cw:send-event "Make" "make" t nil nil))

(defun cw:do-launch ()
  (let* ((first-target-choice (if cw:68K-first "MW 68K" "MW PPC"))
		 (second-target-choice (if cw:68K-first "MW PPC" "MW 68K"))
		 (err1 (launch-application first-target-choice)))
	(if (zerop err1)
		err1
	  (let ((err2 (if cw:first-only err1 (launch-application second-target-choice))))
		(if (= err2 fnfErr)
			(progn
			  (message "Put an alias or aliases to CodeWarrior named ÒMW 68KÓ and ÒMW PPCÓ in the etc folder of Emacs.")
			  noErr)
		  err2)))))
