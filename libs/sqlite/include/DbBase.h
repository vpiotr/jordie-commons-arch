/////////////////////////////////////////////////////////////////////////////
// Name:        DbBase.h
// Project:     scLib
// Purpose:     SQL database abstraction layer. 
// Author:      Piotr Likus
// Modified by:
// Created:     20/12/2008
/////////////////////////////////////////////////////////////////////////////

#ifndef _DBBASE_H__
#define _DBBASE_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file DbBase.h
///
/// Abstraction layer
/// scDbBase - database class
/// scDbCursor - query result class
///
/// Bind params:
/// bind by name using format "select * from table where a = {param}"
/// bind by pos using format "select * from table where a = ?"

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "sc/dtypes.h"

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------
enum scDbObjType {
    dotUndef,
    dotTable,
    dotView,
    dotProcedure,
    dotIndex
};

// ----------------------------------------------------------------------------
// Forward class definitions
// ----------------------------------------------------------------------------
class scDbCursor;

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
// support domains
const unsigned int SUPD_BASE = 0;
// support flags
const unsigned int SUPF_BIND_BY_POS = 1; // bind params by position
const unsigned int SUPF_BIND_BY_NAME = 2; // bind params by name

// ----------------------------------------------------------------------------
// Class definitions
// ----------------------------------------------------------------------------
typedef boost::shared_ptr<scDbCursor> scDbCursorTransporter;

// ----------------------------------------------------------------------------
// scDbBase
// ----------------------------------------------------------------------------
class scDbBase {
public:
  typedef scDbCursorTransporter cursor_transporter;
  scDbBase();
  virtual ~scDbBase();
  virtual scString getVersion() const = 0;
  virtual bool isSupported(unsigned int flags, unsigned int domain = 0);
  virtual void connect(const scString &params) = 0;
  virtual void disconnect() = 0;
  virtual bool isConnected() const = 0;
  virtual void selectContext(const scString &ctx) = 0;
  virtual ulong64 execute(const scString &sql, const scDataNode *params = SC_NULL) = 0;
  virtual scDbCursorTransporter select(const scString &sql, const scDataNode *params = SC_NULL) = 0;
  virtual ulong64 insertData(const scString &a_tableName, const scDataNode *values, const scDataNode *columnNames = SC_NULL);
  virtual ulong64 updateData(const scString &a_tableName, const scDataNode *values, const scDataNode *columnNames = SC_NULL, const scDataNode *selectorParams = SC_NULL, const scString *selectorSql = SC_NULL);
  virtual ulong64 deleteData(const scString &a_tableName, const scDataNode *selectorParams = SC_NULL, const scString *selectorSql = SC_NULL);
  virtual int rowsAffected() = 0; 
  virtual ulong64 getLastInsertedId() = 0;
  virtual void startTrans(const scString &name = scString("")) = 0;
  virtual void commit(const scString &name = scString("")) = 0;
  virtual void rollback(const scString &name = scString("")) = 0;

  virtual bool tableExists(const scString &name) = 0;
  virtual bool getMetaObjExists(scDbObjType oType, const scString &objName) = 0;
  virtual void getMetaObjList(scDbObjType oType, scDataNode &output) = 0;

  /// Returns first row as named item list
  virtual bool getRow(const scString &sql, scDataNode *params, scDataNode &output); 
  /// Returns first row as vector of columns
  virtual bool getVector(const scString &sql, scDataNode *params, scDataNode &output); 
  /// Returns all rows, each as one item
  virtual bool getRowsAsVector(const scString &sql, const scDataNode *params, scDataNode &output); 
  /// Returns all rows, each as one item, with limit & offset support
  virtual bool getRowsAsVector(const scString &sql, const scDataNode *params, ulong64 limit, ulong64 offset, scDataNode &output); 
  virtual bool getValue(const scString &sql, scDataNode *params, scDataNode &output, scDataNode *defValue = SC_NULL); 
  static scDataNode *newExpr(const scString &name, const scString &value);
};

// ----------------------------------------------------------------------------
// scDbCursor
// ----------------------------------------------------------------------------
/// Abstract cursor class interface
class scDbCursor {
public:
  scDbCursor() {}
  virtual ~scDbCursor() {}
  virtual bool eof() = 0;
  virtual scDataNode &fetch() = 0;
  virtual int fieldCount() const = 0;
  virtual scString fieldName(int fieldNo) const = 0;
};

/// Cursor with link to database
class scDbCursorDb: public scDbCursor {
public:
    scDbCursorDb(scDbBase *database): m_database(database) {}
    virtual ~scDbCursorDb() {}
protected:
    scDbBase *m_database;  
};

#endif // _DBBASE_H__
