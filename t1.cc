
#include <iostream>
#include "svTypes.hh"

extern "C"
void
t1(svBitVec32* b)
{
  svBitVector pb(b);
  std::cout << "b:" << pb(63,0).str() << std::endl;
  pb(63,0) = 0xaaaabbbbccccdddd;
  std::cout << "b:" << pb(63,0).str() << std::endl;
}

