/////////////////////////////////////////////////////////////////////////////
// Name:        bitstr.h
// Project:     scLib
// Purpose:     Functions handling values encoded in string of bits.
// Author:      Piotr Likus
// Modified by:
// Created:     04/06/2011
/////////////////////////////////////////////////////////////////////////////


#ifndef _SCBITSTR_H__
#define _SCBITSTR_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file bitstr.h
\brief Short file description

Long description
*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include <cassert>

#include "base/btypes.h"
#include "base/string.h"
#include "base/bmath.h"

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Forward class definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
const int MAX_BIT_NO = 63;

// ----------------------------------------------------------------------------
// Class definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Global functions
// ----------------------------------------------------------------------------

/// Returns value with specified bit set to '1'
template<typename T>
T bitNoToValue(int bitNo) {
  static T bits[MAX_BIT_NO+1] = {0};

  if (bits[0] == 0) {
    bits[0] = 1;
    for(int i=1; i < MAX_BIT_NO; i++) {
      bits[i] = static_cast<T>(1) << i;
    }
  }

  assert(bitNo < MAX_BIT_NO);
  return bits[bitNo];
}

/// Returns maximum value a given mask can hold with all bits are = '1'
template<typename T>
T maxValueForBitMask(int bitLen) {

  static T mvals[MAX_BIT_NO+1] = {0};

  if (mvals[0] == 0) {
    mvals[0] = 1;
    for(int i=1; i < MAX_BIT_NO; i++) {
      mvals[i] = (static_cast<T>(1) << (i+1)) - 1;
    }    
  }

  assert(bitLen > 0);
  assert(bitLen - 1 < MAX_BIT_NO);

  return mvals[bitLen - 1];
}

/// Convert string with 0 & 1 to integer value
template<typename T>
T bitStringToInt(const dtpString &value) {
  T res = 0;
  for(int i=0,epos = value.length(); i != epos; i++)
  {
    res = res << 1;
    if (value[i] == '1') 
      res = res | 1;
  }
  return res;
}

/// Convert string with 0 & 1 to floating point value
template<typename T>
T bitStringToFp(const dtpString &value) {
  uint64 maxVal;
  T res;
  int vlen = value.length();

  if (vlen > 1) {
    maxVal = maxValueForBitMask<uint64>(vlen);
    res = static_cast<T>(bitStringToInt<uint64>(value)) / static_cast<T>(maxVal);
  } else {
    res = static_cast<T>(stringToInt(value));;
  }
  return res;
}

/// Convert string with 0 & 1 to integer and then decode using Gray code
template<typename T>
T bitStringToIntGray(const dtpString &value) {
  T res = bitStringToInt<T>(value);

  res = grayToBin(res, value.length());

  return res;
}

/// Convert string with 0 & 1 to floating point using Gray code
template<typename T>
T bitStringToFpGray(const dtpString &value) {
  T res;
  int vlen(value.length());
  if (vlen > 1) {
    uint64 maxVal;
    maxVal = maxValueForBitMask<uint64>(vlen);
    res = static_cast<T>(bitStringToIntGray<uint64>(value)) / static_cast<T>(maxVal);
  } else {
    res = static_cast<T>(stringToInt(value));;
  }
  return res;
}

/// Convert integer to string with 0 & 1
template<typename T>
void intToBitString(T value, uint len, dtpString &output) {
  strPadThis(output, len, ' ');
  
  T runValue(value);

  for(uint i=0,cpos = len - 1; i < len; i++) {
    if ((runValue & 1) != 0) {      
      output.at(cpos) = '1';
    } else {
      output.at(cpos) = '0';
    }    
    cpos--;
    runValue = runValue >> 1;    
  }

}

/// Convert integer to string with 0 & 1 using Gray code
template<typename T>
void intToBitStringGray(T value, uint len, dtpString &output) {
  T runValue = binToGray(value, len);
  intToBitString(runValue, len, output);
}

/// Convert integer to string with 0 & 1 
template<typename T>
dtpString intToBitString(T value, uint len) {
  dtpString res;
  intToBitString<T>(value, len, res);
  return res;
}

/// Convert integer to string with 0 & 1 using Gray code
template<typename T>
dtpString intToBitStringGray(T value, uint len) {
  dtpString res;
  intToBitStringGray<T>(value, len, res);
  return res;
}

/// Convert floating point value to string with 0 & 1 
template<typename T>
void fpToBitString(T value, uint len, dtpString &output) {
  uint64 maxVal, newVal;
  if (len > 1) {
    maxVal = maxValueForBitMask<uint64>(len);
    newVal = round(value * static_cast<T>(maxVal));
    output = intToBitString<uint64>(newVal, len);
  } else {
    output = "";
  }
}

/// Convert floating point value to string with 0 & 1 using Gray code
template<typename T>
void fpToBitStringGray(T value, uint len, dtpString &output) {
  uint64 maxVal, newVal;
  if (len > 1) {
    maxVal = maxValueForBitMask<uint64>(len);
    newVal = round<uint64>(value * static_cast<T>(maxVal));
    output = intToBitStringGray<uint64>(newVal, len);
  } else {
    output = "";
  }
}

/// Convert floating point value to string with 0 & 1 
template<typename T>
dtpString fpToBitString(T value, uint len) {
  dtpString res;
  fpToBitString<T>(value, len, res);
  return res;
}

/// Convert floating point value to string with 0 & 1 using Gray code
template<typename T>
dtpString fpToBitStringGray(T value, uint len) {
  dtpString res;
  fpToBitStringGray<T>(value, len, res);
  return res;
}

#endif // _SCBITSTR_H__
