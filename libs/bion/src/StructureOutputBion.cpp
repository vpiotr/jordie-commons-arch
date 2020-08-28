/////////////////////////////////////////////////////////////////////////////
// Name:        StructureOutputBion.cpp
// Project:     baseLib
// Purpose:     On-fly serializer for data structures - BION output.
// Author:      Piotr Likus
// Modified by:
// Created:     21/10/2013
/////////////////////////////////////////////////////////////////////////////

#include "base/StructureOutputBion.h"

using namespace base;

void StructureOutputBion::accept(StructureWriterIntf &writer)
{
  writer.visit(*this);
}

// visitation
void StructureOutputBion::beginWrite()
{
  m_writer.writeHeader();
}

void StructureOutputBion::endWrite()
{
  m_writer.writeFooter();
}

void StructureOutputBion::beginMap()
{
  m_writer.writeObjectBegin();
  m_structStack.push_back(st_map);
}

void StructureOutputBion::endMap()
{
  m_writer.writeObjectEnd();
  m_structStack.pop_back();
}

void StructureOutputBion::beginArray(size_t size)
{
  m_writer.writeArrayBegin();
  m_structStack.push_back(st_array_of_var);
  m_fixTypeArrayOpen = false;
}

void StructureOutputBion::beginArrayOf(const std::string &valueTag, size_t size)
{
  m_writer.writeArrayBegin();
  m_structStack.push_back(st_array_of_var);
  m_fixTypeArrayOpen = false;
}

void StructureOutputBion::beginArrayOf(float valueTag, size_t size)
{
  if (size > 0)
  {
    m_writer.writeFixTypeArrayBegin(size);
    m_writer.writeValueType(valueTag);
    m_structStack.push_back(st_array_of_fixtype);
    m_fixTypeArrayOpen = true;
  } else {
    m_writer.writeArrayBegin();
    m_structStack.push_back(st_array_of_var);
    m_fixTypeArrayOpen = false;
  }
}

void StructureOutputBion::beginArrayOf(double valueTag, size_t size)
{
  if (size > 0)
  {
    m_writer.writeFixTypeArrayBegin(size);
    m_writer.writeValueType(valueTag);
    m_structStack.push_back(st_array_of_fixtype);
    m_fixTypeArrayOpen = true;
  } else {
    m_writer.writeArrayBegin();
    m_structStack.push_back(st_array_of_var);
    m_fixTypeArrayOpen = false;
  }
}

void StructureOutputBion::beginArrayOf(xdouble valueTag, size_t size)
{
  if (size > 0)
  {
    m_writer.writeFixTypeArrayBegin(size);
    m_writer.writeValueType(valueTag);
    m_structStack.push_back(st_array_of_fixtype);
    m_fixTypeArrayOpen = true;
  } else {
    m_writer.writeArrayBegin();
    m_structStack.push_back(st_array_of_var);
    m_fixTypeArrayOpen = false;
  }
}

void StructureOutputBion::beginArrayOf(bool valueTag, size_t size)
{
  if (size > 0)
  {
    m_writer.writeFixTypeArrayBegin(size);
    m_writer.writeValueType(valueTag);
    m_structStack.push_back(st_array_of_fixtype);
    m_fixTypeArrayOpen = true;
  } else {
    m_writer.writeArrayBegin();
    m_structStack.push_back(st_array_of_var);
    m_fixTypeArrayOpen = false;
  }
}

void StructureOutputBion::beginArrayOf(byte valueTag, size_t size)
{
  if (size > 0)
  {
    m_writer.writeFixTypeArrayBegin(size);
    m_writer.writeValueType(valueTag);
    m_structStack.push_back(st_array_of_fixtype);
    m_fixTypeArrayOpen = true;
  } else {
    m_writer.writeArrayBegin();
    m_structStack.push_back(st_array_of_var);
    m_fixTypeArrayOpen = false;
  }
}

void StructureOutputBion::beginArrayOf(int valueTag, size_t size)
{
  if (size > 0)
  {
    m_writer.writeFixTypeArrayBegin(size);
    m_writer.writeValueType(valueTag);
    m_structStack.push_back(st_array_of_fixtype);
    m_fixTypeArrayOpen = true;
  } else {
    m_writer.writeArrayBegin();
    m_structStack.push_back(st_array_of_var);
    m_fixTypeArrayOpen = false;
  }
}

void StructureOutputBion::beginArrayOf(uint valueTag, size_t size)
{
  if (size > 0)
  {
    m_writer.writeFixTypeArrayBegin(size);
    m_writer.writeValueType(valueTag);
    m_structStack.push_back(st_array_of_fixtype);
    m_fixTypeArrayOpen = true;
  } else {
    m_writer.writeArrayBegin();
    m_structStack.push_back(st_array_of_var);
    m_fixTypeArrayOpen = false;
  }
}

void StructureOutputBion::beginArrayOf(int64 valueTag, size_t size)
{
  if (size > 0)
  {
    m_writer.writeFixTypeArrayBegin(size);
    m_writer.writeValueType(valueTag);
    m_structStack.push_back(st_array_of_fixtype);
    m_fixTypeArrayOpen = true;
  } else {
    m_writer.writeArrayBegin();
    m_structStack.push_back(st_array_of_var);
    m_fixTypeArrayOpen = false;
  }
}

void StructureOutputBion::beginArrayOf(uint64 valueTag, size_t size)
{
  if (size > 0)
  {
    m_writer.writeFixTypeArrayBegin(size);
    m_writer.writeValueType(valueTag);
    m_structStack.push_back(st_array_of_fixtype);
    m_fixTypeArrayOpen = true;
  } else {
    m_writer.writeArrayBegin();
    m_structStack.push_back(st_array_of_var);
    m_fixTypeArrayOpen = false;
  }
}

void StructureOutputBion::endArray()
{
  if (!m_structStack.empty())
  {
    int structType = m_structStack.back();
    
    if (structType == st_array_of_fixtype)
      m_writer.writeFixTypeArrayEnd();
    else 
      m_writer.writeArrayEnd();
      
    m_structStack.pop_back();

    if (m_structStack.empty())
      m_fixTypeArrayOpen = false;
    else
      m_fixTypeArrayOpen = (m_structStack.back() == st_array_of_fixtype);
  }
}

//----
void StructureOutputBion::writeKeyName(const std::string &name)
{
   m_writer.writeElementName(name);
}

//----
void StructureOutputBion::writeValue(const std::string &value)
{
  if (m_fixTypeArrayOpen)
    m_writer.writeValueData(value);
  else  
    m_writer.writeValue(value);
}

void StructureOutputBion::writeValue(float value)
{
  if (m_fixTypeArrayOpen)
    m_writer.writeValueData(value);
  else  
    m_writer.writeValue(value);
}

void StructureOutputBion::writeValue(double value)
{
  if (m_fixTypeArrayOpen)
    m_writer.writeValueData(value);
  else  
    m_writer.writeValue(value);
}

void StructureOutputBion::writeValue(xdouble value)
{
  if (m_fixTypeArrayOpen)
    m_writer.writeValueData(value);
  else  
    m_writer.writeValue(value);
}

void StructureOutputBion::writeValue(byte value)
{
  if (m_fixTypeArrayOpen)
    m_writer.writeValueData(value);
  else  
    m_writer.writeValue(value);
}

void StructureOutputBion::writeValue(int value)
{
  if (m_fixTypeArrayOpen)
    m_writer.writeValueData(value);
  else  
    m_writer.writeValue(value);
}

void StructureOutputBion::writeValue(uint value)
{
  if (m_fixTypeArrayOpen)
    m_writer.writeValueData(value);
  else  
    m_writer.writeValue(value);
}

void StructureOutputBion::writeValue(int64 value)
{
  if (m_fixTypeArrayOpen)
    m_writer.writeValueData(value);
  else  
    m_writer.writeValue(value);
}

void StructureOutputBion::writeValue(uint64 value)
{
  if (m_fixTypeArrayOpen)
    m_writer.writeValueData(value);
  else  
    m_writer.writeValue(value);
}

void StructureOutputBion::writeValue(bool value)
{
  if (m_fixTypeArrayOpen)
    m_writer.writeValueData(value);
  else  
    m_writer.writeValue(value);
}

void StructureOutputBion::writeNullValue()
{
  if (m_fixTypeArrayOpen)
    m_writer.writeNullValueData();
  else  
    m_writer.writeNullValue();
}

