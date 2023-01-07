# Minimal Scheme-like language

## Basic features
- Numbers
- Strings
- Lists (really more like vectors)
- Define
- Lambda
- Recursion

## TODO
- Figure out why:
  ``` scheme
  (define recursion (lambda (x) 
    (if (= 0 x) 
      "done" 
      (recursion (- x 1))))) 
  ```
  segfaults for values of x > ~20000. 
- I think it's reaching the maximum depth it can before running out of memory. Once I add tail recursion, this probably won't be a problem. 
- Also looks like this is an issue in e.g. Python for even lower values:
  ``` python 
  def recursion(x):
      if x == 0:
          return "done"
      else:
          return recursion(x-1)
  ```
- Should probably still get things to a place where out-of-memory errors like this fail somewhat gracefully
- Fix any remaining TODO items in eval / parse
- Add "load" option, that way I can test larger chunks of code without needing the repl
- Make repl interface better (add history for arrow keys to browse)
- Start building out a standard library
- Write tests for eval (up until this point, I've been testing in the repl)
- Probably am mutating stuff too much and need to be copying values instead
- Need to write a garbage collector
- Need to actually free stuff when I'm done with it