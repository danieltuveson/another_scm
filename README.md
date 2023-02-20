# Minimal Scheme-like language

## Basic features
- Numbers
- Strings
- Lists (really more like vectors)
- Define
- Lambda
- Recursion

## Example
```scm
(define factorial (lambda (n)
  (if (= n 0)
    1
    (* n (factorial (- n 1))))))
```

## TODO
- Have the interpreter treat internally defined functions like regular lambdas (could probably do this somewhat easily with function pointers)
- Proper lexical scope for closures
- Garbage collector
- Macros (maybe hygenic macros, maybe not)
- Fix any remaining TODO items in eval / parse
- Make repl interface better (add history for arrow keys to browse)
- Start building out a standard library of R5RS functions in Scheme
- Write tests for eval (up until this point, I've been testing in the repl)
- Probably am mutating stuff too much and need to be copying values instead
- Need to actually free stuff when I'm done with it
- Tail recursion. Currently the following:
  ``` scheme
  (define recursion (lambda (x) 
    (if (= 0 x) 
      "done" 
      (recursion (- x 1))))) 
  ```
  segfaults for values of x > ~20000. 
- Should probably get things to a place where out-of-memory errors like this fail somewhat gracefully instead of segfaulting
