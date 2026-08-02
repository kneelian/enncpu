#include <bit>
#include <functional>
#include "types.hpp"

/*
	the general syscall interface is as such:

		syscall ID: in the instruction
		parameters 0-3: [ABCD]
*/

bool SYSC_1(CPU* cpu)
{
	/*
		syscall 1 is for interfacing with
		the devices in the device vecto
rr
		the device number is in A
		function id to device in B

		arguments to device are in C and D
	*/

	try
	{
		cpu->DEVICES.at(cpu->ACTIVE_SET.at(0))->SYSC(1, cpu);
	}
	catch(...)
	{
		cpu->ACTIVE_SET.at(0) = 0x00ffffff;
		return false;
	}

	return true;
}

static std::array<std::function<bool(CPU*)>, 32> CALLS = 
{
	SYSC_1,
};

bool CPU::SYSC(u16 NR)
{
	if(NR == 0)
		return false;

	return CALLS[NR](this);
}
