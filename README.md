# Curly
Curly is a functional programming language that focuses on iterators. Some of its main implementation features include lazy evaluation, list comprehensions, and quantifiers.

## Example
```
primes = n in (from 2) where
    for all p in (range 2 n)
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
The parser is done, and the type checker is mostly done. Current effort is focused on the LLVM backend. Everything is highly experimental. Be cautious: code may be explosive.
