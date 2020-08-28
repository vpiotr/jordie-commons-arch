/////////////////////////////////////////////////////////////////////////////
// Name:        bionNodeTest.ipp
// Project:     dtpLib
// Purpose:     Tests for BION support for dnode.
// Author:      Piotr Likus
// Modified by:
// Created:     23/06/2013
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file FileName.h
\brief Short file description

Long description
*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "dtp/dnode_bion.h"
#include "base/varint.h"

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

BOOST_AUTO_TEST_CASE(bion_dnode_test_scalar)
{
  std::stringstream s;

  dnBionWriter<std::stringstream> writer(s);

  dnode testNode1(314);
  dnode testNode2;

  writer.write(testNode1);

  s.seekg(0, s.end);
  size_t length = s.tellg();
  BOOST_CHECK(length > 0);

  BOOST_TEST_MESSAGE(std::string("bion stream size: ") + toString(length));

  s.seekg(0, std::ios::beg);

  dnBionProcessor proc(testNode2);

  BionReader<std::stringstream, dnBionProcessor> reader(s, proc);

  reader.process();

  BOOST_CHECK(testNode1 == testNode2);
}

BOOST_AUTO_TEST_CASE(bion_dnode_test_array)
{
  std::stringstream s;

  dnBionWriter<std::stringstream> writer(s);

  dnode testNode1(ict_array, vt_int);
  dnode testNode2;

  for(uint i=0, epos = 100; i != epos; i++)
    testNode1.addItem(i);

  writer.write(testNode1);

  s.seekg(0, s.end);
  size_t length = s.tellg();
  BOOST_CHECK(length > 0);

  BOOST_TEST_MESSAGE(std::string("bion stream size: ") + toString(length));

  s.seekg(0, std::ios::beg);

  dnBionProcessor proc(testNode2);

  BionReader<std::stringstream, dnBionProcessor> reader(s, proc);

  reader.process();

  BOOST_CHECK(testNode2.isArray());
  BOOST_CHECK(testNode1.size() == testNode2.size());
  BOOST_CHECK(testNode1.accumulate(0) == testNode2.accumulate(0));
}


BOOST_AUTO_TEST_CASE(bion_dnode_test_parent)
{
  std::stringstream s;

  dnBionWriter<std::stringstream> writer(s);

  dnode testNode1(ict_parent);
  dnode testNode2;

  for(uint i=0, epos = 100; i != epos; i++)
    testNode1.addChild(toString(i), i);

  writer.write(testNode1);

  s.seekg(0, s.end);
  size_t length = s.tellg();
  BOOST_CHECK(length > 0);

  BOOST_TEST_MESSAGE(std::string("bion stream size: ") + toString(length));

  s.seekg(0, std::ios::beg);

  dnBionProcessor proc(testNode2);

  BionReader<std::stringstream, dnBionProcessor> reader(s, proc);

  reader.process();

  BOOST_CHECK(testNode2.isParent());
  BOOST_CHECK(testNode1.size() == testNode2.size());
  BOOST_CHECK(testNode1.accumulate(0) == testNode2.accumulate(0));
}

BOOST_AUTO_TEST_CASE(bion_dnode_test_list)
{
  std::stringstream s;

  dnBionWriter<std::stringstream> writer(s);

  dnode testNode1(ict_list);
  dnode testNode2;

  for(uint i=0, epos = 100; i != epos; i++)
    testNode1.addChild(i);

  writer.write(testNode1);

  s.seekg(0, s.end);
  size_t length = s.tellg();
  BOOST_CHECK(length > 0);

  BOOST_TEST_MESSAGE(std::string("bion stream size: ") + toString(length));

  s.seekg(0, std::ios::beg);

  dnBionProcessor proc(testNode2);

  BionReader<std::stringstream, dnBionProcessor> reader(s, proc);

  reader.process();

  BOOST_CHECK(testNode2.isList());
  BOOST_CHECK(testNode1.size() == testNode2.size());
  BOOST_CHECK(testNode1.accumulate(0) == testNode2.accumulate(0));
}

BOOST_AUTO_TEST_CASE(varint_test)
{
  char valbuffer[6];
  uint value, value1;
  size_t writtenSize, readSize;

  value = 14;
  writtenSize = varint_encode(value, valbuffer, sizeof(valbuffer));
  readSize = varint_decode(valbuffer, sizeof(valbuffer), value1);
  BOOST_CHECK(writtenSize > 0);
  BOOST_CHECK(readSize == writtenSize);
  BOOST_CHECK(value = value1);

  value = 128;
  writtenSize = varint_encode(value, valbuffer, sizeof(valbuffer));
  readSize = varint_decode(valbuffer, sizeof(valbuffer), value1);
  BOOST_CHECK(writtenSize > 0);
  BOOST_CHECK(readSize == writtenSize);
  BOOST_CHECK(value = value1);

  value = 256;
  writtenSize = varint_encode(value, valbuffer, sizeof(valbuffer));
  readSize = varint_decode(valbuffer, sizeof(valbuffer), value1);
  BOOST_CHECK(writtenSize > 0);
  BOOST_CHECK(readSize == writtenSize);
  BOOST_CHECK(value = value1);

  value = 1114;
  writtenSize = varint_encode(value, valbuffer, sizeof(valbuffer));
  readSize = varint_decode(valbuffer, sizeof(valbuffer), value1);
  BOOST_CHECK(writtenSize > 0);
  BOOST_CHECK(readSize == writtenSize);
  BOOST_CHECK(value = value1);

  value = 11140;
  writtenSize = varint_encode(value, valbuffer, sizeof(valbuffer));
  readSize = varint_decode(valbuffer, sizeof(valbuffer), value1);
  BOOST_CHECK(writtenSize > 0);
  BOOST_CHECK(readSize == writtenSize);
  BOOST_CHECK(value = value1);

  value = 111400;
  writtenSize = varint_encode(value, valbuffer, sizeof(valbuffer));
  readSize = varint_decode(valbuffer, sizeof(valbuffer), value1);
  BOOST_CHECK(writtenSize > 0);
  BOOST_CHECK(readSize == writtenSize);
  BOOST_CHECK(value = value1);
}

