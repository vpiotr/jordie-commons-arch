/////////////////////////////////////////////////////////////////////////////
// Name:        CsvWriter.cpp
// Project:     scLib
// Purpose:     Writer class for CSV format
// Author:      Piotr Likus
// Modified by:
// Created:     01/12/2008
/////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <sstream>

#include "sc/txtf/CsvWriter.h"
#include "sc/dtypes.h"

#ifdef DEBUG_MEM
#include "sc/DebugMem.h"
#endif

scCsvWriter::scCsvWriter(char sepChar, char quoteChar, scDataNode *header, 
  bool forceQuoted, bool useDataForHeader): scTxtWriter()
{
  setup(sepChar, quoteChar, header, forceQuoted, useDataForHeader);
}

scCsvWriter::~scCsvWriter()
{
}

void scCsvWriter::setup(char sepChar, char quoteChar, scDataNode *header, 
  bool forceQuoted, bool useDataForHeader)
{
  m_sepChar = sepChar;
  m_quoteChar = quoteChar;    
  if (header == SC_NULL)
    m_header.clear();
  else
    m_header = *header;
  m_forceQuoted = forceQuoted;
  m_useDataForHeader = useDataForHeader;
}

void scCsvWriter::setUseDataForHeader(bool value)
{
  m_useDataForHeader = value;
}

void scCsvWriter::setSepChar(char value)
{
  m_sepChar = value;
}

void scCsvWriter::writeHeader(const scDataNode &input)
{
  if (!m_header.isNull())
    writeRow(m_header);
  else if (m_useDataForHeader) {
    if (input.size()) {
      scDataNode row;
      input.getElement(0, row);
      if (row.isParent())
        writeRow(row.childNames());
    }
  }  
}

void scCsvWriter::formatLine(const scDataNode &input, scString &output)
{
	std::ostringstream	out;
	scString value;
	bool first = true;
	scString quoteStr = strCharToStr(char(m_quoteChar));
	scString sepStr = strCharToStr(char(m_sepChar));
	scString sepQuoteStr = sepStr+quoteStr;
	scString dblQuoteStr = quoteStr+quoteStr;
	bool forceQuoted = m_forceQuoted;

  if (input.isParent())
  {    
    //for(scDataNode::const_iterator p = input.begin(), epos = input.end(); p != epos; ++p)
    for(uint i=0, epos = input.size(); i != epos; i++)
    { 
      if (!first)
        out << char(m_sepChar);
      else
        first = false;  
      value = input.getString(i);
      formatValue(value);   
      if (forceQuoted || (value.find_first_of(sepQuoteStr) != scString::npos))
      {
        strReplaceThis(value, sepQuoteStr, dblQuoteStr, true);
        value = quoteStr+value+quoteStr;
      }
      out << value.c_str();  
    } // for child
  } else if (input.isArray())
  {
    scDataNode item;
    int cnt = const_cast<scDataNode &>(input).getArray()->size();
    for(int i=0; i<cnt; i++)
    {
      if (!first)
        out << char(m_sepChar);
      else
        first = false;  

      value = input.getString(i);
      formatValue(value);   
      if (forceQuoted || (value.find_first_of(sepQuoteStr) != scString::npos))
      {
        strReplaceThis(value, sepQuoteStr, dblQuoteStr, true);
        value = quoteStr+value+quoteStr;
      }
      out << value.c_str();  
    } // for item
  } else {
    if (!input.isNull())
      value = input.getAsString();
    else
      value = "";  
    formatValue(value);   
    if (forceQuoted || (value.find_first_of(sepQuoteStr) != scString::npos))
    {
      strReplaceThis(value, sepQuoteStr, dblQuoteStr, true);
      value = quoteStr+value+quoteStr;
    }
    out << value.c_str();  
  }    
  
  output = out.str();
}
