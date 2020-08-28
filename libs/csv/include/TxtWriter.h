/////////////////////////////////////////////////////////////////////////////
// Name:        TxtWriter.h
// Project:     scLib
// Purpose:     Writer class for text files
// Author:      Piotr Likus
// Modified by:
// Created:     03/12/2008
/////////////////////////////////////////////////////////////////////////////
#ifndef _TXTWRITER_H__
#define _TXTWRITER_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file TxtWriter.h
///
/// 

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "sc/dtypes.h"

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
class scTxtWriterValueFormatter {
public:
  scTxtWriterValueFormatter() {}
  virtual ~scTxtWriterValueFormatter() {}
  virtual void formatValue(scString &value) = 0;
};

class scTxtWriter {
public:
  scTxtWriter();
  virtual ~scTxtWriter();
  void writeFile(const scDataNode &input, const scString &fname);
  void writeText(const scDataNode &input, scString &output);
  void writeRow(const scDataNode &input);
  void writeLine(const scString &a_line);
  void setTruncateFileFlag(bool value);
  bool getTruncateFileFlag();
  void setFormatter(const scTxtWriterValueFormatter *formatter);
  bool isFileOpen();
  void beginWrite(const scString &fname, const scDataNode &input);
  void endWrite(const scDataNode &input);
protected:  
  virtual void openFile(const scString &fname);
  void writeData(const scDataNode &input);
  virtual void checkWriteHeader(const scDataNode &input);  
  virtual void writeHeader(const scDataNode &input);  
  virtual void writeFooter(const scDataNode &input);
  virtual void formatLine(const scDataNode &input, scString &output);
  virtual void formatValue(scString &value);
  virtual bool isFileEmpty(const scString &fname);
  void checkClosed();
protected:
  void *m_fileHandle;
  void *m_textHandle;
  bool m_truncateFileFlag;
  bool m_fileEmpty;
  scTxtWriterValueFormatter *m_formatter;
};

#endif // _FIXWRITER_H__
