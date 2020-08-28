/////////////////////////////////////////////////////////////////////////////
// Name:        algorithm.h
// Project:     dtpLib
// Purpose:     Calculators processing STL iterators & datanodes.
// Author:      Piotr Likus
// Modified by:
// Created:     23/05/2011
// Licence:     BSD
/////////////////////////////////////////////////////////////////////////////


#ifndef _DTPSORT_H__
#define _DTPSORT_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file sort.h
\brief Sort algorithm for dnode.
*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
//#include <algorithm>
//#include <numeric>
//#include <functional>

namespace dtp {



namespace Details {

/// QuickSort
/// Uses sort tool object with the following interface:
///
/// \code
///template<typename ValueType>
///struct SortToolIntf
///{
///   // return value at pos
///   ValueType get(size_t pos)
///   { }
///
///   // swap values at given two positions
///   void swap(size_t pos1, size_t pos2)
///   { }
///
///   // Compare values:
///   // - value 1: stored in container at position "pos" &
///   // - value 2: "value"
///   // @returns:
///   // <0: if value1 <  value2
///   //  0: if value1 == value2
///   // >0: if value1 >  value2
///   int compare(size_t pos, const ValueType &value)
///   {}
///};
/// \endcode
template<typename SortTool>
class dtpQuickSorter {
public:
   dtpQuickSorter(size_t aSize, SortTool sortTool):
     m_size(aSize), m_sortTool(sortTool)
   {
   }

   void execute(int startIndex, int endIndex)
   {
      if ((startIndex < 0) || (endIndex >= m_size) || (endIndex <= startIndex))
        return;

      int pivotIndex = (startIndex + endIndex) / 2;
      int left = startIndex;
      int right = endIndex;

      typename SortTool::value_type value = m_sortTool.get(pivotIndex);

      { // partition

        while (left <= right) {
          if (m_sortTool.compare(left, value) < 0) {
            ++left;
            continue;
          }
          if (m_sortTool.compare(right, value) > 0) {
            --right;
            continue;
          }
          if (left <= right) {
            if (left != right)
                m_sortTool.swap(left, right);
            left++;
            right--;
          }
        } // while
      }

      if (startIndex < right)
        execute(startIndex, right);

      if (left < endIndex)
        execute(left, endIndex);
   }

private:
   int m_size;
   SortTool m_sortTool;
};

} // namespace Details

/// Perform sort using "sort tool" - object with internal reference to container
/// @param[in] aSize Number of items to be sorted
/// @param[in] sortTool object with interface: { value_type, int compare(pos, value); T get(pos); swap(pos1, pos2) }
template<class SortTool>
void sort(size_t aSize, SortTool sortTool)
{
  Details::dtpQuickSorter<SortTool> sorter(aSize, sortTool);
  if (aSize > 1)
    sorter.execute(0, aSize - 1);
}

} // namespace dtp
#endif // _DTPSORT_H__
