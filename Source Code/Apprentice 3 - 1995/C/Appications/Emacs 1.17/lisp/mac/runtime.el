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
;;; This file is read after ~emacs/lisp/term/Macintosh-win.el is read at startup.
;;;

(require 'mac-dumptime "mac/dumptime")

(load "mac/AEPackObject.el" nil t)

(load "mac/apple-events" nil t)
(load "mac/core-suite" nil t)
(load "mac/menu-install" nil t)

(setq accept-high-level-events t)
(make-clipboard-current)
(modifiers:initialize)
(provide 'mac-runtime)
