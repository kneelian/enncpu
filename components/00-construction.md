# Construction

When code that uses the EnnCPU constructs the CPU object, it takes the following steps:

- create CPU object
- - set up internal registers (IP, SP, status register, privilege level, interrupts, memory page numbers etc.)
- - create data registers
- - reserve stash for shadow registers and storing internal state on context switches (duplicates of above)
- create MMU object
- - allocate 
- - create and modify kernel memory page
- - create and modify kernel instructions page
- register MMU in CPU (pointer assignment)
- populate kernel instruction page with kernel code loaded from raw binary file
- (optional) create and populate userland program page, and create userland memory page
- (optional) initialise "motherboard"
- - connect CPU and MMU to motherboard as devices 0xfffe and 0xfffd
- - connect interrupt hub as device 0xffff (-1)
- - register motherboard in CPU (pointer assignment)
- - connect all other devices starting from 0x0001
- boot

## Notes

- the kernel memory, instruction and stack page are all separate pages
- there is no automatic self-modification of kernel code: the MMU initialises the kernel code page as read-only executable, and the kernel stack and memory page as a non-executable RW page
- all pages are 2^24 bits (16 MB) long: the minimum memory footprint is 32 MB + components' state overhead
