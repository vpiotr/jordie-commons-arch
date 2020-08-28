#include <vector>
#include "base/btypes.h"
#include "base/algorithm.h"
#include "base/utils.h"
#include "dtp/dnode_cast.h"
#include "dtp/dnode_algorithm.h"

using namespace base;
using namespace dtp;

// Functions that can be used:
//   BOOST_TEST_MESSAGE("balance-0: " << toString(balance));
//   BOOST_CHECK(balance == dec::decimal2(0));

BOOST_AUTO_TEST_CASE(stl_algorithms)
{
  dnode arrTest;
  arrTest.setAsArray(vt_int);
  for(int i=1; i <= 5; i++)
    arrTest.addItemAsInt(i);

  // overloaded algorithm - special version for dnode
  int sum = arrTest.accumulate<int>(0);
  BOOST_CHECK(sum == 15);

  // STL algorithm - invoked with casted dnode iterators
  //int cnt = (int)std::count(dnVectorIterator<int>(arrTest.begin()),
  //                          dnVectorIterator<int>(arrTest.end()),
  //                          5);

  int cnt = (int)std::count(arrTest.scalarBegin<int>(),
                            arrTest.scalarEnd<int>(),
                            5);

  BOOST_CHECK(cnt == 1);
}

BOOST_AUTO_TEST_CASE(test_vector_cast)
{
  // treat data node as array
  dnode arrTest;
  arrTest.setAsArray(vt_int);
  for(int i=1; i <= 5; i++)
    arrTest.addItemAsInt(i % 2);
  std::vector<int> vint;
  // vint is provided as a buffer - in case arrTest is NOT a array of ints
  // otherwise it is not used
  std::vector<int> &vintref = std_vector_cast(arrTest, vint);

  int cnt = (int)std::count(vintref.begin(),
                            vintref.end(),
                            1);
  BOOST_CHECK(cnt == 3);
  // test direct array access
  int sum2 = vintref[0] + vintref[4];
  BOOST_CHECK(sum2 == 2);

  // test array access to non-array data node with cast from int to byte
  std::vector<byte> vbyte;
  dnode arrTest2(ict_list);
  for(int i=1; i <= 5; i++)
    arrTest2.addElement(dnode(i % 2));
  std::vector<byte> &vbyteref2 = std_vector_cast(arrTest2, vbyte);

  int cnt2 = (int)std::count(vbyteref2.begin(),
                             vbyteref2.end(),
                             0);

  BOOST_CHECK(cnt2 == 2);
}

BOOST_AUTO_TEST_CASE(node_utils)
{
  // test items -> values
  dnode node = dnode::explode(";", "first;second;third");
  BOOST_CHECK(node.size() == 3);

  // test items -> names
  dnode node2 = dnode::explode(";", "First;Second;Third", true);
  BOOST_CHECK(node2.size() == 3);
  BOOST_CHECK(node2.hasChild("Third"));

  // test merge
  node.merge(node2);
  BOOST_CHECK(node.size() == 6);
  BOOST_CHECK(node2.size() == 3);

  // test transfers
  node2.eatValueFrom(node);
  BOOST_CHECK(node2.size() == 6);
  BOOST_CHECK(node.empty());
  node.copyFrom(node2);
  BOOST_CHECK(node.size() == 6);

  node = dnode::explode("-x-", "First-x-Second-x-Third-x-End", true);
  node2 = dnode(ict_parent);
  BOOST_CHECK(node.size() == 4);
  BOOST_CHECK(node2.size() == 0);

  node2.transferChildrenFrom(node);
  BOOST_CHECK(node.size() == 0);
  BOOST_CHECK(node2.size() == 4);
  node.copyChildrenFrom(node2);
  BOOST_CHECK(node.size() == 4);
  BOOST_CHECK(node2.size() == 4);
}

BOOST_AUTO_TEST_CASE(test_valgs)
{
  dnode vector(ict_array, vt_double);
  for(uint i=0, epos = 100; i != epos; i++)
    vector.addItemAsDouble(static_cast<double>(i) / 10.0);

  std::vector<double> vstd;
  vstd.resize(vector.size());
  vstd = std_vector_cast(vector, vstd);

  make_normal<double>(
    vector.scalarBegin<double>(),
    vector.scalarEnd<double>(),
    vector.scalarBegin<double>()
  );

  make_normal(
    vstd.begin(),
    vstd.end(),
    vstd.begin()
  );

  //make_normal_gi<double>(
  //  dn_generic_iterator_cast<double>(vector.begin()),
  //  dn_generic_iterator_cast<double>(vector.end()),
  //  dn_generic_iterator_cast<double>(vector.begin())
  //);

  //make_normal_gi<double>(
  //  std_generic_iterator_cast<double>(vstd.begin()),
  //  std_generic_iterator_cast<double>(vstd.end()),
  //  std_generic_iterator_cast<double>(vstd.begin())
  //);

  BOOST_TEST_MESSAGE("Normalized DN vector: ");
  for(uint i=0, epos = 100; i != epos; i++)
    BOOST_TEST_MESSAGE(toString(i)+dtpString(": ")+toString(vector.getDouble(i)));

  BOOST_TEST_MESSAGE("Normalized STL vector: ");
  for(uint i=0, epos = 100; i != epos; i++)
    BOOST_TEST_MESSAGE(toString(i)+dtpString(": ")+toString(vstd[i]));

  double ddot1 =
    dot_product(
      vector.scalarBegin<double>(),
      vector.scalarEnd<double>(),
      vector.scalarBegin<double>()
    );

  BOOST_TEST_MESSAGE(dtpString("Double dot product-DN: ")+toString(ddot1));

  double ddot2 =
    dot_product(
      vstd.begin(),
      vstd.end(),
      vstd.begin()
    );

  BOOST_TEST_MESSAGE(dtpString("Double dot product-STL: ")+toString(ddot1));

  //double ddot =
  //  dot_product_gi<double>(
  //    std_generic_iterator_cast<double>(vstd.begin()),
  //    std_generic_iterator_cast<double>(vstd.end()),
  //    std_generic_iterator_cast<double>(vstd.begin())
  //  );

  //BOOST_TEST_MESSAGE(dtpString("Double dot product: ")+toString(ddot));

  //double dot2 =
  //  dot_product_gv<double>(
  //    dn_generic_vector_cast<double>(vector),
  //    std_generic_vector_cast<double>(vstd)
  //  );

  //BOOST_TEST_MESSAGE(dtpString("Dot product using generic vector cast: ")+toString(dot2));
}

dnode toupleInt2(int a, int b)
{
  dnode r(ict_list);
  dnode result;
  r.addChild(a);
  r.addChild(b);
  result = move(r);
  return result;
}

BOOST_AUTO_TEST_CASE(test_move)
{
  dnode vector(ict_array, vt_double);
  for(uint i=0, epos = 100; i != epos; i++)
    vector.addItemAsDouble(static_cast<double>(i) / 10.0);

  dnode newVector(base::move(vector));
  BOOST_CHECK(vector.empty());
  BOOST_CHECK(!newVector.empty());

  vector = base::move(newVector);
  BOOST_CHECK(!vector.empty());
  BOOST_CHECK(newVector.empty());

  vector = toupleInt2(2,3);
  BOOST_CHECK(!vector.empty());
}

BOOST_AUTO_TEST_CASE(test_init)
{
  // init dnode
  dnode vector(ict_array, vt_double);

  init(vector);
  BOOST_CHECK(vector.isNull());

  // init int
  int a = 5;
  init(a);

  BOOST_CHECK(a == 0);

  // init string
  dtpString str = "This is a test string";

  BOOST_CHECK(str.length() > 0);
  init(str);
  BOOST_CHECK(str.length() == 0);
}