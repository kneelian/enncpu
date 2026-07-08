
# General Concepts

ENNCPU is a fantasy 24-bit CPU roughly meant to fit an eighties-nineties alternative computational
history aesthetic. It's inspired by several real-world chips, and is meant to be somewhat serious
as an ISA.

## Design outline

- 8 general-purpose 24-bit registers (named `ABCDEFGH`)
- rough separation into two privilege classes (kernel and user)
- 24-bit address space
- MMU supporting 4KB pages, remapping of pages, and permission control
- fixed-width, 16-bit instructions
- all instructions can be conditionally executed based on internal flag
- general RISC-like load-store architecture
- orthogonal instruction set
- two-operand instructions (one of the sources is the destination)
- support for floating point instructions (FP24) without a separate FP registry

The main design inspirations for the ENNCPU include:
- ARM7 (predication of every instruction)
- RISC-V (esp. extensions `Zfinx` and `P`)
- ez80 (24-bit instructions)

This repository contains the ENNCPU emulator and the assennbler that produces object files. 

## Running

To build ENNCPU, you need a sufficiently new C++ compiler that supports C++20 due to some
reliance on newer language features. Currently, both the ennmulator and the assennbler
function only as commandline programs.

The ennmulator, built with Clang 16.0 and  `-Og -g`, achieves around 65-100M emulated
instructions a second on a Macbook Pro M1, depending on the exact workload; memory-heavy
workloads take somewhat more time, and most of it is wasted on indirection through the MMU
rather than anything more serious.

## Future optimisations

There is a lot of room for improvement in the code. While the ennmulator code isn't particularly
bad, it is obviously the product of long-term iteration, copying and pasting, and has accrued
a lot of technical debt. The main future optimisations I have in mind include implementing a
prefetch/instruction cache that decodes multiple instructions ahead of time. I've previously
included a prototype version of such a thing in the codebase, but due to certain complications
involving stateful decoding, this had to be torn out.

Future versions of the ENNCPU might do away with stateful decoding entirely, whose presence is
the result of prior attempts at saving space. This has caused a lot more headaches than it's
helped solve, meaning it's on its way out. This will enable easier prefetch and AOT decoding,
reducing the burden on memory accesses.

There are no plans for adding JIT support in the future, as I want the code to stay maximally
platform-independent (that is, architecture-independent). 
