/////////////////////////////////////////////////////////////////////////////
// Name:        dataNodeTestParent.ipp
// Purpose:     Test data node type.
// Author:      Piotr Likus
// Modified by:
// Created:     24/05/2011
/////////////////////////////////////////////////////////////////////////////

#include <vector>

#include "base/btypes.h"
#include "base/algorithm.h"
#include "dtp/dnode_cast.h"
#include "dtp/dnode_algorithm.h"

using namespace dtp;
using namespace base;

// Functions that can be used:
//   BOOST_TEST_MESSAGE("balance-0: " << toString(balance));
//   BOOST_CHECK(balance == dec::decimal2(0));

BOOST_AUTO_TEST_CASE(test_parent)
{
  dnode test;

  BOOST_CHECK(test.isNull());
  BOOST_CHECK(!test.isParent());
  BOOST_CHECK(!test.isList());
  BOOST_CHECK(!test.isContainer());
  BOOST_CHECK(test.size() == 0);
  BOOST_CHECK(test.empty());

  test.addChild("testme", new dnode(2));
  BOOST_CHECK(test.hasChild("testme"));
  BOOST_CHECK(test.getInt("testme") == 2);
  BOOST_CHECK(test["testme"].getAsInt() == 2);
  BOOST_CHECK(!test.getElementName(0).empty());

  test["testme"] = 3;
  BOOST_CHECK(test["testme"].getAsInt() == 3);

  BOOST_CHECK(!test.isNull());
  BOOST_CHECK(test.isParent());
  BOOST_CHECK(!test.isList());
  BOOST_CHECK(test.isContainer());
  BOOST_CHECK(test.size()>0);
  BOOST_CHECK(!test.empty());

  // add using stl-like push_xxx
  dnode dnode1;
  dnode1.clear();
  dnode1.setAsParent();
  for(int i=1; i <= 10; i++)
    dnode1.push_back(i);
  dnode1.push_front(133);
  dnode1.push_back("a1", 122);
  dnode1.push_front("a2", 143);
  BOOST_CHECK(dnode1.size() == 13);

  // other tests
  test.addChild("second", new dnode(true));
  BOOST_CHECK(test.size() == 2);
  BOOST_CHECK(test.getBool("second") == true);

  // test chained addChild
  test.addChild("third", false).
       addChild("forth", 2.12).
       addChild("a1", "test");

  BOOST_CHECK(test.size() == 5);

  // test access by node reference (only parent & list)
  BOOST_CHECK(test[0].getAsInt() == 3);

  // test access by node.getXxxx(index)
  BOOST_CHECK(test.getInt(0) == 3);
  BOOST_CHECK(!test.getBool(2));

  // test access by node.getXxxx(name)
  BOOST_CHECK(test.getInt("testme") == 3);
  BOOST_CHECK(!test.getBool("third"));

  // test access by template
  BOOST_CHECK(test.get<int>("testme") == 3);
  BOOST_CHECK(test.get<int>(0) == 3);
  BOOST_CHECK(!test.getElement(2).getAs<bool>());
  BOOST_CHECK(test.get<float>("forth") > 2.0);
  BOOST_CHECK(test.get<dtpString>("a1") == dtpString("test"));

  // test auto-conversion
  BOOST_CHECK(test.get<dtpString>(0) == dtpString("3"));
  BOOST_CHECK(test.get<uint>(0) == 3);
  BOOST_CHECK(test.get<byte>(0) == 3);

  // test reading missing value
  BOOST_CHECK(test.get<int>("non-existing-value", 123) == 123);

  // test access by "getElement" - can be executed on any container, involves node obj copy
  BOOST_CHECK(test.getElement(1).getAsBool() == true);
  BOOST_CHECK(!test.getElement(2).getAsBool());

  // test access by "getNode" - can be executed on any container, allows to skip copy if not an array
  dnode helper;
  BOOST_CHECK(test.getNode("testme", helper).getAsInt() == 3);

  // test access by "peekNode" - can be executed on any container, allows checking if element is missing on read
  dnode *helperPtr;
  helperPtr = test.peekChild("missing-item");
  BOOST_CHECK(helperPtr == DTP_NULL);

  helperPtr = test.peekChild("testme");
  BOOST_CHECK(helperPtr != DTP_NULL);

  // test element type
  BOOST_CHECK(test.getElementType(0) == vt_int);
  BOOST_CHECK(test.getElementType(1) == vt_bool);

  // test element names
  BOOST_CHECK(test.getElementName(1) == dtpString("second"));
  BOOST_CHECK(test.getElementName(2) == dtpString("third"));
  BOOST_CHECK(test.getElementName(3) == dtpString("forth"));

  // test "safe" functions
  BOOST_CHECK(!test.getElementSafe("nini", helper));
  helper = dnode(134);
  dnode::size_type expectedNiniIndex = test.size();
  test.setElementSafe("nini", helper);
  BOOST_CHECK(test.hasChild("nini"));
  BOOST_CHECK(test.getInt("nini") == 134);

  // test find by name
  int idx = test.indexOfName("nini");
  BOOST_CHECK(idx == expectedNiniIndex);

  // test extract
  BOOST_CHECK(test.hasChild("nini"));
  std::auto_ptr<dnode> niniGuard(test.extractChild(idx));
  BOOST_CHECK(!test.hasChild("nini"));

  // test items are sorted by order of insertion
  dnode intParent(ict_parent);
  for(int i=0; i <= 10; i++)
    intParent.addElement(toString(i), dnode(i));

  for(int i=0; i <= 10; i++) {
    BOOST_CHECK(intParent.getInt(i) == i);
    BOOST_CHECK(intParent.getInt(toString(i)) == i);
  }

  // test auto-add
  intParent["test_autoadd"] = 123;
  BOOST_CHECK(intParent.hasChild("test_autoadd"));

  intParent.set("test_autoadd", 13);
  BOOST_CHECK(intParent.get<int>("test_autoadd") == 13);

  // test resize, fill, replace, accumulate
  intParent.resize(10);
  intParent.fill<int>(0);
  BOOST_CHECK(intParent.accumulate(0) == 0);

  intParent.fill_n<int>(5, 10);
  BOOST_CHECK(intParent.accumulate(0) == 5*10);

  intParent.replace(0, 20);
  BOOST_CHECK(intParent.accumulate(0) == (5*10 + 5*20));
}

