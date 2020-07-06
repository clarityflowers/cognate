## The Cognate programming language
Cognate is a stack oriented, dynamically typed, functional programming language designed to help programmers specify their intent. Cognate uses optional syntax in order to help programmers write statements that are (mostly) readable in plain English and to include extra information about what statements are for, not just what they do and how they work. This leads to more maintainable codebases. Cognate's syntax is much simpler than other languages that aim for readability such as python, making the language consistent and simple to implement. This repository contains CognaC - the Cognate Compiler.

![Program to compute the 42nd Fibonacci number](fibonacci.png?raw=true)

### Cognate is readable
As the above program shows, words starting with lower-case letters are ignored by the compiler (and are not highlighted). This allows comments and code to be easily bundled together, enhancing readability and allowing programmers to more easily express their intentions. While this means that Cognate can be very verbose, verbosity can be a good thing - this 'informal syntax' gives extra information to anyone reading the source.

### Cognate is fast
Cognate is compiled directly to C. Even my naive attempt at a compiler is able to outperform most interpreted languages, leaving python and ruby in the dust. The included 'fib' example already runs at within 2x the performance of its Haskell equivalent (compiled with GHC and -O3). As time goes on, I will be able to optimise the compiler and gain even more performance. Furthermore, cognate produces very small binaries and could be used for a high level language in an embedded environment.

### Cognate is simple
Cognate has only a handful of syntax elements. In future, this will allow me to elegantly introduce compile time macros and reflection, similar to those found in lisp dialects, that manipulate the syntax tree directly. I am also looking at implementing Forth-style code metaprogramming with immediate words, etc. 

### Cognate is functional
Cognate is optimised for functional programming - not complicated over-the-top (in my opinion) functional programming with monads and applicative functors as you might see in a Haskell program, but the dynamically typed functional programming often seen in scheme and other lisp dialects. In Cognate, a bracketed expression (known as a Block) represents a closure - an anonymous function that inherits variables from where it was defined. These allow functions to be passed as arguments and returned from other functions. Cognate also discourages variable mutation in favour of shadowing variables instead and limiting side-effects.

### Cognate is powerful
The stack, around which cognate is designed, allows Cognate to do things which many other languages cannot. One example is multiple return values. While other languages - such as python - require the use of tuples to return multiple values, Cognate uses the stack to achieve this without the use of any data structures. Cognate also uses the stack to allow expressions to be written in point-free notation.

### Building Cognate
Currently, Cognate is not in a particularly usable state, and thus, you probably don't want to install it. Cognate currently does not have a build system, but is not too hard to build on Linux systems. These instructions explain how to build the CognaC compiler for Cognate from this repository.
1) Install git, ghc, cabal-install, clang, tar, wget, and make through your distribution's package manager
2) Run ```git clone https://github.com/stavromuellerbeta/cognate```.
3) Run ```cd cognate```.
4) Run ```chmod +x SETUP BUILD```
5) Run ```./SETUP``` to setup the environment.
6) Run ```./BUILD``` to build the compiler.
7) CognaC is installed. Compile the example 'fib' program with ```./cognac examples/fib.cog``` and run with ```./examples/fib```.
