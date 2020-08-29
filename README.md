# Curly
Curly is a functional programming language designed to be simple and readable. Some of its main implementation features include lazy evaluation, list comprehensions, and quantifiers.

## Example
```python
primes = [
  n in 2.. where
    for all p in (p in primes such that p*p <= n)
      n % p != 0
]
```

## Build
Just type in the following:
```bash
git clone https://github.com/jenra-uwu/curly-lang && cd curly-lang && make
```
Note: This repo has only been tested on macOS, so it may not build on other \*nix operating systems, and it most *definitely* won't build on Windows.

## Progress
The parser is done, but nothing else is implemented yet. Everything is highly experimental. Be cautious: code may be explosive.

### Branch info
There are four branches:
 1. `master`: This is the main branch.
 2. `vm-initial`: This is where new features of the vm were created and tested.
 3. `vm-compiler`: This is where new features of the compiler were created and tested.
 4. `redo`: This is where the language is being reimplemented.

The language is currently being reimplemented due to a lack of maintanance in the past few months. Additionally, it allows the language to remove its only dependancy (curly-comb).
