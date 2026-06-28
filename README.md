# The Linux Programming Interface

This repository contains my implementations, exercises, and experiments while studying **The Linux Programming Interface** by Michael Kerrisk.

The purpose of this repository is to document my progress through the book by writing the examples myself, completing exercises, and experimenting with Linux system programming concepts.

## Repository contents

Each directory corresponds to a chapter/topic from the book.

For example:

```
04_file_io/
05_file_io/
06_processes/
...
```

Each chapter directory contains my work related to that part of the book, including:

- My own implementations of the listings/examples from the book.
- Solutions to exercises.
- Small experiments and modifications I create to better understand the concepts.
- Test files or input data required to run the examples.

The code in this repository is written by me while studying the material. It may differ from the implementations provided with the book.

## TLPI helper library

The book provides a helper library (`tlpi_hdr.h`, `errExit()`, and related utilities) to simplify examples and keep the focus on the concepts being demonstrated.

For my implementations, I will generally **not use the TLPI helper library**. Instead, I will use standard C/POSIX facilities and implement the examples using the underlying APIs directly.

This is intentional so I can better understand:
- system calls,
- error handling,
- `errno`,
- return values,
- and the behavior of the Linux APIs themselves.

## Building

Programs can usually be compiled using:

```bash
gcc -Wall -Wextra -Wpedantic -g program.c -o program
```

Some directories may contain a `Makefile` to simplify building the examples.

## Testing

Some examples require input files or test data to demonstrate their behavior. These files are kept alongside the relevant chapter code.

Generated files such as:
- compiled binaries,
- object files,
- program output,
- temporary files

are not included in the repository.

## Disclaimer

This is a personal study repository.

The code here represents my own implementations and experiments while learning from *The Linux Programming Interface*. It is not intended to replace or redistribute the official examples, source code, or solutions provided with the book.

The official TLPI website:
https://man7.org/tlpi/
