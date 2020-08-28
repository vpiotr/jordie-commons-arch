/////////////////////////////////////////////////////////////////////////////
// Name:        dnode_bion.h
// Project:     dtpLib
// Purpose:     BION serialization for dnode
// Author:      Piotr Likus
// Modified by:
// Created:     23/06/2013
/////////////////////////////////////////////////////////////////////////////

#ifndef _DNBION_H__
#define _DNBION_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file dnode_bion.h
\brief BION serialization for dnode

No further details.
*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include <vector>
#include <string>
#include "base/bion.h"
#include "dtp/dnode.h"

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Forward class definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
enum dnBionStructCode {
  dbscNull = 0,
  dbscArray = 1,
  dbscObject = 2
};

enum dnBionArrayType {
  dbatList = 0,
  dbatArray = 1
};

enum dnBionErrorCodes {
  DBE_WrongScalarType = 1,
  DBE_UndefContainerType = 2,
  DBE_UndefValueType = 3
};

// ----------------------------------------------------------------------------
// Class definitions
// ----------------------------------------------------------------------------
class dnBionError: public std::runtime_error {
public:
  dnBionError(unsigned int errorCode): std::runtime_error(std::string("DNBION")+toString(errorCode)) {}
};

class dnBionProcessor
#ifdef DEBUG
  : public BionReaderProcessorIntf
#endif
{
public:
  dnBionProcessor(dtp::dnode &output);
  void processHeader(void *data, size_t dataSize);
  void processFooter(void *data, size_t dataSize);
  void processObjectBegin();
  void processObjectEnd();
  void processArrayBegin();
  void processArrayEnd();
  void processFixTypeArrayBegin(BionValueType valueType, unsigned int elementSize, size_t arraySize);
  void processFixTypeArrayEnd();
  void processElementName(char *name);
  void processFloat(float value);
  void processDouble(double value);
  void processXDouble(xdouble value);
  void processZString(char *value);
  void processBool(bool value);
  void processNull();
  void processInt(int value);
  void processInt64(int64 value);
  void processUInt(uint value);
  void processUInt64(uint64 value);
protected:
  dtp::dnode *addStruct(dtp::dnode *node);
  void addScalar(const dtp::dnode &node);
  void openNode(dtp::dnode *node);
  void closeNode();
  void initArray(dtp::dnode *target, int arrayType);
  dtp::dnode *addElementImpl(dtp::dnode *target, dtp::dnode *node);
  void addScalarImpl(dtp::dnode *target, const dtp::dnode &node);
  dtp::dnValueType mapBionValTypeToDNodeValType(BionValueType valueType, unsigned int elementSize);
  void beforeOpenStruct();
  //dtp::dnode *addFirstElement(dtp::dnode *node);
private:
  //std::vector<int> m_openStructs;
  std::vector<dtp::dnode *> m_openNodes;
  std::string m_lastName;
  bool m_arrayInitWait;
  dtp::dnode &m_output;
};

template<typename Output>
class dnBionWriter {
public:
  dnBionWriter(Output &output): m_writer(output) {}

  void write(const dtp::dnode &node) {
    m_writer.writeHeader();
    writeNode(node);
    m_writer.writeFooter();
  }

protected:
  void writeNode(const dtp::dnode &node) {
    if (!node.isContainer()) {
      writeScalar(node);
    } else if (node.isList()) {
      m_writer.writeArrayBegin();
      m_writer.writeInt(dbatList);
      dnode helper;
      for(uint i=0, epos = node.size(); i != epos; ++i)
        writeNode(node.getNode(i, helper));
      m_writer.writeArrayEnd();
    } else if (node.isArray())
    {
      if (node.getElementType() == vt_null) {
        m_writer.writeArrayBegin();
        m_writer.writeInt(dbatArray + static_cast<uint>(node.getElementType()));
        dnode helper;
        for(uint i=0, epos = node.size(); i != epos; ++i)
          writeNode(node.getNode(i, helper));
        m_writer.writeArrayEnd();
      } else {
        m_writer.writeFixTypeArrayBegin(node.size());
        writeElementType(node.getElementType());
        for(uint i=0, epos = node.size(); i != epos; ++i)
          writeNodeElementValue(node, i);
        m_writer.writeFixTypeArrayEnd();
      } // array as fix type
    } else if (node.isParent())
    {
      m_writer.writeObjectBegin();
      dnode helper;
      for(uint i=0, epos = node.size(); i != epos; ++i) {
        m_writer.writeElementName(node.getElementName(i));
        writeNode(node.getNode(i, helper));
      }
      m_writer.writeObjectEnd();
    } else {
      throw dnBionError(DBE_UndefContainerType);
    }
  }

  void writeScalar(const dtp::dnode &node) {
    switch(node.getValueType()) {
    case vt_int:
      m_writer.writeInt(node.getAsInt());
      break;
    case vt_uint:
      m_writer.writeUInt(node.getAsUInt());
      break;
    case vt_float:
      m_writer.writeFloat(node.getAsFloat());
      break;
    case vt_double:
      m_writer.writeDouble(node.getAsDouble());
      break;
    case vt_xdouble:
      m_writer.writeXDouble(node.getAsXDouble());
      break;
    case vt_bool:
      m_writer.writeBool(node.getAsBool());
      break;
    case vt_string:
      m_writer.writeZString(node.getAsString().c_str());
      break;
    case vt_null:
      m_writer.writeNull();
      break;
    default:
      throw dnBionError(DBE_WrongScalarType);
    }
  }

  void writeElementType(dtp::dnValueType aType)
  {
      switch(aType) {
      case vt_int:
        m_writer.writeIntType();
        break;
      case vt_int64:
        m_writer.writeInt64Type();
        break;
      case vt_uint:
        m_writer.writeUIntType();
        break;
      case vt_uint64:
        m_writer.writeUInt64Type();
        break;
      case vt_float:
        m_writer.writeFloatType();
        break;
      case vt_double:
        m_writer.writeDoubleType();
        break;
      case vt_xdouble:
        m_writer.writeXDoubleType();
        break;
      case vt_string:
        m_writer.writeZStringType();
        break;
      case vt_bool:
        m_writer.writeBoolType();
        break;
      case vt_null:
        m_writer.writeNullType();
        break;
      default:
        throw dnBionError(DBE_UndefValueType);
      } // switch
  }

  void writeNodeElementValue(const dtp::dnode &node, uint idx)
  {
    switch(node.getElementType()) {
    case vt_int:
      m_writer.writeIntData(node.getInt(idx));
      break;
    case vt_uint:
      m_writer.writeUIntData(node.getUInt(idx));
      break;
    case vt_int64:
      m_writer.writeInt64Data(node.getInt64(idx));
      break;
    case vt_uint64:
      m_writer.writeUInt64Data(node.getUInt64(idx));
      break;
    case vt_float:
      m_writer.writeFloatData(node.getFloat(idx));
      break;
    case vt_double:
      m_writer.writeDoubleData(node.getDouble(idx));
      break;
    case vt_xdouble:
      m_writer.writeXDoubleData(node.getXDouble(idx));
      break;
    case vt_bool:
      m_writer.writeBoolData(node.getBool(idx));
      break;
    case vt_string:
      m_writer.writeZStringData(node.getString(idx).c_str());
      break;
    case vt_null:
      m_writer.writeNullData();
      break;
    default:
      throw dnBionError(DBE_WrongScalarType);
    }
  }

private:
  BionWriter<Output> m_writer;
};

#endif // _DNBION_H__
