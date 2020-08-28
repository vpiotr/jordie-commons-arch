/////////////////////////////////////////////////////////////////////////////
// Name:        YawlIoClasses.cpp
// Project:     scLib
// Purpose:     OOP interface to Yawl
// Author:      Piotr Likus
// Modified by:
// Created:     10/05/2009
/////////////////////////////////////////////////////////////////////////////
#include "base/string.h"

#include "dtp/YawlIoClasses.h"

using namespace dtp;

#define SC_STRING_TO_UCHAR(a) reinterpret_cast<unsigned char *>(const_cast<char *>(stringToCharPtr(a)))
#define USE_ESCAPE_CHARS
#define YAWL_OFFSET_CHAR 'a'

#define YAWL_ESCAPE_CHAR '@'
const dtpString YAWL_ESCAPE_STR = "@";
const dtpString YAWL_ESCAPE_STR2 = "@@";

enum YawlSignalContextCodes {
  ysccAddNode = 1
};

// ----------------------------------------------------------------------------
// YawlWriter
// ----------------------------------------------------------------------------
YawlWriter::YawlWriter(bool beautifyOn, const char * indentString)
{
  m_config.beautify = beautifyOn;
  m_config.indentString = indentString;
  m_context = yajl_gen_alloc(&m_config, DTP_NULL);
}

void YawlWriter::outputToString(dtpString &output)
{
  const unsigned char * buf;
  unsigned int len;
  yajl_gen_get_buf(m_context, &buf, &len);
  std::string str(reinterpret_cast<char *>(const_cast<unsigned char *>(buf)), len);
  output = str;
  yajl_gen_clear(m_context);
}


void YawlWriter::writeAttrib(const dtpString &name, const dtpString &value)
{
  yajl_gen_string(m_context, SC_STRING_TO_UCHAR(name), name.length());
  yajl_gen_string(m_context, SC_STRING_TO_UCHAR(value), value.length());
}

void YawlWriter::writeAttrib(const dtpString &name, bool value)
{
  yajl_gen_string(m_context, SC_STRING_TO_UCHAR(name), name.length());
  yajl_gen_bool(m_context, value);
}

void YawlWriter::writeAttrib(const dtpString &name, int value)
{
  yajl_gen_string(m_context, SC_STRING_TO_UCHAR(name), name.length());
  yajl_gen_integer(m_context, value);
}

void YawlWriter::writeAttrib(const dtpString &name, double value)
{
  yajl_gen_string(m_context, SC_STRING_TO_UCHAR(name), name.length());
  yajl_gen_double(m_context, value);
}

void YawlWriter::writeNull()
{
  yajl_gen_null(m_context);
}

void YawlWriter::startAttrib(const dtpString &name)
{
  yajl_gen_string(m_context, SC_STRING_TO_UCHAR(name), name.length());
}

void YawlWriter::writeDataNode(const dnode &input)
{
  if (input.isNull())
  {
    writeNull();
  } else if (input.isParent())
  {
    if (input.isList())
      writeDataNodeAsList(input);
    else
      writeDataNodeAsParent(input);
  } else if (input.isArray())
  {
    if (isArraySupported(input.getArrayR()->getValueType()))
      writeDataNodeAsArray(input);
    else
      writeDataNodeAsContainer(input);
  } else { // a single value
    writeDataNodeAsScalar(input);
  }
}

bool YawlWriter::isArraySupported(dnValueType a_type)
{
  bool res;
  switch (a_type) {
    case vt_int:
    case vt_byte:
    case vt_string:
    case vt_bool:
    case vt_double:
    case vt_datanode:
      res = true;
      break;
    default:
      res = false;
      break;
  }
  return res;
}

void YawlWriter::writeDataNodeAsParent(const dnode &input)
{
//  dnChildColn &children = input.getChildren();

  yajl_gen_map_open(m_context);
  int cnt = 0;

  bool isList = input.isList();
  dtpString itemName;
  dnode row;

  for(dnode::const_iterator p = input.begin(); p != input.end(); ++p)
  {
    if (isList)
    {
      //itemName = YAWL_ESCAPE_CHAR;
      itemName = YAWL_ESCAPE_STR;
      itemName += toString(cnt);
      cnt++;
    } else {
      itemName = "";
      //if (p->getName().substr(0, 1) == sc::str(YAWL_ESCAPE_CHAR)) {
      if (p->getName().substr(0, 1) == YAWL_ESCAPE_STR) {
        itemName += YAWL_ESCAPE_STR2;
      }
      itemName += p->getName();
      if (itemName.empty()) {
        itemName = toString(cnt);
        cnt++;
      }
    }

    startAttrib(itemName);

    //writeDataNode(*(p));
    //writeDataNode(p->getAsNodeRef());
    writeDataNode(p->getAsNode(row));
  }

  yajl_gen_map_close(m_context);
}

void YawlWriter::writeDataNodeAsList(const dnode &input)
{
  //dnChildColnBase &children = const_cast<dnChildColnBase &>(input.getChildrenR());
  size_t cnt = input.size();

  yajl_gen_array_open(m_context);
  yajl_gen_integer(m_context, (int)vt_list);

  for(size_t i=0; i < cnt; i++)
  {
    //writeDataNode(children.at(i));
    writeDataNode(input[i]);
  }

  yajl_gen_array_close(m_context);
}

void YawlWriter::writeDataNodeAsArray(const dnode &node)
{
  //const dnArray *arr = node.getArrayR();
  size_t cnt = node.size();
  dnode valueNode;

  yajl_gen_array_open(m_context);
  yajl_gen_integer(m_context, (int)node.getElementType());

  for(size_t i=0; i < cnt; i++)
  {
    //arr->getItem(i, valueNode);
    //writeDataNode(valueNode);
    writeDataNode(node.getNode(i, valueNode));
  }

  yajl_gen_array_close(m_context);
}

void YawlWriter::writeDataNodeAsContainer(const dnode &node)
{
  dnode element;

  yajl_gen_map_open(m_context);
  int cnt = 0;
  dtpString name;

  for(int i = 0, epos = node.size(); i != epos; i++)
  {
    node.getElement(i, element);
    name = node.getElementName(i);
    if (!name.length())
    {
      startAttrib(toString(cnt));
      cnt++;
    } else {
      startAttrib(name);
    }

    writeDataNode(element);
  }

  yajl_gen_map_close(m_context);
}


void YawlWriter::writeDataNodeAsScalar(const dnode &node)
{
  bool bDone = false;

  switch (node.getValueType()) {
    case vt_byte: case vt_int:
      yajl_gen_integer(m_context, node.getAs<int>());
      bDone = true;
      break;
    case vt_bool:
      yajl_gen_bool(m_context, node.getAs<bool>());
      bDone = true;
      break;
    case vt_double:
      yajl_gen_double(m_context, node.getAs<double>());
      bDone = true;
      break;
    case vt_string: {
      dtpString svalue(node.getAs<dtpString>());
#ifdef USE_ESCAPE_CHARS
      //if (svalue.substr(0, 1) == YAWL_ESCAPE_CHAR) {
      if (svalue.substr(0, 1) == YAWL_ESCAPE_STR) {
        //svalue = YAWL_ESCAPE_CHAR + svalue;
        svalue = YAWL_ESCAPE_STR + svalue;
      }
#endif
      yajl_gen_string(m_context, SC_STRING_TO_UCHAR(svalue), svalue.length());
      bDone = true;
      break;
    }
    case vt_null:
      yajl_gen_null(m_context);
      bDone = true;
      break;
    default:
      break;
  }

#ifdef USE_ESCAPE_CHARS
  if (!bDone)
  {
      //dtpString svalue(YAWL_ESCAPE_CHAR);
    dtpString svalue(YAWL_ESCAPE_STR);
      //svalue += strPadLeft(toString((int)node.getValueType()), 2, '0');
      svalue +=
        static_cast<char>(
          static_cast<uint>(node.getValueType()) +
          static_cast<uint>(YAWL_OFFSET_CHAR)
        );

      svalue += node.getAs<dtpString>();
      yajl_gen_string(m_context, SC_STRING_TO_UCHAR(svalue), svalue.length());
      bDone = true;
  }
#endif

  if (!bDone)
  {
    yajl_gen_map_open(m_context);
    writeAttrib("_type", (int)node.getValueType());
    writeAttrib("_value", node.getAs<dtpString>());
    yajl_gen_map_close(m_context);
  }
}

// ----------------------------------------------------------------------------
// reader callbacks
// ----------------------------------------------------------------------------

int process_null(void * ctx)
{
  YawlReaderBase *reader = (YawlReaderBase *) ctx;
  reader->processNull();
  return 1;
}

int process_boolean(void * ctx, int boolVal)
{
  YawlReaderBase *reader = (YawlReaderBase *) ctx;
  reader->processBoolean(boolVal);
  return 1;
}

int process_integer(void * ctx, long integerVal)
{
  YawlReaderBase *reader = (YawlReaderBase *) ctx;
  reader->processInteger(integerVal);
  return 1;
}

int process_double(void * ctx, double doubleVal)
{
  YawlReaderBase *reader = (YawlReaderBase *) ctx;
  reader->processDouble(doubleVal);
  return 1;
}

int process_string(void * ctx, const unsigned char * stringVal,
                     unsigned int stringLen)
{
  YawlReaderBase *reader = (YawlReaderBase *) ctx;
  reader->processString(stringVal, stringLen);
  return 1;
}

int process_map_key(void * ctx, const unsigned char * stringVal,
                   unsigned int stringLen)
{
  YawlReaderBase *reader = (YawlReaderBase *) ctx;
  reader->processMapKey(stringVal, stringLen);
  return 1;
}

int process_start_map(void * ctx)
{
  YawlReaderBase *reader = (YawlReaderBase *) ctx;
  reader->processStartMap();
  return 1;
}

int process_end_map(void * ctx)
{
  YawlReaderBase *reader = (YawlReaderBase *) ctx;
  reader->processEndMap();
  return 1;
}

inline int process_start_array(void * ctx)
{
  YawlReaderBase *reader = (YawlReaderBase *) ctx;
  reader->processStartArray();
  return 1;
}

int process_end_array(void * ctx)
{
  YawlReaderBase *reader = (YawlReaderBase *) ctx;
  reader->processEndArray();
  return 1;
}

const static yajl_callbacks callbacks = {
    process_null,
    process_boolean,
    process_integer,
    process_double,
    DTP_NULL,
    process_string,
    process_start_map,
    process_map_key,
    process_end_map,
    process_start_array,
    process_end_array
};


// ----------------------------------------------------------------------------
// YawlReaderBase
// ----------------------------------------------------------------------------
YawlReaderBase::YawlReaderBase(bool a_checkUtf8, bool a_commentsEnabled)
{
  m_config.checkUTF8 = a_checkUtf8?1:0;
  m_config.allowComments = a_commentsEnabled?1:0;
  m_hand = yajl_alloc(&callbacks, &m_config, DTP_NULL, (void *) this);
}

YawlReaderBase::~YawlReaderBase()
{
  yajl_free(m_hand);
}

bool YawlReaderBase::parseString(const dtpString &input)
{
  m_input = SC_STRING_TO_UCHAR(input);
  m_inputLen = input.length();
  yajl_status stat = yajl_parse(m_hand, m_input, m_inputLen);
  checkStatus(stat);
  return true;
}

void YawlReaderBase::checkStatus(yajl_status stat)
{
  if (stat != yajl_status_ok &&
      stat != yajl_status_insufficient_data)
  {
      unsigned char * str = yajl_get_error(m_hand, 1, m_input, m_inputLen);
      std::string err((char *)str);
      yajl_free_error(m_hand, str);
      throw std::runtime_error(dtpString("JSON parse error: ")+dtpString(err));
  } else {
      parsingFinishedOk();
  }
}

void YawlReaderBase::parsingFinishedOk()
{ //empty here
}

// ----------------------------------------------------------------------------
// YawlReaderForDataNode
// ----------------------------------------------------------------------------

YawlReaderForDataNode::YawlReaderForDataNode(bool a_checkUtf8, bool a_commentsEnabled):
  YawlReaderBase(a_checkUtf8, a_commentsEnabled)
{
  m_current = DTP_NULL;
  m_currentIsArray = false;
  m_firstItem = false;
  m_nodeReady = false;
}

YawlReaderForDataNode::~YawlReaderForDataNode()
{
  clearStack();
}

bool YawlReaderForDataNode::parseString(const dtpString &input, dnode &output)
{
  clearStack();
  output.clear();
  m_root = &output;
  m_current = m_root;
#ifdef YAWL_USE_NODE_STACK
  m_nodeStack.push(m_current);
#endif
  m_nodeReady = true;
  bool res = YawlReaderBase::parseString(input);
  return res;
}

int YawlReaderForDataNode::processNull()
{
  if (m_currentIsArray && !m_current->isList())
  {
    if (!m_current->isContainer())
      m_current->setAsArray<dnode>();
    dnode node;
    node.setAsNull();
    m_current->addItem(node);
  } else if (m_current->isList()) {
    m_current->addChild(new dnode());
  } else {
    m_current->setAsNull();
    closeNode();
  }
  return 1;
}

int YawlReaderForDataNode::processBoolean(int boolVal)
{
  bool strictBoolVal = ((boolVal!=0)?true:false);
  //if (m_current->isArray())
  if (m_currentIsArray && !m_current->isList())
  {
    if (!m_current->isContainer())
      m_current->setAsArray<dnode>();
    dnode node;
    node.setAs(strictBoolVal);
    m_current->addItem(node);
  } else if (m_current->isList()) {
    m_current->addChild(new dnode(strictBoolVal));
  } else {
    m_current->setAs(strictBoolVal);
    closeNode();
  }
  return 1;
}

int YawlReaderForDataNode::processInteger(long integerVal)
{
  //if (m_current->isArray())
  if (m_currentIsArray)
  {
    if (m_firstItem)
    {
      dnValueType newType = static_cast<dnValueType>(integerVal);
      if (newType == vt_list)
        m_current->setAsList();
      else
        m_current->setAsArray(dnValueType(integerVal));
      m_firstItem = false;
    } else {
      if (m_current->isList()) {
        m_current->addChild(new dnode(static_cast<int>(integerVal)));
      } else {
        dnValueType vtype = m_current->getArray()->getValueType();
        switch (vtype) {
        case vt_int:
          m_current->addItemAsInt(static_cast<int>(integerVal));
          break;
        case vt_uint:
          m_current->addItemAsUInt(static_cast<uint>(integerVal));
          break;
        case vt_int64:
          m_current->addItemAsInt64(integerVal);
          break;
        case vt_uint64:
          m_current->addItemAsUInt64(integerVal);
          break;
        default:
        {
          dnode node;
          node.setAs<int>(integerVal);
          if (vtype != vt_datanode)
            convertNodeTo(m_current->getArray()->getValueType(), node, node);
          m_current->addItem(node);
        } // default
      } // switch vtype
    } // !isList
  } // m_currentIsArray
  } else {
    m_current->setAs<int>(integerVal);
    closeNode();
  }
  return 1;
}

int YawlReaderForDataNode::processDouble(double doubleVal)
{
  //if (m_current->isArray())
  if (m_currentIsArray && !m_current->isList())
  {
    if (!m_current->isContainer())
      m_current->setAsArray<double>();
    dnode node;
    node.setAs(doubleVal);
    m_current->addItem(node);
  } else if (m_current->isList()) {
    m_current->addChild(new dnode(doubleVal));
  } else {
    m_current->setAs(doubleVal);
    closeNode();
  }
  return 1;
}

int YawlReaderForDataNode::processString(const unsigned char * stringVal,
                     unsigned int stringLen)
{
  std::string str((char *)const_cast<unsigned char *>(stringVal), stringLen);

  //if (m_current->isArray())
  if (m_currentIsArray && !m_current->isList())
  {
    if (!m_current->isContainer())
      m_current->setAsArray<dnode>();
    dnode node;
    node.setAs(str);
    checkCollapse(node);
    m_current->addItem(node);
  } else if (m_current->isList()) {
    DTP_UNIQUE_PTR(dnode) nodeGuard(new dnode(dtpString(str)));
    checkCollapse(*nodeGuard);
    m_current->addChild(nodeGuard.release());
  } else {
    m_current->setAs(str);
    closeNode();
  }
  return 1;
}

int YawlReaderForDataNode::processMapKey(const unsigned char * stringVal,
                   unsigned int stringLen)
{
  dtpString newName(std::string((char *)const_cast<unsigned char *>(stringVal), stringLen));
  //dnode *node =
  addNode(&newName);
  return 1;
}

int YawlReaderForDataNode::processStartMap()
{
  if (!m_nodeReady)
    addNode();
  else if (m_current->isContainer())
    addNode();

  m_current->setAsParent();
  m_currentIsArray = false;
  m_firstItem = true;
  return 1;
}

int YawlReaderForDataNode::processEndMap()
{
  closeNode();
  return 1;
}

int YawlReaderForDataNode::processStartArray()
{
  if (!m_nodeReady)
    addNode();
  else if (m_current->isContainer())
    addNode();

  //m_current->setAsArray(vt_datanode);
  //m_current->setAsArray<dnode>();
  m_currentIsArray = true;
  m_firstItem = true;
  return 1;
}

int YawlReaderForDataNode::processEndArray()
{
  closeNode();
  return 1;
}

bool YawlReaderForDataNode::convertNodeTo(dnValueType a_type, dnode &input, dnode &output)
{
  bool res = true;
  switch (a_type) {
    case vt_int:
      output.setAs(stringToInt(input.getAs<dtpString>()));
      break;
    case vt_bool:
      output.setAs<bool>(input.getAs<dtpString>() == dtpString("true"));
      break;
    case vt_string:
      output.setAs(input.getAs<dtpString>());
      break;
    case vt_float:
      output.setAs(stringToFloat(input.getAs<dtpString>()));
      break;
    case vt_double:
      output.setAs(stringToDouble(input.getAs<dtpString>()));
      break;
    case vt_xdouble:
      //output.setAsXDouble(stringToXDouble(input.getAsString()));
      output.setAs(stringToXDouble(input.getAs<dtpString>()));
      break;
    case vt_null:
      output.clear();
      break;
    case vt_byte:
      output.setAs<byte>(static_cast<byte>(stringToInt(input.getAs<dtpString>())));
      break;
    case vt_uint:
      output.setAs(stringToUInt(input.getAs<dtpString>()));
      break;
    case vt_int64:
      try {
        output.setAs(stringToInt64(input.getAs<dtpString>()));
      } catch(...) {
        throw dnError("JSON - error while converting value to int64, value: "+input.getAs<dtpString>()); 
      }
      break;
    case vt_uint64:
      output.setAs(stringToUInt64(input.getAs<dtpString>()));
      break;
    case vt_date:
      output.setAs(dateTimeToDate(input.getAs<fdatetime_t>()));
      break;
    case vt_time:
      output.setAs(dateTimeToTime(input.getAs<fdatetime_t>()));
      break;
    case vt_datetime:
      output.setAs(input.getAs<fdatetime_t>());
      break;
    default:
      //no conversion
      res = false;
      break;
  }
  return res;
}


dnode *YawlReaderForDataNode::addNode(dtpString *namePtr)
{
  invokeSignal(ysccAddNode);

  std::auto_ptr<dnode> res;
  const char *name = NULL;
  bool isList = false;

  if (namePtr != DTP_NULL) {
    const char *useName = stringToCharPtr(*namePtr);
    uint skipCnt = 0;
    if (useName[0] == YAWL_ESCAPE_CHAR) {
      skipCnt = 1;
      if (useName[1] != YAWL_ESCAPE_CHAR)
        isList = true;
      else
        skipCnt++;
    }
    //if (isList)
    //  res.reset(new dnode()); // empty name
    //else {
    //  name = useName + skipCnt;
    //  res.reset(new dnode());
    //}
    if (!isList)
      name = useName + skipCnt;
  }

  res.reset(new dnode());

  if (m_current->isParent())
  {
    if (name != NULL)
      m_current->addChild(name, res.get());
    else
      m_current->addChild(res.get());
    m_current = res.release();
  } else if (m_current->isArray()) {
    m_current = res.get();
    //m_elementList.push_back(res.release());
    m_elementList.insert(res.release());
  }

#ifdef YAWL_USE_NODE_STACK
  m_nodeStack.push(m_current);
#endif
  m_nodeReady = true;
  return m_current;
}

void YawlReaderForDataNode::checkCollapse(dnode &node)
{
  // check if non-JSON type, if yes - convert to scalar
  if (
      (
        node.isParent()
         &&
        node.hasChild("_type")
         &&
        node.hasChild("_value")
      )
  )
  {
    collapseScalar(node);
  }
#ifdef USE_ESCAPE_CHARS
  else if (
        (
          (!node.isContainer())
          &&
          (node.getValueType() == vt_string)
          &&
          //(node.getAsString().substr(0, 1) == YAWL_ESCAPE_CHAR)
          (node.getAs<dtpString>().substr(0, 1) == YAWL_ESCAPE_STR)
        )
  )
  {
    unpackScalar(node);
  }
#endif
}

void YawlReaderForDataNode::closeNode()
{
  if (m_current != DTP_NULL)
    checkCollapse(*m_current);
  // go to parent
#ifndef YAWL_USE_NODE_STACK
  assert(dnode::supportsParent());
  assert(m_currentGuard.get() == DTP_NULL); // no support for arrays with data nodes

  if (m_current != DTP_NULL) {
    m_current = m_current->getParent();
  }
#else
  if (!m_nodeStack.empty()) {
    dnode *elementPtr = m_current;

    m_nodeStack.pop();

    if (!m_nodeStack.empty()) {
      m_current = m_nodeStack.top();
    } else {
      m_current = DTP_NULL;
    }

    if (m_elementList.find(elementPtr) != m_elementList.end())
    {
      std::auto_ptr<dnode> elementGuard(elementPtr);

      if (m_current != DTP_NULL)
      {
        m_current->eatElement(*elementPtr);
      }

      YawlElementContainer::iterator it = m_elementList.find(elementPtr);
      m_elementList.erase(it);
    }

  } else {
    m_current = DTP_NULL;
  }
#endif
  
  if (m_current != DTP_NULL) {
    m_currentIsArray = m_current->isArray() || m_current->isList();
  } else {
    m_currentIsArray = false;
  }
}

void YawlReaderForDataNode::clearStack()
{
  while(m_current != DTP_NULL)
    closeNode();
}

void YawlReaderForDataNode::collapseScalar(dnode &node)
{
  dnValueType ntype = dnValueType(node["_type"].getAs<int>());
  dnode value = node["_value"];
  dtpString nformat;

  node.clear();
  convertNodeTo(ntype, value, node);
}

void YawlReaderForDataNode::unpackScalar(dnode &node)
{
  const int VAL_TYPE_DEFAULT = -1;
  const int VAL_TYPE_NO_CONV = -2;

  dtpString svalue = node.getAs<dtpString>();
  dtpString valTypeText = svalue.substr(1,2);
  uint skipCnt = 1;
  int valTypeInt;
  char typeChar;

  if (!valTypeText.empty())
    typeChar = valTypeText.c_str()[0];
  else
    typeChar = YAWL_ESCAPE_CHAR;

  if (typeChar != YAWL_ESCAPE_CHAR)
  {
    if ((typeChar >= 'a') && (typeChar <= 'z')) {
      skipCnt += 1;
      valTypeInt = typeChar - YAWL_OFFSET_CHAR;
    } else {
    // unescaped value - accept as-is
      skipCnt = 0;
      valTypeInt = VAL_TYPE_NO_CONV;
    }
  } else {
    valTypeInt = VAL_TYPE_DEFAULT;
  }

  if (valTypeInt == VAL_TYPE_NO_CONV)
      return;

  dnValueType ntype(vt_string);

  if (valTypeInt != VAL_TYPE_DEFAULT) {
    ntype = static_cast<dnValueType>(static_cast<uint>(valTypeInt));
  }

  node.clear();
  const char *cptr = stringToCharPtr(svalue);
  cptr += skipCnt;
  dnode value(cptr);
  convertNodeTo(ntype, value, node);
}

