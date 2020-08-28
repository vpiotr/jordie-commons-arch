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

BOOST_AUTO_TEST_CASE(universal_iterators)
{
  dnode arrTest(ict_array, vt_int);
  for(int i=1; i <= 10; i++)
    arrTest.addItem(i);

  BOOST_CHECK(arrTest.isArray());
  BOOST_CHECK(arrTest.isContainer());
  BOOST_CHECK(!arrTest.empty());

  // test access with universal iterators returned by at()
  BOOST_CHECK(arrTest.at(1)->getAsInt() == 2);
  arrTest.at(1)->setAsInt(123);
  BOOST_CHECK(arrTest.at(1)->getAsInt() != 2);
  arrTest.at(1)->setAs(125);
  BOOST_CHECK(arrTest.at(1)->getAs<int>() == 125);

  // test loop with universal iterators
  bool found3 = false;
  for(dnode::const_iterator it = arrTest.begin(), epos = arrTest.end(); it != epos; ++it)
    if (it->getAsInt() == 3)
      found3 = true;
  BOOST_CHECK(found3);

  // test loop with universal iterators with template access
  bool found4 = false;
  for(dnode::const_iterator it = arrTest.begin(), epos = arrTest.end(); it != epos; ++it)
    if (it->getAs<int>() == 3)
      found4 = true;
  BOOST_CHECK(found4);

  // test find by find()
  bool found5 = (arrTest.find(3) != arrTest.end());
  BOOST_CHECK(found5);
}

