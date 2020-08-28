/////////////////////////////////////////////////////////////////////////////
// Name:        dnode_serializer.h
// Project:     dtpLib
// Purpose:     Serializer for dtp::DataNode - JSON format
// Author:      Piotr Likus
// Modified by:
// Created:     10/05/2009
/////////////////////////////////////////////////////////////////////////////

#ifndef _DATANODESERIALIZERJSON_H__
#define _DATANODESERIALIZERJSON_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file dnode_serializer.h
///
/// JSON parser/writer for dnode

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
// base
#include "base/serializer.h"
#include "base/string.h"

//dtp
#include "dtp/dnode.h"

namespace dtp
{

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
class dnSerializer: public dtp::dtpSerializerIntf<dtp::dnode> {
public:
  // construction
  dnSerializer();
  virtual ~dnSerializer() {};
  // properties
  void setCommentsEnabled(bool value);
  bool getCommentsEnabled();
  // run
  virtual int convToString(const dtp::dnode& input, dtpString &output);
  virtual int convFromString(const dtpString &input, dtp::dnode& output);
protected:
  bool m_commentsEnabled;
};

} // namespace

#endif // _DATANODESERIALIZERJSON_H__
