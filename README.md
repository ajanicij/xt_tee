# xt_tee
Extended tee command that prepends timestamps to its output

## Introduction

xt_tee is a command-line tool written in C that acts like a simple
version of tee command with added timestamps.

I wrote it with the help of ChatGPT, as an experiment to see how much faster
it is to develop software that way and how much I have to fix the code that
ChatGPT produces. See
[steps](https://github.com/ajanicij/xt_tee/tree/main/steps) for the steps
in development.

All in all:
- It took about a few hours, which is probably an order of magnitude less
  that what it would have taken me by hand.
- I developed in two ChatGPT sessions:
  - Source code
  - Build system for CMake
- There was only one thing I had to fix in ChatGPT-generated code. It was
  in the code that processes command-line parameters.

## Building

xt_tee is built using CMake.

- cd to the project's root directory.
- mkdir build
- cd build
- cmake ..
- cmake --build .

## Installing

cmake --install .

## Building a DEB installer

    cpack -G DEB

## Example usage

    ls | ./xt_tee -t relative -T both -C always output.txt

This command pipes the output of ls to xt_tee with the following options:
- Relative timestamps in ms from the beginning of the run of the program.
- Apply timestamp to both standard output and file output.
- Colorize the timestamp always (regardless of whether the stdout is a
  terminal of file).
- Write the output to file output.txt.

## Help

    xt_tee -h

Displays:

```
xt_tee is an extended version of tee command that prepends timestamps to its output
Usage: xt_tee [OPTIONS] [FILE]...

Reads from standard input and writes to standard output and files.

Options:
  -a, --append                Append to files instead of overwriting
  -i, --ignore-interrupts     Ignore SIGINT (Ctrl+C)
  -t, --timestamp=TYPE        Add timestamp: 'absolute' or 'relative'
  -T, --timestamp-target=DEST Where to apply timestamp: 'stdout', 'files', 'both'
  -C, --color[=WHEN]          Colorize timestamps on stdout: 'always', 'never', 'auto'
  -h, --help                  Show this help message and exit
  -v, --version               Print program version
```

## License

MIT license.
