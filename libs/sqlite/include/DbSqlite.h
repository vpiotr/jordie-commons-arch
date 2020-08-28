/////////////////////////////////////////////////////////////////////////////
// Name:        DbSqlite.h
// Project:     scLib
// Purpose:     SQL database classes for SQLite.
// Author:      Piotr Likus
// Modified by:
// Created:     20/12/2008
/////////////////////////////////////////////////////////////////////////////

#ifndef _DBSQLITE_H__
#define _DBSQLITE_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file DbSqlite.h
///
/// scDbSqlite - database class
/// scDbCursor - query result class

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include <list>
#include "sc/db/DbBase.h"

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Forward class definitions
// ----------------------------------------------------------------------------
class scDbCursorSqlite;

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
//#define DBSQLITE_LOG_SQL_IN_ERRORS 
//#define DBSQLITE_LOG_EACH_SQL
#define DBSQLITE_ADD_SQL_TO_ERRORS 

// ----------------------------------------------------------------------------
// Class definitions
// ----------------------------------------------------------------------------
typedef std::list<scDbCursorSqlite *> scDbCursorListSqlite;

// ----------------------------------------------------------------------------
// scDbSqlite
// ----------------------------------------------------------------------------
class scDbSqlite: public scDbBase {
public:
  scDbSqlite();
  virtual ~scDbSqlite();
  virtual bool isSupported(unsigned int flags, unsigned int domain = 0);
  virtual scString getVersion() const;  
  virtual void connect(const scString &params);
  virtual void disconnect();
  virtual bool isConnected() const;
  virtual void selectContext(const scString &ctx);
  virtual ulong64 execute(const scString &sql, const scDataNode *params = SC_NULL);
  virtual scDbCursorTransporter select(const scString &sql, const scDataNode *params = SC_NULL);

  virtual bool tableExists(const scString &name);
  virtual bool getMetaObjExists(scDbObjType oType, const scString &objName);
  virtual void getMetaObjList(scDbObjType oType, scDataNode &output);

  virtual int rowsAffected(); 
  virtual ulong64 getLastInsertedId();
  virtual void startTrans(const scString &name = scString(""));
  virtual void commit(const scString &name = scString(""));
  virtual void rollback(const scString &name = scString(""));
  virtual void addCursor(const scDbCursorSqlite *cursor);
  virtual void removeCursor(const scDbCursorSqlite *cursor);
protected:  
  void bindParam(const scDataNode &param, int pos, void *a_stmt);
  ulong64 executeByPos(const scString &sql, const scDataNode *params);
  ulong64 executeByName(const scString &sql, const scDataNode *params);
  ulong64 executeNoParams(const scString &sql);
  void parseNamedParams(const scString &sql, const scDataNode *params, scString &nsql, scDataNode &posParams);
  scDbCursorTransporter selectNoParams(const scString &sql);
  scDbCursorTransporter selectByName(const scString &sql, const scDataNode *params);
  scDbCursorTransporter selectByPos(const scString &sql, const scDataNode *params);
  void checkOpenCursors();
  scString encodeObjType(scDbObjType oType);
protected:
  void *m_handle;
  int m_lastRowsAffected;
  bool m_connected; 
  scDbCursorListSqlite m_openCursors; 
};

// ----------------------------------------------------------------------------
// scDbCursorSqlite
// ----------------------------------------------------------------------------
class scDbCursorSqlite: public scDbCursorDb {
public:
  scDbCursorSqlite(scDbSqlite *database, void *queryHandle, void *stmtHandle);
  virtual ~scDbCursorSqlite();  
  virtual bool eof();
  virtual scDataNode &fetch();
  virtual int fieldCount() const;
  virtual scString fieldName(int fieldNo) const;
protected:
  void initColTypes();  
protected:
  scDataNode m_valueRow;    
  scDataNode m_typeRow;    
  void *m_dbHandle;  
  void *m_qryHandle;
  void *m_stmtHandle;
};

#endif // _DBSQLITE_H__
