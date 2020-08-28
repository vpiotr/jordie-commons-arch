/////////////////////////////////////////////////////////////////////////////
// Name:        dtpTestUitls.cpp
// Purpose:     Test DTPLib utils
// Author:      Piotr Likus
// Modified by:
// Created:     22/09/2012
/////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include "dtp/interface.h"

using namespace dtp;

// Functions that can be used:
//   BOOST_TEST_MESSAGE("balance-0: " << toString(balance));
//   BOOST_CHECK(balance == dec::decimal2(0));

class FooIntf: public scInterface {
public:
  static uint64 getInterfaceId() { 
    return 0x232C3F40006344e3; 
  }

  virtual double add(double a, double b) = 0;
};

class AcntIntf {
public:
  static uint64 getInterfaceId() { 
    return 0x232C3F4000634411; 
  }
  virtual double sub(double a, double b) = 0;
};

class Foo: public FooIntf {
public:
  virtual double add(double a, double b) { return a + b; }
};

class Boo: public scInterfacedObject {
public:
  virtual scInterface *queryInterface(scInterfaceId intId) { 
    if (intId == FooIntf::getInterfaceId()) 
      return &m_foo; 
    else
      return DTP_NULL; 
  }
  
private:
  Foo m_foo;
};

class Account: public AcntIntf {
public:
  virtual double sub(double a, double b) { return a - b; }
  scInterface *queryInterface(scInterfaceId intId) { return DTP_NULL; }
};


BOOST_AUTO_TEST_CASE(test_interface)
{
  // test w/ interface based on scInterface and object based on interface 
  std::auto_ptr<scInterfacedObject> objPtr(new Foo);

  FooIntf *foo = query_cast<FooIntf>(objPtr.get());

  BOOST_CHECK(foo != DTP_NULL);
  double a = foo->add(2.1, 3.1);

  // test w/ interface based on scInterface and object based on scInterfacedObject (uses delegated interface)
  Boo boo;

  FooIntf *booIntf = query_cast<FooIntf>(&boo);

  BOOST_CHECK(booIntf != DTP_NULL);
  a += booIntf->add(2.1, 3.1);

  BOOST_CHECK(a >= 10.0);

  // test unknown interface on scInterfacedObject 
  AcntIntf *acntIntf = query_cast<AcntIntf>(&boo);
  BOOST_CHECK(acntIntf == DTP_NULL);

  // test interface w/o base class & object based on that interface
  Account acnt;
  acntIntf = query_cast<AcntIntf>(&acnt);

  BOOST_CHECK(acntIntf != DTP_NULL);
  a += acntIntf->sub(2.1, 3.1);
}
