#include "svdpi.h"
#include <stdint.h>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <cassert>
#include <string>
#include <algorithm>

#ifndef SVDPI_EXTN_H
#define SVDPI_EXTN_H

namespace svDpiExtn {

  struct svBitT;
  struct svLogicT;
  typedef enum {Hex, Dec, Oct, Bin} ValType;

  struct svBitSliceT {
  public:
    svBitVec32 *Value;
    const uint16_t msb, lsb;
    svBitSliceT(svBitVec32 *b, uint16_t l, uint16_t r);

    /* */
    void getValue (svBitVec32 *V) const;

    /* assignment */
    svBitSliceT &operator=(const std::string &rhs);
    svBitSliceT &operator=(const uint64_t &rhs);
    //    svBitSliceT &operator=(const svBitT &rhs);
    //    svBitSliceT &operator=(const svBitSliceT &rhs);

    /* value conversions */
    operator uint64_t();
    std::string str(ValType t=Hex);
  };

  struct svBitT {
  public:
    svBitVec32 *Value;
    const uint16_t uWordSize /* # bits */,
      uArraySize /* #svBitVec32 required for uArraySize */;

    /* Access, references */
    svBitSliceT operator()(uint32_t l_idx, uint32_t r_idx);
    svBitSliceT operator()(uint32_t idx) { return operator()(idx, idx); }
    svBitSliceT operator[](uint32_t idx) { return operator()(idx, idx); }

    /* Type conversions */
    operator uint64_t();
    std::string str(ValType t=Hex);
    operator svBitVec32*();

    /* Assignments */
    svBitT& operator=(const std::string &rhs);
    svBitT& operator=(const uint64_t &rhs);
    svBitT& operator=(const svBitT &rhs);
    svBitT& operator=(const svBitSliceT &rhs);
    //	svBitT& operator=(const svLogicT &sv_logic);

    svBitT(uint32_t S=1);
    ~svBitT();
  };

  struct svBitP {
  public:
    svBitVec32 *Value;

    /* Access, references */
    svBitSliceT operator()(uint32_t l_idx, uint32_t r_idx);
    svBitSliceT operator()(uint32_t idx) { return operator()(idx, idx); }
    svBitSliceT operator[](uint32_t idx) { return operator()(idx, idx); }

    svBitP(svBitVec32 *p) : Value(p) {
    }
  };

#ifdef SVLOGIC
  struct svLogicSliceT {
  public:
    svLogicVec32* Value;
    const uint16_t msb, lsb;
    svLogicSliceT(svLogicVec32* b, uint16_t l, uint16_t r);

    /* */
    void getValue (svLogicVec32 *V) const;

    /* assignment */
    svLogicSliceT &operator=(const std::string &rhs);
    svLogicSliceT &operator=(const uint64_t &rhs);
    svLogicSliceT &operator=(const svLogicT &rhs);
    svLogicSliceT &operator=(const svLogicSliceT &rhs);

    /* value conversions */
    operator uint64_t();
    std::string str(ValType t=Hex);
  };

  struct svLogicT {
  public:
    svLogicVec32 *Value;
    const uint16_t uWordSize /* # bits */,
      uArraySize /* #svLogicVec32 required for uArraySize */;

    /* Access, references */
    svLogicSliceT operator()(uint32_t l_idx, uint32_t r_idx);
    svLogicSliceT operator()(uint32_t idx) { return operator()(idx, idx); }
    svLogicSliceT operator[](uint32_t idx) { return operator()(idx, idx); }

    /* Type conversions */
    operator uint64_t();
    std::string str(ValType t=Hex);
    operator svLogicVec32*();

    /* Assignments */
    svLogicT& operator=(const std::string &rhs);
    svLogicT& operator=(const uint64_t &rhs);
    svLogicT& operator=(const svLogicT &rhs);
    svLogicT& operator=(const svLogicSliceT &rhs);
    //	svLogicT& operator=(const svBitT &sv_bit);

    svLogicT(uint32_t S=1);
    ~svLogicT();

  };
#endif

}

typedef svDpiExtn::svBitT ccBitVector;
typedef svDpiExtn::svBitP svBitVector;
#ifdef SVLOGIC
typedef svDpiExtn::svLogicT ccLogicVector;
typedef svDpiExtn::svLogicP svLogicVector;
#endif

#define FATAL_ERROR printf
#endif

