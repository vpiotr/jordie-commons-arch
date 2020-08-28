/////////////////////////////////////////////////////////////////////////////
// Name:        dnode_cast.cpp
// Project:     scLib
// Purpose:     Conversions between data node and STL containers.
// Author:      Piotr Likus
// Modified by:
// Created:     25/05/2011
/////////////////////////////////////////////////////////////////////////////

#include "dtp/dnode_cast.h"

using namespace dtp;

// ----------------------------------------------------------------------------
// std_vector_cast
// ----------------------------------------------------------------------------
/*
template<>
std::vector<double> &std_vector_cast<double>(dnode &src, dtp::dtpVectorOfDouble& helper)
{
   if (src.isArray() && (src.getArrayR()->getValueType() == vt_double)) {
   // return internal array
     return dynamic_cast<dnArrayOfDouble *>(src.getArray())->getItems();
   } else {
     copyDataNodeToVector(src, helper);
     return helper;
   }
}

template<>
std::vector<double> *std_vector_cast<double>(dnode &src)
{
   if (src.isArray() && (src.getArrayR()->getValueType() == vt_double)) {
   // return internal array
     return &(dynamic_cast<dnArrayOfDouble *>(src.getArray())->getItems());
   } else {
     return NULL;
   }
}

template<>
std::vector<int> &std_vector_cast<int>(dnode &src, dtp::dtpVectorOfInt& helper)
{
   if (src.isArray() && (src.getArrayR()->getValueType() == vt_int)) {
   // return internal array
     return dynamic_cast<dnArrayOfInt *>(src.getArray())->getItems();
   } else {
     copyDataNodeToVector(src, helper);
     return helper;
   }
}

template<>
std::vector<int> *std_vector_cast<int>(dnode &src)
{
   if (src.isArray() && (src.getArrayR()->getValueType() == vt_int)) {
   // return internal array
     return &(dynamic_cast<dnArrayOfInt *>(src.getArray())->getItems());
   } else {
     return NULL;
   }
}
*/
