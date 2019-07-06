# Calling Convention

## 1. Warnings

This calling convention is **not** the official calling convention for gameboy assembly.
It has been written to facilitate the use of the command creation on this software.
Therefore, do not expect it to work on another emulator with this functionality.

## 2. Why a calling convention ?

The objective of a calling convention is to allow developers to use functions that are
not written by themselves. It is a sort of code uniformization. We create this calling
convention with an aim: make our work usable by everyone who would like to dig in
Gameboy assembly development.

## 3. Rules

### 3.1 Syntax

- Code is case-insensitive (except for macros and functions).

- The operand order is Intel-like (destination to the left, source to the right).

- There may be several syntaxes for the same usage:
  
  | Official syntax | Supported variants |
  |:---------------:|:------------------:|
  | ld              | mov                |
  | stop            | stop 0             |
  | sbc             | sbb                |
  | jr,jp           | jmp                |
  | jr c, jp c      | jc, jrc, jpc       |
  | jr nc, jp nc    | jnc, jrnc, jpnc    |
  | jr z, jp z      | jz, jrz, jpz       |
  | jr nz, jp nz    | jnz, jrnz, jpnz    |
  | call c, call nc | callc, callnc      |
  | call z, call nz | callz, callnz      |
  | ret c, ret nc   | retc, retnc        |
  | ret z, ret nz   | retz, retnz        |
  | cp              | cmp                |
  | sla             | shlx               |
  | sra             | sarx               |
  | srl             | shrx               |
  | bit             | testb              |
  | res             | reset              |
  | a, b, c, d...   | ax, bx, cx, dx...  |
  | hli, hld        | hl++, hl--         |
  | ld a, (c)       | ld a, ($ff00 + c)  |
  | ldh a, (imm8)   | ld a, ($ff00 + imm8)|
  | ld hl, sp + imm8| ldhl sp, imm8      |
  | (address)       | [address]          |
  | $12             | 0x12, 18, 022, 0b10010|
  
- Functions usually start with an underscore (e.g. `_some_function`).

- Builtin functions start with the prefix `___builtin_`,
  followed by the builtin name (e.g. `___builtin_div`).

### 3.2 Function calls

- Parameters for function call are transmitted in this order:
  - B (or BC if the parameter is 16-bit sized);
  - C (if parameter isn't 16-bit sized);
  - D (or DE if parameter is 16-bit sized);
  - E (if parameter isn't 16-bit sized);
  - H (or HL if parameter is 16-bit sized);
  - L (if parameter isn't 16-bit sized);
  
- If more parameters are needed, HL becomes a pointer to the rest of
  the parameters, organized as a stack (i.e. first parameter is pointed by HL,
  with other parameters below).

e.g.:
> function_one (8-bit value1, 16-bit value2, 8-bit value3)

Parameter transfer: B (value1), DE (value2), H (value3)

> function_two (8-bit value1, 8-bit value2, 16-bit value3)

Parameter transfer: B (value1), C (value2), DE (value3)

> function_three (8-bit value1, 16-bit value2, 8-bit value3, 16-bit value4)

Parameter transfer: B (value1), DE (value2), HL (pointer to a "stack"
containing value3 and value4)

- Return value is stored in the accumulator (A), or the HighLow pointer
  (HL) if 16-bit sized.

- *All* registers are caller-saved, which means that the caller must save its registers before
  calling the function if he wants to keep them.

- Data below the stack pointer (SP) is caller-saved,
  data above the stack pointer is callee-saved.

- The caller must provide 128 bytes of usable memory below the stack pointer to
  ensure that the callee will not overwrite used data. This implies
  that the callee can use up to 128 bytes of data below the stack pointer.
