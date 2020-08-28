/////////////////////////////////////////////////////////////////////////////
// Name:        dataNodeTestAlgs.ipp
// Purpose:     Test data node alorithms
// Author:      Piotr Likus
// Modified by:
// Created:     02/05/2012
/////////////////////////////////////////////////////////////////////////////

#include <ctime>
#include <vector>

#include "base/btypes.h"
#include "base/algorithm.h"
#include "dtp/dnode_cast.h"
#include "dtp/dnode_algorithm.h"

using namespace base;
using namespace dtp;

//const int vt_list = 1000;

// Functions that can be used:
//   BOOST_TEST_MESSAGE("balance-0: " << toString(balance));
//   BOOST_CHECK(balance == dec::decimal2(0));

bool checkAllFoundBinSrch(dnode &node)
{
  bool allFound = true;
  int a;
  for(int i=1; i <= 100; i++)
  {
    a = (i + 13) % 100;
    if (!node.binary_search<int>(a))
    {
      BOOST_TEST_MESSAGE("value not found: " << toString(a));
      BOOST_CHECK(a != a);
      allFound = false;
      break;
    }
  }

  return allFound;
}

bool checkAllFoundSeq(dnode &node)
{
  bool allFound = true;
  int a;
  for(int i=1; i <= 100; i++)
  {
    a = (i + 13) % 100;
    if (node.indexOfValue(a) == dnode::npos)
    {
      BOOST_TEST_MESSAGE("value not found: " << toString(a));
      BOOST_CHECK(a != a);
      allFound = false;
      break;
    }
  }

  return allFound;
}

bool checkIsSortedAsc(dnode &node)
{
  return node.is_sorted<int>();
}

void initContainer(dnode &node, int nodeType)
{
  switch (nodeType) {
    case vt_array: {
      //node.setAsArray(vt_int);
      node.setAsArray<int>();
      break;
    }
    case vt_parent: {
      node.setAsParent();
      break;
    }
    case vt_list: {
      node.setAsList();
      break;
    }
    default: {
      BOOST_CHECK_MESSAGE(false, "unknown node type");
    }
  }
}

void fillContainer(dnode &node, int nodeType, int n, int nmod)
{
  initContainer(node, nodeType);
  for(int i=1; i <= n; i++) {
    node.push_back((i + 13) % nmod);
  }
}

void fillContainer(dnode &node, int nodeType, int n)
{
  initContainer(node, nodeType);

  for(int i=1; i <= n; i++) {
    node.push_back(i);
  }
}

BOOST_AUTO_TEST_CASE(test_search)
{
  int nodeTypeArr[] = {vt_array, vt_parent, vt_list};
  int nodeType;
  size_t nodeTypeCount = sizeof(nodeTypeArr) / sizeof(nodeTypeArr[0]);

  for(uint iType = 0; iType < nodeTypeCount; iType++)
  {
    nodeType = nodeTypeArr[iType];
    BOOST_TEST_MESSAGE(dtpString("node type: ") + toString(nodeType));

  //--- test unique array search
  dnode arrTest;
  fillContainer(arrTest, nodeType, 100, 100);

  bool allFound;

  BOOST_CHECK(checkAllFoundSeq(arrTest));

  //--- check find
  BOOST_CHECK(arrTest.find(1) != arrTest.end());

  //--- check failed find
  BOOST_CHECK(arrTest.find(120) == arrTest.end());

  arrTest.sort<int>();

  BOOST_CHECK(checkIsSortedAsc(arrTest));

  BOOST_CHECK(checkAllFoundBinSrch(arrTest));

  int a;

  //--- check binary_search
  BOOST_CHECK(arrTest.binary_search<int>(1));

  //--- check failed binary_search
  BOOST_CHECK(!arrTest.binary_search<int>(120));

  //--- check search with duplicates
  arrTest.clear();
  fillContainer(arrTest, nodeType, 100, 10);

  arrTest.sort<int>();

  allFound = true;
  for(int i=1; i <= 100; i++)
  {
    a = (i + 13) % 10;
    if (!arrTest.binary_search<int>(a))
      allFound = false;
  }

  BOOST_CHECK(allFound);

  } // for iType
} // test case

BOOST_AUTO_TEST_CASE(test_search_stl)
{
  int nodeTypeArr[] = {vt_array, vt_parent, vt_list};
  int nodeType;
  size_t nodeTypeCount = sizeof(nodeTypeArr) / sizeof(nodeTypeArr[0]);

  for(uint iType = 0; iType < nodeTypeCount; iType++)
  {
    nodeType = nodeTypeArr[iType];
    BOOST_TEST_MESSAGE(dtpString("node type: ") + toString(nodeType));

  dnode arrTest;
  fillContainer(arrTest, nodeType, 100, 100);

  //--- test unique array search
  bool allFound;

  BOOST_CHECK(checkAllFoundSeq(arrTest));

  //--- check find
  BOOST_CHECK(std::find(arrTest.scalarBegin<int>(), arrTest.scalarEnd<int>(), 1) != arrTest.scalarEnd<int>());

  //--- check failed find
  BOOST_CHECK(std::find(arrTest.scalarBegin<int>(), arrTest.scalarEnd<int>(), 120) == arrTest.scalarEnd<int>());

  // sort
  arrTest.sort<int>();

  BOOST_CHECK(checkIsSortedAsc(arrTest));

  int a;

  //--- check binary_search
  BOOST_CHECK(std::binary_search(arrTest.scalarBegin<int>(), arrTest.scalarEnd<int>(), 1));

  //--- check failed binary_search
  BOOST_CHECK(!std::binary_search(arrTest.scalarBegin<int>(), arrTest.scalarEnd<int>(), 120));

  //--- check search with duplicates
  arrTest.clear();

  fillContainer(arrTest, nodeType, 100, 10);

  arrTest.sort<int>();

  allFound = true;
  for(int i=1; i <= 100; i++)
  {
    a = (i + 13) % 10;
    if (!std::binary_search(arrTest.scalarBegin<int>(), arrTest.scalarEnd<int>(), a))
      allFound = false;
  }

  BOOST_CHECK(allFound);

  } // for iType
} // test case

BOOST_AUTO_TEST_CASE(test_count)
{
  int nodeTypeArr[] = {vt_array, vt_parent, vt_list};
  int nodeType;
  size_t nodeTypeCount = sizeof(nodeTypeArr) / sizeof(nodeTypeArr[0]);

  for(uint iType = 0; iType < nodeTypeCount; iType++)
  {
    nodeType = nodeTypeArr[iType];
    BOOST_TEST_MESSAGE(dtpString("node type: ") + toString(nodeType));

  dnode arrTest;
  fillContainer(arrTest, nodeType, 100);

  BOOST_CHECK(arrTest.count(1) == 1);
  int val = 1;
  BOOST_CHECK(arrTest.count_if<int>(std::bind1st(std::equal_to<int>(),val)) == arrTest.count(val));

  } // for iType
}

BOOST_AUTO_TEST_CASE(test_minmax)
{
  int nodeTypeArr[] = {vt_array, vt_parent, vt_list};
  int nodeType;
  size_t nodeTypeCount = sizeof(nodeTypeArr) / sizeof(nodeTypeArr[0]);

  for(uint iType = 0; iType < nodeTypeCount; iType++)
  {
    nodeType = nodeTypeArr[iType];
    BOOST_TEST_MESSAGE(dtpString("node type: ") + toString(nodeType));

  dnode arrTest;
  fillContainer(arrTest, nodeType, 100);

  dnode::iterator it;

  it = arrTest.min_element<int>();
  BOOST_CHECK(it->getAs<int>() == 1);

  it = arrTest.max_element<int>();
  BOOST_CHECK(it->getAs<int>() == 100);

#ifdef DTP_CPP11
  auto p = arrTest.minmax_element<int>();
#else
  std::pair<dnode::iterator, dnode::iterator> p = arrTest.minmax_element<int>();
#endif
  BOOST_CHECK(p.first->getAs<int>() == 1);
  BOOST_CHECK(p.second->getAs<int>() == 100);

  } // for iType
}

BOOST_AUTO_TEST_CASE(test_lowerupper)
{
  dnode arrTest;
  arrTest.setAsArray(vt_int);
  arrTest.addItem(10)
         .addItem(10)
         .addItem(10)
         .addItem(20)
         .addItem(20)
         .addItem(20)
         .addItem(30)
         .addItem(30);

  dnode::iterator it;
  it = arrTest.lower_bound(20);
  BOOST_CHECK((int)(it - arrTest.begin()) == 3);

  it = arrTest.upper_bound(20);
  BOOST_CHECK((int)(it - arrTest.begin()) == 6);

  arrTest.insert(it, 23);
  it = arrTest.lower_bound(23);
  BOOST_CHECK((int)(it - arrTest.begin()) == 6);
  BOOST_CHECK(arrTest.size() == 9);
}

struct IntGtScanner {
  IntGtScanner(int value): m_limit(value) {}
  bool operator()(int a) {
    return (a > m_limit);
  }
protected:
  int m_limit;
};

BOOST_AUTO_TEST_CASE(test_scan)
{
  dnode arrTest;
  arrTest.setAsArray(vt_int);
  arrTest.addItem(10)
         .addItem(10)
         .addItem(30)
         .addItem(10)
         .addItem(25)
         .addItem(21)
         .addItem(20)
         .addItem(30);

  dnode::iterator it;

  //BOOST_CHECK(arrTest.scanVectorValues<int>(IntGtScanner(25)));
  //BOOST_CHECK(arrTest.scanVectorValues<int>(std::bind1st(std::equal_to<int>(),21)));
  //BOOST_CHECK(!arrTest.scanVectorValues<int>(std::bind2nd(std::greater<int>(),30)));
}

BOOST_AUTO_TEST_CASE(test_alg_accumulate)
{
  int nodeTypeArr[] = {vt_array, vt_parent, vt_list};
  int nodeType;
  size_t nodeTypeCount = sizeof(nodeTypeArr) / sizeof(nodeTypeArr[0]);

  for(uint iType = 0; iType < nodeTypeCount; iType++)
  {
    nodeType = nodeTypeArr[iType];
    BOOST_TEST_MESSAGE(dtpString("node type: ") + toString(nodeType));

    dnode arrTest;
    fillContainer(arrTest, nodeType, 100);

    int sumNode = arrTest.accumulate(0);
    int sumStl = std::accumulate(arrTest.scalarBegin<int>(), arrTest.scalarEnd<int>(), 0);
    BOOST_CHECK(sumNode == sumStl);
    BOOST_CHECK(sumNode > (*arrTest.scalarBegin<int>()));

  } // for iType
}

struct IntAdder {
  IntAdder(): m_sum(0) {}

  void operator()(int a) {
    m_sum += a;
  }

  operator int() const {
    return m_sum;
  }
protected:
  int m_sum;
};

BOOST_AUTO_TEST_CASE(test_alg_for_each)
{

  int nodeTypeArr[] = {vt_array, vt_parent, vt_list};
  int nodeType;
  size_t nodeTypeCount = sizeof(nodeTypeArr) / sizeof(nodeTypeArr[0]);

  for(uint iType = 0; iType < nodeTypeCount; iType++)
  {
    nodeType = nodeTypeArr[iType];
    BOOST_TEST_MESSAGE(dtpString("node type: ") + toString(nodeType));

    dnode arrTest;
    fillContainer(arrTest, nodeType, 100);

    IntAdder fa;
    IntAdder fb;

    int sumNode = arrTest.for_each<int, IntAdder>(fb);
    int sumStl = std::for_each(arrTest.scalarBegin<int>(), arrTest.scalarEnd<int>(), fa);

    BOOST_CHECK(sumNode == sumStl);
    BOOST_CHECK(sumNode > (*arrTest.scalarBegin<int>()));
  } // for iType
}

BOOST_AUTO_TEST_CASE(test_alg_replace)
{
  int nodeTypeArr[] = {vt_array, vt_parent, vt_list};
  int nodeType;
  size_t nodeTypeCount = sizeof(nodeTypeArr) / sizeof(nodeTypeArr[0]);

  for(uint iType = 0; iType < nodeTypeCount; iType++)
  {
    nodeType = nodeTypeArr[iType];
    BOOST_TEST_MESSAGE(dtpString("node type: ") + toString(nodeType));

  dnode arrTest;
  fillContainer(arrTest, nodeType, 100);

  BOOST_CHECK(arrTest.count(1) == 1);
  BOOST_CHECK(arrTest.count(777) == 0);
  arrTest.replace(1, 777);
  BOOST_CHECK(arrTest.count(1) == 0);
  BOOST_CHECK(arrTest.count(777) > 0);

  } // for iType
}

template <class T>
struct IntMult2 {
  T operator()(T value) {
    return value * 2;
  }
};

BOOST_AUTO_TEST_CASE(test_alg_transform)
{
  int nodeTypeArr[] = {vt_array, vt_parent, vt_list};
  int nodeType;
  size_t nodeTypeCount = sizeof(nodeTypeArr) / sizeof(nodeTypeArr[0]);

  for(uint iType = 0; iType < nodeTypeCount; iType++)
  {
    nodeType = nodeTypeArr[iType];
    BOOST_TEST_MESSAGE(dtpString("node type: ") + toString(nodeType));

  dnode arrTest;
  fillContainer(arrTest, nodeType, 100);

  int sumNode = arrTest.accumulate(0);

  BOOST_CHECK(arrTest.count(1) == 1);

  IntMult2<int> op;
  arrTest.transform<int>(op);
  arrTest.transform<int>(arrTest.scalarBegin<int>(), op);

  int sumNode2 = arrTest.accumulate(0);

  BOOST_CHECK(arrTest.count(1) == 0);
  BOOST_CHECK(sumNode2 == sumNode * 4);

  } // for iType
}

template <class T>
struct RandomValGenerator {
  T operator()() {
    return rand() % 100;
  }
};

BOOST_AUTO_TEST_CASE(test_alg_generate)
{
  int nodeTypeArr[] = {vt_array, vt_parent, vt_list};
  int nodeType;
  size_t nodeTypeCount = sizeof(nodeTypeArr) / sizeof(nodeTypeArr[0]);

  for(uint iType = 0; iType < nodeTypeCount; iType++)
  {
    nodeType = nodeTypeArr[iType];
    BOOST_TEST_MESSAGE(dtpString("node type: ") + toString(nodeType));

    dnode arrTest;
    initContainer(arrTest, nodeType);
    arrTest.resize(100);

    srand ( time(NULL) );

    RandomValGenerator<int> r;
    arrTest.generate<int>(r);

#ifdef DTP_CPP11
    auto p = arrTest.minmax_element<int>();
#else
    std::pair<dnode::iterator, dnode::iterator> p = arrTest.minmax_element<int>();
#endif

    int minv = p.first->getAs<int>();
    int maxv = p.second->getAs<int>();

    BOOST_CHECK(minv != maxv);

  } // for iType
}

BOOST_AUTO_TEST_CASE(test_alg_all)
{

  dnode node(ict_array, vt_int);
  node.resize(10);
  node.fill(123);

  BOOST_CHECK(node.accumulate(0) == 1230);
  BOOST_CHECK(node.all_of<int>(std::bind1st(std::equal_to<int>(),123)));
  BOOST_CHECK(node.none_of<int>(std::bind1st(std::greater<int>(),123)));
  BOOST_CHECK(node.none_of<int>(std::bind1st(std::less<int>(),123)));

  node.set(9, 11);
  BOOST_CHECK(node.any_of<int>(std::bind1st(std::equal_to<int>(),11)));
}

BOOST_AUTO_TEST_CASE(test_alg_unique)
{
  int nodeTypeArr[] = {vt_array, vt_parent, vt_list};
  int nodeType;
  size_t nodeTypeCount = sizeof(nodeTypeArr) / sizeof(nodeTypeArr[0]);

  for(uint iType = 0; iType < nodeTypeCount; iType++)
  {
      nodeType = nodeTypeArr[iType];
      BOOST_TEST_MESSAGE(dtpString("node type: ") + toString(nodeType));

      dnode arrTest;
      fillContainer(arrTest, nodeType, 10, 5);

      int cnt1 = arrTest.size();
      arrTest.sort<int>();
      arrTest.unique();
      int cnt2 = arrTest.size();

      BOOST_CHECK(cnt1 > cnt2);
      BOOST_CHECK(cnt2 > 0);
  } // for iType
}
