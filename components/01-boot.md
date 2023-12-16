# EnnCPU Booting

After initialisation/power-on, the CPU is in kernel mode, which implies the following:

- program page = 0
-- IP = 0x0
- memory page = 1
-- SP = 0xffffff

The first instruction is 0x0 from page 0.

At boot, all interrupts are disabled, all registers are guaranteed to be zero, though not all locations in mempage 0x1 (practically speaking, they're going to be zero in this implementation, but good code would still erase the zone it needs to ensure is empty).

The first thing that the programmer should do is set up the syscall vector table. The table is a sequence of 32 addresses, 3 bytes each, corresponding to a syscall number. This is done by putting the address in `r1`, putting `0x21` in `r0`, and executing `syscall`: this executes the pseudo-syscall 33 and populates the syscall table pointer with the low 24 bits of `r1`. Without this, all syscall instructions will be treated as `die`, i.e. halt-and-catch-fire. At the very least, the programmer should populate it with the address of 96 zeroes (aka 32 null pointers, turning syscalls into a warm reset function) or something similar if HCF is not desired, and correspondingly write a recovery procedure at the start of the kernel.

After this, the kernel is free to do whatever. When further components get implemented, it would be good to do motherboard exploration immediately after syscall table population, but there currently is no motherboard at all.
