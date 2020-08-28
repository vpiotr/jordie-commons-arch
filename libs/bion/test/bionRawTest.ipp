/////////////////////////////////////////////////////////////////////////////
// Name:        bionRawTest.ipp
// Project:     Test simple input/output for BION format.
// Purpose:     baseLib
// Author:      Piotr Likus
// Modified by:
// Created:     23/06/2013
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "base/Bion.h"

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

class BionConsOutProcessor {
public:  
  void processHeader(void *data, size_t dataSize) {
    std::cout << "header\n";
  }

  void processFooter(void *data, size_t dataSize) {
    std::cout << "footer\n";
  }

  void processObjectBegin() {
    std::cout << "object-begin\n";
  }

  void processObjectEnd(){
    std::cout << "object-end\n";
  }

  void processArrayBegin(){
    std::cout << "array-begin\n";
  }

  void processArrayEnd() {
    std::cout << "array-end\n";
  }

  void processFixTypeArrayBegin(BionValueType valueType, unsigned int elementSize, size_t arraySize) 
  {
    std::cout << "fixtype-array-begin, element type: " << valueType << ", element size: " << elementSize << ", array size: " << arraySize << "\n";
  }

  void processFixTypeArrayEnd() {
    std::cout << "fixtype-array-end\n";
  }

  void processElementName(char *name) {
    std::cout << "element-name: " << name << "\n";
  }

  void processFloat(float value) {
    std::cout << "float: " << value << "\n";
  }

  void processDouble(double value) {
    std::cout << "double: " << value << "\n";
  }

  void processXDouble(xdouble value) {
    std::cout << "xdouble: " << value << "\n";
  }

  void processZString(char *value) {
    std::cout << "zstring: " << value << "\n";
  }

  void processBool(bool value) {
    std::cout << "bool: " << value << "\n";
  }

  void processNull() {
    std::cout << "null\n";
  }

  void processInt(int value) {
    std::cout << "int: " << value << "\n";
  }

  void processInt64(int64 value) {
    std::cout << "int64: " << value << "\n";
  }

  void processUInt(uint value) {
    std::cout << "uint: " << value << "\n";
  }

  void processUInt64(uint64 value) {
    std::cout << "uint64: " << value << "\n";
  }

};

BOOST_AUTO_TEST_CASE(bion_raw_test)
{
  std::stringstream s;

  BionWriter<std::stringstream> writer(s);

  writer.writeHeader();

  writer.writeArrayBegin();
  writer.writeDouble(3.14);
  writer.writeArrayEnd();

  writer.writeFooter();

  s.seekg(0, s.end);
  size_t length = s.tellg();
  BOOST_CHECK(length > 0);

  BOOST_TEST_MESSAGE(std::string("bion stream size: ") + toString(length));

  s.seekg(0, std::ios::beg);

  BionConsOutProcessor proc;
  BionReader<std::stringstream, BionConsOutProcessor> reader(s, proc);

  reader.process();
}

BOOST_AUTO_TEST_CASE(bion_raw_test2)
{
  std::stringstream s;

  BionWriter<std::stringstream> writer(s);

  writer.writeHeader();

  writer.writeArrayBegin();
  writer.writeObjectBegin();
  writer.writeElementName("val1");
  writer.writeDouble(6.28);
  writer.writeElementName("val2");
  writer.writeDouble(3.14);
  writer.writeObjectEnd();
  writer.writeInt(628);
  writer.writeBool(false);
  writer.writeZString("test123");
  writer.writeBool(true);
  writer.writeArrayEnd();

  writer.writeFooter();

  s.seekg(0, s.end);
  size_t length = s.tellg();
  BOOST_CHECK(length > 0);

  BOOST_TEST_MESSAGE(std::string("bion stream size: ") + toString(length));

  s.seekg(0, std::ios::beg);

  BionConsOutProcessor proc;
  BionReader<std::stringstream, BionConsOutProcessor> reader(s, proc);

  reader.process();
}

BOOST_AUTO_TEST_CASE(bion_raw_test_fixtype)
{
  std::stringstream s;

  BionWriter<std::stringstream> writer(s);

  writer.writeHeader();

  writer.writeFixTypeArrayBegin(3);
  writer.writeDoubleType();
  writer.writeDoubleData(3.14);
  writer.writeDoubleData(115.0);
  writer.writeDoubleData(1.0);
  writer.writeFixTypeArrayEnd();

  writer.writeFooter();

  s.seekg(0, s.end);
  size_t length = s.tellg();
  BOOST_CHECK(length > 0);

  BOOST_TEST_MESSAGE(std::string("bion stream size: ") + toString(length));

  s.seekg(0, std::ios::beg);

  BionConsOutProcessor proc;
  BionReader<std::stringstream, BionConsOutProcessor> reader(s, proc);

  reader.process();
}
