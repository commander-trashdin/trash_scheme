# Language Specification

## Primitive Types
The simplest expression in the language is an object of a primitive type: an integer, a boolean value,
_symbol_ (name of a built-in or user-defined function, variable)

```
42 => 42
-7 => -7
#t => #t
#f => #f
and => #<Symbol and>
quote => #<Symbol quote>
+ => #<Function +>
```

`#t`, `#f` are boolean values.

`and`, `quote`, `+` are symbols, built-in functions.

A _symbol_ without definition will result in an error:

```
hello => Execution error: unbound symbol "hello"
```


The error is given only as an example; specific behavior is not specified.

_symbols_ may only contain ASCII characters. User identifiers can consist only of Latin letters and digits. Case is significant. 
`plus` and `PLUS` are different variables.

## Applying Functions to Primitive Types

Arithmetic expressions with numbers look like this:

```
(+ 1 2) => 3
(* 2 3 4) => 24
(/ 10 2) => 5
(/ 1 2) => 0
```

To call a function, you need to form a _list_ as `(<function symbol> [<function arguments>])`.

### Functions for Working with Integers

1. `+`, `-`, `*`, `/`
2. `=`, `>`, `<`, `>=`, `<=`
3. `min`, `max`, `abs`
Can only be applied to integers.

### Logical Functions

1. `not`

In a logical context, only the value `#f` is false, all
other values (including `0` and the empty list `'()`) are
considered true.

### Predicates
Predicate - a function that checks a certain assumption. Returns a boolean value.
By convention, predicate names always end with `?`

Built-in predicates.
1. `null?`
2. `pair?`
3. `number?`
4. `boolean?`
5. `symbol?`
6. `list?`
7. `eq?`, `equal?`
8. `integer-equal?`


## Lists and Pairs

The only composite type is a pair. Recorded as 
`'(1 . 2)`. Lists are constructed from pairs.

`'(1 2 3)` is the same as `'(1 . (2 . (3 . ())))`.

The empty list `'()` is a synonym for `nullptr`.

[More details](https://www.gnu.org/software/emacs/manual/html_node/elisp/Dotted-Pair-Notation.html)


To suppress the execution of an expression, a special form quote must be used
```
(quote (+ 1 2)) => (+ 1 2)
```


The interpreter does not execute expressions enclosed by this form.
For quote there is a short notation
```
'(+ 1 2) => (+ 1 2)
```

## Allowed Tokens

1. `(` - opening bracket.
2. `)` - closing bracket.
3. `3`, `-6` - number.
4. `'` - single quote. Used as a shorthand for
   the special form `quote`.
5. `.` - dot. Used to record a pair `(1 . 2)` and a list `(1 2 . 3)`.
6. `foo-bar` - represents a variable name in the program. The name cannot
   start with `+` or `-`, except in special cases `+` and
   `-`. *`+1` is a number, while `+` is the identifier `+`*

## Execution Rules
   - `2 => 2` - numbers convert themselves.
   - `a => 4` - symbols convert to the value of the variable.
   - `(+ a 3) => (#<plus-function> 2 3) => #<plus-function>(2, 3) => 5` - lists are evaluated in 2 stages.
    1. All list elements are evaluated recursively.
    2. The value of the first element is interpreted as a function and
       applied to the remaining list elements.
   - `(set! x 1)` - If the first element of the list is a special
     form, then evaluation occurs according to special rules
     for evaluating this form. For example, the `set!` form takes `x` as the first
     argument just as a name. By the rules of function evaluation, `x`
     would have to be converted to the value of the variable `x`

## List of Special Forms

### `if`

Works like if. Two forms of notation are possible.

* `(if condition true-branch)`
* `(if condition true-branch false-branch)`
    
First evaluates `condition` and checks the value for truth
(see definition of truth). Then evaluates either `true-branch`,
or `false-branch` and returns as the result of the whole `if`.

### `quote`

Two forms of notation, full and short.

* `(quote (1 2 3))`
* `'(1 2 3)`
  
Returns the single argument AST without evaluating it.

### Lambda expression.
 
* `(lambda (x) (+ 1 x))`
* `(lambda (x y) (* y x))`
* `(lambda () 1)`
  
Creates a new function. First lists the function's arguments,
then its body. The body can consist of several expressions,
in which case they are evaluated in order and the result of the last
expression becomes the result of the function.

### `and`, `or` - logical expressions with _short-circuit evaluation_.

* `(and)`, `(and (= 2 2) (> 2 1))`
* `(or #f)`, `(or #f (launch-nukes))`
  
Work similarly to `&&` and `||` in C++. Evaluate arguments in
order, stopping when the value of the entire expression can no longer change.

### `define`

Declares a new variable or overwrites a variable with a new value.

* `(define x '(1 2))` - creates the variable `x` with the value `(1 2)`
* `(define (inc x) (+ x 1))` - creates a new function `inc`.

Notation `(define (fn-name <args>) <body>)` is equivalent to
`(define fn-name (lambda (<args>) <body>))`

### `set!`

`set!` changes the value of an existing variable. If the variable
did not exist, `set!` ends with an error.

* `(set! x 1)`

## List of Built-in Functions

### Predicates

1. `null?`
2. `pair?`
3. `number?`
4. `boolean?`
5. `symbol?`
6. `list?`
7. `eq?`, `equal?`
8. `integer-equal?`

### Logical Functions

1. `not`

### Functions for Working with Integers

1. `+`, `-`, `*`, `/`
2. `=`, `>`, `<`, `>=`, `<=`
3. `min`, `max`, `abs`
 
### Functions for Working with Lists

1. `cons`
2. `car`, `cdr`
3. `set-car!`, `set-cdr!`
4. `list`
5. `list-ref`, `list-tail`