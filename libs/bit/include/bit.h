/////////////////////////////////////////////////////////////////////////////
// Name:        bit.h
// Project:     dtpLib
// Purpose:     bit handling functions
// Author:      Piotr Likus
// Modified by:
// Created:     16/04/2013
/////////////////////////////////////////////////////////////////////////////

#ifndef _BIT_H__
#define _BIT_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file bit.h
\brief bit handling functions

Long description
*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------

// base
#include "base/btypes.h"

// ----------------------------------------------------------------------------
// Template functions
// ----------------------------------------------------------------------------

/// Returns number of bits in a given value = 1
template < class T >
uint countBitOnes(T arg)
{
  uint res = 0;
  T workValue = arg;
  while (workValue != 0) {
    workValue &= (workValue - 1);
    res++;
  }
  return res;
}

/// Returns minimum number of bits used to represent given value
/// 1111 => 4
/// 101 => 3
template < class T >
uint getActiveBitSize(T arg)
{
  uint res = 0;
  T workValue = arg;
  while (workValue != 0) {
    res++;
    workValue = workValue >> 1;
  }
  return res;
}

/// Returns Nth active bit from bit set, n >= 0
/// for f(18, 2) returns 16
template < class T >
T getNthActiveBit(T arg, uint n)
{
  T res = 1;
  uint selectedNo = n + 1;
  while ((selectedNo > 0) && (res > 0)) {
    if ((arg & res) != 0) {
      selectedNo--;
      if (selectedNo == 0)
        break;
    }
    res = res << 1;
  }
  return res;
}

#endif // _BIT_H__
