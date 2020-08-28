/////////////////////////////////////////////////////////////////////////////
// Name:        YawlIoClasses.h
// Project:     dtpLib
// Purpose:     OOP interface to Yawl - profiled for dtp::dnode
// Author:      Piotr Likus
// Modified by:
// Created:     10/05/2009
/////////////////////////////////////////////////////////////////////////////

#ifndef _YAWLIOCLASSES_H__
#define _YAWLIOCLASSES_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file YawlIoClasses.h
///
/// File description

// ----------------------------------------------------------------------------
// Configuration
// ----------------------------------------------------------------------------
#define YAWL_USE_NODE_STACK

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#ifdef YAWL_USE_NODE_STACK
#include <stack>
#endif

#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "api/yajl_gen.h"
#include "api/yajl_parse.h"

#include "base/string.h"

#include "dtp/details/dtypes.h"
#include "dtp/dnode.h"

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
// YawlWriter
// ----------------------------------------------------------------------------
class YawlWriter {
public:
  YawlWriter(bool beautifyOn, const char * indentString);
  ~YawlWriter() {yajl_gen_free(m_context);};
  yajl_gen *getContext() {return &m_context;}
  void outputToString(dtpString &output);
  void writeAttrib(const dtpString &name, const dtpString &value);
  void writeAttrib(const dtpString &name, bool value);
  void writeAttrib(const dtpString &name, int value);
  void writeAttrib(const dtpString &name, double value);
  void startAttrib(const dtpString &name);
  void writeNull();
  void writeDataNode(const dtp::dnode &input);
protected:
  void writeDataNodeAsParent(const dtp::dnode &input);
  void writeDataNodeAsList(const dtp::dnode &input);
  void writeDataNodeAsArray(const dtp::dnode &node);
  void writeDataNodeAsScalar(const dtp::dnode &node);
  bool isArraySupported(dnValueType a_type);
  void writeDataNodeAsContainer(const dtp::dnode &node);
protected:
  yajl_gen m_context;
  yajl_gen_config m_config;
};

// ----------------------------------------------------------------------------
// YawlReader
// ----------------------------------------------------------------------------
class YawlReaderBase {
public:
  YawlReaderBase(bool a_checkUtf8, bool a_commentsEnabled);
  virtual ~YawlReaderBase();

  bool parseString(const dtpString &input);

  virtual int processNull() = 0;
  virtual int processBoolean(int boolVal) = 0;
  virtual int processInteger(long integerVal) = 0;
  virtual int processDouble(double doubleVal) = 0;
  virtual int processString(const unsigned char * stringVal,
                       unsigned int stringLen) = 0;
  virtual int processMapKey(const unsigned char * stringVal,
                     unsigned int stringLen) = 0;
  virtual int processStartMap() = 0;
  virtual int processEndMap() = 0;
  virtual int processStartArray() = 0;
  virtual int processEndArray() = 0;
protected:
  void parsingFinishedOk();
  void checkStatus(yajl_status stat);
protected:
  yajl_parser_config m_config;
  yajl_handle m_hand;
  unsigned char *m_input;
  size_t m_inputLen;
};

#ifdef YAWL_USE_NODE_STACK
typedef std::stack<dtp::dnode *> YawlNodeStack;
#endif

typedef std::set<dtp::dnode *> YawlElementContainer;

class YawlReaderForDataNode: public YawlReaderBase {
public:
  // construct
  YawlReaderForDataNode(bool a_checkUtf8, bool a_commentsEnabled);
  virtual ~YawlReaderForDataNode();
  // run
  bool parseString(const dtpString &input, dtp::dnode &output);
protected:
  dtp::dnode *addNode(dtpString *namePtr = DTP_NULL);
  void closeNode();
  void collapseScalar(dtp::dnode &node);
  void unpackScalar(dtp::dnode &node);
  bool convertNodeTo(dnValueType a_type, dtp::dnode &input, dtp::dnode &output);

  virtual int processNull();
  virtual int processBoolean(int boolVal);
  virtual int processInteger(long integerVal);
  virtual int processDouble(double doubleVal);
  virtual int processString(const unsigned char * stringVal,
                       unsigned int stringLen);
  virtual int processMapKey(const unsigned char * stringVal,
                     unsigned int stringLen);
  virtual int processStartMap();
  virtual int processEndMap();
  virtual int processStartArray();
  virtual int processEndArray();
  void clearStack();
  void checkCollapse(dtp::dnode &node);
  virtual void invokeSignal(uint context) {} // override to use
protected:
  dtp::dnode *m_root;
  dtp::dnode *m_current;
  YawlElementContainer m_elementList;
#ifdef YAWL_USE_NODE_STACK
  YawlNodeStack m_nodeStack;
#endif
  bool m_nodeReady;
  bool m_firstItem;
  bool m_currentIsArray;
};

} // namespace
#endif // _YAWLIOCLASSES_H__
