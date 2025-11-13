;;;; yam lisp interface

;;;; Copyright (C) 2025 Simon Rehn.

;;;; This file is part of yam.

;;;; yam is free software: you can redistribute it and/or modify
;;;; it under the terms of the GNU General Public License as published by
;;;; the Free Software Foundation, either version 3 of the License, or
;;;; (at your option) any later version.

;;;; yam is distributed in the hope that it will be useful,
;;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;;; GNU General Public License for more details.

;;;; You should have received a copy of the GNU General Public License
;;;; along with yam.  If not, see <https://www.gnu.org/licenses/>.

(cl:in-package :cl-user)

(defpackage #:yam
  (:use #:cl)
  ;; Constants
  ;; TODO: The event interface probably needs some improvements
  (:export #:+event-none+
           #:+event-quit+)
  ;; Functions/macros
  (:export #:with-window
           #:clear-window
           #:present-window
           #:set-pixel-color
           #:poll-event))

(in-package :yam)

(defconstant +event-none+ 0)
(defconstant +event-quit+ 1)

(defmacro with-window ((title width height) &body body)
  `(unwind-protect
        (progn
          (yam::create-window ,title ,width ,height)
          ,@body)
     (yam::destroy-window)))

(define-condition yam-error (error)
  ((code :initarg :code :reader yam-error-code)
   (message :initarg :message :reader yam-error-message))
  (:report (lambda (condition stream)
             (format stream "[YAM] error (~A): ~A"
                     (yam-error-code condition)
                     (yam-error-message condition)))))

(defun yam-true-p (bool)
  (= bool 1))

(defun yam-false-p (bool)
  (= bool 0))

;; TODO: Change this when build system is added
(cffi:define-foreign-library libyam
  (t (:default "libyam")))
(cffi:use-foreign-library libyam)

(cffi:defctype yam-bool :int)

(cffi:defcfun "yam_get_error" :string)
(cffi:defcfun "yam_get_error_code" :int)

(cffi:defcfun ("yam_create_window" yam_create_window) yam-bool
  (title :string)
  (width :int)
  (height :int))

(cffi:defcfun ("yam_destroy_window" yam_destroy_window) yam-bool)

(cffi:defcfun ("yam_clear_window" yam_clear_window) yam-bool)

(cffi:defcfun ("yam_present_window" yam_present_window) yam-bool)

(cffi:defcfun ("yam_set_pixel_color" yam_set_pixel_color) yam-bool
  (x :int)
  (y :int)
  (r :unsigned-char)
  (g :unsigned-char)
  (b :unsigned-char)
  (a :unsigned-char))

(cffi:defcfun ("yam_poll_event" yam_poll_event) :int)

(defun %yam-error ()
  (error 'yam-error :message (yam-get-error)
                    :code (yam-get-error-code)))

(defmacro yam-check (form &optional &key (predicate #'yam-true-p))
  (let ((result (gensym)))
    `(let ((,result ,form))
       (unless (funcall ,predicate ,result)
         (%yam-error)))))

(defun create-window (title width height)
  (yam-check (yam_create_window title width height)))

(defun destroy-window ()
  (yam-check (yam_destroy_window)))

(defun clear-window ()
  (yam-check (yam_clear_window)))

(defun present-window ()
  (yam-check (yam_present_window)))

(defun set-pixel-color (x y r g b a)
  (yam-check (yam_set_pixel_color x y r g b a)))

(defun poll-event ()
  (let ((result (yam_poll_event)))
    (yam-check result :predicate (lambda (result) (/= result -1)))
    result))
