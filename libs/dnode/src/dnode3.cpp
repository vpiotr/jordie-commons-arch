/////////////////////////////////////////////////////////////////////////////
// Name:        dnode2.cpp
// Project:     dtpLib
// Purpose:     Universal XML-DOM-node-like structure - data node.
//              API version 2.0
// Author:      Piotr Likus
// Modified by:
// Created:     28/04/2012
/////////////////////////////////////////////////////////////////////////////

#include "base/btypes.h"
#include "base/date.h"

#include "dtp/details/dnode3.h"
#include "dtp/details/dnode_arrays.h"
#include "dtp/details/utils.h"
#include "dtp/details/defs.h"

using namespace dtp;
using namespace Details;

// ----------------------------------------------------------------------------
// static members
// ----------------------------------------------------------------------------

// Required to define array types in central registry
#define DN_ARRAY_TYPE_BINDER(a) const dnArrayTypeBinder<vt_##a> array_binder_##a
DN_ARRAY_TYPE_BINDER(float);
DN_ARRAY_TYPE_BINDER(double);
DN_ARRAY_TYPE_BINDER(xdouble);
DN_ARRAY_TYPE_BINDER(byte);
DN_ARRAY_TYPE_BINDER(int);
DN_ARRAY_TYPE_BINDER(uint);
DN_ARRAY_TYPE_BINDER(int64);
DN_ARRAY_TYPE_BINDER(uint64);
DN_ARRAY_TYPE_BINDER(vptr);
DN_ARRAY_TYPE_BINDER(datanode);
DN_ARRAY_TYPE_BINDER(string);
DN_ARRAY_TYPE_BINDER(date);
DN_ARRAY_TYPE_BINDER(time);
DN_ARRAY_TYPE_BINDER(datetime);

// Required to define dynamic value conversion
#define DN_VALUE_CAST_BINDER(a) const Details::dnValueDynamicCasterBinder<vt_##a> value_caster_binder_##a
DN_VALUE_CAST_BINDER(bool);
DN_VALUE_CAST_BINDER(byte);
DN_VALUE_CAST_BINDER(int);
DN_VALUE_CAST_BINDER(uint);
DN_VALUE_CAST_BINDER(int64);
DN_VALUE_CAST_BINDER(uint64);
DN_VALUE_CAST_BINDER(float);
DN_VALUE_CAST_BINDER(double);
DN_VALUE_CAST_BINDER(xdouble);
DN_VALUE_CAST_BINDER(string);
DN_VALUE_CAST_BINDER(vptr);
DN_VALUE_CAST_BINDER(date);
DN_VALUE_CAST_BINDER(time);
DN_VALUE_CAST_BINDER(datetime);

// Required to define dynamic value conversion
#define DN_COMP_STRATEGY_BINDER(a) const Details::dnValueCompareStrategyBinder<vt_##a> value_strategy_binder_##a
DN_COMP_STRATEGY_BINDER(null);
DN_COMP_STRATEGY_BINDER(array);
DN_COMP_STRATEGY_BINDER(parent);
DN_COMP_STRATEGY_BINDER(bool);
DN_COMP_STRATEGY_BINDER(byte);
DN_COMP_STRATEGY_BINDER(int);
DN_COMP_STRATEGY_BINDER(uint);
DN_COMP_STRATEGY_BINDER(int64);
DN_COMP_STRATEGY_BINDER(uint64);
DN_COMP_STRATEGY_BINDER(float);
DN_COMP_STRATEGY_BINDER(double);
DN_COMP_STRATEGY_BINDER(xdouble);
DN_COMP_STRATEGY_BINDER(string);
DN_COMP_STRATEGY_BINDER(vptr);
DN_COMP_STRATEGY_BINDER(date);
DN_COMP_STRATEGY_BINDER(time);
DN_COMP_STRATEGY_BINDER(datetime);

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// dnValueBridge
// ----------------------------------------------------------------------------

const dnode::dnValueBridge::size_type dnode::dnValueBridge::npos = static_cast<dnode::dnValueBridge::size_type>(-1);

namespace dtp {

namespace Details {

// ----------------------------------------------------------------------------
// dnValueBridgeIndexed
// ----------------------------------------------------------------------------
class dnValueBridgeIndexed: public dnode::dnValueBridge {
public:
    dnValueBridgeIndexed(): dnode::dnValueBridge()
      ,m_index(npos)
    {}
    dnValueBridgeIndexed( const dnValueBridgeIndexed& src): dnode::dnValueBridge()
      , m_index(src.m_index)
    {}

    void copyFrom(const dnValueBridgeIndexed &rhs) {
      m_index = rhs.m_index;
    }

    // modify position
    virtual size_type getPos() const {
      return m_index;
    }
    virtual void setPos(size_type idx) {
      m_index = idx;
    }
    virtual void setPosBegin() {
      setPos(0);
    }
    virtual void setPosEnd() {
      setPos(this->size());
    }
    virtual void incPos(int value = 1) {
      setPos(m_index + value);
    }
    virtual void decPos(int value = 1) {
      setPos(m_index - value);
    }

    // compare pos
    virtual bool isEqualPos(const dnValueBridge& value) const {
        return (this->getPos() == value.getPos());
    }
    virtual int calcPosDiff(const dnValueBridge& value) const {
        return (this->getPos() - value.getPos());
    }
private:
    size_type m_index;
};

/// throw - only in debug mode
#ifdef DTP_DEBUG
void debugThrow(const dtpString &msg)
{
  throw dnError(msg);
}
#else
#define debugThrow(a) 
#endif

#ifdef DTP_DEBUG
#define fastCheckContainer(a) if (!isContainer()) throwNotContainer()
#define fastCheckParent(a) if (!isParent()) throwNotParent()
#define fastThrowNotParent throwNotParent
#else
#define fastCheckContainer(a) 
#define fastCheckParent(a) 
#define fastThrowNotParent assert(false); int __fastThrowNotParent
#endif


// ----------------------------------------------------------------------------
// dnValueBridgeForArray
// ----------------------------------------------------------------------------
class dnValueBridgeForArray: public dnValueBridgeIndexed {
public:
    typedef dnValueBridgeIndexed inherited;
    typedef dnValueBridgeForArray this_type;
    dnValueBridgeForArray(): dnValueBridgeIndexed(), m_items(DTP_NULL)
    {}
    dnValueBridgeForArray(dnArray *items): dnValueBridgeIndexed(), m_items(items)
    {}
    dnValueBridgeForArray( const dnValueBridgeForArray& src): dnValueBridgeIndexed(src), m_items(src.m_items)
    {}

    virtual void release_to_pool() {
#ifdef DATANODE_POOL_BRIDGE
      ObjectPool<this_type>::deleteObject(this);
#endif
    }

    void setTarget(dnArray *items) { m_items = items; }

    void copyFrom(const dnValueBridgeForArray &rhs) {
      inherited::copyFrom(rhs);
      this->m_items = rhs.m_items;
    }

    virtual dnValueBridge *clone() const {

        this_type *bridge;
#ifndef DATANODE_POOL_BRIDGE
        bridge = new this_type(*this);
#else
        bridge = ObjectPool<this_type>::newObject();
        try {
          bridge->copyFrom(*this);
        } catch (...) {
          operator delete(bridge);  // Deallocate the memory
          throw;               // Re-throw the exception
        }
#endif
        return bridge;
    }

    virtual bool empty() const { return m_items->empty(); }
    virtual size_type size() const { return m_items->size(); }

   virtual const dtpString getName() const {
        return "";
    }
    virtual void setName(const dtpString &value) {
        assert(false); // not implemented for this structure type
    }

    virtual void setAsItem(const dnode& value) {
        m_items->setItem(getPos(), value);
    }

    virtual void getAsItem(dnode& output) const {
        m_items->getItem(getPos(), output);
    }

    virtual void setAsItem(const dnValue& value) {
        dnode temp(value);
        m_items->setItem(getPos(), temp);
    }

    virtual void getAsItem(dnValue& output) const {
        dnode temp;
        m_items->getItem(getPos(), temp);
        output = temp;
    }

    //virtual double getAsDouble() const { return m_items->getDouble(getPos()); }
    //virtual void setAsDouble(double value) { m_items->setDouble(getPos(), value); }

    const dnode &getAsNode(dnode &helper) const
    {
        m_items->getItem(getPos(), helper);
        return helper;
    }

protected:
    dnArray *getItems() const { return m_items; }
private:
    dnArray *m_items;
};

/*
class dnValueBridgeForArrayOfDouble: public dnValueBridgeForArray {
public:
    typedef dnValueBridgeForArray inherited;
    typedef dnValueBridgeForArrayOfDouble this_type;
    dnValueBridgeForArrayOfDouble(): dnValueBridgeForArray(), m_values(DTP_NULL)
    {}
    dnValueBridgeForArrayOfDouble( const dnValueBridgeForArrayOfDouble& src):
        dnValueBridgeForArray(src),
        m_values(const_cast<dnValueBridgeForArrayOfDouble &>(src).m_values)
    {}
    dnValueBridgeForArrayOfDouble(dnArray *items, dtp::dtpVectorOfDouble *values):
      dnValueBridgeForArray(items), m_values(values)
    {}

    virtual void release_to_pool() {
#ifdef DATANODE_POOL_BRIDGE
      ObjectPool<this_type>::deleteObject(this);
#endif
    }

    void copyFrom(const dnValueBridgeForArrayOfDouble &rhs) {
      inherited::copyFrom(rhs);
      this->m_values = rhs.m_values;
    }

    void setTarget(dnArray *items, dtp::dtpVectorOfDouble *values) {
      inherited::setTarget(items);
      m_values = values;
    }

    virtual dnValueBridge *clone() const {
      //return new this_type(*this);
        this_type *bridge;
#ifndef DATANODE_POOL_BRIDGE
        bridge = new this_type(*this);
#else
        bridge = ObjectPool<this_type>::newObject();
        try {
          bridge->copyFrom(*this);
        } catch (...) {
          operator delete(bridge);  // Deallocate the memory
          throw;               // Re-throw the exception
        }
#endif
        return bridge;
    }

    virtual double getAsDouble() const {
        return (*m_values)[getPos()]; }
    virtual void setAsDouble(double value) {
        (*m_values)[getPos()] = value;
    }
private:
    dtp::dtpVectorOfDouble *m_values;
};
*/

template <typename T>
class dnValueBridgeForArrayOfX: public dnValueBridgeForArray {
public:
    typedef dnValueBridgeForArray inherited;
    typedef T value_type;
    typedef dnValueBridgeForArrayOfX<T> this_type;
    typedef dnArrayImplMeta<dnArrayMetaIsDefined<T>::value, T> array_impl_meta;
    typedef typename array_impl_meta::implementation_type array_type;
    typedef typename array_impl_meta::vector_type vector_type;

    dnValueBridgeForArrayOfX(): dnValueBridgeForArray(), m_values(DTP_NULL)
    {}

    dnValueBridgeForArrayOfX( const this_type& src):
        dnValueBridgeForArray(src),
        m_values(const_cast<this_type &>(src).m_values)
    {}

    dnValueBridgeForArrayOfX(dnArray *items, vector_type *values):
      dnValueBridgeForArray(items), m_values(values)
    {}

    virtual void release_to_pool() {
#ifdef DATANODE_POOL_BRIDGE
      ObjectPool<this_type>::deleteObject(this);
#endif
    }

    void copyFrom(const this_type &rhs) {
      inherited::copyFrom(rhs);
      this->m_values = rhs.m_values;
    }

    void setTarget(dnArray *items, vector_type *values) {
      inherited::setTarget(items);
      m_values = values;
    }

    virtual dnValueBridge *clone() const {
        this_type *bridge;
#ifndef DATANODE_POOL_BRIDGE
        bridge = new this_type(*this);
#else
        bridge = ObjectPool<this_type>::newObject();
        try {
          bridge->copyFrom(*this);
        } catch (...) {
          operator delete(bridge);  // Deallocate the memory
          throw;               // Re-throw the exception
        }
#endif
        return bridge;
    }

    template<typename ValueType>
    ValueType getAs() const
    {
       return getAsDetails<ValueType>(dtpIsSameType<ValueType, value_type>());
    }

    template<typename ValueType>
    ValueType getAsDetails(dtpSelector<false> useDirect) const
    {
      dnode node;
      this->getAsItem(node);
      return node.getAs<ValueType>();
    }

    template<typename ValueType>
    ValueType getAsDetails(dtpSelector<true> useDirect) const
    {
      return (*m_values)[getPos()];
    }

    template<typename ValueType>
    void setAs(const ValueType &value)
    {
        setAsDetails(value, dtpIsSameType<ValueType, value_type>());
    }

    template<typename ValueType>
    void setAsDetails(const ValueType &value, dtpSelector<false> useDirect)
    {
      dnode node;
      node.setAs<ValueType>(value);
      this->setAsItem(node);
    }


    template<typename ValueType>
    void setAsDetails(const ValueType &newValue, dtpSelector<true> useDirect)
    {
       (*m_values)[getPos()] = newValue;
    }

private:
    vector_type *m_values;
};

class dnValueBridgeForArrayOfDataNode: public dnValueBridgeForArray {
public:
    typedef dnValueBridgeForArray inherited;
    typedef dnValueBridgeForArrayOfDataNode this_type;

    dnValueBridgeForArrayOfDataNode(): dnValueBridgeForArray()
    {}
    dnValueBridgeForArrayOfDataNode( const dnValueBridgeForArrayOfDataNode& src):
        dnValueBridgeForArray(src)
    {}
    dnValueBridgeForArrayOfDataNode(dnArray *items):
      dnValueBridgeForArray(items)
    {}

    virtual void release_to_pool() {
#ifdef DATANODE_POOL_BRIDGE
      ObjectPool<this_type>::deleteObject(this);
#endif
    }

    virtual dnValueBridge *clone() const {
      //return new this_type(*this);
        this_type *bridge;
#ifndef DATANODE_POOL_BRIDGE
        bridge = new this_type(*this);
#else
        bridge = ObjectPool<this_type>::newObject();
        try {
          bridge->copyFrom(*this);
        } catch (...) {
          operator delete(bridge);  // Deallocate the memory
          throw;               // Re-throw the exception
        }
#endif
        return bridge;
    }

    const dnode &getAsNode(dnode &helper) const
    {
        return getItems()->getNode(getPos(), helper);
    }
};

// ----------------------------------------------------------------------------
// dnValueBridgeForArrayImplMeta
// ----------------------------------------------------------------------------
template <typename ValueType>
class dnValueBridgeForArrayImplMeta {
public:
  typedef dnValueBridgeForArrayOfX<ValueType> implementation_type;
};

template <>
class dnValueBridgeForArrayImplMeta<dnode> {
public:
  typedef dnValueBridgeForArrayOfDataNode implementation_type;
};

template<typename ValueType, typename ArrayType>
static dnode::dnValueBridge *newValueBridgeForArrayAsVector(ArrayType &arr)
{
    typedef typename dnValueBridgeForArrayImplMeta<ValueType>::implementation_type bridge_type;

        bridge_type *bridge;
#ifndef DATANODE_POOL_BRIDGE
        bridge = new bridge_type(this);
#else
        bridge = ObjectPool<bridge_type>::newObject();
        try {
          bridge->setTarget(&arr, &(arr.getItems()));
        } catch (...) {
          operator delete(bridge);  // Deallocate the memory
          throw;               // Re-throw the exception
        }
#endif
        return bridge;
 }

// ----------------------------------------------------------------------------
// dnValueBridgeForChildColn
// ----------------------------------------------------------------------------
class dnValueBridgeForChildColn: public dnValueBridgeIndexed {
public:
    typedef dnValueBridgeIndexed inherited;
    typedef dnValueBridgeForChildColn this_type;

    dnValueBridgeForChildColn(): dnValueBridgeIndexed(), m_items(DTP_NULL)
    {}
    dnValueBridgeForChildColn(dnChildColnBase *items): dnValueBridgeIndexed(), m_items(items)
    {}
    dnValueBridgeForChildColn( const dnValueBridgeForChildColn& src): dnValueBridgeIndexed(src)
        , m_items(src.m_items)
    {}

    virtual void release_to_pool() {
#ifdef DATANODE_POOL_BRIDGE
      ObjectPool<this_type>::deleteObject(this);
#endif
    }

    void setTarget(dnChildColnBase *items) { m_items = items; }

    void copyFrom(const dnValueBridgeForChildColn &rhs) {
      inherited::copyFrom(rhs);
      m_items = rhs.m_items;
    }

    virtual dnValueBridge *clone() const {
        this_type *bridge;
#ifndef DATANODE_POOL_BRIDGE
        bridge = new this_type(*this);
#else
        bridge = ObjectPool<this_type>::newObject();
        try {
          bridge->copyFrom(*this);
        } catch (...) {
          operator delete(bridge);  // Deallocate the memory
          throw;               // Re-throw the exception
        }
#endif
        return bridge;
    }
    virtual bool empty() const { return m_items->empty(); }
    virtual size_type size() const { return m_items->size(); }

    virtual const dtpString getName() const {
        return m_items->getName(getPos());
    }
    virtual void setName(const dtpString &value) {
        m_items->setName(getPos(), value);
    }

    virtual void setAsItem(const dnode& value) {
        m_items->at(getPos()) = value;
    }

    virtual void getAsItem(dnode& output) const {
        output = m_items->at(getPos());
    }

    virtual void setAsItem(const dnValue& value) {
        dnode nvalue(value);
        m_items->at(getPos()).copyFrom(nvalue);
    }

    virtual void getAsItem(dnValue& output) const {
        output = m_items->at(getPos());
    }

    bool supportsRefs() const
    {
        return true;
    }

    bool supportsNames() const
    {
       return m_items->supportsAccessByName();
    }

    dnode &getAsNodeRef()
    {
        return m_items->at(getPos());
    }

    const dnode &getAsNode(dnode &helper) const
    {
        return m_items->at(getPos());
    }
private:
    dnChildColnBase *m_items;
};

// ----------------------------------------------------------------------------
// dnValueBridgeForChildColnNames
// ----------------------------------------------------------------------------
class dnValueBridgeForChildColnNames: public dnValueBridgeIndexed {
public:
    typedef dnValueBridgeIndexed inherited;
    typedef dnValueBridgeForChildColnNames this_type;

    dnValueBridgeForChildColnNames(): dnValueBridgeIndexed(), m_items(DTP_NULL)
    {}
    dnValueBridgeForChildColnNames(dnChildColnBase *items): dnValueBridgeIndexed(), m_items(items)
    {}
    dnValueBridgeForChildColnNames( const dnValueBridgeForChildColnNames& src): dnValueBridgeIndexed(src)
        , m_items(src.m_items)
    {}

    virtual void release_to_pool() {
#ifdef DATANODE_POOL_BRIDGE
      ObjectPool<this_type>::deleteObject(this);
#endif
    }

    void setTarget(dnChildColnBase *items) { m_items = items; }

    void copyFrom(const dnValueBridgeForChildColnNames &rhs) {
      inherited::copyFrom(rhs);
      m_items = rhs.m_items;
    }

    virtual dnValueBridge *clone() const {
        this_type *bridge;
#ifndef DATANODE_POOL_BRIDGE
        bridge = new this_type(*this);
#else
        bridge = ObjectPool<this_type>::newObject();
        try {
          bridge->copyFrom(*this);
        } catch (...) {
          operator delete(bridge);  // Deallocate the memory
          throw;               // Re-throw the exception
        }
#endif
        return bridge;
    }
    virtual bool empty() const { return m_items->empty(); }
    virtual size_type size() const { return m_items->size(); }

    virtual size_type getPos() const {
      if (inherited::getPos() == npos) {
        size_type idx = m_items->indexOfName(m_namePos);
        if (idx != dnode::npos)
        {
          const_cast<this_type *>(this)->inherited::setPos(idx);
        } else {
          const_cast<this_type *>(this)->setPosEnd();
        }
      }
      return inherited::getPos();
    }

    virtual void setPos(size_type idx) {
      inherited::setPos(idx);
      if (idx < size())
        m_namePos = m_items->getName(idx);
      else
        m_namePos.clear();
    }

    virtual const dtpString getName() const {
        return m_items->getName(getPos());
    }
    virtual void setName(const dtpString &value) {
        m_items->setName(getPos(), value);
    }

    virtual const dtpString &getNamePos() const {
        return m_namePos;
    }

    virtual void setNamePos(const dtpString &value) {
        inherited::setPos(npos);
        m_namePos = value;
    }

    virtual void setAsItem(const dnode& value) {
        m_items->getByName(m_namePos) = value;
    }

    virtual void getAsItem(dnode& output) const {
        output = m_items->getByName(m_namePos);
    }

    virtual void setAsItem(const dnValue& value) {
        dnode nvalue(value);
        m_items->getByName(m_namePos).copyFrom(nvalue);
    }

    virtual void getAsItem(dnValue& output) const {
        output = m_items->getByName(m_namePos);
    }

    bool supportsRefs() const
    {
        return true;
    }

    bool supportsNames() const
    {
       return true;
    }

    dnode &getAsNodeRef()
    {
        return m_items->getByName(m_namePos);
    }

    const dnode &getAsNode(dnode &helper) const
    {
        return m_items->getByName(m_namePos);
    }
private:
    dnChildColnBase *m_items;
    dtpString m_namePos;
};

} // namespace Details

} // namespace dtp


// ----------------------------------------------------------------------------
// dnode
// ----------------------------------------------------------------------------
const dnode::size_type dnode::npos = static_cast<dnode::size_type>(-1);

/*
dnode::dnode()
{
}
*/

dnode::dnode( const dnode& src)
{
  initFrom(src);
}

// construction time - initialization
/*
void dnode::initProps()
{
}
*/

// reset all properties
void dnode::resetProps()
{
  inherited::releaseData();
}

dnode &dnode::swap(dnode &rhs)
{
  inherited::swap(rhs);

  return *this;
}

void dnode::swap(size_type pos1, size_type pos2)
{
  if (isParent())
    getChildren().swap(pos1, pos2);
  else if (isArray())
    getArray()->swap(pos1, pos2);
}

dnode& dnode::operator=( const dnode& rhs)
{
  if (this != &rhs)
  {
    copyFrom(rhs);
  }
  return *this;
}

/*
dnode& dnode::operator=( const dtpString& rhs)
{
  resetProps();
  inherited::setAsString(rhs);
  return *this;
}
*/

dnode& dnode::operator=(const dnode_vector& rhs)
{
  resetProps();
  copyValueFrom(rhs);
  return *this;
}

dnode& dnode::operator=(const dnValue &rhs)
{
  resetProps();
  inherited::operator=(rhs);
  return *this;
}

/*
dnode &dnode::operator=(float value)
{
  clearValue();
  inherited::setAsFloat(value);
  return *this;
}

dnode &dnode::operator=(double value)
{
  clearValue();
  inherited::setAsDouble(value);
  return *this;
}
*/

/*
dnode &dnode::operator=(xdouble value)
{
  clearValue();
  inherited::setAsXDouble(value);
  return *this;
}
*/

/*
dnode &dnode::operator=(void_ptr value)
{
  clearValue();
  inherited::setAsVoidPtr(value);
  return *this;
}

dnode &dnode::operator=(bool value)
{
  clearValue();
  inherited::setAsBool(value);
  return *this;
}

dnode &dnode::operator=(int value)
{
  clearValue();
  inherited::setAsInt(value);
  return *this;
}

dnode &dnode::operator=(byte value)
{
  clearValue();
  inherited::setAsByte(value);
  return *this;
}

dnode &dnode::operator=(uint value)
{
  clearValue();
  inherited::setAsUInt(value);
  return *this;
}

dnode &dnode::operator=(int64 value)
{
  clearValue();
  inherited::setAsInt64(value);
  return *this;
}

dnode &dnode::operator=(uint64 value)
{
  clearValue();
  inherited::setAsUInt64(value);
  return *this;
}
*/

bool dnode::operator==(const dnode &rhs) const
{
  if (this == &rhs)
    return true;

  if (static_cast<const dnValue &>(*this) == static_cast<const dnode &>(rhs))
    return true;

  return false;
}

/// compares 2 nodes, when data type is different, compare string representations
bool dnode::isEqualTo(const dnode &value) const
{
  if (this == &value)
    return true;

  dtpStringGuard strValue;

  return isEqualTo(value, &strValue);
}

void dnode::initFrom( const dnode& src)
{
  initValueFrom(src);
}

void dnode::initValueFrom( const dnode& src)
{
  if (src.isParent())
  {
    setupChildren(!src.isList()).copyItemsFrom(
      src.getChildrenR()
    );
  }
  else if (src.isArray())
  {
    //initArray(src.getArrayR()->getValueType());
    //getArray()->copyFrom(src.getArrayR());
    setAsArray(src.getArrayR()->clone());
  } else {
    this->initScalarFrom(src);
  }
}

void dnode::copyFrom( const dnode& src)
{
  copyValueFrom(src);
}

void dnode::copyValueFrom( const dnode& src)
{
  clearValue();
  initValueFrom(src);
}

void dnode::copyValueFrom(const dnode_vector& src)
{
  clearValue();
  DTP_UNIQUE_PTR(dnode) childGuard;

  for(dnode_vector::const_iterator it = src.begin(), epos = src.end(); it != epos; ++it) {
      childGuard.reset(new dnode);
      childGuard->copyFrom(*it);
      addChild(childGuard.release());
  }
}

void dnode::copyTo(dnode_vector& output)
{
  output.clear();
  output.reserve(this->size());
  dnode element;
  for(dnode::const_iterator it = begin(), epos = end(); it != epos; ++it) {
      output.push_back(it->getAsNode(element));
  }
}

void dnode::copyStructureFrom(const dnode& src)
{
  if (src.isParent())
  {
    setupChildren(!src.isList()).copyItemsFrom(
      src.getChildrenR()
    );
  }
  else {
    disposeChildren();
  }
  disposeArray();
  if (src.isArray())
  {
    setAsArray(src.getArrayR()->clone());
  }
}

// Copy value but keep data type as it was
void dnode::assignValueFrom(const dnode& src)
{
  copyStructureFrom(src);
  inherited::assignFrom(src);
}

void dnode::moveFrom(dnode& src)
{
  clearValue();

  if (src.isParent())
  {
    setAsParent(src.extractChildren());
  }
  else if (src.isArray())
  {
    setAsArray(src.extractArray());
  }
  else {
    inherited::initScalarFrom(src);
  }
}

dnChildColnBase *dnode::createChildrenColn(bool aNamed)
{
  if (aNamed)
    return new dnChildColnDblMap;
  else
    return new dnChildColnList;
}

dnChildColnBase &dnode::setupChildren(bool aNamed)
{
  if (isParent())
  {
    return *getAsChildrenNoCheck();
  } else {
    dnChildColnBase *ptr = getChildrenPtr();
    if (ptr == DTP_NULL) {
      setAsParent(createChildrenColn(aNamed));
      ptr = getChildrenPtr();
    }
    assert(ptr != DTP_NULL);
    return *ptr;
  }
}

void dnode::disposeChildren()
{
  inherited::releaseData();
}

void dnode::transferChildrenFrom(dnode &input)
{
  if (!input.isParent())
    throwNotParent();

  setupChildren(!input.isList());

  dnode buffer;
  buffer.setAsParent();
  // transfer two times - to have same order without performance penalty

  while(!input.empty()) {
    buffer.addChild(input.extractChild(input.size() - 1));
  }

  while(!buffer.empty()) {
    addChild(buffer.extractChild(buffer.size() - 1));
  }
}

void dnode::copyChildrenFrom(const dnode &input)
{
  if (!input.isParent())
    throwNotParent();

  setupChildren(!input.isList());

  if (input.isList()) {
    for(size_type i=0,epos = input.size(); i != epos; i++) {
      addChild(new dnode(input[i]));
    }
  } else { // parent
    for(size_type i=0,epos = input.size(); i != epos; i++) {
      addChild(input.getElementName(i), new dnode(input[i]));
    }
  }
}

/*
void dnode::initArray(dnValueType a_valueType)
{
  switch (a_valueType) {
    case vt_int:
      inherited::setAsArray(new dnArrayOfInt());
      break;
    case vt_float:
      inherited::setAsArray(new dnArrayOfFloat());
      break;
    case vt_double:
      inherited::setAsArray(new dnArrayOfDouble());
      break;
    case vt_xdouble:
      //inherited::setAsArray(new dnArrayOfXDouble());
      inherited::setAsArray(new dnArrayByIntMeta<vt_xdouble>::implementation_type());
      break;
    default:
      //inherited::setAsArray(new dnArrayOfDataNode());
      inherited::setAsArray(new dnArrayByIntMeta<vt_datanode>::implementation_type());
      break;
  }
}
*/

void dnode::disposeArray()
{
}

dnode::dnode(const dnValue &src)
{
  initProps();
  inherited::copyFrom(src);
}

/*
dnode::dnode(const dtpString &value)
{
  initProps();
  inherited::initAsString(value);
}
*/

/*
dnode::dnode(const char *value)
{
  initProps();
  inherited::initAsString(dtpString(value));
}
*/

/*
dnode::dnode(float value)
{
  initProps();
  inherited::initAsFloat(value);
}

dnode::dnode(double value)
{
  initProps();
  inherited::initAsDouble(value);
}
*/

/*
dnode::dnode(xdouble value)
{
  initProps();
  inherited::initAsXDouble(value);
}
*/

/*
dnode::dnode(void_ptr value)
{
  initProps();
  inherited::initAsVoidPtr(value);
}

dnode::dnode(bool value)
{
  initProps();
  inherited::initAsBool(value);
}

dnode::dnode(byte value)
{
  initProps();
  inherited::initAsByte(value);
}

dnode::dnode(int value)
{
  initProps();
  inherited::initAsInt(value);
}

dnode::dnode(uint value)
{
  initProps();
  inherited::initAsUInt(value);
}

dnode::dnode(int64 value)
{
  initProps();
  inherited::initAsInt64(value);
}

dnode::dnode(uint64 value)
{
  initProps();
  inherited::initAsUInt64(value);
}
*/

dnode::dnode(dnInitContainerType value)
{
  initProps();
  switch(value) {
    case ict_list:
      setAsList();
      break;
    default:
      setAsParent();
  }
}

/*
dnode::dnode(dnInitContainerType value, dnValueType itemType)
{
  initProps();
  switch(value) {
    case ict_array:
      setAsArray(itemType);
      break;
    case ict_list:
      setAsList();
      break;
    default:
      setAsParent();
  }
}
*/

dnode::size_type dnode::indexOfName(const dtpString &name) const
{
    if (isArray()) {
        return getArrayR()->findByName(name);
    } else if (isParent() || isList()) {
        return getAsChildrenNoCheckR()->indexOfName(name);
    } else {
        throw dnError("Wrong container type");
    }
}

dnode::size_type dnode::intIndexOfValue(const dnode &value) const
{
    if (isArray()) {
        return getArrayR()->indexOfValue(value);
    } else if (isParent() || isList()) {
        return getAsChildrenNoCheckR()->indexOfValue(value);
    } else {
        throw dnError("Wrong container type");
    }
}

/// verifies if value is inside the container, for sorted container use dnode::binary_search
bool dnode::hasValue(const dnode &value) const
{
  return (indexOfValue(value) != dnode::npos);
}

void dnode::setAsArray(dnValueType a_valueType)
{
  setAsArray(dnArrayTypeRegister::newArray(a_valueType));
}

dnChildColnBase &dnode::getChildren()
{
  if (isParent()) {
    return *(getAsChildrenNoCheck());
  } else {
    throwNotParent();
    return *(getAsChildrenNoCheck()); // for warnings
  }
}

const dnChildColnBase &dnode::getChildrenR() const
{
  if (isParent()) {
    return *getAsChildrenNoCheckR();
  } else {
    throwNotParent();
    dnChildColnBase *ptr = NULL;
    return *ptr;
  }
}

dnChildColnBase *dnode::getChildrenPtr()
{
  if (isParent()) {
    return getAsChildrenNoCheck();
  } else {
    return DTP_NULL;
  }
}

const dnChildColnBase *dnode::getChildrenPtrR() const
{
  if (isParent()) {
    return getAsChildrenNoCheckR();
  } else {
    return DTP_NULL;
  }
}

void dnode::setAsParent()
{
  if (!isParent()) {
    clear();
    setupChildren(true);
  }
}

void dnode::setAsList()
{
  if (!isList()) {
    clear();
    setupChildren(false);
  }

}

void dnode::setAsNull()
{
  clear();
}

void dnode::intAddChild(dnode *child)
{
  setupChildren(false).insert(child);
}

void dnode::intAddChildAtFront(dnode *child)
{
  setupChildren(false).insert(0, child);
}

void dnode::intAddChildAtFront(const dtpString &aName, dnode *child)
{
  setupChildren(false).insert(0, aName, child);
}

void dnode::intAddChildAtPos(size_type pos, dnode *child)
{
  setupChildren(false).insert(pos, child);
}

void dnode::intAddChildAtPos(size_type pos, const dtpString &aName, dnode *child)
{
  setupChildren(false).insert(pos, aName, child);
}


void dnode::intAddChild(const dtpString &name, dnode *child)
{
#ifdef DEBUG_DTYPES
  if (child->hasName() && hasChild(child->getName()))
  {
    throw dnError(dtpString("Child already assigned: ")+child->getName());
  }
#endif
  setupChildren(true).insert(name, child);
}

dnode *dnode::extractChild(int index)
{
  return getChildren().extractChild(index);
}

void dnode::checkArrayType(dnValueType atype)
{
  if (!isArray())
    setAsArray(atype);
  dnValueType arrValueType = getArray()->getValueType();
  runtime_check(
    (
      (arrValueType == vt_datanode)
      ||
      (arrValueType == atype)
    ), "Incorrect item type"
  );
}

void dnode::prepareArrayType(dnValueType atype)
{
  if (!isArray())
    setAsArray(atype);
}

void dnode::addItem(const dnode &input)
{
  prepareArrayType(input.getValueType());
  getArray()->addItem(const_cast<dnode &>(input));
}

void dnode::addItemAtFront(const dnode &input)
{
  checkArrayType(input.getValueType());
  getArray()->addItemAtFront(const_cast<dnode &>(input));
}

void dnode::addItemAtPos(size_type pos, const dnode &input)
{
  checkArrayType(input.getValueType());
  getArray()->addItemAtPos(pos, const_cast<dnode &>(input));
}

void dnode::addItem(const dnValue &input)
{
  checkArrayType(input.getValueType());
  getArray()->addItem(input);
}

void dnode::addItemList(const dnode &input)
{
  if (!isArray()) {
    if (!input.isArray()) {
      //setAsArray(vt_datanode);
      setAsArray<dnode>();
    }
    else {
      //setAsArray(input.getArrayR()->getValueType());
      setAsArray(input.getArrayR()->cloneEmpty());
    }
  }

  dnArray *arr = getArray();

  runtime_check(
    (
    (arr->getValueType() == vt_datanode)
    ||
    (arr->getValueType() == const_cast<dnode &>(input).getValueType())
    ),"Wrong item list type"
  );

 if (!input.isArray()) {
   if (input.isParent()) {
     dnode element;
     for(size_type i=0,epos = input.size(); i!=epos; i++) {
       addItem(input.getNode(i, element));
     }
   }
   else {
     throw dnError("Not a container for addItemList");
   }
 } else if (!input.empty()){
   size_type beginPos = 0;
   size_type endPos = input.size();
   dnArray *inArray = const_cast<dnode &>(input).getArray();

   // general branch
   dnode item;

   for(size_type i=beginPos; i!=endPos; i++) {
     inArray->getItem(i, item);
     arr->addItem(item);
   }
 }
}

void dnode::addItemList(const dnode_vector &input)
{
  if (!isArray()) {
    //setAsArray(vt_datanode);
    setAsArray<dnode>();
  }

 if (!input.empty()) {
   dnArray *arr = getArray();
   for(dnode_vector::const_iterator it = input.begin(), epos = input.end(); it != epos; ++it) {
     arr->addItem(*it);
   }
 }
}

void dnode::addItemAsString(const dtpString &value)
{
  checkArrayType(vt_datanode);
  static_cast<dnArrayBase *>(getArray())->addItem(value);
}

void dnode::addItemAsFloat(float value)
{
  checkArrayType(vt_float);
  static_cast<dnArrayBase *>(getArray())->addItem(value);
}

void dnode::addItemAsDouble(double value)
{
  checkArrayType(vt_double);
  static_cast<dnArrayBase *>(getArray())->addItem(value);
}

void dnode::addItemAsXDouble(xdouble value)
{
  checkArrayType(vt_xdouble);
  static_cast<dnArrayBase *>(getArray())->addItem(value);
}

void dnode::addItemAsVoidPtr(void_ptr value)
{
  checkArrayType(vt_datanode);
  static_cast<dnArrayBase *>(getArray())->addItem(value);
}

void dnode::addItemAsBool(bool value)
{
  checkArrayType(vt_datanode);
  static_cast<dnArrayBase *>(getArray())->addItem(value);
}

void dnode::addItemAsInt(int value)
{
  checkArrayType(vt_int);
  static_cast<dnArrayBase *>(getArray())->addItem(value);
}

void dnode::addItemAsUInt(uint value)
{
  checkArrayType(vt_uint);
  static_cast<dnArrayBase *>(getArray())->addItem(value);
}

void dnode::addItemAsInt64(int64 value)
{
  static_cast<dnArrayBase *>(getArray())->addItem(value);
}

void dnode::addItemAsUInt64(uint64 value)
{
  static_cast<dnArrayBase *>(getArray())->addItem(value);
}

bool dnode::hasChildren() const
{
  if (isParent())
    return (!getAsChildrenNoCheckR()->empty());
  else
    return (false);
}

bool dnode::hasChild(const dtpString &name) const
{
  if (isParent())
    return (getAsChildrenNoCheckR()->hasChild(name));
  else
    return false;
}

const dnode dnode::childNames(bool useAsNames) const
{
  dnode res, elem;

  if (isParent())
  {

    if (useAsNames) {
      res.setAsParent();
      for(size_type i=0, epos = size(); i != epos; i++)
      {
        res.addChild(new dnode(getElementName(i)));
      }
    } else {
      for(size_type i=0, epos = size(); i != epos; i++)
      {
        res.addItem(dnode(getElementName(i)));
      }
    }
  }
  return res;
}

/// Returns child pointer - if found. If not found - NULL.
dnode *dnode::peekChild(const dtpString &aName)
{
  if (isParent()) {
    dnChildColnBase *ptr = getAsChildrenNoCheck();
    //if (ptr->hasChild(aName))
    //  return &(ptr->getByName(aName));
    return ptr->peekChild(aName);
  }

  fastThrowNotParent();
  return DTP_NULL;
}

/// read-only version of peekChild
const dnode *dnode::peekChildR(const dtpString &aName) const
{
  if (isParent()) {
    const dnChildColnBase *ptr = getAsChildrenNoCheckR();
    //if (ptr->hasChild(aName))
    //  return &(ptr->getByName(aName));
    return ptr->peekChildR(aName);
  }

  fastThrowNotParent();
  return DTP_NULL;
}

bool dnode::hasItems() const
{
  if (isArray() && (getArrayR() != DTP_NULL))
    return (!getArrayR()->empty());
  else
    return (false);
}

dnode::size_type dnode::size() const
{
  if (isArray())
    return getArrayR()->size();
  else if (isParent()) {
    if (getChildrenPtrR() != DTP_NULL)
      return getChildrenPtrR()->size();
    else
      return 0;
  }
  else
    return 0;
}

bool dnode::empty() const
{
  if (isArray())
    return getArrayR()->empty();
  else if (isParent())
    return getChildrenPtrR()->empty();
  else
    return true;
}

dnode &dnode::getElement(int index, dnode &output) const
{
  if (isArray())
    getArrayR()->getItem(index, output);
  else if (isParent())
    output = const_cast<dnChildColnBase *>(getChildrenPtrR())->at(index);
  else
    throwNotContainer();
  return output;
}

const dnode dnode::getElement(int index) const
{
  dnode res;
  getElement(index, res);
  return res;
}

dnode &dnode::getElement(const dtpString &aName, dnode &output) const
{
  if (isArray()) {
    size_type idx = getArrayR()->findByName(aName);
    getArrayR()->getItem(idx, output);
  }
  else if (isParent())
    output = (*this)[aName];
  else
    throwNotContainer();
  return output;
}

void dnode::throwNotContainer()
{
  throw dnError("Not a container!");
}

void dnode::throwNotParent()
{
  throw dnError("Not a parent!");
}

void dnode::throwNotFound(const dtpString &aName)
{
  throw dnError("Item does not exist: [" + aName + "]");
}

const dnode dnode::getElement(const dtpString &aName) const
{
  dnode res;
  getElement(aName, res);
  return res;
}

// returns <false> if element does not exist
bool dnode::getElementSafe(const dtpString &aName, dnode &output) const
{
  if (!hasElement(aName))
    return false;
  else {
    getElement(aName, output);
    return true;
  }
}

bool dnode::hasElement(const dtpString &name) const
{
  if (isParent())
    return getAsChildrenNoCheckR()->hasChild(name);
  else if (isArray())
    return (getArrayR()->findByName(name) != dnArray::npos);
  else
    return false;
}

dnValueType dnode::getElementType(int index) const
{
  dnValueType res;
  if (isArray()) {
    res = getArrayR()->getValueType();
    if (res == vt_datanode) {
      dnode item;
      getArrayR()->getItem(index, item);
      res = item.getValueType();
    }
  } else if (isParent()) {
    res = getChildrenPtrR()->at(index).getValueType();
  } else {
    throwNotContainer();
    res = vt_null;
  }
  return res;
}

dnValueType dnode::getElementType(const dtpString &aName) const
{
  dnValueType res;
  if (isArray()) {
    res = getArrayR()->getValueType();
    if (res == vt_datanode) {
      dnode item;
      size_type index = getArrayR()->findByName(aName);
      if (index != dnArray::npos) {
        getArrayR()->getItem(index, item);
        res = item.getValueType();
      } else {
        res = vt_null;
        throw dnError("Item does not exist: [" + aName + "]");
      }
    }
  } else if (isParent()) {
    res = getChildrenPtrR()->getByNameR(aName).getValueType();
  } else {
    throwNotContainer();
    res = vt_null;
  }
  return res;
}

dnValueType dnode::getElementType() const
{
  dnValueType res;
  if (isArray()) {
    res = getArrayR()->getValueType();
  } else if (isContainer()) {
    res = vt_datanode;
  } else {
    throwNotContainer();
    res = vt_null;
  }
  return res;
}

void dnode::forceElementType(int index, dnValueType valueType)
{
  if (isArray()) {
    dnValueType oldValType;
    oldValType = getArrayR()->getValueType();
    if (oldValType == vt_datanode) {
      dnode item;
      getArrayR()->getItem(index, item);
      item.convertTo(valueType);
      getArray()->setItem(index, item);
    } else {
      if (oldValType != valueType)
        throw dnError("Cannot change value type inside array");
    }
  } else if (isParent()) {
    getChildrenPtr()->at(index).convertTo(valueType);
  } else {
    throwNotContainer();
  }
}

void dnode::forceElementTypeAll(dnValueType valueType)
{
  size_t aSize = size();

  if (isArray()) {
    dnValueType oldValType;
    oldValType = getArrayR()->getValueType();
    if (oldValType == vt_datanode) {
      dnode item;
      dnArray *myArray = getArray();
      for(uint i=0; i != aSize; i++) 
      {
        myArray->getItem(i, item);
        item.convertTo(valueType);
        myArray->setItem(i, item);
      }
    } else {
      if (oldValType != valueType)
        throw dnError("Cannot change value type inside array");
    }
  } else if (isParent()) {
    dnChildColnBase *coln = getChildrenPtr();

    for(uint i=0; i != aSize; i++) 
    {
      coln->at(i).convertTo(valueType);
    }
  } else if (!isContainer()) {
    convertTo(valueType);
  } else {
    throw dnError("Wrong container type");
  }
}

const dtpString dnode::getElementName(int index) const
{
  dtpString res;
  getElementName(index, res);
  return res;
}

void dnode::getElementName(int index, dtpString &output) const
{
  if (isArray()) {
    output = "";
  } else if (isParent()) {
    output = const_cast<dnChildColnBase *>(getChildrenPtrR())->getName(index);
  } else {
    throwNotContainer();
  }
}

dnode *dnode::cloneElement(int index) const
{
  DTP_UNIQUE_PTR(dnode) guard(new dnode());
  getElement(index, *guard);
  return guard.release();
}

void dnode::setElement(int index, const dnode &value)
{
  if (isArray())
    getArray()->setItem(index, value);
  else if (isParent())
    getChildrenPtr()->at(index).copyValueFrom(value);
  else
    throwNotContainer();
}

// throws error if element was not found
void dnode::setElement(const dtpString &aName, const dnode &value)
{
  if (isArray()) {
    size_type idx = getArray()->findByName(aName);
    if (idx == dnArray::npos) {
      throw dnError("Item does not exist: [" + aName + "]");
    } else {
      getArray()->setItem(idx, value);
    }
  }
  else if (isParent())
    (*this)[aName].copyValueFrom(value);
  else
    throwNotContainer();
}

void dnode::setElementValue(int index, const dnode &value)
{
  if (isArray()) {
    getArray()->setItemValue(index, value);
  } else if (isParent())
    getChildrenPtr()->at(index).copyValueFrom(value);
  else
    throwNotContainer();
}

void dnode::setElementValue(const dtpString &aName, const dnode &value)
{
  if (isArray()) {
    size_type idx = getArray()->findByName(aName);
    if (idx == dnArray::npos) {
      throwNotFound(aName);
    } else {
      getArray()->setItemValue(idx, value);
    }
  }
  else if (isParent())
    (*this)[aName].copyValueFrom(value);
  else
    throwNotContainer();
}

// returns <false> if element was not in container before
bool dnode::setElementSafe(const dtpString &aName, const dnode &value)
{
  bool res = true;
  if (isParent()) {
    if (!getAsChildrenNoCheckR()->hasChild(aName)) {
      addChild(aName, new dnode(value));
      res = false;
    } else {
      setElement(aName, value);
    }
  } else if (isArray()) {
    size_type idx = getArray()->findByName(aName);
    if (idx == dnArray::npos) {
      addElement(aName, value);
      res = false;
    } else {
      setElement(idx, value);
    }
  } else {
    throwNotContainer();
  }
  return res;
}

bool dnode::setElementSafe(const dtpString &aName, base::move_ptr<dnode> value)
{
  bool res = true;
  if (isParent()) {
    if (!getAsChildrenNoCheckR()->hasChild(aName)) {
      addChild(aName, new dnode(value));
      res = false;
    } else {
      setElement(aName, *value);
    }
  } else if (isArray()) {
    size_type idx = getArray()->findByName(aName);
    if (idx == dnArray::npos) {
      addElement(aName, *value);
      res = false;
    } else {
      setElement(idx, *value);
    }
  } else {
    throwNotContainer();
  }
  return res;
}

dnode &dnode::addElement(const dnode &value)
{
  if (isArray())
    getArray()->addItem(value);
  else if (isParent() || isList())
    getChildrenPtr()->insert(new dnode(value));
  else
    throwNotContainer();
  return *this;
}

dnode &dnode::addElement(const dtpString &aName, const dnode &value)
{
  if (isArray())
    getArray()->addItem(value);
  else if (isParent() || isList())
    getChildrenPtr()->insert(aName, new dnode(value));
  else
    throwNotContainer();
  return *this;
}

dnode &dnode::addElement(base::move_ptr<dnode> value)
{
  if (isArray())
    getArray()->addItem(value);
  else if (isParent() || isList())
    getChildrenPtr()->insert(new dnode(value));
  else
    throwNotContainer();
  return *this;
}

dnode &dnode::addElement(const dtpString &aName, base::move_ptr<dnode> value)
{
  if (isArray())
    getArray()->addItem(value);
  else if (isParent() || isList())
    getChildrenPtr()->insert(aName, new dnode(value));
  else
    throwNotContainer();
  return *this;
}

void dnode::eatElement(dnode& src)
{
  if (isArray())
    getArray()->eatItem(src);
  else if (isParent() || isList()) {
    DTP_UNIQUE_PTR(dnode) childGuard(new dnode);
    childGuard->moveFrom(src);
    getChildrenPtr()->insert(childGuard.release());
  } else {
    throwNotContainer();
  }
}

void dnode::eraseElement(uint index)
{
  assert(index < size());
  if (isArray())
    getArray()->eraseItem(index);
  else if (isParent() || isList())
    getChildrenPtr()->erase(index);
  else
    throwNotContainer();
}

void dnode::eraseFrom(uint index)
{
  assert(index < size());
  if (isArray())
    getArray()->eraseFrom(index);
  else if (isParent())
    getChildrenPtr()->eraseFrom(index);
  else
    throwNotContainer();
}

void dnode::clearValue()
{
  clear();
}

void dnode::clearElements()
{
  if (getChildrenPtr() != DTP_NULL) {
    setAsParent(DTP_NULL);
  } else if (getArray() != DTP_NULL) {
    setAsArray(DTP_NULL);
  }
}

void dnode::merge(const dnode &rhs)
{
  if (!isContainer())
  {
    setAsParent();
  }

  dnode element;
  const dnode *elementPtr;
  dtpString name;

  for(size_type i=0, epos = rhs.size(); i != epos; i++)
  {
    elementPtr = rhs.getNodePtrR(i, element);
    name = rhs.getElementName(i);
    if (name.empty() || (!hasChild(name)))
      addElement(name, *elementPtr);
    else
      setElementSafe(name, *elementPtr);
  }
}

void dnode::merge(base::move_ptr<dnode> rhs)
{
  if (!isContainer())
  {
    setAsParent();
  }

  dnode element;
  dtpString name;

  if (rhs->isParent()) {
    std::auto_ptr<dnode> deleter;
    dnode *elementPtr;
    if (!rhs->empty())
    for(size_type i = rhs->size(); i > 0; )
    {
      i--;
      name = rhs->getElementName(i);
      deleter.reset(rhs->extractChild(i));
      if (name.empty()) {
        addElement(*deleter);
      } else if (!hasChild(name)) {
        addChild(name, deleter.release());
      } else {
        setElementSafe(name, *deleter);
      }
    }
  } else {
    const dnode *elementPtr;
    for(size_type i=0, epos = rhs->size(); i != epos; i++)
    {
      elementPtr = rhs->getNodePtrR(i, element);
      name = rhs->getElementName(i);
      if (name.empty() || (!hasChild(name)))
        addElement(name, *elementPtr);
      else
        setElementSafe(name, *elementPtr);
    }
  }
}

bool dnode::sort()
{
  if (isArray())
    return getArray()->sort();
  else
    return false;
}

void dnode::resize(size_type newSize)
{
  if (isArray())
    getArray()->resize(newSize);
  else if (isParent())
    getChildrenPtr()->resize(newSize);
  else
    throwNotContainer();
}

const dnode &dnode::operator[](int idx) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  return ptr->at(idx);
}

const dnode &dnode::operator[](const dtpString &str_idx) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  return ptr->getByNameR(str_idx);
}

dnode &dnode::operator[](int idx)
{
  return getChildrenPtr()->at(idx);
}

dnode &dnode::operator[](const dtpString &str_idx)
{
#ifdef DATANODE_AUTO_ADD_ON_SET
  if (!hasChild(str_idx))
    addChild(str_idx, new dnode());
#endif
  return getChildrenPtr()->getByName(str_idx);
}

// ----------------------------------------------------------------------------
/// getter/setter shortcuts
// ----------------------------------------------------------------------------
const dtpString dnode::getString(const dtpString &a_name, const dtpString &a_defValue) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  const dnode *child = ptr->peekChildR(a_name);
  dtpString res;
  if (child != NULL) {
    if (child->getValueType() == vt_null)
      res = a_defValue;
    else
      res = child->getAs<dtpString>();
  }
  else
    res = a_defValue;
  return res;
}

const dtpString dnode::getString(const dtpString &a_name) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  dtpString res;
  res = ptr->getByNameR(a_name).getAs<dtpString>();
  return res;
}

bool dnode::getBool(const dtpString &a_name, bool a_defValue) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  const dnode *child = ptr->peekChildR(a_name);
  bool res;
  if (child != NULL) {
    if (child->getValueType() == vt_null)
      res = a_defValue;
    else
      res = child->getAs<bool>();
  }
  else
    res = a_defValue;
  return res;
}

bool dnode::getBool(const dtpString &a_name) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  bool res;
  res = ptr->getByNameR(a_name).getAs<bool>();
  return res;
}

int dnode::getInt(const dtpString &a_name, int a_defValue) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  const dnode *child = ptr->peekChildR(a_name);
  int res;
  if (child != NULL) {
    if (child->getValueType() == vt_null)
      res = a_defValue;
    else
      res = child->getAs<int>();
  }
  else
    res = a_defValue;
  return res;
}

int dnode::getInt(const dtpString &a_name) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  int res;
  res = ptr->getByNameR(a_name).getAs<int>();
  return res;
}

byte dnode::getByte(const dtpString &a_name, byte a_defValue) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  const dnode *child = ptr->peekChildR(a_name);
  byte res;
  if (child != NULL) {
    if (child->getValueType() == vt_null)
      res = a_defValue;
    else
      res = child->getAs<byte>();
  }
  else
    res = a_defValue;
  return res;
}

byte dnode::getByte(const dtpString &a_name) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  byte res;
  res = ptr->getByNameR(a_name).getAs<byte>();
  return res;
}

uint dnode::getUInt(const dtpString &a_name, uint a_defValue) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  const dnode *child = ptr->peekChildR(a_name);
  uint res;
  if (child != NULL) {
    if (child->getValueType() == vt_null)
      res = a_defValue;
    else
      res = child->getAs<uint>();
  }
  else
    res = a_defValue;
  return res;
}

uint dnode::getUInt(const dtpString &a_name) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  uint res;
  res = ptr->getByNameR(a_name).getAs<uint>();
  return res;
}

int64 dnode::getInt64(const dtpString &a_name, int64 a_defValue) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  const dnode *child = ptr->peekChildR(a_name);
  int64 res;
  if (child != NULL) {
    if (child->getValueType() == vt_null)
      res = a_defValue;
    else
      res = child->getAs<int64>();
  }
  else
    res = a_defValue;
  return res;
}

int64 dnode::getInt64(const dtpString &a_name) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  assert(ptr != NULL);
  int64 res;
  res = ptr->getByNameR(a_name).getAs<int64>();
  return res;
}


uint64 dnode::getUInt64(const dtpString &a_name, uint64 a_defValue) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  assert(ptr != NULL);
  const dnode *child = ptr->peekChildR(a_name);
  uint64 res;
  if (child != NULL) {
    if (child->getValueType() == vt_null)
      res = a_defValue;
    else
      res = child->getAs<uint64>();
  }
  else
    res = a_defValue;
  return res;
}

uint64 dnode::getUInt64(const dtpString &a_name) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  uint64 res;
  res = ptr->getByNameR(a_name).getAs<uint64>();
  return res;
}

float dnode::getFloat(const dtpString &a_name, float a_defValue) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  const dnode *child = ptr->peekChildR(a_name);
  float res;
  if (child != NULL) {
    if (child->getValueType() == vt_null)
      res = a_defValue;
    else
      res = child->getAs<float>();
  }
  else
    res = a_defValue;
  return res;
}

float dnode::getFloat(const dtpString &a_name) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  float res;
  res = ptr->getByNameR(a_name).getAs<float>();
  return res;
}

double dnode::getDouble(const dtpString &a_name, double a_defValue) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  const dnode *child = ptr->peekChildR(a_name);
  double res;
  if (child != NULL) {
    if (child->getValueType() == vt_null)
      res = a_defValue;
    else
      res = child->getAs<double>();
  }
  else
    res = a_defValue;
  return res;
}

double dnode::getDouble(const dtpString &a_name) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  double res;
  res = ptr->getByNameR(a_name).getAs<double>();
  return res;
}

xdouble dnode::getXDouble(const dtpString &a_name, xdouble a_defValue) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  const dnode *child = ptr->peekChildR(a_name);
  xdouble res;
  if (child != NULL) {
    if (child->getValueType() == vt_null)
      res = a_defValue;
    else
      res = child->getAs<xdouble>();
  }
  else
    res = a_defValue;
  return res;
}

xdouble dnode::getXDouble(const dtpString &a_name) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  xdouble res;
  res = ptr->getByNameR(a_name).getAs<xdouble>();
  return res;
}

void_ptr dnode::getVoidPtr(const dtpString &a_name, void_ptr a_defValue) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  const dnode *child = ptr->peekChildR(a_name);
  void_ptr res;
  if (child != NULL) {
    if (child->getValueType() == vt_null)
      res = a_defValue;
    else
      res = child->getAs<void_ptr>();
  }
  else
    res = a_defValue;
  return res;
}

void_ptr dnode::getVoidPtr(const dtpString &a_name) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  void_ptr res;
  res = ptr->getByNameR(a_name).getAs<void_ptr>();
  return res;
}

fdatetime_t dnode::getDateTime(const dtpString &a_name, fdatetime_t a_defValue) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  const dnode *child = ptr->peekChildR(a_name);
  fdatetime_t res;
  if (child != NULL) {
    if (child->getValueType() == vt_null)
      res = a_defValue;
    else
      res = child->getAs<double>();
  }
  else
    res = a_defValue;
  return res;
}

fdatetime_t dnode::getDateTime(const dtpString &a_name) const
{
  const dnChildColnBase *ptr = getChildrenPtrR();
  fdatetime_t res;
  res = ptr->getByNameR(a_name).getAs<double>();
  return res;
}

// setters
void dnode::setString(const dtpString &a_name, const dtpString &value)
{
  (*this)[a_name].setAs<dtpString>(value);
}

void dnode::setString(const dtpString &a_name, const char *value)
{
  (*this)[a_name].setAs<dtpString>(dtpString(value));
}

void dnode::setBool(const dtpString &a_name, bool value)
{
  (*this)[a_name].setAs<bool>(value);
}

void dnode::setInt(const dtpString &a_name, int value)
{
  (*this)[a_name].setAs<int>(value);
}

void dnode::setByte(const dtpString &a_name, byte value)
{
  (*this)[a_name].setAs<byte>(value);
}

void dnode::setUInt(const dtpString &a_name, uint value)
{
  (*this)[a_name].setAs<uint>(value);
}

void dnode::setUIntDef(const dtpString &a_name, uint value)
{
  if (!hasChild(a_name))
    addChild(a_name, new dnode(value));

  (*this)[a_name].setAs<uint>(value);
}

void dnode::setInt64(const dtpString &a_name, int64 value)
{
  (*this)[a_name].setAs<int64>(value);
}

void dnode::setUInt64(const dtpString &a_name, uint64 value)
{
  (*this)[a_name].setAs<uint64>(value);
}

void dnode::setFloat(const dtpString &a_name, float value)
{
  (*this)[a_name].setAs<float>(value);
}

void dnode::setDouble(const dtpString &a_name, double value)
{
  (*this)[a_name].setAs<double>(value);
}

void dnode::setXDouble(const dtpString &a_name, xdouble value)
{
  (*this)[a_name].setAs<xdouble>(value);
}

void dnode::setVoidPtr(const dtpString &a_name, void_ptr value)
{
  (*this)[a_name].setAs<void_ptr>(value);
}

void dnode::setDateTime(const dtpString &a_name, fdatetime_t value)
{
  (*this)[a_name].setAs<double>(value);
}

//
dtpString dnode::getString(int a_index) const
{
  dnode resNode;
  return getNode(a_index, resNode).getAs<dtpString>();
}

bool dnode::getBool(int a_index) const
{
  dnode resNode;
  return getNode(a_index, resNode).getAs<bool>();
}

int dnode::getInt(int a_index) const
{
  //--- optimized version:
  int res;
  if (isArray()) {
    dnode output;
    getArrayR()->getItem(a_index, output);
    res = output.getAs<int>();
  } else if (isParent()) {
    res = getAsChildrenNoCheckR()->at(a_index).getAs<int>();
  } else {
    throwNotContainer();
    res = 0; // for warnings
  }
  return res;
  //---
}

byte dnode::getByte(int a_index) const
{
  dnode resNode;
  return getNode(a_index, resNode).getAs<byte>();
}

uint dnode::getUInt(int a_index) const
{
  //--- optimized version:
  uint res;
  if (isArray()) {
    dnode output;
    getArrayR()->getItem(a_index, output);
    res = output.getAs<uint>();
  } else if (isParent()) {
    res = getChildrenPtrR()->at(a_index).getAs<uint>();
  } else {
    throwNotContainer();
    res = 0; // for warnings
  }
  return res;
  //---
}

int64 dnode::getInt64(int a_index) const
{
  dnode resNode;
  return getNode(a_index, resNode).getAs<int64>();
}

uint64 dnode::getUInt64(int a_index) const
{
  dnode resNode;
  return getNode(a_index, resNode).getAs<uint64>();
}

float dnode::getFloat(int a_index) const
{
  //--- optimized version:
  float res;
  if (isArray()) {
    dnode output;
    getArrayR()->getItem(a_index, output);
    res = output.getAs<float>();
  } else if (isParent()) {
    res = getChildrenPtrR()->at(a_index).getAs<float>();
  } else {
    throwNotContainer();
    res = 0.0f; // for warnings
  }
  return res;
  //---
}

double dnode::getDouble(int a_index) const
{
  //--- optimized version:
  double res;
  if (isArray()) {
    res = getArrayR()->get<double>(a_index);
  } else if (isParent()) {
    res = getChildrenPtrR()->at(a_index).getAs<double>();
  } else {
    res = 0.0;
    throwNotContainer();
    //res = 0.0; // for warnings
  }
  return res;
  //---
}

xdouble dnode::getXDouble(int a_index) const
{
  dnode resNode;
  return getNode(a_index, resNode).getAs<xdouble>();
}

void_ptr dnode::getVoidPtr(int a_index) const
{
  dnode resNode;
  return getNode(a_index, resNode).getAs<void_ptr>();
}

// setters
void dnode::setString(int a_index, const dtpString &value)
{
  if (isArray()) {
    dnode bufNode;
    bufNode.setAs<dtpString>(value);
    setElementValue(a_index, bufNode);
  } else {
    (*this)[a_index].setAs<dtpString>(value);
  }
}

void dnode::setString(int a_index, const char *value)
{
  if (isArray()) {
    dnode bufNode;
    bufNode.setAs<dtpString>(dtpString(value));
    setElementValue(a_index, bufNode);
  } else {
    (*this)[a_index].setAs<dtpString>(dtpString(value));
  }
}

void dnode::setBool(int a_index, bool value)
{
  if (isArray()) {
    dnode bufNode;
    bufNode.setAs<bool>(value);
    setElementValue(a_index, bufNode);
  } else {
    (*this)[a_index].setAs<bool>(value);
  }
}

void dnode::setInt(int a_index, int value)
{
  if (isArray()) {
    dnode bufNode;
    bufNode.setAs<int>(value);
    setElementValue(a_index, bufNode);
  } else {
    (*this)[a_index].setAs<int>(value);
  }
}

void dnode::setByte(int a_index, byte value)
{
  if (isArray()) {
    dnode bufNode;
    bufNode.setAs<byte>(value);
    setElementValue(a_index, bufNode);
  } else {
    (*this)[a_index].setAs<byte>(value);
  }
}

void dnode::setUInt(int a_index, uint value)
{
  if (isArray()) {
    dnode bufNode;
    bufNode.setAs<uint>(value);
    setElementValue(a_index, bufNode);
  } else {
    (*this)[a_index].setAs<uint>(value);
  }
}

void dnode::setInt64(int a_index, int64 value)
{
  if (isArray()) {
    dnode bufNode;
    bufNode.setAs<int64>(value);
    setElementValue(a_index, bufNode);
  } else {
    (*this)[a_index].setAs<int64>(value);
  }
}

void dnode::setUInt64(int a_index, uint64 value)
{
  if (isArray()) {
    dnode bufNode;
    bufNode.setAs<uint64>(value);
    setElementValue(a_index, bufNode);
  } else {
    (*this)[a_index].setAs<uint64>(value);
  }
}

void dnode::setFloat(int a_index, float value)
{
  if (isArray()) {
    dnode bufNode;
    bufNode.setAs<float>(value);
    setElementValue(a_index, bufNode);
  } else {
    (*this)[a_index].setAs<float>(value);
  }
}

void dnode::setDouble(int a_index, double value)
{
  if (isArray()) {
    dnode bufNode;
    bufNode.setAs<double>(value);
    setElementValue(a_index, bufNode);
  } else {
    (*this)[a_index].setAs<double>(value);
  }
}

void dnode::setXDouble(int a_index, xdouble value)
{
  if (isArray()) {
    dnode bufNode;
    bufNode.setAs<xdouble>(value);
    setElementValue(a_index, bufNode);
  } else {
    (*this)[a_index].setAs<xdouble>(value);
  }
}

void dnode::setVoidPtr(int a_index, void_ptr value)
{
  if (isArray()) {
    dnode bufNode;
    bufNode.setAs<void_ptr>(value);
    setElementValue(a_index, bufNode);
  } else {
    (*this)[a_index].setAs<void_ptr>(value);
  }
}

dtpString dnode::dumpHierarchy() const
{
  // TODO: implement for version without parent ptr in element
  dtpString res;
  return res;
}

dtpString dnode::dump(const dtpString &indent, const dtpString &name) const
{
  dtpString res;
  res = indent + "{";
  if (!name.empty())
    res += "head:[name="+name+"];";
  res += "\n"+indent;
  if (isArray())
  {
    res += "as_array:[count="+toString(const_cast<dnode *>(this)->getArray()->size())+"]";
    res += "[item_type="+toString(int(const_cast<dnode *>(this)->getArray()->getValueType()))+"]";
    res += "[items="+dumpItems(indent+"  ")+"]";
    res += "\n";
  } else if (isParent()) {
    res += "as_parent:[count="+toString(const_cast<dnode *>(this)->getChildren().size())+"]";
    res += "[children="+dumpChildren(indent+"  ")+"]";
    res += "\n";
  } else {
    res += "as_scalar:[type="+toString(int(const_cast<dnode *>(this)->getValueType()))+"]";
    res += "[value="+dumpValue(indent+"  ")+"]";
    res += "\n";
  }
  return res;
}

dtpString dnode::dumpItems(const dtpString &indent) const
{
  dtpString res;

  if (!getArrayR())
    res += indent+"array_not_rdy";
  else {
    size_type cnt = getArrayR()->size();
    dnode node;
    for(size_type i=0; i < cnt; ++i)
    {
      getArrayR()->getItem(i, node);
      res += indent+node.getAs<dtpString>()+";\n";
    }
  }
  return res;
}

dtpString dnode::dumpChildren(const dtpString &indent) const
{
  dtpString res;

  if (!getChildrenPtrR())
    res += indent+"children_not_rdy";
  else {
    size_type cnt = const_cast<dnode *>(this)->getChildren().size();
    dnChildColnBase &children = const_cast<dnode *>(this)->getChildren();
    for(size_type i=0; i < cnt; ++i)
      res += children.at(i).dump(indent, children.getName(i))+";\n";
  }
  return res;
}

dtpString dnode::dumpValue(const dtpString &indent) const
{
  dtpString res;
  if (const_cast<dnode *>(this)->isNull())
    res = "/null/";
  else {
    res = const_cast<dnode *>(this)->getAs<dtpString>();
    strReplaceThis(res, "]", "?", true);
    strReplaceThis(res, "[", "?", true);
    strReplaceThis(res, ":", "?", true);
  }
  return res;
}

void dnode::scan(dnScanner &scanner) const
{
  scanner.start();
  intScan(scanner);
  scanner.stop();
}

void dnode::intScan(dnScanner &scanner) const
{
  scanner.nodeOpen(*this);
  if (isArray())
  {
    scanItems(scanner);
  } else if (isParent()) {
    scanChildren(scanner);
  } else {
    scanValue(scanner);
  }
  scanner.nodeClose(*this);
}

void dnode::scanValue(dnScanner &scanner) const
{
  scanner.nodeValue(*this);
}

void dnode::scanChildren(dnScanner &scanner) const
{
  if (getChildrenPtrR() != DTP_NULL)
  {
    int cnt = size();
    for(int i=0; i < cnt; ++i)
      const_cast<dnode *>(this)->getChildren().at(i).intScan(scanner);
  }
}

void dnode::scanItems(dnScanner &scanner) const
{

  if (this->getArrayR())
  {
    int cnt = size();
    dnode node;
    for(int i=0; i < cnt; ++i)
    {
      getArrayR()->getItem(i, node);
      node.intScan(scanner);
    }
  }
}


dtpString dnode::implode(const dtpString &separator) const
{
  dtpString res;
  if (!size()) {
    if (isArray() || isParent() || isNull())
      res = dtpString(); //"";
    else
      res = const_cast<dnode *>(this)->getAs<dtpString>();
  } else {
    dnode value;
    for(size_type i = 0, epos = size(); i != epos; i++)
    {
      if (i != 0)
        res += separator;
      res += get<dtpString>(i);
    }
  }
  return res;
}

dnode &dnode::explode(const dtpString &separator, const dtpString &a_text, dnode &output, bool useAsNames)
{
  dnode value;
  size_t i, tfind;
  DTP_UNIQUE_PTR(dnode) guard;

  output.clear();

  for(i = 0; (tfind = a_text.find(separator, i)) != std::string::npos; i = tfind + 1)
  {
    if (useAsNames) {
      guard.reset(new dnode());
      guard->setAs<dtpString>("");
      output.addChild(a_text.substr(i, tfind - i), guard.release());
    }
    else {
      value.clear();
      value.setAs<dtpString>(a_text.substr(i, tfind - i));
      output.addItem(value);
    }
  }

  if (i < a_text.length())
  {
    if (useAsNames) {
      guard.reset(new dnode());
      guard->setAs<dtpString>("");
      output.addChild(a_text.substr(i, a_text.length() - i), guard.release());
    }
    else {
      value.clear();
      value.setAs<dtpString>(a_text.substr(i, a_text.length() - i));
      output.addItem(value);
    }
  }

  return output;
}

dnode dnode::explode(const dtpString &separator, const dtpString &a_text, bool useAsNames)
{
  dnode res;
  dnode::explode(separator, a_text, res, useAsNames);
  return res;
}

/// Returns node referenced by array of integers ignoring container types.
/// Containers can be arrays, parents, lists.
/// Example:
/// path = [2,"test",1] => child 1 of child "test" of child 2 from this
bool dnode::getElementByPath(const dnode &pathNode, dnode &output)
{
  bool res;
  if (pathNode.size() == 0) {
  // "this" node
    output.copyValueFrom(*this);
    res = true;
  } else if (pathNode.size() == 1) {
  // own element
    dnode pathIndex;
    pathNode.getElement(0, pathIndex);
    if (isArray()) {
      getElement(pathIndex.getAs<uint>(), output);
    } else {
      dtpString sIndex = pathIndex.getAs<dtpString>();
      if (hasChild(sIndex))
        output.copyValueFrom(getChildren().getByName(sIndex));
      else
        output.copyValueFrom(getChildren().at(stringToUInt(sIndex)));
    }
    res = true;
  } else {
  // forward below
    dnode newPath = pathNode;
    newPath.eraseElement(0);

    dnode pathIndex;
    pathNode.getElement(0, pathIndex);

    if (isArray()) {
      dnode tempValue;

      getElement(pathIndex.getAs<uint>(), tempValue);

      res = tempValue.getElementByPath(newPath, output);
    } else if (isParent()) {
      dtpString sIndex = pathIndex.getAs<dtpString>();
      if (hasChild(sIndex))
        res = getChildren().getByName(sIndex).getElementByPath(newPath, output);
      else
        res = getChildren().at(stringToUInt(sIndex)).getElementByPath(newPath, output);
    } else {
      res = false;
    }
  }

  return res;
}

/// Modify child using access path.
/// See getElementByPath
bool dnode::setElementByPath(const dnode &pathNode, const dnode &value)
{
  bool res;
  if (pathNode.size() == 0) {
  // "this" node
    copyValueFrom(value);
    res = true;
  } else if (pathNode.size() == 1) {
  // own element
    dnode pathIndex;
    pathNode.getElement(0, pathIndex);
    if (isArray()) {
      setElement(pathIndex.getAs<uint>(), value);
    } else { // parent
      dtpString sIndex = pathIndex.getAs<dtpString>();
      if (hasChild(sIndex))
        (getChildren().getByName(sIndex)).copyValueFrom(value);
      else
        (getChildren().at(pathIndex.getAs<uint>())).copyValueFrom(value);
    }
    res = true;
  } else {
  // forward below
    dnode newPath = pathNode;
    newPath.eraseElement(0);

    dnode pathIndex;
    pathNode.getElement(0, pathIndex);
    if (isArray()) {
      dnode tempValue;
      uint idx = pathIndex.getAs<uint>();

      getElement(idx, tempValue);
      tempValue.setElementByPath(newPath, value);
      setElement(idx, tempValue);

      res = true;
    } else if (isParent()) {
      dtpString sIndex = pathIndex.getAs<dtpString>();
      if (hasChild(sIndex))
        res = (getChildren().getByName(sIndex)).setElementByPath(newPath, value);
      else
        res = (getChildren().at(pathIndex.getAs<uint>())).setElementByPath(newPath, value);
    } else {
      res = false;
    }
  }

  return res;
}

/// Returns item value as a node, helper can be used as a temporary object
/// Similar to getElement version, but faster when helper is not required.
/// Note: returned value can be only a shadow of existing node, so
//  to change it's contents you need to call setNode()
const dnode &dnode::getNode(int index, dnode &helper) const
{
  if (isParent()) {
    return getChildrenPtrR()->at(index);
  } else if (isArray()) {
    //getArrayR()->getItem(index, helper);
    //return helper;
    return getArrayR()->getNode(index, helper);
  } else {
    throwNotContainer();
    return helper;
  }
}

/// Returns item value as a node, helper can be used as a temporary object
/// Similar to getElement version, but faster when helper is not required.
/// Note: returned value can be only a shadow of existing node, so
//  to change it's contents you need to call setNode()
const dnode &dnode::getNode(const dtpString &aName, dnode &helper) const
{
  if (isParent()) {
    return (*this)[aName];
  } else if (isArray()) {
    const dnArray *arr = getArrayR();
    size_type idx = arr->findByName(aName);
    //arr->getItem(idx, helper);
    //return helper;
    return arr->getNode(idx, helper);
  } else {
    throwNotContainer();
    return helper;
  }
}

/// Returns item value as a node ptr, helper can be used as a temporary object
/// Similar to getElement version, but faster when helper is not required.
/// Note: returned value can be only a shadow of existing node, so
//  to change it's contents you need to call setNode()
dnode *dnode::getNodePtr(int index, dnode &helper)
{
  if (isParent()) {
    return &(getChildrenPtr()->at(index));
  } else if (isArray()) {
    getArrayR()->getItem(index, helper);
    return &helper;
  } else {
    throwNotContainer();
    return &helper;
  }
}

/// read-only version of getNodePtr
const dnode *dnode::getNodePtrR(int index, dnode &helper) const
{
  if (isParent()) {
    return &(getChildrenPtrR()->at(index));
  } else if (isArray()) {
    getArrayR()->getItem(index, helper);
    return &helper;
  } else {
    throwNotContainer();
    return &helper;
  }
}

/// Returns item value as a node ptr, helper can be used as a temporary object
/// Similar to getElement version, but faster when helper is not required.
/// Note: returned value can be only a shadow of existing node, so
//  to change it's contents you need to call setNode()
dnode *dnode::getNodePtr(const dtpString &aName, dnode &helper)
{
  if (isParent()) {
    return &(*this)[aName];
  } else if (isArray()) {
    const dnArray *arr = getArrayR();
    size_type idx = arr->findByName(aName);
    arr->getItem(idx, helper);
    return &helper;
  } else {
    throwNotContainer();
    return &helper;
  }
}

/// read-only version of getNodePtr
const dnode *dnode::getNodePtrR(const dtpString &aName, dnode &helper) const
{
  if (isParent()) {
    return &(*this)[aName];
  } else if (isArray()) {
    const dnArray *arr = getArrayR();
    size_type idx = arr->findByName(aName);
    arr->getItem(idx, helper);
    return &helper;
  } else {
    throwNotContainer();
    return &helper;
  }
}

// update node, if &value == &target then do nothing
void dnode::setNode(int index, dnode &value)
{
  if (isParent()) {
    dnode *target = &(getChildrenPtr()->at(index));
    if (target != &value)
      target->copyFrom(value);
  } else if (isArray()) {
    getArray()->setItem(index, value);
  } else {
    throwNotContainer();
  }
}

// update node, if &value == &target then do nothing
void dnode::setNode(const dtpString &aName, dnode &value)
{
  if (isParent()) {
    dnode *target = &((*this)[aName]);
    if (target != &value)
      target->copyFrom(value);
  } else if (isArray()) {
    dnArray *arr = getArray();
    size_type idx = arr->findByName(aName);
    arr->setItem(idx, value);
  } else {
    throwNotContainer();
  }
}

dnode::dnValueBridge *dnode::newValueBridge(int idx)
{
    DTP_UNIQUE_PTR(dnValueBridge) res;

    if (isParent() || isList()) {
#ifndef DATANODE_POOL_BRIDGE
        res.reset(new dnValueBridgeForChildColn(this->getChildrenPtr()));
#else
        res.reset(ObjectPool<dnValueBridgeForChildColn>::newObject());
        static_cast<dnValueBridgeForChildColn *>(res.get())->setTarget(this->getChildrenPtr());
#endif
    } else if (isArray()) {
        res.reset(static_cast<dnArrayBase *>(this->getArray())->newValueBridge());
    }

    res->setPos(idx);

    return res.release();
}

dnode::dnValueBridge *dnode::newValueBridge(const char *name)
{
    if (isParent()) {
      return intNewValueBridgeForChildName(name);
    } else {
      size_type idx = indexOfName(name);
      if (idx == dnode::npos)
        idx = size();
      return newValueBridge(idx);
    }
}

dnode::dnValueBridge *dnode::newValueBridge(const dtpString &name)
{
    if (isParent()) {
      return intNewValueBridgeForChildName(name);
    } else {
      size_type idx = indexOfName(name);
      if (idx == dnode::npos)
        idx = size();
      return newValueBridge(idx);
    }
}

dnode::dnValueBridge *dnode::intNewValueBridgeForChildName(const dtpString &name)
{
    DTP_UNIQUE_PTR(dnValueBridgeForChildColnNames) res;

#ifndef DATANODE_POOL_BRIDGE
    res.reset(new dnValueBridgeForChildColnNames(this->getChildrenPtr()));
#else
    res.reset(ObjectPool<dnValueBridgeForChildColnNames>::newObject());
    static_cast<dnValueBridgeForChildColnNames *>(res.get())->setTarget(this->getChildrenPtr());
#endif

    res->setNamePos(name);

    return res.release();
}

dnode::dnValueBridge *dnode::newValueBridge(dnPos pos)
{
    DTP_UNIQUE_PTR(dnValueBridge) res;

    if (isParent() || isList())
    {
#ifndef DATANODE_POOL_BRIDGE
        res.reset(new dnValueBridgeForChildColn(this->getChildrenPtr()));
#else
        res.reset(ObjectPool<dnValueBridgeForChildColn>::newObject());
        static_cast<dnValueBridgeForChildColn *>(res.get())->setTarget(this->getChildrenPtr());
#endif
    } else if (isArray()) {
#ifndef DATANODE_POOL_BRIDGE
        res.reset(new dnValueBridgeForArray(this->getArray()));
#else
        res.reset(ObjectPool<dnValueBridgeForArray>::newObject());
        static_cast<dnValueBridgeForArray *>(res.get())->setTarget(this->getArray());
#endif
    }

    if (pos == dnpBegin)
        res->setPosBegin();
    else if (pos == dnpEnd)
        res->setPosEnd();
    else
        throw dnError("Unknown pos type");

    return res.release();
}

void dnode::deleteValueBridge(dnode::dnValueBridge *bridge)
{
#ifndef DATANODE_POOL_BRIDGE
  delete bridge;
#else
  bridge->release_to_pool();
#endif
}

// ----------------------------------------------------------------------------
// dnValue
// ----------------------------------------------------------------------------
/*
dnValue::dnValue(): m_valueType(vt_null)
{
}
*/

dnValue::dnValue( const dnValue& src): m_valueType(vt_null)
{
  doCopyFromAssign(src);
}

dnValue& dnValue::operator=( const dnValue& rhs)
{
  if (this != &rhs)
    doCopyFromAssign(rhs);
  return *this;
}

bool dnValue::operator!=(const dnValue &rhs)
{
  return !(*this == rhs);
}

bool dnValue::operator==(const dnValue &rhs) const
{
  if (this == &rhs)
    return true;
  if (m_valueType != rhs.getValueType())
    return false;
  if (m_valueData.which() != rhs.m_valueData.which())
    return false;

  if (Details::dnValueDynamicStrategyRegister<Details::dnValueCompareStrategyIntf>::at(m_valueType).canCompareDirectly())
    return (m_valueData == rhs.m_valueData);
  else
    return
       Details::dnValueDynamicCasterRegister::equals(m_valueType, m_valueData, rhs.m_valueData);
}

/// returns <true> of both values are equal
/// performs auto-conversion for comparison if required
bool dnValue::isEqualTo(const dnValue &value) const
{
  dtpStringGuard buffer;
  return isEqualTo(value, &buffer);
}

/// returns <true> of both values are equal.
/// performs auto-conversion for comparison if required,
/// strValue is used as string buffer for value when data types are different
bool dnValue::isEqualTo(const dnValue &value, void *buffer) const
{
  if (this == &value)
    return true;

  if (m_valueType != value.getValueType()) {
    assert(buffer != NULL);
    dtpStringGuard *strValue = static_cast<dtpStringGuard *>(buffer);
    if ((*strValue).get() == DTP_NULL)
      (*strValue).reset(new dtpString(value.getAs<dtpString>()));
    return (getAs<dtpString>() == *(*strValue));
  }

  //if (Details::dnValueCompareMetaScanner<vt_first>::canCompareDirectly(m_valueType))
  if (Details::dnValueDynamicStrategyRegister<Details::dnValueCompareStrategyIntf>::at(m_valueType).canCompareDirectly())
    return (m_valueData == value.m_valueData);
  else
    return
       Details::dnValueDynamicCasterRegister::equals(m_valueType, m_valueData, value.m_valueData);
}

//bool dnValue::empty() const
//{
//  bool res;
//
//  switch (getValueType()) {
//    case vt_array:
//      res = getAsArrayR()->empty();
//      break;
//    case vt_parent:
//      res = getAsChildrenNoCheckR()->empty();
//      break;
//    default:
//      res = true;
//  }
//
//  return res;
//}

void dnValue::copyFrom( const dnValue& src)
{
  if (this != &src)
    doCopyFromAssign(src);
}

// copy full contents of object as it would be full assign
void dnValue::doCopyFromAssign( const dnValue& src)
{
  setValueType(src.getValueType());

  switch (src.getValueType()) {
    case vt_string:
      setStringValue(src.getAs<dtpString>());
      break;
    case vt_array:
    case vt_parent:
      // this operation is not implemented, so
      // do nothing here, just copy data type
      // when source is non-empty, then operation is forbidden (no containers in STL vectors)
      throw dnError("Not implemented");
      break;
    default:
      m_valueData = src.m_valueData;
  }
}

// Copy contents of object. use only for scalar values
void dnValue::initScalarFrom(const dnValue& src)
{
  initValueType(src.getValueType());

  switch (src.getValueType()) {
    case vt_string:
      setStringValue(src.getAs<dtpString>());
      break;
    case vt_array:
    case vt_parent:
      // this operation does not copy structure contents
      throw dnError("Invalid source type");
      break;
    default:
      m_valueData = src.m_valueData;
  }
}

dnValue &dnValue::swap(dnValue &rhs)
{
  indirect_swap(m_valueData, rhs.m_valueData);
  indirect_swap(m_valueType, rhs.m_valueType);

  return *this;
}

inline void dnValue::initValueType(dnValueType aType)
{
  m_valueType = aType;
  m_valueData = (void *)(DTP_NULL);
}

inline void dnValue::initValueTypeNoDataInit(dnValueType aType)
{
  m_valueType = aType;
}

void dnValue::setValueType(dnValueType aType)
{
  if (m_valueType != aType) {
    releaseDataNoInit();
    m_valueType = aType;
    m_valueData = (void *)(DTP_NULL);
  }
}

/*
void dnValue::setValueTypeNoInit(dnValueType aType)
{
  if (m_valueType != aType) {
    if (m_valueType != vt_null)
    {
      releaseDataNoInit();
    }
    m_valueType = aType;
  }
}
*/

/*
void dnValue::releaseData()
{
  switch (m_valueType) {
  case vt_string:
  {
    dtpString *ptr = (dtpString *)(boost::get<void_ptr>(m_valueData));
    m_valueData = (void *)(DTP_NULL);
    delete ptr;
    break;
  }
  case vt_array:
  {
    dnArray *ptr = (dnArray *)(boost::get<void_ptr>(m_valueData));
    m_valueData = (void *)(DTP_NULL);
    delete ptr;
    break;
  }
  case vt_parent:
  {
    dnChildColnBase *ptr = (dnChildColnBase *)(boost::get<void_ptr>(m_valueData));
    m_valueData = (void *)(DTP_NULL);
    delete ptr;
    break;
  }
  default:
  // no nothing
    ;
  }
}
*/

/*
void dnValue::releaseDataNoInit()
{
  switch (m_valueType) {
  case vt_string:
  {
    dtpString *ptr = static_cast<dtpString *>(boost::get<void_ptr>(m_valueData));
    delete ptr;
    break;
  }
  case vt_array:
  {
    dnArray *ptr = static_cast<dnArray *>(boost::get<void_ptr>(m_valueData));
    delete ptr;
    break;
  }
  case vt_parent:
  {
    dnChildColnBase *ptr = static_cast<dnChildColnBase *>(boost::get<void_ptr>(m_valueData));
    delete ptr;
    break;
  }
  default:
  // no nothing
    ;
  }
}
*/

void dnValue::convertTo(dnValueType valueType)
{
  if (m_valueType == valueType)
    return;

  Details::dnValueDynamicCasterRegister::cast(m_valueType, m_valueData, valueType, m_valueData);
  m_valueType = valueType;

/*
  switch (valueType) {
    case vt_float:
      setAsFloat(getAsFloat());
      break;
    case vt_double:
      setAsDouble(getAsDouble());
      break;
    case vt_xdouble:
      setAsXDouble(getAsXDouble());
      break;
    case vt_bool:
      setAsBool(getAsBool());
      break;
    case vt_byte:
      setAsByte(getAsByte());
      break;
    case vt_int:
      setAsInt(getAsInt());
      break;
    case vt_uint:
      setAsUInt(getAsUInt());
      break;
    case vt_int64:
      setAsInt64(getAsInt64());
      break;
    case vt_uint64:
      setAsUInt64(getAsUInt64());
      break;
    case vt_null:
      setAsNull();
      break;
    case vt_vptr:
      setAsVoidPtr(getAsVoidPtr());
      break;
    case vt_string:
      setAsString(getAsString());
      break;
    case vt_date:
      setAsDate(getAsDate());
      break;
    case vt_time:
      setAsTime(getAsTime());
      break;
    case vt_datetime:
      setAsDateTime(getAsDateTime());
      break;
    case vt_parent:
    case vt_array:
      throw dnError(dtpString("Incorrect target value type: "+toString(valueType)));
    default:
      throw dnError(dtpString("Uknown value type: "+toString(valueType)));
  }
*/
}

/*
bool dnValue::dataEqualTo(const dnValue& src) const
{
  dnValue converted(src);
  converted.forceValueType(m_valueType);
  return isEqualTo(converted);
------------------------
-- to be removed:
------------------------
  bool res;
  switch (m_valueType) {
    case vt_bool:
      res = (getAsBool() == src.getAsBool());
      break;
    case vt_int:
      res = (getAsInt() == src.getAsInt());
      break;
    case vt_byte:
      res = (getAsByte() == src.getAsByte());
      break;
    case vt_uint:
      res = (getAsUInt() == src.getAsUInt());
      break;
    case vt_int64:
      res = (getAsInt64() == src.getAsInt64());
      break;
    case vt_uint64:
      res = (getAsUInt64() == src.getAsUInt64());
      break;
    case vt_float: {
      res = (getAsFloat() == src.getAsFloat());
      break;
    }
    case vt_double: {
      res = (getAsDouble() == src.getAsDouble());
      break;
    }
    case vt_xdouble: {
      res = (getAsXDouble() == src.getAsXDouble());
      break;
    }
    case vt_string:
      res = (getAsString() == src.getAsString());
      break;
    case vt_null:
      res = src.isNull();
      break;
    case vt_date:
    case vt_time:
    case vt_datetime:
      res = (getAsDateTime() == src.getAsDateTime());
      break;
    case vt_parent:
    case vt_array:
    case vt_vptr:
      res = false;
      break;
    default:
      throw dnError("Uknown value type");
  }
  return res;
------------------------
}
*/

// copy value, keep type as it was
void dnValue::assignFrom(const dnValue& src)
{
  Details::dnValueDynamicCasterRegister::cast(src.getValueType(), src.m_valueData, m_valueType, m_valueData);
/*
  switch (m_valueType) {
    case vt_bool:
      setAsBool(src.getAsBool());
      break;
    case vt_int:
      setAsInt(src.getAsInt());
      break;
    case vt_byte:
      setAsByte(src.getAsByte());
      break;
    case vt_uint:
      setAsUInt(src.getAsUInt());
      break;
    case vt_int64:
      setAsInt64(src.getAsInt64());
      break;
    case vt_uint64:
      setAsUInt64(src.getAsUInt64());
      break;
    case vt_float: {
      setAsFloat(src.getAsFloat());
      break;
    }
    case vt_double: {
      setAsDouble(src.getAsDouble());
      break;
    }
    case vt_xdouble: {
      setAsXDouble(src.getAsXDouble());
      break;
    }
    case vt_string:
      setAsString(src.getAsString());
      break;
    case vt_date:
      setAsDate(src.getAsDate());
      break;
    case vt_time:
      setAsTime(src.getAsTime());
      break;
    case vt_datetime:
      setAsDateTime(src.getAsDateTime());
      break;
    case vt_null:
    case vt_parent:
    case vt_array:
    case vt_vptr:
      // do nothing
      break;
    default:
      throw dnError("Uknown value type");
  }
*/
}

void dnValue::setAsNull()
{
  clear();
}

//void dnValue::initAsByte(int a_value)
//{
//  initValueTypeNoDataInit(vt_byte);
//  m_valueData = a_value;
//}

void dnValue::setAsByte(int a_value)
{
  setValueTypeNoInit(vt_byte);
  m_valueData = a_value;
}

//void dnValue::initAsInt(int a_value)
//{
//  initValueTypeNoDataInit(vt_int);
//  m_valueData = a_value;
//}

void dnValue::setAsInt(int a_value)
{
  setValueTypeNoInit(vt_int);
  m_valueData = a_value;
}

//void dnValue::initAsInt64(int64 a_value)
//{
//  initValueTypeNoDataInit(vt_int64);
//  m_valueData = a_value;
//}

void dnValue::setAsInt64(int64 a_value)
{
  setValueTypeNoInit(vt_int64);
  m_valueData = a_value;
}

//void dnValue::initAsUInt(uint a_value)
//{
//  initValueTypeNoDataInit(vt_uint);
//  m_valueData = a_value;
//}

void dnValue::setAsUInt(uint a_value)
{
  setValueTypeNoInit(vt_uint);
  m_valueData = a_value;
}

//void dnValue::initAsUInt64(uint64 a_value)
//{
//  initValueTypeNoDataInit(vt_uint64);
//  m_valueData = a_value;
//}

void dnValue::setAsUInt64(uint64 a_value)
{
  setValueTypeNoInit(vt_uint64);
  m_valueData = a_value;
}

inline void dnValue::setStringValue(const dtpString &a_value)
{
  dtpString *ptr = (dtpString *)(boost::get<void_ptr>(m_valueData));
  if (ptr == DTP_NULL) {
    m_valueData = (void *)(new dtpString(a_value));
  } else {
    *ptr = a_value;
  }
}

inline void dnValue::initStringValue(const dtpString &a_value)
{
  m_valueData = (void *)(new dtpString(a_value));
}

/*
void dnValue::initAsString(const dtpString &a_value)
{
  initValueTypeNoDataInit(vt_string);
  initStringValue(a_value);
}
*/

void dnValue::setAsString(const dtpString &a_value)
{
  setValueType(vt_string);
  setStringValue(a_value);
}

/*
void dnValue::initAsBool(bool a_value)
{
  initValueTypeNoDataInit(vt_bool);
  m_valueData = a_value;
}
*/

void dnValue::setAsBool(bool a_value)
{
  setValueTypeNoInit(vt_bool);
  m_valueData = a_value;
}

//void dnValue::initAsFloat(float a_value)
//{
//  initValueTypeNoDataInit(vt_float);
//  m_valueData = a_value;
//}

void dnValue::setAsFloat(float a_value)
{
  setValueTypeNoInit(vt_float);
  m_valueData = a_value;
}

//void dnValue::initAsDouble(double a_value)
//{
//  initValueTypeNoDataInit(vt_double);
//  m_valueData = a_value;
//}

void dnValue::setAsDouble(double a_value)
{
  setValueTypeNoInit(vt_double);
  m_valueData = a_value;
}

/*
void dnValue::initAsXDouble(xdouble a_value)
{
  initValueTypeNoDataInit(vt_xdouble);
  m_valueData = a_value;
}
*/

void dnValue::setAsXDouble(xdouble a_value)
{
  setValueTypeNoInit(vt_xdouble);
  m_valueData = a_value;
}

//void dnValue::initAsVoidPtr(void_ptr a_value)
//{
//  initValueTypeNoDataInit(vt_vptr);
//  m_valueData = a_value;
//}

void dnValue::setAsVoidPtr(void_ptr a_value)
{
  setValueTypeNoInit(vt_vptr);
  m_valueData = a_value;
}

//void dnValue::initAsDate(fdatetime_t a_value)
//{
//  initValueTypeNoDataInit(vt_date);
//  m_valueData = a_value;
//}

void dnValue::setAsDate(fdatetime_t a_value)
{
  setValueTypeNoInit(vt_date);
  m_valueData = a_value;
}

//void dnValue::initAsTime(fdatetime_t a_value)
//{
//  initValueTypeNoDataInit(vt_time);
//  m_valueData = a_value;
//}

void dnValue::setAsTime(fdatetime_t a_value)
{
  setValueTypeNoInit(vt_time);
  m_valueData = a_value;
}

//void dnValue::initAsDateTime(fdatetime_t a_value)
//{
//  initValueTypeNoDataInit(vt_datetime);
//  m_valueData = a_value;
//}

void dnValue::setAsDateTime(fdatetime_t a_value)
{
  setValueTypeNoInit(vt_datetime);
  m_valueData = a_value;
}


byte dnValue::getAsByte() const
{
  if (m_valueType == vt_byte)
    return static_cast<byte>(boost::get<int>(m_valueData));

  byte res;
  switch (m_valueType) {
    case vt_int:
      res = static_cast<byte>(getAs<int>());
      break;
    case vt_uint:
      res = static_cast<byte>(getAs<uint>());
      break;
    default:
      res = static_cast<byte>(stringToInt(getAs<dtpString>()));
      break;
  }
  return res;
}

int dnValue::getAsInt() const
{
  if (m_valueType == vt_int)
    return boost::get<int>(m_valueData);

  int res;
  switch (m_valueType) {
    case vt_uint:
      res = static_cast<int>(getAs<uint>());
      break;
    case vt_byte:
      res = static_cast<byte>(getAs<byte>());
      break;
    case vt_bool:
      res = getAs<bool>()?1:0;
      break;
    default:
      res = stringToInt(getAs<dtpString>());
      break;
  }
  return res;
}

uint dnValue::getAsUInt() const
{
  if (m_valueType == vt_uint)
    return boost::get<uint>(m_valueData);

  uint res;

  switch (m_valueType) {
    case vt_int:
      res = static_cast<uint>(getAs<int>());
      break;
    case vt_byte:
      res = getAs<byte>();
      break;
    case vt_bool:
      res = boost::get<bool>(m_valueData)?1:0;
      break;
    default:
      res = stringToUInt(getAs<dtpString>());
      break;
  }

  return res;
}

int64 dnValue::getAsInt64() const
{
  if (m_valueType == vt_int64)
    return boost::get<int64>(m_valueData);

  int64 res;
  switch (m_valueType) {
    case vt_int:
      res = getAs<int>();
      break;
    case vt_uint:
      res = getAs<uint>();
      break;
    case vt_byte:
      res = getAs<byte>();
      break;
    case vt_bool:
      res = getAs<bool>()?1:0;
      break;
    case vt_null:
      throw dnNullValueAccessError(vt_int64);
      break;
    default:
      res = stringToInt64(getAs<dtpString>());
      break;
  }
  return res;
}

uint64 dnValue::getAsUInt64() const
{
  if (m_valueType == vt_uint64)
    return boost::get<uint64>(m_valueData);

  uint64 res;
  switch (m_valueType) {
    case vt_int:
      res = getAs<int>();
      break;
    case vt_uint:
      res = getAs<uint>();
      break;
    case vt_byte:
      res = getAs<byte>();
      break;
    case vt_bool:
      res = getAs<bool>()?1:0;
      break;
    default:
      res = stringToUInt64(getAs<dtpString>());
      break;
  }
  return res;
}

const dtpString dnValue::getAsString() const
{
  switch (m_valueType) {
    case vt_string: {
      dtpString *ptr = (dtpString *)(boost::get<void_ptr>(m_valueData));
      return *ptr;
    }

    case vt_null:
    case vt_parent:
    case vt_array:
    case vt_vptr:
      return "";

    case vt_bool: {
      bool vbool = boost::get<bool>(m_valueData);
      return (vbool?dtpString("T"):dtpString("F"));
    }
    case vt_int:
    case vt_byte: {
      int vint = boost::get<int>(m_valueData);
      return toString(vint);
    }
    case vt_uint: {
      uint vuint = boost::get<uint>(m_valueData);
      return toString(vuint);
    }
    case vt_int64: {
      int64 vint64 = boost::get<int64>(m_valueData);
      return toString(vint64);
    }
    case vt_uint64: {
      uint64 vuint64 = boost::get<uint64>(m_valueData);
      return toString(vuint64);
    }
    case vt_date: {
      double vdouble = boost::get<double>(m_valueData);
      return dateToIsoStr(vdouble);
    }
    case vt_time: {
      double vdouble = boost::get<double>(m_valueData);
      return timeToIsoStr(vdouble);
    }
    case vt_datetime: {
      double vdouble = boost::get<double>(m_valueData);
      return dateTimeToIsoStr(vdouble);
    }
    case vt_float: {
      float vfloat = boost::get<float>(m_valueData);
      return toString(vfloat);
    }
    case vt_double: {
      double vdouble = boost::get<double>(m_valueData);
      return toString(vdouble);
    }
    case vt_xdouble: {
      xdouble vxdouble = boost::get<xdouble>(m_valueData);
      return toString(vxdouble);
    }
    default:
      throw dnError(dtpString("Uknown value type: "+toString(m_valueType)));
  }
}

bool dnValue::getAsBool() const
{
  bool res;

  if (m_valueType == vt_bool)
    res = (boost::get<bool>(m_valueData));
  else {
    dtpString strVal = getAs<dtpString>();
    if (strVal.empty()) {
      res = false;
    } else {
      char firstChar = strVal[0];
      res = ((firstChar == 'T') || (firstChar == 't') || (firstChar == '1'));
    }
  }
  return res;
}

float dnValue::getAsFloat() const
{
  float res;
  switch (m_valueType) {
    case vt_float:
      res = (boost::get<float>(m_valueData));
      break;
    case vt_double:
      res = static_cast<float>(getAs<double>());
      break;
    case vt_xdouble:
      res = static_cast<float>(getAs<xdouble>());
      break;
    case vt_int:
      res = static_cast<float>(getAs<int>());
      break;
    case vt_uint:
      res = static_cast<float>(getAs<uint>());
      break;
    case vt_int64:
      res = static_cast<float>(getAs<int64>());
      break;
    case vt_uint64:
      res = static_cast<float>(getAs<uint64>());
      break;
    case vt_string:
      res = stringToFloat(getAs<dtpString>());
      break;
    case vt_null:
      throw dnNullValueAccessError(vt_float);
      break;
    default:
      res = static_cast<float>(stringToInt64(getAs<dtpString>()));
  }
  return res;
}

double dnValue::getAsDouble() const
{
  if (m_valueType == vt_double)
    return boost::get<double>(m_valueData);

  double res;
  switch (m_valueType) {
    case vt_xdouble:
      res = static_cast<double>(getAs<xdouble>());
      break;
    case vt_float:
      res = static_cast<double>(getAs<float>());
      break;
    case vt_int:
      res = static_cast<double>(getAs<int>());
      break;
    case vt_uint:
      res = static_cast<double>(getAs<uint>());
      break;
    case vt_int64:
      res = static_cast<double>(getAs<int64>());
      break;
    case vt_uint64:
      res = static_cast<double>(getAs<uint64>());
      break;
    case vt_string:
      res = stringToDouble(getAs<dtpString>());
      break;
    case vt_null:
      throw dnNullValueAccessError(vt_double);
      break;
    default:
      res = static_cast<double>(stringToInt64(getAs<dtpString>()));
  }
  return res;
}

xdouble dnValue::getAsXDouble() const
{
  if (m_valueType == vt_xdouble)
    return boost::get<xdouble>(m_valueData);

  xdouble res;
  switch (m_valueType) {
    case vt_double:
      res = getAs<double>();
      break;
    case vt_float:
      res = getAs<float>();
      break;
    case vt_int:
      res = getAs<int>();
      break;
    case vt_uint:
      res = getAs<uint>();
      break;
    case vt_int64:
      res = static_cast<xdouble>(getAs<int64>());
      break;
    case vt_uint64:
      res = static_cast<xdouble>(getAs<uint64>());
      break;
    case vt_string:
      res = stringToXDouble(getAs<dtpString>());
      break;
    case vt_null:
      throw dnNullValueAccessError(vt_xdouble);
      break;
    default:
      res = static_cast<xdouble>(stringToInt64(getAs<dtpString>()));
  }
  return res;
}

void_ptr dnValue::getAsVoidPtr() const
{
  void_ptr res;
  if (m_valueType == vt_vptr)
    res = (boost::get<void_ptr>(m_valueData));
  else
    res = DTP_NULL;
  return res;
}

fdatetime_t dnValue::getAsDate() const
{
  if (m_valueType == vt_date)
    return static_cast<fdatetime_t>(boost::get<double>(m_valueData));

  double vdouble;
  fdatetime_t res;

  switch (m_valueType) {
    case vt_time:
    case vt_datetime:
      vdouble = boost::get<double>(m_valueData);
      res = dateTimeToDate(static_cast<fdatetime_t>(vdouble));
      break;
    case vt_int:
    case vt_uint:
    case vt_byte:
    case vt_float:
    case vt_double:
    case vt_xdouble:
    case vt_int64:
    case vt_uint64:
      vdouble = getAs<double>();
      res = dateTimeToDate(static_cast<fdatetime_t>(vdouble));
      break;
    default:
      res = dateTimeToDate(isoStrToDateTime(getAs<dtpString>()));
      break;
  }
  return res;
}

fdatetime_t dnValue::getAsTime() const
{
  if (m_valueType == vt_time)
    return static_cast<fdatetime_t>(boost::get<double>(m_valueData));

  double vdouble;
  fdatetime_t res;

  switch (m_valueType) {
    case vt_date:
    case vt_datetime:
      vdouble = boost::get<double>(m_valueData);
      res = dateTimeToTime(static_cast<fdatetime_t>(vdouble));
      break;
    case vt_int:
    case vt_uint:
    case vt_byte:
    case vt_float:
    case vt_double:
    case vt_xdouble:
    case vt_int64:
    case vt_uint64:
      vdouble = getAs<double>();
      res = dateTimeToTime(static_cast<fdatetime_t>(vdouble));
      break;
    default:
      res = dateTimeToTime(isoStrToDateTime(getAs<dtpString>()));
      break;
  }
  return res;
}

fdatetime_t dnValue::getAsDateTime() const
{
  if (m_valueType == vt_datetime)
    return static_cast<fdatetime_t>(boost::get<double>(m_valueData));

  double vdouble;
  fdatetime_t res;

  switch (m_valueType) {
    case vt_time:
    case vt_date:
      vdouble = boost::get<double>(m_valueData);
      res = static_cast<fdatetime_t>(vdouble);
      break;
    case vt_int:
    case vt_uint:
    case vt_byte:
    case vt_float:
    case vt_double:
    case vt_xdouble:
    case vt_int64:
    case vt_uint64:
      vdouble = getAs<double>();
      res = static_cast<fdatetime_t>(vdouble);
      break;
    default:
      res = isoStrToDateTime(getAs<dtpString>());
      break;
  }
  return res;
}

// ----------------------------------------------------------------------------
// dnArray
// ----------------------------------------------------------------------------
const dnArray::size_type dnArray::npos = static_cast<dnArray::size_type>(-1);

void dnArray::setItemValue(int index, const dnode &input)
{
  this->setItem(index, input);
}

void dnArray::eatItem(dnode &input)
{
  addItem(input);
}

const dnode &dnArray::getNode(int index, dnode &helper) const
{
  getItem(index, helper);
  return helper;
}

dnode *dnArray::getNodePtr(int index, dnode &helper) const
{
  getItem(index, helper);
  return &helper;
}

void dnArray::setNode(int index, const dnode &value)
{
  setItem(index, value);
}

// ----------------------------------------------------------------------------
// dnArrayBase
// ----------------------------------------------------------------------------
dnode::dnValueBridge *dnArrayBase::newValueBridge()
{
        dnValueBridgeForArray *bridge;
#ifndef DATANODE_POOL_BRIDGE
        bridge = new dnValueBridgeForArray(this);
#else
        bridge = ObjectPool<dnValueBridgeForArray>::newObject();
        try {
          bridge->setTarget(this);
        } catch (...) {
          operator delete(bridge);  // Deallocate the memory
          throw;               // Re-throw the exception
        }
#endif
        return bridge;

 }


namespace dtp {

namespace Details {

class dnCompareAsStr : public std::unary_function<dnode, bool> {
  public:
    dnCompareAsStr(const dtpString &a_key) : m_key(a_key) {}

    bool operator()(const dnode& obj) const {
        return obj.getAs<dtpString>() == m_key;
    }

  private:
    const dtpString &m_key;
};

} // namespace Details

} // namespace dtp

// ----------------------------------------------------------------------------
// dnArrayOfDataNode2
// ----------------------------------------------------------------------------

void dnArrayOfDataNode2::copyFrom(const dnArray *a_source)
{
  if (a_source->getValueType() != this->getValueType())
    throw dnError("Incorrect array source value for assign");
  const dnodeColn &srcItems = dynamic_cast<const dnArrayOfDataNode2 *>(a_source)->getItems();
  m_items.clear();
  m_items.reserve(a_source->size());
  for(dnodeColn::const_iterator it = srcItems.begin(), epos = srcItems.end(); it != epos; ++it)
    m_items.push_back(new dnode(*it));
}

bool dnArrayOfDataNode2::empty() const
{
  return m_items.empty();
}

void dnArrayOfDataNode2::getItem(int index, dnode &output) const
{
  output.copyFrom(m_items[index]);
}

void dnArrayOfDataNode2::setItem(int index, const dnode &input)
{
  m_items[index] = input;
}

void dnArrayOfDataNode2::setItemValue(int index, const dnode &input)
{
  m_items[index].copyValueFrom(input);
}

const dnode &dnArrayOfDataNode2::getNode(int index, dnode &helper) const
{
  return m_items[index];
}

dnode *dnArrayOfDataNode2::getNodePtr(int index, dnode &helper) const
{
  return &(const_cast<dnArrayOfDataNode2 *>(this)->m_items[index]);
}

void dnArrayOfDataNode2::setNode(int index, const dnode &value)
{
  if (&(m_items[index]) != &value)
    m_items[index] = value;
}

void dnArrayOfDataNode2::addItem(const dnValue &input)
{
  m_items.push_back(new dnode(input));
}

void dnArrayOfDataNode2::addItem(base::move_ptr<dnode> input)
{
  m_items.push_back(new dnode(input));
}

void dnArrayOfDataNode2::addItemAsNode(const dnode &input)
{
  m_items.push_back(new dnode(input));
}

void dnArrayOfDataNode2::addItemAsNode(dnode *input)
{
  m_items.push_back(input);
}

void dnArrayOfDataNode2::eatItem(dnode &input)
{
  m_items.push_back(new dnode);
  dnode &itemRef = m_items.back();
  itemRef.moveFrom(input);
}

void dnArrayOfDataNode2::eraseItem(int index)
{
  self_iterator itRemove = m_items.begin() + index;
  m_items.erase(itRemove);
}

void dnArrayOfDataNode2::eraseFrom(int index)
{
  self_iterator itRemove = m_items.begin() + index;
  m_items.erase(itRemove, m_items.end());
}

dnArray::size_type dnArrayOfDataNode2::indexOfValue(const dnode &input) const
{
  self_const_iterator it = std::find_if(
    m_items.begin(), m_items.end(),
    dnCompareAsStr(input.getAs<dtpString>()));

  size_type pos = it - m_items.begin();
  if (pos >= m_items.size())
    pos = npos;
  return pos;
}

dnArray::size_type dnArrayOfDataNode2::findByName(const dtpString &name) const
{
  size_type pos;
  pos = npos;
  return pos;
}

void dnArrayOfDataNode2::clear()
{
  m_items.clear();
}

dnArray::size_type dnArrayOfDataNode2::size() const
{
  return m_items.size();
}

void dnArrayOfDataNode2::resize(size_type newSize)
{
  if (m_items.size() < newSize)
  {
    size_type addCnt = newSize - m_items.size();
    while(addCnt > 0)
    {
      m_items.push_back(new dnode());
      addCnt--;
    }
  } else {
    m_items.resize(newSize);
  }
}

dnode::dnValueBridge *dnArrayOfDataNode2::newValueBridge()
{
        dnValueBridgeForArrayOfDataNode *bridge;
#ifndef DATANODE_POOL_BRIDGE
        bridge = new dnValueBridgeForArrayOfDataNode(this);
#else
        bridge = ObjectPool<dnValueBridgeForArrayOfDataNode>::newObject();
        try {
          bridge->setTarget(this);
        } catch (...) {
          operator delete(bridge);  // Deallocate the memory
          throw;               // Re-throw the exception
        }
#endif
        return bridge;
}

// ----------------------------------------------------------------------------
// dnChildColnBase
// ----------------------------------------------------------------------------
dnChildColnBase::dnChildColnBase()
{
}

void dnChildColnBase::copyFrom(const dnChildColnBase& src)
{
  dnChildTransporter transp;

  if (this != &src)
  {
    copyItemsFrom(src);
  }
}

void dnChildColnBase::clear()
{
  clearItems();
}

dnode &dnChildColnBase::getByName(const dtpString &name)
{
  size_type idx = indexOfName(name);
  if (idx == dnode::npos)
    throw dnError("Child ["+name+"] not found");

  return at(idx);
}

const dnode &dnChildColnBase::getByNameR(const dtpString &name) const 
{
  size_type idx = indexOfName(name);
  if (idx == dnode::npos)
    throw dnError("Child ["+name+"] not found");

  return at(idx);
}

dnode *dnChildColnBase::peekChild(const dtpString &name)
{
  size_type idx = indexOfName(name);
  if (idx != dnode::npos)
    return &(at(idx));
  else
    return NULL;
}

const dnode *dnChildColnBase::peekChildR(const dtpString &name) const
{
  size_type idx = indexOfName(name);
  if (idx != dnode::npos)
    return &(at(idx));
  else
    return NULL;
}

// ----------------------------------------------------------------------------
// dnChildColnList
// ----------------------------------------------------------------------------
dnChildColnList::dnChildColnList(): dnChildColnBase()
{
}

void dnChildColnList::copyItemsFrom(const dnChildColnBase& src)
{
  DTP_UNIQUE_PTR(dnode) transp;

  if (this != &src)
  {
     clearItems();
     m_items.reserve(src.size());

     for(size_type i=0,epos=src.size(); i != epos; i++) {
       transp.reset(createChild(src.at(i)));
       m_items.push_back(transp.release());
     }
  }
}

void dnChildColnList::clearItems()
{
  m_items.clear();
}

dnChildColnBase::size_type dnChildColnList::size() const
{
  return m_items.size();
}

void dnChildColnList::resize(size_type newSize)
{
  if (m_items.size() < newSize)
  {
    size_type addCnt = newSize - m_items.size();
    while(addCnt > 0)
    {
      m_items.push_back(new dnode());
      addCnt--;
    }
  } else {
    m_items.resize(newSize);
  }
}

bool dnChildColnList::empty() const
{
  return m_items.empty();
}

void dnChildColnList::erase(const dtpString &name)
{
  size_type idx = indexOfName(name);

  if (idx != dnode::npos)
  {
    m_items.erase(m_items.begin() + idx);
  }
}

void dnChildColnList::erase(int index)
{
  m_items.erase(m_items.begin() + index);
}

void dnChildColnList::eraseFrom(int index)
{
  m_items.erase(m_items.begin() + index, m_items.end());
}

void dnChildColnList::insert(dnode *node)
{
  m_items.push_back(node);
}

void dnChildColnList::insert(dnChildColnBase::size_type pos, dnode *node)
{
  m_items.insert(m_items.begin() + pos, node);
}

void dnChildColnList::insert(size_type pos, const dtpString &name, dnode *node)
{
  m_items.insert(m_items.begin() + pos, node);
}

void dnChildColnList::insert(const dtpString &name, dnode *node)
{
  m_items.push_back(node);
}

const dtpString dnChildColnList::getName(int index) const
{
  return dtpString("");
}

void dnChildColnList::setName(int index, const dtpString &name)
{ // do nothing
}

void dnChildColnList::setAt(int pos, dnode *node)
{
  m_items.replace(pos, node);
}

dnode *dnChildColnList::extractChild(int index)
{
  dnodeColn::auto_type item = m_items.release( m_items.begin() + index );
  return item.release();
}

dnode *dnChildColnList::cloneChild(int index) const
{
  return createChild(m_items[index]);
}

dnChildColnBase::size_type dnChildColnList::indexOfName(const dtpString &name) const
{
  return dnode::npos;
}

dnChildColnBase::size_type dnChildColnList::indexOfValue(const dnode &value) const
{
  dtpStringGuard keyValue;

  for(size_type i=0, epos = size(); i!=epos; i++) {
    if (m_items[i].isEqualTo(value, &keyValue))
      return i;
  }

  return dnode::npos;
}

/*
dnode &dnChildColnList::at(int pos)
{
  return m_items[pos];
}
*/

void dnChildColnList::getChild(int index, dnode &output)
{
  output = m_items[index];
}

bool dnChildColnList::hasChild(const dtpString &name) const
{
  return (indexOfName(name) != dnode::npos);
}

// ----------------------------------------------------------------------------
// dnChildColnDblMap
// ----------------------------------------------------------------------------
dnChildColnDblMap::dnChildColnDblMap(): dnChildColnBase()
{
}

dnChildColnDblMap::~dnChildColnDblMap()
{
#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
  for(dnChildColnIndexMap::iterator it = m_map2.begin(), epos = m_map2.end(); it != epos; ++it)
    delete *it;
#endif
}

void dnChildColnDblMap::copyItemsFrom(const dnChildColnBase& src)
{
  if (this != &src)
  {
    dnGuard transp;
    boost::shared_ptr<dnGuard> stransp;

    dnChildColnBase *ptr = &const_cast<dnChildColnBase&>(src);
    dnChildColnDblMap *nsrcPtr = dynamic_cast<dnChildColnDblMap *>(ptr);

    clearItems();

    int id = 0;
    dtpString name;

    if (nsrcPtr) {
       for(size_type i=0,epos=src.size(); i != epos; i++) {
         name = src.getName(i);
         transp.reset(createChild(const_cast<dnChildColnBase&>(src).at(i)));

         if (name.empty())
           name = toString(id);

         m_map1.insert(std::make_pair(name, transp.get()));

         m_map2.push_back(transp.release());
         m_names.push_back(name);
         ++id;
       } // for
    } else {
       for(size_type i=0,epos=src.size(); i != epos; i++) {
         name = toString(id);
         transp.reset(createChild(const_cast<dnChildColnBase&>(src).at(i)));
         m_map1.insert(std::make_pair(name, transp.get()));
         m_map2.push_back(transp.release());
         m_names.push_back(name);
         ++id;
       }
    }
  } // if this != src
} // fun

void dnChildColnDblMap::clearItems()
{
  dnChildColnDblMap tmp;
  swap(tmp);
}

dnChildColnBase::size_type dnChildColnDblMap::size() const
{
  return m_map2.size();
}

void dnChildColnDblMap::resize(size_type newSize)
{
  if (m_map2.size() < newSize)
  {
    size_type addCnt = newSize - m_map2.size();
    while(addCnt > 0)
    {
      insert(new dnode());
      addCnt--;
    }
  } else {
    while(m_map2.size() > newSize)
      erase(m_names.back());
  }
}

bool dnChildColnDblMap::empty() const
{
  return m_map1.empty();
}

void dnChildColnDblMap::erase(const dtpString &name)
{
  dnChildColnNameMap::iterator namePos = m_map1.find(name);
  size_type idx = indexOfName(name);

  if (idx != dnode::npos)
  {
    dnChildColnIndexMap::iterator idxPos = m_map2.begin() + idx;
    if (idxPos != m_map2.end()) {
#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
      dnode *ptr = m_map2[idx];
      m_map2.erase(idxPos);
      delete ptr;
#else
      m_map2.erase(idxPos);
#endif
    }
    m_names.erase(m_names.begin() + idx);
  }
  if (namePos != m_map1.end())
    m_map1.erase(namePos);
}

void dnChildColnDblMap::erase(int index)
{
  eraseItem(index);
}

void dnChildColnDblMap::eraseItem(int index)
{
  dtpString name = m_names[index];

  dnChildColnNameMap::iterator namePos = m_map1.find(name);

#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
  dnode *ptr = m_map2[index];
  m_map2.erase(m_map2.begin() + index);
  delete ptr;
#else
  m_map2.erase(m_map2.begin() + index);
#endif
  m_names.erase(m_names.begin() + index);

  if (namePos != m_map1.end())
    m_map1.erase(namePos);
}

void dnChildColnDblMap::eraseFrom(int index)
{
  size_type atPos = static_cast<size_type>(index);
  size_type endPos = m_map2.size();

  while(atPos < endPos)
  {
    eraseItem(index); // always the same pos
    atPos++;
  }
}

dnChildColnDblMap *dnChildColnDblMap::newEmpty()
{
   return new dnChildColnDblMap();
}

void dnChildColnDblMap::insert(dnode *node)
{
  size_type id = size();
  this->insert(toString(id), node);
}

void dnChildColnDblMap::insert(dnChildColnBase::size_type pos, dnode *node)
{
  this->insert(pos, toString(size()), node);
}

void dnChildColnDblMap::insert(dnChildColnBase::size_type pos, const dtpString &name, dnode *node)
{
  //dnGuard ptr(node);
  //m_map1.insert(std::make_pair(name, ptr.get()));
  try {
    m_map1.insert(std::make_pair(name, node));
  } catch (...) {
    delete node;
    throw;
  }
  //m_map2.insert(m_map2.begin() + pos, ptr.release());
  m_map2.insert(m_map2.begin() + pos, node);
  m_names.insert(m_names.begin() + pos, name);
}

void dnChildColnDblMap::insert(const dtpString &name, dnode *node)
{
  //dnGuard ptr(node);
  //m_map1.insert(std::make_pair(name, ptr.get()));

  try {
    m_map1.insert(std::make_pair(name, node));
  } catch (...) {
    delete node;
    throw;
  }

  //m_map2.push_back(ptr.release());
  m_map2.push_back(node);
  m_names.push_back(name);
}

const dtpString dnChildColnDblMap::getName(int index) const
{
  return m_names[index];
}

void dnChildColnDblMap::setName(int index, const dtpString &name)
{
  dnChildColnIndexMap::const_iterator idxPos = m_map2.begin() + index;
  dtpString oldName;
  dnode *obj;

  assert(idxPos != m_map2.end());

  if (idxPos != m_map2.end())
  {
    oldName = m_names[index];
  }

  dnChildColnNameMap::iterator namePos;
  namePos = m_map1.find(oldName);
  if (namePos != m_map1.end())
  {
    obj = namePos->second;
    m_map1.erase(namePos);
  } else {
    obj = NULL;
  }

  m_map1.insert(std::make_pair(name, obj));
  m_names[index] = name;
}

void dnChildColnDblMap::setAt(int pos, dnode *node)
{
#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
  dnChildColnNameMap::iterator namePos = m_map1.find(m_names[pos]);
  namePos->second = node;

  delete m_map2[pos];
  m_map2[pos] = node;
#else
  DTP_UNIQUE_PTR(dnode) ptr(node);

  dnChildColnIndexMap::iterator idxPos = m_map2.begin() + pos;
  dtpString name;
  if (idxPos != m_map2.end())
    name = m_names[pos];

  dnChildColnNameMap::iterator namePos;
  namePos = m_map1.find(name);

  namePos->second = node;
  *idxPos = node;
#endif
}

dnode *dnChildColnDblMap::extractChild(int index)
{
  dnChildColnNameMap::iterator namePos;

  dtpString name = m_names[index];

  namePos = m_map1.find(name);
  if (namePos == m_map1.end())
    namePos = m_map1.find(toString(index));

#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
  dnode *item;
  if (namePos != m_map1.end())
  {
    item = namePos->second;
    m_map1.erase( namePos );
  } else {
    item = m_map2[index];
  }
  m_map2.erase(m_map2.begin() + index);
  m_names.erase(m_names.begin() + index);
  return item;
#else
  if (namePos != m_map1.end())
    m_map1.erase(namePos);

  dnChildColnIndexMap::iterator itemPos = m_map2.begin() + index;
  dnodeColn::auto_type item = m_map2.release( itemPos );
  m_names.erase(m_names.begin() + index);
  return item.release();
#endif
}

dnode *dnChildColnDblMap::cloneChild(int index) const
{
#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
  return createChild(*m_map2[index]);
#else
  return createChild(m_map2[index]);
#endif
}

dnChildColnBase::size_type dnChildColnDblMap::indexOfName(const dtpString &name) const
{
#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
  dnChildColnNameMap::const_iterator namePos = m_map1.find(name);
  if (namePos == m_map1.end())
    return dnode::npos;
  dnode *ptr = namePos->second;
  dnChildColnIndexMap::const_iterator it = std::find(m_map2.begin(), m_map2.end(), ptr);
  return (it - m_map2.begin());
#else
  for(size_type i=0, epos = m_names.size(); i != epos; i++) {
      if (m_names[i] == name)
      return i;
  }
  return dnode::npos;
#endif
}

dnChildColnBase::size_type dnChildColnDblMap::indexOfValue(const dnode &value) const
{
  dtpStringGuard keyValue;

  for(size_type i=0, epos = m_map2.size(); i != epos; i++) {
#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
    if (m_map2[i]->isEqualTo(value, &keyValue))
#else
    if (m_map2[i].isEqualTo(value, &keyValue))
#endif
      return i;
  }

  return dnode::npos;
}

/*
dnode &dnChildColnDblMap::at(int pos)
{
  return m_map2[pos];
}
*/

void dnChildColnDblMap::getChild(int index, dnode &output)
{
#ifdef DATANODE_CHILD_INDEX_VECTOR_STD
  output = *m_map2[index];
#else
  output = m_map2[index];
#endif
}

dnode &dnChildColnDblMap::getByName(const dtpString &name)
{
  dnChildColnNameMap::iterator it = m_map1.find(name);
  if (it == m_map1.end())
    throw dnError("Child ["+name+"] not found");

  return *(it->second);
}

const dnode &dnChildColnDblMap::getByNameR(const dtpString &name) const
{
  dnChildColnNameMap::const_iterator it = m_map1.find(name);
  if (it == m_map1.end())
    throw dnError("Child ["+name+"] not found");

  return *(it->second);
}

dnode *dnChildColnDblMap::peekChild(const dtpString &name)
{
  dnChildColnNameMap::iterator it = m_map1.find(name);
  if (it != m_map1.end())
    return &(*(it->second));
  else
    return NULL;
}

const dnode *dnChildColnDblMap::peekChildR(const dtpString &name) const
{
  dnChildColnNameMap::const_iterator it = m_map1.find(name);
  if (it != m_map1.end())
    return &(*(it->second));
  else
    return NULL;
}

bool dnChildColnDblMap::hasChild(const dtpString &name) const
{
  return m_map1.find(name) != m_map1.end();
}

void dnChildColnDblMap::swap(dnChildColnDblMap &rhs)
{
  indirect_swap(this->m_map1, rhs.m_map1);
  indirect_swap(this->m_map2, rhs.m_map2);
  indirect_swap(this->m_names, rhs.m_names);
}

void swap(dnChildColnDblMap &lhs, dnChildColnDblMap &rhs)
{
  lhs.swap(rhs);
}

// ----------------------------------------------------------------------------
// dnValueBridge
// ----------------------------------------------------------------------------
dnode::dnValueBridge& dnode::dnValueBridge::operator=(const dnode& rhs)
{
    setAsItem(rhs);
    return *this;
}

dnode::dnValueBridge& dnode::dnValueBridge::operator=(const dnValue& rhs)
{
    setAsItem(rhs);
    return *this;
}

/*
dnode::dnValueBridge& dnode::dnValueBridge::operator=(const dtpString& rhs)
{
    setAsString(rhs);
    return *this;
}
*/

dnode::dnValueBridge& dnode::dnValueBridge::operator=(const dnode_vector& rhs)
{
    dnode temp;
    temp = rhs;
    setAsItem(temp);
    return *this;
}

/*
dnode::dnValueBridge &dnode::dnValueBridge::operator=(float value)
{
    setAsFloat(value);
    return *this;
}

dnode::dnValueBridge &dnode::dnValueBridge::operator=(double value)
{
    setAsDouble(value);
    return *this;
}
*/

/*
dnode::dnValueBridge &dnode::dnValueBridge::operator=(xdouble value)
{
    setAsXDouble(value);
    return *this;
}
*/

/*
dnode::dnValueBridge &dnode::dnValueBridge::operator=(void_ptr value)
{
    setAsVoidPtr(value);
    return *this;
}

dnode::dnValueBridge &dnode::dnValueBridge::operator=(bool value)
{
    setAsBool(value);
    return *this;
}

dnode::dnValueBridge &dnode::dnValueBridge::operator=(int value)
{
    setAsInt(value);
    return *this;
}

dnode::dnValueBridge &dnode::dnValueBridge::operator=(byte value)
{
    setAsByte(value);
    return *this;
}

dnode::dnValueBridge &dnode::dnValueBridge::operator=(uint value)
{
    setAsUInt(value);
    return *this;
}

dnode::dnValueBridge &dnode::dnValueBridge::operator=(int64 value)
{
    setAsInt64(value);
    return *this;
}

dnode::dnValueBridge &dnode::dnValueBridge::operator=(uint64 value)
{
    setAsUInt64(value);
    return *this;
}
*/

bool dnode::dnValueBridge::operator==(const dnValueBridge &rhs) const
{
    dnode temp1, temp2;
    getAsItem(temp1);
    rhs.getAsItem(temp2);
    return (temp1 == temp2);
}

float dnode::dnValueBridge::getAsFloat() const
{
    dnode temp;
    getAsItem(temp);
    return temp.getAs<float>();
}

double dnode::dnValueBridge::getAsDouble() const
{
    dnode temp;
    return getAsNode(temp).getAs<double>();
}

xdouble dnode::dnValueBridge::getAsXDouble() const
{
    dnode temp;
    getAsItem(temp);
    return temp.getAs<xdouble>();
}

void_ptr dnode::dnValueBridge::getAsVoidPtr() const
{
    dnode temp;
    getAsItem(temp);
    return temp.getAs<void_ptr>();
}

bool dnode::dnValueBridge::getAsBool() const
{
    dnode temp;
    getAsItem(temp);
    return temp.getAs<bool>();
}

int dnode::dnValueBridge::getAsInt() const
{
    dnode temp;
    return getAsNode(temp).getAs<int>();
}

byte dnode::dnValueBridge::getAsByte() const
{
    dnode temp;
    getAsItem(temp);
    return temp.getAs<byte>();
}

uint dnode::dnValueBridge::getAsUInt() const
{
    dnode temp;
    getAsItem(temp);
    return temp.getAs<uint>();
}

int64 dnode::dnValueBridge::getAsInt64() const
{
    dnode temp;
    getAsItem(temp);
    return temp.getAs<int64>();
}

uint64 dnode::dnValueBridge::getAsUInt64() const
{
    dnode temp;
    getAsItem(temp);
    return temp.getAs<uint64>();
}

dtpString dnode::dnValueBridge::getAsString() const
{
    dnode temp;
    getAsItem(temp);
    return temp.getAs<dtpString>();
}

void dnode::dnValueBridge::setAsFloat(float value)
{
    dnode temp(value);
    setAsItem(temp);
}

void dnode::dnValueBridge::setAsDouble(double value)
{
    dnode temp(value);
    setAsItem(temp);
}

void dnode::dnValueBridge::setAsXDouble(xdouble value)
{
    dnode temp(value);
    setAsItem(temp);
}

void dnode::dnValueBridge::setAsVoidPtr(void_ptr value)
{
    dnode temp(value);
    setAsItem(temp);
}

void dnode::dnValueBridge::setAsBool(bool value)
{
    dnode temp(value);
    setAsItem(temp);
}

void dnode::dnValueBridge::setAsInt(int value)
{
    dnode temp(value);
    setAsItem(temp);
}

void dnode::dnValueBridge::setAsByte(byte value)
{
    dnode temp(value);
    setAsItem(temp);
}

void dnode::dnValueBridge::setAsUInt(uint value)
{
    dnode temp(value);
    setAsItem(temp);
}

void dnode::dnValueBridge::setAsInt64(int64 value)
{
    dnode temp(value);
    setAsItem(temp);
}

void dnode::dnValueBridge::setAsUInt64(uint64 value)
{
    dnode temp(value);
    setAsItem(temp);
}

void dnode::dnValueBridge::setAsString(const dtpString &value)
{
    dnode temp(value);
    setAsItem(temp);
}

bool dnode::dnValueBridge::isNull()
{
    dnode temp;
    getAsItem(temp);
    return temp.isNull();
}

void dnode::dnValueBridge::setAsNull()
{
    dnode temp;
    //getAsItem(temp); // to read name & other props
    //temp.clear();
    setAsItem(temp);
}

const dnValue dnode::dnValueBridge::getRValue() const
{
    dnValue res;
    getAsItem(res);
    return res;
}

const dnode dnode::dnValueBridge::getAsElement() const
{
    dnode res;
    getAsItem(res);
    return res;
}

/// Returns node reference, can be used with any container thanks to helper.
/// Use returned reference. Helper can be used as temporary object.
const dnode &dnode::dnValueBridge::getAsNode(dnode &helper) const
{
    if (supportsRefs())
        return const_cast<dnValueBridge *>(this)->getAsNodeRef();
    else {
        getAsItem(helper);
        return helper;
    }
}

dnode *dnode::dnValueBridge::getAsNodePtr(dnode &helper)
{
    if (supportsRefs())
        return &(const_cast<dnValueBridge *>(this)->getAsNodeRef());
    else {
        getAsItem(helper);
        return &helper;
    }
}

dnode &dnode::dnValueBridge::getAsNodeRef()
{
    throw dnError("Not implemented");
}

/// Returns <true> if value can be obtained as a reference to node
bool dnode::dnValueBridge::supportsRefs() const
{
    return false;
}

/// Returns <true> if value can be obtained as a reference to node
bool dnode::dnValueBridge::supportsNames() const
{
    return false;
}

/// Equal to setItem, added just for API consistency.
void dnode::dnValueBridge::setAsNode(const dnode &value)
{
    setAsItem(value);
}

void dnode::setAsParent(dnChildColnBase *value)
{
  releaseData();
  setValueTypeNoInit(vt_parent);
  m_valueData = value;
}

void dnode::setAsArray(dnArray *value)
{
  releaseData();
  setValueTypeNoInit(vt_array);
  m_valueData = value;
}

dnChildColnBase *dnode::getAsChildren()
{
  if (m_valueType == vt_parent) {
    dnChildColnBase *ptr = (dnChildColnBase *)(boost::get<void_ptr>(m_valueData));
    return ptr;
  } else {
    return DTP_NULL;
  }
}

const dnChildColnBase *dnode::getAsChildrenR() const
{
  if (m_valueType == vt_parent) {
    dnChildColnBase *ptr = static_cast<dnChildColnBase *>(boost::get<void_ptr>(m_valueData));
    return ptr;
  } else {
    return DTP_NULL;
  }
}

dnChildColnBase *dnode::extractChildren()
{
  DTP_UNIQUE_PTR(dnChildColnBase) resGuard(getAsChildren());
  m_valueData = (void *)(DTP_NULL);
  setValueType(vt_null);
  return resGuard.release();
}

dnArray *dnode::extractArray()
{
  DTP_UNIQUE_PTR(dnArray) resGuard(getAsArray());
  m_valueData = (void *)(DTP_NULL);
  setValueType(vt_null);
  return resGuard.release();
}


// ----------------------------------------------------------------------------
// dnValueBridgeIndexed
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// dnValueBridgeForArray
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// dnValueBridgeForChildColn
// ----------------------------------------------------------------------------

void swap(dnValue& lhs, dnValue& rhs) {
  lhs.swap(rhs);
}

void swap(dnode& lhs, dnode& rhs) {
  lhs.swap(rhs);
}

namespace dtp {

dtpString getValueTypeName(dnValueType accessType)
{
  switch(accessType) {
#ifdef DATANODE_ADD_RTTI
  case vt_null:
    return "null";
  case vt_parent:
    return "parent";
  case vt_array:
    return "array";
  case vt_byte:
    return "byte";
  case vt_int:
    return "int";
  case vt_uint:
    return "uint";
  case vt_int64:
    return "int64";
  case vt_uint64:
    return "uint64";
  case vt_string:
    return "string";
  case vt_bool:
    return "bool";
  case vt_float:
    return "float";
  case vt_double:
    return "double";
  case vt_xdouble:
    return "xdouble";
  case vt_vptr:
    return "vptr";
  case vt_date:
    return "date";
  case vt_time:
    return "time";
  case vt_datetime:
    return "datime";
#endif
  default:
    return "?";
  }
}

};
