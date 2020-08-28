/////////////////////////////////////////////////////////////////////////////
// Name:        dataNodeTest.ipp
// Purpose:     Test data node type.
// Author:      Piotr Likus
// Modified by:
// Created:     24/05/2011
/////////////////////////////////////////////////////////////////////////////

//stl
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

BOOST_AUTO_TEST_CASE(array_processing)
{
  // construction using setAsArray
  dnode arrTest;
  //arrTest.setAsArray(vt_double);
  arrTest.setAsArray<double>();

  for(int i=1; i <= 10; i++)
    //arrTest.addItemAsDouble(static_cast<double>(i)/2.0);
    arrTest.addItem(static_cast<double>(i)/2.0);

  BOOST_CHECK(arrTest.getElementType(0) == vt_double);

  BOOST_CHECK(arrTest.isArray());
  BOOST_CHECK(arrTest.isContainer());
  BOOST_CHECK(!arrTest.empty());
  BOOST_CHECK(arrTest.size() == 10);

  float testConv = 1.0f;

  arrTest.addItem(testConv);
  arrTest.push_back(testConv);

  BOOST_CHECK(arrTest.size() == 12);

  // add using stl-like push_xxx
  arrTest.clear();
  //arrTest.setAsArray(vt_double);
  arrTest.setAsArray<double>();
  for(int i=1; i <= 10; i++)
    arrTest.push_back(static_cast<double>(i)/2.0);
  arrTest.push_front(1.33);
  BOOST_CHECK(arrTest.size() == 11);

  // add using chain of templates
  arrTest.clear();
  //arrTest.setAsArray(vt_double);
  arrTest.setAsArray<double>();
  arrTest.addItem(1.0).
          addItem(2.0).
          addItem(3.0).
          addItem(4.0);
  BOOST_CHECK(arrTest.size() == 4);

  // add using templates
  arrTest.clear();
  //arrTest.setAsArray(vt_double);
  arrTest.setAsArray<double>();
  for(int i=1; i <= 10; i++)
    arrTest.addItem<double>(static_cast<double>(i)/2.0);
  BOOST_CHECK(arrTest.size() == 10);

  // test item access by getXxx(int)
  BOOST_CHECK(arrTest.getDouble(3) > 1.0);
  // test item modification
  arrTest.setDouble(3, 0.1);
  // test item access by get<T>(int)
  BOOST_CHECK(arrTest.get<double>(3) < 1.0);

  // test access by "getElement" - can be executed on any container, involves node obj copy
  BOOST_CHECK(arrTest.getElement(3).getAsDouble() < 1.0);
  // test universal access functions
  BOOST_CHECK(arrTest.getElementType(3) == vt_double);

  // test access by "getNode" - can be executed on any container, allows to skip copy if not an array
  dnode helper;
  BOOST_CHECK(arrTest.getNode(3, helper).getAsDouble() < 1.0);

  // test erase of one element
  arrTest.eraseElement(3);
  BOOST_CHECK(arrTest.size() == 9);
  // test erase of tail of elements
  arrTest.eraseFrom(3);
  BOOST_CHECK(arrTest.size() == 3);

  // construction using special constructor argument
  dnode arrTest2(ict_array, vt_int);
  //dnode arrTest2(ict_array, art_int);
  //dnode arrTest2(ict_array, dnArrayTypeSelector<int>());
  //dnode arrTest2(ict_array, dnArrayTag(int));

  BOOST_CHECK(arrTest2.isArray());
  BOOST_CHECK(arrTest2.isContainer());
  BOOST_CHECK(arrTest2.empty());
  BOOST_CHECK(arrTest2.getArrayR()->getValueType() == vt_int);

  // test resize, fill, replace, accumulate
  arrTest2.resize(10);
  arrTest2.addItem(12);

  arrTest2.fill<int>(0);
  BOOST_CHECK(arrTest2.accumulate(0) == 0);

  arrTest2.fill_n<int>(5, 10);
  BOOST_CHECK(arrTest2.accumulate(0) == 5*10);

  arrTest2.replace(0, 20);
  BOOST_CHECK(arrTest2.accumulate(0) == (5*10 + 6*20));

#ifdef DATANODE_INIT_LIST
  // test initialization list
  dnode arrTest3({2,3,4,5,6,7});
  BOOST_CHECK(arrTest3.size() == 6);
  auto al = {10, 11, 12};
  dnode arrTest4(al);
  BOOST_CHECK(arrTest4.size() == 3);
#endif
}

