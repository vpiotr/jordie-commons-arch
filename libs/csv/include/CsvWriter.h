/////////////////////////////////////////////////////////////////////////////
// Name:        CsvWriter.h
// Project:     scLib
// Purpose:     Writer class for CSV format
// Author:      Piotr Likus
// Modified by:
// Created:     01/12/2008
/////////////////////////////////////////////////////////////////////////////


#ifndef _CSVWRITER_H__
#define _CSVWRITER_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file CsvWriter.h
///
/// 

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "sc/dtypes.h"
#include "sc/txtf/TxtWriter.h"

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
class scCsvWriter: public scTxtWriter {
public:
  scCsvWriter(char sepChar = ',', char quoteChar = '"', scDataNode *header = SC_NULL, 
    bool forceQuoted = false, bool useDataForHeader = false);
  virtual ~scCsvWriter();
  void setup(char sepChar = ',', char quoteChar = '"', scDataNode *header = SC_NULL, 
    bool forceQuoted = false, bool useDataForHeader = false);    
  void setUseDataForHeader(bool value);
  void setSepChar(char value);
protected:  
  virtual void writeHeader(const scDataNode &input);  
  void formatLine(const scDataNode &input, scString &output);
protected:
  char m_sepChar;
  char m_quoteChar;  
  bool m_forceQuoted;
  scDataNode m_header;
  bool m_useDataForHeader;
};

class scCsvNumericFormatter: public scTxtWriterValueFormatter {
public:
  virtual void formatValue(scString &value) {
    strReplaceThis(value,".",",", true);
  }
};

#endif // _CSVWRITER_H__
