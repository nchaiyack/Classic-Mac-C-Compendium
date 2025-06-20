;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1995 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

(defvar cw:cw-mode-map nil "Local keymap for CodeWarrior error buffer.")

(if cw:cw-mode-map
    nil
  (setq cw:cw-mode-map (make-keymap))
  (suppress-keymap cw:cw-mode-map)
  (define-key cw:cw-mode-map "c" 'cw:clear-previous-messages)
  (define-key cw:cw-mode-map "\C-n" 'cw:goto-next-error-message)
  (define-key cw:cw-mode-map "\C-p" 'cw:goto-prev-error-message)
  (define-key cw:cw-mode-map "\C-k" 'cw:delete-current-message)
  (define-key cw:cw-mode-map "\C-m" 'cw:goto-file-and-line) ; return
  (define-key cw:cw-mode-map "\C-x*\014" 'cw:goto-file-and-line) ; enter
  (define-key cw:cw-mode-map "\C-x*\007" 'cw:clear-all-messages) ; clear
  (define-key cw:cw-mode-map "\C-x*\075" 'cw:goto-next-error-message) ; down arrow
  (define-key cw:cw-mode-map "\C-x*\076" 'cw:goto-prev-error-message) ; up arrow
  )

(defun cw:cw-errors-mode ()
  (kill-all-local-variables)
  (use-local-map cw:cw-mode-map)
  (setq major-mode 'cw:cw-errors-mode
	mode-name "CodeWarrior Errors"
	buffer-read-only t
	selective-display t))

;; cw-errors-mode is suitable only for specially formatted data.
(put 'cw:cw-errors-mode 'mode-class 'special)

(defun cw:errors-buffer ()
  (let* ((buffer-name "*codewarrior errors*")
		 (buffer (get-buffer buffer-name))
		 (must-create (not buffer)))
	(if must-create
		  (let ((buffer (get-buffer-create buffer-name)))
			(set-buffer buffer)
			(cw:cw-errors-mode)
			buffer)
	  buffer)))

(defvar cw:error-serial-number 0)
(defvar cw:error-list nil)

(defun cw:display-simple-error (code)
  (let* (buffer-read-only
		 (spaces (make-string 19 (string-to-char " ")))
		 (kind-string "CodeWarrior error")
		 (raw-message (cw:error-code-string code))
		 (message (if raw-message raw-message (concat "code " (int-to-string code)))))
	(insert kind-string (substring spaces 1 (- (length spaces) (length kind-string))) ": "
			message (char-to-string 13) "-1" "\n\n")))

(defconst cw:ErrT
  ;; triples of (error-type-code english-explanatory-phrase has-file-and-line-information)
  '(("ErIn" "Information" nil)
	("ErCE" "Compiler error" t)
	("ErCW" "Compiler warning" t)
	("ErDf" "Definition" nil)
	("ErLE" "Linker error" nil)
	("ErLW" "Linker warning" nil)))

(defun cw:display-error (kind filespec line message)
  (let* ((spaces (make-string 19 (string-to-char " ")))
		 (kind-info (assoc kind cw:ErrT))
		 (kind-string (if kind-info (nth 1 kind-info) ""))
		 (have-name-and-line (if kind-info (nth 2 kind-info) nil))
		 (displayed-info
		  (if have-name-and-line
			  (let* ((filename (FSSpec-to-unix-filename filespec))
					 (file-buffer (if (stringp filename) (get-file-buffer filename) nil))
					 (line-of-code (if file-buffer
									   (save-excursion
										 (set-buffer file-buffer)
										 (goto-line line)
										 (let* ((found-line-start (re-search-forward "[ \t]*" nil t))
												(line-start (point))
												(found-line-end (re-search-forward "\n" nil t)))
										   (if (and found-line-start found-line-end)
											   (buffer-substring line-start (1- (point)))
											 "")))
									 "")))
				(list kind-string (substring spaces 1 (- (length spaces) (length kind-string))) ": "
					  message (char-to-string 13) (int-to-string cw:error-serial-number) "\n"
					  (file-name-nondirectory filename) " line " (int-to-string line)
					  "   " line-of-code "\n\n"))
			(list kind-string (substring spaces 1 (- (length spaces) (length kind-string))) ": "
				  message (char-to-string 13) (int-to-string cw:error-serial-number) "\n\n"))))
	
	(let (buffer-read-only)
	  (apply (function insert) displayed-info))
	(if have-name-and-line
		(progn
		  (setq cw:error-list (cons (list cw:error-serial-number filespec line) cw:error-list))
		  (setq cw:error-serial-number (1+ cw:error-serial-number))))))

(defun cw:goto-next-error-message ()
  (interactive)
  (let ((start (point))
		(found-end-of-message (re-search-forward "\n\n" nil t)))
	(if (and found-end-of-message (not (eobp)))
		t
	  (goto-char start)
	  nil)))

(defun cw:goto-prev-error-message ()
  (interactive)
  (let ((start (point))
		(found-begin-of-current-message (re-search-backward "\n\n" nil t)))
	(if found-begin-of-current-message
		(let ((found-begin-of-prev-message (re-search-backward "\n\n" nil t)))
		  (if found-begin-of-prev-message
			  (progn
				(forward-char 2)
				t)
			(goto-char 1)
			t))
	  (goto-char start)
	  nil)))

(defun cw:verify-message-start ()
  (let ((found-end-of-prev-message (re-search-backward "\n\n" nil t)))
	(if found-end-of-prev-message
		(forward-char 2)
	  (goto-char 1)))
  t)

(defun cw:delete-current-message ()
  (interactive)
  (if (cw:verify-message-start)
	  (let ((message-start (point))
			(have-next-message-position (cw:goto-next-error-message)))
		(let (buffer-read-only)
		  (delete-region message-start (if have-next-message-position (point) (point-max)))
		  (if (eobp) (cw:goto-prev-error-message))))))

(defun cw:clear-all-messages ()
  (interactive)
  (let (buffer-read-only)
	(delete-region (point-min) (point-max))))

(defun cw:clear-previous-messages ()
  (interactive)
  (if (cw:verify-message-start)
	  (let (buffer-read-only)
		(delete-region (point-min) (point)))))

(defun cw:goto-file-and-line ()
  (interactive)
  (if (cw:verify-message-start)
	  (let ((filename-and-line
			 (save-excursion
			   (let ((found-error-info (re-search-forward (char-to-string 13) nil t))
					 (error-info (point))
					 (found-end-error-info (re-search-forward "-*[0-9]+" nil t))
					 (end-error-info (point)))
				 (if (and found-error-info found-end-error-info)
					 (let* ((serial-number (string-to-int (buffer-substring error-info end-error-info)))
							(error-info (assoc serial-number cw:error-list)))
					   (if error-info
						   (let* ((spec (nth 1 error-info))
								  (line (nth 2 error-info))
								  (filename (FSSpec-to-unix-filename spec)))
							 (cons filename line))
						 nil))
				   nil)))))
		(if filename-and-line
			(progn
			  (find-file-other-window (car filename-and-line))
			  (switch-to-buffer (get-buffer (file-name-nondirectory (car filename-and-line))))
			  (goto-line (cdr filename-and-line)))
		  nil))
	nil))

(defun cw:error-code-string (code)
  (if (< code 0)
	  (lookup-error-string code)
	(nth code '("no error" "action failed" "file not in project" "duplicate file"
						 "compile error" "make failed" "no open project"))))

(defun cw:parse-error-obj (error-obj)
  ;; This is just hideous -- there must be a better way.
  (let ((type (c:slotref 'AEDesc error-obj 'descriptorType)))
	(if (equal type "ErrM")
		(let* ((h (c:slotref 'AEDesc error-obj 'dataHandle))
			   (error-kind-offset 20)
			   (error-string-length-offset 32)
			   (error-string-offset 36)
			   (t1 (HLock h))
			   (p (extract-internal h 0 'long))
			   (error-kind (extract-internal p error-kind-offset 'string 4))
			   (error-string-length (extract-internal p error-string-length-offset 'unsigned-long))
			   (rounded-error-string-length (logand (1+ error-string-length) (lognot 1)))
			   (error-string (extract-internal p error-string-offset 'string error-string-length))
			   (error-file-offset (+ rounded-error-string-length 48))
			   (error-line-offset (+ rounded-error-string-length 130))
			   (error-file (extract-internal p error-file-offset 'string (c:sizeof 'FSSpec)))
			   (error-line (extract-internal p error-line-offset 'long))
			   (t2 (HUnlock h)))
		  (cw:display-error error-kind error-file error-line error-string)))))

(defun cw:parse-error-obj-list (error-obj-list)
  (let ((error-obj (make-string (c:sizeof 'AEDesc) 0))
		(i 1))
	(while (<= i count-int)
	  (throw-err (AEGetNthDesc error-obj-list i "ErrM" returned-keyword error-obj))
	  (cw:parse-error-obj error-obj)
	  (AEDisposeDesc error-obj)
	  (setq i (1+ i)))))

(defun cw:do-reply (event history)
  (let* ((error-obj-list (make-string (c:sizeof 'AEDescList) 0))
		 have-error-obj-list
		 (count-string (make-string (c:sizeof 'long) 0))
		 (result-code (make-string (c:sizeof 'short) 0))
		 (returned-type (make-string (c:sizeof 'long) 0))
		 (returned-keyword (make-string (c:sizeof 'long) 0))
		 (actual-size (make-string (c:sizeof 'long) 0))
		 (errorNumber-string (make-string (c:sizeof 'long) 0))
		 (getKeyErrNumErr (AEGetParamPtr event keyErrorNumber typeLongInteger returned-type
										 errorNumber-string (c:sizeof 'long) actual-size))
		 (errors-buffer (cw:errors-buffer))
		 (t1 (set-buffer errors-buffer))
		 (start-point (point-max)))
	
	(goto-char start-point)
	
	(if (cond
		 ((= getKeyErrNumErr noErr)
		  (let ((errorNumber-int (extract-internal errorNumber-string 0 'long)))
			(cw:display-simple-error errorNumber-int)
			t))
		 ((= getKeyErrNumErr errAEDescNotFound)
		  (catch 'panic
			(throw-err (AEGetParamDesc event keyAEResult typeAEList error-obj-list))
			(setq have-error-obj-list t)
			(throw-err (AECountItems error-obj-list count-string))
			(let ((count-int (extract-internal count-string 0 'long)))
			  (if (zerop count-int)
				  nil
				(cw:parse-error-obj-list error-obj-list)
				t))))
		 (t
		  (cw:display-simple-error getKeyErrNumErr)
		  t))
		(select-window (display-buffer errors-buffer))
	  (message "No errors from CodeWarrior"))
	
	(goto-char start-point)
	
	(if have-error-obj-list (AEDisposeDesc error-obj-list))))
