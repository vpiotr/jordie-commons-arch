/////////////////////////////////////////////////////////////////////////////
// Name:        utils.h
// Project:     dtpLib
// Purpose:     Utility, general-purpose functions
// Author:      Piotr Likus
// Modified by:
// Created:     04/10/2008
/////////////////////////////////////////////////////////////////////////////

#ifndef _DTPUTILS_H__
#define _DTPUTILS_H__

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
//std

//base
#include "base/string.h"

//sc
#include "dtp/details/defs.h"

// symbols required because Boost includes WinDefs.h with min/max incompatible with std versions
#define DTP_MIN(a,b) (std::min)(a,b)
#define DTP_MAX(a,b) (std::max)(a,b)

// errors

#define runtime_check(condition, msg) { if (!(condition)) throw_runtime_error(msg); }

namespace dtp {

namespace Details {

template<typename MsgType>
inline void throw_runtime_error(MsgType msg)
{
  throw std::runtime_error(dtpString("Check failed: [")+dtpString(msg)+"]");
}

}; // namespace details

/*
template<typename R, typename T>
inline R round(T value) {
  T val1;

  if (value < 0.0)
    val1 = value - static_cast<T>(0.5);
  else
    val1 = value + static_cast<T>(0.5);

  R intPart = static_cast<R>(val1);
  return intPart;
}
*/


template<typename T>
void dtp_swap(T &v1, T &v2)
{
  T temp(v1);
  v1 = v2;
  v2 = temp;
}

}; // namespace dtp

//template<typename T>
//void move(T &dest, const T &src)
//{
//  dest = src;
//}


#endif // _DTPUTILS_H__
