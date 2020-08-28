/////////////////////////////////////////////////////////////////////////////
// Name:        dnode3.h
// Project:     dtpLib
// Purpose:     Universal XML-DOM-node-like structure - data node.
//              API version 3.0
// Author:      Piotr Likus
// Modified by:
// Created:     01/08/2012
/////////////////////////////////////////////////////////////////////////////

#ifndef _DTPDATANODE3_H__
#define _DTPDATANODE3_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file dnode3.h
\brief Universal XML-DOM-node-like structure - data node.

dnode
Is a dynamic typing solution for C++.
Allows you to keep the following data types inside an object:
- bool, int, uint, uint64, int64, float, double, xdouble, void_ptr
Each node can be a scalar, a map key-string->node (parent), a vector of nodes (list) or a vector of scalar values (array).
The last one is most optimized container for vector operations as it is implemented as vector<value-type>.

Can be universally iterated using native iterator:

\code
 dnode element;
 for(dnode::const_iterator it = v.begin(); it != v.end(); ++it) {
   //.. do something with "*it" (value wrapper, direct value access)
   a = it->get<int>();
   // or access item as node (eventually uses temporary object - when working with arrays)
   a = it->getAsNode(element).getAs<int>();
   // or read as full node first (requires temporary object)
   it->getItem(element);
   a = element.getAs<int>();
 }

 // modify item value ignoring container type (array, parent, list):
 v.at(123)->setAs(12.33); // creates iterator with bridge dependent on container type (2 objs)
 cout << v.at("balance")->getAsDouble();
\endcode

Or using STL-compatible, scalar-only iterator:

\code
  std::accumulate(
    dnVect.scalarBegin<double>(),
    dnVect.scalarEnd<double>(),
    0
  );

 // modify item value ignoring container type (array, parent, list):
 *v.scalarAt<double>(123).set(12.33);
 cout << v.scalarAt<double>("balance");
\endcode

Values can be stored in vector(array) and accessed using universal, direct methods - here as array of doubles:
\code
 dnode v(ict_array, vt_double);

 // ...fill array somehow here

 double dv;
 for(int i = 0, epos = v.size(); i != epos; i++) {
   dv = v.get<double>(i);

   //.. do something with "dv"
   dv += 1.0;

   // write changes back:
   v.set(i, dv);
 }
\endcode

Access using integer pos / getNode / setNode:
\code
dnode node;
dnode nodePtr;
for(dnode::size_type i = 0; i != v.size(); i++) {
   nodePtr = v.getNodePtr(i, node);
   //.. do something with "nodePtr"
   // write changes back using setNode:
   v.setNode(i, *nodePtr);
}
\endcode

Name patterns used in dnode:
 - element: value is accessed using temporary object, abstract access independent from container type
 - node (get/set): access items with optional temporary object usage
 - item: value is stored in typed array and cannot be accessed as node
 - parent: container, nodes stored as double map [name -> value, int -> value],
     most general container
 - list: container, nodes are stored as vector of pointers to nodes, various item types
     used when name of item is not used
 - array: container, nodes are stored as vector of values, optimized for scalar types,
     single, scalar value type or data node
 - init: initialization during construction
 - copy: copying during assign

 Structure can be traversed using:
 - scanner - see scan() method
 - visitor - see visit_values() method. it is using templates like:
      template<typename ValueType>
      struct visitor {
        ValueType sum;
        visitor(): sum(0) {}
        void operator()(ValueType v) {
          sum += v;
        }
      }
 - for_each & other algorithm functions
*/

// ----------------------------------------------------------------------------
// Configuration
// ----------------------------------------------------------------------------
// Enable parent support
/// define to use object pools for internal dnode containers
#define DATANODE_POOL_CONTAINERS
#define DATANODE_POOL_BRIDGE

// define if you have C++11 support
#ifdef DTP_CPP11
#define DATANODE_CPP11
#endif

/// define to use plain pointers instead of smart for child map / parent dnode
#define DATANODE_CHILD_MAP_NOT_SMART
/// define to create an item if not found
#define DATANODE_AUTO_ADD_ON_SET
/// define to use std::vector<dnode *> instead of boost::ptr_vector
#define DATANODE_CHILD_INDEX_VECTOR_STD
// define to have more RTTI information in build
#define DATANODE_ADD_RTTI

// enable to use 'unordered_map' for parent children
//#define DATANODE_UNORDERED_ENABLED sloooow....

// enable to use std::map for parent children indexed by integer
//#define DATANODE_DBL_MAP

#ifdef DATANODE_CPP11
// define to have initialization list support
#define DATANODE_STATIC_ASSERT_STD
#define DATANODE_INIT_LIST
#endif

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
//stl
#include <map>
#include <vector>
#include <functional>

//C++11
#ifdef DATANODE_STATIC_ASSERT_STD
#include <type_traits>
#endif

#ifdef DATANODE_INIT_LIST
#include <initializer_list>
#endif

//boost
#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#ifdef DTP_COMP_VS
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

#include <boost/variant.hpp>

#ifdef DTP_COMP_VS
#pragma warning( pop )
#endif

#ifdef DATANODE_POOL_CONTAINERS
#include "boost/pool/pool_alloc.hpp"
#endif

#ifdef DATANODE_UNORDERED_ENABLED
#include <boost/unordered_map.hpp>
#endif

// base
#include "base/string.h"
#include "base/algorithm.h"
#include "base/bmath.h"
#include "base/opool.h"
#include "base/date.h"
#include "base/static_assert.h"
#include "base/utils.h"

//sc
#include "dtp/details/dtypes.h"
#include "dtp/details/bin_search.h"
#include "dtp/details/utils.h"
#include "dtp/traits.h"
//#include "dtp/dmath.h"

namespace dtp {

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Forward class definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Class definitions
// ----------------------------------------------------------------------------

class dnode;

namespace Details {
class dnArray;
class dnArrayBase;
};

// ----------------------------------------------------------------------------
// dnode - section begin
// ----------------------------------------------------------------------------

// tree structure for data hierarchies
class dnode;
class dnValue;
//class dnValueBridge;

typedef dnode *dnodePtr;

namespace Details {
typedef boost::shared_ptr<dnode> dnChildTransporter;
typedef DTP_UNIQUE_PTR(dtpString) dtpStringGuard;
}; // Details

typedef boost::variant<int, uint, double, float, bool, uint64, int64, xdouble, void_ptr> dnValueStorage;

enum dnValueType {
  vt_null = 0, vt_parent, vt_array,
  vt_byte, vt_int, vt_uint, vt_int64, vt_uint64,
  vt_string, vt_bool,
  vt_float, vt_double, vt_xdouble,
  vt_vptr,
  vt_date, vt_time, vt_datetime
  //, vt_object - used to store objects - see vt_parent for implementation
  //, (?) vt_blob - later, used to store block of bytes
};

#define vt_datanode vt_null
#define vt_first vt_null
#define vt_last vt_datetime
#define vt_undefined 1000
#define vt_list 999

enum dnInitContainerType {
    ict_parent = 1,
    ict_list = 2,
    ict_array
};

enum dnPos {
    dnpBegin,
    dnpEnd
};

dtpString getValueTypeName(dnValueType accessType);

// ----------------------------------------------------------------------------
// Errors
// ----------------------------------------------------------------------------

/// data node error
class dnError: public std::runtime_error {
public:
  dnError(const std::string s)
    : std::runtime_error(s)
    { }
};

class dnNotImplementedError: public dnError {
public:
  dnNotImplementedError(): dnError("Not implemented!") {}
};

class dnNullValueCastError: public dnError {
public:
  dnNullValueCastError(): dnError("Null value cast") {}
};

class dnNullValueAccessError: public dnError {
public:
  dnNullValueAccessError(dnValueType accessType): 
#ifdef DATANODE_ADD_RTTI
      dnError("Null value accessed as "+dtp::getValueTypeName(accessType)) 
#else
      dnError("Null value accessed as "+toString(accessType)) 
#endif
      {}
};

// ----------------------------------------------------------------------------
// dnScanner
// ----------------------------------------------------------------------------
/// Base class for data node scanners - used for traversing data node structure (in SAX way).
class dnScanner {
public:
  dnScanner() {};
  virtual ~dnScanner() {}
  virtual void start() { }
  virtual void stop() { }
  virtual void nodeOpen(const dnode &node) { }
  virtual void nodeClose(const dnode &node) { }
  virtual void nodeValue(const dnode &node) { }
};

// ----------------------------------------------------------------------------
// dnValueMeta
// ----------------------------------------------------------------------------

namespace Details {

class dnChildColnBaseIntf {
public:
  typedef uint size_type;
  virtual bool supportsAccessByName() const = 0;
  virtual dnode &getByName(const dtpString &name) = 0;
  virtual const dnode &getByNameR(const dtpString &name) const = 0;
  virtual dnode *peekChild(const dtpString &name) = 0;
  virtual const dnode *peekChildR(const dtpString &name) const = 0;
  virtual dnode &at(int pos) = 0;
  virtual const dnode &at(int pos) const = 0;
  virtual size_type size() const = 0;
  virtual bool isList() const = 0;
};

class dnChildColnBase;

template <typename T>
struct dnValueMeta {
  typedef T value_type;
  enum Options { is_object = 0 };
  typedef value_type object_type;
  typedef value_type return_type;
  typedef const return_type const_return_type;
};

template <>
struct dnValueMeta<dtpString> {
  enum Options { is_object = 1 };
  typedef dtpString object_type;
  typedef object_type return_type;
  typedef const return_type const_return_type;
};

template <>
struct dnValueMeta<char *> {
  enum Options { is_object = 1 };
  typedef dtpString object_type;
  typedef object_type return_type;
  typedef const return_type const_return_type;
};

template <>
struct dnValueMeta<dnode> {
  enum Options { is_object = 1 };
  typedef dnode object_type;
  typedef object_type return_type;
  typedef const return_type const_return_type;
};

/*
template <unsigned N>
struct dnValueMeta<char const[N]> {
  enum Options { is_object = 1 };
  typedef dtpString object_type;
  typedef object_type return_type;
  typedef const return_type const_return_type;
};
*/

// ----------------------------------------------------------------------------
// dnValueTypeMeta
// ----------------------------------------------------------------------------
template <typename ValueType>
class dnValueTypeMeta {
public:
  enum Options { item_type = vt_undefined }; // type undefined - requires more then this meta class
};

template <>
class dnValueTypeMeta<byte> {
public:
  enum Options { item_type = vt_byte };
};

template <>
class dnValueTypeMeta<int> {
public:
  enum Options { item_type = vt_int };
};

template <>
class dnValueTypeMeta<uint> {
public:
  enum Options { item_type = vt_uint };
};

template <>
class dnValueTypeMeta<int64> {
public:
  enum Options { item_type = vt_int64 };
};

template <>
class dnValueTypeMeta<uint64> {
public:
  enum Options { item_type = vt_uint64 };
};

template <>
class dnValueTypeMeta<bool> {
public:
  enum Options { item_type = vt_bool };
};

template <>
class dnValueTypeMeta<float> {
public:
  enum Options { item_type = vt_float };
};

template <>
class dnValueTypeMeta<double> {
public:
  enum Options { item_type = vt_double };
};

template <>
class dnValueTypeMeta<xdouble> {
public:
  enum Options { item_type = vt_xdouble };
};

template <>
class dnValueTypeMeta<void_ptr> {
public:
  enum Options { item_type = vt_vptr };
};

template <>
class dnValueTypeMeta<dtpString> {
public:
  enum Options { item_type = vt_string };
};

template <>
class dnValueTypeMeta<char *> {
public:
  enum Options { item_type = vt_string };
};

template <>
class dnValueTypeMeta<const char *> {
public:
  enum Options { item_type = vt_string };
};

template <unsigned N>
class dnValueTypeMeta<char const[N]> {
public:
  enum Options { item_type = vt_string };
};

template <>
class dnValueTypeMeta<dnode> {
public:
  enum Options { item_type = vt_datanode };
};


// ----------------------------------------------------------------------------
// dnValueTypeIdMeta
// ----------------------------------------------------------------------------
template <int TypeId>
struct dnValueTypeIdMeta {
  typedef void native_type;
};

template <>
struct dnValueTypeIdMeta<vt_bool> {
  typedef bool native_type;
};

template <>
struct dnValueTypeIdMeta<vt_byte> {
  typedef byte native_type;
};

template <>
struct dnValueTypeIdMeta<vt_int> {
  typedef int native_type;
};

template <>
struct dnValueTypeIdMeta<vt_uint> {
  typedef uint native_type;
};

template <>
struct dnValueTypeIdMeta<vt_int64> {
  typedef int64 native_type;
};

template <>
struct dnValueTypeIdMeta<vt_uint64> {
  typedef uint64 native_type;
};

template <>
struct dnValueTypeIdMeta<vt_xdouble> {
  typedef xdouble native_type;
};

template <>
struct dnValueTypeIdMeta<vt_double> {
  typedef double native_type;
};

template <>
struct dnValueTypeIdMeta<vt_float> {
  typedef float native_type;
};

template <>
struct dnValueTypeIdMeta<vt_string> {
  typedef dtpString native_type;
};

template <>
struct dnValueTypeIdMeta<vt_time> {
  typedef double native_type;
};

template <>
struct dnValueTypeIdMeta<vt_date> {
  typedef double native_type;
};

template <>
struct dnValueTypeIdMeta<vt_datetime> {
  typedef double native_type;
};

template <>
struct dnValueTypeIdMeta<vt_vptr> {
  typedef void* native_type;
};

// ----------------------------------------------------------------------------
// dnValueCompareMeta
// ----------------------------------------------------------------------------
template <int TypeId>
struct dnValueCompareMeta {
  enum Options { can_compare_dir = 1 };
};

template <>
struct dnValueCompareMeta<vt_string> {
  enum Options { can_compare_dir = 0 };
};

template <>
struct dnValueCompareMeta<vt_parent> {
  enum Options { can_compare_dir = 0 };
};

template <>
struct dnValueCompareMeta<vt_array> {
  enum Options { can_compare_dir = 0 };
};

// ----------------------------------------------------------------------------
// dnValueMetaCanCompareDir
// ----------------------------------------------------------------------------
template<typename ValueType, bool Value = (dnValueMeta<ValueType>::is_object == 0)>
struct dnValueMetaCanCompareDir: public dtpSelector<Value>
{
};

// ----------------------------------------------------------------------------
// dnValueMetaIsObject
// ----------------------------------------------------------------------------
template<typename ValueType, bool Value = (dnValueMeta<ValueType>::is_object == 1)>
struct dnValueMetaIsObject: public dtpSelector<Value>
{
};

// ----------------------------------------------------------------------------
// dnValueMetaIsString
// ----------------------------------------------------------------------------
template<typename ValueType, bool Value = (static_cast<dnValueType>(dnValueTypeMeta<ValueType>::item_type) == vt_string)>
struct dnValueMetaIsString: public dtpSelector<Value>
{
};

// ----------------------------------------------------------------------------
// dnValueInitializer
// ----------------------------------------------------------------------------
template <typename ValueType>
struct dnValueInitializer {
  static void init(dnValueStorage &storage, const ValueType &value) {
    storage = value;
  }
};

template <>
struct dnValueInitializer<dtpString> {
  static void init(dnValueStorage &storage, const dtpString &value) {
    storage = (void *)(new dtpString(value));
  }
};

template <>
struct dnValueInitializer<const char *> {
  static void init(dnValueStorage &storage, const char *value) {
    storage = (void *)(new dtpString(value));
  }
};

template <>
struct dnValueInitializer<char *> {
  static void init(dnValueStorage &storage, char *value) {
    storage = (void *)(new dtpString(value));
  }
};

template <unsigned N>
struct dnValueInitializer<char const[N]> {
  static void init(dnValueStorage &storage, char const value[]) {
    storage = (void *)(new dtpString(value));
  }
};

// ----------------------------------------------------------------------------
// dnValueObjectDestructor
// ----------------------------------------------------------------------------
template<class T>
struct dnValueObjectDestructor {
  static void execute(dnValueStorage &storage) {
      T *ptr = static_cast<T *>(boost::get<void_ptr>(storage));
      delete ptr;
  }
};

// ----------------------------------------------------------------------------
// dnValueDestructor
// ----------------------------------------------------------------------------
template <int TypeId>
struct dnValueDestructor {
  static void execRelease(dnValueStorage &storage, dnValueType valueType) {
    dnValueDestructor<TypeId - 1>::execRelease(storage, valueType);
  }
  static void execReleaseNoInit(dnValueStorage &storage, dnValueType valueType) {
    dnValueDestructor<TypeId - 1>::execReleaseNoInit(storage, valueType);
  }
};

template <>
struct dnValueDestructor<vt_null> {
  static void execRelease(dnValueStorage &storage, dnValueType valueType) {
    // do nothing
    static_assert(vt_null == vt_first, "vt_null must be first in enumeration");
  }
  static void execReleaseNoInit(dnValueStorage &storage, dnValueType valueType) {
    // do nothing
    static_assert(vt_null == vt_first, "vt_null must be first in enumeration");
  }
};

template <>
struct dnValueDestructor<vt_string> {

  enum Options { value_type_id = vt_string };
  typedef dtpString value_type;

  static void execRelease(dnValueStorage &storage, dnValueType valueType) {
    if (valueType == static_cast<dnValueType>(value_type_id)) {
      dnValueObjectDestructor<value_type>::execute(storage);
      storage = (void *)(DTP_NULL);
    } else {
      dnValueDestructor<static_cast<int>(value_type_id) - 1>::execRelease(storage, valueType);
    }
  }

  static void execReleaseNoInit(dnValueStorage &storage, dnValueType valueType) {
    if (valueType == static_cast<dnValueType>(value_type_id)) {
      dnValueObjectDestructor<value_type>::execute(storage);
    } else {
      dnValueDestructor<static_cast<int>(value_type_id) - 1>::execReleaseNoInit(storage, valueType);
    }
  }
};

template <>
struct dnValueDestructor<vt_parent> {

  enum Options { value_type_id = vt_parent };
  typedef dnChildColnBase value_type;

  static void execRelease(dnValueStorage &storage, dnValueType valueType) {
    if (valueType == static_cast<dnValueType>(value_type_id)) {
      dnValueObjectDestructor<value_type>::execute(storage);
      storage = (void *)(DTP_NULL);
    } else {
      dnValueDestructor<static_cast<int>(value_type_id) - 1>::execRelease(storage, valueType);
    }
  }

  static void execReleaseNoInit(dnValueStorage &storage, dnValueType valueType) {
    if (valueType == static_cast<dnValueType>(value_type_id)) {
      dnValueObjectDestructor<value_type>::execute(storage);
    } else {
      dnValueDestructor<static_cast<int>(value_type_id) - 1>::execReleaseNoInit(storage, valueType);
    }
  }
};

template <>
struct dnValueDestructor<vt_array> {

  enum Options { value_type_id = vt_array };
  typedef dnArray value_type;

  static void execRelease(dnValueStorage &storage, dnValueType valueType) {
    if (valueType == static_cast<dnValueType>(value_type_id)) {
      dnValueObjectDestructor<value_type>::execute(storage);
      storage = (void *)(DTP_NULL);
    } else {
      dnValueDestructor<static_cast<int>(value_type_id) - 1>::execRelease(storage, valueType);
    }
  }

  static void execReleaseNoInit(dnValueStorage &storage, dnValueType valueType) {
    if (valueType == static_cast<dnValueType>(value_type_id)) {
      dnValueObjectDestructor<value_type>::execute(storage);
    } else {
      dnValueDestructor<static_cast<int>(value_type_id) - 1>::execReleaseNoInit(storage, valueType);
    }
  }
};

// ----------------------------------------------------------------------------
// dnValueReader
// ----------------------------------------------------------------------------
template <typename ValueType>
struct dnValueReader {
  typedef ValueType native_type;
  static typename Details::dnValueMeta<ValueType>::return_type getValue(dnValueStorage &storage) { return boost::get<native_type>(storage); }
  static typename Details::dnValueMeta<ValueType>::const_return_type getValue(const dnValueStorage &storage) { return boost::get<native_type>(storage); }
};

template <>
struct dnValueReader<dtpString> {
  typedef dtpString native_type;
  typedef native_type &read_type;
  typedef const native_type &const_read_type;

  static read_type getValue(dnValueStorage &storage) {
    return *(static_cast<dtpString *>(boost::get<void_ptr>(storage)));
  }

  static const_read_type getValue(const dnValueStorage &storage) {
    return *(static_cast<dtpString *>(boost::get<void_ptr>(storage)));
  }
};

// ----------------------------------------------------------------------------
// dnValueWriter
// ----------------------------------------------------------------------------
template <typename ValueType>
struct dnValueWriter {
  typedef ValueType value_type;
  static void store(dnValueStorage &storage, const ValueType &newValue) {
    storage = newValue;
  }
};

template <>
struct dnValueWriter<char *> {
  typedef char *value_type;
  static void store(dnValueStorage &storage, const value_type &newValue) {
    dtpString *ptr = (dtpString *)(boost::get<void_ptr>(storage));
    if (ptr == DTP_NULL) {
      storage = (void *)(new dtpString(newValue));
    } else {
      *ptr = newValue;
    }
  }
};

template <>
struct dnValueWriter<dtpString> {
  typedef dtpString value_type;
  static void store(dnValueStorage &storage, const value_type &newValue) {
    dtpString *ptr = static_cast<dtpString *>(boost::get<void_ptr>(storage));
    if (ptr == DTP_NULL) {
      storage = (void *)(new dtpString(newValue));
    } else {
      *ptr = newValue;
    }
  }
};

// ----------------------------------------------------------------------------
// dnValueCaster
// ----------------------------------------------------------------------------
template <typename ValueType>
struct dnValueCaster {
  typedef ValueType native_type;
  typedef typename Details::dnValueMeta<ValueType>::return_type return_type;
  typedef typename Details::dnValueMeta<ValueType>::const_return_type const_return_type;

  static return_type cast(const dnValueStorage &storage, int srcType)
  {
      throw std::runtime_error(dtpString("Conversion failed, target type: ")+toString(srcType));
      return 0;
  }

};

template <>
struct dnValueCaster<dtpString> {
  typedef dtpString native_type;
  typedef Details::dnValueMeta<native_type>::return_type return_type;

  static return_type cast(const dnValueStorage &storage, int srcType)
  {
    using namespace Details;

    typedef dtpString return_type;
    switch (srcType) {
      case vt_string: {
        return dnValueReader< dnValueTypeIdMeta<vt_string>::native_type >::getValue(storage);
      }

      case vt_null:
      case vt_parent:
      case vt_array:
      case vt_vptr:
        return "";

      case vt_bool: {
        bool vbool = //boost::get<bool>(storage);
          dnValueReader< dnValueTypeIdMeta<vt_bool>::native_type >::getValue(storage);
        return (vbool?dtpString("T"):dtpString("F"));
      }
      case vt_int:
      case vt_byte: {
        int vint = //boost::get<int>(storage);
          dnValueReader< dnValueTypeIdMeta<vt_int>::native_type >::getValue(storage);
        return toString(vint);
      }
      case vt_uint: {
        uint vuint = //boost::get<uint>(storage);
          dnValueReader< dnValueTypeIdMeta<vt_uint>::native_type >::getValue(storage);
        return toString(vuint);
      }
      case vt_int64: {
        int64 vint64 = //boost::get<int64>(storage);
          dnValueReader< dnValueTypeIdMeta<vt_int64>::native_type >::getValue(storage);
        return toString(vint64);
      }
      case vt_uint64: {
        uint64 vuint64 = //boost::get<uint64>(storage);
          dnValueReader< dnValueTypeIdMeta<vt_uint64>::native_type >::getValue(storage);
        return toString(vuint64);
      }
      case vt_date: {
        double vdouble = //boost::get<double>(storage);
          dnValueReader< dnValueTypeIdMeta<vt_date>::native_type >::getValue(storage);
        return dateToIsoStr(vdouble);
      }
      case vt_time: {
        double vdouble = //boost::get<double>(storage);
          dnValueReader< dnValueTypeIdMeta<vt_time>::native_type >::getValue(storage);
        return timeToIsoStr(vdouble);
      }
      case vt_datetime: {
        double vdouble = //boost::get<double>(storage);
          dnValueReader< dnValueTypeIdMeta<vt_datetime>::native_type >::getValue(storage);
        return dateTimeToIsoStr(vdouble);
      }
      case vt_float: {
        float vfloat = //boost::get<float>(storage);
          dnValueReader< dnValueTypeIdMeta<vt_float>::native_type >::getValue(storage);
        return toString(vfloat);
      }
      case vt_double: {
        double vdouble = //boost::get<double>(storage);
          dnValueReader< dnValueTypeIdMeta<vt_double>::native_type >::getValue(storage);
        return toString(vdouble);
      }
      case vt_xdouble: {
        xdouble vxdouble = //boost::get<xdouble>(storage);
          dnValueReader< dnValueTypeIdMeta<vt_xdouble>::native_type >::getValue(storage);
        return toString(vxdouble);
      }
      default:
        throw dnError(dtpString("Uknown value type: "+toString(srcType)));
    } // switch
  } // function

}; // struct


template <>
struct dnValueCaster<xdouble> {
  typedef xdouble native_type;
  typedef Details::dnValueMeta<native_type>::return_type return_type;

  static return_type cast(const dnValueStorage &storage, int srcType)
  {
    using namespace Details;

    typedef xdouble return_type;
    switch (srcType) {
      case vt_xdouble:
        return dnValueReader< dnValueTypeIdMeta<vt_xdouble>::native_type >::getValue(storage);
      case vt_double:
      case vt_date:
      case vt_time:
      case vt_datetime:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_double>::native_type >::getValue(storage));
      case vt_float:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_float>::native_type >::getValue(storage));
      case vt_int:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_int>::native_type >::getValue(storage));
      case vt_uint:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_uint>::native_type >::getValue(storage));
      case vt_int64:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_int64>::native_type >::getValue(storage));
      case vt_uint64:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_uint64>::native_type >::getValue(storage));
      case vt_string:
        return stringToXDouble(dnValueReader< dnValueTypeIdMeta<vt_string>::native_type >::getValue(storage));
      case vt_null:
        throw dnNullValueCastError();
      default:
        return static_cast<xdouble>(stringToInt64(dnValueCaster<dtpString>::cast(storage, srcType)));
    } // switch
  } // function

}; // struct

template <>
struct dnValueCaster<double> {
  typedef double native_type;
  typedef Details::dnValueMeta<native_type>::return_type return_type;

  static return_type cast(const dnValueStorage &storage, int srcType)
  {
    typedef native_type return_type;
    switch (srcType) {
      case vt_double:
      case vt_date:
      case vt_time:
      case vt_datetime:
        return dnValueReader< dnValueTypeIdMeta<vt_double>::native_type >::getValue(storage);
      case vt_xdouble:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_xdouble>::native_type >::getValue(storage));
      case vt_float:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_float>::native_type >::getValue(storage));
      case vt_int:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_int>::native_type >::getValue(storage));
      case vt_uint:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_uint>::native_type >::getValue(storage));
      case vt_int64:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_int64>::native_type >::getValue(storage));
      case vt_uint64:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_uint64>::native_type >::getValue(storage));
      case vt_string:
        return stringToDouble(dnValueReader< dnValueTypeIdMeta<vt_string>::native_type >::getValue(storage));
      case vt_null:
        throw dnNullValueCastError();
      default:
        return static_cast<return_type>(stringToInt64(dnValueCaster<dtpString>::cast(storage, srcType)));
    } // switch
  } // function

}; // struct

template <>
struct dnValueCaster<float> {
  typedef float native_type;
  typedef Details::dnValueMeta<native_type>::return_type return_type;

  static return_type cast(const dnValueStorage &storage, int srcType)
  {
    typedef native_type return_type;
    switch (srcType) {
      case vt_float:
        return dnValueReader< dnValueTypeIdMeta<vt_float>::native_type >::getValue(storage);
      case vt_double:
      case vt_date:
      case vt_time:
      case vt_datetime:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_double>::native_type >::getValue(storage));
      case vt_xdouble:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_xdouble>::native_type >::getValue(storage));
      case vt_int:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_int>::native_type >::getValue(storage));
      case vt_uint:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_uint>::native_type >::getValue(storage));
      case vt_int64:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_int64>::native_type >::getValue(storage));
      case vt_uint64:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_uint64>::native_type >::getValue(storage));
      case vt_string:
        return stringToFloat(dnValueReader< dnValueTypeIdMeta<vt_string>::native_type >::getValue(storage));
      case vt_null:
        throw dnNullValueCastError();
      default:
        return static_cast<return_type>(stringToInt64(dnValueCaster<dtpString>::cast(storage, srcType)));
    } // switch
  } // function

}; // struct

template <>
struct dnValueCaster<bool> {
  typedef bool native_type;
  typedef Details::dnValueMeta<native_type>::return_type return_type;

  static return_type cast(const dnValueStorage &storage, int srcType)
  {
    typedef native_type return_type;
    switch (srcType) {
      case vt_byte:
        return (dnValueReader< dnValueTypeIdMeta<vt_byte>::native_type >::getValue(storage) != 0);
      case vt_uint:
        return (dnValueReader< dnValueTypeIdMeta<vt_uint>::native_type >::getValue(storage) != 0);
      case vt_int:
        return (dnValueReader< dnValueTypeIdMeta<vt_int>::native_type >::getValue(storage) != 0);
      case vt_float:
        return (round<int>(dnValueReader< dnValueTypeIdMeta<vt_float>::native_type >::getValue(storage)) != 0);
      case vt_double:
      case vt_date:
      case vt_time:
      case vt_datetime:
        return (round<int>(dnValueReader< dnValueTypeIdMeta<vt_double>::native_type >::getValue(storage)) != 0);
      case vt_xdouble:
        return (round<int>(dnValueReader< dnValueTypeIdMeta<vt_xdouble>::native_type >::getValue(storage)) != 0);
      case vt_int64:
        return (dnValueReader< dnValueTypeIdMeta<vt_int64>::native_type >::getValue(storage) != 0);
      case vt_uint64:
        return (dnValueReader< dnValueTypeIdMeta<vt_uint64>::native_type >::getValue(storage) != 0);
      case vt_string: {
        dtpString strVal = dnValueReader< dnValueTypeIdMeta<vt_string>::native_type >::getValue(storage);
        if (strVal.empty()) {
          return false;
        } else {
          char firstChar = strVal[0];
          return ((firstChar == 'T') || (firstChar == 't')); // if was True or true -> return true
        }
      }
      case vt_null:
        throw dnNullValueCastError();
      default:
        return ((stringToInt(dnValueCaster<dtpString>::cast(storage, srcType))) != 0);
    } // switch
  } // function

}; // struct

template <>
struct dnValueCaster<byte> {
  typedef byte native_type;
  typedef Details::dnValueMeta<native_type>::return_type return_type;

  static return_type cast(const dnValueStorage &storage, int srcType)
  {
    typedef native_type return_type;
    switch (srcType) {
      case vt_byte:
        return dnValueReader< dnValueTypeIdMeta<vt_byte>::native_type >::getValue(storage);
      case vt_uint:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_int>::native_type >::getValue(storage));
      case vt_int:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_int>::native_type >::getValue(storage));
      case vt_float:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_float>::native_type >::getValue(storage));
      case vt_double:
      case vt_date:
      case vt_time:
      case vt_datetime:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_double>::native_type >::getValue(storage));
      case vt_xdouble:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_xdouble>::native_type >::getValue(storage));
      case vt_int64:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_int64>::native_type >::getValue(storage));
      case vt_uint64:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_uint64>::native_type >::getValue(storage));
      case vt_string:
        return fromString<byte>(dnValueReader< dnValueTypeIdMeta<vt_string>::native_type >::getValue(storage));
      case vt_null:
        throw dnNullValueCastError();
      default:
        return static_cast<return_type>(stringToInt(dnValueCaster<dtpString>::cast(storage, srcType)));
    } // switch
  } // function

}; // struct

template <>
struct dnValueCaster<int> {
  typedef int native_type;
  typedef Details::dnValueMeta<native_type>::return_type return_type;

  static return_type cast(const dnValueStorage &storage, int srcType)
  {
    typedef native_type return_type;
    switch (srcType) {
      case vt_int:
        return dnValueReader< dnValueTypeIdMeta<vt_int>::native_type >::getValue(storage);
      case vt_float:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_float>::native_type >::getValue(storage));
      case vt_double:
      case vt_date:
      case vt_time:
      case vt_datetime:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_double>::native_type >::getValue(storage));
      case vt_xdouble:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_xdouble>::native_type >::getValue(storage));
      case vt_uint:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_uint>::native_type >::getValue(storage));
      case vt_int64:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_int64>::native_type >::getValue(storage));
      case vt_uint64:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_uint64>::native_type >::getValue(storage));
      case vt_string:
        return stringToInt(dnValueReader< dnValueTypeIdMeta<vt_string>::native_type >::getValue(storage));
      case vt_null:
        throw dnNullValueCastError();
      default:
        return static_cast<return_type>(stringToInt(dnValueCaster<dtpString>::cast(storage, srcType)));
    } // switch
  } // function

}; // struct

template <>
struct dnValueCaster<uint> {
  typedef uint native_type;
  typedef Details::dnValueMeta<native_type>::return_type return_type;

  static return_type cast(const dnValueStorage &storage, int srcType)
  {
    typedef native_type return_type;
    switch (srcType) {
      case vt_uint:
        return dnValueReader< dnValueTypeIdMeta<vt_uint>::native_type >::getValue(storage);
      case vt_int:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_int>::native_type >::getValue(storage));
      case vt_float:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_float>::native_type >::getValue(storage));
      case vt_double:
      case vt_date:
      case vt_time:
      case vt_datetime:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_double>::native_type >::getValue(storage));
      case vt_xdouble:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_xdouble>::native_type >::getValue(storage));
      case vt_int64:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_int64>::native_type >::getValue(storage));
      case vt_uint64:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_uint64>::native_type >::getValue(storage));
      case vt_string:
        return stringToUInt(dnValueReader< dnValueTypeIdMeta<vt_string>::native_type >::getValue(storage));
      case vt_null:
        throw dnNullValueCastError();
      default:
        return static_cast<return_type>(stringToInt(dnValueCaster<dtpString>::cast(storage, srcType)));
    } // switch
  } // function

}; // struct

template <>
struct dnValueCaster<int64> {
  typedef int64 native_type;
  typedef Details::dnValueMeta<native_type>::return_type return_type;

  static return_type cast(const dnValueStorage &storage, int srcType)
  {
    typedef native_type return_type;
    switch (srcType) {
      case vt_int64:
        return dnValueReader< dnValueTypeIdMeta<vt_int64>::native_type >::getValue(storage);
      case vt_int:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_int>::native_type >::getValue(storage));
      case vt_float:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_float>::native_type >::getValue(storage));
      case vt_double:
      case vt_date:
      case vt_time:
      case vt_datetime:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_double>::native_type >::getValue(storage));
      case vt_xdouble:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_xdouble>::native_type >::getValue(storage));
      case vt_uint:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_uint>::native_type >::getValue(storage));
      case vt_uint64:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_uint64>::native_type >::getValue(storage));
      case vt_string:
        return stringToInt64(dnValueReader< dnValueTypeIdMeta<vt_string>::native_type >::getValue(storage));
      case vt_null:
        throw dnNullValueCastError();
      default:
        return static_cast<return_type>(stringToInt(dnValueCaster<dtpString>::cast(storage, srcType)));
    } // switch
  } // function

}; // struct

template <>
struct dnValueCaster<uint64> {
  typedef uint64 native_type;
  typedef Details::dnValueMeta<native_type>::return_type return_type;

  static return_type cast(const dnValueStorage &storage, int srcType)
  {
    typedef native_type return_type;
    switch (srcType) {
      case vt_uint64:
        return dnValueReader< dnValueTypeIdMeta<vt_uint64>::native_type >::getValue(storage);
      case vt_int:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_int>::native_type >::getValue(storage));
      case vt_uint:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_uint>::native_type >::getValue(storage));
      case vt_float:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_float>::native_type >::getValue(storage));
      case vt_double:
      case vt_date:
      case vt_time:
      case vt_datetime:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_double>::native_type >::getValue(storage));
      case vt_xdouble:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_xdouble>::native_type >::getValue(storage));
      case vt_int64:
        return static_cast<return_type>(dnValueReader< dnValueTypeIdMeta<vt_int64>::native_type >::getValue(storage));
      case vt_string:
        return stringToUInt64(dnValueReader< dnValueTypeIdMeta<vt_string>::native_type >::getValue(storage));
      case vt_null:
        throw dnNullValueCastError();
      default:
        return static_cast<return_type>(stringToInt(dnValueCaster<dtpString>::cast(storage, srcType)));
    } // switch
  } // function

}; // struct

template <>
struct dnValueCaster<void_ptr> {
  typedef void_ptr native_type;
  typedef Details::dnValueMeta<native_type>::return_type return_type;

  static return_type cast(const dnValueStorage &storage, int srcType)
  {
    typedef native_type return_type;
    switch (srcType) {
      case vt_vptr:
        return dnValueReader< dnValueTypeIdMeta<vt_vptr>::native_type >::getValue(storage);
      default:
        return DTP_NULL;
    } // switch
  } // function
}; // struct

// ----------------------------------------------------------------------------
// dnValueCasterByEnum
// ----------------------------------------------------------------------------
template <int ResultValueType>
struct dnValueCasterByEnum {
  typedef typename dnValueTypeIdMeta<ResultValueType>::native_type native_type;
  typedef native_type return_type;

  static return_type cast(const dnValueStorage &storage, int srcValueType)
  {
      return dnValueCaster<native_type>::cast(storage, srcValueType);
  }
};

// ----------------------------------------------------------------------------
// dnValueDynamicCaster
// ----------------------------------------------------------------------------
class dnValueDynamicCaster {
public:
  virtual ~dnValueDynamicCaster() {}
  virtual void cast(int srcValueType, const dnValueStorage &srcStorage, int targetValueType, dnValueStorage &outStorage) = 0;
  virtual int compare(const dnValueStorage &storage1, const dnValueStorage &storage2) = 0;
  virtual bool equals(const dnValueStorage &storage1, const dnValueStorage &storage2) = 0;
};

// ----------------------------------------------------------------------------
// dnValueDynamicCasterByType
// ----------------------------------------------------------------------------
template<int ValueType>
class dnValueDynamicCasterByType: public dnValueDynamicCaster {
public:
  typedef typename dnValueTypeIdMeta<ValueType>::native_type target_type;
  virtual void cast(int srcValueType, const dnValueStorage &srcStorage, int targetValueType, dnValueStorage &outStorage)
  {
    dnValueWriter<target_type>::store(outStorage, dnValueCasterByEnum<ValueType>::cast(srcStorage, srcValueType));
  }

  virtual int compare(const dnValueStorage &storage1, const dnValueStorage &storage2) {
    return compareImpl<target_type>(storage1, storage2, dnValueMetaCanCompareDir<target_type>());
  }

  virtual bool equals(const dnValueStorage &storage1, const dnValueStorage &storage2) {
    return equalsImpl<target_type>(storage1, storage2, dnValueMetaCanCompareDir<target_type>());
  }

protected:
  template<typename TargetValueType>
  int compareImpl(const dnValueStorage &storage1, const dnValueStorage &storage2, dtpSelector<false> canCompareDirectly) {
    TargetValueType value1 = dnValueReader<TargetValueType>::getValue(storage1);
    TargetValueType value2 = dnValueReader<TargetValueType>::getValue(storage2);
    if (value1 < value2)
      return -1;
    else if (value1 > value2)
      return 1;
    else
      return 0;
  }

  template<typename TargetValueType>
  int compareImpl(const dnValueStorage &storage1, const dnValueStorage &storage2, dtpSelector<true> canCompareDirectly) {
    if (storage1 == storage2)
      return 0;
    else if (storage1 < storage2)
      return -1;
    else
      return 1;
  }

  template<typename TargetValueType>
  bool equalsImpl(const dnValueStorage &storage1, const dnValueStorage &storage2, dtpSelector<false> canCompareDirectly) {
    TargetValueType value1 = dnValueReader<TargetValueType>::getValue(storage1);
    TargetValueType value2 = dnValueReader<TargetValueType>::getValue(storage2);
    return (value1 == value2);
  }

  template<typename TargetValueType>
  bool equalsImpl(const dnValueStorage &storage1, const dnValueStorage &storage2, dtpSelector<true> canCompareDirectly) {
    return (storage1 == storage2);
  }
};

// ----------------------------------------------------------------------------
// dnValueDynamicCasterRegister
// ----------------------------------------------------------------------------
class dnValueDynamicCasterRegister {
public:
  typedef dnValueDynamicCaster item_type;
  typedef boost::shared_ptr<item_type> item_guard;
  typedef std::map<int, item_guard> item_map;

  static void registerType(dnValueType aType, item_type *item)
  {
    getMap().insert(std::make_pair(aType, item_guard(item)));
  }

  static void cast(int srcValueType, const dnValueStorage &srcStorage, int targetValueType, dnValueStorage &outStorage)
  {
    item_map &map = getMap();
    item_map::iterator it = map.find(targetValueType);
    if (it != map.end()) {
      it->second->cast(srcValueType, srcStorage, targetValueType, outStorage);
    } else {
      throw std::runtime_error(dtpString("Unknown target value type for cast: ")+toString(targetValueType));
    }
  }

  static int compare(int targetValueType, const dnValueStorage &storage1, const dnValueStorage &storage2)
  {
    item_map &map = getMap();
    item_map::iterator it = map.find(targetValueType);
    if (it != map.end()) {
      return it->second->compare(storage1, storage2);
    } else {
      throw std::runtime_error(dtpString("Unknown target value type for cast: ")+toString(targetValueType));
      return 0;
    }
  }

  static bool equals(int targetValueType, const dnValueStorage &storage1, const dnValueStorage &storage2)
  {
    item_map &map = getMap();
    item_map::iterator it = map.find(targetValueType);
    if (it != map.end()) {
      return it->second->equals(storage1, storage2);
    } else {
      throw std::runtime_error(dtpString("Unknown target value type for cast: ")+toString(targetValueType));
      return false;
    }
  }
protected:
  static item_map &getMap() {
    static item_map registeredItems;
    return registeredItems;
  }
private:
};

// ----------------------------------------------------------------------------
// dnValueDynamicCasterBinder
// ----------------------------------------------------------------------------
/// binds integer value type to caster
template<int ValueType>
class dnValueDynamicCasterBinder {
public:
  dnValueDynamicCasterBinder() {
    dnValueDynamicCasterRegister::registerType(static_cast<dnValueType>(ValueType), new dnValueDynamicCasterByType<ValueType>());
  }
};


// ----------------------------------------------------------------------------
// dnValueDynamicStrategyRegister
// ----------------------------------------------------------------------------
template<typename IntfName>
class dnValueDynamicStrategyRegister {
public:
  typedef IntfName item_type;
  typedef boost::shared_ptr<item_type> item_guard;
  typedef item_type *item_ptr;
  typedef std::vector<item_guard> item_container;
  typedef std::vector<item_ptr> item_container_raw;
  typedef dnValueDynamicStrategyRegister<IntfName> this_type;

  static void registerType(dnValueType aType, item_type *item)
  {
    at(aType, item);
  }

  static item_type &at(dnValueType aType, item_type *newItem = DTP_NULL) {
    static item_container registeredItems(vt_last + 1);
    if (newItem != DTP_NULL) {
      size_t item_index = static_cast<int>(aType);
      if (registeredItems.size() <= item_index)
      {
        registeredItems.resize(item_index + 1);
      }
      registeredItems[item_index] = item_guard(newItem);
    }
    return *(registeredItems[aType]);
  }
};

// ----------------------------------------------------------------------------
// dnValueCompareStrategyIntf
// ----------------------------------------------------------------------------
class dnValueCompareStrategyIntf {
public:
  virtual ~dnValueCompareStrategyIntf() {}
  virtual bool canCompareDirectly() = 0;
};

// ----------------------------------------------------------------------------
// dnValueCompareStrategy
// ----------------------------------------------------------------------------
template<int ValueType>
class dnValueCompareStrategy: public dnValueCompareStrategyIntf {
public:
  virtual bool canCompareDirectly() {
    return (dnValueCompareMeta<ValueType>::can_compare_dir != 0);
  }
};

// ----------------------------------------------------------------------------
// dnValueCompareStrategyBinder
// ----------------------------------------------------------------------------
template<int ValueType>
class dnValueCompareStrategyBinder {
public:
  dnValueCompareStrategyBinder() {
    dnValueDynamicStrategyRegister<dnValueCompareStrategyIntf>::registerType(static_cast<dnValueType>(ValueType), new dnValueCompareStrategy<ValueType>());
  }
};

struct parent_visitor_impl_tag {};
struct parent_visitor_null_tag {};

template <typename ValueType>
struct dnParentVisitorMeta {
    enum Options { has_visitor = 1 };
    typedef parent_visitor_impl_tag visitor_category;
};

template <>
struct dnParentVisitorMeta<parent_visitor_null_tag> {
    enum Options { has_visitor = 0 };
    typedef parent_visitor_null_tag visitor_category;
};

template<typename ValueType, bool Value = (dnParentVisitorMeta<ValueType>::has_visitor == 1)>
struct dnValueMetaHasParentVisitor: public dtpSelector<Value>
{
};

template <typename Selector>
class ParentVisitorGeneric {
    typedef uint size_type;

    template<typename ValueType, typename Visitor>
    static
    void visitTreeValues(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
      throw dnNotImplementedError();
    }

    template<typename IntCompareOp>
    static
    bool binarySearchNode(const dnChildColnBaseIntf *parent, const dnode &value, IntCompareOp compOp, int &foundPos)
    {
      throw dnNotImplementedError();
    }

    template<typename ValueType, typename IntCompareOp>
    static
    bool binarySearchValue(const dnChildColnBaseIntf *parent, const ValueType &value, IntCompareOp compOp, int &foundPos)
    {
      throw dnNotImplementedError();
    }

    template<typename CompareOp>
    static
    void sortNodes(const dnChildColnBaseIntf *parent, CompareOp compOp)
    {
      throw dnNotImplementedError();
    }
};

template <int ParentType>
class ParentVisitor {
    typedef uint size_type;
    template<typename ValueType, typename Visitor>
    static
    void visitTreeValues(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
      throw dnNotImplementedError();
    }

    template<typename ValueType, typename Visitor>
    static
    void visitTreeNodes(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
      throw dnNotImplementedError();
    }

    template<typename ValueType, typename Visitor>
    static
    void visitVectorValues(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
      throw dnNotImplementedError();
    }

    template<typename ValueType, typename Visitor>
    static
    void visitVectorNodes(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
      throw dnNotImplementedError();
    }

    template<typename ValueType, typename IntCompareOp>
    static
    bool binarySearchValue(const dnChildColnBaseIntf *parent, const ValueType &value, IntCompareOp compOp, int &foundPos)
    {
      throw dnNotImplementedError();
    }

    template<typename IntCompareOp>
    static
    bool binarySearchNode(const dnChildColnBaseIntf *parent, const dnode &value, IntCompareOp compOp, int &foundPos)
    {
      throw dnNotImplementedError();
    }

    template<typename ValueType, typename CompareOp>
    static
    void sortValues(const dnChildColnBaseIntf *parent, CompareOp compOp)
    {
      throw dnNotImplementedError();
    }

    template<typename CompareOp>
    static
    void sortNodes(const dnChildColnBaseIntf *parent, CompareOp compOp)
    {
      throw dnNotImplementedError();
    }

    template<typename ValueType, typename CompareOp, typename iterator>
    static
    iterator find_if(const dnChildColnBaseIntf *parent, ValueType value, CompareOp compOp)
    {
      throw dnNotImplementedError();
    }

    template<typename ValueType, typename CompareOp>
    static
    size_type find_if(const dnChildColnBaseIntf *parent, ValueType value, CompareOp compOp)
    {
      throw dnNotImplementedError();
    }

    template<typename ValueType, typename iterator>
    static
    size_type find(const dnChildColnBaseIntf *parent, ValueType value)
    {
      throw dnNotImplementedError();
    }

    template<typename ValueType, typename CompOp>
    static
    size_type find_if_derived(const dnChildColnBaseIntf *parent, ValueType value, CompOp compOp)
    {
      throw dnNotImplementedError();
    }

    template<typename ValueType>
    static
    size_type find_derived(const dnChildColnBaseIntf *parent, ValueType value)
    {
      throw dnNotImplementedError();
    }
};

} // namespace Details

// ----------------------------------------------------------------------------
// dnValue
// ----------------------------------------------------------------------------
/// \brief Scalar-only version of dnode.
/// \details Can be used in STL containers.
/// Use it when you want to have STL container with variable-type values.
class dnValue {
public:
    typedef uint size_type;

    //dnValue();
    dnValue(): m_valueType(vt_null) {}
    ~dnValue() {
      releaseDataNoInit();
    }

    /// copy constructor
    dnValue(const dnValue& src);

    /// r-value constructor
    dnValue(base::move_ptr<dnValue> &src)
    {
      this->m_valueData = src->m_valueData;
      this->m_valueType = src->m_valueType;
      src->m_valueData = true;
      src->m_valueType = vt_null;
    }

    dnValue& operator=( const dnValue& rhs);

    dnValue& operator=(const base::move_ptr<dnValue> &src) {
      this->m_valueData = src->m_valueData;
      this->m_valueType = src->m_valueType;
      src->m_valueData = true;
      src->m_valueType = vt_null;
      return *this;
    }

    bool operator!=(const dnValue &rhs);
    bool operator==(const dnValue &rhs) const;

    bool isEqualTo(const dnValue &value) const;
    bool isEqualTo(const dnValue &value, void *buffer) const;

    //dnValueType getValueType() const;
    dnValueType getValueType() const
    {
      return m_valueType;
    }

    /// Change value type with conversion of existing value
    void convertTo(dnValueType valueType);

    /// @brief Change value type with conversion of existing value.
    /// @depricated Use dnValue::convertTo instead
    DTP_DEPRECATED void forceValueType(dnValueType valueType) {
      convertTo(valueType);
    }

    /// Clear all contents - set value to NULL.
    void clear() {
      setValueTypeNoInit(vt_null);
      m_valueData = true;
    }

    /// @depricated Deprecated. Use dnValue::clear instead.
    DTP_DEPRECATED void setAsNull();

    DTP_DEPRECATED void setAsByte(int a_value);
    DTP_DEPRECATED void setAsInt(int a_value);
    DTP_DEPRECATED void setAsInt64(int64 a_value);
    DTP_DEPRECATED void setAsUInt(uint a_value);
    DTP_DEPRECATED void setAsUInt64(uint64 a_value);

    DTP_DEPRECATED void setAsString(const dtpString &a_value);
    DTP_DEPRECATED void setAsBool(bool a_value);

    DTP_DEPRECATED void setAsFloat(float a_value);
    DTP_DEPRECATED void setAsDouble(double a_value);
    DTP_DEPRECATED void setAsXDouble(xdouble a_value);
    DTP_DEPRECATED void setAsVoidPtr(void_ptr a_value);

    DTP_DEPRECATED void setAsDate(fdatetime_t a_value);
    DTP_DEPRECATED void setAsTime(fdatetime_t a_value);
    DTP_DEPRECATED void setAsDateTime(fdatetime_t a_value);

    bool isNull() const
    {
      return (m_valueType == vt_null?true:false);
    }

    DTP_DEPRECATED byte getAsByte() const;
    DTP_DEPRECATED int getAsInt() const;
    DTP_DEPRECATED uint getAsUInt() const;
    DTP_DEPRECATED int64 getAsInt64() const;
    DTP_DEPRECATED uint64 getAsUInt64() const;
    DTP_DEPRECATED const dtpString getAsString() const;
    DTP_DEPRECATED bool getAsBool() const;

    DTP_DEPRECATED float getAsFloat() const;
    DTP_DEPRECATED double getAsDouble() const;
    DTP_DEPRECATED xdouble getAsXDouble() const;
    DTP_DEPRECATED void_ptr getAsVoidPtr() const;

    DTP_DEPRECATED fdatetime_t getAsDate() const;
    DTP_DEPRECATED fdatetime_t getAsTime() const;
    DTP_DEPRECATED fdatetime_t getAsDateTime() const;

/*
    template<typename ValueType>
    ValueType getAs() const
    {
      throw dnError("Not implemented!");
      return 0;
    }
*/

    template<typename ValueType>
    typename Details::dnValueMeta<ValueType>::return_type getAs() const
    {
      using namespace Details;
      if (m_valueType == static_cast<dnValueType>(dnValueTypeMeta<ValueType>::item_type))
      {
        return dnValueReader<ValueType>::getValue(m_valueData);
      } else {
        return dnValueCaster<ValueType>::cast(m_valueData, m_valueType);
      }
    }

    template<typename ValueType>
    //void setAs(ValueType newValue, typename dtpDisableIf<dnValueMetaIsObject<ValueType>, ValueType>::type* = 0)
      typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
        setAs(ValueType newValue)
    {
      using namespace Details;
      setValueType(static_cast<dtp::dnValueType>(dnValueTypeMeta<ValueType>::item_type));
      assert(getValueType() != vt_null);
      dnValueWriter<ValueType>::store(m_valueData, newValue);
    }

    template<typename ValueType>
    //void setAs(const ValueType &newValue, typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, ValueType>::type* = 0)
      typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
        setAs(const ValueType &newValue)
    {
      using namespace Details;
      setValueType(static_cast<dtp::dnValueType>(dnValueTypeMeta<ValueType>::item_type));
      assert(getValueType() != vt_null);
      dnValueWriter<ValueType>::store(m_valueData, newValue);
    }

    template<typename ValueType>
    void setAs(ValueType *newValue)
    {
      using namespace Details;
      setValueType(static_cast<dtp::dnValueType>(dnValueTypeMeta<ValueType *>::item_type));
      assert(getValueType() != vt_null);
      dnValueWriter<ValueType *>::store(m_valueData, newValue);
    }

    void copyFrom(const dnValue& src);
    // for fast STL containers
    dnValue &swap(dnValue &rhs);
    friend void swap(dnValue& lhs, dnValue& rhs);

    template<typename ValueType>
    dnValue & operator=(const ValueType & rhs)
    {
        dnValue(rhs).swap(*this);
        return *this;
    }
protected:
    void initStringValue(const dtpString &a_value);

    void releaseData() {
      using namespace Details;
      dnValueDestructor<vt_last>::execRelease(m_valueData, m_valueType);
    }

    void releaseDataNoInit() {
      using namespace Details;
      dnValueDestructor<vt_last>::execReleaseNoInit(m_valueData, m_valueType);
    }

    void assignFrom( const dnValue& src);
    void setValueType(dnValueType aType);
    void setValueTypeNoInit(dnValueType aType) {
      if (m_valueType != aType) {
        if (m_valueType != vt_null)
        {
          releaseDataNoInit();
        }
        m_valueType = aType;
      }
    }
    void initValueType(dnValueType aType);
    void initValueTypeNoDataInit(dnValueType aType);
    void setStringValue(const dtpString &a_value);
    void initScalarFrom(const dnValue& src);
protected:
    void doCopyFromAssign( const dnValue& src);
protected:
    dnValueStorage m_valueData;
    friend class dnode;
private:
    dnValueType m_valueType;
}; // dnValue

typedef std::vector<dnValue> dnode_vector;

class dnode;

namespace Details {

// ----------------------------------------------------------------------------
// dnArray
// ----------------------------------------------------------------------------
template <typename ValueTypeVisitTag>
class dnArrayVisitor
{
public:
  typedef dnValue::size_type size_type;

  template<typename ValueType, class ArrayType>
  static
    typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
      addItem(ArrayType *aArray, ValueType value)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, class ArrayType>
  static
    typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
      addItem(ArrayType *aArray, const ValueType &value)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, class ArrayType>
  static
    typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
      addItemAtPos(ArrayType *aArray, size_type pos, ValueType value)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, class ArrayType>
  static
    typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
      addItemAtPos(ArrayType *aArray, size_type pos, const ValueType &value)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, class ArrayType>
  static
  ValueType get(ArrayType *aArray, size_type index)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, class ArrayType>
  static
  void get(ArrayType *aArray, size_type index, ValueType &output)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, class ArrayType>
  static
  void set(ArrayType *aArray, size_type index, const ValueType &value)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, class ArrayType>
  static
  void get(ArrayType *aArray, size_type index, ValueType &output, dnode &helper)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, class ArrayType>
  static
  void set(ArrayType *aArray, size_type index, const ValueType &value, dnode &helper)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, typename Visitor, class ArrayType>
  static
  void visitTreeValues(ArrayType *aArray, Visitor visitor)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, typename Visitor, class ArrayType>
  static
  void visitTreeNodes(ArrayType *aArray, Visitor visitor)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, typename Visitor, class ArrayType>
  static
  Visitor visitVectorValues(ArrayType *aArray, Visitor visitor)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, typename Visitor, class ArrayType>
  static
  Visitor visitVectorValues(ArrayType *aArray, Visitor visitor, const ValueType &tag)
  {
      throw dnNotImplementedError();
  }

  template <typename ValueType, typename CompOp, class ArrayType>
  static
  bool binarySearchValue(ArrayType *aArray, const ValueType &value, CompOp compOp, int &foundPos)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, typename IntCompareOp, class ArrayType>
  static
  bool binarySearchValueDirect(ArrayType *aArray, const ValueType &value, IntCompareOp compOp, int &foundPos)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, typename IntCompareOp, class ArrayType>
  static
  bool binarySearchValueByItem(ArrayType *aArray, const ValueType &value, IntCompareOp compOp, int &foundPos)
  {
      throw dnNotImplementedError();
  }

  template<typename IntCompareOp, class ArrayType>
  static
  bool binarySearchNode(ArrayType *aArray, const dnode &value, IntCompareOp compOp, int &foundPos)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, typename IntCompareOp, class ArrayType>
  static
  void sortValues(ArrayType *aArray, IntCompareOp compOp)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, typename IntCompareOp, class ArrayType>
  static
  void sortNodesByRef(ArrayType *aArray, IntCompareOp compOp)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, typename CompOp, class ArrayType>
  static
  size_type find_if(ArrayType *aArray, ValueType value, CompOp compOp)
  {
      throw dnNotImplementedError();
  }

  template<typename ValueType, class ArrayType>
  static
  size_type find(ArrayType *aArray, ValueType value)
  {
      throw dnNotImplementedError();
  }


  template<typename T>
  static
  dnArray *newArray()
  {
      throw dnNotImplementedError();
  }

}; // dnArrayVisitor

struct dnArrayVisitTagImpl {};

template <typename ValueType>
struct dnArrayVisitMeta {
    typedef dnArrayVisitTagImpl visitor_tag;
};

///base class for all arrays
class dnArray {
public:
  typedef uint size_type;
  static const size_type npos;
  //static const size_type npos = static_cast<dnArray::size_type>(-1);

  dnArray(dnValueType a_type): m_valueType(a_type) {}
  virtual ~dnArray() {};
  dnValueType getValueType() const
  {
    return m_valueType;
  }

  virtual void copyFrom(const dnArray *a_source) = 0;
  virtual bool empty() const = 0;
  virtual dnArray *clone() const = 0;
  virtual dnArray *cloneEmpty() const = 0;
  virtual void getItem(int index, dnode &output) const = 0;
  virtual const dnode &getNode(int index, dnode &helper) const;
  virtual dnode *getNodePtr(int index, dnode &helper) const;
  virtual void setItem(int index, const dnode &input) = 0;
  virtual void setItemValue(int index, const dnode &input);
  virtual void setNode(int index, const dnode &value);
  virtual void addItem(const dnValue &input) = 0;
  void addItem(const dnode &input) { addItemAsNode(input); }
  virtual void addItem(base::move_ptr<dnode> input) = 0;
  virtual void addItemAsNode(const dnode &input) = 0;
  virtual void addItemAsNode(dnode *input) = 0;
  virtual void addItemAtFront(const dnode &input) = 0;
  virtual void addItemAtPos(size_type pos, const dnode &input) = 0;
  virtual void eatItem(dnode &input);
  virtual void eraseItem(int index) = 0;
  virtual void eraseFrom(int index) = 0;
  virtual size_type indexOfValue(const dnode &input) const = 0;
  virtual size_type findByName(const dtpString &name) const { return npos; }
  virtual void clear() = 0;
  virtual size_type size() const = 0;
  virtual void resize(size_type newSize) = 0;

  virtual void swap(size_type pos1, size_type pos2) = 0;

  //template<typename ValueType>
  //  typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
  //    addItem(ValueType value)
  //{
    // TODO: correct this call for better class hierarchy handling
    //static_cast<dtp::Details::dnArrayBase *>(this)->addItem(value);
  //}

  template<typename ValueType>
    typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
      addItem(ValueType value)
  {
    typedef typename dnArrayVisitMeta<ValueType>::visitor_tag visitor_tag;
    typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
    ArrayVisitor::addItem(this, value);
  }

  template<typename ValueType>
    typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
      addItem(const ValueType &value)
  {
    typedef typename dnArrayVisitMeta<ValueType>::visitor_tag visitor_tag;
    typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
    //static_cast<dnArrayBase *>(this)->addItem(value);
    ArrayVisitor::addItem(this, value);
  }

  template<typename ValueType>
    typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
      addItemAtPos(size_type pos, ValueType value)
  {
    typedef typename dnArrayVisitMeta<ValueType>::visitor_tag visitor_tag;
    typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
    //static_cast<dnArrayBase *>(this)->addItemAtPos(pos, value);
    ArrayVisitor::addItemAtPos(this, pos, value);
  }

  template<typename ValueType>
    typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
      addItemAtPos(size_type pos, const ValueType &value)
  {
    typedef typename dnArrayVisitMeta<ValueType>::visitor_tag visitor_tag;
    typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
    //static_cast<dnArrayBase *>(this)->addItemAtPos(pos, value);
    ArrayVisitor::addItemAtPos(this, pos, value);
  }

  template<typename ValueType>
  ValueType get(size_type index) const
  {
     typedef typename dnArrayVisitMeta<ValueType>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
     ValueType result;
     //does not compile on GCC:
     //return ArrayVisitor::get<T, dnArray *>(this, index);
     ArrayVisitor::get(this, index, result);
     return result;
  }

  template<typename ValueType>
  void set(size_type index, const ValueType &value)
  {
     typedef typename dnArrayVisitMeta<ValueType>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
     ArrayVisitor::set(this, index, value);
  }

  template<typename ValueType>
  ValueType get(size_type index, dnode &helper) const
  {
     //return getNode(index, helper).getAs<ValueType>();
     typedef typename dnArrayVisitMeta<ValueType>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
     ValueType result;
     ArrayVisitor::get(this, index, result, helper);
     return result;
  }

  template<typename ValueType>
  void set(size_type index, const ValueType &value, dnode &helper)
  {
     //helper.setAs<ValueType>(value);
     //setNode(index, helper);
     typedef typename dnArrayVisitMeta<ValueType>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
     ArrayVisitor::set(this, index, value, helper);
  }

  virtual bool sort() {return false;};

  template<typename ValueType, typename Visitor>
  void visitTreeValues(Visitor visitor) const
  {
     typedef typename dnArrayVisitMeta<ValueType>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
     ArrayVisitor::visitTreeValues(this, visitor);
  }

  template<typename ValueType, typename Visitor>
  void visitTreeNodes(Visitor visitor) const
  {
     typedef typename dnArrayVisitMeta<ValueType>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
     ArrayVisitor::visitTreeNodes(this, visitor);
  }

  template<typename ValueType, typename Visitor>
  Visitor visitVectorValues(Visitor visitor) const
  {
     typedef typename dnArrayVisitMeta<ValueType>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
     return ArrayVisitor::template visitVectorValues<ValueType>(this, visitor);
  }

  template<typename ValueType, typename Visitor>
  void visitVectorNodes(Visitor visitor) const
  {
     dnode helper;
     for(size_type i=0, epos = size(); i != epos; i++)
       visitor(getNode(i, helper));
  }

  template <typename T>
  dnArray *newArray()
  {
     typedef typename dnArrayVisitMeta<T>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;

      return (
        ArrayVisitor::template newArray<T>()
      );
  }

  template <typename T>
  dnArray *newArray(T)
  {
     typedef typename dnArrayVisitMeta<T>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;

      return (
        ArrayVisitor::template newArray<T>()
      );
  }


  template <typename ValueType, typename CompOp>
  bool binarySearchValue(const ValueType &value, CompOp compOp, int &foundPos) const
  {
     typedef typename dnArrayVisitMeta<ValueType>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
     return ArrayVisitor::binarySearchValue(this, value, compOp, foundPos);
  }

  // --- scan
  //template<typename ValueType, typename Visitor>
  //bool scanTreeValues(Visitor visitor) const
  //{
  //   using namespace Details;

  //   bool res = false;
  //   typedef dnArrayImplMeta<ValueType>::implementation_type ImplArray;
  //   typedef dnArrayImplMeta<ValueType>::vector_type VectorType;
  //   ImplArray *implArray = const_cast<ImplArray *>(dynamic_cast<const ImplArray *>(this));
  //   VectorType &vect = implArray->getItems();
  //   if (dnArrayImplMeta<ValueType>::item_is_node != 0)
  //   {
  //     for(size_type i=0, epos = size(); i != epos; i++)
  //       if (vect[i].visitTreeValues<ValueType, Visitor>(visitor))
  //       {
  //         res = true;
  //         break;
  //       }
  //   } else {
  //     for(size_type i=0, epos = size(); i != epos; i++)
  //       if (visitMethod(vect[i], visitor))
  //       {
  //         res = true;
  //         break;
  //       }
  //   }

  //   return res;
  //}

  //template<typename ValueType, typename Visitor>
  //bool scanTreeNodes(Visitor visitor) const
  //{
  //   using namespace Details;

  //   bool res = false;
  //   typedef dnArrayImplMeta<ValueType>::implementation_type ImplArray;
  //   typedef dnArrayImplMeta<ValueType>::vector_type VectorType;
  //   ImplArray *implArray = const_cast<ImplArray *>(dynamic_cast<const ImplArray *>(this));
  //   VectorType &vect = implArray->getItems();
  //   if (dnArrayImplMeta<ValueType>::item_is_node != 0)
  //   {
  //     for(size_type i=0, epos = size(); i != epos; i++)
  //       if (vect[i].visitTreeNodes(visitor))
  //       {
  //         res = true;
  //         break;
  //       }
  //   } else {
  //     dnode helper;
  //     for(size_type i=0, epos = size(); i != epos; i++)
  //       if (visitor(getNode(i, helper)))
  //       {
  //         res = true;
  //         break;
  //       }
  //   }

  //   return res;
  //}

  //template<typename ValueType, typename Visitor>
  //bool scanVectorValues(Visitor visitor) const
  //{
  //   using namespace Details;

  //   typedef dnArrayImplMeta<ValueType>::implementation_type ImplArray;
  //   typedef dnArrayImplMeta<ValueType>::vector_type VectorType;
  //   ImplArray *implArray = const_cast<ImplArray *>(dynamic_cast<const ImplArray *>(this));
  //   VectorType &vect = implArray->getItems();

  //   //std::for_each(vect.begin(), vect.end(), visitor);
  //   for(size_type i=0, epos = size(); i != epos; i++)
  //   {
  //     if (visitor(vect[i]))
  //       return true;
  //   }

  //   return false;
  //}

  //template<typename ValueType, typename Visitor>
  //bool scanVectorNodes(Visitor visitor) const
  //{
  //   dnode helper;
  //   for(size_type i=0, epos = size(); i != epos; i++)
  //   {
  //     if (visitor(getNode(i, helper)))
  //       return true;
  //   }

  //   return false;
  //}

  template<typename ValueType, typename IntCompareOp>
  bool binarySearchValueDirect(const ValueType &value, IntCompareOp compOp, int &foundPos) const
  {
     typedef typename dnArrayVisitMeta<ValueType>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
     return ArrayVisitor::binarySearchValueDirect(this, value, compOp, foundPos);
  }

  template<typename ValueType, typename IntCompareOp>
  bool binarySearchValueByItem(const ValueType &value, IntCompareOp compOp, int &foundPos) const
  {
     typedef typename dnArrayVisitMeta<ValueType>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
     return ArrayVisitor::binarySearchValueByItem(this, value, compOp, foundPos);
  }

  template<typename IntCompareOp>
  bool binarySearchNode(const dnode &value, IntCompareOp compOp, int &foundPos) const
  {
     typedef typename dnArrayVisitMeta<IntCompareOp>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
     return ArrayVisitor::binarySearchNode(this, value, compOp, foundPos);
  }

  template<typename ValueType, typename IntCompareOp>
  void sortValues(IntCompareOp compOp)
  {
     typedef typename dnArrayVisitMeta<IntCompareOp>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
     ArrayVisitor::template sortValues<ValueType, IntCompareOp >(this, compOp);
  }

  template<typename IntCompareOp>
  void sortNodes(IntCompareOp compOp)
  {
      sortNodesByRef<IntCompareOp>(compOp);
  }

  template<typename ValueType, typename IntCompareOp>
  void sortNodesByRef(IntCompareOp compOp)
  {
     typedef typename dnArrayVisitMeta<IntCompareOp>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
     ArrayVisitor::template sortNodesByRef<ValueType, IntCompareOp >(this, compOp);
  }

  template<typename ValueType, typename CompOp>
  size_type find_if(ValueType value, CompOp compOp)
  {
     typedef typename dnArrayVisitMeta<CompOp>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
     return ArrayVisitor::find_if(this, compOp);
  }

  template<typename ValueType>
  size_type find(ValueType value)
  {
     typedef typename dnArrayVisitMeta<ValueType>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
     return ArrayVisitor::find(this, value);
  }

protected:
  dnValueType m_valueType;
};

} // namespace Details

//class dnValueBridge;

namespace Details {

typedef boost::ptr_vector<dnode> dnodeColn;
typedef dnodeColn::auto_type dnodeColnTransport;

#ifdef DATANODE_UNORDERED_ENABLED
typedef boost::unordered_map<dtpString, dnChildTransporter>  dnChildColnNameMap;
typedef boost::unordered_map<int, dnChildTransporter>  dnChildColnIndexMap;
#else
#ifdef DATANODE_POOL_CONTAINERS
typedef boost::fast_pool_allocator<
				std::pair<dtpString, dnChildTransporter>,
				boost::default_user_allocator_new_delete,
				boost::details::pool::null_mutex>
                                //,8192>
		  dnChildColnNameMapAllocator;

#ifdef DATANODE_CHILD_MAP_NOT_SMART
typedef std::map<dtpString, dnodePtr, std::less<dtpString>, dnChildColnNameMapAllocator  >  dnChildColnNameMap;
#else
typedef std::map<dtpString, dnChildTransporter, std::less<dtpString>, dnChildColnNameMapAllocator  >  dnChildColnNameMap;
#endif // DATANODE_CHILD_MAP_NOT_SMART
#else
typedef std::map<dtpString, dnChildTransporter>  dnChildColnNameMap;
#endif
#ifdef DATANODE_DBL_MAP
typedef std::map<dnChildTransporter>  dnChildColnIndexMap;
#else
#ifdef DATANODE_POOL_CONTAINERS
#ifdef DATANODE_CHILD_MAP_NOT_SMART
typedef boost::shared_ptr<DTP_UNIQUE_PTR(dnode)> dnChildAutoTransporter;
typedef std::pair<dtpString, dnChildAutoTransporter> scNamedDataNodeItem;
#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
typedef std::vector<dnode *> dnChildColnIndexMap;
#else
typedef dnodeColn dnChildColnIndexMap;
#endif
typedef std::vector<dtpString> dnChildColnNameVector;
#else
typedef boost::pool_allocator<dnChildTransporter, boost::default_user_allocator_new_delete, boost::details::pool::null_mutex>
  dnChildColnIndexMapAllocator;
typedef std::vector<dnChildTransporter, dnChildColnIndexMapAllocator>  dnChildColnIndexMap;
#endif // DATANODE_CHILD_MAP_NOT_SMART
#else
typedef std::vector<dnChildTransporter>  dnChildColnIndexMap;
#endif // DATANODE_POOL_CONTAINERS
#endif
#endif

typedef dnChildColnIndexMap::iterator dnChildColnMapIterator;

typedef boost::shared_ptr<dnArray> dnArrayTransporter;

} // namespace Details

class dnode;

namespace Details {
  class dnChildColnBase;
  typedef boost::shared_ptr<dnChildColnBase> dnChildColnTransporter;

  typedef boost::shared_ptr<dnode> dnTransporter;
  typedef DTP_UNIQUE_PTR(dnode) dnGuard;
};

class dnConstIterator;
class dnIterator;

namespace Const {
   static const uint npos = static_cast<uint>(-1);
};

// ----------------------------------------------------------------------------
// dnode
// ----------------------------------------------------------------------------
/// Universal, hierarchical data structure, similar to node in XML DOM (composite design pattern).
// dnode start
class dnode: public dnValue {
    typedef dnValue inherited;
    typedef Details::dnChildColnBase dnChildColnBase;
    typedef Details::dnChildColnBaseIntf dnChildColnBaseIntf;
    typedef Details::dnArray dnArray;
public:
    typedef uint size_type;
    typedef dnArray array_type;

    static const size_type npos;

    dnode(): dnValue() {}

    /// copy constructor
    dnode(const dnode& src);

    explicit dnode(base::move_ptr<dnode> &src)
    {
      this->m_valueData = src->m_valueData;
      this->m_valueType = src->m_valueType;
      src->m_valueData = true;
      src->m_valueType = vt_null;
    }

    explicit dnode(const base::move_ptr<dnode> &src)
    {
      this->m_valueData = src->m_valueData;
      this->m_valueType = src->m_valueType;
      src->m_valueData = true;
      src->m_valueType = vt_null;
    }

    explicit dnode(const dnValue &src);

    dnode& operator=(const dnode& rhs);
    dnode& operator=(const dnode_vector& rhs);
    dnode& operator=(const dnValue &rhs);

    dnode& operator=(const base::move_ptr<dnode> &src) {
      resetProps();
      this->m_valueData = src->m_valueData;
      this->m_valueType = src->m_valueType;
      src->m_valueData = true;
      src->m_valueType = vt_null;
      return *this;
    }

    template<typename ValueType>
    typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, dnode &>::type
    operator=(ValueType value)
    {
      setAs<ValueType>(value);
      return *this;
    }

    template<typename ValueType>
    typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, dnode &>::type
    operator=(const ValueType &value)
    {
      setAs<ValueType>(value);
      return *this;
    }

    template<typename ValueType>
    dnode &operator=(const ValueType *value)
    {
      setAs(const_cast<ValueType *>(value));
      return *this;
    }

    bool operator==(const dnode &rhs) const;

    using inherited::isEqualTo;
    bool isEqualTo(const dnode &value) const;

    template<typename ValueType>
    dnode(ValueType value, typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, ValueType>::type* = 0)
    {
      using namespace Details;
      static_assert(dnValueTypeMeta<ValueType>::item_type != vt_undefined, "Data type not supported");
      m_valueType = static_cast<dtp::dnValueType>(dnValueTypeMeta<ValueType>::item_type);
      dnValueInitializer<ValueType>::init(m_valueData, value);
    }

    template<typename ValueType>
    dnode(const ValueType &value, typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, ValueType>::type* = 0)
    {
      using namespace Details;
      static_assert(dnValueTypeMeta<ValueType>::item_type != vt_undefined, "Data type not supported");
      m_valueType = static_cast<dtp::dnValueType>(dnValueTypeMeta<ValueType>::item_type);
      dnValueInitializer<ValueType>::init(m_valueData, value);
    }

#ifdef DATANODE_INIT_LIST
    template<typename T>
    explicit dnode(std::initializer_list<T> aList) {
       using namespace Details;

       setAsArray(new dnArrayImplMeta<T>::implementation_type());
       for(auto it = aList.begin(), epos = aList.end(); it != epos; ++it)
         addItem(*it);
    }
#endif
    explicit dnode(dnInitContainerType value);

    explicit dnode(dnInitContainerType value, dnValueType itemType)
    {
      assert(value == ict_array);
      initProps();
      setAsArray(itemType);
    }

    bool isParent() const
    {
      return (m_valueType == vt_parent?true:false);
    }

    bool isArray() const
    {
      return (m_valueType == vt_array?true:false);
    }

    bool isList() const
    {
      bool res = isParent();
      if (res)
        res = (!getAsChildrenIntfR()->supportsAccessByName());

      return res;
    }

    bool supportsRefs() const {
      return (isParent() || isList());
    }

    bool supportsNames() const {
      return (isParent() && getAsChildrenIntfR()->supportsAccessByName());
    }

    bool isContainer() const
    {
      if (
           (m_valueType == vt_parent)
           ||
           (m_valueType == vt_array)
         )
      {
        return true;
      } else {
        return false;
      }
    }

    void merge(const dnode &rhs);
    void merge(base::move_ptr<dnode> rhs);
    void resize(size_type newSize);

    // for fast STL containers
    dnode &swap(dnode &rhs);
    friend void swap(dnode& lhs, dnode& rhs);

    void swap(size_type pos1, size_type pos2);

    size_type indexOfName(const dtpString &name) const;

    template<typename ValueType>
    size_type indexOfValue(const ValueType &value) const
    {
      return find_index(value);
    }

    size_type indexOfValue(const dtp::dnode &value) const
    {
      return intIndexOfValue(value);
    }

    bool hasValue(const dnode &value) const;

    void setAsArray(dnValueType a_valueType);

    template<typename T>
    void setAsArray()
    {
      typedef typename Details::dnArrayVisitMeta<T>::visitor_tag visitor_tag;
      typedef typename Details::dnArrayVisitor<visitor_tag> ArrayVisitor;

      setAsArray(
        ArrayVisitor::template newArray<T>()
      );
    }

    void setAsParent();
    void setAsList();
    void setAsNull();

    dnChildColnBase &getChildren();
    const dnChildColnBase &getChildrenR() const;

    dnArray *getArray()
    {
      return getAsArray();
    }

    const dnArray *getArrayR() const
    {
      return getAsArrayR();
    }

    template<typename ValueType>
    void push_front(const ValueType &value)
    {
      if (isArray())
        addItemAtFront(value);
      else
        intAddChildAtFront(new dnode(value));
    }

    template<typename ValueType>
    void push_front(const dtpString &aName, const ValueType &value)
    {
      intAddChildAtFront(aName, new dnode(value));
    }

    template<typename ValueType>
    void push_back(const ValueType &value)
    {
      if (isArray())
        getArray()->addItem(value);
      else
        intAddChild(new dnode(value));
    }

    template<typename ValueType>
    void push_back(const dtpString &aName, const ValueType &value)
    {
      addChild(aName, new dnode(value));
    }

    template<typename ValueType>
    dnode &addChild(ValueType child)
    {
      intAddChild(new (std::nothrow) dnode(child));
      return *this;
    }

    dnode &addChild(dnode *child)
    {
      intAddChild(child);
      return *this;
    }

    template<typename ValueType>
    dnode &addChild(const dtpString &name, ValueType child)
    {
      intAddChild(name, new dnode(child));
      return *this;
    }

    dnode &addChild(const dtpString &name, dnode *child)
    {
      intAddChild(name, child);
      return *this;
    }

    /// Remove child from container and returns it (similar to auto_ptr::release)
    dnode *extractChild(int index);

    /// Add children from input - move them
    void transferChildrenFrom(dnode &input);
    void copyChildrenFrom(const dnode &input);

    void addItem(const dnode &input);
    void addItem(const dnValue &input);

    DTP_DEPRECATED void addItemAsString(const dtpString &value);

    DTP_DEPRECATED void addItemAsFloat(float value);
    DTP_DEPRECATED void addItemAsDouble(double value);

    DTP_DEPRECATED void addItemAsXDouble(xdouble value);

    DTP_DEPRECATED void addItemAsBool(bool value);
    DTP_DEPRECATED void addItemAsInt(int value);
    DTP_DEPRECATED void addItemAsUInt(uint value);
    DTP_DEPRECATED void addItemAsInt64(int64 value);
    DTP_DEPRECATED void addItemAsUInt64(uint64 value);

    DTP_DEPRECATED void addItemAsVoidPtr(void_ptr value);

    template<typename ValueType>
      typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, dnode &>::type
        addItem(ValueType value)
    {
      getArray()->addItem(value);
      return *this;
    }

    template<typename ValueType>
      typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, dnode &>::type
        addItem(const ValueType &value)
    {
      getArray()->addItem(value);
      return *this;
    }

    void addItemList(const dnode &input);
    void addItemList(const dnode_vector &input);

    bool hasChild(const dtpString &name) const;
    const dnode childNames(bool useAsNames = false) const;

    dnode *peekChild(const dtpString &aName);
    const dnode *peekChildR(const dtpString &aName) const;

// universal element access
    size_type size() const;

    /// Checks if container is empty.
    /// \Result Returns <false> if node contains any items, <true> otherwise.
    bool empty() const;

    dnode &getElement(int index, dnode &output) const;
    const dnode getElement(int index) const;
    dnode &getElement(const dtpString &aName, dnode &output) const;
    const dnode getElement(const dtpString &aName) const;

    bool getElementSafe(const dtpString &aName, dnode &output) const;
    bool hasElement(const dtpString &name) const;

    dnValueType getElementType(int index) const;
    dnValueType getElementType(const dtpString &aName) const;
    dnValueType getElementType() const;

    void forceElementType(int index, dnValueType valueType);
    void forceElementTypeAll(dnValueType valueType);

    const dtpString getElementName(int index) const;
    void getElementName(int index, dtpString &output) const;
    dnode *cloneElement(int index) const;

    void setElement(int index, const dnode &value);
    void setElement(const dtpString &aName, const dnode &value);
    bool setElementSafe(const dtpString &aName, const dnode &value);
    bool setElementSafe(const dtpString &aName, base::move_ptr<dnode> value);

    void setElementValue(int index, const dnode &value);
    void setElementValue(const dtpString &aName, const dnode &value);

    bool getElementByPath(const dnode &pathNode, dnode &output);
    bool setElementByPath(const dnode &pathNode, const dnode &value);

    const dnode &getNode(int index, dnode &helper) const;
    const dnode &getNode(const dtpString &aName, dnode &helper) const;

    dnode *getNodePtr(int index, dnode &helper);
    dnode *getNodePtr(const dtpString &aName, dnode &helper);

    const dnode *getNodePtrR(int index, dnode &helper) const;
    const dnode *getNodePtrR(const dtpString &aName, dnode &helper) const;

    void setNode(int index, dnode &value);
    void setNode(const dtpString &aName, dnode &value);

    dnode &addElement(const dnode &value);
    dnode &addElement(const dtpString &aName, const dnode &value);

    dnode &addElement(base::move_ptr<dnode> value);
    dnode &addElement(const dtpString &aName, base::move_ptr<dnode> value);
    
    //move value from node to a new sub-node
    void eatElement(dnode& src);
    //remove element from collection
    void eraseElement(uint index);
    void eraseFrom(uint index);

//
// value shortcuts
    // getters
    DTP_DEPRECATED const dtpString getString(const dtpString &a_name, const dtpString &a_defValue) const;
    DTP_DEPRECATED const dtpString getString(const dtpString &a_name) const;
    DTP_DEPRECATED bool getBool(const dtpString &a_name, bool a_defValue) const;
    DTP_DEPRECATED bool getBool(const dtpString &a_name) const;
    DTP_DEPRECATED int getInt(const dtpString &a_name, int a_defValue) const;
    DTP_DEPRECATED int getInt(const dtpString &a_name) const;
    DTP_DEPRECATED byte getByte(const dtpString &a_name, byte a_defValue) const;
    DTP_DEPRECATED byte getByte(const dtpString &a_name) const;
    DTP_DEPRECATED uint getUInt(const dtpString &a_name, uint a_defValue) const;
    DTP_DEPRECATED uint getUInt(const dtpString &a_name) const;
    DTP_DEPRECATED int64 getInt64(const dtpString &a_name, int64 a_defValue) const;
    DTP_DEPRECATED int64 getInt64(const dtpString &a_name) const;
    DTP_DEPRECATED uint64 getUInt64(const dtpString &a_name, uint64 a_defValue) const;
    DTP_DEPRECATED uint64 getUInt64(const dtpString &a_name) const;
    DTP_DEPRECATED float getFloat(const dtpString &a_name, float a_defValue) const;
    DTP_DEPRECATED float getFloat(const dtpString &a_name) const;
    DTP_DEPRECATED double getDouble(const dtpString &a_name, double a_defValue) const;
    DTP_DEPRECATED double getDouble(const dtpString &a_name) const;

    DTP_DEPRECATED xdouble getXDouble(const dtpString &a_name, xdouble a_defValue) const;
    DTP_DEPRECATED xdouble getXDouble(const dtpString &a_name) const;

    DTP_DEPRECATED void_ptr getVoidPtr(const dtpString &a_name, void_ptr a_defValue) const;
    DTP_DEPRECATED void_ptr getVoidPtr(const dtpString &a_name) const;

    DTP_DEPRECATED fdatetime_t getDateTime(const dtpString &a_name, fdatetime_t a_defValue) const;
    DTP_DEPRECATED fdatetime_t getDateTime(const dtpString &a_name) const;

    // setters
    DTP_DEPRECATED void setString(const dtpString &a_name, const dtpString &value);
    DTP_DEPRECATED void setString(const dtpString &a_name, const char *value);
    DTP_DEPRECATED void setBool(const dtpString &a_name, bool value);
    DTP_DEPRECATED void setInt(const dtpString &a_name, int value);
    DTP_DEPRECATED void setByte(const dtpString &a_name, byte value);
    DTP_DEPRECATED void setUInt(const dtpString &a_name, uint value);
    DTP_DEPRECATED void setUIntDef(const dtpString &a_name, uint value);
    DTP_DEPRECATED void setInt64(const dtpString &a_name, int64 value);
    DTP_DEPRECATED void setUInt64(const dtpString &a_name, uint64 value);
    DTP_DEPRECATED void setFloat(const dtpString &a_name, float value);
    DTP_DEPRECATED void setDouble(const dtpString &a_name, double value);
    DTP_DEPRECATED void setXDouble(const dtpString &a_name, xdouble value);
    DTP_DEPRECATED void setVoidPtr(const dtpString &a_name, void_ptr value);
    DTP_DEPRECATED void setDateTime(const dtpString &a_name, fdatetime_t value);

//---- by integer position
    DTP_DEPRECATED dtpString getString(int a_index) const;
    DTP_DEPRECATED bool getBool(int a_index) const;
    DTP_DEPRECATED int getInt(int a_index) const;
    DTP_DEPRECATED byte getByte(int a_index) const;
    DTP_DEPRECATED uint getUInt(int a_index) const;
    DTP_DEPRECATED int64 getInt64(int a_index) const;
    DTP_DEPRECATED uint64 getUInt64(int a_index) const;
    DTP_DEPRECATED float getFloat(int a_index) const;
    DTP_DEPRECATED double getDouble(int a_index) const;
    DTP_DEPRECATED xdouble getXDouble(int a_index) const;
    DTP_DEPRECATED void_ptr getVoidPtr(int a_index) const;
    // setters
    DTP_DEPRECATED void setString(int a_index, const dtpString &value);
    DTP_DEPRECATED void setString(int a_index, const char *value);
    DTP_DEPRECATED void setBool(int a_index, bool value);
    DTP_DEPRECATED void setInt(int a_index, int value);
    DTP_DEPRECATED void setByte(int a_index, byte value);
    DTP_DEPRECATED void setUInt(int a_index, uint value);
    DTP_DEPRECATED void setInt64(int a_index, int64 value);
    DTP_DEPRECATED void setUInt64(int a_index, uint64 value);
    DTP_DEPRECATED void setFloat(int a_index, float value);
    DTP_DEPRECATED void setDouble(int a_index, double value);
    DTP_DEPRECATED void setXDouble(int a_index, xdouble value);
    DTP_DEPRECATED void setVoidPtr(int a_index, void_ptr value);

// with template access
    /// Returns item selected by name, if does not exist or is NULL then defValue is returned.
    /// Version for scalar values.
    /// @param[in] name Item name
    /// @param[in] defValue Default value
    template<typename ValueType>
      typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, ValueType>::type
        get(const dtpString &name, ValueType defValue) const
    {
//      throw dnError("Not implemented!");
//      return 0;
      ValueType res;

      const dnode *child = const_cast<dnChildColnBaseIntf *>(getAsChildrenIntfR())->peekChildR(name);

      if (child != NULL) {
        if (child->getValueType() == vt_null)
          res = defValue;
        else
          res = child->getAs<ValueType>();
      }
      else
        res = defValue;
      return res;
    }

    /// Returns item selected by name, if does not exist or is NULL then defValue is returned.
    /// Version for objects (string).
    /// @param[in] name Item name
    /// @param[in] defValue Default value
    template<typename ValueType>
    const
      typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, ValueType>::type
        get(const dtpString &name, const ValueType &defValue) const
    {
      //throw dnError("Not implemented!");
      //return 0;
      ValueType res;
      dnode *child = const_cast<dnChildColnBaseIntf *>(getChildrenPtrR())->peekChildR(name);
      if (child != NULL) {
        if (child->getValueType() == vt_null)
          res = defValue;
        else
          res = child->getAs<ValueType>();
      }
      else
        res = defValue;
      return res;
    }

    template<typename ValueType>
    ValueType get(const dtpString &name) const
    {
      return (*(const_cast<dnode *>(this)))[name].getAs<ValueType>();
    }

    template<typename ValueType>
      typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
    set(const dtpString &name, ValueType newValue)
    {
      (*this)[name].setAs<ValueType>(newValue);
    }

    template<typename ValueType>
      typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
    set(const dtpString &name, const ValueType &newValue)
    {
      (*this)[name].setAs<ValueType>(newValue);
    }

    template<typename ValueType>
    ValueType get(size_type index) const
    {
      //throw dnError("Not implemented!");
      //return 0;
      if (isArray()) {
        return getAsArrayNoCheckR()->get<ValueType>(index);
      } else if (isParent()) {
        //return const_cast<dnChildColnBaseIntf *>(getAsChildrenNoCheckR())->at(index).getAs<ValueType>();
        return const_cast<dnChildColnBaseIntf *>(getAsChildrenIntfR())->at(index).getAs<ValueType>();
      } else {
        throwNotContainer();
        return ValueType();
      }
    }

    // tagged version
    template<typename ValueType>
    void get(size_type index, ValueType &output) const
    {
      //throw dnError("Not implemented!");
      //return 0;
      if (isArray()) {
        output = getAsArrayNoCheckR()->get<ValueType>(index);
      } else if (isParent()) {
        output = const_cast<dnChildColnBaseIntf *>(getAsChildrenNoCheckR())->at(index).getAs<ValueType>();
      } else {
        throwNotContainer();
        output = ValueType();
      }
    }

    template<typename ValueType>
    void set(size_type index, const ValueType &newValue)
    {
      if (isArray()) {
        getArray()->set<ValueType>(index, newValue);
      } else {
        (*this)[index].setAs<ValueType>(newValue);
      }
    }

//--- copy / assignment
    void copyFrom(const dnode& src);
    void copyValueFrom(const dnode& src);
    void copyValueFrom(const dnode_vector& src);
    void copyTo(dnode_vector& output);

    /// Copy value but keep data type as it was
    void assignValueFrom( const dnode& src);

    /// Move value from source to "this" node.
    /// @depricated Use moveFrom instead.
    DTP_DEPRECATED void eatValueFrom(dnode& src) {
      moveFrom(src);
    }

    /// Move value from source to "this" node.
    void moveFrom(dnode& src);

//---
    const dnode &operator[](int idx) const;
    const dnode &operator[](const dtpString &str_idx) const;
    dnode &operator[](int idx);
    dnode &operator[](const dtpString &str_idx);

    void scan(dnScanner &scanner) const;

    /// Convert node to a string, including each contained item on each level.
    /// Useful for debugging.
    dtpString dump(const dtpString &indent = "", const dtpString &name = "") const;

    DTP_DEPRECATED dtpString dumpHierarchy() const;

    /// Convert list of nodes to string separated by a given separator. \n
    /// Example: implode(dnode(2,3,4), "##") -> 2##3##4
    /// @param[in] separator String separator.
    dtpString implode(const dtpString &separator) const;

    /// Convert string to a list of nodes, use given separator to find items.
    /// @param[in] separator String separator.
    /// @param[in] a_text String to be splitted.
    /// @param[in] output Output list of nodes
    /// @param[in] useAsNames if = true then output values will be used as item names instead of item values
    /// @return Output list
    static dnode &explode(const dtpString &separator, const dtpString &a_text, dnode &output, bool useAsNames = false);

    /// Convert string to a list of nodes, use given separator to find items.
    /// @param[in] separator String separator.
    /// @param[in] a_text String to be splitted.
    /// @param[in] useAsNames if = true then output values will be used as item names instead of item values
    /// @return Output list
    static dnode explode(const dtpString &separator, const dtpString &a_text, bool useAsNames = false);

protected:
    void intAddChild(const dtpString &name, dnode *child);

    using inherited::copyFrom;

    void intAddChild(dnode *child);
    void intAddChildAtFront(dnode *child);
    void intAddChildAtFront(const dtpString &aName, dnode *child);
    void intAddChildAtPos(size_type pos, dnode *child);
    void intAddChildAtPos(size_type pos, const dtpString &aName, dnode *child);

    void setAsArray(dnArray *value);
    void setAsParent(dnChildColnBase *value);
    dnChildColnBase *getAsChildren();
    const dnChildColnBase *getAsChildrenR() const;

    dnChildColnBase *getAsChildrenNoCheck()
    {
      return static_cast<dnChildColnBase *>(boost::get<void_ptr>(m_valueData));
    }

    const dnChildColnBase *getAsChildrenNoCheckR() const
    {
      return static_cast<dnChildColnBase *>(boost::get<void_ptr>(m_valueData));
    }


    const dnChildColnBaseIntf *getAsChildrenIntfR() const
    {
      return static_cast<dnChildColnBaseIntf *>(boost::get<void_ptr>(m_valueData));
    }

    dnArray *getAsArray()
    {
      if (m_valueType == vt_array) {
        return static_cast<dnArray *>(boost::get<void_ptr>(m_valueData));
      } else {
        throw dnError("Not an array");
        return DTP_NULL;
      }
    }

    const dnArray *getAsArrayR() const
    {
      if (m_valueType == vt_array) {
        return static_cast<dnArray *>(boost::get<void_ptr>(m_valueData));
      } else {
        throw dnError("Not an array");
        return DTP_NULL;
      }
    }

    dnArray *getAsArrayNoCheck()
    {
      return static_cast<dnArray *>(boost::get<void_ptr>(m_valueData));
    }

    const dnArray *getAsArrayNoCheckR() const
    {
      return static_cast<dnArray *>(boost::get<void_ptr>(m_valueData));
    }

    dnChildColnBase *extractChildren();
    dnArray *extractArray();

    static void throwNotContainer();
    static void throwNotParent();
    static void throwNotFound(const dtpString &aName);

    bool hasChildren() const;
    bool hasItems() const;

    void clearValue();
    void clearElements();

    template<typename ValueType>
    void addItemAtFront(const ValueType &value)
    {
      dnode node(value);
      addItemAtFront(node);
    }

    template<typename ValueType>
      typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
        addItemAtPos(size_type pos, ValueType value)
    {
      getArray()->addItemAtPos(pos, value);
    }

    template<typename ValueType>
      typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
        addItemAtPos(size_type pos, const ValueType &value)
    {
      getArray()->addItemAtPos(pos, value);
    }

  size_type intIndexOfValue(const dnode &value) const;

  void checkArrayType(dnValueType atype);
  void prepareArrayType(dnValueType atype);

  void initFrom( const dnode& src);
  void initValueFrom( const dnode& src);
  void initProps() {}
  void resetProps();
  dnChildColnBase *createChildrenColn(bool aNamed);
  dnChildColnBase &setupChildren(bool aNamed);
  void disposeChildren();
  void disposeArray();
  dtpString dumpValue(const dtpString &indent) const;
  dtpString dumpChildren(const dtpString &indent) const;
  dtpString dumpItems(const dtpString &indent) const;
  void copyStructureFrom( const dnode& src);
  void scanValue(dnScanner &scanner) const;
  void scanChildren(dnScanner &scanner) const;
  void scanItems(dnScanner &scanner) const;
  void intScan(dnScanner &scanner) const;
  dnChildColnBase *getChildrenPtr();
  const dnChildColnBase *getChildrenPtrR() const;
  void addItemAtFront(const dnode &input);
  void addItemAtPos(size_type pos, const dnode &input);
protected:

public:
// ----------------------------------------------------------------------------
// dnValueBridge
// ----------------------------------------------------------------------------
/// Class used to read/write values of items inside dnode
/// Values can be stored as object (dnode) or scalar.
/// Values are manipulated directly inside underlying container, so different bridges will be
/// constructed for different containers.
/// Bridges are used inside iterators & store also current position inside container.
///
/// bridge.getAsDouble() => (array-of-double) => return m_array[idx]
/// bridge.setAsDouble(value) => (array-of-double) => m_array[idx] = value
///
/// bridge.setAsDouble(value) => (array-of-dnode) => m_array[idx].setValueAsDouble(value)
/// bridge b = *(mylist.at(100)) = assign (with all possible scalars, data node, data value)
///
class dnValueBridge {
protected:
   dnValueBridge() {}
   dnValueBridge( const dnValueBridge& src) {}
public:
    typedef uint size_type;
    static const size_type npos;

    virtual ~dnValueBridge() {}

    virtual void release_to_pool() = 0;

    virtual dnValueBridge *clone() const = 0;
    virtual dnValueBridge& operator=(const dnode& rhs);
    virtual dnValueBridge& operator=(const dnValue& rhs);
    virtual dnValueBridge& operator=(const dnode_vector& rhs);

    template<typename ValueType>
    dnValueBridge &operator=(const ValueType &value)
    {
      setAs(value);
      return *this;
    }

    // to compare values
    virtual bool operator==(const dnValueBridge &rhs) const;

    DTP_DEPRECATED virtual float getAsFloat() const;
    DTP_DEPRECATED virtual double getAsDouble() const;
    DTP_DEPRECATED virtual xdouble getAsXDouble() const;
    DTP_DEPRECATED virtual void_ptr getAsVoidPtr() const;
    DTP_DEPRECATED virtual bool getAsBool() const;
    DTP_DEPRECATED virtual int getAsInt() const;
    DTP_DEPRECATED virtual byte getAsByte() const;
    DTP_DEPRECATED virtual uint getAsUInt() const;
    DTP_DEPRECATED virtual int64 getAsInt64() const;
    DTP_DEPRECATED virtual uint64 getAsUInt64() const;
    DTP_DEPRECATED virtual dtpString getAsString() const;

    DTP_DEPRECATED virtual void setAsFloat(float value);
    DTP_DEPRECATED virtual void setAsDouble(double value);
    DTP_DEPRECATED virtual void setAsXDouble(xdouble value);
    DTP_DEPRECATED virtual void setAsVoidPtr(void_ptr value);
    DTP_DEPRECATED virtual void setAsBool(bool value);
    DTP_DEPRECATED virtual void setAsInt(int value);
    DTP_DEPRECATED virtual void setAsByte(byte value);
    DTP_DEPRECATED virtual void setAsUInt(uint value);
    DTP_DEPRECATED virtual void setAsInt64(int64 value);
    DTP_DEPRECATED virtual void setAsUInt64(uint64 value);
    DTP_DEPRECATED virtual void setAsString(const dtpString &value);

    template<typename ValueType>
    ValueType getAs() const
    {
      dnode node;
      return getAsNode(node).getAs<ValueType>();
    }

    template<typename ValueType>
    void setAs(const ValueType &value)
    {
      dnode node(value);
      setAsItem(node);
    }

    template<typename ValueType>
    ValueType getAs(dnode &helper) const
    {
      return getAsNode(helper).getAs<ValueType>();
    }

    template<typename ValueType>
    void setAs(const ValueType &value, dnode &helper)
    {
      helper.setAs<ValueType>(value);
      setAsItem(helper);
    }

    dnode getAs() const
    {
      dnode res;
      getAsItem(res);
      return res;
    }

    void setAs(const dnode &newValue)
    {
       setAsItem(newValue);
    }

    virtual bool isNull();
    virtual void setAsNull();

    // universal get / set value
    virtual void setAsItem(const dnode& value) = 0;
    virtual void getAsItem(dnode& output) const = 0;

    virtual void setAsItem(const dnValue& value) = 0;
    virtual void getAsItem(dnValue& output) const = 0;

    virtual const dnValue getRValue() const;
    virtual const dnode getAsElement() const;
    virtual const dnode &getAsNode(dnode &helper) const;
    virtual dnode *getAsNodePtr(dnode &helper);
    virtual bool supportsRefs() const;
    virtual bool supportsNames() const;

    virtual void setAsNode(const dnode &value);

    // name access
    virtual const dtpString getName() const = 0;
    virtual void setName(const dtpString &value) = 0;

    // modify position
    virtual size_type getPos() const = 0; /// set absolute pos
    virtual void setPos(size_type idx) = 0; /// set absolute pos

    virtual void setPosBegin() = 0; /// set pos to begin
    virtual void setPosEnd() = 0; /// set pos to end
    virtual void incPos(int value = 1) = 0; /// increment pos
    virtual void decPos(int value = 1) = 0; /// decrement pos

    // container info
    virtual bool empty() const = 0;
    virtual size_type size() const = 0;

    // compare pos
    virtual bool isEqualPos(const dnValueBridge& value) const = 0;

    virtual bool isLessPos(const dnValueBridge& value) const {
      return (calcPosDiff(value) < 0);
    }

    virtual int calcPosDiff(const dnValueBridge& value) const = 0;
protected:
    virtual dnode &getAsNodeRef();
}; // dnValueBridge

// ----------------------------------------------------------------------------
// dnConstIterator
// ----------------------------------------------------------------------------
class dnConstIterator
{
    public:
        typedef dnConstIterator self_type;
        typedef dnValueBridge value_type;
        typedef dnode parent_type;
        typedef value_type& reference;
        typedef value_type* pointer;
        typedef int difference_type;
        typedef std::random_access_iterator_tag iterator_category;
        typedef uint size_type;

        dnConstIterator() : m_target(0) { }
        ~dnConstIterator()
        {
          if (m_valueBridge.get())
           {
             deleteValueBridge(m_valueBridge.release());
           }
        }

        explicit dnConstIterator(parent_type *target, size_type idx) : m_target(target) {
            m_valueBridge.reset(m_target->newValueBridge(idx));
        }
        explicit dnConstIterator(parent_type *target, const dtpString& name) : m_target(target) {
            m_valueBridge.reset(m_target->newValueBridge(name));
        }
        explicit dnConstIterator(parent_type *target, const char *name) : m_target(target) {
            m_valueBridge.reset(m_target->newValueBridge(name));
        }
        explicit dnConstIterator(parent_type *target, dnPos pos) : m_target(target) {
            m_valueBridge.reset(m_target->newValueBridge(pos));
        }
        dnConstIterator(const dnConstIterator &rhs) :
          m_target(rhs.m_target)
        {
          m_valueBridge.reset(rhs.m_valueBridge->clone());
        }

        self_type& operator++() {
          m_valueBridge->incPos();
          return *this;
        }

        self_type operator++(int junk) {
          self_type i(*this);
          m_valueBridge->incPos();
          return i;
        }

        self_type& operator--() {
          m_valueBridge->decPos();
          return *this;
        }

        self_type operator--(int junk) {
          self_type i(*this);
          m_valueBridge->decPos();
          return i;
        }

        self_type& operator=(const self_type& rhs)
        {
            if (this != &rhs) {
              if (m_target != rhs.m_target) {
                  if (m_valueBridge.get())
                    deleteValueBridge(m_valueBridge.release());
                  m_target = rhs.m_target;
              }

              if (m_valueBridge.get() == DTP_NULL) {
                m_valueBridge.reset(rhs.m_valueBridge->clone());
              }

              m_valueBridge->setPos(rhs.m_valueBridge->getPos());
            }

            return(*this);
        }

        const reference operator*() const { return *m_valueBridge; }
        const pointer operator->() { return m_valueBridge.get(); }

        bool operator==(const self_type& rhs) {
           return (m_target == rhs.m_target) && m_valueBridge->isEqualPos(*rhs.m_valueBridge);
        }

        bool operator!=(const self_type& rhs) {
           if (m_target != rhs.m_target)
             return true;
           if (!m_valueBridge->isEqualPos(*rhs.m_valueBridge))
             return true;
           return false;
        }

        bool operator<(const self_type& rhs) {
          if (m_target != rhs.m_target)
            return false;
          return (m_valueBridge->isLessPos(*rhs.m_valueBridge));
        }

        difference_type operator-(const self_type& rhs) const {
            if (m_target != rhs.m_target)
              throw dnError("Wrong iterator target!");
            return m_valueBridge->calcPosDiff(*rhs.m_valueBridge);
        }

        self_type operator+(difference_type rhs) const {
            self_type i(*this);
            i.m_valueBridge->incPos(rhs);
            return i;
        }

        self_type operator-(difference_type rhs) const {
            self_type i(*this);
            i.m_valueBridge->decPos(rhs);
            return i;
        }

        self_type & operator+=(difference_type rhs) {
            m_valueBridge->incPos(rhs);
            return *this;
        }

        self_type & operator-=(difference_type rhs) {
            m_valueBridge->decPos(rhs);
            return *this;
        }

    protected:
        void deleteValueBridge(dnode::dnValueBridge *bridge)
        {
        #ifndef DATANODE_POOL_BRIDGE
          delete bridge;
        #else
          bridge->release_to_pool();
        #endif
        }
    protected:
        parent_type *m_target;
        DTP_UNIQUE_PTR(value_type) m_valueBridge;
}; // dnConstIterator

// ----------------------------------------------------------------------------
// dnIterator
// ----------------------------------------------------------------------------
class dnIterator: public dnConstIterator
{
    public:
        typedef dnIterator self_type;
        typedef dnValueBridge value_type;
        typedef dnode parent_type;
        typedef value_type& reference;
        typedef value_type* pointer;
        typedef std::random_access_iterator_tag iterator_category;
        typedef int difference_type;
        typedef uint size_type;

    private:
        explicit dnIterator(const dnConstIterator& x) : dnConstIterator(x){}

    public:
        dnIterator() : dnConstIterator() { }

        explicit dnIterator(parent_type *target, size_type idx) : dnConstIterator(target, idx) { }
        explicit dnIterator(parent_type *target, const dtpString &name) : dnConstIterator(target, name) { }
        explicit dnIterator(parent_type *target, const char *name) : dnConstIterator(target, name) { }
        explicit dnIterator(parent_type *target, dnPos pos) : dnConstIterator(target, pos) { }
        dnIterator(const self_type &rhs) : dnConstIterator(rhs) { }

        self_type& operator++() { m_valueBridge->incPos(); return *this;  }
        self_type operator++(int junk) { self_type i(*this); m_valueBridge->incPos(); return i;}

        self_type& operator--() { m_valueBridge->decPos(); return *this;  }
        self_type operator--(int junk) { self_type i(*this); m_valueBridge->decPos(); return i;}

        reference operator*() const { return *m_valueBridge; }
        pointer operator->() { return m_valueBridge.get(); }

        bool operator==(const self_type& rhs) { return (m_target == rhs.m_target) && m_valueBridge->isEqualPos(*rhs.m_valueBridge); }
        bool operator!=(const self_type& rhs) {
          if (m_target != rhs.m_target)
            return true;
          if (!m_valueBridge->isEqualPos(*rhs.m_valueBridge))
            return true;
          return false;
        }

        bool operator<(const self_type& rhs) {
          if (m_target != rhs.m_target)
            return false;
          return (m_valueBridge->isLessPos(*rhs.m_valueBridge));
        }

        difference_type operator-(const self_type& rhs) const {
            if (m_target != rhs.m_target)
              throw dnError("Wrong iterator target!");
            return m_valueBridge->calcPosDiff(*rhs.m_valueBridge);
        }
        self_type operator+(difference_type rhs) const { self_type i(*this); i.m_valueBridge->incPos(rhs); return i; }
        self_type operator-(difference_type rhs) const { self_type i(*this); i.m_valueBridge->decPos(rhs); return i; }

        self_type & operator+=(difference_type rhs) {
            m_valueBridge->incPos(rhs);
            return *this;
        }

        self_type & operator-=(difference_type rhs) {
            m_valueBridge->decPos(rhs);
            return *this;
        }
}; // dnIterator


// iterator ready to be used in STL algorithms, fast in assignment
template <class T>
class const_scalar_iterator {
public:
  typedef const_scalar_iterator<T> self_type;
  typedef T value_type;
  typedef value_type& reference;
  typedef value_type* pointer;
  typedef int difference_type;
  typedef dtp::dnode::size_type size_type;
  typedef std::random_access_iterator_tag iterator_category;

  const_scalar_iterator() :m_node(DTP_NULL), m_pos(0) { }

  const_scalar_iterator(const dtp::dnode &node, size_type pos) :m_node(&const_cast<dtp::dnode &>(node)), m_pos(pos) {
  }

  const_scalar_iterator(const dtp::dnode &node, const dtpString &aName) :
    m_node(&const_cast<dtp::dnode &>(node)),
    //m_pos(dtp::dnode::npos),
    m_pos(Const::npos),
    m_valueBridge(const_cast<dtp::dnode &>(node).newValueBridge(aName))
  {
  }

  const_scalar_iterator(const self_type &src) :m_node(src.m_node), m_pos(src.intGetPos()) {
  }

  const_scalar_iterator& operator=(const const_scalar_iterator& other)
  {
      if (this != &other) {
        m_node = other.m_node;
        m_pos = other.intGetPos();
      }
      return(*this);
  }

  //operator T()
  const T operator *() const
  {
      //if ((m_pos == dnode::npos) && (m_valueBridge.get() != DTP_NULL))
      if ((m_pos == Const::npos) && (m_valueBridge.get() != DTP_NULL))
        return m_valueBridge->getAs<T>();
      else
        return m_node->get<T>(m_pos);
  }

  bool operator==(const const_scalar_iterator& other)
  {
      if ((m_pos != other.m_pos) || (m_node != other.m_node))
        return false;
      if ((m_valueBridge.get() != DTP_NULL) != (other.m_valueBridge.get() != DTP_NULL))
        return false;
      if (m_valueBridge.get() == DTP_NULL)
        return true;
      return ((*m_valueBridge) == (*other.m_valueBridge));
  }

  bool operator!=(const const_scalar_iterator& other)
  {
      return!(((*this) == other));
      //return ((m_node != other.m_node) || (m_pos != other.m_pos));
  }

  bool operator<(const self_type& rhs) {
    if (m_node != rhs.m_node)
      return false;
    if ((m_pos != rhs.m_pos) || (m_pos != dnode::npos))
      return (m_pos < rhs.m_pos);
    return false;
    //if ((m_valueBridge.get() != DTP_NULL) != (rhs.m_valueBridge.get() != DTP_NULL))
    //  return false;
    //if (m_valueBridge.get() == DTP_NULL)
    //    return false;
    //return ((*m_valueBridge) < (*rhs.m_valueBridge));
  }

  const_scalar_iterator& operator++()
  {
      checkPos();
      m_pos++;
      return(*this);
  }

  const_scalar_iterator& operator++(int)
  {
      const_scalar_iterator tmp(*this);
      ++(*this);
      return(tmp);
  }

  const_scalar_iterator& operator--()
  {
      checkPos();
      m_pos--;
      return(*this);
  }

  const_scalar_iterator& operator--(int)
  {
      const_scalar_iterator tmp(*this);
      --(*this);
      return(tmp);
  }

  //const const_scalar_iterator& operator*() const
  //{
  //    return(*this);
  //}

  //const_scalar_iterator& operator*()
  //{
  //    return(*this);
  //}

  difference_type operator-(const self_type& rhs) const {
    const_cast<self_type *>(this)->checkPos();
    return m_pos - rhs.m_pos;
  }

  self_type operator+(difference_type rhs) const {
    const_cast<self_type *>(this)->checkPos();
    self_type i(*this); i.m_pos += rhs; return i;
  }

  self_type operator-(difference_type rhs) const {
    const_cast<self_type *>(this)->checkPos();
    self_type i(*this); i.m_pos -= rhs; return i;
  }

  self_type &operator+=(difference_type rhs) {
    checkPos();
    m_pos += rhs; return *this;
  }

  self_type &operator-=(difference_type rhs) {
    checkPos();
    m_pos -= rhs; return *this;
  }

protected:
  void checkPos()
  {
    //if (m_pos == dnode::npos)
    if (m_pos == Const::npos)
     if (m_valueBridge.get() != DTP_NULL)
       m_pos = m_valueBridge->getPos();
  }

  size_type intGetPos() const
  {
    //if (m_pos == dnode::npos)
    if (m_pos == Const::npos)
      if (m_valueBridge.get() != DTP_NULL)
        return m_valueBridge->getPos();
    return m_pos;
  }

protected:
  DTP_UNIQUE_PTR(dnValueBridge) m_valueBridge;
  dtp::dnode *m_node;
  mutable size_type m_pos;
};

// iterator ready to be used in STL algorithms, fast in assignment
template <class T>
class scalar_iterator: public const_scalar_iterator<T> {
public:
  typedef scalar_iterator<T> self_type;
  typedef const_scalar_iterator<T> inherited;
  typedef T value_type;
  typedef value_type& reference;
  typedef value_type* pointer;
  typedef int difference_type;
  typedef dtp::dnode::size_type size_type;
  typedef std::random_access_iterator_tag iterator_category;

  scalar_iterator() :inherited() { }

  scalar_iterator(dtp::dnode &node, size_type pos) :inherited(node, pos) {
  }

  scalar_iterator(const dtp::dnode &node, size_type pos) :
    //m_node(&const_cast<dtp::dnode &>(node)), m_pos(pos)
    inherited(node, pos)
  {
  }

  scalar_iterator(const dtp::dnode &node, const dtpString &aName) :
    //m_node(&const_cast<dtp::dnode &>(node)),
    //m_pos(npos),
    //m_valueBridge(const_cast<dtp::dnode &>(node).newValueBridge(aName))
    inherited(node, aName)
  {
  }

  scalar_iterator(const self_type &src) :inherited(*src.m_node, src.intGetPos()) {
  }

  scalar_iterator(self_type &src) :inherited(*src.m_node, src.intGetPos())
  {
  }

  scalar_iterator& operator=(const scalar_iterator& other)
  {
      if (this != &other) {
        inherited::m_node = other.m_node;
        inherited::m_pos = const_cast<scalar_iterator &>(other).intGetPos();
      }
      return(*this);
  }

  bool operator==(const scalar_iterator& other)
  {
      if ((inherited::m_pos != other.m_pos) || (inherited::m_node != other.m_node))
        return false;
      if ((inherited::m_valueBridge.get() != DTP_NULL) != (other.m_valueBridge.get() != DTP_NULL))
        return false;
      if (inherited::m_valueBridge.get() == DTP_NULL)
        return true;
      return ((*inherited::m_valueBridge) == (*other.m_valueBridge));
  }

  bool operator!=(const scalar_iterator& other)
  {
      return!(((*this) == other));
      //return ((m_node != other.m_node) || (m_pos != other.m_pos));
  }

  bool operator<(const self_type& rhs) {
    if (inherited::m_node != rhs.m_node)
      return false;
    if ((inherited::m_pos != rhs.m_pos) || (inherited::m_pos != dnode::npos))
      return (inherited::m_pos < rhs.m_pos);
    if ((inherited::m_valueBridge.get() != DTP_NULL) != (rhs.m_valueBridge.get() != DTP_NULL))
      return false;
    if (inherited::m_valueBridge.get() == DTP_NULL)
        return false;
    return (inherited::m_valueBridge->isLessPos(*(rhs.m_valueBridge)));
  }

  scalar_iterator& operator++()
  {
      inherited::checkPos();
      inherited::m_pos++;
      return(*this);
  }

  scalar_iterator& operator++(int)
  {
      scalar_iterator tmp(*this);
      ++(*this);
      return(tmp);
  }

  scalar_iterator& operator--()
  {
      inherited::checkPos();
      inherited::m_pos--;
      return(*this);
  }

  scalar_iterator& operator--(int)
  {
      scalar_iterator tmp(*this);
      --(*this);
      return(tmp);
  }


  //operator T()
  /// dereference operator works correctly only for read, otherwise it = it + 5 is ambigous
  /// (when specialized w/ <int>)
  T operator *() const
  {
      //if ((m_pos == dnode::npos) && (m_valueBridge.get() != DTP_NULL))
      if ((inherited::m_pos == Const::npos) && (inherited::m_valueBridge.get() != DTP_NULL))
        return inherited::m_valueBridge->getAs<T>();
      else {
        return inherited::m_node->template get<T>(inherited::m_pos);
      }
  }

  T get() const
  {
      return **this;
  }

  //scalar_iterator&
  //operator=(const T& value)
  scalar_iterator&
  set(const T& value)
  {
      //if ((m_pos == dnode::npos) && (m_valueBridge.get() != DTP_NULL))
      if ((inherited::m_pos == Const::npos) && (inherited::m_valueBridge.get() != DTP_NULL))
          inherited::m_valueBridge->setAs(value);
      else
          inherited::m_node->set(inherited::m_pos, value);
      return(*this);
  }

  //const scalar_iterator& operator*() const
  //{
  //    return(*this);
  //}

  //scalar_iterator& operator*()
  //{
  //    return(*this);
  //}

  difference_type operator-(const self_type& rhs) const {
    const_cast<self_type *>(this)->checkPos();
    return inherited::m_pos - rhs.m_pos;
  }

  self_type operator+(difference_type rhs) const {
    self_type i(*this); i.m_pos += rhs; return i;
  }

  self_type operator-(difference_type rhs) const {
    self_type i(*this); i.m_pos -= rhs; return i;
  }

  self_type &operator+=(difference_type rhs) {
    inherited::checkPos();
    inherited::m_pos += rhs; return *this;
  }

  self_type &operator-=(difference_type rhs) {
    inherited::checkPos();
    inherited::m_pos -= rhs; return *this;
  }

//protected:
//  DTP_UNIQUE_PTR(dnValueBridge) m_valueBridge;
}; // scalar_iterator

protected:
    dnode::dnValueBridge *newValueBridge(int idx);
    dnode::dnValueBridge *newValueBridge(const char *name);
    dnode::dnValueBridge *newValueBridge(const dtpString &name);
    dnode::dnValueBridge *newValueBridge(dnPos pos);
    dnode::dnValueBridge *intNewValueBridgeForChildName(const dtpString &name);

    void deleteValueBridge(dnode::dnValueBridge *bridge);

public:
    typedef dnConstIterator const_iterator;
    typedef dnIterator iterator;

        iterator begin()
        {
            return iterator(this, static_cast<size_type>(0));
        }

       iterator at(int idx)
        {
            return iterator(this, idx);
        }

       iterator at(const dtpString &name)
        {
            return iterator(this, name);
        }

       iterator at(const char *name)
        {
            return iterator(this, name);
        }

        template<typename T>
       iterator at(scalar_iterator<T> it)
        {
            return iterator(this, it.m_pos);
        }

       iterator find(const dnode &value) const
        {
            size_type idx = intIndexOfValue(value);
            if (idx != dnode::npos)
              return iterator(const_cast<dnode *>(this), static_cast<uint64>(idx));
            else
              return const_cast<dnode *>(this)->end();
        }

       void erase(iterator it)
       {
            size_type idx = (it - begin());
            eraseElement(idx);
       }

       void erase(const dtpString &name)
       {
            erase(at(name));
       }

       iterator findByName(const dtpString &name) const
        {
            size_type idx = indexOfName(name);
            if (idx != dnode::npos)
              return iterator(const_cast<dnode *>(this), static_cast<uint64>(idx));
            else
              return const_cast<dnode *>(this)->end();
        }

        iterator end()
        {
            return iterator(this, size());
        }

        const_iterator begin() const
        {
            return const_iterator(const_cast<dnode *>(this), static_cast<size_type>(0));
        }

        const_iterator at(int idx) const
        {
            return const_iterator(const_cast<dnode *>(this), idx);
        }

        template<typename T>
        const_iterator at(const_scalar_iterator<T> it) const
        {
            return const_iterator(const_cast<dnode *>(this), it.m_pos);
        }

        const_iterator at(const dtpString &name) const
        {
            return const_iterator(const_cast<dnode *>(this), name);
        }

        const_iterator at(const char *name) const
        {
            return const_iterator(const_cast<dnode *>(this), name);
        }

        const_iterator end() const
        {
            return const_iterator(const_cast<dnode *>(this), size());
        }

        const_iterator cbegin() const
        {
            return const_iterator(const_cast<dnode *>(this), static_cast<size_type>(0));
        }

        const_iterator cend() const
        {
            return const_iterator(const_cast<dnode *>(this), size());
        }

        template<typename T>
        scalar_iterator<T> scalarBegin()
        {
            return scalar_iterator<T>(*this, static_cast<size_type>(0));
        }

        template<typename T>
        scalar_iterator<T> scalarEnd()
        {
            return scalar_iterator<T>(*this, size());
        }

        template<typename T>
        const_scalar_iterator<T> scalarBeginR() const
        {
            return const_scalar_iterator<T>(*this, static_cast<size_type>(0));
        }

        template<typename T>
        const_scalar_iterator<T> scalarEndR() const
        {
            return const_scalar_iterator<T>(*this, size());
        }

        template<typename T>
        scalar_iterator<T> scalarAt(int idx)
        {
            return scalar_iterator<T>(*this, idx);
        }

        template<typename T>
        scalar_iterator<T> scalarAt(const dtpString &name)
        {
    //        size_type idx = indexOfName(name);
    //        if (idx != dnode::npos)
    //          return scalar_iterator<T>(*this, idx);
    //        else
    //          return scalarEnd<T>();
            return scalar_iterator<T>(*this, name);
        }

        template<typename T>
        scalar_iterator<T> scalarAt(const char *name)
        {
            size_type idx = indexOfName(name);
            if (idx != dnode::npos)
              return scalar_iterator<T>(*this, idx);
            else
              return scalarEnd<T>();
        }


        template<typename T>
        const_scalar_iterator<T> scalarAtR(int idx) const
        {
            return const_scalar_iterator<T>(*this, idx);
        }

        template<typename T>
        const_scalar_iterator<T> scalarAtR(const dtpString &name) const
        {
            size_type idx = indexOfName(name);
            if (idx != dnode::npos)
              return const_scalar_iterator<T>(*this, idx);
            else
              return scalarEndR<T>();
        }

        template<typename T>
        const_scalar_iterator<T> scalarAtR(const char *name) const
        {
            size_type idx = indexOfName(name);
            if (idx != dnode::npos)
              return const_scalar_iterator<T>(*this, idx);
            else
              return scalarEndR<T>();
        }

//------ insert -----------
    template<typename ValueType>
      typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, iterator>::type
        insert(iterator position, ValueType value)
    {
      size_type pos = position - begin();

      if (isParent() || isNull())
        intAddChildAtPos(pos, new dnode(value));
      else
        addItemAtPos(pos, value);

      return begin() + pos;
    }

    template<typename ValueType>
      typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, iterator>::type
        insert(iterator position, const ValueType &value)
    {
      size_type pos = position - begin();

      if (isParent() || isNull())
        intAddChildAtPos(pos, new dnode(value));
      else
        addItemAtPos(pos, value);

      return begin() + pos;
    }

    template<typename ValueType>
      typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
        insertAtPos(size_type pos, ValueType value)
    {
      if (isArray())
        addItemAtPos(pos, value);
      else
        intAddChildAtPos(pos, new dnode(value));
    }

    template<typename ValueType>
      typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
        insertAtPos(size_type pos, const ValueType &value)
    {
      if (isArray())
        addItemAtPos(pos, value);
      else
        intAddChildAtPos(pos, new dnode(value));
    }

    iterator insert(iterator position, dnode *value)
    {
      size_type pos = position - begin();

      if (isParent() || isNull())
        intAddChildAtPos(pos, value);
      else
        throw dnError("Wrong container type!");

      return begin() + pos;
    }

    template<typename ValueType>
    iterator insert(iterator position, const dtpString &aName, const ValueType &value)
    {
      size_type pos = position - begin();
      intAddChildAtPos(pos, aName, new dnode(value));
      return begin() + pos;
    }


//------ algorithms -----------

//-- visit ---
    template<typename ValueType, typename Visitor>
    void visitTreeValues(Visitor visitor)
    {
      if (!isContainer()) {
        visitor(getAs<ValueType>());
      } else if (isParent()) {
        const dnChildColnBaseIntf *parent = &getChildrenR();
        typedef typename Details::dnParentVisitorMeta<ValueType>::visitor_category visitor_category;
        typedef typename Details::ParentVisitorGeneric<visitor_category> parent_visitor;
        parent_visitor::template visitTreeValues<ValueType, Visitor>(parent, visitor);

      } else if (isArray()) {
        const dnArray *arr = getArrayR();
        arr->visitTreeValues<ValueType, Visitor>(visitor);
      }
    }

    template<typename ValueType, typename Visitor>
    void visitTreeNodes(Visitor visitor)
    {
      if (!isContainer()) {
        visitor(*this);
      } else if (isParent()) {
        const dnChildColnBaseIntf *parent = &getChildrenR();
        typedef typename Details::dnParentVisitorMeta<ValueType>::visitor_category visitor_category;
        typedef typename Details::ParentVisitorGeneric<visitor_category> parent_visitor;

        //if (isList())
        //  Details::ParentVisitor<ict_list>::visitTreeNodes<ValueType, Visitor>(parent, visitor);
        //else
        //  Details::ParentVisitor<ict_parent>::visitTreeNodes<ValueType, Visitor>(parent, visitor);
        parent_visitor::template visitTreeNodes<ValueType, Visitor>(parent, visitor);
      } else if (isArray()) {
        const dnArray *arr = getArrayR();
        arr->visitTreeNodes<ValueType, Visitor>(visitor);
      }
    }

    template<typename ValueType, typename Visitor>
    Visitor visitVectorValues(Visitor visitor) const
    {
      using namespace Details;

      if (!isContainer()) {
        visitor(getAs<ValueType>());
        return (visitor);
      } else if (isParent()) {
        const dnChildColnBaseIntf *parent = &getChildrenR();
        typedef typename Details::dnParentVisitorMeta<ValueType>::visitor_category visitor_category;
        typedef typename Details::ParentVisitorGeneric<visitor_category> parent_visitor;

        //if (isList())
        //  Details::ParentVisitor<ict_list>::visitVectorValues<ValueType, Visitor>(parent, visitor);
        //else
        //  Details::ParentVisitor<ict_parent>::visitVectorValues<ValueType, Visitor>(parent, visitor);
        return parent_visitor::template visitVectorValues<ValueType, Visitor>(parent, visitor);
      } else if (isArray()) {
        const dnArray *arr = getArrayR();
        return (arr->visitVectorValues<ValueType, Visitor>(visitor));
      } else {
        return (visitor);
      }
    }

    template<typename ValueType, typename Visitor>
    void visitVectorNodes(Visitor visitor)
    {
      if (!isContainer()) {
        visitor(*this);
      } else if (isParent()) {
        const dnChildColnBaseIntf *parent = &getChildrenR();
        typedef typename Details::dnParentVisitorMeta<ValueType>::visitor_category visitor_category;
        typedef typename Details::ParentVisitorGeneric<visitor_category> parent_visitor;

        //if (isList())
        //  Details::ParentVisitor<ict_list>::visitVectorNodes<ValueType, Visitor>(parent, visitor);
        //else
        //  Details::ParentVisitor<ict_parent>::visitVectorNodes<ValueType, Visitor>(parent, visitor);
        parent_visitor::template visitVectorNodes<ValueType, Visitor>(parent, visitor);
      } else if (isArray()) {
        const dnArray *arr = getArrayR();
        arr->visitVectorNodes<ValueType, Visitor>(visitor);
      }
    }

//-- scan ---
    //template<typename ValueType, typename Visitor>
    //bool scanTreeValues(Visitor visitor)
    //{
    //  if (!isContainer()) {
    //    return visitor(getAs<ValueType>());
    //  } else if (isParent()) {
    //    const dnChildColnBase &parent = getChildrenR();

    //    if (isList())
    //      return parent.scanTreeValues<ValueType, Visitor, dnChildColnList>(visitor);
    //    else
    //      return parent.scanTreeValues<ValueType, Visitor, dnChildColnDblMap>(visitor);
    //  } else if (isArray()) {
    //    const dnArray *arr = getArrayR();
    //    return arr->scanTreeValues<ValueType, Visitor>(visitor);
    //  } else {
    //    return false;
    //  }
    //}

    //template<typename ValueType, typename Visitor>
    //bool scanTreeNodes(Visitor visitor)
    //{
    //  if (!isContainer()) {
    //    return visitor(*this);
    //  } else if (isParent()) {
    //    const dnChildColnBase &parent = getChildrenR();

    //    if (isList())
    //      return parent.scanTreeNodes<ValueType, Visitor, dnChildColnList>(visitor);
    //    else
    //      return parent.scanTreeNodes<ValueType, Visitor, dnChildColnDblMap>(visitor);
    //  } else if (isArray()) {
    //    const dnArray *arr = getArrayR();
    //    return arr->scanTreeNodes<ValueType, Visitor>(visitor);
    //  } else {
    //    return false;
    //  }
    //}

    //template<typename ValueType, typename Visitor>
    //bool scanVectorValues(Visitor visitor) const
    //{
    //  if (!isContainer()) {
    //    return visitor(getAs<ValueType>());
    //  } else if (isParent()) {
    //    const dnChildColnBase &parent = getChildrenR();

    //    if (isList())
    //      return parent.scanVectorValues<ValueType, Visitor, Details::dnChildColnList>(visitor);
    //    else
    //      return parent.scanVectorValues<ValueType, Visitor, Details::dnChildColnDblMap>(visitor);
    //  } else if (isArray()) {
    //    const dnArray *arr = getArrayR();
    //    return arr->scanVectorValues<ValueType, Visitor>(visitor);
    //  } else {
    //    return false;
    //  }
    //}

    //template<typename ValueType, typename Visitor>
    //bool scanVectorNodes(Visitor visitor)
    //{
    //  if (!isContainer()) {
    //    return visitor(*this);
    //  } else if (isParent()) {
    //    const dnChildColnBase &parent = getChildrenR();

    //    if (isList())
    //      return parent.scanVectorNodes<ValueType, Visitor, dnChildColnList>(visitor);
    //    else
    //      return parent.scanVectorNodes<ValueType, Visitor, dnChildColnDblMap>(visitor);
    //  } else if (isArray()) {
    //    const dnArray *arr = getArrayR();
    //    return arr->scanVectorNodes<ValueType, Visitor>(visitor);
    //  } else {
    //    return false;
    //  }
    //}

//-- binary search ---
    template<typename ValueType, typename IntCompareOp>
    bool binarySearchValue(const ValueType &value, IntCompareOp compOp) const
    {
      using namespace Details;

      int temp;
      if (isArray())
      {
        const dnArray *arr = getArrayR();
        return arr->binarySearchValue(value, compOp, temp);
      }
      else if (isParent())
      {
        typedef typename Details::dnParentVisitorMeta<ValueType>::visitor_category visitor_category;
        typedef typename Details::ParentVisitorGeneric<visitor_category> parent_visitor;
        //dnChildColnBase *children = const_cast<dnChildColnBase *>(&(getChildrenR()));
        //return children->binarySearchValue<ValueType, IntCompareOp>(value, compOp, temp);
        dnChildColnBaseIntf *children = const_cast<dnChildColnBaseIntf *>(getAsChildrenIntfR());
        //return ParentVisitor<ict_parent>::binarySearchValue<ValueType, IntCompareOp>(children, value, compOp, temp);
        return parent_visitor::template binarySearchValue<ValueType, IntCompareOp>(children, value, compOp, temp);
      } else {
        return false;
      }
    }

    template<typename LtCompareOp>
    bool binarySearchNode(const dnode &value, LtCompareOp compOp) const
    {
      int temp;
      if (isArray())
      {
        const dnArray *arr = getArrayR();
        return arr->binarySearchNode<LtCompareOp>(value, compOp, temp);
      }
      else if (isParent())
      {
        typedef typename Details::dnParentVisitorMeta<LtCompareOp>::visitor_category visitor_category;
        typedef typename Details::ParentVisitorGeneric<visitor_category> parent_visitor;
        dnChildColnBaseIntf *children = const_cast<dnChildColnBaseIntf *>(getAsChildrenIntfR());
        //return Details::ParentVisitor<ict_parent>::binarySearchNode(children, value, compOp, temp);
        return parent_visitor::template binarySearchNode(children, value, compOp, temp);
      } else {
        return false;
      }
    }

//-- sort ---
    template<typename ValueType, typename CompareOp>
    void sortValues(CompareOp compOp)
    {
      if (isArray())
      {
        dnArray *arr = getArray();
        arr->sortValues<ValueType, CompareOp>(compOp);
      }
      else if (isParent())
      {
        typedef typename Details::dnParentVisitorMeta<ValueType>::visitor_category visitor_category;
        typedef typename Details::ParentVisitorGeneric<visitor_category> parent_visitor;
        //return children->sortValues<ValueType, CompareOp>(compOp);
        dnChildColnBaseIntf *children = const_cast<dnChildColnBaseIntf *>(getAsChildrenIntfR());
        //return Details::ParentVisitor<ict_parent>::sortValues<ValueType, CompareOp>(children, compOp);
        return parent_visitor::template sortValues<ValueType, CompareOp>(children, compOp);
      }
    }

    template<typename CompareOp>
    void sortNodes(CompareOp compOp)
    {
      if (isArray())
      {
        dnArray *arr = getArray();
        arr->sortNodes<CompareOp>(compOp);
      }
      else if (isParent())
      {
        typedef typename Details::dnParentVisitorMeta<CompareOp>::visitor_category visitor_category;
        typedef typename Details::ParentVisitorGeneric<visitor_category> parent_visitor;
        //dnChildColnBase *children = const_cast<dnChildColnBase *>(&(getChildrenR()));
        //return children->sortNodes<CompareOp>(compOp);
        dnChildColnBaseIntf *children = const_cast<dnChildColnBaseIntf *>(getAsChildrenIntfR());
        //return Details::ParentVisitor<ict_parent>::sortNodes<CompareOp>(children, compOp);
        return parent_visitor::template sortNodes<CompareOp>(children, compOp);
      }
    }

//-- find ---
    template<typename ValueType>
    dnode::iterator find(ValueType value) const
    {
      return do_find(value);
    }

    template<typename ValueType, typename CompareOp>
    iterator find_if(ValueType value, CompareOp compOp) const
    {
      size_type idx;
      if (isArray())
      {
        dnArray *arr = const_cast<dnode *>(this)->getArray();
        idx = arr->find_if<ValueType>(value, compOp);
        if (idx >= arr->size())
          idx = npos;
      }
      else if (isParent())
      {
        typedef typename Details::dnParentVisitorMeta<ValueType>::visitor_category visitor_category;
        typedef typename Details::ParentVisitorGeneric<visitor_category> parent_visitor;
        //dnChildColnBase *children = const_cast<dnChildColnBase *>(&(getChildrenR()));
        //idx = children->find_if<ValueType>(value, compOp);
        dnChildColnBaseIntf *children = const_cast<dnChildColnBaseIntf *>(&(getChildrenR()));
        //idx = Details::ParentVisitor<ict_parent>::find_if(children, value, compOp);
        idx = parent_visitor::template find_if(children, value, compOp);

        if (idx >= children->size())
          idx = npos;
      } else {
        idx = npos;
      }

      if (idx != npos)
        return (const_cast<dnode *>(this)->begin()) + idx;
      else
        return const_cast<dnode *>(this)->end();
    }

    /// Calculate average value, result = sum(a[i]) / size \n
    /// \param[in] init Initial value.
    /// \result Returns 0 when node is empty.
    template <class T>
    T avg (T init)
    {
      init = accumulate(init);
      int cnt = size();
      if (cnt == 0)
        cnt = 1;
      return init / static_cast<T>(cnt);
    }

    /// sum items, result = a[0] + a[1] + ... + a[n]
    template<class T>
    T accumulate(T init) const
    {
      T sum;
      //dnSumVisitor<T> a_sum_visitor(sum, init);
      //visitVectorValues<T>(a_sum_visitor);
      dnSumVisitorFast<T> a_sum_visitor(init);
      sum = visitVectorValues<T>(a_sum_visitor).total();
      return sum;
    }

    /// product of items, result = a[0] * a[1] * ... * a[n]
    template<class T>
    T product(T init) const
    {
      T prod;
      dnMultVisitor<T> a_mult_visitor(prod, init);
      visitVectorValues<T>(a_mult_visitor);
      return prod;
    }

    /// Calculate standard deviation - data node-specific version
    template<class T>
    T std_dev(T defValue ) const
    {
      return ::std_dev(scalarBeginR<T>(),
                     scalarEndR<T>(),
                     defValue);
    }

    /// Find position of minimum value
    template<typename T>
       dtp::dnode::iterator
         min_element()
    {
      dtp::dnode helper1, helper2;
      dtp::dnode::iterator it = begin();
      dtp::dnode::iterator last = end();
      dtp::dnode::iterator res = it;

      while (it!=last)
      {
        if (it->getAs<T>(helper1) < res->getAs<T>(helper2))
        {
          res = it;
        }
        ++it;
      }

      return res;
    }

    /// Find position of maximum value
    template<typename T>
      dtp::dnode::iterator
        max_element()
    {
      dtp::dnode helper1, helper2;
      dtp::dnode::iterator it = begin();
      dtp::dnode::iterator res = it;
      dtp::dnode::iterator last = end();

      while (it!=last)
      {
        if (it->getAs<T>(helper1) > res->getAs<T>(helper2))
        {
          res = it;
        }
        ++it;
      }

      return res;
    }

    /// Find minimum & maximum element
    template<typename T>
      std::pair<dtp::dnode::iterator, dtp::dnode::iterator>
        minmax_element()
    {
      dtp::dnode helper1, helper2;
      dtp::dnode::iterator it = begin();
      dtp::dnode::iterator last = end();
      dtp::dnode::iterator min_it = it;
      dtp::dnode::iterator max_it = it;

      while (it!=last)
      {
        if (it->getAs<T>(helper1) < min_it->getAs<T>(helper2))
        {
          min_it = it;
        }
        if (it->getAs<T>(helper1) > max_it->getAs<T>(helper2))
        {
          max_it = it;
        }
        ++it;
      }

      return std::make_pair(min_it, max_it);
    }

    /// Perform function for each element of node
    template<typename T, typename FuncOp>
    FuncOp for_each(FuncOp f)
    {
      return visitVectorValues<T>(f);
    }

    /// Calculate how many given values appear inside container
    template<typename T>
      dtp::dnode::size_type
        count(const T &value)
    {
      dtp::dnode helper;
      dtp::dnode::size_type res = 0;
      dtp::dnode::iterator first = begin();
      dtp::dnode::iterator last = end();

      while (first!=last)
      {
        if (first->getAs<T>(helper) == value)
          res++;
        ++first;
      }

      return res;
    }

    /// Calculate how many items satisfying given predicator object exist inside container
    template<typename T, typename Predicator>
    dtp::dnode::size_type count_if(Predicator pred)
    {
      dtp::dnode helper;
      dtp::dnode::size_type res = 0;
      dtp::dnode::iterator first = begin();
      dtp::dnode::iterator last = end();

      while (first!=last)
      {
        if (pred(first->getAs<T>(helper)))
          res++;
        ++first;
      }

      return res;
    }

    /// Check if all elements satisfy predicator
    template<typename T, typename Predicator>
    bool all_of(Predicator pred) const
    {
      dtp::dnode::const_scalar_iterator<T> first = scalarBeginR<T>();
      dtp::dnode::const_scalar_iterator<T> last = scalarEndR<T>();

      while (first!=last)
      {
        if (!pred(*first))
          return false;
        ++first;
      }

      return true;
    }

    /// Check if any element satisfy predicator
    template<typename T, typename Predicator>
    bool any_of(Predicator pred) const
    {
      dtp::dnode::const_scalar_iterator<T> first = scalarBeginR<T>();
      dtp::dnode::const_scalar_iterator<T> last = scalarEndR<T>();

      while (first!=last)
      {
        if (pred(*first))
          return true;
        ++first;
      }

      return false;
    }

    /// Check if no element satisfy predicator
    template<typename T, typename Predicator>
    bool none_of(Predicator pred) const
    {
      dtp::dnode::const_scalar_iterator<T> first = scalarBeginR<T>();
      dtp::dnode::const_scalar_iterator<T> last = scalarEndR<T>();

      while (first!=last)
      {
        if (pred(*first))
          return false;
        ++first;
      }

      return true;
    }

    /// fill items with value
    template<typename T>
    void fill(const T &value)
    {
      dtp::dnode helper;
      dtp::dnode::iterator first = begin();
      dtp::dnode::iterator last = end();

      while (first!=last)
      {
        first->setAs<T>(value, helper);
        ++first;
      }
    }

    /// fill items with value, n first items
    template<typename T>
    void fill_n(size_t n, const T &value)
    {
      dtp::dnode helper;
      dtp::dnode::scalar_iterator<T> first = scalarBegin<T>();

      while (n > 0)
      {
        //*first = value;
        first.set(value);
        ++first;
        n--;
      }
    }

    template<>
    void fill_n<dnode>(size_t n, const dnode &value)
    {
      dtp::dnode helper;
      dtp::dnode::iterator first = begin();

      while (n > 0)
      {
        //*first = value;
        first->setAs(value);
        ++first;
        n--;
      }
    }

    template<typename ArgType, typename TargetType>
    void fill_n(size_t n, const ArgType &value)
    {
      dtp::dnode helper;
      dtp::dnode::iterator first = begin();

      while (n > 0)
      {
        first->setAs<TargetType>(value);
        ++first;
        n--;
      }
    }

    /// fill items with generator
    template<typename T, typename Generator>
    void generate(Generator gen)
    {
      dtp::dnode helper;
      dtp::dnode::iterator first = begin();
      dtp::dnode::iterator last = end();

      while (first!=last)
      {
        first->setAs<T>(gen(), helper);
        ++first;
      }
    }

    /// fill items with generator
    template<typename T, typename Generator, typename OutputIterator>
    void generate(OutputIterator result, Generator gen)
    {
      dtp::dnode helper;
      dtp::dnode::iterator first = begin();
      dtp::dnode::iterator last = end();

      while (first!=last)
      {
        result->setAs<T>(gen(), helper);
        ++first;
        ++result;
      }
    }

    /// fill items with generator, n first items
    template<typename T, typename Generator>
    void generate_n(dtp::dnode::iterator first, size_t n, Generator gen)
    {
      dtp::dnode helper;
      while (n > 0)
      {
        first->setAs<T>(gen(), helper);
        ++first;
        n--;
      }
    }

    /// fill items with generator{hasNext(), next()}
    template<typename T, typename Generator>
    void generate_while(Generator gen)
    {
      dtp::dnode helper;
      dtp::dnode::iterator first = begin();

      while (gen.hasNext())
      {
        first->setAs<T>(gen.next(), helper);
        ++first;
      }
    }

    /// fill items with generator{hasNext(), next()}
    template<typename T, typename Generator, typename OutputIterator>
    void generate_while(OutputIterator result, Generator gen)
    {
      dtp::dnode helper;

      while (gen.hasNext())
      {
        result->setAs<T>(gen.next(), helper);
        ++result;
      }
    }

    /// replace item with new value if item equal to old value
    template<typename T>
    void replace(const T &oldValue, const T &newValue)
    {
      dtp::dnode helper;
      dtp::dnode::iterator first = begin();
      dtp::dnode::iterator last = end();

      while (first!=last)
      {
        //if (first->getAs<T>() == oldValue)
        if (first->getAs<T>(helper) == oldValue)
        {
          //first->setAs<T>(newValue);
          first->setAs<T>(newValue, helper);
        }
        ++first;
      }
    }

    /// replace item with new value if predicate returns true
    template<typename T, typename Predicate>
    void replace_if(Predicate pred, const T &newValue)
    {
      dtp::dnode helper;
      dtp::dnode::iterator first = begin();
      dtp::dnode::iterator last = end();

      while (first!=last)
      {
        if (pred(first->getAs<T>(helper)))
        {
          first->setAs<T>(newValue, helper);
        }
        ++first;
      }
    }

    template<typename T, typename TransOp, typename OutputIterator>
    OutputIterator transform(OutputIterator result, TransOp transOp)
    {
      dtp::dnode helper;
      dtp::dnode::iterator first = begin();
      dtp::dnode::iterator last = end();

      while (first!=last)
      {
        result->setAs<T>(transOp(first->getAs<T>(helper)), helper);
        ++first;
        ++result;
      }

      return result;
    }

    template<typename T, typename TransOp>
    dtp::dnode::scalar_iterator<T> transform(dtp::dnode::scalar_iterator<T> result, TransOp transOp)
    {
      dtp::dnode helper;
      dtp::dnode::scalar_iterator<T> first = scalarBegin<T>();
      dtp::dnode::scalar_iterator<T> last = scalarEnd<T>();

      while (first!=last)
      {
        result.set(transOp(*first));
        ++first;
        ++result;
      }

      return result;
    }

    template<typename T, typename TransOp>
    dnode::iterator transform(TransOp transOp)
    {
      return transform<T>(this->begin(), transOp);
    }

    /// Find first element >= value
    // based on http://www.cplusplus.com/reference/algorithm/lower_bound/
    template <class T>
    dtp::dnode::iterator lower_bound(const T& value)
    {
      dtp::dnode::iterator first = begin();
      dtp::dnode::iterator last = end();
      dtp::dnode::iterator it;
      dtp::dnode::size_type count, step;
      count = last - first;
      dtp::dnode helper;

      while (count>0)
      {
        it = first;
        step = count / 2;

        std::advance(it, step);

        if (it->getAs<T>(helper) < value) {
          first = ++it;
          count -= (step + 1);
        } else {
          count = step;
        }
      }
      return first;
    }

    /// Find first element >= value
    // based on http://www.cplusplus.com/reference/algorithm/lower_bound/
    template <class T>
    dtp::dnode::size_type lower_bound_index(const T& value)
    {
      dtp::dnode::size_type first = 0;
      dtp::dnode::size_type last = size();
      dtp::dnode::size_type i;
      dtp::dnode::size_type count, step;
      //dtp::dnode helper;
      count = last - first;

      while (count>0)
      {
        i = first;
        step = count / 2;

        i += step;

        if (get<T>(i) < value) {
          first = ++i;
          count -= (step + 1);
        } else {
          count = step;
        }
      }
      return first;
    }

    /// Find first element >= value
    // based on http://www.cplusplus.com/reference/algorithm/lower_bound/
    template <class T, typename CompOp>
    dtp::dnode::iterator lower_bound(const T& value, CompOp compOp)
    {
      dtp::dnode::iterator first = begin();
      dtp::dnode::iterator last = end();
      dtp::dnode::iterator it;
      dtp::dnode::size_type count, step;
      dtp::dnode helper;
      count = last - first;

      while (count>0)
      {
        it = first;
        step = count / 2;
        it = it + step;

        if (compOp(it->getAs<T>(helper), value)) {
          first = ++it;
          count -= (step + 1);
        } else {
          count = step;
        }
      }
      return first;
    }

    /// Find first element >= value
    // based on http://www.cplusplus.com/reference/algorithm/lower_bound/
    template <class T, typename CompOp>
    dtp::dnode::size_type lower_bound_index(const T& value, CompOp compOp)
    {
      dtp::dnode::size_type first = 0;
      dtp::dnode::size_type last = size();
      dtp::dnode::size_type i;
      dtp::dnode::size_type count, step;
      //dtp::dnode helper;
      count = last - first;

      while (count>0)
      {
        i = first;
        step = count / 2;
        i = i + step;

        if (compOp(get<T>(i), value)) {
          first = ++i;
          count -= (step + 1);
        } else {
          count = step;
        }
      }
      return first;
    }

    /// Find first element >= value
    // based on http://www.cplusplus.com/reference/algorithm/lower_bound/
    template <typename CompOp>
    dtp::dnode::iterator lower_bound(const dtp::dnode& value, CompOp compOp)
    {
      dtp::dnode::iterator first = begin();
      dtp::dnode::iterator last = end();
      dtp::dnode::iterator it;
      dtp::dnode::size_type count, step;
      dtp::dnode helper;
      count = last - first;

      while (count>0)
      {
        it = first;
        step = count / 2;
        it = it + step;

        if (compOp(it->getAsNode(helper), value)) {
          first = ++it;
          count -= (step + 1);
        } else {
          count = step;
        }
      }
      return first;
    }

    /// Find first element > value
    // based on http://www.cplusplus.com/reference/algorithm/upper_bound/
    template <class T>
    dtp::dnode::iterator upper_bound(const T& value)
    {
      dtp::dnode::iterator first = begin();
      dtp::dnode::iterator last = end();
      dtp::dnode::iterator it;
      dtp::dnode::size_type count, step;
      dtp::dnode helper;
      count = last - first;

      while (count>0)
      {
        it = first;
        step = count / 2;
        it = it + step;

        if (!(value < it->getAs<T>(helper)))
        {
          first = ++it;
          count -= (step + 1);
        } else {
          count = step;
        }
      }

      return first;
    }

    /// Find first element > value
    // based on http://www.cplusplus.com/reference/algorithm/upper_bound/
    template <class T>
    dtp::dnode::size_type upper_bound_index(const T& value)
    {
      dtp::dnode::size_type first = 0;
      dtp::dnode::size_type last = size();
      dtp::dnode::size_type it;
      dtp::dnode::size_type count, step;
      //dtp::dnode helper;
      count = last - first;

      while (count>0)
      {
        it = first;
        step = count / 2;
        it = it + step;

        if (!(value < get<T>(it)))
        {
          first = ++it;
          count -= (step + 1);
        } else {
          count = step;
        }
      }

      return first;
    }

    /// Find first element > value
    // based on http://www.cplusplus.com/reference/algorithm/upper_bound/
    template <class T, typename CompOp>
    dtp::dnode::iterator upper_bound(const T& value, CompOp compOp)
    {
      dtp::dnode::iterator first = begin();
      dtp::dnode::iterator last = end();
      dtp::dnode::iterator it;
      dtp::dnode::size_type count, step;
      dtp::dnode helper;
      count = last - first;

      while (count>0)
      {
        it = first;
        step = count / 2;
        it = it + step;

        if (!(compOp(value, it->getAs<T>(helper))))
        {
          first = ++it;
          count -= (step + 1);
        } else {
          count = step;
        }
      }

      return first;
    }

    /// Find first element > value
    // based on http://www.cplusplus.com/reference/algorithm/upper_bound/
    template <typename CompOp>
    dtp::dnode::iterator upper_bound(const dtp::dnode& value, CompOp compOp)
    {
      dtp::dnode::iterator first = begin();
      dtp::dnode::iterator last = end();
      dtp::dnode::iterator it;
      dtp::dnode::size_type count, step;
      dtp::dnode helper;
      count = last - first;

      while (count>0)
      {
        it = first;
        step = count / 2;
        it = it + step;
        if (!(compOp(value, it->getAsNode(helper))))
        {
          first = ++it;
          count -= (step + 1);
        } else {
          count = step;
        }
      }

      return first;
    }

    /// Check if node-as-container is sorted ascending
    template <typename T>
    bool is_sorted()
    {
      bool res = true;

      for(size_type i=1, epos = size(); i != epos; i++)
      {
        if (get<T>(i - 1) > get<T>(i))
        {
          res = false;
          break;
        }
      }

      return res;
    }

    /// Array-only sort
    /// \result Returns <true> if data has been sorted.
    bool sort();

    /// Sort values using provided data type.
    template<typename ValueType>
    void sort()
    {
      sortValues<ValueType>(dtpSignCompareOp<ValueType>());
    }

    void unique()
    {
      iterator it = std::unique(begin(), end());
      resize(it - begin());
    }

    /// Sort values using provided compare operator (int).
    template<typename ValueType, typename IntCompareOp>
    void sort(IntCompareOp compareOp)
    {
      sortValues<ValueType>(compareOp);
    }

    /// \brief Verify if container has item with provided value.
    /// \description Container must be sorted.
    template<typename ValueType>
    bool binary_search(ValueType value)
    {
      return binarySearchValue<ValueType, dtpSignCompareOp<ValueType> >(value, dtpSignCompareOp<ValueType>());
    }

protected:
    template<typename ValueType>
    class dnSumVisitor {
    public:
      dnSumVisitor(ValueType &sumRef, ValueType init): m_sumRef(sumRef) { sumRef = init; }
      void operator()(ValueType v) {
        m_sumRef += v;
      }
    protected:
      ValueType &m_sumRef;
    };

    template<typename ValueType>
    class dnSumVisitorFast {
    public:
      dnSumVisitorFast(ValueType init): m_sum(init) { }
      void operator()(ValueType v) {
        m_sum += v;
      }
      ValueType total() { return m_sum; };
    protected:
      ValueType m_sum;
    };


    template<typename ValueType>
    class dnMultVisitor {
    public:
      dnMultVisitor(ValueType &sumRef, ValueType init): m_sumRef(sumRef) { sumRef = init; }
      void operator()(ValueType v) {
        m_sumRef *= v;
      }
    protected:
      ValueType &m_sumRef;
    };

template<typename ValueType>
    iterator do_find(ValueType value) const
    {
      size_type idx;
      if (isArray())
      {
        dnArray *arr = const_cast<dnode *>(this)->getArray();
        idx = arr->find<ValueType>(value);
        if (idx >= arr->size())
          idx = npos;
      }
      else if (isParent())
      {
        typedef typename Details::dnParentVisitorMeta<ValueType>::visitor_category visitor_category;
        typedef typename Details::ParentVisitorGeneric<visitor_category> parent_visitor;
        //idx = children->find(value);
        dnChildColnBaseIntf *children = const_cast<dnChildColnBaseIntf *>(getAsChildrenIntfR());
        //idx = Details::ParentVisitor<ict_parent>::find(children, value);
        idx = parent_visitor::template find(children, value);
        if (idx >= children->size())
          idx = npos;
      } else {
        idx = npos;
      }

      if (idx != npos)
        return (const_cast<dnode *>(this)->begin()) + idx;
      else
        return const_cast<dnode *>(this)->end();
    }

    template<typename ValueType, typename CompareOp>
    dnode::size_type find_if_index(ValueType value, CompareOp compOp) const
    {
      size_type idx;
      if (isArray())
      {
        dnArray *arr = const_cast<dnode *>(this)->getArray();
        idx = arr->find_if(value, compOp);
        if (idx >= arr->size())
          idx = npos;
      }
      else if (isParent())
      {
        typedef typename Details::dnParentVisitorMeta<ValueType>::visitor_category visitor_category;
        typedef typename Details::ParentVisitorGeneric<visitor_category> parent_visitor;
        //dnChildColnBase *children = const_cast<dnChildColnBase *>(&(getChildrenR()));
        //idx = children->find_if(value, compOp);
        dnChildColnBaseIntf *children = const_cast<dnChildColnBaseIntf *>(&(getChildrenR()));
        //idx = Details::ParentVisitor<ict_parent>::find_if(children, value, compOp);
        idx = parent_visitor::template find_if(children, value, compOp);
        if (idx >= children->size())
          idx = npos;
      } else {
        idx = npos;
      }

      return idx;
    }

    template<typename ValueType>
    dnode::size_type find_index(ValueType value) const
    {
      size_type idx;
      if (isArray())
      {
        dnArray *arr = const_cast<dnode *>(this)->getArray();
        idx = arr->find<ValueType>(value);
        if (idx >= arr->size())
          idx = npos;
      }
      else if (isParent())
      {
        typedef typename Details::dnParentVisitorMeta<ValueType>::visitor_category visitor_category;
        typedef typename Details::ParentVisitorGeneric<visitor_category> parent_visitor;
        //dnChildColnBase *children = const_cast<dnChildColnBase *>(&(getChildrenR()));
        //idx = children->find(value);
        dnChildColnBaseIntf *children = const_cast<dnChildColnBaseIntf *>(getAsChildrenIntfR());
        //idx = Details::ParentVisitor<ict_parent>::find(children, value);
        idx = parent_visitor::find(children, value);
        if (idx >= children->size())
          idx = npos;
      } else {
        idx = npos;
      }

      return idx;
    }

private:
  friend class dnConstIterator;
public:

}; //dnode

// ----------------------------------------------------------------------------
// dnode explicit method specializations
// ----------------------------------------------------------------------------
template<>
inline dnValue dnode::dnValueBridge::getAs<dnValue>() const
{
  dnValue res;
  getAsItem(res);
  return res;
}

template<>
inline void dnode::dnValueBridge::setAs<dnValue>(const dnValue &newValue)
{
   setAsItem(newValue);
}

namespace Details {

// ----------------------------------------------------------------------------
// dnChildColnBase
// ----------------------------------------------------------------------------
class dnChildColnBase: public dnChildColnBaseIntf {
public:
  typedef uint size_type;
  dnChildColnBase();
  virtual ~dnChildColnBase() {};
  virtual void clear();
  virtual size_type size() const = 0;
  virtual void resize(size_type newSize) = 0;
  virtual bool empty() const = 0;
  virtual bool hasChild(const dtpString &name) const = 0;
  virtual const dtpString getName(int index) const = 0;
  virtual void setName(int index, const dtpString &name) = 0;
  virtual size_type indexOfName(const dtpString &name) const = 0;
  virtual size_type indexOfValue(const dnode &value) const = 0;
  virtual dnode &getByName(const dtpString &name);
  virtual const dnode &getByNameR(const dtpString &name) const;
  virtual dnode *peekChild(const dtpString &name);
  virtual const dnode *peekChildR(const dtpString &name) const;
  virtual dnode &at(int pos) = 0;
  virtual const dnode &at(int pos) const = 0;
  virtual void getChild(int index, dnode &output) = 0;
  virtual void erase(const dtpString &name) = 0;
  virtual void erase(int index) = 0;
  virtual void eraseFrom(int index) = 0;
  virtual bool isList() const {return false;}
  virtual dnode *cloneChild(int index) const = 0;
  virtual dnode *extractChild(int index) = 0;

  virtual void swap(size_type pos1, size_type pos2) = 0;

  // --- visit
  template<typename ValueType, typename Visitor, typename DerivedClass>
  void visitTreeValues(Visitor visitor) const
  {
     DerivedClass *derived = const_cast<DerivedClass *>(dynamic_cast<const DerivedClass *>(this));
     derived->visitTreeValues<ValueType>(visitor);
  }

  template<typename ValueType, typename Visitor, typename DerivedClass>
  void visitTreeNodes(Visitor visitor) const
  {
     DerivedClass *derived = const_cast<DerivedClass *>(dynamic_cast<const DerivedClass *>(this));
     derived->visitTreeNodes<ValueType>(visitor);
  }

  template<typename ValueType, typename Visitor, typename DerivedClass>
  Visitor visitVectorValues(Visitor visitor) const
  {
     DerivedClass *derived = const_cast<DerivedClass *>(dynamic_cast<const DerivedClass *>(this));
     return derived->visitVectorValues<ValueType>(visitor);
  }

  template<typename ValueType, typename Visitor, typename DerivedClass>
  void visitVectorNodes(Visitor visitor) const
  {
     DerivedClass *derived = const_cast<DerivedClass *>(dynamic_cast<const DerivedClass *>(this));
     derived->visitVectorNodes<ValueType>(visitor);
  }

  // --- scan
  template<typename ValueType, typename Visitor, typename DerivedClass>
  bool scanTreeValues(Visitor visitor) const
  {
     typedef typename DerivedClass::vector_type vector_type;
     typedef typename vector_type::iterator vector_iterator;
     DerivedClass *derived = const_cast<DerivedClass *>(dynamic_cast<const DerivedClass *>(this));
     vector_type &vector = derived->getItems();

     for(vector_iterator it = vector.begin(), epos = vector.end(); it != epos; ++it)
     {
       if ((*it).visitTreeValues<ValueType>(visitor))
         return true;
     }

     return false;
  }

  template<typename ValueType, typename Visitor, typename DerivedClass>
  bool scanTreeNodes(Visitor visitor) const
  {
     typedef typename DerivedClass::vector_type vector_type;
     typedef typename vector_type::iterator vector_iterator;
     DerivedClass *derived = const_cast<DerivedClass *>(dynamic_cast<const DerivedClass *>(this));
     vector_type &vector = derived->getItems();

     for(vector_iterator it = vector.begin(), epos = vector.end(); it != epos; ++it)
     {
       if ((*it).visitTreeNodes<ValueType>(visitor))
         return true;
     }

     return false;
  }

  template<typename ValueType, typename Visitor, typename DerivedClass>
  bool scanVectorValues(Visitor visitor) const
  {
     typedef typename DerivedClass::vector_type vector_type;
     typedef typename vector_type::iterator vector_iterator;
     DerivedClass *derived = const_cast<DerivedClass *>(dynamic_cast<const DerivedClass *>(this));
     vector_type &vector = derived->getItems();

#ifdef DTP_TPL_RES_SPEC_BY_TAG
     ValueType value;
#endif

     for(vector_iterator it = vector.begin(), epos = vector.end(); it != epos; ++it)
     {
       //value = (*it).template getAs<ValueType>();
       if (visitor((*it).template getAs<ValueType>()))
         return true;
     }

     return false;
  }

  template<typename ValueType, typename Visitor, typename DerivedClass>
  bool scanVectorNodes(Visitor visitor) const
  {
     typedef typename DerivedClass::vector_type vector_type;
     typedef typename vector_type::iterator vector_iterator;
     DerivedClass *derived = const_cast<DerivedClass *>(dynamic_cast<const DerivedClass *>(this));
     vector_type &vector = derived->getItems();
     for(vector_iterator it = vector.begin(), epos = vector.end(); it != epos; ++it)
     {
       if (visitor(*it))
         return true;
     }

     return false;
  }

  template<typename ValueType, typename CompOp>
  size_type find_if(ValueType value, CompOp compOp)
  {
    typedef typename Details::dnParentVisitorMeta<ValueType>::visitor_category visitor_category;
    typedef typename Details::ParentVisitorGeneric<visitor_category> parent_visitor;

    //if (isList())
    //  return Details::ParentVisitor<ict_list>::find_if_derived(this, value, compOp);
    //else
    //  return Details::ParentVisitor<ict_parent>::find_if_derived(this, value, compOp);
    return parent_visitor::template find_if_derived(this, value, compOp);
  }

  template<typename ValueType>
  size_type find(ValueType value)
  {
    typedef typename Details::dnParentVisitorMeta<ValueType>::visitor_category visitor_category;
    typedef typename Details::ParentVisitorGeneric<visitor_category> parent_visitor;
    using namespace Details;

    //if (isList())
    //    return Details::ParentVisitor<ict_list>::find_derived(this, value);
    //else
    //    return Details::ParentVisitor<ict_parent>::find_derived(this, value);
    //return parent_visitor::find_derived(this, value);
    return parent_visitor::find_derived(static_cast<dnChildColnBaseIntf *>(this), value);
  }

  template<typename ValueType, typename IntCompareOp>
  bool binarySearchValue(const ValueType &value, IntCompareOp compOp, int &foundPos) const
  {
     return binary_search_by_pos(0, size(), value, VectorItemCompPosGetAs<ValueType, IntCompareOp>(*this, compOp), foundPos);
  }

  template<typename IntCompareOp>
  bool binarySearchNode(const dnode &value, IntCompareOp compOp, int &foundPos) const
  {
     return binary_search_by_pos(0, size(), value, VectorItemCompPosAt<IntCompareOp>(*this, compOp), foundPos);
  }

  template<typename ValueType, typename CompareOp>
  void sortValues(CompareOp compOp)
  {
     typedef typename Details::dnParentVisitorMeta<ValueType>::visitor_category visitor_category;
     typedef typename Details::ParentVisitorGeneric<visitor_category> parent_visitor;

     //using namespace Details;
     //if (isList())
     //  ParentVisitor<ict_list>::sortValues(this, compOp);
     //else
     // ParentVisitor<ict_parent>::sortValues(this, compOp);
     parent_visitor::template sortValues(this, compOp);
  }

  template<typename CompareOp>
  void sortNodes(CompareOp compOp)
  {
     typedef typename Details::dnParentVisitorMeta<CompareOp>::visitor_category visitor_category;
     typedef typename Details::ParentVisitorGeneric<visitor_category> parent_visitor;
     //using namespace Details;
     //if (isList())
     //  ParentVisitor<ict_list>::sortNodes(this, compOp);
     //else
     //  ParentVisitor<ict_parent>::sortNodes(this, compOp);
     parent_visitor::template sortNodes(this, compOp);
}

  virtual void copyItemsFrom(const dnChildColnBase& src) = 0;
  virtual void insert(dnode *node) = 0;
  virtual void insert(size_type pos, dnode *node) = 0;
  virtual void insert(size_type pos, const dtpString &name, dnode *node) = 0;
  virtual void insert(const dtpString &name, dnode *node) = 0;
protected:
  virtual void setAt(int pos, dnode *node) = 0;
  virtual void copyFrom(const dnChildColnBase& src);
  virtual void clearItems() = 0;
  virtual dnode *createChild(const dnode &src) const {  return (new dnode(src)); }

  // comparator with internal container & at() / getAs<> access method
  template<typename ValueType, typename IntCompareOp>
  class VectorItemCompPosGetAs {
    public:
     VectorItemCompPosGetAs(const dnChildColnBase &childColn, IntCompareOp compOp): m_childColn(childColn), m_compOp(compOp) {}
     // compares item at a given pos with provided value
     int operator()(int pos, const ValueType &value)
     {
       const dnode &nodeRef = const_cast<dnChildColnBase &>(m_childColn).at(pos);
       ValueType valueAtPos = nodeRef.getAs<ValueType>();
       return m_compOp(valueAtPos, value);
     }
    protected:
      const dnChildColnBase &m_childColn;
      IntCompareOp m_compOp;
  };

  // comparator with internal container & at() access method
  template<typename IntCompareOp>
  class VectorItemCompPosAt {
    public:
     VectorItemCompPosAt(const dnChildColnBase &childColn, IntCompareOp compOp): m_childColn(childColn), m_compOp(compOp) {}
     // compares item at a given pos with provided value
     int operator()(int pos, const dnode &value)
     {
       const dnode &nodeRef = const_cast<dnChildColnBase &>(m_childColn).at(pos);
       return m_compOp(nodeRef, value);
     }
    protected:
      const dnChildColnBase &m_childColn;
      IntCompareOp m_compOp;
  };



private:
  friend class dnode;
};

} // namespace Details


namespace Details {

typedef boost::shared_ptr<dnode> dnodeTransport;

class dnChildColnList: public dnChildColnBase {
public:
  typedef dnodeColn vector_type;

  dnChildColnList();
  virtual ~dnChildColnList() {};

  virtual size_type size() const;
  virtual void resize(size_type newSize);
  virtual bool empty() const;
  virtual void clearItems();

  virtual void erase(const dtpString &name);
  virtual void erase(int index);
  virtual void eraseFrom(int index);

  virtual dnode &at(int pos) { return m_items[pos]; }
  virtual const dnode &at(int pos) const { return m_items[pos]; }
  virtual void getChild(int index, dnode &output);
  virtual size_type indexOfName(const dtpString &name) const;
  virtual size_type indexOfValue(const dnode &value) const;
  virtual bool hasChild(const dtpString &name) const;
  virtual const dtpString getName(int index) const;
  virtual void setName(int index, const dtpString &name);

  virtual bool isList() const {return true;}
  virtual bool supportsAccessByName() const { return false; }

  vector_type &getItems() { return m_items; }
  const vector_type &getItems() const { return m_items; }

  void swap(size_type pos1, size_type pos2)
  {
    if (pos1 == pos2)
      return;

    dnode dnode = this->at(pos1);
    this->at(pos1) = this->at(pos2);
    this->at(pos2) = dnode;
  }

  template<typename ValueType, typename Visitor>
  void visitTreeValues(Visitor visitor) const
  {
     typedef typename vector_type::const_iterator vector_iterator;
     vector_type &vector = getItems();
     for(vector_iterator it = vector.begin(), epos = vector.end(); it != epos; ++it)
       (*it).visitTreeValues<ValueType>(visitor);
  }

  template<typename ValueType, typename Visitor>
  void visitTreeNodes(Visitor visitor) const
  {
     typedef typename vector_type::const_iterator vector_iterator;
     vector_type &vector = getItems();
     for(vector_iterator it = vector.begin(), epos = vector.end(); it != epos; ++it)
       (*it).visitTreeNodes<ValueType>(visitor);
  }

  template<typename ValueType, typename Visitor>
  Visitor visitVectorValues(Visitor visitor) const
  {
     typedef typename vector_type::const_iterator vector_iterator;
     const vector_type &vector = getItems();

     for(vector_iterator it = vector.begin(), epos = vector.end(); it != epos; ++it)
       visitor((*it).template getAs<ValueType>());

     return (visitor);
  }

  template<typename ValueType, typename Visitor>
  void visitVectorNodes(Visitor visitor) const
  {
     typedef typename vector_type::const_iterator vector_iterator;
     vector_type &vector = getItems();
     for(vector_iterator it = vector.begin(), epos = vector.end(); it != epos; ++it)
       visitor(*it);
  }

protected:
  virtual void copyItemsFrom(const dnChildColnBase& src);
  virtual void insert(dnode *node);
  virtual void insert(size_type pos, dnode *node);
  virtual void insert(size_type pos, const dtpString &name, dnode *node);
  virtual void insert(const dtpString &name, dnode *node);
  virtual void setAt(int pos, dnode *node);
  virtual dnode *extractChild(int index);
  virtual dnode *cloneChild(int index) const;
protected:
  dnodeColn m_items;
};

class dnChildColnDblMap: public dnChildColnBase {
public:
  typedef dnChildColnIndexMap vector_type;
  dnChildColnDblMap();
  virtual ~dnChildColnDblMap();

  virtual size_type size() const;
  virtual void resize(size_type newSize);
  virtual bool empty() const;
  virtual void clearItems();

  virtual void erase(const dtpString &name);
  virtual void erase(int index);
  virtual void eraseFrom(int index);

  virtual dnode &at(int pos) { 
#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
    return *m_map2[pos]; 
#else
    return m_map2[pos]; 
#endif
  }

  virtual const dnode &at(int pos) const
  {
#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
    return *m_map2[pos]; 
#else
    return m_map2[pos]; 
#endif
  }

  virtual void getChild(int index, dnode &output);
  virtual size_type indexOfName(const dtpString &name) const;
  virtual size_type indexOfValue(const dnode &value) const;
  virtual dnode &getByName(const dtpString &name);
  virtual const dnode &getByNameR(const dtpString &name) const;
  virtual dnode *peekChild(const dtpString &name);
  virtual const dnode *peekChildR(const dtpString &name) const;
  virtual bool hasChild(const dtpString &name) const;
  virtual const dtpString getName(int index) const;
  virtual void setName(int index, const dtpString &name);
  void swap(dnChildColnDblMap &rhs);
  vector_type &getItems() { return m_map2; }
  const vector_type &getItems() const { return m_map2; }
  virtual bool supportsAccessByName() const { return true; }

  void swap(size_type pos1, size_type pos2)
  {
    if (pos1 == pos2)
      return;

    dnode dnode1 = this->at(pos1);
    dtpString name1 = getName(pos1);
    this->at(pos1) = this->at(pos2);
    setName(pos1, getName(pos2));
    this->at(pos2) = dnode1;
    setName(pos2, name1);
  }

  template<typename ValueType, typename Visitor>
  void visitTreeValues(Visitor visitor) const
  {
     typedef typename vector_type::const_iterator vector_iterator;
     const vector_type &vector = getItems();
     for(vector_iterator it = vector.begin(), epos = vector.end(); it != epos; ++it)
#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
       (*it)->visitTreeValues<ValueType>(visitor);
#else
       (*it).visitTreeValues<ValueType>(visitor);
#endif
  }

  template<typename ValueType, typename Visitor>
  void visitTreeNodes(Visitor visitor) const
  {
     typedef typename vector_type::const_iterator vector_iterator;
     const vector_type &vector = getItems();
     for(vector_iterator it = vector.begin(), epos = vector.end(); it != epos; ++it)
#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
       (*it)->visitTreeNodes<ValueType>(visitor);
#else
       (*it).visitTreeNodes<ValueType>(visitor);
#endif
  }

  template<typename ValueType, typename Visitor>
  Visitor visitVectorValues(Visitor visitor) const
  {
     typedef typename vector_type::const_iterator vector_iterator;
     const vector_type &vector = getItems();

     for(vector_iterator it = vector.begin(), epos = vector.end(); it != epos; ++it)
#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
       visitor((*it)->template getAs<ValueType>());
#else
       visitor((*it).template getAs<ValueType>());
#endif

     return (visitor);
  }

  template<typename ValueType, typename Visitor>
  void visitVectorNodes(Visitor visitor) const
  {
     typedef typename vector_type::const_iterator vector_iterator;
     const vector_type &vector = getItems();
     for(vector_iterator it = vector.begin(), epos = vector.end(); it != epos; ++it)
#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
       visitor(**it);
#else
       visitor(*it);
#endif
  }

protected:
  virtual void copyItemsFrom(const dnChildColnBase& src);
  virtual void insert(dnode *node);
  virtual void insert(size_type pos, dnode *node);
  virtual void insert(size_type pos, const dtpString &name, dnode *node);
  virtual void insert(const dtpString &name, dnode *node);
  virtual void setAt(int pos, dnode *node);
  virtual dnode *extractChild(int index);
  //inline void rebuildMap();
  virtual dnode *cloneChild(int index) const;
  void eraseItem(int index);
  dnChildColnDblMap *newEmpty();
private:
  dnChildColnNameMap m_map1; /// name -> node
  dnChildColnIndexMap m_map2; // node
  dnChildColnNameVector m_names; // name
};

// ----------------------------------------------------------------------------
// dnChildColnImplMeta
// ----------------------------------------------------------------------------
template <int ContainerType>
class dnChildColnImplMeta {
public:
  typedef dnChildColnBase implementation_type;
};

template <>
class dnChildColnImplMeta<ict_list> {
public:
  typedef dnChildColnList implementation_type;
};

template <>
class dnChildColnImplMeta<ict_parent> {
public:
  typedef dnChildColnDblMap implementation_type;
};

typedef dnChildColnBase dnChildColn;

class ParentVisitorCommon {
public:
  typedef uint size_type;

  template<typename ValueType, typename CompOp, typename DerivedClass>
  static
  size_type find_if_derived(const dnChildColnBaseIntf *parent, ValueType value, CompOp compOp)
  {
     typedef typename DerivedClass::vector_type vector_type;
     DerivedClass *derived = const_cast<DerivedClass *>(dynamic_cast<const DerivedClass *>(parent));
     vector_type &vector = derived->getItems();
     size_type res = vector.size();

     size_type epos = res;
#ifdef DTP_TPL_RES_SPEC_BY_TAG
     ValueType readValue;
#endif

     for(size_type i = 0; i != epos; i++)
     {
       if (compOp(vector[i].template getAs<ValueType>(), value))
       {
         res = i;
         break;
       }
     }

     return res;
  }

  template<typename ValueType, typename CompareOp, typename DerivedClass>
  static
  void sortValuesDirect(const dnChildColnBaseIntf *parent, CompareOp compOp)
  {
     typedef typename DerivedClass::vector_type vector_type;
     DerivedClass *derived = const_cast<DerivedClass *>(dynamic_cast<const DerivedClass *>(parent));
     vector_type &vector = derived->getItems();
     size_t n = vector.size();
     if (n > 1)
     {
        dtp::sort(n, SortToolForVector<ValueType, vector_type, CompareOp>(vector, compOp));
     }
  }

  template<typename ValueType, typename CompareOp, typename DerivedClass>
  static
  void sortValuesWithNodeRef(const dnChildColnBaseIntf *parent, CompareOp compOp)
  {
     size_t n = parent->size();
     if (n > 1)
     {
        SortToolForValueAs<ValueType, DerivedClass, CompareOp> tool(dynamic_cast<DerivedClass &>(const_cast<dnChildColnBaseIntf &>(*parent)), compOp);
        dtp::sort(n, tool);
     }
  }

  template<typename CompareOp, typename DerivedClass>
  static
  void sortNodesWithNodeRef(const dnChildColnBaseIntf *parent, CompareOp compOp)
  {
     size_t n = parent->size();
     if (n > 1)
     {
        SortToolForNodeRef<DerivedClass, CompareOp> tool(dynamic_cast<DerivedClass &>(*parent), compOp);
        dtp::sort(n, tool);
     }
  }

protected:
  template<typename ValueType, typename VectorType, typename CompareOp>
  class SortToolForVector {
    public:
     typedef ValueType value_type;

     SortToolForVector(VectorType &vect, CompareOp compOp): m_vect(vect), m_compareOp(compOp) {}

     ValueType get(int pos)
     {
       dnode &nodeRef = m_vect[pos];
       return nodeRef.getAs<ValueType>();
     }

     void swap(size_t pos1, size_t pos2)
     {
        if (pos1 == pos2)
          return;

        ValueType value1 = m_vect[pos1].template getAs<ValueType>();
        m_vect[pos1].setAs(m_vect[pos2].template getAs<ValueType>());
        m_vect[pos2].setAs(value1);
     }

     int compare(size_t pos, const ValueType &value)
     {
       return m_compareOp(m_vect[pos].template getAs<ValueType>(), value);
     }

    protected:
     void set(int pos, ValueType newValue)
     {
       dnode &nodeRef = m_vect[pos];
       nodeRef.setAs<ValueType>(newValue);
     }

    protected:
      VectorType &m_vect;
      CompareOp m_compareOp;
  };

template<typename ValueType, typename ChildColn, typename CompareOp>
  class SortToolForValueAs {
    public:
     typedef ValueType value_type;

     SortToolForValueAs(ChildColn &childColn, CompareOp compOp): m_childColn(childColn), m_compareOp(compOp) {}

     ValueType get(int pos)
     {
       dnode &nodeRef = m_childColn.at(pos);
       return nodeRef.getAs<ValueType>();
     }

     void swap(size_t pos1, size_t pos2)
     {
        m_childColn.swap(pos1, pos2);
     }

     int compare(size_t pos, const ValueType &value)
     {
       return m_compareOp(m_childColn.at(pos).template getAs<ValueType>(), value);
     }

    protected:
     void set(int pos, ValueType newValue)
     {
       dnode &nodeRef = m_childColn.at(pos);
       nodeRef.setAs<ValueType>(newValue);
     }

    protected:
      ChildColn &m_childColn;
      CompareOp m_compareOp;
  };

  template<typename ChildColn, typename CompareOp>
  class SortToolForNodeRef {
    public:
     SortToolForNodeRef(ChildColn &childColn, CompareOp compOp): m_childColn(childColn), m_compareOp(compOp) {}

     dnode &get(int pos)
     {
       return m_childColn.at(pos);
     }

     void swap(size_t pos1, size_t pos2)
     {
        m_childColn.swap(pos1, pos2);
     }

     int compare(size_t pos, const dnode &value)
     {
       return m_compareOp(m_childColn.at(pos), value);
     }

    protected:
     void set(int pos, const dnode &newValue)
     {
       dnode &nodeRef = m_childColn.at(pos);
       //nodeRef.setValue(newValue);
       nodeRef.setAs(newValue);
     }

    protected:
      ChildColn &m_childColn;
      CompareOp m_compareOp;
  };

}; // ParentVisitorCommon

template <>
class ParentVisitor<ict_parent> {
public:
    typedef dnChildColnDblMap implementation_type;
    typedef uint size_type;

    template<typename ValueType, typename Visitor>
    static
    void visitTreeValues(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
        static_cast<dnChildColnBase *>(const_cast<dnChildColnBaseIntf *>(parent))->visitTreeValues<ValueType, Visitor, implementation_type>(visitor);
    }

    template<typename ValueType, typename Visitor>
    static
    void visitTreeNodes(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
        static_cast<dnChildColnBase *>(const_cast<dnChildColnBaseIntf *>(parent))->visitTreeNodes<ValueType, Visitor, implementation_type>(visitor);
    }

    template<typename ValueType, typename Visitor>
    static
    Visitor visitVectorValues(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
        return static_cast<dnChildColnBase *>(const_cast<dnChildColnBaseIntf *>(parent))->visitVectorValues<ValueType, Visitor, implementation_type>(visitor);
    }

    template<typename ValueType, typename Visitor>
    static
    void visitVectorNodes(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
        static_cast<dnChildColnBase *>(const_cast<dnChildColnBaseIntf *>(parent))->visitVectorNodes<ValueType, Visitor, implementation_type>(visitor);
    }

    template<typename ValueType, typename CompareOp>
    static
    void sortValues(const dnChildColnBaseIntf *parent, CompareOp compOp)
    {
        //dnChildColnBase *children = static_cast<dnChildColnBase *>(const_cast<dnChildColnBaseIntf *>(parent));
        //return children->sortValues<ValueType, CompareOp>(compOp);
        ParentVisitorCommon::sortValuesWithNodeRef<ValueType, CompareOp, implementation_type>(parent, compOp);
    }

    template<typename CompareOp>
    static
    void sortNodes(const dnChildColnBaseIntf *parent, CompareOp compOp)
    {
        ParentVisitorCommon::sortNodesWithNodeRef<CompareOp, implementation_type>(parent, compOp);
    }

    template<typename ValueType, typename CompareOp>
    static
    size_type find_if(const dnChildColnBaseIntf *parent, ValueType value, CompareOp compOp)
    {
        dnChildColnBase *children = static_cast<dnChildColnBase *>(const_cast<dnChildColnBaseIntf *>(parent));
        return children->find_if(value, compOp);
    }

    template<typename ValueType, typename CompOp>
    static
    size_type find_if_derived(const dnChildColnBaseIntf *parent, ValueType value, CompOp compOp)
    {
      return ParentVisitorCommon::find_if_derived<ValueType, CompOp, dnChildColnImplMeta<ict_parent>::implementation_type>(parent, value, compOp);
    }

    template<typename ValueType>
    static
    size_type find_derived(dnChildColnBaseIntf *parent, ValueType value)
    {
       typedef implementation_type DerivedClass;
       typedef typename DerivedClass::vector_type vector_type;
       DerivedClass *derived = const_cast<DerivedClass *>(dynamic_cast<const DerivedClass *>(parent));
       vector_type &vector = derived->getItems();
       size_type res = vector.size();

       size_type epos = res;

       for(size_type i = 0; i != epos; i++)
       {
#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
         if (vector[i]->template getAs<ValueType>() == value)
#else
         if (vector[i].template getAs<ValueType>() == value)
#endif
         {
           res = i;
           break;
         }
       }

       return res;
    }

};

template <>
class ParentVisitor<ict_list> {
public:
    typedef dnChildColnList implementation_type;
    typedef uint size_type;

    template<typename ValueType, typename Visitor>
    static
    void visitTreeValues(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
        static_cast<dnChildColnBase *>(const_cast<dnChildColnBaseIntf *>(parent))->visitTreeValues<ValueType, Visitor, implementation_type>(visitor);
    }

    template<typename ValueType, typename Visitor>
    static
    void visitTreeNodes(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
        static_cast<dnChildColnBase *>(const_cast<dnChildColnBaseIntf *>(parent))->visitTreeNodes<ValueType, Visitor, implementation_type>(visitor);
    }

    template<typename ValueType, typename Visitor>
    static
    Visitor visitVectorValues(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
        return static_cast<dnChildColnBase *>(const_cast<dnChildColnBaseIntf *>(parent))->visitVectorValues<ValueType, Visitor, implementation_type>(visitor);
    }

    template<typename ValueType, typename Visitor>
    static
    void visitVectorNodes(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
        static_cast<dnChildColnBase *>(const_cast<dnChildColnBaseIntf *>(parent))->visitVectorNodes<ValueType, Visitor, implementation_type>(visitor);
    }

    template<typename ValueType, typename CompareOp>
    static
    void sortValues(const dnChildColnBaseIntf *parent, CompareOp compOp)
    {
        //dnChildColnBase *children = static_cast<dnChildColnBase *>(const_cast<dnChildColnBaseIntf *>(parent));
        //return children->sortValues<ValueType, CompareOp>(compOp);
        if (dnValueTypeMeta<ValueType>::item_type != vt_datanode)
          ParentVisitorCommon::sortValuesDirect<ValueType, CompareOp, implementation_type>(parent, compOp);
        else
          ParentVisitorCommon::sortValuesWithNodeRef<ValueType, CompareOp, implementation_type>(parent, compOp);
    }

    template<typename CompareOp>
    static
    void sortNodes(const dnChildColnBaseIntf *parent, CompareOp compOp)
    {
        ParentVisitorCommon::sortNodesWithNodeRef<CompareOp, implementation_type>(parent, compOp);
    }

    template<typename ValueType, typename CompOp>
    static
    size_type find_if_derived(const dnChildColnBaseIntf *parent, ValueType value, CompOp compOp)
    {
      return ParentVisitorCommon::find_if_derived<ValueType, CompOp, dnChildColnImplMeta<ict_list>::implementation_type>(parent, value, compOp);
    }

    template<typename ValueType>
    static
    size_type find_derived(dnChildColnBaseIntf *parent, ValueType value)
    {
       typedef implementation_type DerivedClass;
       typedef typename DerivedClass::vector_type vector_type;
       DerivedClass *derived = const_cast<DerivedClass *>(dynamic_cast<const DerivedClass *>(parent));
       vector_type &vector = derived->getItems();
       size_type res = vector.size();

       size_type epos = res;

       for(size_type i = 0; i != epos; i++)
       {
         if (vector[i].template getAs<ValueType>() == value)
         {
           res = i;
           break;
         }
       }

       return res;
    }
};

template <>
class ParentVisitorGeneric<parent_visitor_impl_tag> {
public:
    typedef uint size_type;

    template<typename ValueType, typename Visitor>
    static
    void visitTreeValues(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
        if (parent->isList())
          ParentVisitor<ict_list>::visitTreeValues<ValueType, Visitor>(parent, visitor);
        else
          ParentVisitor<ict_parent>::visitTreeValues<ValueType, Visitor>(parent, visitor);
    }

    template<typename IntCompareOp>
    static
    bool binarySearchNode(const dnChildColnBaseIntf *parent, const dnode &value, IntCompareOp compOp, int &foundPos)
    {
        dnChildColnBase *children = static_cast<dnChildColnBase *>(const_cast<dnChildColnBaseIntf *>(parent));
        return children->binarySearchNode(value, compOp, foundPos);
    }

    template<typename ValueType, typename IntCompareOp>
    static
    bool binarySearchValue(const dnChildColnBaseIntf *parent, const ValueType &value, IntCompareOp compOp, int &foundPos)
    {
        dnChildColnBase *parentBase = static_cast<dnChildColnBase *>(const_cast<dnChildColnBaseIntf *>(parent));
        return parentBase->binarySearchValue<ValueType, IntCompareOp>(value, compOp, foundPos);
    }

    template<typename ValueType, typename Visitor>
    static
    void visitTreeNodes(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
        if (parent->isList())
          ParentVisitor<ict_list>::visitTreeValues<ValueType, Visitor>(parent, visitor);
        else
          ParentVisitor<ict_parent>::visitTreeValues<ValueType, Visitor>(parent, visitor);
    }

    template<typename ValueType, typename Visitor>
    static
    Visitor visitVectorValues(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
        //static_cast<dnChildColnBase *>(parent)->visitVectorValues<ValueType, Visitor, implementation_type>(visitor);
        if (parent->isList())
          return ParentVisitor<ict_list>::visitVectorValues<ValueType, Visitor>(parent, visitor);
        else
          return ParentVisitor<ict_parent>::visitVectorValues<ValueType, Visitor>(parent, visitor);
    }

    template<typename ValueType, typename Visitor>
    static
    void visitVectorNodes(const dnChildColnBaseIntf *parent, Visitor visitor)
    {
        //static_cast<dnChildColnBase *>(parent)->visitVectorNodes<ValueType, Visitor, implementation_type>(visitor);
        if (parent->isList())
          ParentVisitor<ict_list>::visitVectorNodes<ValueType, Visitor>(parent, visitor);
        else
          ParentVisitor<ict_parent>::visitVectorNodes<ValueType, Visitor>(parent, visitor);
    }

    template<typename CompareOp>
    static
    void sortNodes(const dnChildColnBaseIntf *parent, CompareOp compOp)
    {
        //ParentVisitorCommon::sortNodesWithNodeRef<CompareOp, implementation_type>(parent, compOp);
        if (parent->isList())
          ParentVisitor<ict_list>::sortNodes<CompareOp>(parent, compOp);
        else
          ParentVisitor<ict_parent>::sortNodes<CompareOp>(parent, compOp);
    }

    template<typename ValueType, typename CompareOp>
    static
    void sortValues(const dnChildColnBaseIntf *parent, CompareOp compOp)
    {
        if (parent->isList())
          ParentVisitor<ict_list>::sortValues<ValueType, CompareOp>(parent, compOp);
        else
          ParentVisitor<ict_parent>::sortValues<ValueType, CompareOp>(parent, compOp);
    }

    template<typename ValueType>
    static
    size_type find(const dnChildColnBaseIntf *parent, ValueType value)
    {
        dnChildColnBase *children = static_cast<dnChildColnBase *>(const_cast<dnChildColnBaseIntf *>(parent));
        return children->find(value);
    }

    template<typename ValueType, typename CompOp>
    static
    size_type find_if_derived(const dnChildColnBaseIntf *parent, ValueType value, CompOp compOp)
    {
      if (parent->isList())
        return ParentVisitorCommon::find_if_derived<ValueType, CompOp, dnChildColnImplMeta<ict_list>::implementation_type>(parent, value, compOp);
      else
        return ParentVisitorCommon::find_if_derived<ValueType, CompOp, dnChildColnImplMeta<ict_parent>::implementation_type>(parent, value, compOp);
    }

    template<typename ValueType>
    static
    size_type find_derived(dnChildColnBaseIntf *parent, ValueType value)
    {
      if (parent->isList())
        //return ParentVisitorCommon::find_derived<ValueType, dnChildColnImplMeta<ict_list>::implementation_type>(parent, value);
        return ParentVisitor<ict_list>::find_derived<ValueType>(parent, value); 
      else
        //return ParentVisitorCommon::find_derived<ValueType, dnChildColnImplMeta<ict_parent>::implementation_type>(parent, value);
        return ParentVisitor<ict_parent>::find_derived<ValueType>(parent, value); 
    }
};

} // namespace Details


void swap(dnValue& lhs, dnValue& rhs);

namespace {
  template<class T>
  void indirect_swap(T& l, T& r) {
    using std::swap; // needed for
                     // fundamental types
    swap(l, r); // Now Koenig lookup can
                // find arg::swap
  }
}

// ----------------------------------------------------------------------------
// dnode - section end
// ----------------------------------------------------------------------------

} // namespace dtp

#include "dtp/details/dnode_arrays.h"
#endif // _DTPDATANODE3_H__
