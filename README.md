# EEE — EnnCPU Ennplementation/Ennmulator

----

Copyright © 2022–2023 @the.enemy

Released under the BSD License.

The aim of this project is to create an emulator for a custom ISA and processor, tentatively named EnnCPU (after my internet handle). This is supposed to be the reference implementation of this elementary processor design, incorportaing the full default feature-set in a simple but validatable and testable virtual CPU. It aims to ultimately be instruction-accurate for this custom CPU design, but does not (yet) assume cycle accuracy.

----

## Building

The project as-is does not have any unexpected dependencies. To build a minimum working binary, you tentatively need `make` and a C++ compiler compatible with the following C++14 features:

* binary literals
* generic lambdas

The oldest GCC that will be able to build the repository with minimal modification will be v4.9.0 (released in April 2014). In the event that older compiler support becomes desirable or even necessary, this can be subject to change.

Further functionality can in the future be extended with:

* SDL2 (to enable a graphical frontend and a reasonable interactive environment)
* FMT (for prettier terminal output, debugging and info logs)

Currently, the only intended build methods include either the alpha build script `build.sh` (to be removed in future versions) or the more fully-fledged build via a `make` script (intended to be the only compilation method later on).

----

## The CPU and ISA

The ISA (tentatively called EnnISA) is an instruction set for a 24/32-bit CPU, with instructions fixed to 16 bits in width to make decoding and pipelining easier (if in the future it expands to include this). There are 16 general-purpose registers that are each 32-bits wide. They are divided into a high and low register bank, which can be toggled with an instruction; this is used to save encoding space. 

There are two privilege levels in the CPU, to facilitate a kernel-vs-userspace divide. The kernel additionally has its own shadow register space with two banks, inaccessible to userspace programs, and are automatically toggled whenever the processor switches contexts between kernel and userspace.

The CPU can theoretically address a full 32-bit memory space, divided into 256 separate 24-bit pages visible to the CPU at any one moment. This is handled through a simple MMU module which read access attempts from both the kernel and userland, and either provides the requested memory or blocks access to it and raises an exception in case the type of access attempted is disallowed. The pages themselves can be marked as readable, writable and executable for either the kernel or userspace. This kind of MMU is one of the baseline requirements needed to run a Linux on a processor.

Practically all instructions (that is, all except NOPs, specials and unassigned codes) are predicated: the encoding allocates two bits per instruction reflecting its condition status. These are consistently mapped to bits 9 and 10 of every instruction, corresponding to the results of the one conditional operator implemented in the architecture, the spaceship operator. They reflect the results of the operation `cmp r1, r2` (in C++20: `r1 <=> r2`) in the following manner:

binary | description
--- | ---
00 | this instruction is always enabled
01 | enabled if r1 > r2
10 | enabled if r1 < r2
11 | enabled if r1 = r2

<br>

The ISA is resembles a traditional load-store architecture, with very few addressing modes (almost exclusively just either immediate addressing or register addressing), reflecting a sort of RISC design principle. The limitations of the fixed-width instruction encoding necessitate small immediates, and as such instructions which *do* take immediates (these being a minority)

It does not have any floating point capabilities, compensating for this with hardware fixed-point arithmetic with 8 bits of integral and 24 bits of fractional precision. To this end, the ISA exposes several operations typically reserved for floating point units, such as trigonometric operations, squaring and roots.

It also includes some 'joke' instructions, such as phinary arithmetic, because I find it phenomenally funny. Implementation of these is supposed to be 'optional' and they will, in more serious iterations of this design, probably be replaced with more useful instructions.

EnnISA is not yet explicitly described in detail; a preliminary visual representation of the instructions and their encodings can be found [here](./misc/enncpu-isa.png). It is not finalised and exists in a temporary state of flux: the chart will eventually be superceded by a code implementation in this repository.

## Roadmap

- [ ] skeletal `main()`
- [ ] basics of register implementation
- [ ] register banking
- [ ] shadow registers
- [ ] program counter and instruction reading
- [ ] decoding of instructions

 