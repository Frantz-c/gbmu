# Calling Convention

## 1. Disclaimer

This calling convention is **not** the official calling convention for gameboy assembly.
It has been written to facilitate the use of the command creation on this software.
Therefore, do not expect it to work on another emulator with this functionality.

## 2. Why a calling convention ?

The objective of a calling convention is to allow developers to use functions that are
not written by themselves. It is a sort of code uniformization. We created this calling
convention with an aim: make our work usable by anyone who would like to dig in
Gameboy assembly development.

## 3. Rules

### 3.1 Syntax

- Code is case-insensitive (except for macros and functions).

- The operand order is Intel-like (destination to the left, source to the right).

- Functions usually start with an underscore (e.g. `_some_function`).

- Builtin functions start with the prefix `___builtin_`,
  followed by the builtin name (e.g. `___builtin_div`).

### 3.2 Function calls

- Parameters for function call are transmitted in this order:
  - C (or BC if the parameter is 16-bit sized);
  - E (or DE if parameter is 16-bit sized);
  - L (or HL if parameter is 16-bit sized);
  
- If more parameters are needed, they are transmitted to the stack.

e.g.:
> function_one (8-bit value1, 16-bit value2, 8-bit value3)

Parameter transfer: C (value1), DE (value2), L (value3)

> function_two (8-bit value1, 8-bit value2, 16-bit value3)

Parameter transfer: C (value1), E (value2), HL (value3)

> function_three (8-bit value1, 16-bit value2, 8-bit value3, 16-bit value4)

Parameter transfer: C (value1), DE (value2), HL (value3), (SP + 2) (value4)

- Return value is stored in the accumulator (A), or the HighLow pointer
  (HL) if 16-bit sized.

- *AF and HL* registers are caller-saved, which means that the caller must save its registers before
  calling the function if he wants to keep them. BC and DE are callee-saved.

- The caller must provide 32 bytes of usable memory below the stack pointer to
  ensure that the callee will not overwrite used data. This implies
  that the callee can use up to 32 bytes of data below the stack pointer.
