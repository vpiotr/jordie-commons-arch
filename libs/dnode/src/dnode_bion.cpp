/////////////////////////////////////////////////////////////////////////////
// Name:        dnode_bion.cpp
// Project:     dtpLib
// Purpose:     BION serializer for dnode
// Author:      Piotr Likus
// Modified by:
// Created:     23/06/2013
/////////////////////////////////////////////////////////////////////////////

#include "dtp/dnode_bion.h"

using namespace dtp;

dnBionProcessor::dnBionProcessor(dtp::dnode &output): m_output(output)
{
  m_output.clear();
  m_arrayInitWait = false;
}

void dnBionProcessor::processHeader(void *data, size_t dataSize)
{
  m_openNodes.clear();
  m_arrayInitWait = false;
  m_openNodes.push_back(&m_output);
}

void dnBionProcessor::processFooter(void *data, size_t dataSize)
{
  while(!m_openNodes.empty())
    closeNode();
}

void dnBionProcessor::processObjectBegin()
{
  beforeOpenStruct();
  std::auto_ptr<dtp::dnode> node(new dnode(ict_parent));
  openNode(addStruct(node.release()));
}

void dnBionProcessor::processObjectEnd()
{
  closeNode();
}

void dnBionProcessor::beforeOpenStruct()
{
  if (m_arrayInitWait && !m_openNodes.empty()) {
    dtp::dnode *target = m_openNodes.back();

    if (!target->isContainer())
       initArray(target, dbatList);
  }
}

void dnBionProcessor::processArrayBegin()
{
  beforeOpenStruct();
  dnode *ptr = addStruct(new dnode());
  openNode(ptr);
  m_arrayInitWait = true;
}
  
void dnBionProcessor::processArrayEnd()
{
  m_arrayInitWait = false;
  closeNode();
}

dnValueType dnBionProcessor::mapBionValTypeToDNodeValType(BionValueType valueType, unsigned int elementSize)
{
  switch (valueType) {
  case bvt_null: return vt_null;
  case bvt_bool: return vt_bool;
  case bvt_int: 
    if (elementSize == sizeof(int64))
      return vt_int64;
    else 
      return vt_int;
  case bvt_uint: 
    if (elementSize == sizeof(uint64))
      return vt_uint64;
    else 
      return vt_uint;
  case bvt_float: 
    if (elementSize == sizeof(float))
      return vt_float;
    else if (elementSize == sizeof(xdouble))
      return vt_xdouble;
    else 
      return vt_double;
  case bvt_zstring:
    return vt_string;
  default:
    throw dnBionError(DBE_UndefValueType);
  }
}

void dnBionProcessor::processFixTypeArrayBegin(BionValueType valueType, unsigned int elementSize, size_t arraySize)
{
  dnValueType dnValType = mapBionValTypeToDNodeValType(valueType, elementSize);
  openNode(addStruct(new dnode(ict_array, dnValType)));
}

void dnBionProcessor::processFixTypeArrayEnd()
{
  closeNode();
}

/// Returns added node or NULL if node cannot be open
dtp::dnode *dnBionProcessor::addStruct(dtp::dnode *node)
{
  assert(!m_openNodes.empty());
  dtp::dnode *target = m_openNodes.back();

  if (target->isContainer()) {
    // add struct to container
    return addElementImpl(target, node);
  } else {
    std::auto_ptr<dnode> guard(node);

    if (m_arrayInitWait)
    {
      if (node->isContainer()) {
        // adding struct to not-initialized-yet array
        initArray(target, dbatList); 
        return addElementImpl(target, guard.release());
      } else {
        // target is not-initialized-yet array & we received item type
        initArray(target, node->getAsInt()); 
        return NULL;
      }
    } else {
      // target is output (root) node & not a container yet
      *target = *guard;
      return NULL;
    }
  }
}

//dtp::dnode *dnBionProcessor::addFirstElement(dtp::dnode *node)
//{
//  assert(m_openNodes.empty());
//  m_openNodes.push_back(node);
//  return node;
//}

void dnBionProcessor::addScalar(const dtp::dnode &node)
{
  assert(!m_openNodes.empty());
  dtp::dnode *target = m_openNodes.back();

  if (!target->isContainer())
  {
    if (m_arrayInitWait) {
      initArray(target, node.getAsInt());
    } else {
      *target = node;
    }
  } else {
    return addScalarImpl(target, node);
  }
}

dtp::dnode *dnBionProcessor::addElementImpl(dtp::dnode *target, dtp::dnode *node)
{
  assert(target->isContainer());
  dtp::dnode *result;

  if (target->isArray())
  {
    std::auto_ptr<dnode> guard(node);
    target->addItem(*guard);
    result = target->getNodePtr(target->size() - 1, *node);
    if (result == node) 
      result = NULL; // node pointer cannot be returned
  } else if (m_lastName.empty()) {
    target->addChild(node);
    result = node;
  } else {
    target->addChild(m_lastName, node);
    result = node;
    m_lastName.clear();
  }

  return result;
}

void dnBionProcessor::addScalarImpl(dtp::dnode *target, const dtp::dnode &node)
{
  assert(target->isContainer());

  if (target->isArray())
  {
    target->addItem(node);
  } else if (m_lastName.empty()) {
    target->addChild(node);
  } else {
    target->addChild(m_lastName, node);
    m_lastName.clear();
  }
}

void dnBionProcessor::initArray(dtp::dnode *target, int arrayType)
{
  m_arrayInitWait = false;

  if (arrayType == dbatList) {
    target->setAsList();
  } else {
    target->setAsArray(static_cast<dnValueType>(arrayType - dbatArray));
  }
}

void dnBionProcessor::openNode(dtp::dnode *node)
{
  if (node != NULL)
    m_openNodes.push_back(node);
}

void dnBionProcessor::closeNode()
{
  m_openNodes.pop_back();
}

void dnBionProcessor::processElementName(char *name)
{
  m_lastName = std::string(name);
}

void dnBionProcessor::processFloat(float value)
{
  //addElement(new dnode(value));
  dnode node(value);
  addScalar(node);
}

void dnBionProcessor::processDouble(double value)
{
  //addElement(new dnode(value));
  dnode node(value);
  addScalar(node);
}

void dnBionProcessor::processXDouble(xdouble value)
{
  dnode node(value);
  addScalar(node);
}

void dnBionProcessor::processZString(char *value)
{
  //addElement(new dnode(value));
  dnode node(value);
  addScalar(node);
}

void dnBionProcessor::processBool(bool value)
{
  //addElement(new dnode(value));
  dnode node(value);
  addScalar(node);
}

void dnBionProcessor::processNull()
{
  //addElement(new dnode());
  dnode node;
  addScalar(node);
}

void dnBionProcessor::processInt(int value)
{
  //addElement(new dnode(value));
  dnode node(value);
  addScalar(node);
}

void dnBionProcessor::processInt64(int64 value)
{
  //addElement(new dnode(value));
  dnode node(value);
  addScalar(node);
}

void dnBionProcessor::processUInt(uint value)
{
  //addElement(new dnode(value));
  dnode node(value);
  addScalar(node);
}

void dnBionProcessor::processUInt64(uint64 value)
{
  //addElement(new dnode(value));
  dnode node(value);
  addScalar(node);
}

