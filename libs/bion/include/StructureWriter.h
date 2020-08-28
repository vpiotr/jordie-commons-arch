/////////////////////////////////////////////////////////////////////////////
// Name:        StructureWriter.h
// Project:     dtpLib
// Purpose:     On-fly serializer for data structures.
//              Can be used with any JSON-compatible output format (JSON, BION, XML, etc).
// Author:      Piotr Likus
// Modified by:
// Created:     21/10/2013
/////////////////////////////////////////////////////////////////////////////

#ifndef _BASESTRUCTWRITER_H__
#define _BASESTRUCTWRITER_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file StructureWriter.h
///
/// On-fly serializer for data structures.
/// Can be used with any JSON-compatible output format (JSON, BION, XML, etc).

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
// base
#include "base/string.h"

namespace base
{

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
 class StructureOutputIntf;

 class StructureWriterIntf {
public: 
   virtual ~StructureWriterIntf() {}
   virtual void visit(StructureOutputIntf &output) = 0;
 };

 class StructureOutputIntf {
public: 
   // construction
   virtual ~StructureOutputIntf() {}
   // execution
   virtual void accept(StructureWriterIntf &writer) = 0;
   // visitation
   virtual void beginWrite() = 0;
   virtual void endWrite() = 0;
   virtual void beginMap() = 0;
   virtual void endMap() = 0;
   virtual void beginArray(size_t size = 0) = 0;
   virtual void beginArrayOf(const std::string &valueTag, size_t size = 0) = 0;
   virtual void beginArrayOf(float valueTag, size_t size = 0) = 0;
   virtual void beginArrayOf(double valueTag, size_t size = 0) = 0;
   virtual void beginArrayOf(xdouble valueTag, size_t size = 0) = 0;
   virtual void beginArrayOf(bool valueTag, size_t size = 0) = 0;
   virtual void beginArrayOf(byte valueTag, size_t size = 0) = 0;
   virtual void beginArrayOf(int valueTag, size_t size = 0) = 0;
   virtual void beginArrayOf(uint valueTag, size_t size = 0) = 0;
   virtual void beginArrayOf(int64 valueTag, size_t size = 0) = 0;
   virtual void beginArrayOf(uint64 valueTag, size_t size = 0) = 0;
   virtual void endArray() = 0;
   //----
   virtual void writeKeyName(const std::string &name) = 0;
   //----
   virtual void writeValue(const std::string &value) = 0;
   virtual void writeValue(float value) = 0;
   virtual void writeValue(double value) = 0;
   virtual void writeValue(xdouble value) = 0;
   virtual void writeValue(byte value) = 0;
   virtual void writeValue(int value) = 0;
   virtual void writeValue(uint value) = 0;
   virtual void writeValue(int64 value) = 0;
   virtual void writeValue(uint64 value) = 0;
   virtual void writeValue(bool value) = 0;
   virtual void writeNullValue() = 0;
 };

} // namespace

#endif // _BASESTRUCTWRITER_H__
