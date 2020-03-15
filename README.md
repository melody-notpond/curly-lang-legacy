# Curly
Curly is a functional programming language designed to be simple and readable. Some of its main implementation features include lazy evaluation, list comprehensions, and quantifiers.

## Example
```python
primes = [
  n in 2.. such that
    for all p in (p in primes such that p*p <= n)
      n % p != 0
]
```

## Build
Just type in the following:
```
git clone --recursive https://github.com/jenra-uwu/curly-lang && cd curly-lang && make
```
Note: This repo has only been tested on macOS, so it may not build on other \*nix operating systems, and it most *definitely* won't build on Windows.

## Progress
The parser is virtually done for the sake of creating a backend to test with. Everything is highly experimental. Be cautious: code may be explosive.

### Branch info
There are three branches currently in use:
 1. `master`: This is the main branch.
 2. `vm-initial`: This is where new features of the vm are created and tested.
 3. `vm-compiler`: This is where new features of the compiler are created and tested.
 
Development of a new feature starts in `vm-initial`. Once the appropriate opcodes are sufficiently implemented, it is merged into `vm-compiler` and the appropriate compilation steps are implemented.
