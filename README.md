Manual
======

### Building

Building the main program:

`gcc logic.c -O2 -o logic`

Building the test suite:

`gcc test.c -O2 -o test`

Note: keep the `testcases.txt` and the executable `test` under the same directory.

### Running

Usage: `./logic [d|t]`, where:
* `d`: digest only
* `t`: truth table mode

#### No arguments: Expression mode (verbose)

Analyses a given infix logical expression. Prints a truth table, a CNF and a DNF.

Possible atomic propositions are the 26 English letters and are case insensitive. Among them, `T` is assigned ⊤ and `F` is assigned ⊥, and the remaining 24 are variable.

All whitespaces identifiable by the `isspace` standard library function will be ignored.

Supported operators, in descending precedence:
* __Negation (Not)__: `!`
* __Conjunction (And)__: `&`
* __Disjunction (Or)__: `|`
* __Material conditional (Implies)__: `^` or `>`
* __Material equivalence (Iff)__: `~` or `=`

All operators except Negation are left associative; that is, `a~b~c` will be interpreted as `(a~b)~c` (contrary to some conventions where it's right associative).

Negation is the only unary operator and is a prefix notation.

The order for the variable assignments in the truth table is a binary counter with each bit assigned to a variable in opposite orders of bit significance and lexicographic ranks of variable names.

#### `d`: Expression mode (digest only)

Digest-only mode produces a more machine-readable output. It produces a string of `0`'s and `1`'s representing each row in the truth table. The order is the same as that in verbose mode.

#### `t`: Truth table mode

Produces a valid logical expression given a truth table in the format of digest-only mode's output.

The input should consist of two lines: the first should contain an integer _N_ between 1 and 24 (inclusive) denoting the number of variables; the second should contain a binary string of 2<sup>_N_</sup> characters representing the truth table.

Variable names are allocated in increasing lexicographic order; `T` and `F` are skipped.

The expression is only guaranteed to be valid; it's usually far longer than is necessary.

### Test suite

Usage: `./test [<program> [<cases-file>]]`
* `<program>`: the path to the executable to be tested, defaults to `./a.out`;
* `<cases-file>`: the path to the file containing all test cases, defaults to `testcases.txt`.

The format for a test case is a line containing an expression followed by the expected truth table produced in digest mode. Lines starting with a `#` character are ignored and do not interrupt a two-line pair. Empty lines are **not** ignored.

In case of Mozibake, comment the `change_colour()` implementation and re-compile. The most probable cause is that the terminal does not recognize ANSI escape codes, as is the case with Windows CMD before Windows 10 TH2.
