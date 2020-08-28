/////////////////////////////////////////////////////////////////////////////
// Name:        algorithm.h
// Project:     baseLib
// Purpose:     Calculators processing STL iterators & datanodes.
// Author:      Piotr Likus
// Modified by:
// Created:     23/05/2011
// Licence:     BSD
/////////////////////////////////////////////////////////////////////////////

#ifndef _BASEALG_H__
#define _BASEALG_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file algorithm.h
\brief Calculators processing STL iterators & datanodes.

List of algorithms:
- copy_if       - copy iterator items if predicate is true
- avg           - calculate average value
- product       - product of series of data
- std_dev       - calculate standard deviation
- make_normal   - normalize vector (using AVG)
- dot_product   - calculate dot_product of 2 vectors
- find_min_max  - find minimum and maximum in the same time
- find_if_reader - version of find_if with reader for accessing values

*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include <algorithm>
#include <numeric>
#include <functional>

namespace base {

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
// STL upgrades - more / better STL functions
// ----------------------------------------------------------------------------

/// Implementation of copy_if based on "Effective STL" book by Scott Meyers
/// Deprecated, use std::copy_if if possible.
template<typename InputIterator,
	typename OutputIterator,
	typename Predicate>
OutputIterator copy_if(InputIterator begin,
		InputIterator end,
		OutputIterator destBegin,
		Predicate p)
{
	while (begin != end) {
		if (p(*begin)) *destBegin++ = *begin;
		++begin;
	}
	return destBegin;
}

/// \brief Product of items
/// \result Result = a[i] * a[i+1] * ... * a[n]
template <class InputIterator, class T>
T product( InputIterator first, InputIterator last, T init )
{
  return std::accumulate(first,
                         last,
                         init,
                         std::multiplies<T>());
}


// ----------------------------------------------------------------------------
// New algorithms
// ----------------------------------------------------------------------------

/// Calculate average value
template <class InputIterator, class T>
T avg ( InputIterator first, InputIterator last, T init )
{
  init = std::accumulate(first, last, init);
  int cnt = std::distance(first, last);
  if (cnt == 0)
    cnt = 1;
  return init / static_cast<T>(cnt);
}

/// Calculate standard deviation
template <class InputIterator, class T>
T std_dev(InputIterator first, InputIterator last, T defValue)
{
  T outValue = defValue;
  T difAvg;

  int cnt = (last - first);

  if (cnt > 0) {
    T avgVal = avg(first, last, 0.0);
    outValue = 0.0;

    for(InputIterator it = first; it != last; ++it) {
      difAvg = *it - avgVal;
      outValue += (difAvg * difAvg);
    }

    if (cnt == 0) cnt = 1;
    outValue = (static_cast<T>(1.0) / static_cast<T>(cnt)) * outValue;
    outValue = sqrt(outValue);
  } else {
    outValue = defValue;
  }

  return outValue;
}

/// Find minimum & maximum at the same time
/// See also boost::minmax_element
template <class T, class StlIterator>
bool find_min_max(StlIterator beginIt, StlIterator endIt, T &minVal, T &maxVal)
{
  bool assigned = false;
  T newVal;

  if (beginIt != endIt)
  {
    minVal = maxVal = *beginIt;
    ++beginIt;
    assigned = true;

    for(; beginIt != endIt; ++beginIt)
    {
      newVal = *beginIt;
      if (newVal < minVal) {
        minVal = newVal;
      } else if (newVal > maxVal) {
        maxVal = newVal;
      }
    } // for
  }

  if (!assigned) {
    minVal = maxVal = T();
  }

  return assigned;
} // function


/// Min-max value normalization
template <class IteratorClassIn, class IteratorClassOut>
void make_normal(IteratorClassIn beginIt, IteratorClassIn endIt, IteratorClassOut output)
{
  typedef typename IteratorClassIn::value_type value_type;
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
      *itOut = (static_cast<value_type>(*itIn) - minVal) / diff;
    }
  } else {
    IteratorClassIn itIn = beginIt;
    IteratorClassOut itOut = output;
    for(; itIn != endIt; ++itIn, ++itOut)
    {
      *itOut = *itIn;
    }
  }
}

/// Dot product for two vectors, Z[i] = X[i] * Y[i]
/// \deprecated Deprecated. Use STL inner_product instead.
template <class IteratorType>
typename IteratorType::value_type
  dot_product(IteratorType beginIt, IteratorType endIt, IteratorType beginIt2)
{
  typedef typename IteratorType::value_type value_type;

  value_type res(0);

  for(; beginIt != endIt; ++beginIt, ++beginIt2)
      res += ((*beginIt) * (*beginIt2));

  return res;
}

/// find_if version using "pos" reader
template<typename ValueType, typename Reader, typename CompareOp>
size_t find_if_reader(size_t aSize, ValueType value, Reader reader, CompareOp compOp)
{
  size_t res = aSize;

  for(size_t i = 0, epos = aSize; i != epos; i++)
    if (compOp(reader(i), value))
    {
      res = i;
      break;
    }

  return res;
}

} // namespace dtp
#endif // _BASEALG_H__
