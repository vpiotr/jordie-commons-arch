/////////////////////////////////////////////////////////////////////////////
// Name:        dataNodeTest.ipp
// Purpose:     Test data node type.
// Author:      Piotr Likus
// Modified by:
// Created:     24/05/2011
/////////////////////////////////////////////////////////////////////////////

#include <vector>

#include "base/btypes.h"
#include "base/algorithm.h"
#include "dtp/dnode.h"
#include "dtp/dnode_cast.h"
#include "dtp/dnode_algorithm.h"

using namespace base;
using namespace dtp;

// Functions that can be used:
//   BOOST_TEST_MESSAGE("balance-0: " << toString(balance));
//   BOOST_CHECK(balance == dec::decimal2(0));

BOOST_AUTO_TEST_CASE(test_list)
{
  // test implicit construction
  dnode listTest;
  listTest.addChild(new dnode(2));
  BOOST_CHECK(listTest.isList());
  BOOST_CHECK(listTest.isContainer());
  BOOST_CHECK(!listTest.empty());

  // test access by node reference (only parent & list)
  BOOST_CHECK(listTest[0].getAsInt() == 2);
  // test access by "getElement" - can be executed on any container, involves node obj copy
  BOOST_CHECK(listTest.getElement(0).getAsInt() == 2);

  // test universal access functions
  BOOST_CHECK(listTest.getElementType(0) == vt_int);

  // test access by "getNode" - can be executed on any container, allows to skip obj copy if not an array
  dnode helper;
  BOOST_CHECK(listTest.getNode(0, helper).getAsInt() == 2);
  helper.setAsInt(122);
  BOOST_CHECK(helper.getAsInt() == 122);

  // add using stl-like push_xxx
  dnode dnode1;
  dnode1.clear();
  dnode1.setAsList();
  for(int i=1; i <= 10; i++)
    dnode1.push_back(i);
  dnode1.push_front(133);
  BOOST_CHECK(dnode1.getInt(0) == 133);
  BOOST_CHECK(dnode1.size() == 11);
  helper.setAsInt(133);
  BOOST_CHECK(dnode1.indexOfValue(helper) == 0);

  // test construction using chain of addChild
  dnode listTest2a;
  listTest2a = dnode(ict_list);
  BOOST_CHECK(listTest2a.isList());

  listTest2a.addChild(0).
             addChild(1).
             addChild(2).
             addChild(3);

  BOOST_CHECK(listTest2a.size() == 4);
  BOOST_CHECK(listTest2a.getUInt(3) == 3);

  // test construction with constructor container type
  dnode listTest2(ict_list);
  BOOST_CHECK(listTest2.isList());
  for(int i=1; i <= 10; i++)
    listTest2.addItemAsInt(i);
  BOOST_CHECK(listTest2.size() == 10);

  // test erase of one element
  listTest2.eraseElement(3);
  BOOST_CHECK(listTest2.size() == 9);
  // test erase of tail of elements
  listTest2.eraseFrom(3);
  BOOST_CHECK(listTest2.size() == 3);

  // test resize, fill, replace, accumulate
  listTest2.resize(10);
  listTest2.fill<int>(0);
  BOOST_CHECK(listTest2.accumulate(0) == 0);

  listTest2.fill_n<int>(5, 10);
  BOOST_CHECK(listTest2.accumulate(0) == 5*10);

  listTest2.replace(0, 20);
  BOOST_CHECK(listTest2.accumulate(0) == (5*10 + 5*20));

}

