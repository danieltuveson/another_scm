;;; This document contains standard functionality from R5RS 
;;; that is able to be implemented directly in Scheme
;;; TODO: Most of this


(define null? (lambda (val) (eq? val (quote ()))))

;; Math functions
(define map
  (lambda (fun ls)
    (if (null? ls)
      (quote ())
      (cons (fun (car ls)) (map fun (cdr ls)))))) 

;; (define mult-helper
;;   (lambda (n acc)
;;     (if (
;; (define *
;;   (lambda n
;;     (

; Checks if number is zero 
(define zero? 
  (lambda (z) 
    (= z 0)))

; Checks if positive / negative
(define positive? 
  (lambda (x) 
    (> x 0)))
(define negative? 
  (lambda (x) 
    (and (not (= x 0)) (not (positive? x)))))

(define even? 
  (lambda (x)
    (integer? (/ x 2))))

(define odd? 
  (lambda (x)
    (and (integer? x) (not (even? x)))))

(define max-min-helper
  (lambda (cmp val lst)
    (if (null? lst)
      val
      (if (cmp val (car lst))
        (max-min-helper cmp val (cdr lst))
        (max-min-helper cmp (car lst) (cdr lst))))))

(define max
  (lambda x
    (max-min-helper > (car x) (cdr x))))

(define min
  (lambda x
    (max-min-helper < (car x) (cdr x))))


;; Boolean functions

(define not (lambda (x) (if x #f #t)))


;; List functions

(define reverse-helper
  (lambda (lst acc)
    (if (null? lst)
      acc
      (reverse-helper 
        (cdr lst)
        (cons (car lst) acc)))))

(define reverse
  (lambda (lst) 
    (reverse-helper lst (quote ()))))

(define list (lambda x x))

; Variations on car / cdr
(define caar (lambda (x) (car (car x))))
(define cadr (lambda (x) (car (cdr x))))
(define cdar (lambda (x) (cdr (car x))))

(define caaar (lambda (x) (car (car (car x)))))
(define caadr (lambda (x) (car (car (cdr x))))) 
(define cadar (lambda (x) (car (cdr (car x)))))
(define cdaar (lambda (x) (cdr (car (car x)))))
(define cddar (lambda (x) (cdr (cdr (car x)))))
(define caddr (lambda (x) (car (cdr (cdr x)))))
(define cdadr (lambda (x) (cdr (car (cdr x)))))
(define cdddr (lambda (x) (cdr (cdr (cdr x)))))

