#include <vector>

#include "base/btypes.h"
#include "dtp/dnode.h"
#include "dtp/dnode_cast.h"

using namespace dtp;

BOOST_AUTO_TEST_CASE(test_scalar)
{
  dnode test;

  BOOST_CHECK(test.isNull());
  BOOST_CHECK(!test.isParent());
  BOOST_CHECK(!test.isContainer());
  BOOST_CHECK(!test.isList());
  BOOST_CHECK(test.size() == 0);
  BOOST_CHECK(test.empty());

  test = 2;
  BOOST_CHECK(test.getAsInt() == 2);

  // test access by template
  BOOST_CHECK(test.getAs<int>() == 2);

  test = "ala";
  BOOST_CHECK(test.getAsString() == dtpString("ala"));

  test = false;
  BOOST_CHECK(!test.getAsBool());

  test = 2.0;
  BOOST_CHECK(test.getAsFloat() > 0.0);

  BOOST_CHECK(!test.isNull());
  BOOST_CHECK(!test.isParent());
  BOOST_CHECK(!test.isContainer());
  BOOST_CHECK(!test.isList());
  BOOST_CHECK(test.size() == 0);
  BOOST_CHECK(test.empty());

  test.clear();
  BOOST_CHECK(test.isNull());
}

BOOST_AUTO_TEST_CASE(test_short_type_impl)
{
  dnode test;

  BOOST_CHECK(test.isNull());
  BOOST_CHECK(!test.isParent());
  BOOST_CHECK(!test.isContainer());
  BOOST_CHECK(!test.isList());
  BOOST_CHECK(test.size() == 0);
  BOOST_CHECK(test.empty());

  // test write
  test.setAs<xdouble>(2.0);

  BOOST_CHECK(!test.isNull());

  // test read
  xdouble b = test.getAs<xdouble>();
  BOOST_CHECK(b > 0.0);

  // test iterator read
  dnode testList(ict_list);
  testList.push_back(2);

  dnode::iterator it = testList.begin();
  b += it->getAs<xdouble>();

  // test iterator write
  *it = static_cast<xdouble>(3.0);

  // test constuctor
  dnode c(static_cast<xdouble>(2.0));

  // test assignment
  xdouble d(333.0);
  c = d;

  // test array
  dnode testArray(ict_array, vt_xdouble);
  //dnode testArray(ict_array, dnArrayTag(xdouble));

  testArray.addItem(d);

  BOOST_CHECK(!testArray.empty());

  // test item access
  d += testArray.get<xdouble>(0);

  // test write
  testArray.set(0, d);

  // test cast
  dnode c1(static_cast<xdouble>(222.0));
  c1.forceValueType(vt_float);
  BOOST_CHECK(c1.getAs<float>() > 220.0);
  BOOST_CHECK(c1.getValueType() == vt_float);

  // test compare
  dnode i1(2);
  dnode i2(static_cast<uint>(2));
  dnode i3(static_cast<int>(2));
  BOOST_CHECK(i1 != i2);
  //BOOST_CHECK(i1.dataEqualTo(i2));
  BOOST_CHECK(i1.isEqualTo(i2));
  BOOST_CHECK(i1.isEqualTo(i3));

  // test convert
  dnode s1("test");
  s1.assignValueFrom(i1);
  BOOST_CHECK(s1.isEqualTo(i1));
  BOOST_CHECK(s1.getValueType() == vt_string);

  // test destructor
  s1.clear();
  BOOST_CHECK(s1.isNull());
  BOOST_CHECK(s1.empty());
}

BOOST_AUTO_TEST_CASE(test_array)
{
  // test array
  dnode testArray(ict_array, vt_xdouble);

  xdouble d(2.0);
  testArray.addItem(d);

  BOOST_CHECK(!testArray.empty());

  // test item access
  d += testArray.get<xdouble>(0);

  // test write
  testArray.set(0, d);

  // test string array
  dnode testArrayStr(ict_array, vt_datanode);

  testArrayStr.addItem<dtpString>("Test1");
  testArrayStr.addItem<dtpString>("Test2");
  dtpString s("Test2");
  //BOOST_CHECK(testArrayStr.indexOfValue<dtpString>("Test2") != dnode::npos);
  BOOST_CHECK(testArrayStr.indexOfValue<dtpString>(s) != dnode::npos);
}

BOOST_AUTO_TEST_CASE(test_stl_iterator)
{
  dnode test(ict_array, vt_int);
  test.push_back(1);
  test.push_back(2);
  test.push_back(3);

  dnode::scalar_iterator<int> it = test.scalarBegin<int>();
  dnode::scalar_iterator<int> epos = test.scalarEnd<int>();

  int sum = 0;
  for(; it != epos; ++it)
    sum += *it;

  BOOST_CHECK(sum > 0);

  int sum2 = std::accumulate(
      test.scalarBegin<int>(),
      test.scalarEnd<int>(),
      0);

  BOOST_CHECK(sum == sum2);

  it = test.scalarBegin<int>();
  it.set(2);

  BOOST_CHECK(*test.scalarBegin<int>() == 2);
}
