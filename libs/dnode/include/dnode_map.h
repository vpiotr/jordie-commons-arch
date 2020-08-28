/////////////////////////////////////////////////////////////////////////////
// Name:        dnode_map.h
// Project:     dtpLib
// Purpose:     Map based on data node
// Author:      Piotr Likus
// Modified by:
// Created:     27/06/2012
/////////////////////////////////////////////////////////////////////////////

#ifndef _DTPDNODEMAP_H__
#define _DTPDNODEMAP_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file dnode_map.h
\brief Map handling functions based on dnode object.

Can be used to put maps inside dnode objects or to store it externally.
For string key "parent" node is used.
For other keys pair of value vectors is used (see dmap_init).

Function list:
- dmap_init - initialize dmap object
- dmap_get, dmap_get_def - read item value
- dmap_set
- dmap_insert
- dmap_push_back
- dmap_sort
- dmap_find
- dmap_erase
- dmap_lower_bound
- dmap_upper_bound
- dmap_has_key
- dmap_key, dmap_value: read item's key & value from iterator
- dmap_is_sorted - check if sorted
- dmap_accumulate
- dmap_visit
- dmap_size
- dmap_empty
- dmap_for_each

TODO:
- dmap-unique, dmap-unique-copy
- dmap-get-key-list
- dmap-get-value-list
- dmap-set( new arg: new_required = false )
- dmap-merge(input1, input2, output)

*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "dtp/details/dtypes.h"
#include "dtp/dnode.h"
#include "dtp/dnode_algorithm.h"
#include "dtp/details/sort.h"

namespace dtp {

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------
#define DMAP_SORT_SUPPORTED

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
// Function definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// dmap_xxx = map<T, dnode> stored inside dnode
// ----------------------------------------------------------------------------
const uint _DMAP_KEY_OFFSET = 0;
const uint _DMAP_VALUE_OFFSET = 1;

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, void>::type
    dmap_init(dtp::dnode &node)
{
  using namespace Details;
  typedef dnArrayImplMeta<dnArrayMetaIsDefined<KeyType>::value, KeyType>  array_impl_meta_key;
  typedef dnArrayImplMeta<dnArrayMetaIsDefined<ValueType>::value, ValueType>  array_impl_meta_value;

  node.clear();
  if (array_impl_meta_key::item_type != vt_null)
    node.addChild(new dtp::dnode(ict_array, dnValueType(array_impl_meta_key::item_type)));
  else
    node.addChild(new dtp::dnode(ict_array, vt_datanode));

  if (array_impl_meta_value::item_type != vt_null)
    node.addChild(new dtp::dnode(ict_array, dnValueType(array_impl_meta_value::item_type)));
  else
    node.addChild(new dtp::dnode(ict_array, vt_datanode));
}

template<typename KeyType, typename ValueType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, void>::type
    dmap_init(dtp::dnode &node)
{
  node.clear();
  node.setAsParent();
}

template<typename KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::size_type>:: type
    dmap_size(const dtp::dnode &node)
{
  return const_cast<dtp::dnode &>(node)[_DMAP_KEY_OFFSET].size();
}

template<typename KeyType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::size_type>:: type
    dmap_size(const dtp::dnode &node)
{
  return node.size();
}

template<typename KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, bool>:: type
    dmap_empty(const dtp::dnode &node)
{
  return const_cast<dtp::dnode &>(node)[_DMAP_KEY_OFFSET].empty();
}

template<typename KeyType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, bool>:: type
    dmap_empty(const dtp::dnode &node)
{
  return node.empty();
}

template<typename KeyType>
KeyType dmap_key(dtp::dnode::iterator it)
{
  return it->getAs<KeyType>();
}

template<typename KeyType>
KeyType dmap_key(dtp::dnode::const_iterator it)
{
  return it->getAs<KeyType>();
}

namespace Details {

template<typename KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, KeyType>::type
    dmap_key(const dtp::dnode &node, dtp::dnode::size_type index)
{
  return node[_DMAP_KEY_OFFSET].get<KeyType>(index);
}

template<typename KeyType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, KeyType>::type
    dmap_key(const dtp::dnode &node, dtp::dnode::size_type index)
{
  return node[index].getAs<KeyType>();
}

} // namespace Details

template<typename KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::iterator>::type
    dmap_begin(dtp::dnode &node)
{
  return node[_DMAP_KEY_OFFSET].begin();
}

template<typename KeyType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::iterator>::type
    dmap_begin(dtp::dnode &node)
{
  return node.begin();
}

template<typename KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::iterator>::type
    dmap_end(dtp::dnode &node)
{
  return node[_DMAP_KEY_OFFSET].end();
}

template<typename KeyType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::iterator>::type
    dmap_end(dtp::dnode &node)
{
  return node.end();
}

/// Use when you don't need key (faster)
//template<typename ValueType>
//dtp::dnode::scalar_iterator<ValueType> dmap_begin_value(const dtp::dnode &node)
//{
//  return const_cast<dtp::dnode &>(node)[_DMAP_VALUE_OFFSET].scalarBegin<ValueType>();
//}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::scalar_iterator<ValueType> >::type
    dmap_begin_value(dtp::dnode &node)
{
  return node[_DMAP_VALUE_OFFSET].scalarBegin<ValueType>();
}

template<typename KeyType, typename ValueType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::scalar_iterator<ValueType> >::type
    dmap_begin_value(dtp::dnode &node)
{
  return node.scalarBegin<ValueType>();
}

/// Use when you don't need key (faster)
//template<typename T>
//dtp::dnode::scalar_iterator<T> dmap_end_value(const dtp::dnode &node)
//{
//  return const_cast<dtp::dnode &>(node)[_DMAP_VALUE_OFFSET].scalarEnd<T>();
//}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::scalar_iterator<ValueType> >::type
    dmap_end_value(dtp::dnode &node)
{
  return node[_DMAP_VALUE_OFFSET].scalarEnd<ValueType>();
}

template<typename KeyType, typename ValueType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::scalar_iterator<ValueType> >::type
    dmap_end_value(dtp::dnode &node)
{
  return node.scalarEnd<ValueType>();
}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, ValueType>::type
    dmap_value(const dtp::dnode &node, dtp::dnode::iterator it)
{
  dtp::dnode::size_type offset = it - dmap_begin<KeyType>(const_cast<dtp::dnode &>(node));
  return node[_DMAP_VALUE_OFFSET].get<ValueType>(offset);
}

template<typename KeyType, typename ValueType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, ValueType>::type
    dmap_value(const dtp::dnode &node, dtp::dnode::iterator it)
{
  return (*it).getAs<ValueType>();
}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, ValueType>::type
    dmap_value(const dtp::dnode &node, dtp::dnode::const_iterator it)
{
  dtp::dnode::size_type offset = it - dmap_begin<KeyType>(const_cast<dtp::dnode &>(node));
  return node[_DMAP_VALUE_OFFSET].get<ValueType>(offset);
}

template<typename KeyType, typename ValueType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, ValueType>::type
    dmap_value(const dtp::dnode &node, dtp::dnode::const_iterator it)
{
  return (*it).getAs<ValueType>();
}

namespace Details {

/// Find index of value.
/// \return dnode::npos if not found
template <class KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::size_type>::type
    dmap_find_index(dtp::dnode &node, const KeyType& key)
{
  dtp::dnode::size_type epos = dmap_size<KeyType>(node);
  dtp::dnode::size_type res = node[_DMAP_KEY_OFFSET].lower_bound_index(key);
  if ((res == epos) || (Details::dmap_key<KeyType>(node, res) != key))
    return dtp::dnode::npos;
  else
    return res;
}

template <class KeyType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::size_type>::type
    dmap_find_index(dtp::dnode &node, const KeyType& key)
{
  return node.indexOfName(key);
}

} // namespace Details

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsObject<KeyType>, ValueType>::type
    dmap_get_def(const dtp::dnode &node, KeyType key, const ValueType &defValue)
{
  dtp::dnode::size_type pos = Details::dmap_find_index(const_cast<dtp::dnode &>(node), key);
  if (pos != dtp::dnode::npos)
    return node[_DMAP_VALUE_OFFSET].get<ValueType>(pos);
  else
    return defValue;
}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>,
    typename dtpEnableIf<Details::dnValueMetaIsObject<KeyType>, ValueType>::type>::type
      dmap_get_def(const dtp::dnode &node, const KeyType &key, const ValueType &defValue)
{
  dtp::dnode::size_type pos = Details::dmap_find_index(const_cast<dtp::dnode &>(node), key);
  if (pos != dtp::dnode::npos)
    return node[_DMAP_VALUE_OFFSET].get<ValueType>(pos);
  else
    return defValue;
}

template<typename KeyType, typename ValueType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, ValueType>::type
    dmap_get_def(const dtp::dnode &node, const KeyType &key, const ValueType &defValue)
{
  return node.get(key, defValue);
}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsObject<KeyType>, bool>::type
    dmap_get(const dtp::dnode &node, KeyType key, ValueType &outValue)
{
  dtp::dnode::size_type pos = Details::dmap_find_index(const_cast<dtp::dnode &>(node), key);
  if (pos != dtp::dnode::npos) {
    outValue = node[_DMAP_VALUE_OFFSET].get<ValueType>(pos);
    return true;
  }
  else {
    return false;
  }
}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>,
    typename dtpEnableIf<Details::dnValueMetaIsObject<KeyType>, bool>::type>::type
      dmap_get(const dtp::dnode &node, const KeyType &key, ValueType &outValue)
{
  dtp::dnode::size_type pos = Details::dmap_find_index(const_cast<dtp::dnode &>(node), key);
  if (pos != dtp::dnode::npos) {
    outValue = node[_DMAP_VALUE_OFFSET].get<ValueType>(pos);
    return true;
  }
  else {
    return false;
  }
}

template<typename KeyType, typename ValueType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, bool>::type
    dmap_get(const dtp::dnode &node, const KeyType &key, ValueType &outValue)
{
  dtp::dnode::size_type pos = node.indexOfName(key);

  if (pos != dtp::dnode::npos) {
    outValue = node[pos].getAs<ValueType>();
    return true;
  }
  else {
    return false;
  }
}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, ValueType>::type
    dmap_get(const dtp::dnode &node, const dtp::dnode::iterator &it)
{
  return dmap_value<KeyType, ValueType>(node, it);
}

template<typename KeyType, typename ValueType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, ValueType>::type
    dmap_get(const dtp::dnode &node, const dtp::dnode::iterator &it)
{
  return (*it).getAs<ValueType>();
}

/// Returns all values associated with a given key (use when duplicates are allowed)
template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode &>::type
    dmap_get_all(const dtp::dnode &node, KeyType key, dtp::dnode &output)
{
  output = dtp::dnode(ict_array, vt_datanode);
  dtp::dnode::iterator it = dmap_find(const_cast<dtp::dnode &>(node), key);
  dtp::dnode::iterator epos = dmap_end<KeyType>(const_cast<dtp::dnode &>(node));

  if (it != epos)
  {
    do {
      output.push_back(dmap_value<KeyType, ValueType>(node, it));
      ++it;
    } while((it != epos) && (dmap_key<KeyType>(it) == key));
  }

  return output;
}

template<typename KeyType, typename ValueType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode &>::type
    dmap_get_all(const dtp::dnode &node, KeyType key, dtp::dnode &output)
{
  output = dtp::dnode(ict_array, vt_datanode);

  dtp::dnode::iterator it = node.find(key);
  dtp::dnode::iterator epos = node.end();
  dnode helper;

  if (it != epos)
    output.push_back((*it).getAsNode(helper));

  return output;
}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsObject<KeyType>, void>::type
    dmap_set(dtp::dnode &node, KeyType key, const ValueType &value)
{
  dtp::dnode::size_type i = Details::dmap_find_index(node, key);
  if (i != dtp::dnode::npos) {
    node[_DMAP_VALUE_OFFSET].set<ValueType>(i, value);
  } else {
    dmap_insert(node, key, value);
  }
}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>,
    typename dtpEnableIf<Details::dnValueMetaIsObject<KeyType>, void>::type>::type
      dmap_set(dtp::dnode &node, const KeyType &key, const ValueType &value)
{
  dtp::dnode::size_type i = Details::dmap_find_index(node, key);
  if (i != dtp::dnode::npos) {
    node[_DMAP_VALUE_OFFSET].set<ValueType>(i, value);
  } else {
    dmap_insert(node, key, value);
  }
}

template<typename KeyType, typename ValueType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, void>::type
    dmap_set(dtp::dnode &node, const KeyType &key, const ValueType &value)
{
  if (node.hasChild(key)) {
    node[key].setAs(value);
  } else {
    node.addChild(key, value);
  }
}

template<typename KeyType>
   typename dtpDisableIf<Details::dnValueMetaIsObject<KeyType>, bool>::type
     dmap_has_key(dtp::dnode &node, KeyType key)
{
  dtp::dnode::size_type i = Details::dmap_find_index(node, key);
  return (i != dtp::dnode::npos);
}

template<typename KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>,
    typename dtpEnableIf<Details::dnValueMetaIsObject<KeyType>, bool>::type>::type
      dmap_has_key(dtp::dnode &node, const KeyType &key)
{
  dtp::dnode::size_type i = Details::dmap_find_index(node, key);
  return (i != dtp::dnode::npos);
}

template<typename KeyType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, bool>::type
    dmap_has_key(dtp::dnode &node, const KeyType &key)
{
  return node.hasChild(key);
}

namespace Details {

template <class KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::size_type>::type
    dmap_lower_bound_index(dtp::dnode &node, const KeyType& value)
{
  return node[_DMAP_KEY_OFFSET].lower_bound_index(value);
}

template <class KeyType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::size_type>::type
    dmap_lower_bound_index(dtp::dnode &node, const KeyType& key)
{
  dtp::dnode::size_type pos = node.indexOfName(key);
  if (pos != dtp::dnode::npos)
    return pos;
  else
    return 0;
}

} // namespace Details

template <class KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::iterator>::type
    dmap_lower_bound(dtp::dnode &node, const KeyType& key)
{
  return node[_DMAP_KEY_OFFSET].lower_bound(key);
}

template <class KeyType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::iterator>::type
    dmap_lower_bound(dtp::dnode &node, const KeyType& key)
{
  dtp::dnode::size_type pos = node.indexOfName(key);
  if (pos != dtp::dnode::npos)
    return node.begin()+pos;
  else
    return node.begin();
}

template <class KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::iterator>::type
    dmap_upper_bound(dtp::dnode &node, const KeyType& key)
{
  return node[_DMAP_KEY_OFFSET].upper_bound(key);
}

template <class KeyType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::iterator>::type
    dmap_upper_bound(dtp::dnode &node, const KeyType& key)
{
  dtp::dnode::size_type pos = node.indexOfName(key);
  if (pos != dtp::dnode::npos)
    return node.begin() + (pos + 1);
  else
    return node.end();
}

namespace Details {

template <class KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::size_type >::type
    dmap_upper_bound_index(dtp::dnode &node, const KeyType& key)
{
  return node[_DMAP_KEY_OFFSET].upper_bound_index(key);
}

template <class KeyType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::size_type >::type
    dmap_upper_bound_index(dtp::dnode &node, const KeyType& key)
{
  dtp::dnode::size_type pos = node.indexOfName(key);
  if (pos != dtp::dnode::npos)
    return (pos + 1);
  else
    return node.size();
}

} // namespace Details


template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, void>::type
    dmap_set(dtp::dnode &node, KeyType key, const dtp::dnode &value)
{
  dtp::dnode::size_type i = Details::dmap_find_index(node, key);
  if (i != dtp::dnode::npos) {
    node[_DMAP_VALUE_OFFSET].set<ValueType>(i, value.getAs<ValueType>());
  } else {
    dmap_insert(node, key, value);
  }
}

template<typename KeyType, typename ValueType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, void>::type
    dmap_set(dtp::dnode &node, const KeyType &key, const dtp::dnode &value)
{
  dtp::dnode::size_type i = node.indexOfName(key);
  if (i != dtp::dnode::npos) {
    node[i].setAs<ValueType>(value.getAs<ValueType>());
  } else {
    node.addChild(key, value);
  }
}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, void>::type
    dmap_set(dtp::dnode &node, const dtp::dnode::iterator &it, const ValueType &value)
{
  dtp::dnode::size_type offset = it - dmap_begin<KeyType>(node);
  node[_DMAP_VALUE_OFFSET].set<ValueType>(offset, value);
}

template<typename KeyType, typename ValueType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, void>::type
    dmap_set(dtp::dnode &node, const dtp::dnode::iterator &it, const ValueType &value)
{
  (*it).setAs(value);
}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, void>::type
    dmap_set(dtp::dnode &node, const dtp::dnode::iterator &it, const dtp::dnode &value)
{
  dtp::dnode::size_type offset = it - dmap_begin<KeyType>(node);
  node[_DMAP_VALUE_OFFSET].set<ValueType>(offset, value.getAs<ValueType>());
}

template<typename KeyType, typename ValueType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, void>::type
    dmap_set(dtp::dnode &node, const dtp::dnode::iterator &it, const dtp::dnode &value)
{
  (*it).setAsNode(value);
}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsObject<KeyType>, void>::type
    dmap_insert(dtp::dnode &node, KeyType key, const ValueType &value, bool allowDuplicates = false)
{
  dtp::dnode::size_type it;
  dtp::dnode::size_type epos(dmap_size<KeyType>(node));
  bool foundDup = false;

  if (!allowDuplicates)
  {
    it = Details::dmap_lower_bound_index(node, key);
    if ((it < epos) && (Details::dmap_key<KeyType>(node, it) == key))
      foundDup = true;
  } else {
    it = Details::dmap_upper_bound_index(node, key);
  }

  if (!foundDup)
  {
    node[_DMAP_KEY_OFFSET].insertAtPos(it, key);
    node[_DMAP_VALUE_OFFSET].insertAtPos(it, value);
  } else {
      throw dnError("Item already exists: "+toString(key));
  }
}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>,
    typename dtpEnableIf<Details::dnValueMetaIsObject<KeyType>, void>::type>::type
      dmap_insert(dtp::dnode &node, const KeyType &key, const ValueType &value, bool allowDuplicates = false)
{
  dtp::dnode::size_type it;
  dtp::dnode::size_type epos(dmap_size<KeyType>(node));
  bool foundDup = false;

  if (!allowDuplicates)
  {
    it = Details::dmap_lower_bound_index(node, key);
    if ((it < epos) && (Details::dmap_key<KeyType>(node, it) == key))
    //throw dnError("Item already exists: "+toString(key));
      foundDup = true;
  } else {
    it = Details::dmap_upper_bound_index(node, key);
  }

  if (!foundDup)
  {
    node[_DMAP_KEY_OFFSET].insertAtPos(it, key);
    node[_DMAP_VALUE_OFFSET].insertAtPos(it, value);
  } else {
      throw dnError("Item already exists: "+toString(key));
  }
}

template<typename KeyType, typename ValueType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, void>::type
    dmap_insert(dtp::dnode &node, const KeyType &key, const ValueType &value, bool allowDuplicates = false)
{
  bool foundDup = false;

  if (!allowDuplicates)
  {
    foundDup = node.hasChild(key);
  }

  if (!foundDup) {
    node.addChild(key, value);
  } else {
      throw dnError("Item already exists: "+key);
  }

  //else
  //  node[key].setAs(value);
}

template<typename KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, void>::type
    dmap_insert(dtp::dnode &node, KeyType key, dtp::dnode *value, bool allowDuplicates = false)
{
  dtp::dnode::iterator it;
  dtp::dnode::size_type offset;

  if (!allowDuplicates)
  {
    it = dmap_lower_bound(node, key);
    if (dmap_key<KeyType>(it) != key)
    {
      offset = it - dmap_begin<KeyType>(node);
    } else {
      std::auto_ptr<dtp::dnode> nodeGuard(value);
      throw dnError("Item already exists: "+toString(key));
    }
  } else {
    it = dmap_upper_bound(node, key);
    offset = it - dmap_begin<KeyType>(node);
  }

  node[_DMAP_KEY_OFFSET].insert(node[_DMAP_KEY_OFFSET].begin() + offset, key);
  node[_DMAP_VALUE_OFFSET].insert(node[_DMAP_VALUE_OFFSET].begin() + offset, value);
}

template<typename KeyType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, void>::type
    dmap_insert(const dtp::dnode &node, KeyType key, dtp::dnode *value, bool allowDuplicates = false)
{
  bool found;

  if (!allowDuplicates)
  {
    found = node.hasChild(key);
    if (found)
      throw dnError("Item already exists: "+key);
  } else {
    found = false; // when duplicates allowed - always add
  }

  if (!found)
    node.addChild(key, value);
  //else {
  //  std::unique_ptr<dtp::dnode> valuePtr(value);
  //  node[key].setAs(*value);
  //}
}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsObject<KeyType>, void>::type
    dmap_push_back(dtp::dnode &node, KeyType key, const ValueType &value)
{
  node[_DMAP_KEY_OFFSET].push_back(key);
  node[_DMAP_VALUE_OFFSET].push_back(value);
}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>,
    typename dtpEnableIf<Details::dnValueMetaIsObject<KeyType>, void>::type>::type
      dmap_push_back(dtp::dnode &node, const KeyType &key, const ValueType &value)
{
  node[_DMAP_KEY_OFFSET].push_back(key);
  node[_DMAP_VALUE_OFFSET].push_back(value);
}

template<typename KeyType, typename ValueType>
    typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, void>::type
      dmap_push_back(dtp::dnode &node, const KeyType &key, const ValueType &value)
{
  node.push_back(key, value);
}

template<typename KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, void>::type
    dmap_push_back(dtp::dnode &node, KeyType key, dtp::dnode *value)
{
  node[_DMAP_KEY_OFFSET].push_back(key);
  node[_DMAP_VALUE_OFFSET].push_back(value);
}

template<typename KeyType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, void>::type
    dmap_push_back(dtp::dnode &node, const KeyType &key, dtp::dnode *value)
{
  node.addChild(key, value);
}

template <class KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::iterator>::type
    dmap_find(dtp::dnode::iterator first, dtp::dnode::iterator last, const KeyType& key)
{
  dtp::dnode::iterator res = lower_bound(first, last, key);
  if ((res == last) || (dmap_key<KeyType>(res) != key))
    return last;
  else
    return res;
}

template <class KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::iterator>::type
    dmap_find(dtp::dnode &node, const KeyType& key)
{
  dtp::dnode::iterator epos = dmap_end<KeyType>(node);
  dtp::dnode::iterator res = node[_DMAP_KEY_OFFSET].lower_bound(key);
  if ((res == epos) || (dmap_key<KeyType>(res) != key))
    return epos;
  else
    return res;
}

template <class KeyType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::iterator>::type
    dmap_find(dtp::dnode &node, const KeyType& key)
{
  //dtp::dnode::size_type pos = node.indexOfName(key);
  //dtp::dnode::size_type epos = dnode::npos;

  //if (pos != dnode::npos)
  //  return node.begin()+pos;
  //else
  //  return node.end();
  return node.at(key);
}

/// version with key & value type provided should be faster - uses scalar iterator
template <class KeyType, class ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::scalar_iterator<ValueType> >::type
    dmap_find_value(dtp::dnode &node, const KeyType& key)
{
  typedef dtp::dnode::scalar_iterator<ValueType> res_type;
  dtp::dnode::size_type epos(dmap_size<KeyType>(node));
  dtp::dnode::size_type idx = node[_DMAP_KEY_OFFSET].lower_bound_index(key);
  if ((idx < epos) && (node[_DMAP_KEY_OFFSET].get<KeyType>(idx) == key))
    return node.scalarAt<ValueType>(idx);
  else
    return node.scalarEnd<ValueType>();
}

template <class KeyType, class ValueType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, dtp::dnode::scalar_iterator<ValueType> >::type
    dmap_find_value(dtp::dnode &node, const KeyType& key)
{
  //dtp::dnode::size_type pos = node.indexOfName(key);

  //if (pos != dnode::npos)
  //  return node.scalarAt<ValueType>(pos);
  //else
  //  return node.scalarEnd<ValueType>();
  return node.scalarAt<ValueType>(key);
}

template <class KeyType>
void dmap_erase(dtp::dnode &node, const KeyType& key)
{
  dtp::dnode::iterator it = dmap_find(node, key);
  dmap_erase<KeyType>(node, it);
}

template <class KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, void>::type
    dmap_erase(dtp::dnode &node, dtp::dnode::iterator it)
{
  if (it != dmap_end<KeyType>(node))
  {
    dtp::dnode::size_type offset;
    offset = it - dmap_begin<KeyType>(node);
    node[_DMAP_KEY_OFFSET].erase(node[_DMAP_KEY_OFFSET].begin() + offset);
    node[_DMAP_VALUE_OFFSET].erase(node[_DMAP_VALUE_OFFSET].begin() + offset);
  }
}

template <class KeyType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, void>::type
    dmap_erase(dtp::dnode &node, dtp::dnode::iterator it)
{
  node.erase(it);
}

template<typename KeyType>
bool dmap_is_sorted(const dtp::dnode &node)
{
  bool res = true;
  dtp::dnode::const_iterator itprior = node.begin();
  for(dtp::dnode::const_iterator it = itprior + 1, epos = node.end(); it != epos; ++it, ++itprior) {
    if (dmap_key<KeyType>(itprior) > dmap_key<KeyType>(it))
    {
      res = false;
      break;
    }
  }
  return res;
}

template<typename KeyType, typename ValueType, typename Visitor>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, Visitor>::type
    dmap_visit(const dtp::dnode &node, Visitor visitor)
{
  dtp::dnode helper;
  const dtp::dnode *keyNodePtr;
  const dtp::dnode *valueNodePtr;
  KeyType key;
  ValueType value;

  keyNodePtr = &(node[_DMAP_KEY_OFFSET]);
  valueNodePtr = &(node[_DMAP_VALUE_OFFSET]);

  for(uint i = 0, epos = dmap_size<KeyType>(node); i != epos; i++)
  {
    key = (*keyNodePtr).get<KeyType>(i);
    value = (*valueNodePtr).get<ValueType>(i);
    visitor(key, value);
  }

  return visitor;
}

template<typename KeyType, typename ValueType, typename Visitor>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, Visitor>::type
    dmap_visit(const dtp::dnode &node, Visitor visitor)
{
  KeyType key;
  ValueType value;

  for(uint i = 0, epos = node.size(); i != epos; i++)
  {
    key = node.getElementName(i);
    value = node.get<ValueType>(i);
    visitor(key, value);
  }

  return visitor;
}

template<typename KeyType, typename ValueType, typename Visitor>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, Visitor>::type
    dmap_visit_values(const dtp::dnode &node, Visitor visitor)
{
  const dtp::dnode *valueNodePtr;
  ValueType value;

  valueNodePtr = &(node[_DMAP_VALUE_OFFSET]);

  for(dtp::dnode::size_type i = 0, epos = dmap_size<KeyType>(node); i != epos; i++)
  {
    visitor((*valueNodePtr).get<ValueType>(i));
  }

  return visitor;
}

template<typename KeyType, typename ValueType, typename Visitor>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, Visitor>::type
    dmap_visit_values(const dtp::dnode &node, Visitor visitor)
{
  for(dtp::dnode::size_type i = 0, epos = node.size(); i != epos; i++)
  {
    visitor(node.get<ValueType>(i));
  }

  return visitor;
}

template<typename KeyType, typename Visitor>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, Visitor>::type
    dmap_visit_keys(const dtp::dnode &node, Visitor visitor)
{
  const dtp::dnode *keyNodePtr;
  KeyType key;

  keyNodePtr = &(node[_DMAP_KEY_OFFSET]);

  for(uint i = 0, epos = dmap_size<KeyType>(node); i != epos; i++)
  {
    key = (*keyNodePtr).get<KeyType>(i);
    visitor(key);
  }

  return visitor;
}

template<typename KeyType, typename Visitor>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, Visitor>::type
    dmap_visit_keys(const dtp::dnode &node, Visitor visitor)
{
  for(uint i = 0, epos = node.size(); i != epos; i++)
  {
    visitor(node.getElementName(i));
  }

  return visitor;
}

template<typename KeyType, typename ValueType, typename TransOp, typename OutputIterator>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, OutputIterator >::type
    dmap_transform(dtp::dnode &node, OutputIterator result, TransOp transOp)
{
  dtp::dnode *valueNodePtr = &(node[_DMAP_VALUE_OFFSET]);
  return valueNodePtr->transform<ValueType>(result, transOp);
}

template<typename KeyType, typename ValueType, typename TransOp, typename OutputIterator>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, OutputIterator >::type
    dmap_transform(dtp::dnode &node, OutputIterator result, TransOp transOp)
{
  return node.transform<ValueType>(result, transOp);
}

template<typename KeyType, typename ValueType, typename TransOp>
  dtp::dnode::scalar_iterator<ValueType>
    dmap_transform_values(dtp::dnode &node, TransOp transOp)
{
  dtp::dnode::scalar_iterator<ValueType> it, epos;
  for(it = dmap_begin_value<KeyType, ValueType>(node), epos = dmap_end_value<KeyType, ValueType>(node); it != epos; ++it)
  {
        *it = transOp(*it);
  }
  return it;
}

template<typename KeyType, typename ValueType, class Visitor>
Visitor dmap_for_each(dtp::dnode &node, dtp::dnode::iterator first, dtp::dnode::iterator last, Visitor f)
{
    for (; first != last; ++first) {
        f(dmap_key<KeyType>(first), dmap_value<KeyType, ValueType>(node, first));
    }
    return f;
}

template<typename KeyType, typename ValueType, class Visitor>
Visitor dmap_for_each(const dtp::dnode &node, Visitor f)
{
  return dmap_visit<KeyType, ValueType>(node, f);
}

/// Used for value scanning
template<typename KeyType, typename ValueType, class Visitor>
Visitor dmap_for_each_value(const dtp::dnode &node, Visitor f)
{
  for(dtp::dnode::scalar_iterator<ValueType> it = dmap_begin_value<KeyType, ValueType>(node), epos = dmap_end_value<KeyType, ValueType>(node); it != epos; ++it)
  {
        f(*it);
  }
  return f;
}

namespace Details {

template<typename KeyType>
struct MapKeyCompareOp {
  int operator()(const KeyType &val1, const KeyType &val2)
  {
    if (val1 < val2)
      return -1;
    else if (val1 > val2)
      return 1;
    else
      return 0;
  }
};

template<typename KeyType, typename CompareOp>
class DMapSortTool {
  public:
    typedef KeyType value_type;

    DMapSortTool(dtp::dnode &node, CompareOp compOp):
       m_keys(static_cast<dnArrayBase *>(node[_DMAP_KEY_OFFSET].getArray())),
       m_values(static_cast<dnArrayBase *>(node[_DMAP_VALUE_OFFSET].getArray())),
       m_compareOp(compOp) {}

    KeyType get(int pos)
    {
      return m_keys->get<KeyType>(pos);
    }

    void swap(size_t pos1, size_t pos2)
    {
      m_keys->swapItems(pos1, pos2);
      m_values->swapItems(pos1, pos2);
    }

    int compare(size_t pos, const KeyType &keyValue)
    {
      return m_compareOp(m_keys->get<KeyType>(pos), keyValue);
    }

  protected:
    dnArrayBase *m_keys;
    dnArrayBase *m_values;
    CompareOp m_compareOp;
};

template<typename CompareOp>
class DMapSortTool<dtpString, CompareOp> {
  public:
    typedef dtpString value_type;
    typedef dtpString KeyType;

    DMapSortTool(dtp::dnode &node, CompareOp compOp):
       m_node(&node),
       m_compareOp(compOp) {}

    KeyType get(int pos)
    {
      return m_node->getElementName(pos);
    }

    void swap(size_t pos1, size_t pos2)
    {
      m_node->swap(pos1, pos2);
    }

    int compare(size_t pos, const KeyType &keyValue)
    {
      return m_compareOp(m_node->getElementName(pos), keyValue);
    }

  protected:
    dnode *m_node;
    CompareOp m_compareOp;
};

} // namespace Details

template<typename KeyType, typename CompareOp>
void dmap_sort(dtp::dnode &node, CompareOp compareOp)
{
     size_t n = dmap_size<KeyType>(node);
     if (n > 1)
     {
        Details::DMapSortTool<KeyType, CompareOp> tool(node, compareOp);
        dtp::sort(n, tool);
     }
}

template<typename KeyType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, void >::type
    dmap_sort(dtp::dnode &node)
{
     size_t n = dmap_size<KeyType>(node);
     if (n > 1)
     {
        typedef Details::MapKeyCompareOp<KeyType> CompareOp;
        CompareOp compOp;
        Details::DMapSortTool<KeyType, CompareOp> tool(node, compOp);
        dtp::sort(n, tool);
     }
}

template<typename KeyType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, void >::type
    dmap_sort(dtp::dnode &node)
{
  // empty - not required for scanning dmap in case of string key
}

template<typename KeyType, typename ValueType>
  typename dtpDisableIf<Details::dnValueMetaIsString<KeyType>, ValueType>::type
    dmap_accumulate(const dtp::dnode &node, const ValueType &init)
{
  return node[_DMAP_VALUE_OFFSET].accumulate(init);
}

template<typename KeyType, typename ValueType>
  typename dtpEnableIf<Details::dnValueMetaIsString<KeyType>, ValueType>::type
    dmap_accumulate(const dtp::dnode &node, const ValueType &init)
{
  return node.accumulate(init);
}

} // namespace

#endif // _DTPDNODEMAP_H__
