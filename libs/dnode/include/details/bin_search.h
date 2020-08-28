/////////////////////////////////////////////////////////////////////////////
// Name:        bin_search.h
// Project:     dtpLib
// Purpose:     Binary search algorithm for dnode.
// Author:      Piotr Likus
// Modified by:
// Created:     23/05/2011
/////////////////////////////////////////////////////////////////////////////


#ifndef _DTPBINSRCH_H__
#define _DTPBINSRCH_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file bin_search.h
\brief Binary search algorithm for dnode.

*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------

namespace dtp {


/// Perform binary search using compact compare operator which includes logic for accessing container items
/// so prototype for CompareOpByPos is:
///
/// \code
/// template<typename T>
/// struct MyCompareOpByPos {
///    int operator()(int pos, const T &value); // compares item at a given pos with provided value
/// };
/// \endcode
template<typename ValueType, typename CompareOpByPos>
bool binary_search_by_pos(size_t beginPos, size_t endPos, const ValueType &value, CompareOpByPos compareOp, int &found_pos)
{
  int left = beginPos;
  int right = endPos - 1;
  int cpos;
  bool res = false;
  ValueType cv;
  int cres;

  while (left <= right)
  {
    cpos = left + ((right - left) / 2);

    cres = compareOp(cpos, value);

    if (cres == 0)
    {
      found_pos = cpos;
      res = true;
      break;
    }

    if (left == right)
      break;

    if (cres > 0) {
      if (right != cpos)
        right = cpos;
      else
        right = cpos - 1;
    } else {
      if (left != cpos)
        left = cpos;
      else
        left = cpos + 1;
    }
  } // while

  return res;
}

/// Binary search default compare operator
///
/// Result: \n
/// <0 if val1 < val2 \n
/// =0 if val1 = val2 \n
/// >0 if val1 > val2 \n
template<typename T>
struct dtpSignCompareOp {
  int operator()(T val1, T val2)
  {
    if (val1 < val2)
      return -1;
    else if (val1 > val2)
      return 1;
    else
      return 0;
  }
};



} // namespace dtp
#endif // _DTPBINSRCH_H__
