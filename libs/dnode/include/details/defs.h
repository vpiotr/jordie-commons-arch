/////////////////////////////////////////////////////////////////////////////
// Name:        defs.h
// Project:     dtpLib
// Purpose:     Macro definitions + configuration
// Author:      Piotr Likus
// Modified by:
// Created:     06/03/2009
/////////////////////////////////////////////////////////////////////////////
#ifndef _DTPDEFS_H__
#define _DTPDEFS_H__

// detect compiler
#if defined(__GNUC__) || defined(__GNUG__)
#define DTP_COMP_GCC
#elif defined (_MSC_VER)
#define DTP_COMP_VS
#endif

//--------------------------------------------------------------------
// compiler warnings: pragmas
//--------------------------------------------------------------------
#ifdef DTP_COMP_VS
#pragma warning( disable : 4512 )
#pragma warning( disable : 4100 )
#pragma warning( disable : 4103 ) // (boost) alignment changed after including header, may be due to missing #pragma pack(pop)
#pragma warning( 4 : 4265 ) // virtual destructor
#pragma warning( 4 : 4623 ) // default constructor could not be generated because a base class default constructor is inaccessible
#pragma warning( 4 : 4242 ) // possible loss of data on char = int
#pragma warning( 4 : 4254 ) // possible loss of data on char = int
#pragma warning( 4 : 4253 ) // inherited class do not override base class methods
#pragma warning( 4 : 4296 ) // always true or false
#pragma warning( 4 : 4546 ) // function call before comma missing argument list
#pragma warning( 4 : 4547 ) // operator before comma has no effect; expected operator with side-effect
//#pragma warning( 4 : 4548 ) // expression before comma has no effect; expected expression with side-effect
#pragma warning( 4 : 4549 ) // operator before comma has no effect; did you intend 'operator'?
#pragma warning( 4 : 4555 ) // expression has no effect; expected expression with side-effect
//#pragma warning( 4 : 4625 ) // copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning( 4 : 4640 ) // construction of local static object is not thread-safe
//#pragma warning( 4 : 4668 ) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
//#pragma warning( 4 : 4710 ) // function not inlined
#pragma warning( 4 : 4793 ) // ASM: native code generated for function 'function': 'reason'
#pragma warning( 4 : 4946 ) // reinterpret_cast used between related classes: 'class1' and 'class2'

// to test funct inlined
//#pragma warning( push )
//#pragma warning( 4 : 4710 )
//.. code..
//#pragma warning( pop )
#endif

//--------------------------------------------------------------------
//----- user options
//--------------------------------------------------------------------
//#define DTP_CPP11

//--------------------------------------------------------------------
//----- calculated symbols
//--------------------------------------------------------------------

#if defined(_DEBUG) 
#define DTP_DEBUG
#endif

#define DTP_DEPRECATED __declspec(deprecated)

// compiler features

#if defined(DTP_CPP11) || defined(DTP_COMP_VS)
// use std::unique_ptr
#define DTP_UNIQUE_PTR_STD
#endif

#endif //_DTPDEFS_H__
