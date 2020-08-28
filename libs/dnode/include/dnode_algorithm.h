/////////////////////////////////////////////////////////////////////////////
// Name:        dnode_algorithm.h
// Project:     dtpLib
// Purpose:     DataNode-specific versions of algorithms
// Author:      Piotr Likus
// Modified by:
// Created:     23/05/2011
/////////////////////////////////////////////////////////////////////////////

#ifndef _DTPDNALG_H__
#define _DTPDNALG_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file dtp::dnode_algorithm.h
\brief DataNode-specific versions of algorithms

List of algorithms:
- avg
- accumulate
- product
- std_dev
- min_element
- max_element
- count
- count_if
- lower_bound
- upper_bound
- sort
- binary_search
- index_of_value
- find, find_if, find_if_node
- fill, fill_n
- generate, generate_n
- replace, replace_if
- transform
- for_each

TODO:
- partition (use in sort)
- unique, unique-copy
- remove, remove-if, remove-copy, remove-copy-if
- replace-copy, replace-copy-if
- reverse, reverse-copy
- copy, copy-backward
- merge
- set-union, set-difference, set-intersection

See dtp::dnode_cast.h for cast functions.
*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include <algorithm>
#include <iterator>
#include <numeric>

#include "dtp/details/dtypes.h"
#include "dtp/dnode.h"
#include "dtp/dnode_cast.h"
#include "base/algorithm.h"

namespace dtp {

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Forward class definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Class definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Function definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// STL-like functions for scLib data types
// ----------------------------------------------------------------------------
template <class T>
void make_normal(
    dtp::dnode::const_scalar_iterator<T> beginIt, dtp::dnode::const_scalar_iterator<T> endIt,
    dtp::dnode::scalar_iterator<T> output)
{
  typedef dtp::dnode::const_scalar_iterator<T> IteratorClassIn;
  typedef dtp::dnode::scalar_iterator<T> IteratorClassOut;
  typedef T value_type;

  value_type minVal, maxVal, diff;

  find_min_max(
    beginIt,
    endIt,
    minVal,
    maxVal);

  if (minVal != maxVal) {
    IteratorClassIn itIn = beginIt;
    IteratorClassOut itOut = output;
    diff = maxVal - minVal;
    for(; itIn != endIt; ++itIn, ++itOut)
    {
      itOut.set((static_cast<value_type>(*itIn) - minVal) / diff);
    }
  } else {
    IteratorClassIn itIn = beginIt;
    IteratorClassOut itOut = output;
    for(; itIn != endIt; ++itIn, ++itOut)
    {
      itOut.set(*itIn);
    }
  }
}


/// perform function for each element of node
template<typename T, typename InputIterator, typename FuncOp>
FuncOp for_each(InputIterator first, InputIterator last, FuncOp f)
{
  dtp::dnode helper;

  while (first!=last)
  {
    f(first->template getAs<T>(helper));
    ++first;
  }

  return f;
}

/// perform function for each element of node
template<typename InputIterator, typename FuncOp>
FuncOp for_each(InputIterator first, InputIterator last, FuncOp f)
{
  dtp::dnode helper;

  while (first!=last)
  {
    f(first->getAsNode(helper));
    ++first;
  }

  return f;
}

// ----------------------------------------------------------------------------
// index_of_value
// ----------------------------------------------------------------------------
template<typename ValueType>
dtp::dnode::size_type index_of_value(const dtp::dnode &node, ValueType value)
{
  dtp::dnode::const_iterator it = node.template find(value);
  if (it != node.end())
    return it - node.begin();
  else
    return dtp::dnode::npos;
}

// ----------------------------------------------------------------------------
// find
// ----------------------------------------------------------------------------
/* DEPRECATED - use node.find directly

template<typename ValueType>
dtp::dnode::iterator find(dtp::dnode &dnode, const ValueType &value)
{
  return dnode.find(value);
}

template<typename ValueType, typename CompareOp>
dtp::dnode::iterator find_if(dtp::dnode &dnode, const ValueType &value, CompareOp compOp)
{
  return dnode.find_if(value, compOp);
}

template<typename CompareOp>
dtp::dnode::const_iterator find_if_node(dtp::dnode &container, const dtp::dnode &value, CompareOp compOp)
{
  int foundPos;
  int idx = -1;
  dtp::dnode *nodePtr;
  dtp::dnode helper;
  dtp::dnode::const_iterator it;

  for(int i = 0, epos = container.size(); i != epos; i++)
  {
    nodePtr = container.getNodePtr(i, helper);

    if (compOp(*nodePtr, value))
    {
      idx = i;
      break;
    }
  }

  if (idx >= 0)
    it = container.begin() + idx;
  else
    it = container.end();

  return it;
}
*/

} // namespace dtd
#endif //_DTPDNALG_H__
