# Overview

ENNCPU assennbly has three main components:
- preprocessor directives (e.g. `.INT24 0x512102`)
- labels (e.g. `@START`)
- instructions (e.g. `LDRW A, C+`)

Preprocessor directives are all preceded by a single period. Labels are either
preceded by `@` in the general case, or by `%` for section names (more on that later).

# Code and Instructions

An instruction consists of a mnemonic indicating which operation it is,
and optional operands. The operands of an instruction can be registers,
immediates and, occasionally, labels.
There are eight general-purpose 24-bit registers, named `ABCDEFGH`. 
Immediates are preceded by `#` if they are numeric, or enclosed in
single quotes if they represent a character (e.g. `MOVL A, '%'`).

Due to the extremely limited encoding space, most immediates are limited to
6 or 8 bits (i.e. up to 63, or up to 255). This includes labels and offsets
to addresses. A very small set of instructions can take larger offsets,
and these are all specially prefixed.
