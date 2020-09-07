# Curly
Curly is a functional programming language that focuses on iterators. Some of its main implementation features include lazy evaluation, list comprehensions, and quantifiers.

## Example
```
primes: *Int = n in (from 2) where
    for all p in (p in primes where p*p <= n)
        n % p != 0
```

## Build
Just type in the following:
```bash
git clone https://github.com/jenra-uwu/curly-lang && cd curly-lang && make
```
This project depends on `libedit-dev`/`readline` (for Linux and macOS respectively) and `llvm`, which can each be installed using your favourite package manager (`apt`/`pacman`/`yum` for Linux and Homebrew/MacPorts for macOS).

Note: This repo has been tested on macOS and Ubuntu and as of now, but will not build on Windows. Windows support is coming soon.

## Progress
The parser is done, but nothing else is implemented yet. Everything is highly experimental. Be cautious: code may be explosive.

### Branch info
There are four branches:
 1. `master`: This is the main branch.
 2. `vm-initial`: This is where new features of the vm were created and tested.
 3. `vm-compiler`: This is where new features of the compiler were created and tested.
 4. `redo`: This is where the language is being reimplemented.

The language is currently being reimplemented due to a lack of maintanance in the past few months. Additionally, it allows the language to remove its only dependancy (curly-comb).
