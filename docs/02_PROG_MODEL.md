# Overview

ENNCPU assennbly has three main components:
- preprocessor directives (e.g. `.INT24 0x512102`)
- labels (e.g. `@START`)
- instructions (e.g. `LDRW A, C+`)

Preprocessor directives are almost all  preceded by a single period. Labels are either
preceded by `@` in the general case, or by `%` for section names (more on that later).

Additionally, ENNCPU assennbly supports line comments that start with `;`.

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

# Program structure

Execution is strictly sequential: ENNCPU is not pipelined and doesn't
do speculative execution or branch prediction. 

The entry point for a cold boot of the ENNCPU is the address 0x000000.
Memory can be divided into sections, which are handled by the preprocessor.
It is vital for there to be at least one section in the code, so that the
program loader will know where in memory to load the binary blob to.
A minimal ENNCPU program thus looks like this:

```
.ORG 0x0000
.SEC %START
	ERR
%START
```

The `.SECTION` directive has the preprocessor include eight bytes of bookkeeping at the start
of the section, and thus the first instruction to be included and executed 
(in this case `ERR`) will be located at `0x0008`, assuming the first section starts at zero.

A program can consist of a large number of sections, and can be made up of a number of files;
the assembler supports including whole files verbatim (which can iteratively include
other files) through the directive `$include [filename]`.

# Basic instructions

There are a few broad classes of instructions:

- loading values into registers (`MOV`, `ADRL`, ...)
- memory operations (`PSHB`, `STRW`, ...)
- arithmetic and bit manipulation (`ADD`, `BXOR`, ...)
- conditional tests (`CGT`, `CXOR`, ...)
- floating point (`FADD`, `FLOG`, ...)
- jumps/branches (`JMR`, `JLA`, ...)
- manipukating internal registers (`WSP`, `RXS`, ...)
- miscellanea

# Alignment and memory

While ENNCPU allows for unaligned reads, instructions must be aligned to even addresses.
Although it is possible to get the IP to hold an unaligned value, fetches and jumps all
occur on even addresses, and the CPU should realign itself after one read.

There are no requirements for alignment of reads or writes, pushes or pops. As the
address space is 24-bit, trying to increment or decrement beyond it wraps around.
All reads amd writes happen in the scope of a page. Since memory is paged in 4K chunks, 
accesses larger than a byte (e.g. `LDRW` to read a 16-bit word) will wrap around if they're
made at a page boundary (that is, trying to read two bytes from `0x0fff` will read one byte
from `0x0fff` and one byte from `0x0000`). This also applies to stack accesses (so make sure
your stack is not hugging the page boundary of the first page!), as the stack also goes
through the MMU interface like normal.

# Limitations

The reference ennmulator has some limitations:

- technically limited to 2^34 ticks (no infinite looping)
- can load only one binary file
- can load only one floppy image that must be 1440K
