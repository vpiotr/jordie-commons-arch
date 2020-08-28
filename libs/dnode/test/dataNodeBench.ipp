/////////////////////////////////////////////////////////////////////////////
// Name:        dataNodeBench.ipp
// Purpose:     Test data node performance.
// Author:      Piotr Likus
// Modified by:
// Created:     26/05/2011
/////////////////////////////////////////////////////////////////////////////

// test performance
// compare container & access modes for operations
// operations: insert, read, update, delete, find, sort, sum, size
// containers:
// + stl: vector, list, map,
// - stl2: unordered_map, set
// + data-value-vector
// + data-node: parent, list, array
// access modes for data node:
// + dnode_parent_val_by_idx
// + dnode_list_val_by_idx
// + dnode_array_val_by_idx
// + dnode_parent_ref
// + dnode_parent_elem
// + dnode_parent_qref
// + dnode_parent_stl
// + dnode_list_stl
// + dnode_array_dbl_val
// + dnode_array_dbl_stl
// operations:
// - insert: insert k elements with value (i % 10) w/ or w/o names, eliminate name generation overhead
// - accum: sum elements
// - update: make all elements v = v * 2
// - delete: while !empty delete first item
// - find: sum all values found by name or value - depending on container type
// - size: for i=1 to k add size(container) as item

// std
#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include <numeric>

//boost
#include <boost/bind.hpp>
#include <boost/any.hpp>

//base
#include "base/algorithm.h"
#include "base/string.h"

//sc
#include "dtp/details/dtypes.h"
#include "dtp/dnode_cast.h"
#include "dtp/dnode_algorithm.h"
#include "dtp/dnode_map.h"
#include "dtp/details/bin_search.h"
#include "dtp/dnode_serializer.h"
#include "dtp/dnode_bion.h"

#include "perf/Timer.h"

using namespace dtp;
using namespace perf;
using namespace base;

//#define TEST_BENCH_BASE // define if you want to run basic performance tests
//#define TEST_BENCH_IT // define if you want to run iterator performance tests
//#define TEST_BENCH_VECT // define if you want to run vector performance tests
//#define TEST_BENCH_SEL
//#define TEST_BENCH_MAP_SORT

// constants
const int ITEM_COUNT_DEBUG = 100;
const int ITEM_COUNT_RELEASE = 100000; //x10

#ifdef _DEBUG
const int ITEM_COUNT = ITEM_COUNT_DEBUG;
const int REPEAT_COUNT = 2;
#elif BENCH_PROFILER
const int ITEM_COUNT = ITEM_COUNT_RELEASE;
const int REPEAT_COUNT = 10;
#else
const int ITEM_COUNT = ITEM_COUNT_RELEASE;
const int REPEAT_COUNT = 10;
#endif
const int DELETE_DIV = 100;
const int SIZE_DIV = 1;
const int FIND_DIV = 10;
const int SERIALIZE_REPEAT_COUNT = REPEAT_COUNT;

// Functions that can be used:
//   BOOST_TEST_MESSAGE("balance-0: " << toString(balance));
//   BOOST_CHECK(balance == dec::decimal2(0));

//template <typename T>
//std::string toString(const T &arg) {
//  std::ostringstream	out;
//  out << arg;
//  return(out.str());
//}

int sum_second(int x, std::map<dtpString, int>::value_type const& v) { return x + v.second; }
int sum_dvector_int(int x, dnValue const& v) { return x + v.getAs<int>(); }


void showBenchResults(const char *title, const scDataNode &results)
{
  BOOST_TEST_MESSAGE("Benchmark - results:");
  BOOST_TEST_MESSAGE("Test name - result");
  BOOST_TEST_MESSAGE("-----------------------------------------");

  dtpString testName, line;
  for(int i=0, epos = results.size(); i < epos; i++) {
    testName = strPadRight(results.getElementName(i), 30);
    line = testName + dtpString("; ") + toString<uint64>(results.get<uint64>(i));
    BOOST_TEST_MESSAGE(line.c_str());
  }
  BOOST_TEST_MESSAGE("-----------------------------------------");
}

//-----------------------------------------
// vector
//-----------------------------------------
void test_insert_vector()
{
  //------- BEGIN -------
  std::vector<int> vect;
  for(int i=1; i <= ITEM_COUNT; i++)
    vect.push_back(i % 10);
  //-------  END  -------
}

void test_accum_vector()
{
  Timer::stop("bench");
  std::vector<int> vect;
  for(int i=1; i <= ITEM_COUNT; i++)
    vect.push_back(i % 10);
  Timer::start("bench");
  //------- BEGIN -------
  int sum = std::accumulate(vect.begin(), vect.end(), 0);
  vect.push_back(sum);
  //-------  END  -------
}

void test_update_vector()
{
  Timer::stop("bench");
  std::vector<int> vect;
  for(int i=1; i <= ITEM_COUNT; i++)
    vect.push_back(i % 10);
  Timer::start("bench");
  //------- BEGIN -------
  for(int i=0, epos = vect.size(); i < epos; i++)
    vect[i] = 2 * vect[i];
  //-------  END  -------
}

void test_delete_vector()
{
  Timer::stop("bench");
  std::vector<int> vect;
  for(int i=1, epos = ITEM_COUNT / DELETE_DIV; i < epos; i++)
    vect.push_back(i % 10);
  Timer::start("bench");
  //------- BEGIN -------
  while(!vect.empty())
    vect.erase(vect.begin());
  //-------  END  -------
}

void test_find_vector()
{
  bool stopped = Timer::stop("bench");
  std::vector<int> vect;
  for(int i=0; i <= ITEM_COUNT / FIND_DIV; i++)
    vect.push_back(i);
  if (stopped) Timer::start("bench");
  //------- BEGIN -------
  int sum = 0;
  for(int i=0, epos = vect.size(); i < epos; i++)
    sum += *find(vect.begin(), vect.end(), i);
  //-------  END  -------
  stopped = Timer::stop("bench");
  vect.push_back(sum);
  if (stopped) Timer::start("bench");
}

void test_size_vector()
{
  //------- BEGIN -------
  std::vector<int> vect;
  for(int i=1, epos = ITEM_COUNT / SIZE_DIV; i < epos; i++)
    vect.push_back(vect.size());
  //-------  END  -------
}

//-----------------------------------------
// list
//-----------------------------------------
void test_insert_list()
{
  //------- BEGIN -------
  std::list<int> vect;
  for(int i=1; i <= ITEM_COUNT; i++)
    vect.push_back(i % 10);
  //-------  END  -------
}

void test_accum_list()
{
  Timer::stop("bench");
  std::list<int> vect;
  for(int i=1; i <= ITEM_COUNT; i++)
    vect.push_back(i % 10);
  Timer::start("bench");
  //------- BEGIN -------
  int sum = std::accumulate(vect.begin(), vect.end(), 0);
  //-------  END  -------
  vect.push_back(sum);
}

void test_update_list()
{
  Timer::stop("bench");
  std::list<int> list;
  for(int i=1; i <= ITEM_COUNT; i++)
    list.push_back(i % 10);
  Timer::start("bench");
  //------- BEGIN -------
  for(std::list<int>::iterator it = list.begin(), epos = list.end(); it != epos; ++it)
    *it = 2 * (*it);
  //-------  END  -------
}

void test_delete_list()
{
  Timer::stop("bench");
  std::list<int> list;
  for(int i=1, epos = ITEM_COUNT / DELETE_DIV; i < epos; i++)
    list.push_back(i % 10);
  Timer::start("bench");
  //------- BEGIN -------
  while(!list.empty())
    list.erase(list.begin());
  //-------  END  -------
}

void test_find_list()
{
  bool stopped = Timer::stop("bench");
  std::list<int> list;
  for(int i=0; i <= ITEM_COUNT / FIND_DIV; i++)
    list.push_back(i);
  if (stopped) Timer::start("bench");
  //------- BEGIN -------
  int sum = 0;
  for(int i=0, epos = list.size(); i < epos; i++)
    sum += *find(list.begin(), list.end(), i);
  //-------  END  -------
  stopped = Timer::stop("bench");
  list.push_back(sum);
  if (stopped) Timer::start("bench");
}

void test_size_list()
{
  //------- BEGIN -------
  std::list<int> list;
  for(int i=1, epos = ITEM_COUNT / SIZE_DIV; i < epos; i++)
    list.push_back(list.size());
  //-------  END  -------
}

void fillnames(int firstValue, int count, std::vector<dtpString> &names)
{
  dtpString str;
  names.reserve(count);
  for(int i=1, val = firstValue; i <= count; i++, val++) {
    names.push_back(toString(val, str));
  }
}

//-----------------------------------------
// map
//-----------------------------------------
void fillnames_map_map(int n, std::vector<dtpString> &names)
{
  dtpString str;
  names.reserve(n);
  for(int i=1; i <= n; i++) {
    names.push_back(toString(i, str));
  }
}

void test_insert_map()
{
  bool wasRunning = Timer::stop("bench");
  std::vector<dtpString> names;
  fillnames_map_map(ITEM_COUNT, names);
  if (wasRunning) Timer::start("bench");
  //------- BEGIN -------
  std::map<dtpString, int> map;
  for(int i=1; i <= ITEM_COUNT; i++)
    map.insert(std::make_pair(names[i-1], i % 10));
  //-------  END  -------
}

void test_accum_map()
{
  bool wasRunning = Timer::stop("bench");
  std::vector<dtpString> names;
  fillnames_map_map(ITEM_COUNT, names);
  std::map<dtpString, int> map;
  for(int i=1; i <= ITEM_COUNT; i++)
    map.insert(std::make_pair(names[i-1], i % 10));
  if (wasRunning) Timer::start("bench");
  //------- BEGIN -------
  int sum = std::accumulate(map.begin(), map.end(), 0, sum_second);
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  map.insert(std::make_pair(toString(sum), sum % 10));
  if (wasRunning) Timer::start("bench");
}

void test_update_map_key()
{
  Timer::stop("bench");
  std::map<dtpString, int> map;
  std::vector<dtpString> name_list;

  for(int i=1; i <= ITEM_COUNT; i++) {
    name_list.push_back(toString(i));
    map.insert(std::make_pair(toString(i), i % 10));
  }

  std::map<dtpString, int>::iterator it;
  std::map<dtpString, int>::iterator eposIt = map.end();

  Timer::start("bench");
  //------- BEGIN -------
  for(int i=0, epos = map.size(); i < epos; i++)
  {
    it = map.find(name_list[i]);
    if (it != eposIt)
      it->second = 2 * it->second;
  }
  //-------  END  -------
}

void test_update_map_it()
{
  Timer::stop("bench");
  std::map<dtpString, int> map;
  for(int i=1; i <= ITEM_COUNT; i++)
    map.insert(std::make_pair(toString(i), i % 10));
  Timer::start("bench");
  //------- BEGIN -------
  for(std::map<dtpString, int>::iterator it = map.begin(), epos = map.end(); it != epos; ++it)
    it->second = 2 * it->second;
  //-------  END  -------
}

void test_delete_map()
{
  Timer::stop("bench");
  std::map<dtpString, int> map;
  for(int i=1; i <= ITEM_COUNT / DELETE_DIV; i++)
    map.insert(std::make_pair(toString(i), i % 10));
  Timer::start("bench");
  //------- BEGIN -------
  while(!map.empty())
    map.erase(map.begin());
  //-------  END  -------
}

void test_find_map()
{
  bool stopped = Timer::stop("bench");
  std::map<dtpString, int> map;
  std::vector<dtpString> name_list;
  for(int i=0; i <= ITEM_COUNT / FIND_DIV; i++) {
    name_list.push_back(toString(i));
    map.insert(std::make_pair(name_list[i], i % 10));
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------
  int sum = 0;
  for(int i=0, epos = map.size(); i < epos; i++)
    sum += map.find(name_list[i])->second;
  //-------  END  -------
  stopped = Timer::stop("bench");
  map.insert(std::make_pair(toString(sum), sum % 10));
  if (stopped) Timer::start("bench");
}

void test_size_map()
{
  Timer::stop("bench");
  std::vector<dtpString> name_list;
  for(int i=0; i <= ITEM_COUNT / SIZE_DIV; i++)
    name_list.push_back(toString(i));

  Timer::start("bench");
  //------- BEGIN -------
  std::map<dtpString, int> map;
  for(int i=0; i <= ITEM_COUNT / SIZE_DIV; i++)
    map.insert(std::make_pair(name_list[i], map.size()));
  //-------  END  -------
}

//-----------------------------------------
// dvector
//-----------------------------------------

void fill_dvector(int n, dnode_vector &vect)
{
  vect.clear();
  dnValue val;
  for(int i=1; i <= n; i++) {
    val.setAs(i % 10);
    vect.push_back(val);
  }
}

void test_insert_dvector()
{
  //------- BEGIN -------
  dnode_vector vect;
  fill_dvector(ITEM_COUNT, vect);
  //-------  END  -------
}

void test_accum_dvector()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  dnValue val;
  dnode_vector vect;
  fill_dvector(ITEM_COUNT, vect);
  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  int sum = std::accumulate(vect.begin(), vect.end(), 0, sum_dvector_int);
  //-------  END  -------
  val.setAs(sum);
  vect.push_back(val);
}

void test_update_dvector()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  dnValue val;
  dnode_vector vect;
  fill_dvector(ITEM_COUNT, vect);
  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  for(int i=0, epos = vect.size(); i < epos; i++)
    vect[i].setAs(2 * vect[i].getAs<int>());
  //-------  END  -------
}

void test_delete_dvector()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  dnValue val;
  dnode_vector vect;
  fill_dvector(ITEM_COUNT / DELETE_DIV, vect);
  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  while(!vect.empty())
    vect.erase(vect.begin());
  //-------  END  -------
}

template <typename ObjClass, typename ValueType>
class MatchDataValueByInt
{
private:
ValueType m_value;

public:
MatchDataValueByInt(const ValueType &value) : m_value(value) {}

bool operator () (const ObjClass &rhs) const {return rhs.template getAs<int>() == m_value;}
};


void test_find_dvector()
{
  bool stopped = Timer::stop("bench");
  dnValue val;
  dnode_vector vect;
  for(int i=0; i <= ITEM_COUNT / FIND_DIV; i++) {
    val.setAs(i);
    vect.push_back(val);
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------
  int sum = 0;
  for(int i=0, epos = vect.size(); i < epos; i++) {
    sum += find_if(vect.begin(), vect.end(), MatchDataValueByInt<dnValue, int>(i))->getAs<int>();
  }
  //-------  END  -------
  stopped = Timer::stop("bench");
  val.setAs(sum);
  vect.push_back(val);
  if (stopped) Timer::start("bench");
}

void test_size_dvector()
{
  //------- BEGIN -------
  dnValue val;
  dnode_vector vect;
  for(int i=1; i <= ITEM_COUNT / SIZE_DIV; i++) {
    val.setAs(vect.size());
    vect.push_back(val);
  }
  //-------  END  -------
}

//-----------------------------------------
// map_any
//-----------------------------------------
typedef std::map<dtpString, boost::any> StdMapIntAny;

void fillnames_map_any(int n, std::vector<dtpString> &names)
{
  dtpString str;
  names.reserve(n);
  for(int i=1; i <= n; i++) {
    names.push_back(toString(i, str));
  }
}

void fill_map_any(int n, StdMapIntAny &vect)
{
  bool wasRunning = Timer::stop("bench");
  std::vector<dtpString> names;
  fillnames_map_any(n, names);
  if (wasRunning)
    Timer::start("bench");

  vect.clear();
  for(int i=1; i <= n; i++) {
    vect.insert(std::make_pair(names[i - 1], i % 10));
  }
}

void test_insert_map_any()
{
  //------- BEGIN -------
  StdMapIntAny map;
  fill_map_any(ITEM_COUNT, map);
  //-------  END  -------
}

void test_accum_map_any()
{
  using boost::any_cast;

  bool wasRunning = Timer::stop("bench");
  StdMapIntAny map;
  fill_map_any(ITEM_COUNT, map);
  if (wasRunning)
    Timer::start("bench");

  //------- BEGIN -------
  int sum = 0;
  for(StdMapIntAny::iterator it = map.begin(), epos = map.end(); it != epos; ++it)
    sum = sum + any_cast<int>((*it).second);
  //-------  END  -------

  map.insert(std::make_pair(toString(map.size()+1), sum));
}

void test_update_map_any()
{
  using boost::any_cast;

  Timer::stop("bench");
  StdMapIntAny map;
  for(int i=1; i <= ITEM_COUNT; i++)
    map.insert(std::make_pair(toString(i), i % 10));
  Timer::start("bench");
  //------- BEGIN -------
  for(StdMapIntAny::iterator it = map.begin(), epos = map.end(); it != epos; ++it)
    it->second = 2 * any_cast<int>((*it).second);
  //-------  END  -------
}

void test_delete_map_any()
{
  Timer::stop("bench");
  StdMapIntAny map;
  for(int i=1; i <= ITEM_COUNT / DELETE_DIV; i++)
    map.insert(std::make_pair(toString(i), i % 10));
  Timer::start("bench");
  //------- BEGIN -------
  while(!map.empty())
    map.erase(map.begin());
  //-------  END  -------
}

void test_find_map_any()
{
  using boost::any_cast;

  bool stopped = Timer::stop("bench");
  StdMapIntAny map;
  std::vector<dtpString> name_list;
  for(int i=0; i <= ITEM_COUNT / FIND_DIV; i++) {
    name_list.push_back(toString(i));
    map.insert(std::make_pair(name_list[i], i % 10));
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------
  int sum = 0;
  for(int i=0, epos = map.size(); i < epos; i++)
    sum += any_cast<int>(map.find(name_list[i])->second);
  //-------  END  -------
  stopped = Timer::stop("bench");
  map.insert(std::make_pair(toString(sum), sum % 10));
  if (stopped) Timer::start("bench");
}

void test_size_map_any()
{
  Timer::stop("bench");
  std::vector<dtpString> name_list;
  for(int i=0; i <= ITEM_COUNT / SIZE_DIV; i++)
    name_list.push_back(toString(i));

  Timer::start("bench");
  //------- BEGIN -------
  StdMapIntAny map;
  for(int i=0; i <= ITEM_COUNT / SIZE_DIV; i++)
    map.insert(std::make_pair(name_list[i], map.size()));
  //-------  END  -------
}

//-----------------------------------------
// dnode_parent_val_by_idx
//-----------------------------------------
void fill_dnode_parent_val_by_idx(int n, scDataNode &node, std::vector<dtpString> &names)
{
  bool wasRunning = Timer::stop("bench");
  dtpString str;
  node = scDataNode(ict_parent);
  names.reserve(n);
  for(int i=1; i <= n; i++) {
    names.push_back(toString(i, str));
  }
  if (wasRunning) Timer::start("bench");
  for(int i=0; i < n; i++) {
    node.addChild(names[i], new scDataNode(i % 10));
  }
}

void test_insert_dnode_parent_val_by_idx()
{
  //------- BEGIN -------
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_val_by_idx(ITEM_COUNT, node, names);
  //-------  END  -------
}

/*
void test_insert_dnode_parent_val_str()
{
  scDataNode node;
  std::vector<dtpString> names;
  int n = ITEM_COUNT;

  Timer::stop("bench");
  //toStringGate<int> strGate;
  dtpString str;
  node = scDataNode(ict_parent);
  //std::vector<dtpString> names;
  names.reserve(n);
  for(int i=1; i <= n; i++) {
    //names.push_back(strGate.toString(i, str));
    names.push_back(toString(i, str));
  }
  Timer::start("bench");
  for(int i=0; i < n; i++) {
    node.addChild(new scDataNode(names[i]));
  }
}
*/

//void test_insert_dnode_parent_val_by_idx_fsb()
//{
//  Timer::stop("bench");
//  scDataNode node;
//  std::vector<dtpString> names;
//  dtpString str;
//  node = scDataNode(ict_parent);
//  names.reserve(ITEM_COUNT);
//  for(int i=1; i <= ITEM_COUNT; i++) {
//    names.push_back(toString(i, str));
//  }
//  Timer::start("bench");
//
//  FSBAllocator<scDataNode> alloc;
//  scDataNode* anInstance;
//  scDataNode tpl;
//
//  for(int i=0; i < ITEM_COUNT; i++) {
//    tpl.getValue().setAs(i % 10);
//    anInstance = alloc.allocate(1);
//    alloc.construct(anInstance, tpl);
//    anInstance->setName(names[i]);
//    node.addChild(anInstance);
//  }
//
//  for(int i=0; i < ITEM_COUNT; i++) {
//    anInstance = node.getChildPtr(i);
//    alloc.destroy(anInstance);
//  }
//
//  //?node.releaseChildren();
//}


void test_accum_dnode_parent_val_by_idx()
{
  bool wasRunning = Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_val_by_idx(ITEM_COUNT, node, names);
  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  int sum = 0;
  for(int i=0, epos = node.size(); i < epos; i++)
    sum += node.get<int>(i);
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (wasRunning)
    Timer::start("bench");
}

void test_update_dnode_parent_val_by_idx()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_val_by_idx(ITEM_COUNT, node, names);
  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  for(int i=0, epos = node.size(); i < epos; i++)
    node.set(i, 2 * node.get<int>(i));
  //-------  END  -------
}

void test_delete_dnode_parent_val_by_idx()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_val_by_idx(ITEM_COUNT / DELETE_DIV, node, names);
  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  while(!node.empty())
    node.eraseElement(0);
  //-------  END  -------
}

void test_find_dnode_parent_val_by_idx()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_parent);

  std::vector<dtpString> names;
  int n = ITEM_COUNT / FIND_DIV;
  names.resize(n);
  for(int i=0; i < n; i++) {
    names.push_back(toString(i));
    node.addChild(names[i], new scDataNode(i));
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------
  int sum = 0;
  int idx;
  for(int i=0, epos = node.size(); i < epos; i++) {
    //idx = node.indexOfName(names[i]);
    idx = node.indexOfValue<int>(i);
    sum += node.get<int>(idx);
  }
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_size_dnode_parent_val_by_idx()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node(ict_parent);
  int n = ITEM_COUNT / SIZE_DIV;
  std::vector<dtpString> names;
  names.reserve(n);

  for(int i=0; i < n; i++) {
    names.push_back(toString(i));
  }

  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  for(int i=0; i < n; i++) {
    node.addChild(names[i], new scDataNode(node.size()));
  }
  //-------  END  -------
}

//-----------------------------------------
// dnode_list_val
//-----------------------------------------
void fill_dnode_list_val_by_idx(int n, scDataNode &node)
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  node = scDataNode(ict_list);
  if (wasRunning)
    Timer::start("bench");
  for(int i=0; i < n; i++) {
    node.addChild(new scDataNode(i % 10));
  }
}

void test_insert_dnode_list_val_by_idx()
{
  //------- BEGIN -------
  scDataNode node;
  fill_dnode_list_val_by_idx(ITEM_COUNT, node);
  //-------  END  -------
}

void test_accum_dnode_list_val_by_idx()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_list_val_by_idx(ITEM_COUNT, node);
  if (wasRunning) Timer::start("bench");
  //------- BEGIN -------
  int sum = 0;
  for(int i=0, epos = node.size(); i < epos; i++)
    sum += node.get<int>(i);
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (wasRunning) Timer::start("bench");
}

void test_update_dnode_list_val_by_idx()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_list_val_by_idx(ITEM_COUNT, node);
  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  for(int i=0, epos = node.size(); i < epos; i++)
    node.set(i, 2 * node.get<int>(i));
  //-------  END  -------
}

void test_delete_dnode_list_val_by_idx()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_list_val_by_idx(ITEM_COUNT / DELETE_DIV, node);
  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  while(!node.empty())
    node.eraseElement(0);
  //-------  END  -------
}

//template <typename ObjClass, typename ValueType>
//class MatchDataValueByInt_dpvi
//{
//private:
//ValueType m_value;
//
//public:
//MatchDataValueByInt_dpvi(const ValueType &value) : m_value(value) {}
//
//bool operator () (const ObjClass &rhs) const {return rhs.getAs<int>() == m_value;}
//};
//

void test_find_dnode_list_val_by_idx()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_list);

  int n = ITEM_COUNT / FIND_DIV;
  for(int i=0; i < n; i++) {
    node.addChild(new scDataNode((i + 13) % n)); // little unsorted
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  int sum = 0;
  int idx;
  scDataNode findVal(0);
  for(int i=0, epos = node.size(); i < epos; i++) {
    //findVal.setAs(i);
    //idx = node.indexOfValue(findVal);
    idx = node.indexOfValue<int>(i);
    sum += node.get<int>(idx);
  }
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_size_dnode_list_val_by_idx()
{
  Timer::stop("bench");
  scDataNode node(ict_list);
  int n = ITEM_COUNT / SIZE_DIV;

  Timer::start("bench");
  //------- BEGIN -------
  for(int i=0; i < n; i++) {
    node.addChild(new scDataNode(node.size()));
  }
  //-------  END  -------
}

//-----------------------------------------
// dnode_array_val
//-----------------------------------------
void fill_dnode_array_val_by_idx(int n, scDataNode &node)
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  node = scDataNode(ict_array, vt_int);
  if (wasRunning)
    Timer::start("bench");
  for(int i=0; i < n; i++) {
    node.addItem(i % 10);
  }
}

void test_insert_dnode_array_val_by_idx()
{
  //------- BEGIN -------
  scDataNode node;
  fill_dnode_array_val_by_idx(ITEM_COUNT, node);
  //-------  END  -------
}

void test_accum_dnode_array_val_by_idx()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_array_val_by_idx(ITEM_COUNT, node);
  if (wasRunning) Timer::start("bench");
  //------- BEGIN -------
  int sum = 0;
  for(int i=0, epos = node.size(); i < epos; i++)
    sum += node.get<int>(i);
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  node.addItem(sum);
  if (wasRunning) Timer::start("bench");
}

void test_update_dnode_array_val_by_idx()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_array_val_by_idx(ITEM_COUNT, node);
  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  for(int i=0, epos = node.size(); i < epos; i++)
    node.set(i, 2 * node.get<int>(i));
  //-------  END  -------
}

void test_delete_dnode_array_val_by_idx()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_array_val_by_idx(ITEM_COUNT / DELETE_DIV, node);
  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  while(!node.empty())
    node.eraseElement(0);
  //-------  END  -------
}

void test_find_dnode_array_val_by_idx()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_array, vt_int);

  int n = ITEM_COUNT / FIND_DIV;
  for(int i=0; i < n; i++) {
    node.addItem(i);
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  int sum = 0;
  int idx;
  scDataNode findVal(0);
  for(int i=0, epos = node.size(); i < epos; i++) {
#ifdef DATANODE_DERIVED_FROM_VAL
    findVal.setAs(i);
#else
    findVal.setAs(i);
#endif
    idx = node.getArray()->indexOfValue(findVal);
    sum += node.get<int>(idx);
  }
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addItem(sum);
  if (stopped) Timer::start("bench");
}

void test_find_dnode_array_val_by_idx_gen()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_array, vt_int);

  int n = ITEM_COUNT / FIND_DIV;
  for(int i=0; i < n; i++) {
    node.addItem(i);
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  int sum = 0;
  int idx;
  scDataNode findVal(0);
  for(int i=0, epos = node.size(); i < epos; i++) {
    idx = node.indexOfValue<int>(i);
    sum += node.get<int>(idx);
  }
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addItem(sum);
  if (stopped) Timer::start("bench");
}

void test_size_dnode_array_val_by_idx()
{
  Timer::stop("bench");
  scDataNode node(ict_array, vt_int);
  int n = ITEM_COUNT / SIZE_DIV;

  Timer::start("bench");
  //------- BEGIN -------
  for(int i=0; i < n; i++) {
    node.addItem(node.size());
  }
  //-------  END  -------
}

//-----------------------------------------
// dnode_parent_stl
//-----------------------------------------

//void test_accum_dnode_parent_stl()
//{
//  bool wasRunning = Timer::stop("bench");
//  scDataNode node;
//  std::vector<dtpString> names;
//  fill_dnode_parent_val_by_idx(ITEM_COUNT, node, names);
//  if (wasRunning)
//    Timer::start("bench");
//  //------- BEGIN -------
//  dnVectorIterator<int> it(node.begin());
//  dnVectorIterator<int> epos(node.end());
//  int sum = std::accumulate(it, epos, 0);
//  //-------  END  -------
//  wasRunning = Timer::stop("bench");
//  node.addChild("sum", new scDataNode(sum));
//  if (wasRunning) Timer::start("bench");
//}

//note: stl iterators are read-only, so we use default iterators here
void test_update_dnode_parent_stl()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_val_by_idx(ITEM_COUNT, node, names);
  if (wasRunning)
    Timer::start("bench");

  //------- BEGIN -------
  scDataNode::iterator it(node.begin());
  scDataNode::iterator epos(node.end());
  for(; it != epos; ++it)
    it->setAs(it->getAs<int>() * 2);
  //-------  END  -------
}

// no support for delete by iterator, so standard way
/*
void test_delete_dnode_parent_stl()
{
  Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_val_by_idx(ITEM_COUNT / DELETE_DIV, node, names);
  Timer::start("bench");
  while(!node.empty())
    node.eraseElement(0);
}
*/

//void test_find_dnode_parent_stl()
//{
//  bool wasRunning = Timer::stop("bench");
//  scDataNode node(ict_parent);
//
//  std::vector<dtpString> names;
//  int n = ITEM_COUNT / FIND_DIV;
//  names.resize(n);
//  for(int i=0; i < n; i++) {
//    names.push_back(toString(i));
//    node.addChild(names[i], new scDataNode(i));
//  }
//  if (wasRunning) Timer::start("bench");
//  //------- BEGIN -------
//
//  scDataNode::iterator it;
//  scDataNode::iterator eposIt = node.end();
//
//  int sum = 0;
//  for(int i=0, epos = node.size(); i < epos; i++) {
//    //sum += node.at(names[i])->getAs<int>();
//    it = node.at(names[i]);
//    if (it != eposIt)
//      sum += it->getAs<int>();
//  }
//  //-------  END  -------
//  wasRunning = Timer::stop("bench");
//  node.addChild("sum", new scDataNode(sum));
//  if (wasRunning) Timer::start("bench");
//}

//void test_size_dnode_parent_stl()
//{
//  Timer::stop("bench");
//  scDataNode node(ict_parent);
//  int n = ITEM_COUNT / SIZE_DIV;
//  std::vector<dtpString> names;
//  names.reserve(n);
//
//  for(int i=0; i < n; i++) {
//    names.push_back(toString(i));
//  }
//
//  Timer::start("bench");
//  //------- BEGIN -------
//  for(int i=0; i < n; i++) {
//    //node.addChild(new scDataNode(names[i], std::distance(node.begin(), node.end())));
//    node.addChild(
//       names[i],
//       new scDataNode(
//         node.size()
//       ));
//  }
//  //-------  END  -------
//}

//-----------------------------------------
// dnode_parent_scalar
//-----------------------------------------

void test_accum_dnode_parent_scalar()
{
  bool wasRunning = Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_val_by_idx(ITEM_COUNT, node, names);
  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  int sum = std::accumulate(node.scalarBegin<int>(), node.scalarEnd<int>(), 0);
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (wasRunning) Timer::start("bench");
}

void test_find_dnode_parent_scalar_at()
{
  bool wasRunning = Timer::stop("bench");
  scDataNode node(ict_parent);

  std::vector<dtpString> names;
  int n = ITEM_COUNT / FIND_DIV;
  names.resize(n);
  for(int i=0; i < n; i++) {
    names.push_back(toString(i));
    node.addChild(names[i], new scDataNode(i));
  }
  if (wasRunning) Timer::start("bench");
  //------- BEGIN -------

  scDataNode::scalar_iterator<int> it;
  scDataNode::scalar_iterator<int> eposIt = node.scalarEnd<int>();

  int sum = 0;
  for(int i=0, epos = node.size(); i < epos; i++) {
    //sum += node.at(names[i])->getAs<int>();
    it = node.scalarAt<int>(names[i]);
    if (it != eposIt)
      sum += (*it);
  }
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (wasRunning) Timer::start("bench");
}

//-----------------------------------------
// dnode_list_stl
//-----------------------------------------

// no support for insert by iterator, so standard way
void fill_dnode_list_stl(int n, scDataNode &node)
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  node = scDataNode(ict_list);
  if (wasRunning)
    Timer::start("bench");
  for(int i=0; i < n; i++) {
    node.addChild(new scDataNode(i % 10));
  }
}

void test_insert_dnode_list_stl()
{
  //------- BEGIN -------
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_val_by_idx(ITEM_COUNT, node, names);
  //-------  END  -------
}

//void test_accum_dnode_list_stl()
//{
//  bool wasRunning = Timer::isRunning("bench");
//  Timer::stop("bench");
//  scDataNode node;
//  fill_dnode_list_stl(ITEM_COUNT, node);
//  if (wasRunning) Timer::start("bench");
//  //------- BEGIN -------
//  dnVectorIterator<int> it(node.begin());
//  dnVectorIterator<int> epos(node.end());
//  int sum = std::accumulate(it, epos, 0);
//  //-------  END  -------
//  wasRunning = Timer::stop("bench");
//  node.addChild("sum", new scDataNode(sum));
//  if (wasRunning) Timer::start("bench");
//}

//note: stl iterators are read-only, so we use default iterators here
void test_update_dnode_list_stl()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_list_stl(ITEM_COUNT, node);
  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------

  scDataNode::iterator it(node.begin());
  scDataNode::iterator epos(node.end());
  for(; it != epos; ++it)
    it->setAs(it->getAs<int>() * 2);
  //-------  END  -------
}

// no support for delete by iterator, so standard way
void test_delete_dnode_list_stl()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_list_stl(ITEM_COUNT / DELETE_DIV, node);
  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  while(!node.empty())
    node.eraseElement(0);
  //-------  END  -------
}

void test_find_dnode_list_alg_node()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_list);

  int n = ITEM_COUNT / FIND_DIV;
  for(int i=0; i < n; i++) {
    node.addChild(new scDataNode((i + 13) % n)); // little unsorted
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  scDataNode::iterator it;
  scDataNode::iterator eposIt = node.end();
  scDataNode valNode(0);

  int sum = 0;
  for(int i=0, epos = node.size(); i < epos; i++) {
    valNode.setAs(i);
    it = node.find(valNode);
    if (it != eposIt)
      sum += it->getAs<int>();
  }
  //-------  END  -------

  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_size_dnode_list_stl()
{
  Timer::stop("bench");
  scDataNode node(ict_list);
  int n = ITEM_COUNT / SIZE_DIV;

  Timer::start("bench");
  //------- BEGIN -------
  for(int i=0; i < n; i++) {
    //node.addChild(new scDataNode(names[i], std::distance(node.begin(), node.end())));
    node.addChild(
       new scDataNode(
         node.size()
       ));
  }
  //-------  END  -------
}

//-----------------------------------------
// dnode_list_scalar
//-----------------------------------------
void fill_dnode_list_int_mod10(int n, scDataNode &node)
{
  bool wasRunning = Timer::stop("bench");
  dtpString str;
  node = scDataNode(ict_list);
  if (wasRunning)
    Timer::start("bench");
  for(int i=0; i < n; i++) {
    node.addChild<int>(i % 10);
  }
}

void test_accum_dnode_list_int_scalar()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;
  //fill_dnode_list_stl(ITEM_COUNT, node);
  fill_dnode_list_int_mod10(ITEM_COUNT, node);
  if (wasRunning) Timer::start("bench");
  //------- BEGIN -------
  int sum = std::accumulate(node.scalarBegin<int>(), node.scalarEnd<int>(), 0);
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (wasRunning) Timer::start("bench");
}

void test_find_dnode_list_int_scalar()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_list);

  int n = ITEM_COUNT / FIND_DIV;
  for(int i=0; i < n; i++) {
    node.addChild(new scDataNode((i + 13) % n)); // little unsorted
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  scDataNode::scalar_iterator<int> it;
  scDataNode::scalar_iterator<int> beginIt = node.scalarBegin<int>();
  scDataNode::scalar_iterator<int> eposIt = node.scalarEnd<int>();

  int sum = 0;
  for(int i=0, epos = node.size(); i < epos; i++) {
    it = std::find(
       beginIt,
       eposIt,
       i
    );
    if (it != eposIt)
      sum += *it;
  }
  //-------  END  -------

  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

//-----------------------------------------
// dnode_array_dbl_val
//-----------------------------------------
void fill_dnode_array_dbl_val(int n, scDataNode &node)
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  node = scDataNode(ict_array, vt_double);
  if (wasRunning)
    Timer::start("bench");
  double a = 0.0;
  for(int i=0; i < n; i++) {
    node.addItem(a / 10.0);
    a += 1.0;
  }
}

void test_insert_dnode_array_dbl_val()
{
  //------- BEGIN -------
  scDataNode node;
  fill_dnode_array_dbl_val(ITEM_COUNT, node);
  //-------  END  -------
}

void test_accum_dnode_array_dbl_val()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_array_dbl_val(ITEM_COUNT, node);
  if (wasRunning) Timer::start("bench");
  //------- BEGIN -------
  double sum = 0;
  for(int i=0, epos = node.size(); i < epos; i++)
    sum += node.get<double>(i);
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  node.addItem(sum);
  if (wasRunning) Timer::start("bench");
}

void test_update_dnode_array_dbl_val()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_array_dbl_val(ITEM_COUNT, node);
  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  for(int i=0, epos = node.size(); i < epos; i++)
    node.set(i, 2.0 * node.get<double>(i));
  //-------  END  -------
}

void test_delete_dnode_array_dbl_val()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_array_dbl_val(ITEM_COUNT / DELETE_DIV, node);
  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  while(!node.empty())
    node.eraseElement(0);
  //-------  END  -------
}

void test_find_dnode_array_dbl_val()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node(ict_array, vt_double);

  int n = ITEM_COUNT / FIND_DIV;
  double a(0.0);
  for(int i=0; i < n; i++) {
    node.addItem(a);
    a += 1.0;
  }
  if (wasRunning) Timer::start("bench");
  //------- BEGIN -------

  double sum = 0;
  int idx;
  scDataNode findVal(0.0);
  a = 0.0;
  for(int i=0, epos = node.size(); i < epos; i++) {
    findVal.setAs(a);
    idx = node.getArray()->indexOfValue(findVal);
    sum += node.get<double>(idx);
    a += 1.0;
  }
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  node.addItem(sum);
  if (wasRunning) Timer::start("bench");
}

void test_find_dnode_array_dbl_val_gen()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node(ict_array, vt_double);

  int n = ITEM_COUNT / FIND_DIV;
  double a(0.0);
  for(int i=0; i < n; i++) {
    node.addItem(a);
    a += 1.0;
  }
  if (wasRunning) Timer::start("bench");
  //------- BEGIN -------

  double sum = 0;
  int idx;
  scDataNode findVal(0.0);
  a = 0.0;
  for(int i=0, epos = node.size(); i < epos; i++) {
    idx = node.indexOfValue<double>(i);
    sum += node.get<double>(idx);
    a += 1.0;
  }
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  node.addItem(sum);
  if (wasRunning) Timer::start("bench");
}

void test_size_dnode_array_dbl_val()
{
  Timer::stop("bench");
  scDataNode node(ict_array, vt_double);
  int n = ITEM_COUNT / SIZE_DIV;

  Timer::start("bench");
  //------- BEGIN -------
  for(int i=0; i < n; i++) {
    node.addItem(node.size());
  }
  //-------  END  -------
}

//-----------------------------------------
// dnode_array_dbl_stl
//-----------------------------------------
void fill_dnode_array_dbl_stl(int n, scDataNode &node)
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  node = scDataNode(ict_array, vt_double);
  if (wasRunning)
    Timer::start("bench");
  double a = 0.0;
  for(int i=0; i < n; i++) {
    node.addItem(a / 10.0);
    a += 1.0;
  }
}

void test_accum_dnode_array_dbl_scalar()
{
  bool stopped = Timer::stop("bench");
  scDataNode node;
  fill_dnode_array_dbl_stl(ITEM_COUNT, node);
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  scDataNode::scalar_iterator<double> it(node.scalarBegin<double>());
  scDataNode::scalar_iterator<double> epos(node.scalarEnd<double>());
  double sum = std::accumulate(it, epos, 0.0);

  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addItem(sum);
  if (stopped) Timer::start("bench");
}

void test_find_dnode_array_dbl_scalar()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_array, vt_double);

  int n = ITEM_COUNT / FIND_DIV;
  double a(0.0);
  for(int i=0; i < n; i++) {
    node.addItem(a);
    a += 1.0;
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  scDataNode::scalar_iterator<double> it;
  scDataNode::scalar_iterator<double> begIt = node.scalarBegin<double>();
  scDataNode::scalar_iterator<double> eposIt = node.scalarEnd<double>();

  a = 0.0;

  double sum = 0;
  for(int i=0, epos = node.size(); i < epos; i++) {
    it = std::find(
       begIt,
       eposIt,
       static_cast<double>(i)
    );
    if (it != eposIt)
      sum += *it;
  }

  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addItem(sum);
  if (stopped) Timer::start("bench");
}

void test_find_dnode_array_dbl_alg_node()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_array, vt_double);

  int n = ITEM_COUNT / FIND_DIV;
  double a(0.0);
  for(int i=0; i < n; i++) {
    node.addItem(a);
    a += 1.0;
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  scDataNode::iterator it;
  scDataNode::iterator eposIt = node.end();
  scDataNode valNode(0.0);

  a = 0.0;

  double sum = 0;
  for(int i=0, epos = node.size(); i < epos; i++) {
    valNode.setAs(a);
    it = node.find(valNode);
    if (it != eposIt)
      sum += it->getAs<double>();
  }

  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addItem(sum);
  if (stopped) Timer::start("bench");
}

void test_find_dnode_array_dbl_alg()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_array, vt_double);

  int n = ITEM_COUNT / FIND_DIV;
  double a(0.0);
  for(int i=0; i < n; i++) {
    node.addItem(a);
    a += 1.0;
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  scDataNode::iterator it;
  scDataNode::iterator eposIt = node.end();
  scDataNode valNode(0.0);

  a = 0.0;

  double sum = 0;
  for(int i=0, epos = node.size(); i < epos; i++) {
    it = node.find(a);
    if (it != eposIt)
      sum += it->getAs<double>();
  }

  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addItem(sum);
  if (stopped) Timer::start("bench");
}

//-----------------------------------------
// dnode_parent_qref
//-----------------------------------------
void fill_dnode_parent_qref(int n, scDataNode &node, std::vector<dtpString> &names)
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  dtpString str;
  node = scDataNode(ict_parent);
  names.reserve(n);
  for(int i=1; i <= n; i++) {
    names.push_back(toString(i, str));
  }
  if (wasRunning)
    Timer::start("bench");
  for(int i=0; i < n; i++) {
    node.addChild(names[i], new scDataNode(i % 10));
  }
}

void test_accum_dnode_parent_qref()
{
  bool stopped = Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_qref(ITEM_COUNT, node, names);
  if (stopped) Timer::start("bench");
  //------- BEGIN -------
  int sum = 0;
  scDataNode valNode;
  for(int i=0, epos = node.size(); i < epos; i++)
    sum += node.getNode(i, valNode).getAs<int>();
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_update_dnode_parent_qref()
{
  Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_qref(ITEM_COUNT, node, names);
  Timer::start("bench");
  //------- BEGIN -------
  scDataNode *nodePtr;
  scDataNode nodeVal;
  for(int i=0, epos = node.size(); i < epos; i++)
  {
    nodePtr = node.getNodePtr(i, nodeVal);
    nodePtr->setAs(nodePtr->getAs<int>());
    node.setNode(i, *nodePtr);
  }
  //-------  END  -------
}

void test_delete_dnode_parent_qref()
{
  Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_qref(ITEM_COUNT / DELETE_DIV, node, names);
  Timer::start("bench");
  //------- BEGIN -------
  while(!node.empty())
    node.eraseElement(0);
  //-------  END  -------
}

void test_find_dnode_parent_qref()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_parent);

  std::vector<dtpString> names;
  int n = ITEM_COUNT / FIND_DIV;
  names.resize(n);
  for(int i=0; i < n; i++) {
    names.push_back(toString(i));
    node.addChild(names[i], new scDataNode(i));
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  int sum = 0;
  int idx;
  scDataNode valNode;
  for(int i=0, epos = node.size(); i < epos; i++) {
    idx = node.indexOfName(names[i]);
    sum += node.getNode(idx, valNode).getAs<int>();
  }
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_size_dnode_parent_qref()
{
  Timer::stop("bench");
  scDataNode node(ict_parent);
  int n = ITEM_COUNT / SIZE_DIV;
  std::vector<dtpString> names;
  names.reserve(n);

  for(int i=0; i < n; i++) {
    names.push_back(toString(i));
  }

  Timer::start("bench");
  //------- BEGIN -------
  for(int i=0; i < n; i++) {
    node.addChild(names[i], new scDataNode(node.size()));
  }
  //-------  END  -------
  //---- correct timer - do not include insert
  Timer::stop("bench");
  cpu_ticks totalSize = Timer::getTotal("bench");
  Timer::reset("bench");
  node = scDataNode(ict_parent);
  Timer::start("bench");
  for(int i=0; i < n; i++) {
    node.addChild(names[i], new scDataNode(i));
  }
  Timer::stop("bench");
  cpu_ticks totalInsert = Timer::getTotal("bench");
  if (totalInsert <= totalSize)
    totalSize = totalSize - totalInsert;
  else
    totalSize = 0;
  Timer::reset("bench");
  Timer::inc("bench", totalSize);
  Timer::start("bench");
  //---- end of correction
}

//-----------------------------------------
// dnode_parent_ref
//-----------------------------------------
void fill_dnode_parent_ref(int n, scDataNode &node, std::vector<dtpString> &names)
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  dtpString str;
  node = scDataNode(ict_parent);
  names.reserve(n);
  for(int i=1; i <= n; i++) {
    names.push_back(toString(i, str));
  }
  if (wasRunning)
    Timer::start("bench");
  for(int i=0; i < n; i++) {
    node.addChild(names[i], new scDataNode(i % 10));
  }
}

void test_accum_dnode_parent_ref()
{
  bool stopped = Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_ref(ITEM_COUNT, node, names);
  if (stopped) Timer::start("bench");
  //------- BEGIN -------
  int sum = 0;
  scDataNode valNode;
  for(int i=0, epos = node.size(); i < epos; i++)
    sum += node[i].getAs<int>();
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_update_dnode_parent_ref()
{
  Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_ref(ITEM_COUNT, node, names);
  Timer::start("bench");
  //------- BEGIN -------
  for(int i=0, epos = node.size(); i < epos; i++)
    node[i].setAs(2 * node[i].getAs<int>());
  //-------  END  -------
}

void test_delete_dnode_parent_ref()
{
  Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_ref(ITEM_COUNT / DELETE_DIV, node, names);
  Timer::start("bench");
  //------- BEGIN -------
  while(!node.empty())
    node.eraseElement(0);
  //-------  END  -------
}

void test_find_dnode_parent_ref()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_parent);

  std::vector<dtpString> names;
  int n = ITEM_COUNT / FIND_DIV;
  names.resize(n);
  for(int i=0; i < n; i++) {
    names.push_back(toString(i));
    node.addChild(names[i], new scDataNode(i));
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  int sum = 0;
  int idx;
  scDataNode valNode;
  for(int i=0, epos = node.size(); i < epos; i++) {
    idx = node.indexOfName(names[i]);
    sum += node[idx].getAs<int>();
  }
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

//void test_size_dnode_parent_ref()
//{
//  Timer::stop("bench");
//  scDataNode node(ict_parent);
//  int n = ITEM_COUNT / SIZE_DIV;
//  std::vector<dtpString> names;
//  names.reserve(n);
//
//  for(int i=0; i < n; i++) {
//    names.push_back(toString(i));
//  }
//
//  Timer::start("bench");
//  for(int i=0; i < n; i++) {
//    node.addChild(names[i], new scDataNode(node.size()));
//  }
//
//  //---- correct timer - do not include insert
//  Timer::stop("bench");
//  cpu_ticks totalSize = Timer::getTotal("bench");
//  Timer::reset("bench");
//  node = scDataNode(ict_parent);
//  Timer::start("bench");
//  for(int i=0; i < n; i++) {
//    node.addChild(names[i], new scDataNode(i));
//  }
//  Timer::stop("bench");
//  cpu_ticks totalInsert = Timer::getTotal("bench");
//  if (totalInsert <= totalSize)
//    totalSize = totalSize - totalInsert;
//  Timer::reset("bench");
//  Timer::inc("bench", totalSize);
//  Timer::start("bench");
//  //---- end of correction
//}

//-----------------------------------------
// dnode_parent_elem
//-----------------------------------------
void fill_dnode_parent_elem(int n, scDataNode &node, std::vector<dtpString> &names)
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  dtpString str;
  node = scDataNode(ict_parent);
  names.reserve(n);
  for(int i=1; i <= n; i++) {
    names.push_back(toString(i, str));
  }
  if (wasRunning)
    Timer::start("bench");
  for(int i=0; i < n; i++) {
    node.addChild(names[i], new scDataNode(i % 10));
  }
}

void test_accum_dnode_parent_elem()
{
  bool stopped = Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_elem(ITEM_COUNT, node, names);
  if (stopped) Timer::start("bench");
  //------- BEGIN -------
  int sum = 0;
  scDataNode valNode;
  for(int i=0, epos = node.size(); i < epos; i++)
    sum += node.getElement(i, valNode).getAs<int>();
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_update_dnode_parent_elem()
{
  Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_elem(ITEM_COUNT, node, names);
  Timer::start("bench");
  //------- BEGIN -------
  scDataNode nodeVal;
  int ival;
  for(int i=0, epos = node.size(); i < epos; i++)
  {
    ival = node.getElement(i, nodeVal).getAs<int>();
    nodeVal.setAs(ival * 2);
    node.setElement(i, nodeVal);
  }
  //-------  END  -------
}

void test_delete_dnode_parent_elem()
{
  Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fill_dnode_parent_elem(ITEM_COUNT / DELETE_DIV, node, names);
  Timer::start("bench");
  //------- BEGIN -------
  while(!node.empty())
    node.eraseElement(0);
  //-------  END  -------
}

void test_find_dnode_parent_elem()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_parent);

  std::vector<dtpString> names;
  int n = ITEM_COUNT / FIND_DIV;
  names.resize(n);
  for(int i=0; i < n; i++) {
    names.push_back(toString(i));
    node.addChild(names[i], new scDataNode(i));
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  int sum = 0;
  int idx;
  scDataNode valNode;
  for(int i=0, epos = node.size(); i < epos; i++) {
    idx = node.indexOfName(names[i]);
    sum += node.getElement(idx, valNode).getAs<int>();
  }
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_size_dnode_parent_elem()
{
  Timer::stop("bench");
  scDataNode node(ict_parent);
  int n = ITEM_COUNT / SIZE_DIV;
  std::vector<dtpString> names;
  names.reserve(n);

  for(int i=0; i < n; i++) {
    names.push_back(toString(i));
  }

  Timer::start("bench");
  //------- BEGIN -------
  for(int i=0; i < n; i++) {
    node.addChild(names[i], new scDataNode(node.size()));
  }
  //-------  END  -------
}

//-----------------------------------------
// dnode_array_dbl_stl_it_cast
//-----------------------------------------
void fill_dnode_array_dbl_stl_it_cast(int n, scDataNode &node)
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  node = scDataNode(ict_array, vt_double);
  if (wasRunning)
    Timer::start("bench");
  double a = 0.0;
  for(int i=0; i < n; i++) {
    node.addItem(a / 10.0);
    a += 1.0;
  }
}

//void test_accum_dnode_array_dbl_stl_it_cast()
//{
//  bool stopped = Timer::stop("bench");
//  scDataNode node;
//  fill_dnode_array_dbl_stl_it_cast(ITEM_COUNT, node);
//  if (stopped) Timer::start("bench");
//  //------- BEGIN -------
//
//  double sum = std::accumulate(std_iterator_cast<double>(node.begin()),
//                               std_iterator_cast<double>(node.end()),
//                               0.0);
//
//
//  //-------  END  -------
//  stopped = Timer::stop("bench");
//  node.addItem(sum);
//  if (stopped) Timer::start("bench");
//}

//-----------------------------------------
// dnode_array_dbl_stl_sorted_binsrch
//-----------------------------------------
bool my_lt_comp_function(double i,double j) { return (i<j); }

void test_sort_dnode_array_dbl()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_array, vt_double);

  int n = ITEM_COUNT / FIND_DIV;
  //int n = ITEM_COUNT;
  int ai;
  double a(0.0);
  for(int i=0; i < n; i++) {
    ai = (i + 13) % n; // little unsorted
    a = static_cast<double>(ai);
    node.addItem(a);
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  double sum = 0;

  node.sort();

  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addItem(sum);
  if (stopped) Timer::start("bench");
}

//void test_find_dnode_array_dbl_stl_sorted_binsrch()
//{
//  bool stopped = Timer::stop("bench");
//  scDataNode node(ict_array, vt_double);
//
//  int n = ITEM_COUNT / FIND_DIV;
//  int ai;
//  double a(0.0);
//  for(int i=0; i < n; i++) {
//    ai = (i + 13) % n; // little unsorted
//    a = static_cast<double>(ai);
//    node.addItem(a);
//  }
//  if (stopped) Timer::start("bench");
//  //------- BEGIN -------
//
//  double sum = 0;
//
//  if (node.sort())
//  {
//    a = 0.0;
//
//    dnVectorIterator<double> it(std_iterator_cast<double>(node.begin()));
//    dnVectorIterator<double> eposIt(std_iterator_cast<double>(node.end()));
//
//    for(int i=0, epos = node.size(); i < epos; i++) {
//      ai = (i + 13) % n; // little unsorted
//      a = static_cast<double>(ai);
//
//      if (std::binary_search(
//           it,
//           eposIt,
//           a, my_lt_comp_function))
//      {
//        sum += a;
//      }
//    }
//  }
//
//  //-------  END  -------
//  stopped = Timer::stop("bench");
//  node.addItem(sum);
//  if (stopped) Timer::start("bench");
//}

void test_find_dnode_array_dbl_stl_sorted_binsrch_scalar()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_array, vt_double);

  int n = ITEM_COUNT / FIND_DIV;
  int ai;
  double a(0.0);
  for(int i=0; i < n; i++) {
    ai = (i + 13) % n; // little unsorted
    a = static_cast<double>(ai);
    node.addItem(a);
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  double sum = 0;

  if (node.sort())
  {
    a = 0.0;

    dnode::scalar_iterator<double> it(node.scalarBegin<double>());
    dnode::scalar_iterator<double> eposIt(node.scalarEnd<double>());

    for(int i=0, epos = node.size(); i < epos; i++) {
      ai = (i + 13) % n; // little unsorted
      a = static_cast<double>(ai);

      if (std::binary_search(
           it,
           eposIt,
           a, std::less<double>()))
      {
        sum += a;
      }
    }
  }

  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addItem(sum);
  if (stopped) Timer::start("bench");
}

//-----------------------------------------
// dnode_list_int
//-----------------------------------------

void fill_dnode_list_int_stl_sorted_binsrch(int n, scDataNode &node)
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  node = scDataNode(ict_list);
  if (wasRunning)
    Timer::start("bench");
  for(int i=0; i < n; i++) {
    node.addChild(new scDataNode(i % 10));
  }
}

void test_sort_dnode_list_int()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_list);

  int n = ITEM_COUNT / FIND_DIV;
  //int n = ITEM_COUNT;
  for(int i=0; i < n; i++) {
    node.addChild(new scDataNode((i + 13) % n)); // little unsorted
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  //sort<int>(node);
  node.sort<int>();

  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(0.0));
  if (stopped) Timer::start("bench");
}

//void test_find_dnode_list_int_stl_sorted_binsrch()
//{
//  bool stopped = Timer::stop("bench");
//  scDataNode node(ict_list);
//
//  int n = ITEM_COUNT / FIND_DIV;
//  for(int i=0; i < n; i++) {
//    node.addChild(new scDataNode((i + 13) % n)); // little unsorted
//  }
//  if (stopped) Timer::start("bench");
//  //------- BEGIN -------
//
//  int sum = 0;
//
//  node.sort<int>();
//
//  {
//    dnVectorIterator<int> it(std_iterator_cast<int>(node.begin()));
//    dnVectorIterator<int> eposIt(std_iterator_cast<int>(node.end()));
//
//    for(int i=0, epos = node.size(); i < epos; i++) {
//      if (std::binary_search(
//           it,
//           eposIt,
//           i, my_lt_comp_function))
//      {
//        sum += i;
//      }
//    }
//  }
//
//  //-------  END  -------
//  stopped = Timer::stop("bench");
//  node.addChild("sum", new scDataNode(sum));
//  if (stopped) Timer::start("bench");
//}

//void test_find_dnode_list_int_stl_sorted_binsrch_lt()
//{
//  bool stopped = Timer::stop("bench");
//  scDataNode node(ict_list);
//
//  int n = ITEM_COUNT / FIND_DIV;
//  for(int i=0; i < n; i++) {
//    node.addChild(new scDataNode((i + 13) % n)); // little unsorted
//  }
//  if (stopped) Timer::start("bench");
//  //------- BEGIN -------
//
//  int sum = 0;
//
//  node.sort<int>();
//
//  {
//    dnVectorIterator<int> it(std_iterator_cast<int>(node.begin()));
//    dnVectorIterator<int> eposIt(std_iterator_cast<int>(node.end()));
//
//    for(int i=0, epos = node.size(); i < epos; i++) {
//      if (std::binary_search(
//           it,
//           eposIt,
//           i, std::less<int>()))
//      {
//        sum += i;
//      }
//    }
//  }
//
//  //-------  END  -------
//  stopped = Timer::stop("bench");
//  node.addChild("sum", new scDataNode(sum));
//  if (stopped) Timer::start("bench");
//}

void test_find_dnode_list_int_stl_sorted_binsrch_scalar()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_list);

  int n = ITEM_COUNT / FIND_DIV;
  for(int i=0; i < n; i++) {
    node.addChild(new scDataNode((i + 13) % n)); // little unsorted
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  int sum = 0;

  //sort<int>(node);
  node.sort<int>();

  {
    dnode::scalar_iterator<int> it(node.scalarBegin<int>());
    dnode::scalar_iterator<int> eposIt(node.scalarEnd<int>());

    for(int i=0, epos = node.size(); i < epos; i++) {
      if (std::binary_search(
           it,
           eposIt,
           i, std::less<int>()))
      {
        sum += i;
      }
    }
  }

  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_find_dnode_array_int_stl_sorted_binsrch_scalar()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_array, vt_int);

  int n = ITEM_COUNT / FIND_DIV;
  for(int i=0; i < n; i++) {
    node.addItem((i + 13) % n); // little unsorted
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  int sum = 0;

  //sort<int>(node);
  node.sort<int>();

  {
    scDataNode::scalar_iterator<int> it(node.scalarBegin<int>());
    scDataNode::scalar_iterator<int> eposIt(node.scalarEnd<int>());

    for(int i=0, epos = node.size(); i < epos; i++) {
      if (std::binary_search(
           it,
           eposIt,
           i, std::less<int>()))
      {
        sum += i;
      }
    }
  }

  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_find_dnode_array_int_alg()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_array, vt_int);

  int n = ITEM_COUNT / FIND_DIV;
  for(int i=0; i < n; i++) {
    node.addItem((i + 13) % n); // little unsorted
  }
  if (stopped) Timer::start("bench");
  //------- BEGIN -------

  int sum = 0;

  //sort<int>(node);
  //node.sort<int>();

  {
    scDataNode::iterator eposIt = node.end();

    for(int i=0, epos = node.size(); i < epos; i++) {
      if (node.find(i) != eposIt)
      {
        sum += i;
      }
    }
  }

  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_find_dnode_array_int_sorted_binsrch()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_array, vt_int);

  int n = ITEM_COUNT / FIND_DIV;
  for(int i=0; i < n; i++) {
    node.addItem((i + 13) % n); // little unsorted
  }
  if (stopped) Timer::start("bench");

  //------- BEGIN -------
  int sum = 0;

  //sort<int>(node);
  node.sort<int>();

  {
    for(int i=0, epos = node.size(); i < epos; i++) {
      //if (binary_search(node, i))
      if (node.binary_search(i))
      {
        sum += i;
      }
    }
  }

  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

//-----------------------------------------
// test_accum_dnode_array_dbl_visitor
//-----------------------------------------
template<typename ValueType>
class sum_visitor {
public:
  sum_visitor(ValueType &sumRef): m_sumRef(sumRef) { sumRef = 0; }
  void operator()(ValueType v) {
    m_sumRef += v;
  }
protected:
  ValueType &m_sumRef;
};

void fill_dnode_array_dbl(int n, scDataNode &node)
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  node = scDataNode(ict_array, vt_double);
  if (wasRunning)
    Timer::start("bench");
  double a = 0.0;
  for(int i=0; i < n; i++) {
    node.addItem(a / 10.0);
    a += 1.0;
  }
}

//-----------------------------------------
// test_accum_dnode_list_int_visitor
//-----------------------------------------
void fill_dnode_list_int_intmod(int n, scDataNode &node)
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  dtpString str;
  node = scDataNode(ict_list);
  if (wasRunning)
    Timer::start("bench");
  for(int i=0; i < n; i++) {
    node.addChild<int>(i % 10);
  }
}

void test_accum_dnode_list_int_visit()
{
  bool stopped = Timer::stop("bench");
  scDataNode node;
  fill_dnode_list_int_intmod(ITEM_COUNT, node);
  if (stopped) Timer::start("bench");

  //------- BEGIN -------
  //double sum = std::accumulate(it, epos, 0.0);
  int sum;
  sum_visitor<int> a_sum_visitor(sum);
  node.visitVectorValues<int>(a_sum_visitor);

  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild(sum);
  if (stopped) Timer::start("bench");
}

//-----------------------------------------
// dnode_list_val_sorted_binsrch
//-----------------------------------------
void test_find_dnode_list_int_sorted_binsrch()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_list);

  int n = ITEM_COUNT / FIND_DIV;
  for(int i=0; i < n; i++) {
    node.addChild(new scDataNode((i + 13) % n)); // little unsorted
  }
  if (stopped) Timer::start("bench");

  //------- BEGIN -------
  int sum = 0;

  //sort<int>(node);
  node.sort<int>();

  {
    //dnVectorIterator<int> it(std_iterator_cast<int>(node.begin()));
    //dnVectorIterator<int> eposIt(std_iterator_cast<int>(node.end()));

    for(int i=0, epos = node.size(); i < epos; i++) {
      //if (binary_search(node, i))
      if (node.binary_search(i))
      {
        sum += i;
      }
    }
  }

  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild(new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_find_dnode_list_int_sorted_binsrch2()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_list);

  int n = ITEM_COUNT / FIND_DIV;
  for(int i=0; i < n; i++) {
    node.addChild(new scDataNode((i + 13) % n)); // little unsorted
  }
  if (stopped) Timer::start("bench");

  //------- BEGIN -------
  int sum = 0;

  //sort<int>(node);
  node.sort<int>();

  {
    //dnVectorIterator<int> it(std_iterator_cast<int>(node.begin()));
    //dnVectorIterator<int> eposIt(std_iterator_cast<int>(node.end()));

    for(int i=0, epos = node.size(); i < epos; i++) {
      if (node.binarySearchValue(i, dtpSignCompareOp<int>()))
      {
        sum += i;
      }
    }
  }

  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild(new scDataNode(sum));
  if (stopped) Timer::start("bench");
}


//-----------------------------------------
// dnode_list_val_alg
//-----------------------------------------
void test_find_dnode_list_int_alg()
{
  bool stopped = Timer::stop("bench");
  scDataNode node(ict_list);

  int n = ITEM_COUNT / FIND_DIV;
  for(int i=0; i < n; i++) {
    node.addChild(new scDataNode((i + 13) % n)); // little unsorted
  }
  if (stopped) Timer::start("bench");

  //------- BEGIN -------
  int sum = 0;

  {
    scDataNode::iterator eposIt = node.end();

    for(int i=0, epos = node.size(); i < epos; i++) {
      if (node.find(i) != eposIt)
      {
        sum += i;
      }
    }
  }

  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild(new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

//-----------------------------------------
// test_accum_dnode_list_int_alg
//-----------------------------------------
void test_accum_dnode_list_int_alg()
{
  bool stopped = Timer::stop("bench");
  scDataNode node;
  fill_dnode_list_int_mod10(ITEM_COUNT, node);
  if (stopped) Timer::start("bench");

  //------- BEGIN -------
  //double sum = std::accumulate(it, epos, 0.0);
  //int sum = accumulate(node, 0);
  int sum = node.accumulate(0);

  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild(sum);
  if (stopped) Timer::start("bench");
}

//void test_accum_dnode_list_int_scalar()
//{
//  bool stopped = Timer::stop("bench");
//  scDataNode node;
//  fill_dnode_list_int_mod10(ITEM_COUNT, node);
//  if (stopped) Timer::start("bench");
//
//  //------- BEGIN -------
//  //double sum = std::accumulate(it, epos, 0.0);
//  //int sum = accumulate(node, 0);
//  int sum = std::accumulate(node.scalarBegin<int>(), node.scalarEnd<int>(), 0);
//
//  //-------  END  -------
//  stopped = Timer::stop("bench");
//  node.addItem(sum);
//  if (stopped) Timer::start("bench");
//}

//-----------------------------------------
// dnode_map
//-----------------------------------------
void fill_dnode_map(int n, scDataNode &node)
{
  bool wasRunning = Timer::stop("bench");
  dtpString str;
  dmap_init<int, int>(node);
  if (wasRunning)
    Timer::start("bench");
  for(int i=0; i < n; i++) {
    dmap_insert(node, i, i % 10);
  }
}

#ifdef DMAP_SORT_SUPPORTED
void fill_dnode_map_sort(int n, scDataNode &node)
{
  Timer::stop("bench");
  dtpString str;
  dmap_init<int, int>(node);
  Timer::start("bench");
  for(int i=0; i < n; i++) {
    dmap_push_back(node, i, i % 10);
  }
  dmap_sort<int>(node);
}
#endif

void test_insert_dnode_map()
{
  //------- BEGIN -------
  scDataNode node;
  fill_dnode_map(ITEM_COUNT, node);
  //-------  END  -------
}

#ifdef DMAP_SORT_SUPPORTED

void test_insert_dnode_map_sort()
{
  //------- BEGIN -------
  scDataNode node;
  fill_dnode_map_sort(ITEM_COUNT, node);
  //-------  END  -------
}

void test_insert_dnode_map_sort_integrity()
{
  //------- BEGIN -------
  scDataNode node;
  fill_dnode_map_sort(ITEM_COUNT_RELEASE, node);
  BOOST_CHECK(dmap_is_sorted<int>(node));
  //-------  END  -------
}
#endif

void test_accum_dnode_map()
{
  bool stopped = Timer::stop("bench");
  scDataNode node;
  fill_dnode_map(ITEM_COUNT, node);
  if (stopped) Timer::start("bench");
  //------- BEGIN -------
  int sum;
  sum = dmap_accumulate<int>(node, 0);
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_update_dnode_map_key()
{
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_map(ITEM_COUNT, node);
  Timer::start("bench");
  //------- BEGIN -------
  for(int i=0, epos = dmap_size<int>(node); i < epos; i++)
    //node.set(i, 2 * node.get<int>(i));
    dmap_set<int, int>(node, i, 2 * dmap_get_def<int>(node, i, -1));
  //-------  END  -------
}

void test_update_dnode_map_it()
{
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_map(ITEM_COUNT, node);
  Timer::start("bench");
  //------- BEGIN -------
  for(scDataNode::iterator it = dmap_begin<int>(node), epos = dmap_end<int>(node); it != epos; ++it)
    dmap_set<int, int>(node, it, 2 * dmap_get<int, int>(node, it));
  //-------  END  -------
}

void test_update_dnode_map_it_scalar()
{
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_map(ITEM_COUNT, node);
  Timer::start("bench");
  //------- BEGIN -------
  for(scDataNode::scalar_iterator<int> it = dmap_begin_value<int, int>(node), epos = dmap_end_value<int, int>(node); it != epos; ++it)
    it.set(2 * (*it));
  //-------  END  -------
}

template<typename T>
struct dmap_update_trans {
  T operator()(T value) {
    return 2 * value;
  }
};

void test_update_dnode_map_transform()
{
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_map(ITEM_COUNT, node);
  Timer::start("bench");
  //------- BEGIN -------
  //for(int i=0, epos = dmap_size(node); i < epos; i++)
    //node.set(i, 2 * node.get<int>(i));
    //dmap_set(node, i, 2 * dmap_get_def<int>(node, i, -1));
  //dmap_transform<int>(node, dmap_value_iterator(node, dmap_begin(node)), dmap_update_trans<int>());
  //std::transform(dmap_begin_value<int, int>(node), dmap_end_value<int, int>(node), dmap_begin_value<int, int>(node), dmap_update_trans<int>());
  dmap_transform<int, int>(node, dmap_begin_value<int, int>(node), dmap_update_trans<int>());
  //-------  END  -------
}

void test_delete_dnode_map()
{
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_map(ITEM_COUNT / DELETE_DIV, node);
  Timer::start("bench");
  //------- BEGIN -------
  while(!dmap_empty<int>(node))
    dmap_erase<int>(node, dmap_begin<int>(node));
  //-------  END  -------
}

void test_find_dnode_map()
{
  bool stopped = Timer::stop("bench");
  scDataNode node;
  dmap_init<int, int>(node);

  int n = ITEM_COUNT / FIND_DIV;

#ifdef DMAP_SORT_SUPPORTED
  for(int i=0; i < n; i++) {
    dmap_push_back(node, i, i);
  }
  dmap_sort<int>(node);
#else
  for(int i=0; i < n; i++)
    dmap_insert(node, i, i);
#endif

  if (stopped) Timer::start("bench");

  //------- BEGIN -------
  int sum = 0;
  for(int i=0, epos = dmap_size<int>(node); i < epos; i++) {
    //idx = node.indexOfName(names[i]);
    //sum += node.get<int>(idx);
    sum += dmap_find(node, i)->getAs<int>();
  }
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_find_dnode_map_get()
{
  bool stopped = Timer::stop("bench");
  scDataNode node;
  dmap_init<int, int>(node);

  int n = ITEM_COUNT / FIND_DIV;

#ifdef DMAP_SORT_SUPPORTED
  for(int i=0; i < n; i++) {
    dmap_push_back(node, i, i);
  }
  dmap_sort<int>(node);
#else
  for(int i=0; i < n; i++)
    dmap_insert(node, i, i);
#endif

  if (stopped) Timer::start("bench");

  //------- BEGIN -------
  int sum = 0;
  for(int i=0, epos = dmap_size<int>(node); i < epos; i++) {
    sum += dmap_get_def(node, i, -1);
  }
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_size_dnode_map()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;

  dmap_init<int, uint>(node);

  int n = ITEM_COUNT / SIZE_DIV;

  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  for(int i=0; i < n; i++) {
    //node.addChild(names[i], new scDataNode(node.size()));
    //dmap_insert(node, i, static_cast<uint>(dmap_size(node)));
    dmap_push_back(node, i, static_cast<uint>(dmap_size<int>(node)));
  }
  dmap_sort<int>(node);
  //-------  END  -------
}

//-----------------------------------------
// dnode_map_str
//-----------------------------------------
void fill_dnode_map_str(int n, scDataNode &node)
{
  bool wasRunning = Timer::stop("bench");
  dtpString str;
  std::vector<dtpString> names;
  fillnames(0, n, names);
  dmap_init<dtpString, int>(node);
  if (wasRunning)
    Timer::start("bench");
  for(int i=0; i < n; i++) {
    dmap_insert(node, names[i], i % 10);
  }
}

#ifdef DMAP_SORT_SUPPORTED
void fill_dnode_map_str_sort(int n, scDataNode &node)
{
  Timer::stop("bench");
  dtpString str;
  std::vector<dtpString> names;
  fillnames(0, n, names);
  dmap_init<dtpString, int>(node);
  Timer::start("bench");
  for(int i=0; i < n; i++) {
    dmap_push_back(node, names[i], i % 10);
  }
  dmap_sort<dtpString>(node);
}
#endif

void test_insert_dnode_map_str()
{
  //------- BEGIN -------
  scDataNode node;
  fill_dnode_map_str(ITEM_COUNT, node);
  //-------  END  -------
}

#ifdef DMAP_SORT_SUPPORTED

void test_insert_dnode_map_str_sort()
{
  //------- BEGIN -------
  scDataNode node;
  fill_dnode_map_str_sort(ITEM_COUNT, node);
  //-------  END  -------
}

void test_insert_dnode_map_str_sort_integrity()
{
  //------- BEGIN -------
  scDataNode node;
  fill_dnode_map_str_sort(ITEM_COUNT_RELEASE, node);
  BOOST_CHECK(dmap_is_sorted<dtpString>(node));
  //-------  END  -------
}
#endif

void test_accum_dnode_map_str()
{
  bool stopped = Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fillnames(0, ITEM_COUNT, names);
  fill_dnode_map_str(ITEM_COUNT, node);
  if (stopped) Timer::start("bench");
  //------- BEGIN -------
  int sum;
  sum = dmap_accumulate<int>(node, 0);
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_update_dnode_map_str_key()
{
  Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fillnames(0, ITEM_COUNT, names);
  fill_dnode_map_str(ITEM_COUNT, node);
  Timer::start("bench");
  //------- BEGIN -------
  for(int i=0, epos = dmap_size<dtpString>(node); i < epos; i++)
    //node.set(i, 2 * node.get<int>(i));
    dmap_set<dtpString, int>(node, names[i], 2 * dmap_get_def(node, names[i], -1));
  //-------  END  -------
}

void test_update_dnode_map_str_it()
{
  Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;
  fillnames(0, ITEM_COUNT, names);
  fill_dnode_map_str(ITEM_COUNT, node);
  Timer::start("bench");
  //------- BEGIN -------
  for(scDataNode::iterator it = dmap_begin<dtpString>(node), epos = dmap_end<dtpString>(node); it != epos; ++it)
    dmap_set<dtpString, int>(node, it, 2 * dmap_get<dtpString, int>(node, it));
  //-------  END  -------
}

void test_update_dnode_map_str_it_scalar()
{
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_map_str(ITEM_COUNT, node);
  Timer::start("bench");
  //------- BEGIN -------
  for(scDataNode::scalar_iterator<int> it = dmap_begin_value<dtpString, int>(node), epos = dmap_end_value<dtpString, int>(node); it != epos; ++it)
    it.set(2 * (*it));
  //-------  END  -------
}

template<typename T>
struct dnode_map_str_update_trans {
  T operator()(T value) {
    return 2 * value;
  }
};

void test_update_dnode_map_str_transform()
{
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_map_str(ITEM_COUNT, node);
  Timer::start("bench");
  //------- BEGIN -------
  dmap_transform<dtpString, int>(node, dmap_begin_value<dtpString, int>(node), dnode_map_str_update_trans<int>());
  //-------  END  -------
}

void test_delete_dnode_map_str()
{
  Timer::stop("bench");
  scDataNode node;
  fill_dnode_map_str(ITEM_COUNT / DELETE_DIV, node);
  Timer::start("bench");
  //------- BEGIN -------
  while(!dmap_empty<dtpString>(node))
    dmap_erase<dtpString>(node, dmap_begin<dtpString>(node));
  //-------  END  -------
}

void test_find_dnode_map_str()
{
  bool stopped = Timer::stop("bench");
  scDataNode node;
  dmap_init<dtpString, int>(node);
  std::vector<dtpString> names;

  int n = ITEM_COUNT / FIND_DIV;
  fillnames(0, n, names);

#ifdef DMAP_SORT_SUPPORTED
  for(int i=0; i < n; i++) {
    dmap_push_back(node, names[i], i);
  }
  dmap_sort<dtpString>(node);
#else
  for(int i=0; i < n; i++)
    dmap_insert(node, names[i], i);
#endif

  if (stopped) Timer::start("bench");

  //------- BEGIN -------
  int sum = 0;
  for(int i=0, epos = dmap_size<dtpString>(node); i < epos; i++) {
    sum += dmap_find(node, names[i])->getAs<int>();
  }
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_find_dnode_map_str_get()
{
  bool stopped = Timer::stop("bench");
  scDataNode node;
  std::vector<dtpString> names;

  dmap_init<dtpString, int>(node);

  int n = ITEM_COUNT / FIND_DIV;
  fillnames(0, n, names);

#ifdef DMAP_SORT_SUPPORTED
  for(int i=0; i < n; i++) {
    dmap_push_back(node, names[i], i);
  }
  dmap_sort<dtpString>(node);
#else
  for(int i=0; i < n; i++)
    dmap_insert(node, names[i], i);
#endif

  if (stopped) Timer::start("bench");

  //------- BEGIN -------
  int sum = 0;
  for(int i=0, epos = dmap_size<dtpString>(node); i < epos; i++) {
    sum += dmap_get_def(node, names[i], -1);
  }
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_find_dnode_map_str_value()
{
  bool stopped = Timer::stop("bench");
  scDataNode node;
  dmap_init<dtpString, int>(node);
  std::vector<dtpString> names;

  int n = ITEM_COUNT / FIND_DIV;
  fillnames(0, n, names);

#ifdef DMAP_SORT_SUPPORTED
  for(int i=0; i < n; i++) {
    dmap_push_back(node, names[i], i);
  }
  dmap_sort<dtpString>(node);
#else
  for(int i=0; i < n; i++)
    dmap_insert(node, names[i], i);
#endif

  if (stopped) Timer::start("bench");

  //------- BEGIN -------
  int sum = 0;
  for(int i=0, epos = dmap_size<dtpString>(node); i < epos; i++) {
    sum += *(dmap_find_value<dtpString, int>(node, names[i]));
  }
  //-------  END  -------
  stopped = Timer::stop("bench");
  node.addChild("sum", new scDataNode(sum));
  if (stopped) Timer::start("bench");
}

void test_size_dnode_map_str()
{
  bool wasRunning = Timer::isRunning("bench");
  Timer::stop("bench");
  scDataNode node;

  std::vector<dtpString> names;
  int n = ITEM_COUNT / SIZE_DIV;

  dmap_init<dtpString, uint>(node);
  fillnames(0, n, names);

  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  for(int i=0; i < n; i++) {
    dmap_push_back(node, names[i], static_cast<uint>(dmap_size<dtpString>(node)));
  }
  dmap_sort<dtpString>(node);
  //-------  END  -------
}

//-----------------------------------------
// addBench
//-----------------------------------------
template< class F >
void addBench(F fun, const dtpString &testName, scDataNode &output) {
  Timer::reset("bench");
  Timer::start("bench");
  for(int i=1; i <= REPEAT_COUNT; i++)
    fun();
  Timer::stop("bench");
  output.addElement(testName, scDataNode(Timer::getTotal("bench")));
}


void test_bench_fill_vector(scDataNode &output)
{
  output = scDataNode(ict_array, vt_double);
  for(uint i=0, epos = ITEM_COUNT; i != epos; i++)
    output.addItem(static_cast<double>(i) / 10.0);
}

//void test_bench_make_normal_it_stl()
//{
//  Timer::stop("bench");
//  scDataNode dnVect;
//  test_bench_fill_vector(dnVect);
//  std::vector<double> stlVect;
//  std::vector<double> *stlVectPtr;
//  stlVectPtr = &(std_vector_cast<double>(dnVect, stlVect));
//  Timer::start("bench");
//
//  //------- BEGIN -------
//  make_normal_v(*stlVectPtr, *stlVectPtr);
//  //-------  END  -------
//}

//void test_bench_make_normal_gi_stl()
//{
//  Timer::stop("bench");
//  scDataNode dnVect;
//  test_bench_fill_vector(dnVect);
//  std::vector<double> stlVect;
//  std::vector<double> *stlVectPtr;
//  stlVectPtr = &(std_vector_cast<double>(dnVect, stlVect));
//  Timer::start("bench");
//
//  //------- BEGIN -------
//  make_normal_gi<double>(
//    std_generic_iterator_cast<double>(stlVectPtr->begin()),
//    std_generic_iterator_cast<double>(stlVectPtr->end()),
//    std_generic_iterator_cast<double>(stlVectPtr->begin())
//  );
//  //-------  END  -------
//}

//void test_bench_make_normal_gi_dn()
//{
//  Timer::stop("bench");
//  scDataNode dnVect;
//  test_bench_fill_vector(dnVect);
//  Timer::start("bench");
//
//  //------- BEGIN -------
//  make_normal_gi<double>(
//    dn_generic_iterator_cast<double>(dnVect.begin()),
//    dn_generic_iterator_cast<double>(dnVect.end()),
//    dn_generic_iterator_cast<double>(dnVect.begin())
//  );
//  //-------  END  -------
//}

void test_bench_make_normal_scalar()
{
  Timer::stop("bench");
  scDataNode dnVect;
  test_bench_fill_vector(dnVect);
  Timer::start("bench");

  //------- BEGIN -------
  make_normal<double>(
    dnVect.scalarBegin<double>(),
    dnVect.scalarEnd<double>(),
    dnVect.scalarBegin<double>()
  );
  //-------  END  -------
}

//void test_bench_find_min_max_gi_stl()
//{
//  Timer::stop("bench");
//  scDataNode dnVect;
//  test_bench_fill_vector(dnVect);
//  std::vector<double> stlVect;
//  std::vector<double> *stlVectPtr;
//  stlVectPtr = &(std_vector_cast<double>(dnVect, stlVect));
//  Timer::start("bench");
//
//  //------- BEGIN -------
//  double minVal, maxVal;
//
//  find_min_max_gi<double>(
//    std_generic_iterator_cast<double>(stlVectPtr->begin()),
//    std_generic_iterator_cast<double>(stlVectPtr->end()),
//    minVal,
//    maxVal
//  );
//  //-------  END  -------
//}

//void test_bench_find_min_max_gi_dn()
//{
//  Timer::stop("bench");
//  scDataNode dnVect;
//  test_bench_fill_vector(dnVect);
//  Timer::start("bench");
//
//  //------- BEGIN -------
//  double minVal, maxVal;
//
//  find_min_max_gi<double>(
//    dn_generic_iterator_cast<double>(dnVect.begin()),
//    dn_generic_iterator_cast<double>(dnVect.end()),
//    minVal,
//    maxVal
//  );
//  //-------  END  -------
//}

void test_bench_find_min_max_scalar()
{
  Timer::stop("bench");
  scDataNode dnVect;
  test_bench_fill_vector(dnVect);
  Timer::start("bench");

  //------- BEGIN -------
  double minVal, maxVal;

  find_min_max(
    dnVect.scalarBegin<double>(),
    dnVect.scalarEnd<double>(),
    minVal,
    maxVal
  );
  //-------  END  -------
}

//void test_bench_dot_prod_gv_dn()
//{
//  Timer::stop("bench");
//  scDataNode dnVect;
//  test_bench_fill_vector(dnVect);
//  Timer::start("bench");
//
//  //------- BEGIN -------
//  double dotProd;
//
//  dotProd = dot_product_gv<double>(
//    dn_generic_vector_cast<double>(dnVect),
//    dn_generic_vector_cast<double>(dnVect)
//  );
//  //-------  END  -------
//}

//void test_bench_dot_prod_gv_stl()
//{
//  Timer::stop("bench");
//  scDataNode dnVect;
//  test_bench_fill_vector(dnVect);
//  std::vector<double> stlVect;
//  std::vector<double> *stlVectPtr;
//  stlVectPtr = &(std_vector_cast<double>(dnVect, stlVect));
//  Timer::start("bench");
//
//  //------- BEGIN -------
//  double dotProd;
//
//  dotProd = dot_product_gv<double>(
//    std_generic_vector_cast<double>(*stlVectPtr),
//    std_generic_vector_cast<double>(*stlVectPtr)
//  );
//  //-------  END  -------
//}

//void test_bench_dot_prod_gi_dn()
//{
//  Timer::stop("bench");
//  scDataNode dnVect;
//  test_bench_fill_vector(dnVect);
//  Timer::start("bench");
//
//  //------- BEGIN -------
//  double dotProd;
//
//  dotProd = dot_product_gi<double>(
//    dn_generic_iterator_cast<double>(dnVect.begin()),
//    dn_generic_iterator_cast<double>(dnVect.end()),
//    dn_generic_iterator_cast<double>(dnVect.begin())
//  );
//  //-------  END  -------
//}

void test_bench_dot_prod_scalar()
{
  Timer::stop("bench");
  scDataNode dnVect;
  test_bench_fill_vector(dnVect);
  Timer::start("bench");

  //------- BEGIN -------
  double dotProd;

  dotProd = dot_product(
    dnVect.scalarBegin<double>(),
    dnVect.scalarEnd<double>(),
    dnVect.scalarBegin<double>()
  );
  //-------  END  -------
}

//void test_bench_dot_prod_gi_stl()
//{
//  Timer::stop("bench");
//  scDataNode dnVect;
//  test_bench_fill_vector(dnVect);
//  std::vector<double> stlVect;
//  std::vector<double> *stlVectPtr;
//  stlVectPtr = &(std_vector_cast<double>(dnVect, stlVect));
//  Timer::start("bench");
//
//  //------- BEGIN -------
//  double dotProd;
//
//  dotProd = dot_product_gi<double>(
//    std_generic_iterator_cast<double>(stlVectPtr->begin()),
//    std_generic_iterator_cast<double>(stlVectPtr->end()),
//    std_generic_iterator_cast<double>(stlVectPtr->begin())
//  );
//  //-------  END  -------
//}

void test_serialize_via_bion_list_ss()
{
  bool wasRunning = Timer::stop("bench");
  scDataNode node;
  //std::vector<dtpString> names;
  //fill_dnode_parent_val_by_idx(ITEM_COUNT, node, names);
  fill_dnode_list_val_by_idx(ITEM_COUNT, node);

  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  std::string str;
  dnode node2;

  for(uint i=0; i < SERIALIZE_REPEAT_COUNT; i++)
  {
      std::stringstream s;
      dnBionWriter<std::stringstream> writer(s);

      writer.write(node);
      s.seekg(0, std::ios::beg);

      dnBionProcessor proc(node2);
      BionReader<std::stringstream, dnBionProcessor> reader(s, proc);
      reader.process();
  }
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  node.addChild("sum", new scDataNode(0.0));
  if (wasRunning)
    Timer::start("bench");
}

void test_serialize_via_bion_list_mem()
{
  bool wasRunning = Timer::stop("bench");
  scDataNode node;
  //std::vector<dtpString> names;
  //fill_dnode_parent_val_by_idx(ITEM_COUNT, node, names);
  fill_dnode_list_val_by_idx(ITEM_COUNT, node);

  std::string str;
  dnode node2;
  const uint BUF_SIZE = 1024000;
  char *buffer = new char[BUF_SIZE];

  BionMemoryOutputStream out(buffer, BUF_SIZE);
  dnBionWriter<BionMemoryOutputStream> writer(out);
  size_t dataSize;
  dnBionProcessor proc(node2);

  BionInputMemoryStream input(buffer, BUF_SIZE);
  BionReader<BionInputMemoryStream, dnBionProcessor> reader(input, proc);

  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------

  for(uint i=0; i < SERIALIZE_REPEAT_COUNT; i++)
  {
      out.reset();
      writer.write(node);

      input.setLimit(out.tellg());

      input.reset();
      reader.process();
  }
  //-------  END  -------
  wasRunning = Timer::stop("bench");

  delete []buffer;

  node.addChild("sum", new scDataNode(0.0));
  if (wasRunning)
    Timer::start("bench");
}

void test_serialize_via_bion_list_wr()
{
  bool wasRunning = Timer::stop("bench");
  scDataNode node;
  //std::vector<dtpString> names;
  //fill_dnode_parent_val_by_idx(ITEM_COUNT, node, names);
  fill_dnode_list_val_by_idx(ITEM_COUNT, node);

  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  std::string str;
  dnode node2;

  for(uint i=0; i < SERIALIZE_REPEAT_COUNT; i++)
  {
      std::stringstream s;
      dnBionWriter<std::stringstream> writer(s);

      writer.write(node);
  }
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  node.addChild("sum", new scDataNode(0.0));
  if (wasRunning)
    Timer::start("bench");
}

void test_serialize_via_bion_list_rd_ss()
{
  bool wasRunning = Timer::stop("bench");
  scDataNode node;
  //std::vector<dtpString> names;
  //fill_dnode_parent_val_by_idx(ITEM_COUNT, node, names);
  fill_dnode_list_val_by_idx(ITEM_COUNT, node);

  std::stringstream s;
  dnBionWriter<std::stringstream> writer(s);

  writer.write(node);

  std::string str;
  dnode node2;

  str = s.str();
  s.seekg(0, s.end);
  size_t length = s.tellg();
  s.seekg(0, std::ios::beg);
  BionInputMemoryStream input(const_cast<char *>(str.c_str()), length);

  if (wasRunning)
    Timer::start("bench");

  //------- BEGIN -------
  for(uint i=0; i < SERIALIZE_REPEAT_COUNT; i++)
  {
      s.clear();
      s.seekg(0, std::ios::beg);
      dnBionProcessor proc(node2);
      BionReader<std::stringstream, dnBionProcessor> reader(s, proc);
      reader.process();
  }
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  node.addChild("sum", new scDataNode(0.0));
  if (wasRunning)
    Timer::start("bench");
}

void test_serialize_via_bion_list_rd_mem()
{
  bool wasRunning = Timer::stop("bench");
  scDataNode node;
  //std::vector<dtpString> names;
  //fill_dnode_parent_val_by_idx(ITEM_COUNT, node, names);
  fill_dnode_list_val_by_idx(ITEM_COUNT, node);

  std::stringstream s;
  dnBionWriter<std::stringstream> writer(s);

  writer.write(node);

  std::string str;
  dnode node2;

  str = s.str();
  s.seekg(0, s.end);
  size_t length = s.tellg();
  s.seekg(0, std::ios::beg);
  BionInputMemoryStream input(const_cast<char *>(str.c_str()), length);

  if (wasRunning)
    Timer::start("bench");

  //------- BEGIN -------
  for(uint i=0; i < SERIALIZE_REPEAT_COUNT; i++)
  {
      input.reset();
      dnBionProcessor proc(node2);
      BionReader<BionInputMemoryStream, dnBionProcessor> reader(input, proc);
      reader.process();
  }
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  node.addChild("sum", new scDataNode(0.0));
  if (wasRunning)
    Timer::start("bench");
}

void test_serialize_via_string_list()
{
  bool wasRunning = Timer::stop("bench");
  scDataNode node;
  //std::vector<dtpString> names;
  //fill_dnode_parent_val_by_idx(ITEM_COUNT, node, names);
  fill_dnode_list_val_by_idx(ITEM_COUNT, node);

  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  std::string str;
  dnode node2;

  dnSerializer serializer;
  for(uint i=0; i < SERIALIZE_REPEAT_COUNT; i++)
  {
      serializer.convToString(node, str);
      serializer.convFromString(str, node2);
  }
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  node.addChild("sum", new scDataNode(0.0));
  if (wasRunning)
    Timer::start("bench");
}

void test_serialize_via_string_list_wr()
{
  bool wasRunning = Timer::stop("bench");
  scDataNode node;
  //std::vector<dtpString> names;
  //fill_dnode_parent_val_by_idx(ITEM_COUNT, node, names);
  fill_dnode_list_val_by_idx(ITEM_COUNT, node);

  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  std::string str;
  dnode node2;

  dnSerializer serializer;
  for(uint i=0; i < SERIALIZE_REPEAT_COUNT; i++)
  {
      serializer.convToString(node, str);
  }
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  node.addChild("sum", new scDataNode(0.0));
  if (wasRunning)
    Timer::start("bench");
}

void test_serialize_via_string_list_rd()
{
  bool wasRunning = Timer::stop("bench");
  scDataNode node;
  //std::vector<dtpString> names;
  //fill_dnode_parent_val_by_idx(ITEM_COUNT, node, names);
  fill_dnode_list_val_by_idx(ITEM_COUNT, node);

  std::string str;
  dnSerializer serializer;
  serializer.convToString(node, str);

  if (wasRunning)
    Timer::start("bench");
  //------- BEGIN -------
  dnode node2;

  for(uint i=0; i < SERIALIZE_REPEAT_COUNT; i++)
  {
      serializer.convFromString(str, node2);
  }
  //-------  END  -------
  wasRunning = Timer::stop("bench");
  node.addChild("sum", new scDataNode(0.0));
  if (wasRunning)
    Timer::start("bench");
}

//--------------------------------------------------------------------------------------------------
// Test cases
//--------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_perf_dnode_map_sort)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

#ifdef DMAP_SORT_SUPPORTED
#ifdef TEST_BENCH_MAP_SORT
  addBench(boost::bind(test_insert_dnode_map_sort_integrity), "insert_dnode_map_sort", results);
#endif
#endif

  BOOST_TEST_MESSAGE("Benchmark - end...");
  showBenchResults("Benchmark - results:", results);
}

void fixBenchSize(scDataNode &results, const dtpString &nameInsert, const dtpString &nameSize)
{
  if (results.get<uint64>(nameSize) > results.get<uint64>(nameInsert))
    results.set(nameSize, results.get<uint64>(nameSize) - results.get<uint64>(nameInsert));
  else
    results.set(nameSize, 0);
}

BOOST_AUTO_TEST_CASE(test_perf_dnode_map)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_insert_dnode_map), "insert_dnode_map", results);
  addBench(boost::bind(test_insert_dnode_map_sort), "insert_dnode_map_sort", results);
  addBench(boost::bind(test_accum_dnode_map), "accum_dnode_map", results);
  addBench(boost::bind(test_update_dnode_map_key), "update_dnode_map_key", results);
  addBench(boost::bind(test_update_dnode_map_it), "update_dnode_map_it", results);
  addBench(boost::bind(test_update_dnode_map_it_scalar), "update_dnode_map_it_scalar", results);
  addBench(boost::bind(test_update_dnode_map_transform), "update_dnode_map_transform", results);
  addBench(boost::bind(test_delete_dnode_map), "delete_dnode_map", results);
  addBench(boost::bind(test_find_dnode_map), "find_dnode_map", results);
  addBench(boost::bind(test_find_dnode_map_get), "find_dnode_map_get", results);
  addBench(boost::bind(test_size_dnode_map), "size_dnode_map", results);
  fixBenchSize(results, "insert_dnode_map", "size_dnode_map");

  BOOST_TEST_MESSAGE("Benchmark - end...");
  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_dnode_map_str)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_insert_dnode_map_str), "insert_dnode_map_str", results);
  addBench(boost::bind(test_insert_dnode_map_str_sort), "insert_dnode_map_str_sort", results);
  addBench(boost::bind(test_accum_dnode_map_str), "accum_dnode_map_str", results);
  addBench(boost::bind(test_update_dnode_map_str_key), "update_dnode_map_str_key", results);
  addBench(boost::bind(test_update_dnode_map_str_it), "update_dnode_map_str_it", results);
  addBench(boost::bind(test_update_dnode_map_str_it_scalar), "update_dnode_map_str_it_scalar", results);
  addBench(boost::bind(test_update_dnode_map_str_transform), "update_dnode_map_str_transform", results);
  addBench(boost::bind(test_delete_dnode_map_str), "delete_dnode_map_str", results);
  addBench(boost::bind(test_find_dnode_map_str), "find_dnode_map_str", results);
  addBench(boost::bind(test_find_dnode_map_str_value), "find_dnode_map_str_value", results);
  addBench(boost::bind(test_find_dnode_map_str_get), "find_dnode_map_str_get", results);
  addBench(boost::bind(test_size_dnode_map_str), "size_dnode_map_str", results);
  fixBenchSize(results, "insert_dnode_map_str", "size_dnode_map_str");

  BOOST_TEST_MESSAGE("Benchmark - end...");
  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_vector)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_insert_vector), "insert_vector", results);
  addBench(boost::bind(test_accum_vector), "accum_vector", results);
  addBench(boost::bind(test_update_vector), "update_vector", results);
  addBench(boost::bind(test_delete_vector), "delete_vector", results);
  addBench(boost::bind(test_find_vector), "find_vector", results);
  addBench(boost::bind(test_size_vector), "size_vector", results);
  fixBenchSize(results, "insert_vector", "size_vector");

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_list)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_insert_list), "insert_list", results);
  addBench(boost::bind(test_accum_list), "accum_list", results);
  addBench(boost::bind(test_update_list), "update_list", results);
  addBench(boost::bind(test_delete_list), "delete_list", results);
  addBench(boost::bind(test_find_list), "find_list", results);
  addBench(boost::bind(test_size_list), "size_list", results);
  fixBenchSize(results, "insert_list", "size_list");

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_map)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_insert_map), "insert_map", results);
  addBench(boost::bind(test_accum_map), "accum_map", results);
  addBench(boost::bind(test_update_map_it), "update_map_it", results);
  addBench(boost::bind(test_update_map_key), "update_map_key", results);
  addBench(boost::bind(test_delete_map), "delete_map", results);
  addBench(boost::bind(test_find_map), "find_map", results);
  addBench(boost::bind(test_size_map), "sizemap_map", results);
  fixBenchSize(results, "insert_map", "sizemap_map");

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_vector_of_double)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_insert_dvector), "insert_dvector", results);
  addBench(boost::bind(test_accum_dvector), "accum_dvector", results);
  addBench(boost::bind(test_update_dvector), "update_dvector", results);
  addBench(boost::bind(test_delete_dvector), "delete_dvector", results);
  addBench(boost::bind(test_find_dvector), "find_dvector", results);
  addBench(boost::bind(test_size_dvector), "size_dvector", results);
  fixBenchSize(results, "insert_dvector", "size_dvector");

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_map_any)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_insert_map_any), "insert_map_any", results);
  addBench(boost::bind(test_accum_map_any), "accum_map_any", results);
  addBench(boost::bind(test_update_map_any), "update_map_any", results);
  addBench(boost::bind(test_delete_map_any), "delete_map_any", results);
  addBench(boost::bind(test_find_map_any), "find_map_any", results);
  addBench(boost::bind(test_size_map_any), "size_map_any", results);
  fixBenchSize(results, "insert_map_any", "size_map_any");

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_dnode_parent_val_by_idx)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_insert_dnode_parent_val_by_idx), "insert_dnode_parent_val_by_idx", results);
  addBench(boost::bind(test_accum_dnode_parent_val_by_idx), "accum_dnode_parent_val_by_idx", results);

  addBench(boost::bind(test_update_dnode_parent_val_by_idx), "update_dnode_parent_val_by_idx", results);

  addBench(boost::bind(test_delete_dnode_parent_val_by_idx), "delete_dnode_parent_val_by_idx", results);
  addBench(boost::bind(test_find_dnode_parent_val_by_idx), "find_dnode_parent_val_by_idx", results);
  addBench(boost::bind(test_size_dnode_parent_val_by_idx), "size_dnode_parent_val_by_idx", results);
  fixBenchSize(results, "insert_dnode_parent_val_by_idx", "size_dnode_parent_val_by_idx");

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_dnode_list_val_by_idx)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_insert_dnode_list_val_by_idx), "insert_dnode_list_val_by_idx", results);
  addBench(boost::bind(test_accum_dnode_list_val_by_idx), "accum_dnode_list_val_by_idx", results);
  addBench(boost::bind(test_update_dnode_list_val_by_idx), "update_dnode_list_val_by_idx", results);
  addBench(boost::bind(test_delete_dnode_list_val_by_idx), "delete_dnode_list_val_by_idx", results);
  addBench(boost::bind(test_find_dnode_list_val_by_idx), "find_dnode_list_val_by_idx", results);
  addBench(boost::bind(test_size_dnode_list_val_by_idx), "size_dnode_list_val_by_idx", results);
  fixBenchSize(results, "insert_dnode_list_val_by_idx", "size_dnode_list_val_by_idx");

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_dnode_array_val_by_idx)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_insert_dnode_array_val_by_idx), "insert_dnode_array_val_by_idx", results);
  addBench(boost::bind(test_accum_dnode_array_val_by_idx), "accum_dnode_array_val_by_idx", results);
  addBench(boost::bind(test_update_dnode_array_val_by_idx), "update_dnode_array_val_by_idx", results);
  addBench(boost::bind(test_delete_dnode_array_val_by_idx), "delete_dnode_array_val_by_idx", results);
  addBench(boost::bind(test_find_dnode_array_val_by_idx), "find_dnode_array_val_by_idx", results);
  addBench(boost::bind(test_find_dnode_array_val_by_idx_gen), "find_dnode_array_val_by_idx_gen", results);
  addBench(boost::bind(test_size_dnode_array_val_by_idx), "size_dnode_array_val_by_idx", results);
  fixBenchSize(results, "insert_dnode_array_val_by_idx", "size_dnode_array_val_by_idx");

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_dnode_parent_by_dn_iter)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

//not supported:
//addBench(boost::bind(test_insert_dnode_parent_stl), "insert_dnode_parent_stl", results);

//  addBench(boost::bind(test_accum_dnode_parent_stl), "accum_dnode_parent_stl", results);

//not supported:
//addBench(boost::bind(test_update_dnode_parent_stl), "update_dnode_parent_stl", results);

//not supported:
//  addBench(boost::bind(test_delete_dnode_parent_stl), "delete_dnode_parent_stl", results);
//  addBench(boost::bind(test_find_dnode_parent_stl), "find_dnode_parent_stl", results);

// not a STL version
//  addBench(boost::bind(test_size_dnode_parent_stl), "size_dnode_parent_stl", results);

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_dnode_parent_scalar)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_accum_dnode_parent_scalar), "accum_dnode_parent_scalar", results);
  addBench(boost::bind(test_find_dnode_parent_scalar_at), "find_dnode_parent_scalar_at", results);

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_dnode_list_by_dn_iter)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

// not a STL version
//  addBench(boost::bind(test_insert_dnode_list_stl), "insert_dnode_list_stl", results);
//  addBench(boost::bind(test_accum_dnode_list_stl), "accum_dnode_list_stl", results);
// not a STL version
//  addBench(boost::bind(test_update_dnode_list_stl), "update_dnode_list_stl", results);
// not a STL version
//  addBench(boost::bind(test_delete_dnode_list_stl), "delete_dnode_list_stl", results);

  addBench(boost::bind(test_find_dnode_list_alg_node), "find_dnode_list_alg_node", results);

// not a STL version
//  addBench(boost::bind(test_size_dnode_list_stl), "size_dnode_list_stl", results);

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_dnode_list_scalar)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

// not a STL version
//  addBench(boost::bind(test_insert_dnode_list_scalar), "insert_dnode_list_scalar", results);
  addBench(boost::bind(test_accum_dnode_list_int_scalar), "accum_dnode_list_int_scalar", results);
// not a STL version
//  addBench(boost::bind(test_update_dnode_list_scalar), "update_dnode_list_scalar", results);
// not a STL version
//  addBench(boost::bind(test_delete_dnode_list_scalar), "delete_dnode_list_scalar", results);

  addBench(boost::bind(test_find_dnode_list_int_scalar), "find_dnode_list_int_scalar", results);

// not a STL version
//  addBench(boost::bind(test_size_dnode_list_scalar), "size_dnode_list_scalar", results);

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_dnode_array_dbl_val)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_insert_dnode_array_dbl_val), "insert_dnode_array_dbl_val", results);
  addBench(boost::bind(test_accum_dnode_array_dbl_val), "accum_dnode_array_dbl_val", results);
  addBench(boost::bind(test_update_dnode_array_dbl_val), "update_dnode_array_dbl_val", results);
  addBench(boost::bind(test_delete_dnode_array_dbl_val), "delete_dnode_array_dbl_val", results);
  addBench(boost::bind(test_find_dnode_array_dbl_val), "find_dnode_array_dbl_val", results);
  addBench(boost::bind(test_find_dnode_array_dbl_val_gen), "find_dnode_array_dbl_val_gen", results);
  addBench(boost::bind(test_size_dnode_array_dbl_val), "size_dnode_array_dbl_val", results);
  fixBenchSize(results, "insert_dnode_array_dbl_val", "size_dnode_array_dbl_val");

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_dnode_array_dbl_gen)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_accum_dnode_array_dbl_scalar), "accum_dnode_array_dbl_scalar", results);
  addBench(boost::bind(test_find_dnode_array_dbl_scalar), "find_dnode_array_dbl_scalar", results);
  addBench(boost::bind(test_find_dnode_array_dbl_alg_node), "find_dnode_array_dbl_alg_node", results);
  addBench(boost::bind(test_find_dnode_array_dbl_alg), "find_dnode_array_dbl_alg", results);

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_dnode_parent_qref)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_accum_dnode_parent_qref), "accum_dnode_parent_qref", results);
  addBench(boost::bind(test_update_dnode_parent_qref), "update_dnode_parent_qref", results);
  addBench(boost::bind(test_delete_dnode_parent_qref), "delete_dnode_parent_qref", results);
  addBench(boost::bind(test_find_dnode_parent_qref), "find_dnode_parent_qref", results);
  addBench(boost::bind(test_size_dnode_parent_qref), "size_dnode_parent_qref", results);
  //correction inside test size function
  //fixBenchSize(results, "insert_dnode_parent_qref", "size_dnode_parent_qref");

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_dnode_parent_ref)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_accum_dnode_parent_ref), "accum_dnode_parent_ref", results);
  addBench(boost::bind(test_update_dnode_parent_ref), "update_dnode_parent_ref", results);
  addBench(boost::bind(test_delete_dnode_parent_ref), "delete_dnode_parent_ref", results);
  addBench(boost::bind(test_find_dnode_parent_ref), "find_dnode_parent_ref", results);
  // nothing special - see parent_qref version
  //addBench(boost::bind(test_size_dnode_parent_ref), "size_dnode_parent_ref", results);

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_dnode_parent_elem)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_accum_dnode_parent_elem), "accum_dnode_parent_elem", results);
  addBench(boost::bind(test_update_dnode_parent_elem), "update_dnode_parent_elem", results);
  addBench(boost::bind(test_delete_dnode_parent_elem), "delete_dnode_parent_elem", results);
  addBench(boost::bind(test_find_dnode_parent_elem), "find_dnode_parent_elem", results);
  // nothing special - see parent_qref version
  //addBench(boost::bind(test_size_dnode_parent_elem), "size_dnode_parent_elem", results);

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_array_dbl_stl_it_cast)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  //addBench(boost::bind(test_accum_dnode_array_dbl_stl_it_cast), "accum_dnode_array_dbl_stl_it_cast", results);

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_sort)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_sort_dnode_array_dbl), "sort_dnode_array_dbl", results);
  //addBench(boost::bind(test_find_dnode_array_dbl_stl_sorted_binsrch), "find_dnode_array_dbl_stl_sorted_binsrch", results);
  addBench(boost::bind(test_find_dnode_array_dbl_stl_sorted_binsrch_scalar), "find_dnode_array_dbl_stl_sorted_binsrch_scalar", results);

  addBench(boost::bind(test_sort_dnode_list_int), "sort_dnode_list_val_by_idx", results);
  //addBench(boost::bind(test_find_dnode_list_int_stl_sorted_binsrch), "find_dnode_list_int_stl_sorted_binsrch", results);
  //addBench(boost::bind(test_find_dnode_list_int_stl_sorted_binsrch_lt), "find_dnode_list_int_stl_sorted_binsrch_lt", results);
  addBench(boost::bind(test_find_dnode_list_int_stl_sorted_binsrch_scalar), "find_dnode_list_int_stl_sorted_binsrch_scalar", results);

  addBench(boost::bind(test_find_dnode_array_int_stl_sorted_binsrch_scalar), "find_dnode_array_int_stl_sorted_binsrch_scalar", results);
  addBench(boost::bind(test_find_dnode_array_int_alg), "find_dnode_array_int_alg", results);
  addBench(boost::bind(test_find_dnode_array_int_sorted_binsrch), "find_dnode_array_int_sorted_binsrch", results);

  addBench(boost::bind(test_find_dnode_list_int_alg), "find_dnode_list_int_alg", results);

  addBench(boost::bind(test_find_dnode_list_int_sorted_binsrch), "find_dnode_list_int_sorted_binsrch", results);
  addBench(boost::bind(test_find_dnode_list_int_sorted_binsrch2), "find_dnode_list_int_sorted_binsrch2", results);
  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(test_perf_accum_alg)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_accum_dnode_list_int_visit), "accum_dnode_list_int_visit", results);
  addBench(boost::bind(test_accum_dnode_list_int_alg), "accum_dnode_list_int_alg", results);
  addBench(boost::bind(test_accum_dnode_list_int_scalar), "accum_dnode_list_int_scalar", results);

  showBenchResults("Benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(bench_make_normal)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

//  addBench(boost::bind(test_bench_make_normal_gi_dn), "make_normal_gi_dn", results);
//  addBench(boost::bind(test_bench_make_normal_it_stl), "make_normal_it_stl", results);
//  addBench(boost::bind(test_bench_make_normal_gi_stl), "make_normal_gi_stl", results);
  addBench(boost::bind(test_bench_make_normal_scalar), "make_normal_scalar", results);

  showBenchResults("VAlgorithm benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(bench_find_min_max)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

//  addBench(boost::bind(test_bench_find_min_max_gi_stl), "find_min_max_gi_stl", results);
//  addBench(boost::bind(test_bench_find_min_max_gi_dn), "find_min_max_gi_dn", results);
  addBench(boost::bind(test_bench_find_min_max_scalar), "find_min_max_scalar", results);

  showBenchResults("VAlgorithm benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(bench_dot_prod)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

//  addBench(boost::bind(test_bench_dot_prod_gi_stl), "dot_prod_gi_stl", results);
//  addBench(boost::bind(test_bench_dot_prod_gi_dn), "dot_prod_gi_dn", results);

//  addBench(boost::bind(test_bench_dot_prod_gv_stl), "dot_prod_gv_stl", results);
//  addBench(boost::bind(test_bench_dot_prod_gv_dn), "dot_prod_gv_dn", results);
  addBench(boost::bind(test_bench_dot_prod_scalar), "dot_prod_scalar", results);

  showBenchResults("VAlgorithm benchmark - results:", results);
}

BOOST_AUTO_TEST_CASE(bench_serialize)
{
  BOOST_TEST_MESSAGE("Benchmark - start...");
  scDataNode results(ict_parent);

  addBench(boost::bind(test_serialize_via_bion_list_ss), "serialize_via_bion_list_ss", results);
  addBench(boost::bind(test_serialize_via_bion_list_mem), "serialize_via_bion_list_mem", results);
  addBench(boost::bind(test_serialize_via_bion_list_rd_ss), "serialize_via_bion_list_rd_ss", results);
  addBench(boost::bind(test_serialize_via_bion_list_rd_mem), "serialize_via_bion_list_rd_mem", results);
  addBench(boost::bind(test_serialize_via_bion_list_wr), "serialize_via_bion_list_wr", results);
  addBench(boost::bind(test_serialize_via_string_list), "serialize_via_string_list", results);
  addBench(boost::bind(test_serialize_via_string_list_rd), "serialize_via_string_list_rd", results);
  addBench(boost::bind(test_serialize_via_string_list_wr), "serialize_via_string_list_wr", results);

  showBenchResults("Serialization benchmark - results:", results);
}
