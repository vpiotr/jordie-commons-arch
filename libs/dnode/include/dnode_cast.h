/////////////////////////////////////////////////////////////////////////////
// Name:        dnode_cast.h
// Project:     dtpLib
// Purpose:     Conversions between data node and STL containers.
// Author:      Piotr Likus
// Modified by:
// Created:     25/05/2011
/////////////////////////////////////////////////////////////////////////////


#ifndef _DTPDNODECAST_H__
#define _DTPDNODECAST_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file dnode_cast.h
\brief Conversions between data node and STL containers.

\deprecated Note: iterator casting is deprecated. Use dnode's scalar_iterator functions instead.

Conversion from data node to std::vector with optional copying, for super-fast access:
  // std_vector_cast
  std::vector<int> helperVector;
  std::vector<int> &vintref = std_vector_cast(dtaNode, helperVector);

*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
//std
#include <vector>
#include <map>
#include <set>
#include <list>

// base
#include "base/btypes.h"

//sc
#include "dtp/details/dnode_arrays.h"
#include "dtp/dnode.h"
#include "dtp/traits.h"

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
// Vector accessors.
// ----------------------------------------------------------------------------

/// Copy data from data node to std::set
template<class T>
std::set<T> &set_cast(dtp::dnode &src, std::set<T> &output)
{
   output.clear();
   dnode::scalar_iterator<T> vbegin = src.scalarBegin<T>();
   dnode::scalar_iterator<T> vend = src.scalarEnd<T>();

   for(; vbegin != vend; ++vbegin)
     output.insert(*vbegin);
   return output;
}

/// Copy data from data node to std::list
template<class T>
std::list<T> &list_cast(dtp::dnode &src, std::list<T> &output)
{
   output.clear();
   dnode::scalar_iterator<T> vbegin = src.scalarBegin<T>();
   dnode::scalar_iterator<T> vend = src.scalarEnd<T>();
   for(; vbegin != vend; ++vbegin)
     output.insert(*vbegin);
   return output;
}

/// Copy data from data node to std::map
template<class T1, class T2>
std::map<T1, T2> &map_cast(dtp::dnode &keys, dtp::dnode &values, std::map<T1, T2> &output)
{
   dnode::scalar_iterator<T1> itk(keys.scalarBegin<T1>());
   dnode::scalar_iterator<T1> kend(keys.scalarEnd<T1>());
   dnode::scalar_iterator<T2> itv(values.scalarBegin<T2>());
   dnode::scalar_iterator<T2> vend(values.scalarEnd<T2>());

   for(; itk != kend; ++itk, ++itv)
   {
     output.insert(make_pair(*itk, *itv));
   }

   return output;
}

/// Copy data from data node to std::map
template<class T>
std::map<dtpString, T> &map_cast(dtp::dnode &keysWithValues, std::map<dtpString, T> &output)
{
   for(dtp::dnode::iterator it = keysWithValues.begin(), epos = keysWithValues.end(); it != epos; ++it)
   {
     output.insert(make_pair(it->getName(), it->getAs<T>(it)));
   }
   return output;
}

/// Copy data from data node to std::multimap
template<class T1, class T2>
std::multimap<T1, T2> &multimap_cast(dtp::dnode &keys, dtp::dnode &values, std::multimap<T1, T2> &output)
{
   dnode::scalar_iterator<T1> itk(keys.scalarBegin<T1>());
   dnode::scalar_iterator<T1> kend(keys.scalarEnd<T1>());
   dnode::scalar_iterator<T2> itv(values.scalarBegin<T2>());
   dnode::scalar_iterator<T2> vend(values.scalarEnd<T2>());

   for(; itk != kend; ++itk, ++itv)
   {
     output.insert(make_pair(*itk, *itv));
   }

   return output;
}

/// Copy data from data node to std::multimap
template<class T>
std::multimap<dtpString, T> &multimap_cast(dtp::dnode &keysWithValues, std::multimap<dtpString, T> &output)
{
   for(dtp::dnode::iterator it = keysWithValues.begin(), epos = keysWithValues.end(); it != epos; ++it)
   {
     output.insert(make_pair(it->getName(), it->getAs<T>(it)));
   }
   return output;
}

/// This function performs casting / conversion from data node to STL vector
/// When data node is an array with correct type, it's internal vector will be returned.
/// Otherwise provided helper will be filled-in by data from source data node - values will be copied.
template<class T>
std::vector<T> &std_vector_cast(dtp::dnode &src, std::vector<T>& helper)
{
   using namespace Details;

   typedef dnArrayImplMeta<dnArrayMetaIsDefined<T>::value, T>  array_impl_meta;
   typedef typename array_impl_meta::implementation_type impl_type;

   if (src.isArray() && (src.getArrayR()->getValueType() == static_cast<dnValueType>(array_impl_meta::direct_item_type))) {
   // return internal array
     return dynamic_cast<impl_type *>(src.getArray())->getItems();
   } else {
     copyDataNodeToVector(src, helper);
     return helper;
   }
}

// Version of std_vector_cast which returns NULL if cast cannot be performed directly
template<class T>
std::vector<T> *std_vector_cast(dtp::dnode &src)
{
   typedef Details::dnArrayImplMeta<Details::dnArrayMetaIsDefined<T>::value, T>  array_impl_meta;
   typedef typename array_impl_meta::implementation_type impl_type;

   if (src.isArray() && (src.getArrayR()->getValueType() == array_impl_meta::direct_item_type)) {
   // return internal array
     return &(dynamic_cast<impl_type *>(src.getArray())->getItems());
   } else {
     return DTP_NULL;
   }
}

// ----------------------------------------------------------------------------
// copyDataNodeToVector
// ----------------------------------------------------------------------------
/// Copy data node to vector
template<class T>
std::vector<T> &copyDataNodeToVector(dtp::dnode &src, std::vector<T>& output)
{
   // copy values
   dtp::dnode::scalar_iterator<T> vbegin = src.scalarBegin<T>();
   dtp::dnode::scalar_iterator<T> vend = src.scalarEnd<T>();
   int i = 0;
   output.resize(src.size());
   for(dtp::dnode::scalar_iterator<T> it = vbegin; it != vend; ++it)
   {
     output[i] = *it;
     i++;
   }
   return output;
}


} // namespace dtp

#endif // _DTPDNODECAST_H__
