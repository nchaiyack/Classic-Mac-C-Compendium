;;;; Emacs 19 compatibility functions for use in Emacs 18.
;;;; $Id: emacs-19.el,v 1.26 1992/04/22 17:02:50 sk RelBeta $

;; These functions are used in dired.el, but are also of general
;; interest, so you may want to add this to your .emcas:
;; 
;; (autoload 'make-directory "emacs-19" "Make a directory." t)
;; (autoload 'remove-directory "emacs-19" "Remove a directory." t)
;; (autoload 'diff "emacs-19" "Diff two files." t)
;; (autoload 'member "emacs-19" "Like memq, but uses `equal' instead of `eq'.")
;; (autoload 'compiled-function-p "emacs-19" "Emacs 18 doesn't have these.")
;; (autoload 'comint::background "emacs-19" "Run a COMMAND in the background, like csh." t)
;; We need these functions in function diff:
(autoload 'shell-quote "dired")		; actually belongs into simple.el
(autoload 'latest-backup-file "dired")	; actually belongs into files.el

;; call-process below may lose if filename starts with a `-', but I
;; fear not all mkdir or rmdir implementations understand `--'.
(defun remove-directory (fn)
  "Remove a directory.
This is a subr in Emacs 19."
  (interactive 
   (list (read-file-name "Remove directory: " nil nil 'confirm)))
  (setq fn (expand-file-name fn))
  (if (file-directory-p fn)
      (call-process "rmdir" nil nil nil fn)
    (error "Not a directory: %s" fn))
  (if (file-exists-p fn)
      (error "Could not remove directory %s" fn)))

(defun make-directory (fn)
  "Make a directory.
This is a subr in Emacs 19."
  (interactive (list (read-file-name "Make directory: ")))
  (setq fn (directory-file-name (expand-file-name fn)))
  (if (file-exists-p fn)
      (error "Cannot make directory %s: file already exists" fn)
    (call-process "mkdir" nil nil nil fn))
  (or (file-directory-p fn)
      (error "Could not make directory %s" fn)))

;; variable so that rcs-diff or whatever can let-bind it.
;; diff would then have to provide a way to insert into a given buffer (?)
;; and run synchronously (?)
(defvar diff-program "diff"
  "*Program used for function diff (which see).")

(defvar diff-switches nil
  "*If non-nil, a string specifying switches to be be passed to diff.
A list of strings (or nil) means that the commandline can be edited
after inserting the strings in the list.") 

(defun diff (fn1 fn2 &optional switches)
  "Diff two files FN1 and FN2.
With a prefix arg, you are prompted for the optional third arg
  SWITCHES, the diff switches used.
See also variable diff-switches."
  (interactive (diff-read-args "Diff: " "Diff %s with: "
			       "Diff with switches: "))
  (or (and (stringp fn1)
	   (stringp fn2))
      (error  "diff: arguments must be strings: %s %s" fn1 fn2))
  (or switches
      (setq switches (if (stringp diff-switches)
			       diff-switches
			    (if (listp diff-switches)
				(mapconcat 'identity diff-switches " ")
			      ""))))
  (let ((command (concat diff-program " " switches
			 " "
			 (shell-quote (expand-file-name fn1))
			 " "
			 (shell-quote (expand-file-name fn2)))))
    (require 'compile)
    (compile1 command "No more diff's" "diff")))

(defun diff-backup (file &optional switches)
  "Diff FILE with its backup file.
Uses the latest backup, if there are several numerical backups.
If FILE is a backup, diff it with its original.
With a prefix arg, you are prompted for the optional third arg
  SWITCHES, the diff switches used.
The backup file is the first file given to `diff'.
See the command `diff'."
  (interactive
   (diff-read-args "Backup Diff: " nil "Switches for Backup Diff: "))
  (let (bak ori)
    (if (backup-file-name-p file)
	(setq bak file
	      ori (file-name-sans-versions file))
      (setq bak (or (latest-backup-file file)
		    (error "File has no backup: %s" file))
	    ori file))
    (diff bak ori switches)))

(defun diff-read-args (msg1 msg2 msg3)
  (let* ((fn1 (read-file-name msg1))
	 (fn2 (and msg2 (read-file-name (format msg2 fn1))))
	 (switches (diff-read-switches msg3)))
    (if msg2
	(list fn1 fn2 switches)
      (list fn1 switches))))

(defun diff-read-switches (msg3)
  (if (or current-prefix-arg
	  (listp diff-switches))
      (read-string msg3
		   (if (stringp diff-switches)
		       diff-switches
		     (if (listp diff-switches)
			 (mapconcat 'identity
				    diff-switches " ")
		       "")))))

;; not needed any longer, sk@sun4 21-May-1991 16:37
;; (defun vms-read-directory (dirname switches buffer)
;;   ;; Dired calls this function only in the current buffer
;;   ;; dired-ls gets redefined in dired-vms.el to work under VMS.
;;   (and buffer
;;        (not (eq buffer (current-buffer)))
;;        (error "Must be called in current buffer"))
;;   (dired-ls dirname switches nil t))

(defun member (x y)
  "Like memq, but uses `equal' for comparison.
This is a subr in Emacs 19."
  (while (and y (not (equal x (car y))))
    (setq y (cdr y)))
  y)

(defun compiled-function-p (x)
  "Emacs 18 doesn't have these."
  nil)

;; The 18.57 version has a bug that causes C-x C-v RET (which usually
;; re-visits the current buffer) to fail on dired buffers.
;; Only the last statement was changed to avoid killing the current
;; buffer.
(defun find-alternate-file (filename)
  "Find file FILENAME, select its buffer, kill previous buffer.
If the current buffer now contains an empty file that you just visited
\(presumably by mistake), use this command to visit the file you really want."
  (interactive "FFind alternate file: ")
  (and (buffer-modified-p)
       (not buffer-read-only)
       (not (yes-or-no-p (format "Buffer %s is modified; kill anyway? "
				 (buffer-name))))
       (error "Aborted"))
  (let ((obuf (current-buffer))
	(ofile buffer-file-name)
	(oname (buffer-name)))
    (rename-buffer " **lose**")
    (setq buffer-file-name nil)
    (unwind-protect
	(progn
	  (unlock-buffer)
	  (find-file filename))
      (cond ((eq obuf (current-buffer))
	     (setq buffer-file-name ofile)
	     (lock-buffer)
	     (rename-buffer oname))))
    (or (eq (current-buffer) obuf)
	(kill-buffer obuf))))

;; At least in Emacs 18.55 this defvar has been forgotten to be copied
;; from lpr.el into loaddefs.el

(defvar lpr-command (if (eq system-type 'usg-unix-v)
			"lp" "lpr")
  "Shell command for printing a file")

;; We need background.el (from Olin Shiver's comint release)
;; in Emacs 18 - only Emacs 19's shell command understands `&'.

;; These contortions are necessary because Epoch also defines a
;; functions named `background', to set the background color of a
;; screen.  Why o why didn't they call it epoch::background?

(defun comint::background (command)
  "Run COMMAND in the background like csh.  
A message is displayed when the job starts and finishes.  The buffer is in
comint mode, so you can send input and signals to the job.  The process object
is returned if anyone cares.  See also comint-mode and the variables
background-show and background-select."
  (require 'background)
  (background command))

(defun background-comint-or-epoch (&rest args)
  "Dispatch functions that calls `comint::background' if called with
one argument, `epoch::background' else.
If called interactively, does `comint::background'."
  (interactive "s%% ")
  (apply (if (= (length args) 1)
	     (function comint::background)
	   (function epoch::background))
	 args))

(if (not (boundp 'epoch::version))
    ;; Without Epoch there are no problems at all:  `comint::background'
    ;; will simply use CMU background.el's version.
    nil
  ;; Else we're running Epoch.  Save its version of background before
  ;; we try loading CMU background.el, maybe clobbering Epoch's version.
  (or (fboundp 'epoch::background)	; if this file is loaded twice
      (fset 'epoch::background (symbol-function 'background)))
  ;; Now both comint::background and epoch::background are suitably defined.
  (if (not (or (featurep 'background)
	       (load "background" t 'silent)))
      ;; If CMU's background.el is not avaiable, leave it at
      ;; comint::background's default definition: the require will
      ;; signal an error if actually called.  No need to abort now.
      nil
    ;; background.el is loaded by now: save CMU background for dired
    (fset 'comint::background (symbol-function 'background))
    ;; This used to
    ;; restore Epoch's original definition, making background a subr again:
    ;;- (fset 'background (symbol-function 'epoch::background))
    ;; This is better, it even makes all those packages work that
    ;; never new about the background confusion in Epoch (like
    ;; cmutex's C-c P tex preview command, or dired-x11.el):
    (fset 'background 'background-comint-or-epoch)))

(provide 'emacs-19)
