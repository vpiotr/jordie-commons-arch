/////////////////////////////////////////////////////////////////////////////
// Name:        DbSqlite.cpp
// Project:     scLib
// Purpose:     Interface for Sqlite
// Author:      Piotr Likus
// Modified by:
// Created:     20/12/2008
/////////////////////////////////////////////////////////////////////////////
#include "sc/db/DbSqlite.h"
#include "sqlite3.h"
#include "CppSQLite3.h"
#include "sc/utils.h"
#include "perf/Log.h"

using namespace dtp;
using namespace perf;

// ----------------------------------------------------------------------------
// scDbSqlite
// ----------------------------------------------------------------------------
scDbSqlite::scDbSqlite()
{
  m_handle = new CppSQLite3DB();
  m_lastRowsAffected = -1;
}

scDbSqlite::~scDbSqlite()
{ 
  try {
    disconnect();
  }
  catch(const std::exception& e) {
    Log::addError(scString("sqlite disconnect error - [")+e.what()+"]");
  }
  
  delete static_cast<CppSQLite3DB *>(m_handle);
}

scString scDbSqlite::getVersion() const 
{
  return scString("sqlite:CppSQLite3:scDbSqlite:")+scString(static_cast<CppSQLite3DB *>(m_handle)->SQLiteVersion());
}

bool scDbSqlite::isSupported(unsigned int flags, unsigned int domain)
{
  if ((flags & SUPF_BIND_BY_POS) == flags)
    return true;
  else  
    return false;
}

void scDbSqlite::connect(const scString &params)
{
  disconnect();
  static_cast<CppSQLite3DB *>(m_handle)->open(params.c_str());
  m_connected = true;
}

void scDbSqlite::disconnect()
{
  if (m_connected)
  { 
    checkOpenCursors();
    static_cast<CppSQLite3DB *>(m_handle)->close();
    m_connected = false;
  }  
}

bool scDbSqlite::isConnected() const
{
  return m_connected;
}

void scDbSqlite::selectContext(const scString &ctx)
{ // do nothing (not required)
}

ulong64 scDbSqlite::execute(const scString &sql, const scDataNode *params)
{
  if (params == SC_NULL) 
    return executeNoParams(sql);
  else if (params->isArray())
    return executeByPos(sql, params);
  else if (!params->isParent())    
  {
    scDataNode arr;
    arr.addItem(*params);
    return executeByPos(sql, &arr);    
  }
  else if (params->isParent())
    return executeByName(sql, params);
  else {
    scDataNode parent, child;
    child.copyValueFrom(*params);
    parent.addItem(child);
    return executeByName(sql, &parent);       
  }  
}

ulong64 scDbSqlite::executeNoParams(const scString &sql)
{
  m_lastRowsAffected = static_cast<CppSQLite3DB *>(m_handle)->execDML(sql.c_str());
  return m_lastRowsAffected;
}

ulong64 scDbSqlite::executeByName(const scString &sql, const scDataNode *params)
{
  scString nsql;
  scDataNode posParams;

  parseNamedParams(sql, params, nsql, posParams);
  return executeByPos(nsql, &posParams);  
}

// - find every {name} macro
// - add name to list (case sensitive)
// - replace with ?
// - on the end replace every '\{' and '\}' with '{' and '}' 
// - execute by pos
void scDbSqlite::parseNamedParams(const scString &sql, const scDataNode *params, scString &nsql, scDataNode &posParams)
{
  size_t i, j, m, len;
  int foundCnt;
  char c; 
  scString macroName, paramName;
  scDataNode paramNameNode;
  
  posParams.clear();
  len = sql.length();
  nsql = "";
  
  i = 0;
  while (i < len)
  {
    m = sql.find_first_of("{", i);
    if (m == std::string::npos || m >= len + 1)
    {
      nsql += sql.substr(i, len - i);
      i = len;
    } else {
    // delimiter found
      j = m + 1;
      macroName = "";      
      c = sql[j];
      while ((c >= 'a' && c <= 'z') || 
          (c >= 'A' && c <= 'Z') || 
          (c >= '0' && c <= '9') ||
          (c == '_'))
      {
        if (j < len - 1)
        {
          j++;
          c = sql[j];
        } else {
          break;
        }          
      } // while c in macro name     
      if ((c == '}') && (j - m - 1 > 0))
      { // macro accepted 
        macroName = sql.substr(m + 1, j - m - 1);
        nsql += sql.substr(i, m - i);
        
        if (params->hasChild(macroName) && 
              (
                (*params)[macroName].hasChild("expr")
                ||
                (*params)[macroName].hasChild("expression")
              )
           ) {
          nsql += (*params)[macroName].getString("value");
        } else {   
          posParams.addItem(scDataNode(macroName));
          nsql += '?';
        }
        i = j + 1;
      } else {
      // incorrect macro
        nsql += sql.substr(i, j - i);
        i = j;
      }           
    } // delimiter found
  } // while i
  foundCnt = posParams.size();
  for(int k=0; k < foundCnt; k++)
  {
    posParams.getElement(k, paramNameNode);
    paramName = paramNameNode.getAsString();
    if (params->hasChild(paramName))
    {
      posParams.setElement(k, (*params)[paramName]);
    }
  }
  
  strReplaceThis(nsql, "\\{", "{", true);
  strReplaceThis(nsql, "\\}", "}", true);
  strReplaceThis(nsql, "\\\\", "\\", true);  
} // function 

ulong64 scDbSqlite::executeByPos(const scString &sql, const scDataNode *params)
{
#ifdef DBSQLITE_LOG_EACH_SQL
  Log::addText("scDbSqlite.executeByPos: "+sql);
#endif
  try {  
    CppSQLite3Statement stmt = static_cast<CppSQLite3DB *>(m_handle)->compileStatement(sql.c_str());
    if (params != SC_NULL)
    {
      int cnt = params->size();
      scDataNode param;    
      for(int i=0; i < cnt; i++)
      {
        params->getElement(i, param);
        bindParam(param, i+1, &stmt);
      }
    }
    m_lastRowsAffected = stmt.execDML();  
  }   
  catch(const std::exception& e) {
    scString msg;
    msg = scString("scDbSqlite::executeByPos - [")+e.what()+"], "+ sql;
#ifdef DBSQLITE_LOG_SQL_IN_ERRORS  
    Log::addError(msg);    
#endif    
#ifdef DBSQLITE_ADD_SQL_TO_ERRORS    
    throw scError(msg);
#else
    throw;
#endif        
  }  
  return m_lastRowsAffected;
}

void scDbSqlite::bindParam(const scDataNode &param, int pos, void *a_stmt)
{
  using namespace dtp;
  CppSQLite3Statement *stmt = static_cast<CppSQLite3Statement *>(a_stmt);
  
  switch (const_cast<scDataNode &>(param).getValueType())
  {
    case vt_byte:
    case vt_int:
      stmt->bind(pos, int(const_cast<scDataNode &>(param).getAsInt()));
      break;
    case vt_double:  
      stmt->bind(pos, double(const_cast<scDataNode &>(param).getAsDouble()));
      break;
    case vt_null:  
      stmt->bindNull(pos);
      break;
    default:
      stmt->bind(pos, const_cast<scDataNode &>(param).getAsString().c_str());
      break;        
  }
}

scDbCursorTransporter scDbSqlite::select(const scString &sql, const scDataNode *params)
{
  if (params == SC_NULL) 
    return selectNoParams(sql);
  else if (params->isArray())
    return selectByPos(sql, params);
  else if (!params->isParent() )    
  {
    scDataNode arr;
    arr.addItem(*params);
    return selectByPos(sql, &arr);    
  }
  else if (params->isParent())
    return selectByName(sql, params);
  else {
    scDataNode parent, child;
    child.copyValueFrom(*params);
    parent.addItem(child);
    return selectByName(sql, &parent);       
  }  
}

scDbCursorTransporter scDbSqlite::selectNoParams(const scString &sql)
{
  std::auto_ptr<CppSQLite3Query> guard(new CppSQLite3Query());
  *guard = static_cast<CppSQLite3DB *>(m_handle)->execQuery(sql.c_str());  
  std::auto_ptr<scDbCursor> guardRes(new scDbCursorSqlite(this, guard.release(), SC_NULL));
  return scDbCursorTransporter(guardRes.release());
}

scDbCursorTransporter scDbSqlite::selectByName(const scString &sql, const scDataNode *params)
{
  scString nsql;
  scDataNode posParams;

  parseNamedParams(sql, params, nsql, posParams);
  return selectByPos(nsql, &posParams);  
}

scDbCursorTransporter scDbSqlite::selectByPos(const scString &sql, const scDataNode *params)
{
#ifdef DBSQLITE_LOG_EACH_SQL
  Log::addText("scDbSqlite.selectByPos: "+sql);
#endif
  try {
    std::auto_ptr<CppSQLite3Statement> stmtGuard;
    stmtGuard.reset(
      new CppSQLite3Statement(
        static_cast<CppSQLite3DB *>(m_handle)->compileStatement(sql.c_str())
      )
    );    
    if (params != SC_NULL)
    {
      int cnt = params->size();
      scDataNode param;    
      for(int i=0; i < cnt; i++)
      {
        params->getElement(i, param);
        bindParam(param, i+1, stmtGuard.get());
      }
    }
    std::auto_ptr<scDbCursor> guardRes(
      new scDbCursorSqlite(
        this, 
        new CppSQLite3Query(stmtGuard->execQuery()),
        stmtGuard.release()
      ));
    return scDbCursorTransporter(guardRes.release());
  }   
  catch(const std::exception& e) {
    scString msg;
    msg = scString("scDbSqlite::selectByPos - [")+e.what()+"], "+ sql;
#ifdef DBSQLITE_LOG_SQL_IN_ERRORS  
    Log::addError(msg);    
#endif    
#ifdef DBSQLITE_ADD_SQL_TO_ERRORS    
    throw scError(msg);
#else
    throw;
#endif        
  }
}

int scDbSqlite::rowsAffected()
{
  return m_lastRowsAffected;
}

bool scDbSqlite::tableExists(const scString &name)
{
  return static_cast<CppSQLite3DB *>(m_handle)->tableExists(name.c_str());
}

scString scDbSqlite::encodeObjType(scDbObjType oType)
{
    scString res;

    switch (oType) {
    case dotTable: 
        res = "table";
        break;
    case dotView: 
        res = "view";
        break;
    case dotIndex: 
        res = "index";
        break;
    default: 
        res = "";
        break;
    }

    return res;
}

bool scDbSqlite::getMetaObjExists(scDbObjType oType, const scString &objName)
{   
    if (oType == dotTable)
        return tableExists(objName);
    
    scString oTypeSqlite = encodeObjType(oType);

    scDataNode params(ict_parent);
    params.addElement("otype", scDataNode(oTypeSqlite));
    params.addElement("oname", scDataNode(objName));

    scString sqlText = "select 1 from sqlite_master where type = {otype} and name = {oname}";
    scDataNode row;
    getRow(sqlText, &params, row);

    return (!row.empty());
}

void scDbSqlite::getMetaObjList(scDbObjType oType, scDataNode &output) 
{
    scString oTypeSqlite = encodeObjType(oType);

    scDataNode params(ict_parent);
    params.addElement("otype", scDataNode(oTypeSqlite));

    scString sqlText = "select name as oname from sqlite_master where type = {otype}";
    output = scDataNode(ict_list);
    getRowsAsVector(sqlText, &params, output);
}

ulong64 scDbSqlite::getLastInsertedId()
{
  return static_cast<CppSQLite3DB *>(m_handle)->lastRowId();  
}

void scDbSqlite::startTrans(const scString &name)
{
  execute(scString("begin transaction ")+name+";");
}

void scDbSqlite::commit(const scString &name)
{
  execute(scString("commit transaction ")+name+";");
}

void scDbSqlite::rollback(const scString &name)
{
  execute(scString("rollback transaction ")+name+";");
}

void scDbSqlite::checkOpenCursors()
{
  if (m_openCursors.size())
    throw scError("Pending open cursors ("+toString(m_openCursors.size())+") exists!");
}

void scDbSqlite::addCursor(const scDbCursorSqlite *cursor)
{
  removeCursor(cursor);
  m_openCursors.push_back(const_cast<scDbCursorSqlite *>(cursor));
}

void scDbSqlite::removeCursor(const scDbCursorSqlite *cursor)
{
  scDbCursorListSqlite::iterator it = find(m_openCursors.begin(), m_openCursors.end(), cursor);
  if (it != m_openCursors.end())
    m_openCursors.erase(it);
}


// ----------------------------------------------------------------------------
// scDbCursorSqlite
// ----------------------------------------------------------------------------
scDbCursorSqlite::scDbCursorSqlite(scDbSqlite *database, void *queryHandle, void *stmtHandle): scDbCursorDb(database)
{
  assert(database != SC_NULL);
  assert(queryHandle != SC_NULL);
  m_dbHandle = database;
  m_qryHandle = queryHandle;
  m_stmtHandle = stmtHandle;
  database->addCursor(this);
  initColTypes();
}

scDbCursorSqlite::~scDbCursorSqlite()  
{
  static_cast<scDbSqlite *>(m_dbHandle)->removeCursor(this);
  delete static_cast<CppSQLite3Query *>(m_qryHandle);
  delete static_cast<CppSQLite3Statement *>(m_stmtHandle);
}

int scDbCursorSqlite::fieldCount() const
{
  return m_typeRow.size();
}

scString scDbCursorSqlite::fieldName(int fieldNo) const
{
  return m_typeRow.getElementName(fieldNo);
}

bool scDbCursorSqlite::eof()
{
  return static_cast<CppSQLite3Query *>(m_qryHandle)->eof();
}

scDataNode &scDbCursorSqlite::fetch()
{
  CppSQLite3Query *qry = static_cast<CppSQLite3Query *>(m_qryHandle);

  if (!eof())
  {
    m_valueRow = m_typeRow;
    
    for (int fld = 0; fld < qry->numFields(); fld++)
    {
      if (qry->fieldIsNull(fld))
        m_valueRow[fld].setAsNull();
      else {  
        switch (m_typeRow[fld].getValueType()) {
          case vt_byte:
          case vt_int:
            m_valueRow[fld].setAsInt(qry->getIntField(fld));
            break;
          case vt_double:  
            m_valueRow[fld].setAsDouble(qry->getFloatField(fld));
            break;
          default:  
            m_valueRow[fld].setAsString(qry->getStringField(fld));
            break;
        } // switch type
      } // not null
    } // for fld
    
    qry->nextRow();
  } // not eof  
  
  return m_valueRow;
} // function

void scDbCursorSqlite::initColTypes()
{
  CppSQLite3Query *qry = static_cast<CppSQLite3Query *>(m_qryHandle);
  std::auto_ptr<scDataNode> newNodeGuard;
  scString locFieldName;
  
  m_typeRow.clear();
  for (int fld = 0; fld < qry->numFields(); fld++)
  {
    locFieldName = qry->fieldName(fld); 
    switch (qry->fieldDataType(fld)) {
      case SQLITE_INTEGER:
        m_typeRow.addChild(locFieldName, new scDataNode(0));
        break;
      case SQLITE_FLOAT:
        m_typeRow.addChild(locFieldName, new scDataNode(0.0));
        break;         
      case SQLITE_NULL:
        newNodeGuard.reset(new scDataNode());
        newNodeGuard->setAsNull();        
        m_typeRow.addChild(locFieldName, newNodeGuard.release());
        break;         
      case SQLITE_TEXT:
        m_typeRow.addChild(locFieldName, new scDataNode(""));
        break;        
      default:  
        m_typeRow.addChild(locFieldName, new scDataNode(""));
        break;        
    }
  }
}

