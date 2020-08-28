/////////////////////////////////////////////////////////////////////////////
// Name:        CsvReader.cpp
// Project:     scLib
// Purpose:     Reader for CSV format, output: scDataNode
// Author:      Piotr Likus
// Modified by:
// Created:     30/11/2008
/////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include "sc/txtf/CsvReader.h"
#include "sc/utils.h"

#ifdef DEBUG_MEM
#include "sc/DebugMem.h"
#endif

using namespace dtp;

scCsvReader::scCsvReader(char sepChar, char quoteChar , bool headerIncluded): scTxtReader()
{
  m_fileHandle = SC_NULL;
  setup(sepChar, quoteChar, headerIncluded);
}

scCsvReader::~scCsvReader()
{
}

void scCsvReader::setup(char sepChar, char quoteChar, bool headerIncluded)
{
  m_sepChar = sepChar;
  m_quoteChar = quoteChar;
  m_headerIncluded = headerIncluded;
}

void scCsvReader::initParse()
{
  m_headerParsed = false;
}

void scCsvReader::handleParsedLine(scDataNode &input, scDataNode &output)
{
  if (m_headerParsed || !m_headerIncluded)
    scTxtReader::handleParsedLine(input, output);
  else {
    m_headerParsed = true;
    m_header = input;
  }      
}

bool scCsvReader::parseLine(const scString &line, scDataNode &output)
{
  int c = 0, cp1 = 0;
  int sepChar = m_sepChar, setQuoteChar = m_quoteChar;
  scString::size_type i, m;
  scString::size_type endPos = line.length();
  const int EOF_CHAR = 26;
  scString value;
  scDataNode newValue;

  int ctx;
  int quoteChar = 0;

  enum {
    CTX_START, 
    CTX_VALUE, 
    CTX_VAL_NQUOTED,
    CTX_VAL_QUOTED,
    CTX_END
  };  
  
  initLineOutput(output);
  
  ctx = CTX_START;
  i = 0;
  while( (i<(int)endPos) && (ctx != CTX_END))  
  {
    c = line[i];

    if (i < endPos - 1) cp1 = line[i + 1];
    else cp1 = 0;
        
    if (isControl(c)) {     
      if (c != EOF_CHAR)
        throwSyntaxError(line, i, c); 
      else
        ctx = CTX_END;  
    }  
    
    if (ctx == CTX_START)
    {
      m = line.find_first_not_of(" \t", i);
      if (m != scString::npos) {       
        i = m;
        ctx = CTX_VALUE;
      } else {
        ctx = CTX_END;
      }         
    } else if (ctx == CTX_VALUE) {
    //undetected value type yet
      if (c == setQuoteChar)
      {
        ctx = CTX_VAL_QUOTED;
        quoteChar = c;
        i++;
      } else if (c == sepChar) {
        addCellToRow(value, output);
        value = "";
        i++;
      } else {  
        ctx = CTX_VAL_NQUOTED;        
      }      
    } else if (ctx == CTX_VAL_QUOTED) {
    // value is quoted
      if (c == quoteChar) { 
        if (cp1 == quoteChar) {
        // double quote char - include in output
          value += char(c);
          i+=2;
        } else {
        // end of qoted string - add value
          addCellToRow(value, output);          
          value = "";
          m = line.find_first_not_of(" \t", i+1);
          if (m != scString::npos) {       
            if (line[m] == sepChar)
              i = m+1;
            else  
              i = m;
            ctx = CTX_VALUE;  
          } else {
            ctx = CTX_END;
          }    
        } // end of quoted string
     } // char - quote char
     else {
     // std char in quoted string
       value += char(c);
       i++;
     }   
   } // ctx = quoted 
   else if (ctx == CTX_VAL_NQUOTED) {
     if (c == sepChar) {
     // end of non-quoted value
        addCellToRow(value, output);
        value = "";
        m = line.find_first_not_of(" \t", i+1);
        if (m != scString::npos) {       
          i = m;
          ctx = CTX_VALUE;  
        } else {
          ctx = CTX_END;
        }         
     } else {
     //std char in not quoted string
       value += char(c);
       i++;
     }
   } // ctx = not quoted
   else if (ctx != CTX_END) {
     throwSyntaxError(line, i, c);    
   }  
  } // while 

  if (!value.empty())
  {
    addCellToRow(value, output);
  }
  
  return true;
} // function

void scCsvReader::addCellToRow(const scString &value, scDataNode &output)
{
  bool bDone = false;
  scDataNode valueNode;
   
  if (m_headerParsed || !m_headerIncluded) {
    if (m_parser != SC_NULL) {
      bDone = parseValue(value, output.size(), valueNode);
    }
  }

  if (!bDone)
      valueNode.setAs(value);

  output.addItem(valueNode);
}

bool scCsvReader::parseValue(const scString &value, uint columnIndex, scDataNode &output)
{
    bool res = false; 
    scDataNodeValueType vtype = getColumnDataType(columnIndex);
    if (vtype != vt_null) {
        m_parser->parseValue(value, vtype, output);
        res = true;
    }
    return res;
}

void scCsvReader::initLineOutput(scDataNode &output)
{
  output.clear();
  output.setAsList();  
}

void scCsvReader::throwSyntaxError(const scString &text, int a_pos, int a_char) const {
   throw scError("CSV syntax error, wrong character ["+toString(a_char)+"] in line ["+text+"] at pos: "+toString(a_pos));
}

bool scCsvReader::getHeader(scDataNode &output)
{
  bool res = (!m_header.isNull());
  if (res)
    output = m_header;
  return res;  
}

void scCsvReader::setColumnTypes(const scDataNode &valueList)
{
    m_columnTypes = valueList;
}

void scCsvReader::getColumnTypes(scDataNode &valueList)
{
    valueList = m_columnTypes;  
}

scDataNodeValueType scCsvReader::getColumnDataType(uint idx)
{
    if (m_columnTypes.size() > idx)
        return m_columnTypes.getElementType(idx);
    else
        return vt_null;
}

