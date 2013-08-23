
#include <iostream>
#include "svTypes.h"

extern "C"
void
t1(pBit b)
{
	std::cout << "b:" << (uint64_t) b(63,0) << std::endl;
}

