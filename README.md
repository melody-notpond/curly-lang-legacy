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
The parser is being implemented in the parser branch. Everything is highly experimental. Be cautious: code may be explosive.
