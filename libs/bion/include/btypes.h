/////////////////////////////////////////////////////////////////////////////
// Name:        btypes.h
// Project:     baseLib
// Purpose:     Fundamental data types
// Author:      Piotr Likus
// Modified by:
// Created:     25/04/2012
/////////////////////////////////////////////////////////////////////////////

#ifndef _BASBTYPES_H__
#define _BASBTYPES_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file btypes.h
\brief Fundamental data types

 - new scalar data types:
   - uint
   - byte
   - int64 / uint64
   - xdouble - long double
   - fdatetime_t - time with fraction of day
*/

// ----------------------------------------------------------------------------
// Configuration
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------
typedef unsigned int uint;

#if !defined(DTP_DISABLE_BYTE)
typedef unsigned char byte;
#endif

#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef unsigned __int64 uint64;
typedef signed __int64 int64;
#else
typedef unsigned long long uint64;
typedef signed long long int64;
#endif

typedef long double xdouble;
typedef double fdatetime_t;  // days with time as fraction of day

#endif // _BASBTYPES_H__
