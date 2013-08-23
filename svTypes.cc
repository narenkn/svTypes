
#include "svTypes.h"

namespace svDpiExtn {

static void
parseVerilogInitialStr(std::string str, uint32_t& width, std::string& result);
static void
setUsingBinaryString(svBitVec32 *V, uint16_t msb, uint16_t lsb, const std::string& binStr );
static void
setUsingBinaryString(svLogicVec32 *V, uint16_t msb, uint16_t lsb, const std::string& binStr );
static void
ctypes_copy_bit_parray (svBitVec32 *to, svBitVec32 *from, uint32_t to_bits, uint32_t from_bits, uint32_t to_offset, uint32_t from_offset);

svBitSliceT::svBitSliceT(svBitT& b, uint16_t l, uint16_t r)
	: B(b), msb(l), lsb(r) {
	/* The following assertions should fail for out of uWordSize,
		wrong usage */
	assert (msb >= lsb);
	assert(b.uWordSize > msb);
	assert(b.uWordSize > lsb);
}

void
svBitSliceT::getValue (svBitVec32 *V) const
{
	uint16_t v_idx;

	/* */
	uint16_t lsb_mod = lsb % (sizeof(svBitVec32)<<3);
	uint16_t msb_mod = msb % (sizeof(svBitVec32)<<3);
	svBitVec32 lsb_shift_mask = ((svBitVec32)-1) >> lsb_mod;
//naren	std::cout << "str()::uWordSize:" << std::dec << B.uWordSize << " lsb:" << lsb << " msb:" << msb
//naren		<< " lsb_mod:" << lsb_mod << " msb_mod:" << msb_mod
//naren		<< " lsb_shift_mask:" << std::hex << lsb_shift_mask
//naren		<< std::endl;
	v_idx = 0;
	for(uint32_t ui1=SV_CANONICAL_SIZE(lsb+1)-1; ui1<SV_CANONICAL_SIZE(msb+1); ui1++, v_idx++) {
		V[v_idx] = (B.Value[ui1] >> lsb_mod) & lsb_shift_mask;
//naren		if (v_idx) {
//naren			std::cout << " V[-1]:" << std::hex << V[v_idx-1] <<
//naren				" Value[ui1]:" << ((B.Value[ui1]<<((sizeof(svBitVec32)<<3)-lsb_mod))&(((svBitVec32)-1)<<((sizeof(svBitVec32)<<3)-lsb_mod))) << std::endl ;
//naren		}
		if ((v_idx) && lsb_mod) {
			V[v_idx-1] |= ((B.Value[ui1]<<((sizeof(svBitVec32)<<3)-lsb_mod))&(((svBitVec32)-1)<<((sizeof(svBitVec32)<<3)-lsb_mod)));
		}
//naren		std::cout << "Debug 2:v_idx:" << v_idx << " ui1:" << ui1 <<
//naren			" lsb_mod:" << lsb_mod << " V[]:" << std::hex << V[v_idx];
//naren		if (v_idx)
//naren			std::cout << " V[-1]:" << std::hex << V[v_idx-1];
//naren		std::cout << " B.Value[]:" << std::hex << B.Value[ui1] << std::endl;
	}

	/* Last word has to be fixed */
	if ((1 == v_idx) && (SV_CANONICAL_SIZE(lsb+1) != SV_CANONICAL_SIZE(msb+1))) {
//naren		std::cout << "  v_idx:" << v_idx << std::endl;
		V[v_idx-1] |= (B.Value[SV_CANONICAL_SIZE(msb+1)]<<((sizeof(svBitVec32)<<3)-lsb_mod)) & (((svBitVec32)-1) << ((sizeof(svBitVec32)<<3)-lsb_mod));
		assert(v_idx);
//		V[v_idx-1] |= (B.Value[SV_CANONICAL_SIZE(msb+1)]<<((sizeof(svBitVec32)<<3)-lsb_mod));
	} //else {
		v_idx = (SV_CANONICAL_SIZE(msb-lsb+1));
		assert(msb>=lsb);
		uint32_t valid_bits = (msb_mod>=lsb_mod) ? msb_mod-lsb_mod+1 : (((sizeof(svBitVec32)<<3)-lsb_mod)+msb_mod+1);
//		uint32_t valid_bits = msb-lsb+1;
		assert(valid_bits <= (sizeof(svBitVec32)<<3));
		svBitVec32 msb_shift_mask = (valid_bits == (sizeof(svBitVec32)<<3)) ? ((svBitVec32)-1) : ~(((svBitVec32)-1) << valid_bits);
		V[v_idx-1] &= msb_shift_mask;
//	}
//naren	std::cout << "Debug 1: msb_shift_mask:" << std::hex << msb_shift_mask << " :";
//naren	for (uint32_t ui1=v_idx; ui1; ui1--) {
//naren		std::cout << V[ui1-1];
//naren	}
//naren	std::cout << std::endl;

}

svBitSliceT::operator uint64_t()
{
	svBitVec32 V[SV_CANONICAL_SIZE((msb-lsb)+1)+1]; /* otherwise things may get corrupt */

#if 0
	ctypes_copy_bit_parray(V, B.Value, 64, msb-lsb+1, 0, lsb);
#else
//	for (uint32_t ui1=0; ui1<B.uArraySize; ui1++)
//		std::cout << "B.Value" << ui1 << ":" << B.Value[ui1] << std::endl;
	getValue(V);
#endif
//naren	for (uint32_t ui1=0; ui1<SV_CANONICAL_SIZE((msb-lsb)+1); ui1++)
//naren		std::cout << "getValue:V" << std::dec << ui1 << ":" << std::hex << V[ui1] << std::endl;

	uint64_t ret_val = 0;

	if (SV_CANONICAL_SIZE((msb-lsb)+1)>1) {
		ret_val = V[1];
		ret_val <<= sizeof(svBitVec32)<<3;
	}
	ret_val |= V[0];

	return ret_val;
}

std::string
svBitSliceT::str(ValType t)
{
	svBitVec32 V[SV_CANONICAL_SIZE((msb-lsb)+1)+1];
	getValue(V);

	std::stringstream ss;

	/* */
	if (t == Bin) {
		ss << std::dec << msb-lsb+1 << "'b";
		for(int32_t ui1=SV_CANONICAL_SIZE(msb-lsb+1)-1; ui1>=0; ui1--)
			for (int32_t ui2=(sizeof(svBitVec32)<<3)-1; ui2>=0; ui2--)
				ss << (V[ui1] & (1<<ui2)) ? "1" : "0";
	} else {
		ss << std::dec << msb-lsb+1 << "'h";
		for(int32_t ui1=SV_CANONICAL_SIZE(msb-lsb+1)-1; ui1>=0; ui1--)
			ss << std::hex << V[ui1];
	}

	return ss.str();
}

svBitSliceT&
svBitSliceT::operator=(const std::string &rhs)
{
	/* initialize myself to 0 */
	for (uint32_t ui1=lsb; msb>=ui1; ui1++) {
		B.Value[SV_CANONICAL_SIZE(ui1+1)-1] &= ~(((svBitVec32)1)<<(ui1%(sizeof(svBitVec32)<<3)));
	}
	if (0 == rhs.length()) return *this;

	uint32_t width;
	std::string result;
	parseVerilogInitialStr(rhs, width, result);
	setUsingBinaryString(B.Value, msb, lsb, result);

	return *this;
}

svBitSliceT&
svBitSliceT::operator=(const uint64_t &rhs)
{
	uint64_t l_rhs = rhs;
	uint16_t l=lsb;

	/* first time */
	for (uint16_t ui1=0; (ui1 < (sizeof(uint64_t)<<3)) && (l<=msb); ui1++, l++, l_rhs>>=1) {
		uint16_t l_mod = l % (sizeof(svBitVec32)<<3);
		B.Value[SV_CANONICAL_SIZE(l+1)-1] &= ~(((svBitVec32)1)<<l_mod);
		B.Value[SV_CANONICAL_SIZE(l+1)-1] |= (l_rhs&1)<<l_mod;
//		printf("l:%d l_mod:%d Value:%x\n", l, l_mod, B.Value[SV_CANONICAL_SIZE(l+1)-1]);
	}

	return *this;
}

svBitSliceT&
svBitSliceT::operator=(const svBitT &rhs)
{
	svBitSliceT rhs_slice( ((svBitT &)rhs)(rhs.uWordSize-1, 0) );
	return operator=(rhs_slice);
}

svBitSliceT&
svBitSliceT::operator=(const svBitSliceT &rhs)
{
	uint16_t copy_size = (((rhs.msb-rhs.lsb)+1) > (msb-lsb+1)) ? (msb-lsb+1) : ((rhs.msb-rhs.lsb)+1);
	uint16_t arr_sz = SV_CANONICAL_SIZE(copy_size)+1;

	/* To store rhs info */
	svBitVec32 *V = new svBitVec32[arr_sz];
	for (uint16_t ui1=0; ui1<arr_sz; ui1++)
		V[ui1] = 0;
	rhs.getValue(V);
//naren	for (uint32_t ui1=0; ui1<arr_sz; ui1++)
//naren		std::cout << "getValue:V" << std::dec << ui1 << ":" << std::hex << V[ui1] << std::endl;

	/* 0 out lhs */
	for (uint16_t ui1=lsb; (ui1<=msb); ui1+=sizeof(uint64_t)<<3) {
		uint16_t ui2 = ((ui1+(sizeof(uint64_t)<<3)-1) > msb) ? msb : (ui1+(sizeof(uint64_t)<<3)-1);
//naren		std::cout << "B()" << std::dec << ui2 << ":" << ui1 << "=" << std::hex << 0 << std::endl;
		B(ui2, ui1) = (uint64_t)0;
	}

	/* Assign only required bits */
	for (uint16_t ui1=lsb, ui3=0; ui1<(lsb+copy_size); ui1+=sizeof(svBitVec32)<<3, ui3++) {
		assert(ui3<arr_sz);
		uint16_t ui2 = ((ui1+(sizeof(svBitVec32)<<3)-1) > msb) ? msb : (ui1+(sizeof(svBitVec32)<<3)-1);
//naren		std::cout << "B()" << std::dec << ui2 << ":" << ui1 << "=" << std::hex << V[ui3] << std::endl;
		B(ui2, ui1) = (uint64_t)V[ui3];
	}

	delete[] V;
	return *this;
}

svBitT::svBitT(uint32_t S)
	: uWordSize(S), uArraySize(SV_CANONICAL_SIZE(S))
{
	assert(S);
	Value = new svBitVec32[uArraySize];
	for (uint32_t ui1=uArraySize; ui1; ui1--) {
		Value[ui1-1] = 0;
	}
}

svBitT::~svBitT()
{
	delete [] Value;
}

svBitT::operator uint64_t()
{
	uint64_t ret = 0;
	if (uArraySize > 1) {
		ret = Value[1];
		ret <<= (sizeof(svBitVec32)<<3);
	}
	ret |= Value[0];
	return ret;
}

#ifdef SVT_CREG
svBitT::operator cReg()
{
	cReg ret(uWordSize-1, 0);

	for (uint32 ui1=0; ui1<uWordSize; ui1+=(sizeof(svBitVec32)<<3)) {
		uint32 ui2 = ((ui1+31)>(uWordSize-1)) ? uWordSize-1 : (ui1+31);
		ret(ui2, ui1) = Value[SV_CANONICAL_SIZE(ui1+1)-1];
	}

	return ret;
}

void
svBitT::copyto(cReg& r)
{
	r.resize(uWordSize-1, 0);

	for (uint32 ui1=0; ui1<uWordSize; ui1+=(sizeof(svBitVec32)<<3)) {
		uint32 ui2 = ((ui1+31)>(uWordSize-1)) ? uWordSize-1 : (ui1+31);
		r(ui2, ui1) = Value[SV_CANONICAL_SIZE(ui1+1)-1];
	}
}
#endif

std::string
svBitT::str(ValType t)
{
	std::stringstream ss;

	if (Bin == t) {
		ss << std::dec << uWordSize << "'b";

		svBitVec32 mask = ((svBitVec32)1)<<(uWordSize%(sizeof(svBitVec32)<<3)-1);
		for (int32_t ui2=uWordSize%(sizeof(svBitVec32)<<3); ui2; ui2++) {
			ss << (Value[0]&mask) ? "1" : "0";
			mask >>= 1;
		}

		/* until the last word */
		for (int32_t ui1=uArraySize-2; ui1>=0; ui1--) {
			svBitVec32 mask = ((svBitVec32)1)<<((sizeof(svBitVec32)<<3)-1);
			for (uint32_t ui2=(sizeof(svBitVec32)<<3); ui2; ui2++) {
				ss << (Value[ui1]&mask) ? "1" : "0";
				mask >>= 1;
			}
		}
		return ss.str();
	}

	/* Printing Hex starts... last word first */
	ss << std::dec << uWordSize << "'h";
	svBitVec32 mask = (svBitVec32)-1;
	mask >>= (sizeof(svBitVec32)<<3) - (uWordSize%(sizeof(svBitVec32)<<3));
	ss << std::hex << std::setw( ((uWordSize%(sizeof(svBitVec32)<<3))+3) >> 2 ) <<
		std::setfill('0') << (Value[uArraySize-1]&mask);
	/* until the last word */
	for (int32_t ui1=uArraySize-2; ui1>=0; ui1--) {
		ss << std::hex << std::setw(sizeof(svBitVec32)<<1) << std::setfill('0') << Value[ui1];
	}

	return ss.str();
}

svBitT&
svBitT::operator=(const std::string &rhs)
{
	/* initialize myself to 0 */
	for (uint32_t ui1=uArraySize; ui1; ui1--) {
		Value[ui1-1] = 0;
	}
	if (0 == rhs.length()) return *this;

	uint32_t width;
	std::string result;
	parseVerilogInitialStr(rhs, width, result);
	setUsingBinaryString(Value, uWordSize-1, 0, result);

	return *this;
}

svBitT&
svBitT::operator=(const svBitT &rhs)
{

	uint32_t min_size, max_size;
	if (uWordSize < rhs.uWordSize)
		min_size = uWordSize, max_size = rhs.uWordSize;
	else
		min_size = rhs.uWordSize, max_size = uWordSize;

	/* Until the last word */
	for (int32_t ui1=SV_CANONICAL_SIZE(min_size)-1; ui1>=0; ui1--) {
		Value[ui1] = rhs.Value[ui1];
	}

	/* Last word */
	svBitVec32 mask = (((svBitVec32)1)<<(min_size%(sizeof(svBitVec32)<<3)))-1;
	Value[SV_CANONICAL_SIZE(min_size)-1] &= ~mask;

	/* Beyond last word */
	for (int32_t ui1=SV_CANONICAL_SIZE(min_size); ui1<uArraySize; ui1++) {
		Value[ui1] = 0;
	}

	return *this;
}


svBitT&
svBitT::operator=(const uint64_t &rhs)
{
	assert(sizeof(uint64_t) == (sizeof(svBitVec32)+sizeof(svBitVec32)));

	/* Assign lower 32 bits and fix upper bits corrupt */
	Value[0] = rhs;
	if (uWordSize < (sizeof(svBitVec32)<<3))
		Value[0] &= (((uint64_t)1)<<uWordSize)-1;

	/* Do the same for uppper 32 bits */
	if (uWordSize > (sizeof(svBitVec32)<<3)) {
		Value[1] = rhs>>(sizeof(svBitVec32)<<3);
		if (uWordSize < (sizeof(svBitVec32)<<3<<1))
			Value[1] &= (((uint64_t)1)<<(uWordSize%(sizeof(svBitVec32)<<3)))-1;
	}

	return *this;
}

svBitSliceT
svBitT::operator()(uint32_t l_idx, uint32_t r_idx)
{
	return svBitSliceT(*this, l_idx, r_idx);
}

svBitT::operator svBitVec32*()
{
	return Value;
}

svBitT&
svBitT::operator=(const svBitSliceT &rhs)
{
	operator()(uWordSize-1,0).operator=(rhs);
	return *this;
}

#ifdef  SVT_CREG
svBitT&
svBitT::operator=(const cReg &reg)
{
	/* initialise with 0 */
	for (uint32_t ui1=uArraySize; ui1; ui1--) {
		Value[ui1-1] = 0;
	}

	/* with value */
	uint32_t msb = (reg.getMsb() < (uWordSize-1)) ? reg.getMsb() : (uWordSize-1);
	for (uint32_t ui1=0; ui1<=msb; ui1+=32) {
		uint32_t ui2 = ((ui1+31)>msb) ? msb : (ui1+31);
		Value[SV_CANONICAL_SIZE(ui1+1)-1] = reg.getInt32(ui2, ui1);
	}
}
#endif

static void
parseVerilogInitialStr( std::string str, uint32_t& width, std::string& result )
{
	// convert to lower-case (makes comparison easy)
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);

	std::ostringstream os;

	ValType radix        = Bin;  // Radix (bin=binary, hex=hexidecimal)
	char   digit         = '0';    // Binary/Hex digit
	bool   headerFlag    = true;   // Parsing flag: the header is <num>'<radix>
	bool   sizeFlag      = false;  // Size flag: the size is the same as bit width
	bool   radixFlag     = false;  // Radix flag
	bool   fourStateFlag = false;  // Four State Flag

	width = 0;

	for ( uint32_t i = 0; i < str.size(); i++ ) {
		digit = str[i];
		// Ignore underscores
		if ( digit == '_' ) {
			continue;
		}
		if ( headerFlag && ! sizeFlag && digit == '\'' ) {
			// Parsing of the size (bit width) of this intializer is complete
			// Convert the character string to an integer and set appropriate flags.
			if ( fourStateFlag ) {
				FATAL_ERROR("cReg::set: Illegal bit width '%s' was specified in: %s", os.str().c_str(), str.c_str());
			}
			if ( ! os.str().empty() ) {
				width = atoi(os.str().c_str());
			}
			sizeFlag = true;
			os.str("");
		} else if ( headerFlag && ! sizeFlag ) {
      // This section is looking for the size (i.e. bit width)
      // If the tick character is never found then the default
      // behavior is to treat the whole string as binary.
      if ( ! isdigit(digit) && digit != 'x' && digit != 'z' ) {
        FATAL_ERROR("cReg::set: Illegal digit '%c' was specified at position %d in: %s", digit, i, str.c_str());
      } else if ( digit == 'x' || digit == 'z' ) {
        fourStateFlag = true;
      }
      os << digit;
    } else if ( headerFlag && sizeFlag && ! radixFlag ) {
			// Parsing of the radix is complete. It should be either
			// b for binary or h for hexidecimal.
			if ( digit == 'b' ) {
				radix = Bin;
			} else if ( digit == 'h' ) {
				radix = Hex;
			} else {
				FATAL_ERROR("cReg::set: Illegal radix '%c' was specified at position %d in : %s", digit, i, str.c_str());
			}
			radixFlag  = true;
			headerFlag = false; // done parsing header
    } else if ( ! headerFlag ) {
			if ( radix == Bin ) {
				// Make sure digit is 0 or 1 bin
				if ( digit == '0' || digit == '1' || digit == 'x' || digit == 'z' ) {
					os << digit;
				} else {
					FATAL_ERROR("cReg::set: Illegal binary digit '%c' was specified at position %d in : %s", digit, i, str.c_str());
				}
			} else {
        // Make sure digit is 0-F hex
        if ( digit == '0' || digit == '1' || digit == '2' || digit == '3' ||
             digit == '4' || digit == '5' || digit == '6' || digit == '7' ||
             digit == '8' || digit == '9' || digit == 'a' || digit == 'b' ||
             digit == 'c' || digit == 'd' || digit == 'e' || digit == 'f' ||
             digit == 'x' || digit == 'z' ) {
          os << digit;
        } else {
          FATAL_ERROR("cReg::set: Illegal hex digit '%c' was specified at position %d in: %s", digit, i, str.c_str());
        }
      }
    }
  }

#ifdef SVT_CREG
  if ( radix == Hex ) {
    result = cReg::hex(os.str());
  } else
#endif
	{
    result = os.str();
  }

  // If width was not specified then the string was not a verilog
  // string but rather a binary string and the width is the size
  // of that binary string.
  if ( width == 0 ) {
    width = result.size();
  }

  if ( width > result.size() ) {
    // Pad with zeros
    std::string paddedZeros(width-result.size(), '0');
    result = paddedZeros + result;
  } else if ( width < result.size() ) {
    // Truncate
    result = result.substr(result.size()-width,result.size()-1);
  }
  width = result.size();
}

static void
setUsingBinaryString(svBitVec32 *V, uint16_t msb, uint16_t lsb, const std::string& binStr )
{
	uint32_t width = binStr.size();

	for ( int32_t strIndex=width-1; (msb >= lsb) && (strIndex >= 0); lsb++, strIndex-- ) {
		V[SV_CANONICAL_SIZE(lsb+1)-1] |= 
			(svBitVec32)((binStr[strIndex]=='1')?sv_1:(
				(binStr[strIndex]=='x')?sv_1:(0)
			)) << (lsb%(sizeof(svBitVec32)<<3));
	}
}

/* It is required to un-touch the bits that doesn't belong to
 * copy... so be carefull
 * to_bits, from_bits, to_offset, from_offset are in # of bits
 * */
void
ctypes_copy_bit_parray (svBitVec32 *to, svBitVec32 *from, uint32_t to_bits, uint32_t from_bits, uint32_t to_offset, uint32_t from_offset)
{
	svBitVec32 *from_t, *out_t;
	uint32_t bit_occupied,num_to_copied,temp;
	int32_t length_t;

	/* check */
	/*INIT*/
	from_t = from;
	out_t = to;

	/* go to that offset .. */
	out_t += (SV_CANONICAL_SIZE(to_offset+1)-1);
	from_t += (SV_CANONICAL_SIZE(to_offset+1)-1);

	num_to_copied = (sizeof(svBitVec32)<<3) - (from_offset % (sizeof(svBitVec32)<<3));
	bit_occupied = to_offset % (sizeof(svBitVec32)<<3);
	
	/*clear the bits at the place to be copied*/
	for(length_t = to_bits;length_t > 0;){
		temp = (typeof(temp))-1;
		temp >>= bit_occupied;
		temp <<= bit_occupied;
		if((length_t + bit_occupied) < (sizeof(svBitVec32)<<3)){
			temp <<= (sizeof(svBitVec32)<<3) -(length_t + bit_occupied);
			temp >>= (sizeof(svBitVec32)<<3) -(length_t + bit_occupied);
			length_t = 0;	
			out_t[0] &=(~ temp);
		}else{
			out_t[0] &=(~ temp);
			length_t -= ((sizeof(svBitVec32)<<3) - bit_occupied);
			bit_occupied = 0;
			out_t++;
		}
	}	
	
	if (to_bits != from_bits) {
		if (to_bits > from_bits)
			to_bits = from_bits;
//naren		std::cout << std::dec << "to_bits:" << to_bits << " from_bits:" << from_bits << std::endl;
	}

	out_t = to + (to_offset >> 5);
	bit_occupied = to_offset % (sizeof(svBitVec32)<<3);
	length_t = to_bits;
	temp = from_t[0];

	/*start coping and cont,...until req*/
	while(length_t > 0){
	
		/*unwanted msb r not to be copied*/
		if((length_t <  num_to_copied) || (!num_to_copied) ){
			if(num_to_copied == 0)
				num_to_copied = (sizeof(svBitVec32)<<3);
			temp = temp & ( ~ (((svBitVec32)-1) << (length_t + ((sizeof(svBitVec32)<<3) - num_to_copied))));
		}
		if(num_to_copied == (sizeof(svBitVec32)<<3))
			num_to_copied = 0;
		
		/*if dest vector is to copied from 0th bit*/
		if(!bit_occupied){
			/*from Xth bit of source to 0th bit of dest*/	
			if(num_to_copied){
				out_t[0] |= ( temp>> ((sizeof(svBitVec32)<<3) - num_to_copied));
				bit_occupied = bit_occupied + num_to_copied;
				length_t -=  num_to_copied;
				from_t++;
				temp = from_t[0];
				num_to_copied = 0;
			}else{
			/*from 0th bit of source to 0th bit of dest*/	
				out_t[0] |= (temp<<bit_occupied);
				if(length_t > (sizeof(svBitVec32)<<3)){
					length_t -= (sizeof(svBitVec32)<<3);
					out_t++;
					from_t++;
					temp = from_t[0];
					num_to_copied = 0;
				}else{
					length_t = 0;
				}
			}
		
		/*if vector of input data is to b copied to Xth bit of dest.. vector*/	
		}else{
			/*from Xth bit of source to xth bit of dest*/	
			if(num_to_copied){
				temp = temp>> ((sizeof(svBitVec32)<<3) - num_to_copied);
				out_t[0] |= (temp<<bit_occupied);
				bit_occupied = bit_occupied + num_to_copied;
					
				length_t -=  num_to_copied;
				num_to_copied = 0;
				if(bit_occupied >= (sizeof(svBitVec32)<<3)){
					num_to_copied = bit_occupied -(sizeof(svBitVec32)<<3); 
					length_t +=  num_to_copied;
					bit_occupied = 0 ;
					out_t++;
				}
					
				temp = from_t[0];
				if(!num_to_copied){
					from_t++;
					temp = from_t[0];
				}
			}else{
			/*from 0th bit of source to Xth bit of dest*/	
				out_t[0] |= (temp<<bit_occupied);
				num_to_copied +=  bit_occupied;	
				bit_occupied = bit_occupied + length_t;
				length_t = 0 ;
				temp = from_t[0];
				if(bit_occupied >= (sizeof(svBitVec32)<<3)){
					length_t = bit_occupied -(sizeof(svBitVec32)<<3); 
					bit_occupied = 0 ;
					out_t++;
				}
			}
		}
	}

}

#ifdef SVLOGIC
svLogicSliceT::svLogicSliceT(svLogicT& b, uint16_t l, uint16_t r)
	: B(b), msb(l), lsb(r) {
	/* The following assertions should fail for out of uWordSize,
		wrong usage */
	assert (msb >= lsb);
	assert(b.uWordSize > msb);
	assert(b.uWordSize > lsb);
}

void
svLogicSliceT::getValue (svLogicVec32 *V) const
{
	uint16_t v_idx;

	/* */
	uint16_t lsb_mod = lsb % (sizeof(svBitVec32)<<3);
	uint16_t msb_mod = msb % (sizeof(svBitVec32)<<3);
	svBitVec32 lsb_shift_mask = ((svBitVec32)-1) >> lsb_mod;
//FIXME	std::cout << "str()::uWordSize:" << std::dec << B.uWordSize << " lsb:" << lsb << " msb:" << msb
//FIXME		<< " lsb_mod:" << lsb_mod << " msb_mod:" << msb_mod
//FIXME		<< " lsb_shift_mask:" << std::hex << lsb_shift_mask
//FIXME		<< std::endl;
	v_idx = 0;
	for(uint32_t ui1=SV_CANONICAL_SIZE(lsb+1)-1; ui1<SV_CANONICAL_SIZE(msb+1); ui1++, v_idx++) {
		V[v_idx].c = (B.Value[ui1].c >> lsb_mod) & lsb_shift_mask;
		V[v_idx].d = (B.Value[ui1].d >> lsb_mod) & lsb_shift_mask;
//FIXME		if (v_idx) {
//FIXME			std::cout << " V[-1]:" << std::hex << V[v_idx-1] <<
//FIXME				" Value[ui1]:" << ((B.Value[ui1]<<((sizeof(svBitVec32)<<3)-lsb_mod))&(((svBitVec32)-1)<<((sizeof(svBitVec32)<<3)-lsb_mod))) << std::endl ;
//FIXME		}
		if ((v_idx) && lsb_mod) {
			V[v_idx-1].c |= ((B.Value[ui1].c<<((sizeof(svBitVec32)<<3)-lsb_mod))&(((svBitVec32)-1)<<((sizeof(svBitVec32)<<3)-lsb_mod)));
			V[v_idx-1].d |= ((B.Value[ui1].d<<((sizeof(svBitVec32)<<3)-lsb_mod))&(((svBitVec32)-1)<<((sizeof(svBitVec32)<<3)-lsb_mod)));
		}
//FIXME		std::cout << "Debug 2:v_idx:" << v_idx << " ui1:" << ui1 <<
//FIXME			" lsb_mod:" << lsb_mod << " V[]:" << std::hex << V[v_idx];
//FIXME		if (v_idx)
//FIXME			std::cout << " V[-1]:" << std::hex << V[v_idx-1];
//FIXME		std::cout << " B.Value[]:" << std::hex << B.Value[ui1] << std::endl;
	}

	/* Last word has to be fixed */
	if ((1 == v_idx) && (SV_CANONICAL_SIZE(lsb+1) != SV_CANONICAL_SIZE(msb+1))) {
//FIXME		std::cout << "  v_idx:" << v_idx << std::endl;
		V[v_idx-1].c |= (B.Value[SV_CANONICAL_SIZE(msb+1)].c<<((sizeof(svBitVec32)<<3)-lsb_mod)) & (((svBitVec32)-1) << ((sizeof(svBitVec32)<<3)-lsb_mod));
		V[v_idx-1].d |= (B.Value[SV_CANONICAL_SIZE(msb+1)].d<<((sizeof(svBitVec32)<<3)-lsb_mod)) & (((svBitVec32)-1) << ((sizeof(svBitVec32)<<3)-lsb_mod));
		assert(v_idx);
	} //else {
		v_idx = (SV_CANONICAL_SIZE(msb-lsb+1));
		assert(msb>=lsb);
		uint32_t valid_bits = (msb_mod>=lsb_mod) ? msb_mod-lsb_mod+1 : (((sizeof(svBitVec32)<<3)-lsb_mod)+msb_mod+1);
//FIXME		uint32_t valid_bits = msb-lsb+1;
		assert(valid_bits <= (sizeof(svBitVec32)<<3));
		svBitVec32 msb_shift_mask = (valid_bits == (sizeof(svBitVec32)<<3)) ? ((svBitVec32)-1) : ~(((svBitVec32)-1) << valid_bits);
		V[v_idx-1].c &= msb_shift_mask;
		V[v_idx-1].d &= msb_shift_mask;
//FIXME	}
//FIXME	std::cout << "Debug 1: msb_shift_mask:" << std::hex << msb_shift_mask << " :";
//FIXME	for (uint32_t ui1=v_idx; ui1; ui1--) {
//FIXME		std::cout << V[ui1-1];
//FIXME	}
//FIXME	std::cout << std::endl;

}

svLogicSliceT::operator uint64_t()
{
	svLogicVec32 V[SV_CANONICAL_SIZE((msb-lsb)+1)+1]; /* otherwise things may get corrupt */

	getValue(V);

	uint64_t ret_val = 0;

	if (SV_CANONICAL_SIZE((msb-lsb)+1)>1) {
		ret_val = V[1].d;
		ret_val <<= sizeof(svBitVec32)<<3;
	}
	ret_val |= V[0].d;

	return ret_val;
}

std::string
svLogicSliceT::str(ValType t)
{
	svLogicVec32 V[SV_CANONICAL_SIZE((msb-lsb)+1)+1];
	getValue(V);

	std::stringstream ss;

	/* */
	if (t == Bin) {
		ss << std::dec << msb-lsb+1 << "'b";
		for(int32_t ui1=SV_CANONICAL_SIZE(msb-lsb+1)-1; ui1>=0; ui1--)
			for (int32_t ui2=(sizeof(svBitVec32)<<3)-1; ui2>=0; ui2--)
				if (V[ui1].c&(1<<ui2))
					ss << (V[ui1].d&(1<<ui2)) ? "x" : "z";
				else
					ss << (V[ui1].d&(1<<ui2)) ? "1" : "0";
	} else {
		ss << std::dec << msb-lsb+1 << "'h" << std::hex;
		for(int32_t ui1=SV_CANONICAL_SIZE(msb-lsb+1)-1; ui1>=0; ui1--)
				if (0 != V[ui1].c)
					for (uint32_t ui3=0; ui3<sizeof(svBitVec32); ui3++)
						ss << "XX";
				else
					ss << V[ui1].d;
	}

	return ss.str();
}

svLogicSliceT&
svLogicSliceT::operator=(const std::string &rhs)
{
	/* initialize myself to 0 */
	for (uint32_t ui1=lsb; msb>=ui1; ui1++) {
		B.Value[SV_CANONICAL_SIZE(ui1+1)-1].c &= ~(((svBitVec32)1)<<(ui1%(sizeof(svBitVec32)<<3)));
		B.Value[SV_CANONICAL_SIZE(ui1+1)-1].d &= ~(((svBitVec32)1)<<(ui1%(sizeof(svBitVec32)<<3)));
	}
	if (0 == rhs.length()) return *this;

	uint32_t width;
	std::string result;
	parseVerilogInitialStr(rhs, width, result);
	setUsingBinaryString(B.Value, msb, lsb, result);

	return *this;
}

svLogicSliceT&
svLogicSliceT::operator=(const uint64_t &rhs)
{
	uint64_t l_rhs = rhs;
	uint16_t l=lsb;

	/* first time */
	for (uint16_t ui1=0; (ui1 < (sizeof(uint64_t)<<3)) && (l<=msb); ui1++, l++, l_rhs>>=1) {
		uint16_t l_mod = l % (sizeof(svBitVec32)<<3);
		B.Value[SV_CANONICAL_SIZE(l+1)-1].d &= ~(((svBitVec32)1)<<l_mod);
		B.Value[SV_CANONICAL_SIZE(l+1)-1].d |= (l_rhs&1)<<l_mod;
		B.Value[SV_CANONICAL_SIZE(l+1)-1].c &= ~(((svBitVec32)1)<<l_mod);
	}

	return *this;
}

svLogicSliceT&
svLogicSliceT::operator=(const svLogicT &rhs)
{
	svLogicSliceT rhs_slice( ((svLogicT &)rhs)(rhs.uWordSize-1, 0) );
	return operator=(rhs_slice);
}

svLogicSliceT&
svLogicSliceT::operator=(const svLogicSliceT &rhs)
{
	uint16_t copy_size = (((rhs.msb-rhs.lsb)+1) > (msb-lsb+1)) ? (msb-lsb+1) : ((rhs.msb-rhs.lsb)+1);
	uint16_t arr_sz = SV_CANONICAL_SIZE(copy_size)+1;

	/* To store rhs info */
	svLogicVec32 *V = new svLogicVec32[arr_sz];
	for (uint16_t ui1=0; ui1<arr_sz; ui1++)
		V[ui1].c = 0, V[ui1].d = 0;
	rhs.getValue(V);

	/* 0 out lhs */
	for (uint16_t ui1=lsb; (ui1<=msb); ui1+=sizeof(uint64_t)<<3) {
		uint16_t ui2 = ((ui1+(sizeof(uint64_t)<<3)-1) > msb) ? msb : (ui1+(sizeof(uint64_t)<<3)-1);
		B(ui2, ui1) = (uint64_t)0;
	}

	/* Assign only required bits */
	for (uint16_t ui1=lsb, ui2=0; ui1<(lsb+copy_size); ui1++, ui2++) {
		uint16_t ui3 = SV_CANONICAL_SIZE(ui1+1)-1;
		uint16_t ui4 = SV_CANONICAL_SIZE(ui2+1)-1;
		assert(ui3<arr_sz);

		svBitVec32 m1 = (svBitVec32)1 << (ui1%(sizeof(svBitVec32)<<3));
		svBitVec32 m2 = (svBitVec32)1 << (ui2%(sizeof(svBitVec32)<<3));

		B[ui3].c &= ~m1;
		B[ui3].d &= ~m1;

		B[ui3].c |= ( (V[ui4].c&m2)>>(ui2%(sizeof(svBitVec32))) ) << (ui1%(sizeof(svBitVec32)));
		B[ui3].d |= ( (V[ui4].d&m2)>>(ui2%(sizeof(svBitVec32))) ) << (ui1%(sizeof(svBitVec32)));
	}

	delete[] V;
	return *this;
}

svLogicT::svLogicT(uint32_t S)
	: uWordSize(S), uArraySize(SV_CANONICAL_SIZE(S))
{
	assert(S);
	Value = new svLogicVec32[uArraySize];
	for (uint32_t ui1=uArraySize; ui1; ui1--) {
		Value[ui1-1].c = 0;
		Value[ui1-1].d = 0;
	}
}

svLogicT::~svLogicT()
{
	delete [] Value;
}

svLogicT::operator uint64_t()
{
	uint64_t ret = 0;
	if (uArraySize > 1) {
		ret = Value[1].d;
		ret <<= (sizeof(svLogicVec32::d)<<3);
	}
	ret |= Value[0].d;
	return ret;
}

std::string
svLogicT::str(ValType t)
{
	std::stringstream ss;

	if (Bin == t) {
		ss << std::dec << uWordSize << "'b";

		svBitVec32 mask = ((svBitVec32)1)<<(uWordSize%(sizeof(svBitVec32)<<3)-1);
		for (int32_t ui2=uWordSize%(sizeof(svBitVec32)<<3); ui2; ui2++) {
			if (Value[0].c&mask)
				ss << (Value[ui1].d&mask) ? "x" : "z";
			else
				ss << (Value[ui1].d&mask) ? "1" : "0";
			mask >>= 1;
		}

		/* until the last word */
		for (int32_t ui1=uArraySize-2; ui1>=0; ui1--) {
			svBitVec32 mask = ((svBitVec32)1)<<((sizeof(svBitVec32)<<3)-1);
			for (uint32_t ui2=(sizeof(svBitVec32)<<3); ui2; ui2++) {
				if (Value[0].c&mask)
					ss << (Value[ui1].d&mask) ? "x" : "z";
				else
					ss << (Value[ui1].d&mask) ? "1" : "0";
				mask >>= 1;
			}
		}
		return ss.str();
	}

	/* Printing Hex starts... last word first */
	ss << std::dec << uWordSize << "'h";
	svBitVec32 mask = (svBitVec32)-1;
	mask >>= (sizeof(svBitVec32)<<3) - (uWordSize%(sizeof(svBitVec32)<<3));
	if (!(Value[uArraySize-1].c&mask)) {
		ss << std::hex << std::setw( ((uWordSize%(sizeof(svBitVec32)<<3))+3) >> 2 ) <<
			std::setfill('0') << (Value[uArraySize-1].d&mask);
	} else {
		for (uint32_t ui1=sizeof(svBitVec32); ui1; ui1--)
			ss << "XX";
	}
	/* until the last word */
	for (int32_t ui1=uArraySize-2; ui1>=0; ui1--) {
		if (!(Value[ui1].c)) {
			ss << std::hex << std::setw(sizeof(svBitVec32)<<1) << std::setfill('0') << Value[ui1].d;
		} else {
			for (uint32_t ui1=sizeof(svBitVec32); ui1; ui1--)
				ss << "XX";
		}
	}

	return ss.str();
}

svLogicT&
svLogicT::operator=(const std::string &rhs)
{
	/* initialize myself to 0 */
	for (uint32_t ui1=uArraySize; ui1; ui1--) {
		Value[ui1-1].c = 0;
		Value[ui1-1].d = 0;
	}
	if (0 == rhs.length()) return *this;

	uint32_t width;
	std::string result;
	parseVerilogInitialStr(rhs, width, result);
	setUsingBinaryString(Value, uWordSize-1, 0, result);

	return *this;
}

svLogicT&
svLogicT::operator=(const svLogicT &rhs)
{

	uint32_t min_size, max_size;
	if (uWordSize < rhs.uWordSize)
		min_size = uWordSize, max_size = rhs.uWordSize;
	else
		min_size = rhs.uWordSize, max_size = uWordSize;

	/* Until the last word */
	for (int32_t ui1=SV_CANONICAL_SIZE(min_size)-2; ui1>=0; ui1--) {
		Value[ui1] = rhs.Value[ui1];
	}

	/* Last word */
	svBitVec32 mask = (svBitVec32)-1;
	mask >>= min_size%(sizeof(svBitVec32)<<3);
	mask <<= min_size%(sizeof(svBitVec32)<<3);
	Value[SV_CANONICAL_SIZE(min_size)-1].c = rhs.Value[SV_CANONICAL_SIZE(min_size)-1].c & ~mask;
	Value[SV_CANONICAL_SIZE(min_size)-1].d = rhs.Value[SV_CANONICAL_SIZE(min_size)-1].d & ~mask;

	/* Beyond last word */
	for (int32_t ui1=SV_CANONICAL_SIZE(min_size); ui1<uArraySize; ui1++) {
		Value[ui1].c = 0;
		Value[ui1].d = 0;
	}

	return *this;
}


svLogicT&
svLogicT::operator=(const uint64_t &rhs)
{
	assert(sizeof(uint64_t) == sizeof(svLogicVec32));

	/* Assign lower 32 bits and fix upper bits corrupt */
	Value[0].c = 0;
	Value[0].d = rhs;
	if (uWordSize < (sizeof(svBitVec32)<<3))
		Value[0].d &= (((uint64_t)1)<<uWordSize)-1;

	/* Do the same for uppper 32 bits */
	if (uWordSize > (sizeof(svBitVec32)<<3)) {
		Value[1].c = 0;
		Value[1].d = rhs>>(sizeof(svBitVec32)<<3);
		if (uWordSize < (sizeof(svBitVec32)<<3<<1))
			Value[1].d &= (((uint64_t)1)<<(uWordSize%(sizeof(svBitVec32)<<3)))-1;
	}

	return *this;
}

svLogicSliceT
svLogicT::operator()(uint32_t l_idx, uint32_t r_idx)
{
	return svLogicSliceT(*this, l_idx, r_idx);
}

svLogicT::operator svLogicVec32*()
{
	return Value;
}

svLogicT&
svLogicT::operator=(const svLogicSliceT &rhs)
{
	operator()(uWordSize-1,0).operator=(rhs);
	return *this;
}

#ifdef SVT_CREG
svLogicT&
svLogicT::operator=(const cReg &reg)
{
	/* initialise with 0 */
	for (uint32_t ui1=uArraySize; ui1; ui1--) {
		Value[ui1-1] = 0;
	}

	/* with value */
	uint32_t msb = (reg.getMsb() < (uWordSize-1)) ? reg.getMsb() : (uWordSize-1);
	for (uint32_t ui1=0; ui1<=msb; ui1+=32) {
		uint32_t ui2 = ((ui1+31)>msb) ? msb : (ui1+31);
		Value[SV_CANONICAL_SIZE(ui1+1)-1].c = reg.getInt32(ui2, ui1, cReg::CONTROL);
		Value[SV_CANONICAL_SIZE(ui1+1)-1].d = reg.getInt32(ui2, ui1);
	}
}

svLogicT::operator cReg()
{
	cReg ret(uWordSize-1, 0);

	for (uint32 ui1=0; ui1<uWordSize; ui1++) {
		uint32 ui2 = SV_CANONICAL_SIZE(ui1+1)-1;
		uint32 mod_ui1 = ui1%(sizeof(svBitVec32)<<3);
		svBitVec32 mask = 1 << mod_ui1;
		ret.set(ui1, (cReg::FourState)( ((Value[ui2].c&mask)>>mod_ui1<<1) | ((Value[ui2].d&mask)>>mod_ui1) ));
	}

	return ret;
}

void
svLogicT::copyto(cReg& r)
{
	r.resize(uWordSize-1, 0);

	for (uint32 ui1=0; ui1<uWordSize; ui1++) {
		uint32 ui2 = SV_CANONICAL_SIZE(ui1+1)-1;
		uint32 mod_ui1 = ui1%(sizeof(svBitVec32)<<3);
		svBitVec32 mask = 1 << mod_ui1;
		r.set(ui1, (cReg::FourState)( ((Value[ui2].c&mask)>>mod_ui1<<1) | ((Value[ui2].d&mask)>>mod_ui1) ));
	}
}
#endif

static void
setUsingBinaryString(svLogicVec32 *V, uint16_t msb, uint16_t lsb, const std::string& binStr )
{
	uint32_t width = binStr.size();

	for ( int32_t strIndex=width-1; (msb >= lsb) && (strIndex >= 0); lsb++, strIndex-- ) {
		V[SV_CANONICAL_SIZE(lsb+1)-1].c |= 
			(svBitVec32)((binStr[strIndex]=='z')?sv_1:(
				(binStr[strIndex]=='x')?sv_1:(0)
			)) << (lsb%(sizeof(svBitVec32)<<3));
		V[SV_CANONICAL_SIZE(lsb+1)-1].d |= 
			(svBitVec32)((binStr[strIndex]=='1')?sv_1:(
				(binStr[strIndex]=='x')?sv_1:(0)
			)) << (lsb%(sizeof(svBitVec32)<<3));
	}
}
#endif

}


