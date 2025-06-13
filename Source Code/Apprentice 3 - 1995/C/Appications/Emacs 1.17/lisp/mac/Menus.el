;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

; pascal void CheckItem(MenuHandle theMenu,short item,Boolean checked)
;    = 0xA945; 
(deftrap CheckItem ("a945")
  ((theMenu long)
   (item short)
   (check char))
  nil)

; pascal MenuHandle NewMenu(short menuID,const Str255 menuTitle)
;    = 0xA931; 
(deftrap NewMenu-internal ("a931")
  ((menuID short)
   (menuTitle address))
  long)

(defun NewMenu (menuID menuTitle)
  (NewMenu-internal menuID (CtoPstr menuTitle)))

(defun MenuList ()
  (extract-internal 2588 0 'unsigned-long))

; pascal void InsertMenu(MenuHandle theMenu,short beforeID)
;    = 0xA935; 
(deftrap InsertMenu-internal ("a935")
  ((theMenu long)
   (beforeID short))
  nil)

(defun InsertMenu (theMenu beforeID)
  (if (numberp beforeID)
      (InsertMenu-internal theMenu beforeID)
    (let* ((mlist (MenuList)))
      (HLock mlist)
      (let* ((menus-so-far (/ (extract-internal (deref mlist) 0 'short) 6))
	     (beforeID (if (zerop menus-so-far)
			   0
			 (let ((first-menu-handle (deref (+ (deref mlist) 6))))
			   (prog2
			    (HLock first-menu-handle)
			    (extract-internal (deref first-menu-handle) 0 'short)
			    (HUnlock first-menu-handle))))))
	(HUnlock mlist)
	(InsertMenu-internal theMenu beforeID)))))

; pascal MenuHandle GetMHandle(short menuID)
;    = 0xA949; 
(deftrap GetMHandle ("a949")
  ((menuID short))
  long)

; pascal void AppendMenu(MenuHandle menu,ConstStr255Param data)
;    = 0xA933; 
(deftrap AppendMenu-internal ("a933")
  ((menu long)
   (data address))
  nil)

(defvar mac-menu-callback-list nil)

(defun AppendMenu (menu data callback)
  (AppendMenu-internal menu (CtoPstr data))
  (setq mac-menu-callback-list (cons (cons (cons menu (CountMItems menu)) callback)
									 mac-menu-callback-list)))

; pascal short CountMItems(MenuHandle theMenu)
;    = 0xA950; 
(deftrap CountMItems ("a950")
  ((theMenu long))
  short)

; pascal void SetItem(MenuHandle theMenu,short item,ConstStr255Param itemString)
;    = 0xA947; 
(deftrap SetItem-internal ("a947")
  ((theMenu long)
   (item short)
   (itemString address))
  nil)

(defun SetItem (theMenu item itemString)
  (SetItem-internal theMenu item (CtoPstr itemString)))

; pascal void SetItemMark(MenuHandle theMenu,short item,short markChar)
;    = 0xA944; 
(deftrap SetItemMark ("a944")
  ((theMenu long)
   (item short)
   (markChar short))
  nil)

; pascal void SetItemStyle(MenuHandle theMenu, short item, short chStyle) ONEWORDINLINE(0xA942)
(deftrap SetItemStyle ("a942")
  ((theMenu long)
   (item short)
   (chStyle short))
  nil)

; pascal void GetItem(MenuHandle theMenu,short item,Str255 itemString)
;    = 0xA946; 
(deftrap GetItem ("a946")
  ((theMenu long)
   (item short)
   (itemString address))
  nil)

; pascal void AddResMenu(MenuHandle theMenu,ResType theType)
;    = 0xA94D; 
(deftrap AddResMenu ("a94d")
  ((theMenu long)
   (theType immediate-string))
  nil)

; pascal void DrawMenuBar(void)
;    = 0xA937; 
(deftrap DrawMenuBar ("a937")
  nil
  nil)

; pascal void DelMenuItem(MenuHandle theMenu,short item)
;    = 0xA952; 
(deftrap DelMenuItem ("a952")
  ((theMenu long)
   (item short))
  nil)
