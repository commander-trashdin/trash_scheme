# scheme

The language will consist of:
 - Primitive types: integers, booleans, and _symbols_ (identifiers).
 - Composite types: pairs and lists.
 - Variables with syntactic scope.
 - Functions and lambda expressions.

```
1 => 1
(+ 1 2) => 3
```

The `=>` notation in examples here and elsewhere separates the expression and the result of its execution.

[language specification](spec.md) is described in a separate document.

## Expression Execution
Execution of the language occurs in 3 stages:

**Tokenization** - converts the program text into a sequence
   of atomic tokens. 

**Syntax Analysis** - converts the sequence of tokens
   into an [AST](https://en.wikipedia.org/wiki/Abstract_syntax_tree). In Lisp-like programming languages, the AST is
   represented as lists.
   
**Evaluation** - recursively traverses the AST of the program and transforms it
   according to a set of rules.

### Example

The expression 
```
(+ 2 (/ -3 +4))
```

as a result of tokenization will be transformed into a list of tokens:
```
{ 
    OpenParen(),
    Symbol("+"),
    Number(2),
    OpenParen(),
    Symbol("/"),
    Number(-3),
    Number(4),
    CloseParen(),
    CloseParen()
}
```

The sequence of tokens as a result of syntax analysis
will be transformed into a tree:
     
```
Cell{
    Symbol("+"),
    Cell{
        Number(2),
        Cell{
            Cell{
                Symbol("/"),
                Cell{
                    Number(-3),
                    Cell{
                        Number(4),
                        nullptr
                    }
                }
            }
            nullptr
        }
    }
}
```


The result of executing the expression will be 

```
(+ 2 (/ -3 +4)) => 1
```