#include "random_module.hpp"

u32 hash_a(u64 rand_state)
{
	u32 x = u32(rand_state ^ (rand_state >> 32)) + 1;
	x ^= x >> 17; x *= 0xed5ad4bb;
	x ^= x >> 11; x *= 0xac4c1b51;
	x ^= x >> 15; x *= 0x31848bab;
	x ^= x >> 14;
	return x;
}
