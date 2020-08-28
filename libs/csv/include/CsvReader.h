/////////////////////////////////////////////////////////////////////////////
// Name:        CsvReader.h
// Project:     scLib
// Purpose:     Reader for CSV format, output: scDataNode
// Author:      Piotr Likus
// Modified by:
// Created:     30/11/2008
/////////////////////////////////////////////////////////////////////////////


#ifndef _CSVREADER_H__
#define _CSVREADER_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file CsvReader.h
///
/// 

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "sc/dtypes.h"
#include "sc/txtf/TxtReader.h"

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
class scCsvReader: public scTxtReader {
public:
  scCsvReader(char sepChar = ',', char quoteChar = '"', bool headerIncluded = false);
  virtual ~scCsvReader();
  void setup(char sepChar = ',', char quoteChar = '"', bool headerIncluded = false);  
  virtual bool parseLine(const scString &line, scDataNode &output);
  bool getHeader(scDataNode &output);
  void setColumnTypes(const scDataNode &valueList);
  void getColumnTypes(scDataNode &valueList);
protected:
  void throwSyntaxError(const scString &text, int a_pos, int a_char) const;
  bool isControl(int c) {return ((unsigned char)c < 32);};
  virtual void initParse();
  virtual void handleParsedLine(scDataNode &input, scDataNode &output);
  virtual void addCellToRow(const scString &value, scDataNode &output);
  virtual void initLineOutput(scDataNode &output);
  virtual scDataNodeValueType getColumnDataType(uint idx);
  virtual bool parseValue(const scString &value, uint columnIndex, scDataNode &output);
protected:
  char m_sepChar;
  char m_quoteChar;  
  bool m_headerIncluded;
  bool m_headerParsed;
  scDataNode m_header;
  scDataNode m_columnTypes;
};

#endif // _CSVREADER_H__
