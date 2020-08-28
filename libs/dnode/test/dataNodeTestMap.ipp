/////////////////////////////////////////////////////////////////////////////
// Name:        dataNodeTestMap.ipp
// Purpose:     Test data node map
// Author:      Piotr Likus
// Modified by:
// Created:     13/06/2012
/////////////////////////////////////////////////////////////////////////////

#include <vector>

#include "base/btypes.h"
#include "base/algorithm.h"
#include "dtp/dnode_cast.h"
#include "dtp/dnode_algorithm.h"
#include "dtp/dnode_map.h"

using namespace base;
using namespace dtp;

// Functions that can be used:
//   BOOST_TEST_MESSAGE("balance-0: " << toString(balance));
//   BOOST_CHECK(balance == dec::decimal2(0));

template<typename KeyType, typename ValueType>
struct MapVisitor {
  void operator()(const KeyType &key, const ValueType &value)
  {
    BOOST_TEST_MESSAGE("key: " << toString(key));
    BOOST_TEST_MESSAGE("-> value: " << toString(value));
  }
};


BOOST_AUTO_TEST_CASE(test_map)
{
  dnode map1;
  dmap_init<int, double>(map1);
  dmap_insert(map1, 123, 3.4);
  dmap_insert(map1, 10, 1.4);
  dmap_insert(map1, 5, 14.0);
  BOOST_REQUIRE_THROW(dmap_insert(map1, 10, 24.0), dnError); // should fail
  dmap_insert(map1, 10, 24.0, true); // won't fail
  dmap_set(map1, 5, 3.1415);
  double val = dmap_get_def(map1, 5, 1.0);
  BOOST_CHECK((val >= 3.0) && (val <= 3.2));
  dnode::iterator it = dmap_find(map1, 5);
  BOOST_CHECK(it != map1.end());
  dmap_erase(map1, 123);
  it = dmap_find(map1, 123);
  BOOST_CHECK(it == dmap_end<int>(map1));
  dmap_visit<int, double>(map1, MapVisitor<int, double>());
  dmap_insert(map1, 100, 4.0);
  dmap_insert(map1, 7, 1.41);
  BOOST_TEST_MESSAGE("-- after some inserts:");
  dmap_for_each<int, double>(map1, MapVisitor<int, double>());
  BOOST_CHECK(dmap_is_sorted<int>(map1));
  BOOST_CHECK(dmap_size<int>(map1) == 5);

  // test dmap_get_all
  dnode get_all_res;
  dmap_get_all<int, double>(map1, 10, get_all_res);
  BOOST_CHECK(get_all_res.size() == 2);
}

BOOST_AUTO_TEST_CASE(test_map2)
{
  dnode map1;
  dmap_init<int, double>(map1);

#ifdef DMAP_SORT_SUPPORTED
  dmap_push_back(map1, 123, 3.4);
  dmap_push_back(map1, 10, 1.4);
  dmap_push_back(map1, 5, 14.0);
  dmap_push_back(map1, 100, 4.0);
  dmap_push_back(map1, 7, 1.41);
  BOOST_CHECK(dmap_size<int>(map1) == 5);
  dmap_sort<int>(map1);
#endif

  BOOST_CHECK(dmap_is_sorted<int>(map1));

  dmap_visit<int, double>(map1, MapVisitor<int, double>());
}

BOOST_AUTO_TEST_CASE(test_map3)
{
  dnode map1;
  dmap_init<int, double>(map1);
#ifdef DMAP_SORT_SUPPORTED
  for(uint i=0; i < 1000; i++)
    dmap_push_back(map1, rand()%1000, 3.4+double(i));
  dmap_sort<int>(map1);
#endif
  BOOST_CHECK(dmap_is_sorted<int>(map1));
}
