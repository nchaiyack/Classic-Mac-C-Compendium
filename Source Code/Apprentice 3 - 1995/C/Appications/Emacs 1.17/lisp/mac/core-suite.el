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
;;; This is the implementation of the Apple event required suite.
;;;
;;; This file cannot be included in a dump.  The AEInstallEventHandler calls
;;; must be executed on each invokation.
;;;

(AEInstallEventHandler kCoreEventClass kAEOpenApplication 'do-AE-open-appl 0 0)

(defun do-AE-open-appl (event reply refCon)
  noErr)

(AEInstallEventHandler kCoreEventClass kAEOpenDocuments 'do-AE-open-docs 0 0)

(defun do-AE-open-docs (event reply refCon)
  (let* ((docList (make-string (c:sizeof 'AEDesc) 0))
		 have-docList
		 (itemsInList (make-string 4 0))
		 (result
		  (catch 'panic
			(throw-err (AEGetParamDesc event keyDirectObject typeAEList docList))
			(setq have-docList t)
		    ;;; We get an error from this next line if Think C sends a position record.
			;(throw-err (AE-have-required-parameters event))
			(throw-err (AECountItems docList itemsInList))
			(setq itemsInList (extract-internal itemsInList 0 'long))
			(throw-err (AE-open-list-items docList 1 itemsInList event))
			noErr)))
    (if have-docList (AEDisposeDesc docList))
	(if (not (zerop result))
		(let ((err-string (make-string (c:sizeof 'short) 0)))
		  (encode-internal err-string 0 'short err)
		  (AEPutParamPtr reply keyErrorNumber typeShortInteger err-string (c:sizeof 'short))))
    result))

(defconst sizeof-FInfo 16)

(defun AE-is-stationery (spec)
  (let* ((info (make-string sizeof-FInfo 0))
		 (err (FSpGetFInfo spec info)))
    (if (zerop err)
		(let ((flags (extract-internal info 8 'short)))
		  (not (zerop (logand 2048 flags))))
      nil)))

(defun AE-open-list-items (docList i itemsInList event)
  (if (> i itemsInList)
      noErr
    (let* ((keywd (make-string 4 0))
		   (returnedType (make-string 4 0))
		   (spec (make-string (c:sizeof 'FSSpec) 0))
		   (actualSize (make-string 4 0))
		   (err (AEGetNthPtr docList i typeFSS keywd returnedType spec
							 (c:sizeof 'FSSpec) actualSize)))
      (if (not (zerop err))
		  err
		(let ((filename (FSSpec-to-unix-filename spec)))
		  (if (AE-is-stationery spec)
			  (let ((old-buffer (current-buffer))
					(new-buffer (generate-new-buffer "untitled")))
				(set-buffer new-buffer)
				(insert-file-contents filename)
				(if (eq (selected-window) (minibuffer-window))
					(set-buffer old-buffer)
				  (switch-to-buffer new-buffer)))
			(if (eq (selected-window) (minibuffer-window))
				(progn
				  (find-file-other-window filename)
				  ;;; find-file-other-window will select the new window, probably
				  ;;; not a good idea.	If we use save-excursion, the
				  ;;; newly created buffer will not be displayed immediately, also
				  ;;; not a good idea.	So we do this.
				  (select-window (minibuffer-window)))
			  (find-file filename)))
		  (if (fboundp 'tc:parse-position-record)
			  (tc:parse-position-record event)))))
    (AE-open-list-items docList (1+ i) itemsInList event)))

(AEInstallEventHandler kCoreEventClass kAEPrintDocuments 'do-AE-print-docs 0 0)

(defun do-AE-print-docs (event reply refCon)
  (let* ((docList (make-string (c:sizeof 'AEDesc) 0))
		 have-docList
		 (itemsInList (make-string 4 0))
		 err
		 (result
		  (catch 'panic
			(throw-err (AEGetParamDesc event keyDirectObject typeAEList docList))
			(setq have-docList t)
			(throw-err (AE-have-required-parameters event))
			(throw-err (AECountItems docList itemsInList))
			(setq itemsInList (extract-internal itemsInList 0 'long))
			(throw-err (AE-print-list-items docList itemsInList))
			noErr)))
    (if have-docList (AEDisposeDesc docList))
	(if (not (zerop result))
		(let ((err-string (make-string (c:sizeof 'short) 0)))
		  (encode-internal err-string 0 'short err)
		  (AEPutParamPtr reply keyErrorNumber typeShortInteger err-string (c:sizeof 'short))))
    result))

(defun AE-get-items-to-print (docList i itemsInList)
  (if (> i itemsInList)
	  nil
    (let* ((keywd (make-string 4 0))
		   (returnedType (make-string 4 0))
		   (spec (make-string (c:sizeof 'FSSpec) 0))
		   (actualSize (make-string 4 0))
		   (err (AEGetNthPtr docList i typeFSS keywd returnedType spec
							 (c:sizeof 'FSSpec) actualSize))
		   (tail (AE-get-items-to-print docList (1+ i) itemsInList)))
      (if (not (zerop err))
		  tail
		(let ((filename (FSSpec-to-unix-filename spec)))
		  (cons filename tail))))))

(defun AE-print-list-items (docList itemsInList)
  (let* ((files-to-print (AE-get-items-to-print docList 1 itemsInList))
		 (err (apply (function call-process) "lpr" nil nil nil files-to-print)))
	err))

(AEInstallEventHandler kCoreEventClass kAEQuitApplication 'do-AE-quit-appl 0 0)

(defun do-AE-quit-appl (event reply refCon)
  (let ((err (AE-have-required-parameters event)))
    (if (not (zerop err))
		err
	  (friendly-quit-queries)
	  (setq quit-in-main-event-loop t)
      noErr)))
