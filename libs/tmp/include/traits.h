/////////////////////////////////////////////////////////////////////////////
// Name:        traits.h
// Project:     dtpLib
// Purpose:     Trait support classes & methods
// Author:      Piotr Likus
// Modified by:
// Created:     03/11/2012
// Licence:     BSD
/////////////////////////////////////////////////////////////////////////////


#ifndef _DTPTRAITS_H__
#define _DTPTRAITS_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file traits.h
\brief Trait support classes & methods

*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------

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
// dtpSelector
// ----------------------------------------------------------------------------
// used to tag functions
//
//    template<typename ValueType, bool Value = (dnValueMeta<ValueType>::is_object == 0)>
//    struct dnValueMetaCanCompareDir: public dtpSelector<Value>
//    {};
//
// function declaration (version for "false"):
//
//    template<typename ValueType>
//    int compareImpl(const dnValueStorage &storage1, const dnValueStorage &storage2, dtpSelector<false> canCompareDirectly) { }
//
// usage:
//
//    return compareImpl<target_type>(storage1, storage2, dnValueMetaCanCompareDir<target_type>());
//

template<bool Parameter>
struct dtpSelector {
  static const bool value = Parameter;
};

// ----------------------------------------------------------------------------
// dtpEnableIf
// ----------------------------------------------------------------------------
/// used to select function using return type:
///
/// \code
///    template<typename ValueType>
///      typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
///        setAs(ValueType newValue)
///    {
///    }
/// \endcode
///
/// or dummy argument:
///
/// \code
///    template<typename ValueType>
///      dnode(const ValueType &value, typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, ValueType>::type* = 0)
///    {}
/// \endcode

template<bool Condition, class T>
struct dtpEnableIfType {
  typedef T type;
};

template<class T>
struct dtpEnableIfType<false, T> {
};

template<class Condition, class T>
struct dtpEnableIf: public dtpEnableIfType<Condition::value, T> {
};

template<class Condition, class T>
struct dtpDisableIf: public dtpEnableIfType<(!Condition::value), T> {
};

template <typename T, typename U>
struct dtpIsSameType: public dtpSelector<false>
{
};

template <typename T>
struct dtpIsSameType<T, T>: public dtpSelector<true>
{
};

} // namespace dtp

#include "base/static_assert.h"

#endif // _DTPTRAITS_H__
