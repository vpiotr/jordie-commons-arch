/////////////////////////////////////////////////////////////////////////////
// Name:        varintTest.ipp
// Project:     dtpLib
// Purpose:     Tests for varint
// Author:      Piotr Likus
// Modified by:
// Created:     23/06/2013
/////////////////////////////////////////////////////////////////////////////


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

