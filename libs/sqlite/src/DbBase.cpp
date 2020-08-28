/////////////////////////////////////////////////////////////////////////////
// Name:        DbBase.cpp
// Project:     scLib
// Purpose:     SQL database abstraction layer
// Author:      Piotr Likus
// Modified by:
// Created:     20/12/2008
/////////////////////////////////////////////////////////////////////////////
#include "sc/db/DbBase.h"

// ----------------------------------------------------------------------------
// scDbBase
// ----------------------------------------------------------------------------
scDbBase::scDbBase()
{
}

scDbBase::~scDbBase()
{
}

bool scDbBase::isSupported(unsigned int flags, unsigned int domain)
{
  return false;
}

/// perform SQL insert
ulong64 scDbBase::insertData(const scString &a_tableName, const scDataNode *values, const scDataNode *columnNames)
{
  scString columns, sqlTxt, nameTxt;
  scDataNode columnNameList, valueList, paramList, name;
  
  if (columnNames != SC_NULL)
  {
    columns = columnNames->implode(",");
  } else if (values->isParent()) {
    columns = values->childNames().implode(",");
  } 
  
  if (columns.length() <= 0) {
    throw scError("insertData: column names not provided");
  }
  
  scDataNode::explode(",", columns, columnNameList);
  std::auto_ptr<scDataNode> paramGuard;
  paramList.setAsParent();
   
  for(int i=0, epos = columnNameList.size(); i != epos; i++)
  {    
    columnNameList.getElement(i, name);  
    nameTxt = name.getAsString();
    paramGuard.reset(new scDataNode((*values)[i]));
    paramList.addChild(nameTxt, paramGuard.release());
    valueList.addItemAsString("{"+nameTxt+"}");
  }
  
  sqlTxt = 
  "insert into "+a_tableName+" ("+columns+") values("+valueList.implode(",")+")";
  
  return execute(sqlTxt, &paramList);
}

/// perform SQL update
ulong64 scDbBase::updateData(const scString &a_tableName, 
  const scDataNode *values, const scDataNode *columnNames, const scDataNode *selectorParams, const scString *selectorSql)
{
  scString columns, sqlTxt, nameTxt, paramName, whereTxt;
  scDataNode columnNameList, paramList, name, setList, selectorList, param;
  assert((selectorParams != SC_NULL) || (selectorSql != SC_NULL));    
  
  if (columnNames != SC_NULL)
  {
    columns = columnNames->implode(",");
  } else if (values->isParent()) {
    columns = values->childNames().implode(",");
  } 
  
  if (columns.length() <= 0) {
    throw scError("updateData: column names not provided");
  }
  
  scDataNode::explode(",", columns, columnNameList);
  std::auto_ptr<scDataNode> paramGuard;

  paramList.setAsParent();
   
  for(int i=0, epos = columnNameList.size(); i != epos; i++)
  {    
    columnNameList.getElement(i, name);  
    nameTxt = name.getAsString();
    paramName = "v_"+nameTxt;
    setList.addItemAsString(nameTxt+" = "+"{"+paramName+"}");
    
    paramGuard.reset(new scDataNode());
    if (values->size())
      values->getElement(i, *paramGuard);
    else
      paramGuard->copyFrom(*values);  
    paramList.addChild(paramName, paramGuard.release());
  }
  
  if (selectorParams != SC_NULL)
  {
    if (selectorParams->size()) { 
      for(int i=0, epos = selectorParams->size(); i != epos; i++)
      {    
        selectorParams->getElement(i, param);  
        nameTxt = selectorParams->getElementName(i);  
        paramName = "s_"+nameTxt;
        selectorList.addItemAsString(nameTxt+" = "+"{"+paramName+"}");
        
        paramGuard.reset(new scDataNode(param));
        paramList.addChild(paramName, paramGuard.release());
      }
    } 
  }

  whereTxt = selectorList.implode(" and ");
  if (selectorSql != SC_NULL)
  {
    if (whereTxt.length()>0)
      whereTxt += " and ";
    whereTxt += "( " +(*selectorSql)+" )";
  }
    
  if (whereTxt.length() <= 0)
    throw scError("updateData: empty filter condition");

  sqlTxt = 
  "update "+a_tableName+
  " set "+setList.implode(",")+
  " where "+whereTxt;
    
  return execute(sqlTxt, &paramList);
}

ulong64 scDbBase::deleteData(const scString &a_tableName, const scDataNode *selectorParams, const scString *selectorSql)
{
  scString sqlTxt, nameTxt, whereTxt;
  scDataNode paramList, selectorList, param;

  assert((selectorParams != SC_NULL) || (selectorSql != SC_NULL));    
  std::auto_ptr<scDataNode> paramGuard;
  paramList.setAsParent();
  
  if (selectorParams != SC_NULL)
  {
    for(int i=0, epos = selectorParams->size(); i != epos; i++)
    {    
      selectorParams->getElement(i, param);  
      //nameTxt = param.getName();
      nameTxt = selectorParams->getElementName(i);  
      selectorList.addItemAsString(nameTxt+" = "+"{"+nameTxt+"}");
      
      paramGuard.reset(new scDataNode(param));
      paramList.addChild(paramGuard.release());
    }
  }

  whereTxt = selectorList.implode(" and ");
  if (selectorSql != SC_NULL)
  {
    if (whereTxt.length()>0)
      whereTxt += " and ";
    whereTxt += "( " +(*selectorSql)+" )";
  }
    
  if (whereTxt.length() <= 0)
    throw scError("deleteData: empty filter condition");

  sqlTxt = 
  "delete from "+a_tableName+
  " where "+whereTxt;
    
  return execute(sqlTxt, &paramList);
}

bool scDbBase::getRow(const scString &sql, scDataNode *params, scDataNode &output)
{
  bool res = false;
  scDbCursorTransporter guard = select(sql, params);
  output.clear();
  if (!guard->eof())
  {
    output = guard->fetch();
    res = true;
  }
  return res;  
}

bool scDbBase::getVector(const scString &sql, scDataNode *params, scDataNode &output)
{
  scDbCursorTransporter guard = select(sql, params);
  scDataNode element, value;
  output.clear();
  if (!guard->eof())
  {
    element = guard->fetch();
    if (element.size())
    {
      for(int i=0; i != element.size(); i++)
      {
        element.getElement(i, value);
        output.addItem(value);
      }
    }
    else
      output = element;        
  }
  return (!output.empty());  
}

bool scDbBase::getRowsAsVector(const scString &sql, const scDataNode *params, scDataNode &output)
{
    return getRowsAsVector(sql, params, 0, 0, output);
}

bool scDbBase::getRowsAsVector(const scString &sql, const scDataNode *params, ulong64 limit, ulong64 offset, scDataNode &output)
{
  scDbCursorTransporter guard = select(sql, params);
  scDataNode element;
  ulong64 toSkip = offset;
  output.clear();

  while (!guard->eof())
  {
    element = guard->fetch();

    if (toSkip > 0)
      toSkip--;
    else
      output.addItem(element);

    if (limit > 0) 
        if (output.size() >= limit)
            break;
  }

  return (!output.empty());  
}

bool scDbBase::getValue(const scString &sql, scDataNode *params, scDataNode &output, scDataNode *defValue)
{
  bool res = true;
  scDbCursorTransporter guard = select(sql, params);
  output.clear();
  if (!guard->eof())
  {
    scDataNode element = guard->fetch();
    element.getElement(0, output);
  } else {
    if (defValue != SC_NULL)
      output = *defValue; 
    res = false;
  }  
  return (res);  
}

scDataNode *scDbBase::newExpr(const scString &name, const scString &value)
{
  std::auto_ptr<scDataNode> guard(new scDataNode(name));
  guard->addChild("expr", new scDataNode(true));
  guard->addChild("value", new scDataNode(value));
  return guard.release();
}


