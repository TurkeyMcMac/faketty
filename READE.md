# faketty

Some programs output different things depending on whether they are writing to a
terminal. Some behave differently depending on whether the input is a terminal.
faketty provides a uniform way to test such differences. For more information,
see `./faketty -h` or `man ./faketty.1.gz`.

## Implementation and Portability

faketty intercepts calls to `isatty` by overriding `LD_PRELOAD`. It sets the
variable to the path of its own executable, discovered through `/proc/self/exe`.
It must be compiled, therefore, to be both executable and dynamically linkable.
GCC is the only C compiler for which I know this to be possible. The code also
makes use of the non-standard `__attribute__` keyword. Therefore, this code is
not portable; it requires Linux and GCC.
