
#include <iostream>
#include "svTypes.hh"

extern "C"
void
t1(svBitVec32* b)
{
  svBitVector pb(b);
  std::cout << "b:" << (uint64_t) pb(63,0) << std::endl;
}

