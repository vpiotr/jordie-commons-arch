/////////////////////////////////////////////////////////////////////////////
// Name:        StructureOutputBion.h
// Project:     baseLib
// Purpose:     On-fly serializer for data structures - BION output.
// Author:      Piotr Likus
// Modified by:
// Created:     21/10/2013
/////////////////////////////////////////////////////////////////////////////

#ifndef _BASESTRUCTWRITERBION_H__
#define _BASESTRUCTWRITERBION_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file StructureWriter.h
///
/// On-fly serializer for data structures - BION output.

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
// base
#include "base/string.h"
#include "base/StructureWriter.h"
#include "base/Bion.h"

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
class StructureOutputBion: public StructureOutputIntf {
public: 
   // construction
   StructureOutputBion(BionWriterIntf &writer): m_writer(writer), m_fixTypeArrayOpen(false) {}
   virtual ~StructureOutputBion() {}
   // execution
   virtual void accept(StructureWriterIntf &writer);
   // visitation
   virtual void beginWrite();
   virtual void endWrite();
   virtual void beginMap();
   virtual void endMap();
   virtual void beginArray(size_t size = 0);
   virtual void beginArrayOf(const std::string &valueTag, size_t size = 0);
   virtual void beginArrayOf(float valueTag, size_t size = 0);
   virtual void beginArrayOf(double valueTag, size_t size = 0);
   virtual void beginArrayOf(xdouble valueTag, size_t size = 0);
   virtual void beginArrayOf(bool valueTag, size_t size = 0);
   virtual void beginArrayOf(byte valueTag, size_t size = 0);
   virtual void beginArrayOf(int valueTag, size_t size = 0);
   virtual void beginArrayOf(uint valueTag, size_t size = 0);
   virtual void beginArrayOf(int64 valueTag, size_t size = 0);
   virtual void beginArrayOf(uint64 valueTag, size_t size = 0);
   virtual void endArray();
   //----
   virtual void writeKeyName(const std::string &name);
   //----
   virtual void writeValue(const std::string &value);
   virtual void writeValue(float value);
   virtual void writeValue(double value);
   virtual void writeValue(xdouble value);
   virtual void writeValue(byte value);
   virtual void writeValue(int value);
   virtual void writeValue(uint value);
   virtual void writeValue(int64 value);
   virtual void writeValue(uint64 value);
   virtual void writeValue(bool value);
   virtual void writeNullValue();
 private:
   enum StructType {
     st_array_of_fixtype = 1,
     st_array_of_var,
     st_map
   };
 private:   
   BionWriterIntf &m_writer;
   std::vector<int> m_structStack;
   bool m_fixTypeArrayOpen;
 };

} // namespace

#endif // _BASESTRUCTWRITERBION_H__
