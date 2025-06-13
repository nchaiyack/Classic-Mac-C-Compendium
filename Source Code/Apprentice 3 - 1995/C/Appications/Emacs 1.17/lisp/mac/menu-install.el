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
;;; Default menu initialization
;;;
;;; Create the menus.  Menus may already have been inserted by the
;;; .emacs file, so we have to insert these in front of those.	This
;;; is why we insert in reverse order, each one in front of all those
;;; inserted previously.
;;;

;;;
;;; This variable can be overridden in .emacs
;;;
(defvar fixed-width-fonts '("Courier" "Monaco"))

(defconst font-name-preference-index 130)
(defconst font-size-preference-index 131)

(defvar have-menus nil)
(if (not have-menus)
    (progn
      (setq buffers-menu (NewMenu (get-unique-menu-ID) "Buffers"))
      (InsertMenu buffers-menu t)
      
      (setq font-name-menu-id (get-unique-menu-ID))
      (setq font-name-menu (NewMenu font-name-menu-id ""))
      (mapcar (function (lambda (font-name)
			  (AppendMenu font-name-menu font-name 'do-font-name)))
	      fixed-width-fonts)
      (InsertMenu font-name-menu -1)

      ;;; Check the correct item in the font name menu
      (setq last-font-name-menu-check 1)
      (let ((n (CountMItems font-name-menu))
	    (current-font-name-code
	     (c:slotref 'GrafPort (console-WindowPtr) 'txFont))
	    (s (make-string 256 0))
	    (font-name-code (make-string 2 0)))
	(while (> n 0)
	  (GetItem font-name-menu n s)
	  (GetFNum s font-name-code)
	  (if (= (extract-internal font-name-code 0 'short)
		 current-font-name-code)
	      (progn
		(setq last-font-name-menu-check n)
		(setq n 0))
	    (setq n (1- n)))))
      (CheckItem font-name-menu last-font-name-menu-check 1)
      
      (setq font-size-menu-id (get-unique-menu-ID))
      (setq font-size-menu (NewMenu font-size-menu-id ""))
      (AppendMenu font-size-menu "9"  'do-font-size)
      (AppendMenu font-size-menu "10" 'do-font-size)
      (AppendMenu font-size-menu "12" 'do-font-size)
      (AppendMenu font-size-menu "14" 'do-font-size)
      (AppendMenu font-size-menu "18" 'do-font-size)
      (AppendMenu font-size-menu "24" 'do-font-size)
      (AppendMenu font-size-menu "(-" nil)
      (AppendMenu font-size-menu "Other..." 'do-font-size-other)
      (InsertMenu font-size-menu -1)

      ;;; Check the correct item in the font size menu
      (setq last-font-size-menu-check 8)
      (let ((n (CountMItems font-size-menu))
	    (current-font-size
	     (c:slotref 'GrafPort (console-WindowPtr) 'txSize))
	    (s (make-string 256 0)))
	(while (> n 0)
	  (GetItem font-size-menu n s)
	  (if (= (string-to-int (PtoCstr s)) current-font-size)
	      (progn
		(setq last-font-size-menu-check n)
		(setq n 0))
	    (setq n (1- n)))))
      (CheckItem font-size-menu last-font-size-menu-check 1)
      
      (setq special-menu (NewMenu (get-unique-menu-ID) "Special"))
      (AppendMenu special-menu "Show stdout-stderr" 'special-menu-show-stdout)
      (AppendMenu special-menu "Change Stack Size..." 'do-stacksize)
      (AppendMenu special-menu "Change Modifier Keys..." 'do-modifiers)
      (AppendMenu special-menu "(-" nil)
      (AppendMenu special-menu "Font/\033" nil)
      (AppendMenu special-menu "Font Size/\033" nil)
      (AppendMenu special-menu "Edit Colors..." 'do-edit-colors)
      (SetItemMark special-menu 5 font-name-menu-id)
      (SetItemMark special-menu 6 font-size-menu-id)
      (InsertMenu special-menu t)
      
      (setq edit-menu (NewMenu (get-unique-menu-ID) "Edit"))
      (AppendMenu edit-menu "Undo/Z" 'do-undo)
      (AppendMenu edit-menu "(-" nil)
      (AppendMenu edit-menu "Cut/X" 'do-cut)
      (AppendMenu edit-menu "Copy/C" 'do-copy)
      (AppendMenu edit-menu "Paste/V" 'do-paste)
      (AppendMenu edit-menu "Clear" 'do-clear)
      (InsertMenu edit-menu t)
      
      (setq format-menu-id (get-unique-menu-ID))
      (setq format-menu (NewMenu format-menu-id ""))
	  (AppendMenu format-menu "(Current buffer file format is"	nil)
      (AppendMenu format-menu "Macintosh"						'do-file-format-change)
      (AppendMenu format-menu "Unix"							'do-file-format-change)
      (AppendMenu format-menu "MS-DOS"							'do-file-format-change)
      (AppendMenu format-menu "Binary"							'do-file-format-change)
      (AppendMenu format-menu "(-"								nil)
	  (AppendMenu format-menu "(Default file format is"			nil)
      (AppendMenu format-menu "Macintosh"						'do-file-format-change)
      (AppendMenu format-menu "Unix"							'do-file-format-change)
      (AppendMenu format-menu "MS-DOS"							'do-file-format-change)
      (AppendMenu format-menu "Binary"							'do-file-format-change)
      (AppendMenu format-menu "(-"								nil)
      (AppendMenu format-menu "Inspect files when reading"      'do-deduce-change)
      (InsertMenu format-menu -1)

      (setq file-menu (NewMenu (get-unique-menu-ID) "File"))
      (AppendMenu file-menu "New Buffer/N" 'do-new)
      (AppendMenu file-menu "Open File Into Buffer.../O" 'do-open)
      (AppendMenu file-menu "Close Buffer/W" 'do-close)
      (AppendMenu file-menu "(-" nil)
      (AppendMenu file-menu "Save/S" 'do-save)
      (AppendMenu file-menu "Save As..." 'do-save-as)
      (AppendMenu file-menu "Revert Buffer" 'do-revert)
      (AppendMenu file-menu "File formats/\033" nil)
      (AppendMenu file-menu "(-" nil)
      (AppendMenu file-menu "Page Setup..." 'do-page-setup)
      (AppendMenu file-menu "Print Buffer/P" 'do-print-buffer)
      (AppendMenu file-menu "Print File From Disk..." 'do-print-file)
      (AppendMenu file-menu "(-" nil)
      (AppendMenu file-menu "Quit Emacs/Q" 'do-quit)
      (SetItemMark file-menu 8 format-menu-id)
      (InsertMenu file-menu t)
      
      (setq apple-menu (NewMenu (get-unique-menu-ID) "\024"))
      (AppendMenu apple-menu "About Emacs..." 'do-about)
      (AddResMenu apple-menu "DRVR")
      (InsertMenu apple-menu t)

      (DrawMenuBar)
      (setq have-menus t)))

(defun has-binary-suffix (fn suffixes)
  (cond
   ((null suffixes)
	nil)
   ((equal (substring fn (- (length (car suffixes)))) (car suffixes))
	t)
   (t
	(has-binary-suffix fn (cdr suffixes)))))

(defun fixup-format-menu ()
  (mapcar
   (function (lambda (q)
			   (let ((pivot (car q))
					 (base (cdr q)))
				 (mapcar
				  (function (lambda (p)
							  (CheckItem format-menu (+ base (car p)) (if (eq pivot (cdr p)) 1 0))))
				  '((0 . macintosh) (1 . unix) (2 . msdos) (3 . binary))))))
   (list (cons text-file-format 2) (cons default-text-file-format 8)))
  
  (CheckItem format-menu 13 (if deduce-text-file-format 1 0)))

;;; Functions to be called in response to the selection of menu items

(defun do-new (menu item)
  (let ((buffer (generate-new-buffer "untitled")))
    (switch-to-buffer buffer)))

(defun do-open (menu item)
  (let ((file-name (GetFile)))
    (if file-name
		(find-file file-name))))

(defun do-close (menu item)
  (if (and (buffer-file-name)
		   (buffer-modified-p))
	  (progn
		(ParamText (CtoPstr (buffer-name)) (CtoPstr "closing") 0 0)
		(let ((choice (Alert 138 (function alert-filter))))
		  (cond
		   ((= choice 1)
			(save-buffer)
			(kill-buffer (current-buffer)))
		   ((= choice 2)
			nil)
		   ((= choice 3)
			(set-buffer-modified-p nil)
			(kill-buffer (current-buffer))))))
	(kill-buffer (current-buffer))))

(defun do-save (menu item)
  (if (buffer-file-name)
      (save-buffer)
    (do-save-as 0 0)))

(defun do-save-as (menu item)
  (let ((file-name (PutFile "Save file as:" (buffer-name))))
    (if file-name
		(write-file file-name))))

(defun do-revert (menu item)
  (revert-buffer))

(defun do-deduce-change (menu item)
  (setq deduce-text-file-format (not deduce-text-file-format)))

(defun do-file-format-change (menu item)
  (let* ((formats '(macintosh unix msdos binary))
		 (default-group (>= item 8))
		 (format-index (- item (if default-group 8 2)))
		 (pivot (if default-group 'default-text-file-format 'text-file-format)))
	(set pivot (nth format-index formats))))

(defun do-page-setup (menu item)
  (call-process "lpr" nil nil nil "-P"))

(defun do-print-buffer (menu item)
  (print-buffer))

(defun do-print-file (menu item)
  (let ((file-name (GetFile)))
    (if file-name
		(apply (function call-process)
			   (append '("lpr" nil 0 nil "-p") lpr-switches (list file-name))))))

(defun friendly-quit-queries ()
  (catch 'cancel
	(mapcar
	 (function (lambda (x)
				 (if (and (buffer-file-name x)
						  (buffer-modified-p x))
					 (progn
					   (ParamText (CtoPstr (buffer-name x)) (CtoPstr "quitting") 0 0)
					   (let ((choice (Alert 138 (function alert-filter))))
						 (cond
						  ((= choice 1)
						   (save-buffer x))
						  ((= choice 2)
						   (throw 'cancel nil))
						  ((= choice 3)
						   nil)))))))
	 (buffer-list))
	t))

(defun do-quit (menu item)
  (if (friendly-quit-queries)
	  (kill-emacs t)))

(defun apple-undo ()
  (interactive)
  (undo)
  (setq last-command 'undo))
  
(defun apple-cut ()
  (interactive)
  (save-excursion (copy-region-to-clipboard))
  (if (mark) (delete-region (point) (mark))))

(defun apple-copy ()
  (interactive)
  (save-excursion (copy-region-to-clipboard)))

(defun apple-paste ()
  (interactive)
  (insert-buffer-substring (save-excursion (make-clipboard-current))))

(defun apple-clear ()
  (interactive)
  (if (mark) (delete-region (point) (mark))))

(defun do-undo (menu item) (apple-undo))

(defun do-cut (menu item) (apple-cut))

(defun do-copy (menu item) (apple-copy))

(defun do-paste (menu item) (apple-paste))

(defun do-clear (menu item) (apple-clear))

(defun do-font-size-internal (item size)
  (if (or (>= size 128) (<= size 0))
	  (message "You can't be serious!")
	(let ((h (NewHandle 2)))
	  (if (zerop (MemError))
		  (progn
			(unwind-protect
				(progn
				  (HLock h)
				  (encode-internal (deref h) 0 'short font-size))
			  (HUnlock h))
			(set-preference "DATA" font-size-preference-index h))))
	(special-menu-font-change -1 font-size)
	(CheckItem font-size-menu last-font-size-menu-check 0)
	(CheckItem font-size-menu item 1)
	(setq last-font-size-menu-check item)))

(defun do-font-size-other (menu item)
  (let ((font-size (call-interactively (function (lambda (font-size)
					      (interactive "nPoint size: ")
					      font-size)))))
    (do-font-size-internal item font-size)))

(defun do-font-size (menu item)
  (let ((s (make-string 256 0)))
    (GetItem font-size-menu item s)
    (let ((font-size (string-to-int (PtoCstr s))))
      (do-font-size-internal item font-size))))

(defun do-font-name (menu item)
  (let ((s (make-string 256 0)))
    (GetItem font-name-menu item s)
    (let ((font-number-string (make-string 2 0)))
      (GetFNum s font-number-string)
      (let ((font-number (extract-internal font-number-string 0 'short)))
		(let ((h (NewHandle 2)))
		  (if (zerop (MemError))
			  (progn
				(HLock h)
				(encode-internal (deref h) 0 'short font-number)
				(HUnlock h)
				(set-preference "DATA" font-name-preference-index h))))
		(special-menu-font-change font-number -1))))
  (CheckItem font-name-menu last-font-name-menu-check 0)
  (CheckItem font-name-menu item 1)
  (setq last-font-name-menu-check item))

(defun do-menu (menu item)
  (let* ((menu-handle (GetMHandle menu))
		 (callback (assoc (cons menu-handle item) mac-menu-callback-list)))
    (cond
     (callback
	  (let ((callback-func (cdr callback)))
		(if callback-func
			(funcall (cdr callback) menu-handle item)
		  nil)))
     ((= menu-handle apple-menu)
      (let ((s (make-string 256 0)))
		(GetItem apple-menu item s)
		(OpenDeskAcc s)))
     (t
      nil))))

(defun buffer-list-for-buffers-menu ()
  (sort (mapcar (function (lambda (x) (cons x (buffer-name x)))) (buffer-list))
	(function (lambda (x y) (string< (upcase (cdr x)) (upcase (cdr y)))))))

(defun classify-buffer-list (buffer-list)
  (if (null buffer-list)
      (list nil nil nil)
    (let ((cdr-result (classify-buffer-list (cdr buffer-list)))
		  (first-char (substring (cdr (car buffer-list)) 0 1)))
      (set-buffer (car (car buffer-list)))
      (cond
       ((equal " " first-char)
		cdr-result)
       ((equal major-mode 'dired-mode)
		(list (nth 0 cdr-result)
			  (cons (car buffer-list) (nth 1 cdr-result))
			  (nth 2 cdr-result)))
       ((and (equal "*" first-char) (not (buffer-file-name (car (car buffer-list)))))
		(list (cons (car buffer-list) (nth 0 cdr-result))
			  (nth 1 cdr-result)
			  (nth 2 cdr-result)))
       (t
		(list (nth 0 cdr-result)
			  (nth 1 cdr-result)
			  (cons (car buffer-list) (nth 2 cdr-result))))))))

(defun append-buffer-menu (buffer-list i check-modified)
  (if (null buffer-list)
      i
    (let ((buffer (car (car buffer-list)))
	  (name (cdr (car buffer-list))))
      (AppendMenu buffers-menu " " 'do-buffers-menu-item)
      (SetItem buffers-menu i name)
      (if (and check-modified (buffer-modified-p buffer))
	  (SetItemMark buffers-menu i 215)))
    (append-buffer-menu (cdr buffer-list) (1+ i) check-modified)))

(defun fixup-buffers-menu ()
  ;;; Remove the old buffer list.
  (let ((n (CountMItems buffers-menu)))
    (while (> n 0)
      (DelMenuItem buffers-menu 1)
      (setq n (1- n))))

  ;;; Remove the old callback functions from the callback list.
  (let ((x mac-menu-callback-list))
    (setq mac-menu-callback-list nil)
    (while x
      (if (not (eq (car (car (car x))) buffers-menu))
	  (setq mac-menu-callback-list (cons (car x) mac-menu-callback-list)))
      (setq x (cdr x))))

  ;;; Create the new buffer list
  (let* ((old-buffer (current-buffer))
	 next-item
	 (buffer-list (buffer-list-for-buffers-menu))
	 (classified-buffers (classify-buffer-list buffer-list))
	 (temp-buffers (nth 0 classified-buffers))
	 (dired-buffers (nth 1 classified-buffers))
	 (other-buffers (nth 2 classified-buffers)))
    (setq next-item (append-buffer-menu temp-buffers 1 nil))
    (if (and temp-buffers dired-buffers)
	(progn (AppendMenu buffers-menu "(-" nil) (setq next-item (1+ next-item))))
    (setq next-item (append-buffer-menu dired-buffers next-item nil))
    (if (and (or temp-buffers dired-buffers) other-buffers)
	(progn (AppendMenu buffers-menu "(-" nil) (setq next-item (1+ next-item))))
    (append-buffer-menu other-buffers next-item t)
    (set-buffer old-buffer)))

(defun do-buffers-menu-item (menu item)
  (let ((s (make-string 256 0)))
    (GetItem buffers-menu item s)
    (switch-to-buffer (PtoCstr s))))

(defvar MenuSelect-before-hooks (list (function fixup-buffers-menu)
									  (function fixup-format-menu)))

(defun do-MenuSelect-before-hooks ()
  (mapcar (function funcall) MenuSelect-before-hooks))

(defun alert-filter (d e i)
  (let ((what (c:slotref 'EventRecord e 'what)))
    (cond
     ((= what keyDown)
      (let ((c (logand (c:slotref 'EventRecord e 'message) charCodeMask))
			(modifiers (c:slotref 'EventRecord e 'modifiers)))
		(cond
		 ((or (= c (string-to-char "\r")) (= c 3))
		  (encode-internal i 0 'short 1)
		  (blink d 1)
		  1)
		 ((and (= c (string-to-char ".")) (not (zerop (logand modifiers cmdKey))))
		  (encode-internal i 0 'short 2)
		  (blink d 2)
		  1)
		 ((and (= c (string-to-char "d")) (not (zerop (logand modifiers cmdKey))))
		  (encode-internal i 0 'short 3)
		  (blink d 3)
		  1)
		 (t
		  0))))
     (t
      0))))

(defvar menu-install-hooks nil)
(mapcar 'eval menu-install-hooks)
