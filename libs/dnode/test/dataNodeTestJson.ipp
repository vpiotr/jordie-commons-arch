/////////////////////////////////////////////////////////////////////////////
// Name:        dataNodeTestJson.ipp
// Purpose:     Test data node type.
// Author:      Piotr Likus
// Modified by:
// Created:     03/09/2012
/////////////////////////////////////////////////////////////////////////////

#include <vector>

#include "base/btypes.h"
#include "dtp/dnode.h"
#include "dtp/dnode_serializer.h"

using namespace dtp;

BOOST_AUTO_TEST_CASE(test_json)
{
  dnode test;

  test.addChild("test_value", new dnode(2));

  dnode dnode1;
  dnode1.setAsParent();
  for(int i=1; i <= 10; i++)
    dnode1.push_back(i);

  test.addChild("test_parent", dnode1);

  dtpString str;
  dnSerializer serializer;
  serializer.convToString(test, str);

  BOOST_TEST_MESSAGE("JSON form of test node: " << str);

  BOOST_CHECK(!str.empty());
  BOOST_CHECK(str.find("test_value") != str.npos);
  BOOST_CHECK(str.find("test_parent") != str.npos);

  dnode testImported;

  serializer.convFromString(str, testImported);

  BOOST_TEST_MESSAGE("Dump of imported node: " << testImported.dump());

  BOOST_CHECK(!testImported.empty());
  BOOST_CHECK(testImported.hasChild("test_value"));
  BOOST_CHECK(testImported.hasChild("test_parent"));
  BOOST_CHECK(testImported["test_parent"].size() == 10);
  BOOST_CHECK(testImported["test_parent"].accumulate(0) > 0);
  BOOST_CHECK(testImported["test_value"].getAs<int>() == 2);
}
