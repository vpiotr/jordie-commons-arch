/////////////////////////////////////////////////////////////////////////////
// Name:        dnode2_arrays.h
// Project:     dtpLib
// Purpose:     Data node array declarations - private include module.
// Author:      Piotr Likus
// Modified by:
// Created:     01/05/2012
/////////////////////////////////////////////////////////////////////////////

#ifndef _DTPDNODE2ARR_H__
#define _DTPDNODE2ARR_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file dnode2_arrays.h
\brief Data node array declarations - private include module.

*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "base/object.h"
#include "dtp/dnode.h"
#include "dtp/details/sort.h"

namespace dtp {

namespace Details {

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

// ----------------------------------------------------------------------------
// meta info - first part
// ----------------------------------------------------------------------------
struct dn_array_value_type_null_tag {};

template<class ValueType>
struct dnArrayMetaIsDefined: public dtpSelector<true>
{
};

template<>
struct dnArrayMetaIsDefined<dn_array_value_type_null_tag>: public dtpSelector<false>
{
};

template<bool Condition, class T>
struct dnArrayImplMeta {
  typedef void vector_type;
};

template<class ValueType, bool Value = (dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType>::item_is_node == 1)>
struct dnArrayMetaItemIsNode: public dtpSelector<Value>
{
};

// ----------------------------------------------------------------------------
// dnArrayBase
// ----------------------------------------------------------------------------
class dnArrayBase: public Details::dnArray {
public:
  typedef uint size_type;
  typedef dnArray inherited;

  dnArrayBase(dnValueType a_type):dnArray(a_type) {}
  virtual ~dnArrayBase() {};
  virtual dnode::dnValueBridge *newValueBridge();

  template<typename T>
  void addItem(const T &input)
  {
    addItemImpl(input, dnArrayMetaItemIsNode<T>());
  }

  template<typename T>
  void addItemAtPos(size_type pos, const T &input)
  {
    typedef dnArrayImplMeta<dnArrayMetaIsDefined<T>::value, T>  array_impl_meta;
    typedef typename array_impl_meta::implementation_type impl_type;
    if ((array_impl_meta::item_is_node == 1) || (array_impl_meta::direct_item_type != getValueType())) {
      addItemAtPosAsNode(pos, new dnode(input));
    }
    else {
      static_cast<impl_type *>(this)->addItemAtPos(pos, input);
    }
  }

  template<typename T>
  void addItemDirectly(const T &input)
  {
    typedef dnArrayImplMeta<dnArrayMetaIsDefined<T>::value, T>  array_impl_meta;
    typedef typename array_impl_meta::implementation_type impl_type;
    if (array_impl_meta::direct_item_type == getValueType())
    {
      static_cast<impl_type *>(this)->addItem(input);
    } else {
      dnode value(input);
      inherited::addItem(input);
    }
  }

  void swapItems(dtp::dnode::size_type pos1, dtp::dnode::size_type pos2)
  {
    doSwapItems(pos1, pos2);
  }

  virtual void addItemAtPosAsNode(size_type pos, const dnode &input) = 0;
  virtual void addItemAtPosAsNode(size_type pos, dnode *input) = 0;

protected:
  template<typename T>
  void addItemImpl(const T &input, dtpSelector<true> performAsNode)
  {
    addItemAsNode(new dnode(input));
  }

  template<typename T>
  void addItemImpl(const T &input, dtpSelector<false> performAsNode)
  {
    typedef dnArrayImplMeta<dnArrayMetaIsDefined<T>::value, T>  array_impl_meta;
    typedef typename array_impl_meta::implementation_type impl_type;

    if (static_cast<dnValueType>(array_impl_meta::direct_item_type) != getValueType()) {
      addItemAsNode(new dnode(input));
    }
    else {
      static_cast<impl_type *>(this)->addItem(input);
    }
  }

  template<typename T>
  void addItemUsingNode(const T &input)
  {
    addItemAsNode(new dnode(input));
  }

  virtual void doSwapItems(dtp::dnode::size_type pos1, dtp::dnode::size_type pos2)
  {
    dnode temp1, temp2;
    getItem(pos1, temp1);
    getItem(pos2, temp2);
    setItem(pos1, temp2);
    setItem(pos2, temp1);
  }

};


template<typename ValueType, class Derived>
class dnArrayOf: public dnArrayBase {
public:
  dnArrayOf(dnValueType a_type = vt_datanode): dnArrayBase(a_type) {}
  virtual ~dnArrayOf() {}
protected:
};

template<typename ValueType>
class dnArrayOfPod: public dnArrayOf<ValueType, dnArrayOfPod<ValueType> > {
public:
  typedef std::vector<ValueType> vector_type;
  typedef dnArrayOf<ValueType, dnArrayOfPod<ValueType> > inherited;
  typedef typename vector_type::iterator self_iterator;
  typedef ValueType value_type;
  typedef dnArrayOfPod<ValueType> self_type;
  typedef uint size_type;

  dnArrayOfPod(dnValueType a_type = vt_datanode): inherited(a_type) {}
  virtual ~dnArrayOfPod() {}

  vector_type &getItems() { return m_items; }
  const vector_type &getItems() const { return m_items; }

  void copyFrom(const dnArray *a_source)
  {
    if (a_source->getValueType() != this->getValueType())
      throw dnError("Incorrect array source value for assign");
    const vector_type &srcItems = checked_cast<const self_type *>(a_source)->getItems();
    //m_items.assign(srcItems.begin(), srcItems.end());
    m_items = srcItems;
  }

  bool empty() const
  {
    return m_items.empty();
  }

  virtual dnArray *clone() const {
    std::auto_ptr<dnArray> res(new self_type(this->getValueType()));
    res->copyFrom(this);
    return res.release();
  }

  virtual dnArray *cloneEmpty() const {
    return new self_type(this->getValueType());
  }

  void getItem(int index, dnode &output) const
  {
    output.setAs<ValueType>(m_items[index]);
  }

  void setItem(int index, const dnode &input)
  {
    m_items[index] = input.getAs<ValueType>();
  }

  void addItem(const dnValue &input)
  {
    m_items.push_back(input.getAs<ValueType>());
  }

  void addItem(base::move_ptr<dnode> input) 
  {
    m_items.push_back(input->getAs<ValueType>());
  }

  void addItemAsNode(const dnode &input)
  {
    m_items.push_back(input.getAs<ValueType>());
  }

  void addItemAsNode(dnode *input)
  {
    DTP_UNIQUE_PTR(dnode) inputGuard(input);
    m_items.push_back(inputGuard->getAs<ValueType>());
  }

  void eraseItem(int index)
  {
    self_iterator itRemove = m_items.begin() + index;
    m_items.erase(itRemove);
  }

  void eraseFrom(int index)
  {
    self_iterator itRemove = m_items.begin() + index;
    m_items.erase(itRemove, m_items.end());
  }

  dnArray::size_type indexOfValue(const dnode &input) const
  {
    size_type pos = std::find(m_items.begin(), m_items.end(), input.getAs<ValueType>()) - m_items.begin();
    if (pos >= m_items.size())
      pos = Const::npos;
    return pos;
  }

  void clear()
  {
    m_items.clear();
  }

  dnArray::size_type size() const
  {
    return m_items.size();
  }

  void resize(size_type newSize)
  {
    m_items.resize(newSize);
  }

  bool sort()
  {
    std::sort(m_items.begin(), m_items.end());
    return true;
  }

  virtual void addItemAtFront(const dnode &input) { m_items.insert(m_items.begin(), input.getAs<value_type>()); }
  virtual void addItemAtPos(size_type pos, const dnode &input) { m_items.insert(m_items.begin() + pos, input.getAs<value_type>()); }

  virtual void addItemAtPosAsNode(size_type pos, const dnode &input) { m_items.insert(m_items.begin() + pos, input.getAs<value_type>()); }

  virtual void addItemAtPosAsNode(size_type pos, dnode *input) {
    DTP_UNIQUE_PTR(dnode) inputGuard(input);
    m_items.insert(m_items.begin() + pos, input->getAs<value_type>());
  }

  template<typename T>
  void addItem(const T &input)
  {
    addItemDetails(input, dtp::dtpIsSameType<T, ValueType>());
  }

  template<typename T>
  void addItemDetails(const T &input, dtpSelector<false>)
  {
    addItemUsingNode(input);
  }

  template<typename T>
  void addItemDetails(const T &input, dtpSelector<true>)
  {
    m_items.push_back(input);
  }

  template<typename T>
  void addItemAtPos(size_type pos, const T &input)
  {
    addItemAtPosDetails(pos, input, dtp::dtpIsSameType<T, ValueType>());
  }

  template<typename T>
  void addItemAtPosDetails(size_type pos, const T &input, dtpSelector<false>)
  {
    addItemAtPosAsNode(pos, input);
  }

  template<typename T>
  void addItemAtPosDetails(size_type pos, const T &input, dtpSelector<true>)
  {
    m_items.insert(m_items.begin() + pos, input);
  }

  template<typename T>
  T getFromNode(dtp::dnode::size_type pos) const
  {
    dnode value(m_items[pos]);
    return value.getAs<T>();
  }

  virtual void swap(size_type pos1, size_type pos2) {
    if (pos1 == pos2)
      return;

    ValueType value = m_items[pos1];
    m_items[pos1] = m_items[pos2];
    m_items[pos2] = value;
  }

protected:
  void doSwapItems(dtp::dnode::size_type pos1, dtp::dnode::size_type pos2)
  {
    value_type temp;
    temp = m_items[pos1];
    m_items[pos1] = m_items[pos2];
    m_items[pos2] = temp;
  }
protected:
  vector_type m_items;
};

class dnArrayOfDataNode2: public dnArrayOf<dtp::dnode, dnArrayOfDataNode2> {
public:
  typedef dnodeColn vector_type;
  typedef dnArrayOf<dtp::dnode, dnArrayOfDataNode2> inherited;
  typedef vector_type::iterator self_iterator;
  typedef vector_type::const_iterator self_const_iterator;
  typedef dtp::dnode value_type;
  typedef dnArrayOfDataNode2 self_type;

  dnArrayOfDataNode2(dnValueType a_type = vt_null): inherited(a_type) {}
  virtual ~dnArrayOfDataNode2() {}

  dnodeColn &getItems() { return m_items; }
  const dnodeColn &getItems() const { return m_items; }

  virtual dnArray *clone() const {
    std::auto_ptr<dnArray> res(new self_type(getValueType()));
    res->copyFrom(this);
    return res.release();
  }

  virtual dnArray *cloneEmpty() const {
    return new self_type(getValueType());
  }

  virtual void copyFrom(const dnArray *a_source);
  virtual bool empty() const;

  virtual void getItem(int index, dnode &output) const;
  virtual void setItem(int index, const dnode &input);
  virtual void setItemValue(int index, const dnode &input);
  virtual const dnode &getNode(int index, dnode &helper) const;
  virtual dnode *getNodePtr(int index, dnode &helper) const;
  virtual void setNode(int index, const dnode &value);

  virtual void addItem(base::move_ptr<dnode> input);
  virtual void addItem(const dnValue &input);
  virtual void addItemAsNode(const dnode &input);
  virtual void addItemAsNode(dnode *input);
  virtual void addItemAtFront(const dnode &input) { m_items.insert(m_items.begin(), new dnode(input)); }
  virtual void addItemAtPos(size_type pos, const dnode &input) { m_items.insert(m_items.begin() + pos, new dnode(input)); }
  virtual void addItemAtPosAsNode(size_type pos, const dnode &input) { m_items.insert(m_items.begin() + pos, new dnode(input)); }
  virtual void addItemAtPosAsNode(size_type pos, dnode *input) { m_items.insert(m_items.begin() + pos, input); }

  template<typename ValueType>
  ValueType get(size_type index) const
  {
     return m_items[index].getAs<ValueType>();
  }

  template<typename T>
  void addItem(const T &input)
  {
    addItemUsingNode(input);
  }

  virtual void eatItem(dnode &input);
  virtual void eraseItem(int index);
  virtual void eraseFrom(int index);
  virtual size_type indexOfValue(const dnode &input) const;
  virtual size_type findByName(const dtpString &name) const;
  virtual void clear();
  virtual size_type size() const;
  virtual void resize(size_type newSize);
  virtual dnode::dnValueBridge *newValueBridge();

  template<typename T>
  T getFromNode(dtp::dnode::size_type pos) const
  {
    return m_items[pos].getAs<T>();
  }

  virtual void swap(size_type pos1, size_type pos2) {
    if (pos1 == pos2)
      return;

    dtp::dnode value = m_items[pos1];
    m_items[pos1] = m_items[pos2];
    m_items[pos2] = value;
  }

protected:
  void doSwapItems(dtp::dnode::size_type pos1, dtp::dnode::size_type pos2)
  {
    value_type temp;
    temp = m_items[pos1];
    m_items[pos1] = m_items[pos2];
    m_items[pos2] = temp;
  }
protected:
  vector_type m_items;
};

// ----------------------------------------------------------------------------
// dnArrayImplMeta
// ----------------------------------------------------------------------------

// default version, storing in vector data nodes - for strings & data nodes
template <bool Condition, typename ValueType>
class dnArrayImplClassMetaInt {
public:
  typedef dtp::dnode value_type;
  typedef dnArrayOfDataNode2 implementation_type;
  enum Options {
      item_is_node = 1,
      item_type = vt_datanode,
      direct_item_type = vt_undefined
  };
};

// optimal version - storing in vector values directly (like vector<int>)
template <typename ValueType>
class dnArrayImplClassMetaInt<false, ValueType> {
public:
  typedef ValueType value_type;
  typedef dnArrayOfPod<ValueType> implementation_type;
  typedef typename Details::dnValueTypeMeta<ValueType> value_meta;
  enum Options {
      item_is_node = 0,
      item_type = value_meta::item_type,
      direct_item_type = value_meta::item_type
  };
};

template<class Condition, class T>
struct dnArrayImplClassMetaInt2: public dnArrayImplClassMetaInt<Condition::value, T> {
   typedef dnArrayImplClassMetaInt<Condition::value, T> inherited;
};

//template<class T>
//struct dnArrayImplMeta: public dnArrayImplClassMetaInt2<dtp::Details::dnValueMetaIsObject<T>, T> {
//  typedef implementation_type::vector_type vector_type;
//};

template<class T>
struct dnArrayImplMeta<true, T>: public dnArrayImplClassMetaInt2<dtp::Details::dnValueMetaIsObject<T>, T> {
  typedef dnArrayImplClassMetaInt2<dtp::Details::dnValueMetaIsObject<T>, T> inherited;
  typedef typename inherited::implementation_type::vector_type vector_type;
};

// ----------------------------------------------------------------------------
// dnValueMetaCanAddItemDirectly
// ----------------------------------------------------------------------------
template<typename ValueType, bool Value = (dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType>::item_is_node != 1)>
struct dnValueMetaCanAddItemDirectly: public dtpSelector<Value>
{
};

template<typename ValueType,
  bool Value = (
    dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType>::direct_item_type ==
    dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType>::item_type)>
struct dnArrayImplCanReadAsVector: public dtpSelector<Value>
{
};

// ----------------------------------------------------------------------------
// dnArrayByIntMeta
// ----------------------------------------------------------------------------
template <int ValueType>
class dnArrayByIntMeta {
public:
  typedef dnode value_type;
  typedef dnArrayOfDataNode2 implementation_type;
  typedef implementation_type::vector_type vector_type;
  enum Options { item_type = vt_datanode };
};

template <>
class dnArrayByIntMeta<vt_float> {
public:
  typedef float value_type;
  typedef dnArrayOfPod<value_type> implementation_type;
  typedef implementation_type::vector_type vector_type;
  enum Options { item_type = vt_float };
};

template <>
class dnArrayByIntMeta<vt_double> {
public:
  typedef double value_type;
  typedef dnArrayOfPod<value_type> implementation_type;
  typedef implementation_type::vector_type vector_type;
  enum Options { item_type = vt_double };
};

template <>
class dnArrayByIntMeta<vt_xdouble> {
public:
  typedef xdouble value_type;
  typedef dnArrayOfPod<value_type> implementation_type;
  typedef implementation_type::vector_type vector_type;
  enum Options { item_type = vt_xdouble };
};

template <>
class dnArrayByIntMeta<vt_byte> {
public:
  typedef byte value_type;
  typedef dnArrayOfPod<value_type> implementation_type;
  typedef implementation_type::vector_type vector_type;
  enum Options { item_type = vt_byte };
};

template <>
class dnArrayByIntMeta<vt_int> {
public:
  typedef int value_type;
  typedef dnArrayOfPod<value_type> implementation_type;
  typedef implementation_type::vector_type vector_type;
  enum Options { item_type = vt_int };
};

template <>
class dnArrayByIntMeta<vt_uint> {
public:
  typedef uint value_type;
  typedef dnArrayOfPod<value_type> implementation_type;
  typedef implementation_type::vector_type vector_type;
  enum Options { item_type = vt_uint };
};

// ----------------------------------------------------------------------------
// dnArrayFactory
// ----------------------------------------------------------------------------
class dnArrayFactory {
public:
  virtual ~dnArrayFactory() {}
  virtual dnArray *newArray() = 0;
};

// ----------------------------------------------------------------------------
// dnArrayFactoryByType
// ----------------------------------------------------------------------------
template<int ValueType>
class dnArrayFactoryByType: public dnArrayFactory {
public:
  virtual dnArray *newArray() {
    typedef typename dnArrayByIntMeta<ValueType>::implementation_type impl_type;
    return new impl_type(static_cast<dnValueType>(dnArrayByIntMeta<ValueType>::item_type));
  }
};

// ----------------------------------------------------------------------------
// dnArrayTypeRegister
// ----------------------------------------------------------------------------
/// Register for array types & factories for them
class dnArrayTypeRegister {
public:
  typedef boost::shared_ptr<dnArrayFactory> dnArrayFactoryGuard;
  typedef std::map<int, dnArrayFactoryGuard> dnArrayTypeMap;

  static void registerType(dnValueType aType, dnArrayFactory *factory)
  {
    dnArrayTypeRegister::getMap().insert(std::make_pair(aType, dnArrayFactoryGuard(factory)));
  }

  static dnArray *newArray(dnValueType aType) {
    dnArrayTypeMap::iterator it = dnArrayTypeRegister::getMap().find(aType);
    if (it != dnArrayTypeRegister::getMap().end())
      return it->second->newArray();
    else {
      throw std::runtime_error(dtpString("Unknown array type: ")+toString(aType));
      return DTP_NULL;
    }
  }
protected:
  static dnArrayTypeMap &getMap() {
    static dnArrayTypeMap typeMap;
    return typeMap;
  }
private:
};

// ----------------------------------------------------------------------------
// dnArrayTypeBinder
// ----------------------------------------------------------------------------
/// binds integer array type to factory
template<int ValueType>
class dnArrayTypeBinder {
public:
  dnArrayTypeBinder() {
    dnArrayTypeRegister::registerType(static_cast<dnValueType>(ValueType), new dnArrayFactoryByType<ValueType>());
  }
};

template <>
class dnArrayVisitor<dnArrayVisitTagImpl>
{
public:
  typedef dnValue::size_type size_type;

  template<typename ValueType, class ArrayType>
  static
    typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
      addItem(ArrayType *aArray, ValueType value)
  {
    typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
    typedef typename array_impl_meta::implementation_type impl_type;
    if ((array_impl_meta::item_is_node == 1) || (array_impl_meta::direct_item_type != aArray->getValueType())) {
      aArray->addItemAsNode(new dnode(value));
    }
    else
      static_cast<impl_type *>(aArray)->addItem(value);
  }

  template<typename ValueType, class ArrayType>
  static
    typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
      addItem(ArrayType *aArray, const ValueType &value)
  {
    typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
    typedef typename array_impl_meta::implementation_type impl_type;
    if ((array_impl_meta::item_is_node == 1) || (array_impl_meta::direct_item_type != aArray->getValueType())) {
      static_cast<dnArrayBase *>(aArray)->addItemAsNode(new dnode(value));
    }
    else
      static_cast<impl_type *>(aArray)->addItem(value);
  }

  template<typename ValueType, class ArrayType>
  static
    typename dtpDisableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
      addItemAtPos(ArrayType *aArray, size_type pos, ValueType value)
  {
    typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
    typedef typename array_impl_meta::implementation_type impl_type;
    if ((array_impl_meta::item_is_node == 1) || (array_impl_meta::direct_item_type != aArray->getValueType())) {
      static_cast<dnArrayBase *>(aArray)->addItemAtPosAsNode(pos, new dnode(value));
    }
    else
      static_cast<impl_type *>(aArray)->addItemAtPos(pos, value);
  }

  template<typename ValueType, class ArrayType>
  static
    typename dtpEnableIf<Details::dnValueMetaIsObject<ValueType>, void>::type
      addItemAtPos(ArrayType *aArray, size_type pos, const ValueType &value)
  {
    typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
    typedef typename array_impl_meta::implementation_type impl_type;
    if ((array_impl_meta::item_is_node == 1) || (array_impl_meta::direct_item_type != aArray->getValueType())) {
      static_cast<dnArrayBase *>(aArray)->addItemAtPosAsNode(pos, new dnode(value));
    }
    else
      static_cast<impl_type *>(aArray)->addItemAtPos(pos, value);
  }

  template<typename ValueType, class ArrayType>
  static
  ValueType get(ArrayType *aArray, size_type index)
  {
     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     typedef typename array_impl_meta::implementation_type impl_type;
     using namespace Details;

     if (array_impl_meta::direct_item_type == aArray->getValueType()) {
       return
         getImpl<ValueType>(aArray, index, dtpSelector<array_impl_meta::direct_item_type == dnArrayByIntMeta<array_impl_meta::direct_item_type>::item_type>());
     } else {
       if (array_impl_meta::item_type == vt_datanode)
         return
           getImplFromNode<ValueType>(aArray, index);
       else
         return
           getImpl<ValueType>(aArray, index, dtpSelector<false>());
    }
  }

  template<typename ValueType, class ArrayType>
  static
  void get(ArrayType *aArray, size_type index, ValueType &output)
  {
     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     typedef typename array_impl_meta::implementation_type impl_type;
     using namespace Details;

     if (static_cast<dnValueType>(array_impl_meta::direct_item_type) == aArray->getValueType()) {
       output =
         getImpl<ValueType>(aArray, index, dtpSelector<static_cast<dnValueType>(array_impl_meta::direct_item_type) == static_cast<dnValueType>(dnArrayByIntMeta<array_impl_meta::direct_item_type>::item_type)>());
     } else {
       if (static_cast<dnValueType>(array_impl_meta::item_type) == vt_datanode)
         output =
           getImplFromNode<ValueType>(aArray, index);
       else
         output =
           getImpl<ValueType>(aArray, index, dtpSelector<false>());
    }
  }

  template<typename ValueType, class ArrayType>
  static
  void set(ArrayType *aArray, size_type index, const ValueType &value)
  {
     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     using namespace Details;

     if (array_impl_meta::direct_item_type == aArray->getValueType()) {
       typedef typename array_impl_meta::implementation_type ImplArray;
       typedef typename array_impl_meta::vector_type VectorType;
       const_cast<ImplArray *>(static_cast<const ImplArray *>(aArray))->getItems()[index] = value;
     } else {
        dnode node;
        node.setAs<ValueType>(value);
        aArray->setItem(index, node);
     }
  }

  template<typename ValueType, class ArrayType>
  static
  void get(ArrayType *aArray, size_type index, ValueType &output, dnode &helper)
  {
     typedef typename dnArrayVisitMeta<ValueType>::visitor_tag visitor_tag;
     typedef dnArrayVisitor<visitor_tag> ArrayVisitor;
     output = aArray->getNode(index, helper).template getAs<ValueType>();
  }

  template<typename ValueType, class ArrayType>
  static
  void set(ArrayType *aArray, size_type index, const ValueType &value, dnode &helper)
  {
     helper.setAs<ValueType>(value);
     aArray->setNode(index, helper);
  }

  template<typename ValueType, typename Visitor, class ArrayType>
  static
  void visitTreeValues(ArrayType *aArray, Visitor visitor)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     typedef typename array_impl_meta::implementation_type ImplArray;
     typedef typename array_impl_meta::vector_type VectorType;
     ImplArray *implArray = const_cast<ImplArray *>(checked_cast<const ImplArray *>(aArray));
     VectorType &vect = implArray->getItems();
     if (array_impl_meta::item_is_node != 0)
     {
       for(size_type i=0, epos = aArray->size(); i != epos; i++)
         vect[i].visitTreeValues<ValueType, Visitor>(visitor);
     } else {
       for(size_type i=0, epos = aArray->size(); i != epos; i++)
         if (visitMethod(vect[i], visitor))
           break;
     }
  }

  template<typename ValueType, typename Visitor, class ArrayType>
  static
  void visitTreeNodes(ArrayType *aArray, Visitor visitor)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     typedef typename array_impl_meta::implementation_type ImplArray;
     typedef typename array_impl_meta::vector_type VectorType;

     ImplArray *implArray = const_cast<ImplArray *>(checked_cast<const ImplArray *>(aArray));
     VectorType &vect = implArray->getItems();
     if (array_impl_meta::item_is_node != 0)
     {
       for(size_type i=0, epos = aArray->size(); i != epos; i++)
         vect[i].visitTreeNodes(visitor);
     } else {
       dnode helper;
       for(size_type i=0, epos = aArray->size(); i != epos; i++)
         visitor(aArray->getNode(i, helper));
     }
  }

  template<typename ValueType, typename Visitor, class ArrayType>
  static
  Visitor visitVectorValues(ArrayType *aArray, Visitor visitor)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     typedef typename array_impl_meta::implementation_type ImplArray;
     typedef typename array_impl_meta::vector_type VectorType;

     ImplArray *implArray = const_cast<ImplArray *>(checked_cast<const ImplArray *>(aArray));
     VectorType &vect = implArray->getItems();
     return (std::for_each(vect.begin(), vect.end(), visitor));
  }

  /// version with tag value
  template<typename ValueType, typename Visitor, class ArrayType>
  static
  Visitor visitVectorValues(ArrayType *aArray, Visitor visitor, const ValueType &tag)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     typedef typename array_impl_meta::implementation_type ImplArray;
     typedef typename array_impl_meta::vector_type VectorType;

     ImplArray *implArray = const_cast<ImplArray *>(checked_cast<const ImplArray *>(aArray));
     VectorType &vect = implArray->getItems();
     return (std::for_each(vect.begin(), vect.end(), visitor));
  }

  template <typename ValueType, typename CompOp, class ArrayType>
  static
  bool binarySearchValue(ArrayType *aArray, const ValueType &value, CompOp compOp, int &foundPos)
  {
     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     bool directMode = (array_impl_meta::item_type == aArray->getValueType()) && (aArray->getValueType() != vt_datanode);
     if (directMode)
       return binarySearchValueDirect<ValueType, CompOp>(aArray, value, compOp, foundPos);
     else
       return binarySearchValueByItem<ValueType, CompOp>(aArray, value, compOp, foundPos);
  }

  template<typename ValueType, typename IntCompareOp, class ArrayType>
  static
  bool binarySearchValueDirect(ArrayType *aArray, const ValueType &value, IntCompareOp compOp, int &foundPos)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     typedef typename array_impl_meta::implementation_type ImplArray;
     typedef typename array_impl_meta::vector_type VectorType;

     ImplArray *implArray = const_cast<ImplArray *>(checked_cast<const ImplArray *>(aArray));
     VectorType &vect = implArray->getItems();
     return binary_search_by_pos(0, vect.size(), value, VectorItemCompPosDirect<ValueType, VectorType, IntCompareOp>(vect, compOp), foundPos);
  }

  template<typename ValueType, typename IntCompareOp, class ArrayType>
  static
  bool binarySearchValueByItem(ArrayType *aArray, const ValueType &value, IntCompareOp compOp, int &foundPos)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     typedef typename array_impl_meta::implementation_type ImplArray;

     ImplArray *implArray = const_cast<ImplArray *>(checked_cast<const ImplArray *>(aArray));
     return binary_search_by_pos(0, implArray->size(), value, VectorItemCompPosByItem<ValueType, ImplArray, IntCompareOp>(*implArray, compOp), foundPos);
  }

  template<typename IntCompareOp, class ArrayType>
  static
  bool binarySearchNode(ArrayType *aArray, const dnode &value, IntCompareOp compOp, int &foundPos)
  {
     using namespace Details;
     dnArray *implArray = const_cast<dnArray *>(checked_cast<const dnArray *>(aArray));
     return binary_search_by_pos(0, implArray->size(), value, VectorItemCompPosByNode<dnArray, IntCompareOp>(*implArray, compOp), foundPos);
  }

  template<typename ValueType, typename IntCompareOp, class ArrayType>
  static
  void sortValues(ArrayType *aArray, IntCompareOp compOp)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;

     bool directMode = (array_impl_meta::direct_item_type == aArray->getValueType());
     if (directMode)
        sortValuesDirect<ValueType, IntCompareOp>(aArray, compOp);
     else
        sortValuesByItem<ValueType, IntCompareOp>(aArray, compOp);
  }

  template<typename ValueType, typename IntCompareOp, class ArrayType>
  static
  void sortNodesByRef(ArrayType *aArray, IntCompareOp compOp)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     typedef typename array_impl_meta::implementation_type ImplArray;

     ImplArray *implArray = checked_cast<ImplArray *>(aArray);
     size_t n = implArray->size();
     if (n > 1)
     {
        dtp::sort(n, SortToolInArrayByNodeRef<ImplArray, IntCompareOp, dnode>(*implArray, compOp));
     }
  }

  template<typename ValueType, typename CompOp, typename ArrayType>
  static
  size_type find_if(ArrayType *aArray, ValueType value, CompOp compOp)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     bool directMode = (array_impl_meta::direct_item_type == aArray->getValueType());
      if (directMode)
        return findValueDirectByType<ValueType, CompOp>(aArray, value, compOp, dnArrayImplCanReadAsVector<ValueType>());
      else
        return findValueByItem<ValueType>(aArray, value, compOp);
  }

  template<typename ValueType, class ArrayType>
  static
  size_type find(ArrayType *aArray, ValueType value)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     bool directMode = (array_impl_meta::direct_item_type == aArray->getValueType());
      if (directMode)
        return findValueDirectNoOp<ValueType>(aArray, value);
      else
        return findValueByItemNoOp<ValueType>(aArray, value);
  }

  template<typename T>
  static
  dnArray *newArray()
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<T>::value, T> array_impl_meta;
     typedef typename array_impl_meta::implementation_type impl_type;
//     typedef typename array_impl_meta::implementation_type impl_type;
     //const uint item_type = array_impl_meta::item_type;

     return
         new impl_type(
           static_cast<dtp::dnValueType>(
             array_impl_meta::item_type
           ));
  }

  template<typename T>
  static
  dnArray *newArray(T tag)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<T>::value, T> array_impl_meta;
     typedef typename array_impl_meta::implementation_type impl_type;

     return
         new impl_type(
           static_cast<dtp::dnValueType>(
             array_impl_meta::item_type
           ));
  }

protected:
  template<typename ValueType, typename IntCompareOp, class ArrayType>
  static
  void sortValuesDirect(ArrayType *aArray, IntCompareOp compOp)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     typedef typename array_impl_meta::implementation_type ImplArray;
     typedef typename array_impl_meta::vector_type VectorType;

     ImplArray *implArray = checked_cast<ImplArray *>(aArray);
     VectorType &vect = implArray->getItems();
     size_t n = vect.size();
     if (n > 1)
     {
        dtp::sort(n, SortToolInVector<ValueType, VectorType, IntCompareOp>(vect, compOp));
     }
  }

  template<typename ValueType, typename IntCompareOp, class ArrayType>
  static
  void sortValuesByItem(ArrayType *aArray, IntCompareOp compOp)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     typedef typename array_impl_meta::implementation_type ImplArray;

     ImplArray *implArray = checked_cast<ImplArray *>(aArray);
     size_t n = implArray->size();
     if (n > 1)
     {
        dtp::sort(n, SortToolInArrayByItem<ValueType, ImplArray, IntCompareOp>(*implArray, compOp));
     }
  }

  template<typename ValueType, typename CompOp, class ArrayType>
  static
  size_type findValueDirectByType(ArrayType *aArray, ValueType value, CompOp compOp, dtpSelector<false>)
  {
     return findValueByItem(aArray, value, compOp);
  }

  template<typename ValueType, typename CompOp, class ArrayType>
  static
  size_type findValueDirectByType(ArrayType *aArray, ValueType value, CompOp compOp, dtpSelector<true>)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     typedef typename array_impl_meta::implementation_type ImplArray;
     typedef typename array_impl_meta::vector_type VectorType;

     ImplArray *implArray = checked_cast<ImplArray *>(aArray);
     assert(implArray != DTP_NULL);
     VectorType &vect = implArray->getItems();

     return find_if_reader(vect.size(), value,
       ValueReaderVector<ValueType, VectorType>(vect), compOp);
  }

  template<typename ValueType, typename CompOp, class ArrayType>
  static
  size_type findValueByItem(ArrayType *aArray, ValueType value, CompOp compOp)
  {
     using namespace Details;

     if (aArray->getValueType() == vt_datanode)
     {
       typedef dnArrayByIntMeta<vt_datanode>::implementation_type ImplArray;

       ImplArray *implArray = checked_cast<ImplArray *>(aArray);
       return find_if_reader(aArray->size(), value,
         ValueReaderNodeItem<ValueType, ImplArray>(*implArray), compOp);
     } else {
       typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
       typedef typename array_impl_meta::implementation_type ImplArray;

       ImplArray *implArray = checked_cast<ImplArray *>(aArray);
       assert(implArray != DTP_NULL);

       return find_if_reader(aArray->size(), value,
           ValueReaderNodeGet<ValueType, ImplArray>(*implArray), compOp);
     }
  }

  template<typename ValueType, class ArrayType>
  static
  size_type findValueDirectNoOp(ArrayType *aArray, ValueType value)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     typedef typename array_impl_meta::implementation_type ImplArray;
     typedef typename array_impl_meta::vector_type VectorType;

     ImplArray *implArray = checked_cast<ImplArray *>(aArray);
     assert(implArray != DTP_NULL);
     VectorType &vect = implArray->getItems();
     typename VectorType::iterator epos = vect.end();
     typename VectorType::iterator it = std::find(vect.begin(), epos, value);
     if (it == epos)
       return vect.size();
     else
       return it - vect.begin();
  }

  template<typename ValueType, class ArrayType>
  static
  size_type findValueByItemNoOp(ArrayType *aArray, ValueType value)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     typedef typename array_impl_meta::implementation_type ImplArray;

     ImplArray *implArray = checked_cast<ImplArray *>(aArray);
     assert(implArray != DTP_NULL);

     return find_if_reader(aArray->size(), value, ValueReaderNodeGet<ValueType, ImplArray>(*implArray), std::equal_to<ValueType>());
  }

  template<typename ValueType, class ArrayType>
  static
  ValueType getImpl(ArrayType *aArray, size_type index, dtpSelector<false> canReadDirectly)
  {
      dnode helper;
      //return get<ValueType>(aArray, index, node);
      return aArray->getNode(index, helper).template getAs<ValueType>();
  }

  template<typename ValueType, class ArrayType>
  static
  ValueType getImpl(ArrayType *aArray, size_type index, dtpSelector<true> canReadDirectly)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     typedef typename array_impl_meta::implementation_type ImplArray;
     typedef typename array_impl_meta::vector_type VectorType;

     return
      const_cast<ImplArray *>(static_cast<const ImplArray *>(aArray))->getItems()[index];
  }

  template<typename ValueType, class ArrayType>
  static
  ValueType getImplFromNode(ArrayType *aArray, size_type index)
  {
     using namespace Details;

     typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType> array_impl_meta;
     typedef typename array_impl_meta::implementation_type ImplArray;

     return
       const_cast<ImplArray *>(static_cast<const ImplArray *>(aArray))->template getFromNode<ValueType>(index);
  }

  // accessor using array[] access
  template<typename ValueType, typename VectorType, typename CompareOp>
  class SortToolInVector {
  public:
    typedef ValueType value_type;

    SortToolInVector(VectorType &vect, CompareOp compOp): m_vect(vect), m_compareOp(compOp) {}

    ValueType get(size_t pos) { return m_vect[pos]; }

    void swap(size_t pos1, size_t pos2)
    {
       if (pos1 == pos2)
         return;

       ValueType value = m_vect[pos1];
       m_vect[pos1] = m_vect[pos2];
       m_vect[pos2] = value;
    }

    int compare(size_t pos, const ValueType &value)
    {
      return m_compareOp(m_vect[pos], value);
    }

  protected:
    void set(size_t pos, ValueType value) { m_vect[pos] = value; }
  protected:
    VectorType &m_vect;
    CompareOp m_compareOp;
  }; // SortToolInVector

  // accessor using getItem, setItem, getNodePtr
  template<typename ValueType, typename ArrayType, typename CompareOp>
  class SortToolInArrayByItem {
  public:
    typedef ValueType value_type;

    SortToolInArrayByItem(ArrayType &arr, CompareOp compOp): m_array(arr), m_compareOp(compOp) {}

    ValueType get(size_t pos) {
      dnode helper;
      dnode *nodePtr = m_array.getNodePtr(pos, helper);
      return nodePtr->getAs<ValueType>();
    }

    void swap(size_t pos1, size_t pos2)
    {
       if (pos1 == pos2)
         return;

       dnode dnode1, dnode2;
       m_array.getItem(pos1, dnode1);
       m_array.getItem(pos2, dnode2);
       m_array.setItem(pos2, dnode1);
       m_array.setItem(pos1, dnode2);
    }

    int compare(size_t pos, const ValueType &value)
    {
      return m_compareOp(get(pos), value);
    }

  protected:
    void set(size_t pos, ValueType value) {
      dnode dnode(value);
      m_array.setItem(pos, dnode);
    }

  protected:
    ArrayType &m_array;
    CompareOp m_compareOp;
  }; // SortToolInArrayByItem

  // reader using internal container & array[] access
  template<typename ValueType, typename ContainerType>
  struct ValueReaderVector {
    ValueReaderVector(const ContainerType &vect): m_vect(vect) {}
    ValueType operator()(size_t idx)
    {
      return (m_vect[idx]);
    }
  protected:
    const ContainerType &m_vect;
  }; // ValueReaderVector

  // reader using internal container & getNode() access
  template<typename ValueType, typename ContainerType>
  struct ValueReaderNodeGet {
    ValueReaderNodeGet(const ContainerType &container): m_container(container) {}
    ValueType operator()(size_t idx)
    {
      dnode helper;
      return m_container.getNode(idx, helper).template getAs<ValueType>();
    }
  protected:
    const ContainerType &m_container;
  }; // ValueReaderNodeGet

  template<typename ValueType, typename ContainerType>
  struct ValueReaderNodeItem {
    ValueReaderNodeItem(const ContainerType &container): m_container(container) {}
    ValueType operator()(size_t idx)
    {
      return m_container.get<ValueType>(idx);
    }
  protected:
    const ContainerType &m_container;
  }; // ValueReaderNodeItem

  // comparator using internal container & array[] access
  template<typename ValueType, typename VectorType, typename IntCompareOp>
  class VectorItemCompPosDirect {
    public:
     VectorItemCompPosDirect(const VectorType &vector, IntCompareOp compOp): m_vector(vector), m_compOp(compOp) {}
     // compares item at a given pos with provided value
     int operator()(int pos, const ValueType &value)
     {
       ValueType valueAtPos = m_vector[pos];
       return m_compOp(valueAtPos, value);
     }
    protected:
     const VectorType &m_vector;
     IntCompareOp m_compOp;
  }; // VectorItemCompPosDirect

  // comparator using internal container & getNodePtr, getAs<> access
  template<typename ValueType, typename ImplArray, typename IntCompareOp>
  class VectorItemCompPosByItem {
    public:
     VectorItemCompPosByItem(const ImplArray &arr, IntCompareOp compOp): m_array(arr), m_compOp(compOp) {}
     // compares item at a given pos with provided value
     int operator()(int pos, const ValueType &value)
     {
       dnode helper;
       const dnode *nodePtr = m_array.getNodePtr(pos, helper);
       assert(nodePtr != DTP_NULL);
       ValueType valueAtPos = nodePtr->getAs<ValueType>();
       return m_compOp(valueAtPos, value);
     }
    protected:
     const ImplArray &m_array;
     IntCompareOp m_compOp;
  }; // VectorItemCompPosByItem

  // comparator using internal container & getNodePtr access
  template<typename ImplArray, typename IntCompareOp>
  class VectorItemCompPosByNode {
    public:
     VectorItemCompPosByNode(const ImplArray &arr, IntCompareOp compOp): m_array(arr), m_compOp(compOp) {}
     // compares item at a given pos with provided value
     int operator()(int pos, const dnode &value)
     {
       dnode helper;
       const dnode *nodePtr = m_array.getNodePtr(pos, helper);
       assert(nodePtr != DTP_NULL);
       return m_compOp(*nodePtr, value);
     }
    protected:
     const ImplArray &m_array;
     IntCompareOp m_compOp;
  }; // VectorItemCompPosByNode

  // accessor using getItem, setItem, getNodePtr
  template<typename ArrayType, typename CompareOp, typename NodeType>
  class SortToolInArrayByNodeRef {
  public:
    SortToolInArrayByNodeRef(ArrayType &arr, CompareOp compOp): m_array(arr), m_compareOp(compOp) {}

    NodeType get(size_t pos) {
      NodeType dnode1;
      m_array.getItem(pos, dnode1);
      return dnode1;
    }

    void swap(size_t pos1, size_t pos2)
    {
       if (pos1 == pos2)
         return;

       dnode dnode1, dnode2;
       m_array.getItem(pos1, dnode1);
       m_array.getItem(pos2, dnode2);
       m_array.setItem(pos2, dnode1);
       m_array.setItem(pos1, dnode2);
    }

    int compare(size_t pos, const NodeType &value)
    {
      dnode dnode1;
      m_array.getItem(pos, dnode1);
      return m_compareOp(dnode1, value);
    }

  protected:
    ArrayType &m_array;
    CompareOp m_compareOp;
  }; // SortToolInArrayByNodeRef


}; // dnArrayVisitor


} // namespace Details

} // namespace dtp
#endif // _DTPDNODE2ARR_H__
